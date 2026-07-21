// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Decimal.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/DivideByZeroException.hpp"
#include "System/FormatException.hpp"
#include "System/OverflowException.hpp"
#include <algorithm>
#include <cmath>

namespace System {

// ---------------------------------------------------------------------------
// File-scope helpers
// ---------------------------------------------------------------------------

static constexpr double kPow10[29] = {
    1e0,  1e1,  1e2,  1e3,  1e4,  1e5,  1e6,  1e7,  1e8,  1e9,
    1e10, 1e11, 1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19,
    1e20, 1e21, 1e22, 1e23, 1e24, 1e25, 1e26, 1e27, 1e28
};

// ---------------------------------------------------------------------------
// Private static helpers
// ---------------------------------------------------------------------------

Decimal::uint192 Decimal::mul96x96(u128 a, u128 b) {
    uint64_t a0 = uint64_t(a),       a1 = uint64_t(a >> 64);
    uint64_t b0 = uint64_t(b),       b1 = uint64_t(b >> 64);
    u128 p00 = u128(a0) * b0;
    u128 p01 = u128(a0) * b1;
    u128 p10 = u128(a1) * b0;
    u128 p11 = u128(a1) * b1;
    uint192 r;
    r.lo  = uint64_t(p00);
    u128 c1 = (p00 >> 64) + p01 + p10;
    r.mid = uint64_t(c1);
    u128 c2 = (c1 >> 64) + p11;
    r.hi  = uint64_t(c2);
    return r;
}

uint32_t Decimal::div192by10(uint192& v) {
    u128 r = v.hi;
    v.hi  = uint64_t(r / 10);
    r     = ((r % 10) << 64) | v.mid;
    v.mid = uint64_t(r / 10);
    r     = ((r % 10) << 64) | v.lo;
    v.lo  = uint64_t(r / 10);
    return uint32_t(r % 10);
}

bool Decimal::fits96(const uint192& v) {
    return v.hi == 0 && (v.mid >> 32) == 0;
}

Decimal::u128 Decimal::to128(const uint192& v) {
    return (u128(v.mid) << 64) | v.lo;
}

void Decimal::alignScales(u128& m1, uint8_t& s1, u128& m2, uint8_t& s2) {
    while (s1 < s2) {
        if (m1 <= MAX_MANTISSA / 10) { m1 *= 10; ++s1; }
        else { uint32_t rem = uint32_t(m2 % 10); m2 /= 10; if (rem >= 5) ++m2; --s2; }
    }
    while (s2 < s1) {
        if (m2 <= MAX_MANTISSA / 10) { m2 *= 10; ++s2; }
        else { uint32_t rem = uint32_t(m1 % 10); m1 /= 10; if (rem >= 5) ++m1; --s1; }
    }
}

std::string Decimal::u128str(u128 v) {
    if (v == 0) return "0";
    std::string s;
    while (v > 0) { s = char('0' + int(v % 10)) + s; v /= 10; }
    return s;
}

double Decimal::u128tod(u128 v) {
    static const double pow2_64 = 18446744073709551616.0;
    return double(uint64_t(v >> 64)) * pow2_64 + double(uint64_t(v));
}

// ---------------------------------------------------------------------------
// Private instance helpers
// ---------------------------------------------------------------------------

void Decimal::fitMantissa() {
    while (mantissa_ > MAX_MANTISSA) {
        uint32_t rem = uint32_t(mantissa_ % 10);
        mantissa_ /= 10;
        if (rem >= 5) ++mantissa_;
        if (scale_ == 0) throw System::OverflowException("Decimal overflow.");
        --scale_;
    }
}

void Decimal::normalize() {
    while (scale_ > 0 && mantissa_ % 10 == 0) { mantissa_ /= 10; --scale_; }
    if (mantissa_ == 0) { scale_ = 0; negative_ = false; }
}

// ---------------------------------------------------------------------------
// Private constructor
// ---------------------------------------------------------------------------

Decimal::Decimal(u128 m, uint8_t s, bool n)
    : mantissa_(m), scale_(s), negative_(n) {}

// ---------------------------------------------------------------------------
// Public constructors
// ---------------------------------------------------------------------------

Decimal::Decimal(intcs v)
    : mantissa_(v < 0 ? -(u128)v : u128(v)), scale_(0), negative_(v < 0) {}

Decimal::Decimal(long long v)
    : mantissa_(v < 0 ? -(u128)v : u128(v)), scale_(0), negative_(v < 0) {}

Decimal::Decimal(long v)
    : mantissa_(v < 0 ? -(u128)v : u128(v)), scale_(0), negative_(v < 0) {}

Decimal::Decimal(double v) {
    if (std::isnan(v))  throw System::OverflowException("Cannot convert NaN to Decimal.");
    if (std::isinf(v))  throw System::OverflowException("Cannot convert Infinity to Decimal.");
    negative_ = std::signbit(v);
    v = std::abs(v);
    scale_ = 0;
    while (v != std::floor(v) && scale_ < 15) { v *= 10.0; ++scale_; }
    if (v > u128tod(MAX_MANTISSA)) throw System::OverflowException("Value too large for Decimal.");
    // v can legitimately reach ~7.9e28 here (Decimal's own mantissa range), far beyond what
    // std::llround's `long long` return type can hold (~9.2e18) -- for any v roughly between
    // 9.2e18 and 7.9e28 (e.g. Decimal(1e20), a perfectly in-range, exactly-representable double)
    // std::llround(v) silently overflowed to LLONG_MIN, producing a garbage mantissa instead of
    // the correct value. Split v into two 64-bit limbs instead, each within double's/uint64_t's
    // exactly-representable range, avoiding `long long` entirely.
    static const double pow2_64 = 18446744073709551616.0;
    double hi = std::floor(v / pow2_64);
    double lo = std::round(v - hi * pow2_64);
    if (lo >= pow2_64) { lo -= pow2_64; hi += 1.0; }
    mantissa_ = (u128(static_cast<uint64_t>(hi)) << 64) | static_cast<uint64_t>(lo);
    fitMantissa();
    normalize();
}

Decimal::Decimal(float v) : Decimal(double(v)) {}

Decimal::Decimal(uintcs v)
    : mantissa_(v), scale_(0), negative_(false) {}

Decimal::Decimal(ulongcs v)
    : mantissa_(v), scale_(0), negative_(false) {}

Decimal::Decimal(intcs lo, intcs mid, intcs hi, bool isNegative, bytecs scale) {
    if (scale > 28)
        throw System::ArgumentOutOfRangeException("scale", "Decimal scale must be in range 0-28.");
    mantissa_ = (u128(uint32_t(hi)) << 64) | (u128(uint32_t(mid)) << 32) | u128(uint32_t(lo));
    scale_    = scale;
    negative_ = isNegative && mantissa_ != 0;
}

// ---------------------------------------------------------------------------
// Static constants
// ---------------------------------------------------------------------------

const Decimal Decimal::Zero     = Decimal();
const Decimal Decimal::One      = Decimal(1);
const Decimal Decimal::MinusOne = Decimal(-1);
const Decimal Decimal::MaxValue = Decimal::Parse("79228162514264337593543950335");
const Decimal Decimal::MinValue = Decimal::Parse("-79228162514264337593543950335");

// ---------------------------------------------------------------------------
// Conversions
// ---------------------------------------------------------------------------

double Decimal::ToDouble() const {
    double r = u128tod(mantissa_) / kPow10[scale_];
    return negative_ ? -r : r;
}

float     Decimal::ToSingle() const { return float(ToDouble()); }

intcs Decimal::ToInt32() const {
    Decimal t = Truncate(*this);
    if (t.mantissa_ > u128(UINT32_MAX))
        throw System::OverflowException("Value was either too large or too small for an Int32.");
    uint32_t mag = uint32_t(t.mantissa_);
    int64_t v = t.negative_ ? -int64_t(mag) : int64_t(mag);
    if (v < INT32_MIN || v > INT32_MAX)
        throw System::OverflowException("Value was either too large or too small for an Int32.");
    return intcs(v);
}

longcs Decimal::ToInt64() const {
    Decimal t = Truncate(*this);
    if (t.mantissa_ > u128(UINT64_MAX))
        throw System::OverflowException("Value was either too large or too small for an Int64.");
    uint64_t mag = uint64_t(t.mantissa_);
    if (t.negative_) {
        if (mag > (uint64_t(1) << 63))
            throw System::OverflowException("Value was either too large or too small for an Int64.");
        return mag == (uint64_t(1) << 63) ? INT64_MIN : -static_cast<longcs>(mag);
    }
    if (mag > uint64_t(INT64_MAX))
        throw System::OverflowException("Value was either too large or too small for an Int64.");
    return static_cast<longcs>(mag);
}

uintcs Decimal::ToUInt32() const {
    Decimal t = Truncate(*this);
    if (t.mantissa_ > u128(UINT32_MAX))
        throw System::OverflowException("Value was either too large or too small for a UInt32.");
    uint32_t mag = uint32_t(t.mantissa_);
    if (t.negative_ && mag != 0)
        throw System::OverflowException("Value was either too large or too small for a UInt32.");
    return mag;
}

ulongcs Decimal::ToUInt64() const {
    Decimal t = Truncate(*this);
    if (t.mantissa_ > u128(UINT64_MAX))
        throw System::OverflowException("Value was either too large or too small for a UInt64.");
    uint64_t mag = uint64_t(t.mantissa_);
    if (t.negative_ && mag != 0)
        throw System::OverflowException("Value was either too large or too small for a UInt64.");
    return mag;
}

bytecs Decimal::ToByte(const Decimal& value) {
    uintcs temp;
    try {
        temp = value.ToUInt32();
    } catch (const System::OverflowException&) {
        throw System::OverflowException("Value was either too large or too small for an unsigned byte.");
    }
    if (temp != bytecs(temp))
        throw System::OverflowException("Value was either too large or too small for an unsigned byte.");
    return bytecs(temp);
}

sbytecs Decimal::ToSByte(const Decimal& value) {
    intcs temp;
    try {
        temp = value.ToInt32();
    } catch (const System::OverflowException&) {
        throw System::OverflowException("Value was either too large or too small for a signed byte.");
    }
    if (temp != sbytecs(temp))
        throw System::OverflowException("Value was either too large or too small for a signed byte.");
    return sbytecs(temp);
}

shortcs Decimal::ToInt16(const Decimal& value) {
    intcs temp;
    try {
        temp = value.ToInt32();
    } catch (const System::OverflowException&) {
        throw System::OverflowException("Value was either too large or too small for an Int16.");
    }
    if (temp != shortcs(temp))
        throw System::OverflowException("Value was either too large or too small for an Int16.");
    return shortcs(temp);
}

ushortcs Decimal::ToUInt16(const Decimal& value) {
    uintcs temp;
    try {
        temp = value.ToUInt32();
    } catch (const System::OverflowException&) {
        throw System::OverflowException("Value was either too large or too small for a UInt16.");
    }
    if (temp != ushortcs(temp))
        throw System::OverflowException("Value was either too large or too small for a UInt16.");
    return ushortcs(temp);
}

void Decimal::GetBits(const Decimal& d, intcs& lo, intcs& mid, intcs& hi, intcs& flags) {
    lo  = intcs(uint32_t(d.mantissa_));
    mid = intcs(uint32_t(d.mantissa_ >> 32));
    hi  = intcs(uint32_t(d.mantissa_ >> 64));
    flags = intcs((uint32_t(d.scale_) << 16) | (d.negative_ ? 0x80000000u : 0u));
}

// ---------------------------------------------------------------------------
// Parse / ToString
// ---------------------------------------------------------------------------

bool Decimal::TryParse(const std::string& s, Decimal& result) {
    if (s.empty()) return false;
    size_t i = 0;
    bool neg = false;
    if (s[i] == '-') { neg = true; ++i; }
    else if (s[i] == '+') { ++i; }

    u128    mantissa = 0;
    uint8_t scale    = 0;
    bool    seenDot  = false, seenDigit = false;

    for (; i < s.size(); ++i) {
        char c = s[i];
        if (c == '.' || c == ',') {
            if (seenDot) return false;
            seenDot = true;
        } else if (c >= '0' && c <= '9') {
            seenDigit = true;
            if (seenDot) {
                if (scale >= 28) continue; // extra precision silently dropped
                ++scale;
            }
            if (mantissa > MAX_MANTISSA / 10) return false;
            mantissa = mantissa * 10 + uint8_t(c - '0');
            if (mantissa > MAX_MANTISSA) return false;
        } else { return false; }
    }
    if (!seenDigit) return false;
    result = Decimal(mantissa, scale, neg && mantissa != 0);
    return true;
}

Decimal Decimal::Parse(const std::string& s) {
    Decimal r;
    if (!TryParse(s, r))
        throw System::FormatException("Input string was not in a correct format.");
    return r;
}

std::string Decimal::ToString() const {
    std::string m = u128str(mantissa_);
    if (scale_ == 0)
        return (negative_ && mantissa_ != 0 ? "-" : "") + m;
    while (int(m.size()) <= int(scale_)) m = "0" + m;
    int dot = int(m.size()) - int(scale_);
    std::string r = m.substr(0, dot) + "." + m.substr(dot);
    if (negative_ && mantissa_ != 0) r = "-" + r;
    return r;
}

// ---------------------------------------------------------------------------
// Arithmetic
// ---------------------------------------------------------------------------

Decimal Decimal::operator+(const Decimal& o) const {
    u128 m1 = mantissa_, m2 = o.mantissa_;
    uint8_t s1 = scale_, s2 = o.scale_;
    alignScales(m1, s1, m2, s2);
    Decimal r;
    r.scale_ = s1;
    if (negative_ == o.negative_) {
        r.mantissa_ = m1 + m2; r.negative_ = negative_; r.fitMantissa();
    } else if (m1 >= m2) {
        r.mantissa_ = m1 - m2; r.negative_ = negative_;
    } else {
        r.mantissa_ = m2 - m1; r.negative_ = o.negative_;
    }
    if (r.mantissa_ == 0) r.negative_ = false;
    return r;
}

Decimal Decimal::operator-(const Decimal& o) const {
    if (o.mantissa_ == 0) return *this;
    Decimal neg_o(o.mantissa_, o.scale_, !o.negative_);
    return *this + neg_o;
}

Decimal Decimal::operator*(const Decimal& o) const {
    if (mantissa_ == 0 || o.mantissa_ == 0) return Decimal();
    uint192 prod = mul96x96(mantissa_, o.mantissa_);
    int scale = int(scale_) + int(o.scale_);
    while (!fits96(prod) || scale > 28) {
        if (scale == 0) throw System::OverflowException("Decimal overflow.");
        uint32_t rem = div192by10(prod); --scale;
        if (rem >= 5) {
            if (++prod.lo == 0) if (++prod.mid == 0) ++prod.hi;
        }
    }
    return Decimal(to128(prod), uint8_t(scale), negative_ ^ o.negative_);
}

Decimal Decimal::operator/(const Decimal& o) const {
    if (o.mantissa_ == 0) throw System::DivideByZeroException("Attempted to divide by zero.");
    if (mantissa_ == 0) return Decimal();
    u128 dividend = mantissa_;
    int scale = int(scale_) - int(o.scale_);
    while (scale < 28 && dividend <= MAX_MANTISSA / 10) { dividend *= 10; ++scale; }
    u128 q = dividend / o.mantissa_;
    u128 rem = dividend % o.mantissa_;
    if (rem * 2 >= o.mantissa_) ++q;
    if (scale < 0) { while (scale < 0) { q /= 10; ++scale; } }
    Decimal r(q, uint8_t(std::min(scale, 28)), negative_ ^ o.negative_);
    r.fitMantissa();
    return r;
}

Decimal Decimal::operator%(const Decimal& o) const {
    return *this - Truncate(*this / o) * o;
}

Decimal Decimal::operator-() const {
    if (mantissa_ == 0) return *this;
    return Decimal(mantissa_, scale_, !negative_);
}

Decimal& Decimal::operator+=(const Decimal& o) { *this = *this + o; return *this; }
Decimal& Decimal::operator-=(const Decimal& o) { *this = *this - o; return *this; }
Decimal& Decimal::operator*=(const Decimal& o) { *this = *this * o; return *this; }
Decimal& Decimal::operator/=(const Decimal& o) { *this = *this / o; return *this; }

// ---------------------------------------------------------------------------
// Comparison
// ---------------------------------------------------------------------------

bool Decimal::operator==(const Decimal& o) const {
    if (negative_ != o.negative_)
        return mantissa_ == 0 && o.mantissa_ == 0;
    u128 m1 = mantissa_, m2 = o.mantissa_;
    uint8_t s1 = scale_, s2 = o.scale_;
    alignScales(m1, s1, m2, s2);
    return m1 == m2;
}

bool Decimal::operator!=(const Decimal& o) const { return !(*this == o); }

bool Decimal::operator<(const Decimal& o) const {
    if (negative_ != o.negative_) {
        if (mantissa_ == 0 && o.mantissa_ == 0) return false;
        return negative_;
    }
    u128 m1 = mantissa_, m2 = o.mantissa_;
    uint8_t s1 = scale_, s2 = o.scale_;
    alignScales(m1, s1, m2, s2);
    return negative_ ? m1 > m2 : m1 < m2;
}

bool Decimal::operator<=(const Decimal& o) const { return !(o < *this); }
bool Decimal::operator> (const Decimal& o) const { return  (o < *this); }
bool Decimal::operator>=(const Decimal& o) const { return !(*this < o); }

// ---------------------------------------------------------------------------
// Math methods
// ---------------------------------------------------------------------------

Decimal Decimal::Abs(const Decimal& d) {
    return Decimal(d.mantissa_, d.scale_, false);
}

Decimal Decimal::Truncate(const Decimal& d) {
    if (d.scale_ == 0) return d;
    u128 m = d.mantissa_; uint8_t s = d.scale_;
    while (s > 0) { m /= 10; --s; }
    return Decimal(m, 0, d.negative_ && m != 0);
}

Decimal Decimal::Floor(const Decimal& d) {
    Decimal t = Truncate(d);
    if (d.negative_ && d != t) t = t - Decimal(1);
    return t;
}

Decimal Decimal::Ceiling(const Decimal& d) {
    Decimal t = Truncate(d);
    if (!d.negative_ && d != t) t = t + Decimal(1);
    return t;
}

Decimal Decimal::Round(const Decimal& d, intcs decimals, MidpointRounding mode) {
    if (decimals < 0 || decimals > 28)
        throw System::ArgumentOutOfRangeException("decimals", "decimals must be in range 0-28.");
    if (d.scale_ <= uint8_t(decimals)) return d;

    int dropCount = int(d.scale_) - decimals;
    u128 divisor = 1;
    for (int i = 0; i < dropCount; ++i) divisor *= 10;

    u128 m = d.mantissa_;
    u128 quotient  = m / divisor;
    u128 remainder = m % divisor;
    u128 halfDivisor = divisor / 2;

    bool roundUp;
    switch (mode) {
        case MidpointRounding::ToEven:
            if (remainder > halfDivisor) roundUp = true;
            else if (remainder < halfDivisor) roundUp = false;
            else roundUp = (quotient % 2) != 0;
            break;
        case MidpointRounding::AwayFromZero:
            roundUp = remainder >= halfDivisor;
            break;
        case MidpointRounding::ToZero:
            roundUp = false;
            break;
        case MidpointRounding::ToNegativeInfinity:
            roundUp = d.negative_ && remainder != 0;
            break;
        case MidpointRounding::ToPositiveInfinity:
            roundUp = !d.negative_ && remainder != 0;
            break;
        default:
            roundUp = false;
            break;
    }
    if (roundUp) ++quotient;

    return Decimal(quotient, uint8_t(decimals), d.negative_ && quotient != 0);
}

} // namespace System

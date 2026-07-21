// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Numerics/BigInteger.hpp"
#include "System/DivideByZeroException.hpp"
#include "System/FormatException.hpp"
#include <algorithm>

namespace System::Numerics {

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

void BigInteger::trim() {
    while (mag_.size() > 1 && mag_.back() == 0) mag_.pop_back();
    if (mag_.size() == 1 && mag_[0] == 0) negative_ = false;
}

int BigInteger::cmpMag(const std::vector<uint32_t>& a, const std::vector<uint32_t>& b) {
    if (a.size() != b.size()) return a.size() < b.size() ? -1 : 1;
    for (int i = static_cast<int>(a.size()) - 1; i >= 0; --i)
        if (a[i] != b[i]) return a[i] < b[i] ? -1 : 1;
    return 0;
}

std::vector<uint32_t> BigInteger::addMag(const std::vector<uint32_t>& a,
                                          const std::vector<uint32_t>& b) {
    std::vector<uint32_t> r;
    uint64_t carry = 0;
    for (size_t i = 0; i < std::max(a.size(), b.size()) || carry; ++i) {
        uint64_t sum = carry;
        if (i < a.size()) sum += a[i];
        if (i < b.size()) sum += b[i];
        r.push_back(static_cast<uint32_t>(sum % BASE));
        carry = sum / BASE;
    }
    return r;
}

std::vector<uint32_t> BigInteger::subMag(const std::vector<uint32_t>& a,
                                          const std::vector<uint32_t>& b) {
    // Precondition: |a| >= |b|
    std::vector<uint32_t> r;
    int64_t borrow = 0;
    for (size_t i = 0; i < a.size(); ++i) {
        int64_t d = static_cast<int64_t>(a[i]) - borrow
                    - (i < b.size() ? b[i] : 0);
        if (d < 0) { d += BASE; borrow = 1; } else { borrow = 0; }
        r.push_back(static_cast<uint32_t>(d));
    }
    while (r.size() > 1 && r.back() == 0) r.pop_back();
    return r;
}

std::vector<uint32_t> BigInteger::mulMag(const std::vector<uint32_t>& a,
                                          const std::vector<uint32_t>& b) {
    std::vector<uint32_t> r(a.size() + b.size(), 0);
    for (size_t i = 0; i < a.size(); ++i) {
        uint64_t carry = 0;
        for (size_t j = 0; j < b.size() || carry; ++j) {
            uint64_t cur = static_cast<uint64_t>(r[i + j]) + carry;
            if (j < b.size()) cur += static_cast<uint64_t>(a[i]) * b[j];
            r[i + j] = static_cast<uint32_t>(cur % BASE);
            carry    = cur / BASE;
        }
    }
    while (r.size() > 1 && r.back() == 0) r.pop_back();
    return r;
}

// Single-limb divisor — simple long division in base BASE.
std::vector<uint32_t> BigInteger::divMagByLimb(const std::vector<uint32_t>& a,
                                                uint32_t b, uint32_t& rem) {
    std::vector<uint32_t> q(a.size());
    uint64_t r = 0;
    for (int i = static_cast<int>(a.size()) - 1; i >= 0; --i) {
        uint64_t cur = r * BASE + a[i];
        q[i] = static_cast<uint32_t>(cur / b);
        r    = cur % b;
    }
    rem = static_cast<uint32_t>(r);
    while (q.size() > 1 && q.back() == 0) q.pop_back();
    return q;
}

// Multi-limb division: Knuth Algorithm D (base BASE = 10^9).
// Returns {quotient magnitude, remainder magnitude}, both least-significant first.
std::pair<std::vector<uint32_t>, std::vector<uint32_t>>
BigInteger::divmodMag(std::vector<uint32_t> u, std::vector<uint32_t> v) {
    int n = static_cast<int>(v.size());
    int m = static_cast<int>(u.size()) - n; // quotient has m+1 digits

    // Step D1: normalise — multiply both by d = BASE / (v[n-1] + 1)
    uint64_t d = static_cast<uint64_t>(BASE) / (static_cast<uint64_t>(v[n-1]) + 1);

    // Extend u by one leading zero limb after normalisation
    u.push_back(0);
    if (d > 1) {
        uint64_t carry = 0;
        for (auto& x : u)  { uint64_t p = static_cast<uint64_t>(x)*d + carry; x = static_cast<uint32_t>(p % BASE); carry = p / BASE; }
        carry = 0;
        for (auto& x : v)  { uint64_t p = static_cast<uint64_t>(x)*d + carry; x = static_cast<uint32_t>(p % BASE); carry = p / BASE; }
    }

    std::vector<uint32_t> q(static_cast<size_t>(m + 1), 0);

    for (int j = m; j >= 0; --j) {
        // Step D3: estimate quotient digit
        uint64_t top2 = static_cast<uint64_t>(u[j + n]) * BASE
                      + static_cast<uint64_t>(u[j + n - 1]);
        uint64_t vn1  = static_cast<uint64_t>(v[n - 1]);
        uint64_t qhat = top2 / vn1;
        uint64_t rhat = top2 % vn1;

        // Step D3 refinement: adjust qhat down at most twice
        uint64_t vn2 = (n >= 2) ? static_cast<uint64_t>(v[n - 2]) : 0;
        while (qhat >= static_cast<uint64_t>(BASE) ||
               (n >= 2 && qhat * vn2 > rhat * static_cast<uint64_t>(BASE)
                                      + static_cast<uint64_t>(u[j + n - 2]))) {
            --qhat;
            rhat += vn1;
            if (rhat >= static_cast<uint64_t>(BASE)) break;
        }

        // Step D4: u[j..j+n] -= qhat * v
        int64_t  borrow = 0;
        uint64_t carry  = 0;
        for (int i = 0; i <= n; ++i) {
            uint64_t prod = (i < n) ? qhat * static_cast<uint64_t>(v[i]) + carry : carry;
            carry = prod / BASE;
            int64_t diff = static_cast<int64_t>(u[j + i])
                         - static_cast<int64_t>(prod % BASE) - borrow;
            if (diff < 0) { diff += static_cast<int64_t>(BASE); borrow = 1; }
            else            borrow = 0;
            u[j + i] = static_cast<uint32_t>(diff);
        }

        // Step D5: if underflow, add back
        if (borrow) {
            --qhat;
            uint64_t c = 0;
            for (int i = 0; i <= n; ++i) {
                uint64_t s = static_cast<uint64_t>(u[j + i])
                           + (i < n ? static_cast<uint64_t>(v[i]) : 0) + c;
                u[j + i] = static_cast<uint32_t>(s % BASE);
                c = s / BASE;
            }
        }

        q[j] = static_cast<uint32_t>(qhat);
    }

    while (q.size() > 1 && q.back() == 0) q.pop_back();

    // Step D8: unnormalise remainder
    std::vector<uint32_t> rem(u.begin(), u.begin() + n);
    if (d > 1) {
        uint64_t r = 0;
        for (int i = n - 1; i >= 0; --i) {
            uint64_t cur = r * BASE + rem[i];
            rem[i] = static_cast<uint32_t>(cur / d);
            r      = cur % d;
        }
    }
    while (rem.size() > 1 && rem.back() == 0) rem.pop_back();

    return {q, rem};
}

std::vector<uint32_t> BigInteger::fromUInt64(uint64_t v) {
    std::vector<uint32_t> r;
    if (v == 0) { r.push_back(0); return r; }
    while (v) { r.push_back(static_cast<uint32_t>(v % BASE)); v /= BASE; }
    return r;
}

// ---------------------------------------------------------------------------
// Constructors
// ---------------------------------------------------------------------------

BigInteger::BigInteger() : mag_({0}) {}

BigInteger::BigInteger(intcs v) {
    negative_ = v < 0;
    mag_ = fromUInt64(negative_ ? static_cast<uint64_t>(-(int64_t)v)
                                : static_cast<uint64_t>(v));
}

BigInteger::BigInteger(longcs v) {
    negative_ = v < 0;
    // Negating v directly (`-v`) is undefined behavior in C++ for v == LONGCS_MIN (confirmed
    // via UBSan: "negation of -9223372036854775808 cannot be represented in type 'long int'")
    // -- unlike the intcs constructor above, which safely widens to int64_t before negating
    // (int64_t can represent -INT32_MIN exactly), there is no wider standard integer type to
    // widen into here. Compute the magnitude via well-defined unsigned subtraction instead:
    // `0 - (uint64_t)v` reinterprets v's bit pattern as unsigned first (well-defined in C++20)
    // and performs the two's-complement negation via unsigned wraparound (also well-defined),
    // producing the identical, correct magnitude without ever negating a signed value.
    mag_ = fromUInt64(negative_ ? (0ULL - static_cast<uint64_t>(v))
                                : static_cast<uint64_t>(v));
}

// ---------------------------------------------------------------------------
// Properties
// ---------------------------------------------------------------------------

bool BigInteger::getIsZeroProperty()     const { return mag_.size() == 1 && mag_[0] == 0; }
bool BigInteger::getIsOneProperty()      const { return !negative_ && mag_.size() == 1 && mag_[0] == 1; }
bool BigInteger::getIsNegativeProperty() const { return negative_; }
int  BigInteger::Sign()                  const { return getIsZeroProperty() ? 0 : (negative_ ? -1 : 1); }

// ---------------------------------------------------------------------------
// Parse / ToString
// ---------------------------------------------------------------------------

BigInteger BigInteger::Parse(const std::string& s) {
    if (s.empty()) throw System::FormatException("The value could not be parsed.");
    BigInteger r;
    bool neg = (s[0] == '-');
    size_t start = (neg || s[0] == '+') ? 1 : 0;
    if (start >= s.size()) throw System::FormatException("The value could not be parsed.");
    for (size_t k = start; k < s.size(); ++k)
        if (s[k] < '0' || s[k] > '9')
            throw System::FormatException("The value could not be parsed.");

    std::vector<uint32_t> m;
    int i = static_cast<int>(s.size());
    while (i > static_cast<int>(start)) {
        int from = std::max(static_cast<int>(start), i - 9);
        m.push_back(static_cast<uint32_t>(std::stoul(s.substr(from, i - from))));
        i = from;
    }
    if (m.empty()) m.push_back(0);
    r.mag_      = m;
    r.negative_ = neg;
    r.trim();
    return r;
}

bool BigInteger::TryParse(const std::string& s, BigInteger& result) {
    try { result = Parse(s); return true; }
    catch (...) { result = BigInteger(0); return false; }
}

std::string BigInteger::ToString() const {
    if (getIsZeroProperty()) return "0";
    std::string s;
    for (int i = static_cast<int>(mag_.size()) - 1; i >= 0; --i) {
        if (s.empty()) {
            s += std::to_string(mag_[i]);
        } else {
            std::string chunk = std::to_string(mag_[i]);
            s += std::string(9 - chunk.size(), '0') + chunk;
        }
    }
    return negative_ ? "-" + s : s;
}

// ---------------------------------------------------------------------------
// Arithmetic
// ---------------------------------------------------------------------------

BigInteger BigInteger::operator-() const {
    BigInteger r(*this);
    if (!getIsZeroProperty()) r.negative_ = !negative_;
    return r;
}

BigInteger BigInteger::Abs() const {
    BigInteger r(*this); r.negative_ = false; return r;
}

BigInteger BigInteger::operator+(const BigInteger& o) const {
    if (negative_ == o.negative_) {
        BigInteger r; r.negative_ = negative_; r.mag_ = addMag(mag_, o.mag_); r.trim();
        return r;
    }
    int c = cmpMag(mag_, o.mag_);
    if (c == 0) return BigInteger(0);
    BigInteger r;
    if (c > 0) { r.negative_ = negative_;   r.mag_ = subMag(mag_, o.mag_); }
    else       { r.negative_ = o.negative_; r.mag_ = subMag(o.mag_, mag_); }
    r.trim();
    return r;
}

BigInteger BigInteger::operator-(const BigInteger& o) const { return *this + (-o); }

BigInteger BigInteger::operator*(const BigInteger& o) const {
    BigInteger r;
    r.negative_ = negative_ != o.negative_;
    r.mag_ = mulMag(mag_, o.mag_);
    r.trim();
    return r;
}

BigInteger BigInteger::operator/(const BigInteger& o) const {
    if (o.getIsZeroProperty()) throw System::DivideByZeroException("Attempted to divide by zero.");
    if (getIsZeroProperty())   return BigInteger(0);
    int c = cmpMag(mag_, o.mag_);
    if (c < 0) return BigInteger(0);
    if (c == 0) { BigInteger r(1); r.negative_ = negative_ != o.negative_; return r; }

    std::vector<uint32_t> q;
    if (o.mag_.size() == 1) {
        uint32_t rem;
        q = divMagByLimb(mag_, o.mag_[0], rem);
    } else {
        q = divmodMag(mag_, o.mag_).first;
    }
    BigInteger r;
    r.mag_      = q;
    r.negative_ = negative_ != o.negative_;
    r.trim();
    return r;
}

BigInteger BigInteger::operator%(const BigInteger& o) const {
    if (o.getIsZeroProperty()) throw System::DivideByZeroException("Attempted to divide by zero.");
    if (getIsZeroProperty())   return BigInteger(0);
    int c = cmpMag(mag_, o.mag_);
    if (c < 0) return *this;  // |dividend| < |divisor| → remainder = dividend
    if (c == 0) return BigInteger(0);

    std::vector<uint32_t> rem;
    if (o.mag_.size() == 1) {
        uint32_t r;
        divMagByLimb(mag_, o.mag_[0], r);
        rem = {r};
    } else {
        rem = divmodMag(mag_, o.mag_).second;
    }
    BigInteger r;
    r.mag_      = rem;
    r.negative_ = negative_; // remainder has the sign of the dividend (.NET semantics)
    r.trim();
    return r;
}

BigInteger& BigInteger::operator+=(const BigInteger& o) { *this = *this + o; return *this; }
BigInteger& BigInteger::operator-=(const BigInteger& o) { *this = *this - o; return *this; }
BigInteger& BigInteger::operator*=(const BigInteger& o) { *this = *this * o; return *this; }
BigInteger& BigInteger::operator/=(const BigInteger& o) { *this = *this / o; return *this; }
BigInteger& BigInteger::operator%=(const BigInteger& o) { *this = *this % o; return *this; }

// ---------------------------------------------------------------------------
// Comparison
// ---------------------------------------------------------------------------

bool BigInteger::operator==(const BigInteger& o) const {
    return negative_ == o.negative_ && mag_ == o.mag_;
}
bool BigInteger::operator!=(const BigInteger& o) const { return !(*this == o); }
bool BigInteger::operator<(const BigInteger& o) const {
    if (negative_ != o.negative_) return negative_;
    int c = cmpMag(mag_, o.mag_);
    return negative_ ? c > 0 : c < 0;
}
bool BigInteger::operator<=(const BigInteger& o) const { return !(o < *this); }
bool BigInteger::operator> (const BigInteger& o) const { return   o < *this;  }
bool BigInteger::operator>=(const BigInteger& o) const { return !(*this < o); }

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------

const BigInteger BigInteger::Zero    {  0 };
const BigInteger BigInteger::One     {  1 };
const BigInteger BigInteger::MinusOne{ -1 };

} // namespace System::Numerics

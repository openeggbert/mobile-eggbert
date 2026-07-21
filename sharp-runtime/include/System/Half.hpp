// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <cstdint>
#include <cstring>
#include <string>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <limits>
#include <algorithm>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Single.hpp"
#include "System/Span.hpp"

namespace System {

    using SharpRuntime::intcs;
    using SharpRuntime::longcs;

    class IFormatProvider;

    /**
     * @brief Represents a half-precision (16-bit, IEEE 754 binary16) floating-point number.
     *
     * C++ counterpart of .NET System.Half.
     *
     * @note Status: Partial. Deviations from .NET:
     *   - The ~40 explicit/implicit conversion operators .NET defines to/from every other
     *     numeric primitive (byte, sbyte, short, ushort, uint, ulong, nint, nuint, char,
     *     decimal, Int128, UInt128, plus their `checked` variants — a C# 11 language
     *     feature with no C++ equivalent) are not ported; every one of them is defined in
     *     .NET as a trivial `(Half)(float)value` round-trip, so callers can do the same
     *     via ToSingle()/FromSingle() in one line. float and double are ported directly.
     *   - The ~40 static math functions mirroring `System.MathF` (Exp, Log, Sin, Cos, Sqrt,
     *     Pow, Round, Ceiling, Floor, Truncate, Lerp, FusedMultiplyAdd, BitIncrement/
     *     Decrement, ReciprocalEstimate, ...) are not ported for the same reason: widen via
     *     ToSingle(), call the existing System::Single/std::/MathF equivalent, narrow back
     *     via FromSingle(). Duplicating the entire float math surface as Half overloads
     *     adds no capability, only near-identical forwarding boilerplate.
     *   - Generic math interface conformance (INumber&lt;Half&gt;, IFloatingPointIeee754&lt;Half&gt;,
     *     IMinMaxValue&lt;Half&gt;, etc.) is out of scope, consistent with this codebase's
     *     position on C# generic-math machinery elsewhere.
     *   - FromSingle()/ToSingle() implement IEEE 754 round-to-nearest-even exactly as .NET's
     *     `(Half)float` / `(float)Half` operators do (verified against known bit patterns:
     *     PositiveOneBits, MaxValueBits, Epsilon, NaN, E/Pi/Tau). FromDouble() narrows via
     *     an intermediate float rounding rather than rounding directly from double to half,
     *     which can theoretically double-round differently right at an exact tie boundary —
     *     an extremely narrow, low-impact edge case.
     *   - NaN payload bits are not preserved bit-for-bit through FromSingle()/FromDouble();
     *     a canonical quiet NaN (sign preserved) is produced instead, matching all
     *     NaN-producing entry points' externally observable behavior (IsNaN, comparisons).
     */
    struct Half {
        /** @brief The raw IEEE 754 half-precision bit pattern. */
        uint16_t bits = 0;

        /** @brief Initializes a new Half with zero value. */
        Half() = default;
        /** @brief Initializes a new Half from a raw 16-bit bit pattern. */
        explicit Half(uint16_t rawBits) : bits(rawBits) {}

        /** @brief Converts a 32-bit float to the nearest representable Half (round to nearest, ties to even). */
        [[nodiscard]] static Half FromSingle(float value) noexcept {
            uint32_t f;
            std::memcpy(&f, &value, sizeof(f));
            const uint32_t sign16 = (f >> 16) & 0x8000u;
            const uint32_t absF = f & 0x7FFFFFFFu;

            if (absF > 0x7F800000u) {
                // NaN: canonical quiet NaN, sign preserved. See class-level deviation note.
                return Half(static_cast<uint16_t>(sign16 | 0x7E00u));
            }
            if (absF >= 0x7F800000u) {
                return Half(static_cast<uint16_t>(sign16 | 0x7C00u)); // Infinity
            }
            if (absF == 0u) {
                return Half(static_cast<uint16_t>(sign16)); // signed zero
            }
            if (absF < 0x00800000u) {
                // Float subnormal input: far smaller than Half's smallest subnormal -> zero.
                return Half(static_cast<uint16_t>(sign16));
            }

            const int32_t exp = static_cast<int32_t>((absF >> 23) & 0xFFu) - 127;
            const uint32_t significand = (absF & 0x7FFFFFu) | 0x800000u; // 24-bit, implicit bit set

            const int32_t targetExp = exp + 15;
            if (targetExp >= 31) {
                return Half(static_cast<uint16_t>(sign16 | 0x7C00u)); // definite overflow
            }

            int32_t dropBits;
            int32_t finalExpField;
            if (targetExp <= 0) {
                dropBits = 13 + (1 - targetExp);
                finalExpField = 0;
            } else {
                dropBits = 13;
                finalExpField = targetExp;
            }

            if (dropBits >= 25) {
                return Half(static_cast<uint16_t>(sign16)); // underflows to zero
            }

            const uint32_t dropMask = (1u << dropBits) - 1u;
            const uint32_t lowPart = significand & dropMask;
            const uint32_t halfway = 1u << (dropBits - 1);
            uint32_t rounded = significand >> dropBits;
            if (lowPart > halfway || (lowPart == halfway && (rounded & 1u) != 0u)) {
                rounded += 1u;
            }

            uint32_t mantissaField;
            if (finalExpField == 0) {
                if (rounded == 0x400u) { finalExpField = 1; rounded = 0u; }
                mantissaField = rounded;
            } else {
                if (rounded == 0x800u) { finalExpField += 1; rounded = 0x400u; }
                mantissaField = rounded & 0x3FFu;
            }

            if (finalExpField >= 31) {
                return Half(static_cast<uint16_t>(sign16 | 0x7C00u)); // rounding overflow
            }

            return Half(static_cast<uint16_t>(sign16 | (static_cast<uint32_t>(finalExpField) << 10) | mantissaField));
        }

        /** @brief Converts this Half to its exactly-equal 32-bit float value (widening is always exact). */
        [[nodiscard]] float ToSingle() const noexcept {
            const uint32_t sign = (static_cast<uint32_t>(bits) << 16) & 0x80000000u;
            const uint32_t exp  = (bits >> 10) & 0x1Fu;
            const uint32_t mant = bits & 0x3FFu;
            uint32_t f;
            if (exp == 0x1Fu) {
                f = sign | 0x7F800000u | (mant << 13); // Inf / NaN
            } else if (exp == 0u) {
                if (mant == 0u) {
                    f = sign; // signed zero
                } else {
                    // Subnormal half -> renormalize into a normal float.
                    uint32_t m = mant;
                    int shift = 0;
                    while ((m & 0x400u) == 0u) { m <<= 1; ++shift; }
                    m &= 0x3FFu;
                    const int unbiasedExp = -14 - shift;
                    f = sign | (static_cast<uint32_t>(unbiasedExp + 127) << 23) | (m << 13);
                }
            } else {
                f = sign | ((exp + 112u) << 23) | (mant << 13); // normal: rebias 15 -> 127
            }
            float result;
            std::memcpy(&result, &f, sizeof(f));
            return result;
        }

        /** @brief Converts a double to the nearest representable Half. */
        [[nodiscard]] static Half FromDouble(double value) noexcept {
            return FromSingle(static_cast<float>(value));
        }

        /** @brief Converts this Half to its exactly-equal double value. */
        [[nodiscard]] double ToDouble() const noexcept { return static_cast<double>(ToSingle()); }

        /** @brief Explicit conversion to a 32-bit float. */
        explicit operator float() const noexcept { return ToSingle(); }
        /** @brief Explicit conversion to a 64-bit double. */
        explicit operator double() const noexcept { return ToDouble(); }

        /** @brief Represents the value zero. C++ counterpart of .NET Half.Zero. */
        static const Half Zero;
        /** @brief Represents the value one. C++ counterpart of .NET Half.One. */
        static const Half One;
        /** @brief Represents the value negative one. C++ counterpart of .NET Half.NegativeOne. */
        static const Half NegativeOne;
        /** @brief Represents negative zero. C++ counterpart of .NET Half.NegativeZero. */
        static const Half NegativeZero;
        /** @brief Represents Not a Number (NaN). C++ counterpart of .NET Half.NaN. */
        static const Half NaN;
        /** @brief Represents positive infinity. C++ counterpart of .NET Half.PositiveInfinity. */
        static const Half PositiveInfinity;
        /** @brief Represents negative infinity. C++ counterpart of .NET Half.NegativeInfinity. */
        static const Half NegativeInfinity;
        /** @brief Represents the largest finite half-precision value (65504). C++ counterpart of .NET Half.MaxValue. */
        static const Half MaxValue;
        /** @brief Represents the most negative finite half-precision value (-65504). C++ counterpart of .NET Half.MinValue. */
        static const Half MinValue;
        /** @brief Represents the smallest positive half-precision value (~5.96e-8). C++ counterpart of .NET Half.Epsilon. */
        static const Half Epsilon;
        /** @brief Represents the natural logarithmic base, e. C++ counterpart of .NET Half.E. */
        static const Half E;
        /** @brief Represents the ratio of a circle's circumference to its diameter, pi. C++ counterpart of .NET Half.Pi. */
        static const Half Pi;
        /** @brief Represents the number of radians in one turn, tau. C++ counterpart of .NET Half.Tau. */
        static const Half Tau;

        /**
         * @brief Returns true if the value is Not a Number (NaN).
         * C++ counterpart of .NET Half.IsNaN(Half).
         */
        [[nodiscard]] static bool IsNaN(Half h) noexcept {
            return (h.bits & 0x7FFF) > 0x7C00;
        }

        /**
         * @brief Returns true if the value is positive or negative infinity.
         * C++ counterpart of .NET Half.IsInfinity(Half).
         */
        [[nodiscard]] static bool IsInfinity(Half h) noexcept {
            return (h.bits & 0x7FFF) == 0x7C00;
        }

        /**
         * @brief Returns true if the value is positive infinity.
         * C++ counterpart of .NET Half.IsPositiveInfinity(Half).
         */
        [[nodiscard]] static bool IsPositiveInfinity(Half h) noexcept {
            return h.bits == 0x7C00;
        }

        /**
         * @brief Returns true if the value is negative infinity.
         * C++ counterpart of .NET Half.IsNegativeInfinity(Half).
         */
        [[nodiscard]] static bool IsNegativeInfinity(Half h) noexcept {
            return h.bits == 0xFC00;
        }

        /**
         * @brief Returns true if the value is finite (not NaN and not infinity).
         * C++ counterpart of .NET Half.IsFinite(Half).
         */
        [[nodiscard]] static bool IsFinite(Half h) noexcept {
            return (h.bits & 0x7C00) != 0x7C00;
        }

        /**
         * @brief Returns true if the value is negative.
         * C++ counterpart of .NET Half.IsNegative(Half).
         */
        [[nodiscard]] static bool IsNegative(Half h) noexcept {
            return (h.bits & 0x8000) != 0;
        }

        /**
         * @brief Returns true if the value is normal (finite, nonzero, and not subnormal).
         * C++ counterpart of .NET Half.IsNormal(Half).
         */
        [[nodiscard]] static bool IsNormal(Half h) noexcept {
            const uint16_t absBits = h.bits & 0x7FFF;
            return absBits >= 0x0400 && absBits < 0x7C00;
        }

        /**
         * @brief Returns true if the value is subnormal (finite, nonzero, and not normal).
         * C++ counterpart of .NET Half.IsSubnormal(Half).
         */
        [[nodiscard]] static bool IsSubnormal(Half h) noexcept {
            const uint16_t absBits = h.bits & 0x7FFF;
            return absBits >= 1 && absBits <= 0x03FF;
        }

        /**
         * @brief Parses a string into a Half. C++ counterpart of .NET Half.Parse(string).
         * @throws System::FormatException if the string is not a valid floating-point literal.
         */
        [[nodiscard]] static Half Parse(const std::string& s) { return FromSingle(Single::Parse(s)); }

        /** @brief Tries to parse a string into a Half without throwing. C++ counterpart of .NET Half.TryParse. */
        static bool TryParse(const std::string& s, Half& result) noexcept {
            float f;
            if (!Single::TryParse(s, f)) { result = Half(); return false; }
            result = FromSingle(f);
            return true;
        }

        /** @brief C++ counterpart of .NET Half.Parse(string, IFormatProvider). The provider is ignored. */
        [[nodiscard]] static Half Parse(const std::string& s, const IFormatProvider* provider) {
            (void)provider;
            return Parse(s);
        }

        /** @brief C++ counterpart of .NET Half.TryParse(string, IFormatProvider, out Half). The provider is ignored. */
        static bool TryParse(const std::string& s, const IFormatProvider* provider, Half& result) noexcept {
            (void)provider;
            return TryParse(s, result);
        }

        /**
         * @brief Tries to format this Half into the provided character span.
         * C++ counterpart of .NET Half.TryFormat(Span&lt;char&gt;, out int, ReadOnlySpan&lt;char&gt;, IFormatProvider).
         */
        bool TryFormat(Span<char> destination, intcs& charsWritten, const std::string& format = "") const {
            const std::string s = ToString(format);
            if (static_cast<intcs>(s.size()) > destination.getLengthProperty()) {
                charsWritten = 0;
                return false;
            }
            std::copy(s.begin(), s.end(), destination.getPointer());
            charsWritten = static_cast<intcs>(s.size());
            return true;
        }

        /**
         * @brief Returns a string representation of this Half value.
         * C++ counterpart of .NET Half.ToString(). Uses up to 5 significant digits, which
         * is always sufficient to round-trip any binary16 value (a documented simplification
         * of .NET's shortest-round-trip formatter; see class-level note).
         */
        [[nodiscard]] std::string ToString() const {
            if (IsNaN(*this)) return "NaN";
            if (bits == 0x7C00) return "Infinity";
            if (bits == 0xFC00) return "-Infinity";
            std::ostringstream oss;
            oss.imbue(std::locale::classic());
            oss << std::setprecision(5) << ToDouble();
            return oss.str();
        }

        /** @brief Returns a string representation using the specified numeric format ("F2", "E3", "G", "R"). */
        [[nodiscard]] std::string ToString(const std::string& format) const {
            if (format.empty()) return ToString();
            if (IsNaN(*this)) return "NaN";
            if (bits == 0x7C00) return "Infinity";
            if (bits == 0xFC00) return "-Infinity";
            return Single::ToString(ToSingle(), format);
        }

        /** @brief C++ counterpart of .NET Half.ToString(IFormatProvider). The provider is ignored. */
        [[nodiscard]] std::string ToString(const IFormatProvider* provider) const { (void)provider; return ToString(); }

        /** @brief C++ counterpart of .NET Half.ToString(string, IFormatProvider). The provider is ignored. */
        [[nodiscard]] std::string ToString(const std::string& format, const IFormatProvider* provider) const {
            (void)provider;
            return ToString(format);
        }

        /**
         * @brief Returns a hash code for this Half value.
         * C++ counterpart of .NET Half.GetHashCode(): all NaNs and both zeros hash the same.
         */
        [[nodiscard]] intcs GetHashCode() const noexcept {
            // Real Half.GetHashCode() does `bits &= PositiveInfinityBits` (an AND-mask, not an
            // assignment): for NaN this is a no-op in effect (the exponent field is already
            // all-ones, so it survives the 0x7C00 mask unchanged, giving 0x7C00), but for zero
            // (+0 or -0, exponent field 0) it masks everything away to 0 -- NOT 0x7C00.
            // Assigning b=0x7C00 unconditionally for both cases (the previous code here) still
            // satisfied the hash contract (equal values -- verified via Equals() above, which
            // does treat +0/-0 and all NaNs as mutually equal -- still hashed equally), but
            // didn't match .NET's actual hash value for zero.
            uint16_t b = bits;
            if (IsNaN(*this) || (bits & 0x7FFF) == 0) b &= 0x7C00;
            return static_cast<intcs>(b);
        }

        /**
         * @brief Determines whether this instance equals @p other.
         * C++ counterpart of .NET Half.Equals(Half): unlike operator==, NaN equals NaN here.
         */
        [[nodiscard]] bool Equals(const Half& other) const noexcept {
            return bits == other.bits
                || (((bits | other.bits) & 0x7FFF) == 0)
                || (IsNaN(*this) && IsNaN(other));
        }

        /**
         * @brief Compares this Half to another.
         * C++ counterpart of .NET Half.CompareTo(Half).
         * @return negative if less, 0 if equal, positive if greater; NaN sorts less than
         *         everything except another NaN (which it's equal to) — matching .NET's
         *         Half/Single/Double.CompareTo convention (NaN sorts first, ascending).
         */
        [[nodiscard]] intcs CompareTo(const Half& other) const noexcept {
            if (*this < other) return -1;
            if (*this > other) return 1;
            if (*this == other) return 0;
            if (IsNaN(*this)) return IsNaN(other) ? 0 : -1;
            return 1;
        }

        /** @brief Returns true if this Half is equal to @p o (IEEE 754 semantics: NaN != NaN, +0 == -0). */
        bool operator==(const Half& o) const noexcept { return ToSingle() == o.ToSingle(); }
        /** @brief Returns true if this Half is not equal to @p o. */
        bool operator!=(const Half& o) const noexcept { return !(*this == o); }
        /** @brief Returns true if this Half is less than @p o. */
        bool operator< (const Half& o) const noexcept { return ToSingle() <  o.ToSingle(); }
        /** @brief Returns true if this Half is less than or equal to @p o. */
        bool operator<=(const Half& o) const noexcept { return ToSingle() <= o.ToSingle(); }
        /** @brief Returns true if this Half is greater than @p o. */
        bool operator> (const Half& o) const noexcept { return ToSingle() >  o.ToSingle(); }
        /** @brief Returns true if this Half is greater than or equal to @p o. */
        bool operator>=(const Half& o) const noexcept { return ToSingle() >= o.ToSingle(); }

        /** @brief Adds two Half values. C++ counterpart of .NET Half.operator+(Half,Half). */
        friend Half operator+(const Half& l, const Half& r) noexcept { return FromSingle(l.ToSingle() + r.ToSingle()); }
        /** @brief Subtracts two Half values. C++ counterpart of .NET Half.operator-(Half,Half). */
        friend Half operator-(const Half& l, const Half& r) noexcept { return FromSingle(l.ToSingle() - r.ToSingle()); }
        /** @brief Multiplies two Half values. C++ counterpart of .NET Half.operator*(Half,Half). */
        friend Half operator*(const Half& l, const Half& r) noexcept { return FromSingle(l.ToSingle() * r.ToSingle()); }
        /** @brief Divides two Half values. C++ counterpart of .NET Half.operator/(Half,Half). */
        friend Half operator/(const Half& l, const Half& r) noexcept { return FromSingle(l.ToSingle() / r.ToSingle()); }
        /** @brief Computes the remainder of two Half values. C++ counterpart of .NET Half.operator%(Half,Half). */
        friend Half operator%(const Half& l, const Half& r) noexcept { return FromSingle(std::fmod(l.ToSingle(), r.ToSingle())); }

        /** @brief Unary plus. C++ counterpart of .NET Half.operator+(Half). */
        Half operator+() const noexcept { return *this; }
        /** @brief Unary negation. C++ counterpart of .NET Half.operator-(Half). */
        Half operator-() const noexcept { return Half(static_cast<uint16_t>(bits ^ 0x8000u)); }

        /** @brief Pre-increment by one. C++ counterpart of .NET Half.operator++(Half). */
        Half& operator++() noexcept { *this = FromSingle(ToSingle() + 1.0f); return *this; }
        /** @brief Post-increment by one. */
        Half operator++(int) noexcept { Half tmp = *this; ++(*this); return tmp; }
        /** @brief Pre-decrement by one. C++ counterpart of .NET Half.operator--(Half). */
        Half& operator--() noexcept { *this = FromSingle(ToSingle() - 1.0f); return *this; }
        /** @brief Post-decrement by one. */
        Half operator--(int) noexcept { Half tmp = *this; --(*this); return tmp; }
    };

    inline const Half Half::Zero             = Half(0x0000);
    inline const Half Half::One              = Half(0x3C00);
    inline const Half Half::NegativeOne      = Half(0xBC00);
    inline const Half Half::NegativeZero     = Half(0x8000);
    inline const Half Half::NaN              = Half(0xFE00);
    inline const Half Half::PositiveInfinity = Half(0x7C00);
    inline const Half Half::NegativeInfinity = Half(0xFC00);
    inline const Half Half::MaxValue         = Half(0x7BFF);  //  65504
    inline const Half Half::MinValue         = Half(0xFBFF);  // -65504
    inline const Half Half::Epsilon          = Half(0x0001);  //  ~5.96e-8
    inline const Half Half::E                = Half(0x4170);  //  ~2.71875
    inline const Half Half::Pi               = Half(0x4248);  //  ~3.140625
    inline const Half Half::Tau              = Half(0x4648);  //  ~6.28125

} // namespace System

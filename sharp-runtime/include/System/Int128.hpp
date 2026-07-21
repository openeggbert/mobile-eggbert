// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/DivideByZeroException.hpp"
#include "System/FormatException.hpp"
#include "System/OverflowException.hpp"

#if defined(_MSC_VER)
#  error "Int128 requires __int128 (GCC/Clang only). MSVC is not supported for this type."
#endif

namespace System {

    using SharpRuntime::intcs;

    /**
     * @brief Represents a 128-bit signed integer.
     *
     * C++ counterpart of .NET System.Int128.
     * Backed by the GCC/Clang @c __int128 extension; MSVC is not supported.
     */
    class Int128 {
        __int128 value_;

    public:
        /** @brief Initializes a new instance of Int128 with value 0. */
        constexpr Int128() noexcept : value_(0) {}

        /** @brief Initializes a new instance of Int128 from a raw __int128 value. */
        constexpr explicit Int128(__int128 v) noexcept : value_(v) {}

        /**
         * @brief Initializes a new instance of Int128.
         * @param upper The upper 64 bits of the 128-bit value.
         * @param lower The lower 64 bits of the 128-bit value.
         */
        constexpr Int128(uint64_t upper, uint64_t lower) noexcept
            : value_(static_cast<__int128>(
                (static_cast<unsigned __int128>(upper) << 64) | lower)) {}

        // ---------------------------------------------------------------
        // Static constants
        // ---------------------------------------------------------------

        /** @brief Represents the minimum value of Int128 (-2^127). */
        static constexpr Int128 MinValue() noexcept {
            return Int128(static_cast<__int128>(static_cast<unsigned __int128>(1) << 127));
        }

        /** @brief Represents the maximum value of Int128 (2^127 - 1). */
        static constexpr Int128 MaxValue() noexcept {
            return Int128(~(static_cast<__int128>(static_cast<unsigned __int128>(1) << 127)));
        }

        /** @brief Gets an Int128 whose value is 0. */
        static constexpr Int128 Zero() noexcept { return Int128(static_cast<__int128>(0)); }

        /** @brief Gets an Int128 whose value is 1. */
        static constexpr Int128 One()  noexcept { return Int128(static_cast<__int128>(1)); }

        /** @brief Gets an Int128 whose value is -1. C++ counterpart of .NET Int128.NegativeOne. */
        static constexpr Int128 NegativeOne() noexcept { return Int128(static_cast<__int128>(-1)); }

        // ---------------------------------------------------------------
        // Properties
        // ---------------------------------------------------------------

        /**
         * @brief Gets the lower 64 bits of the 128-bit value.
         *
         * C++ counterpart of .NET Int128 internal Lower property.
         */
        [[nodiscard]] uint64_t getLowerProperty() const noexcept {
            return static_cast<uint64_t>(value_);
        }

        /**
         * @brief Gets the upper 64 bits of the 128-bit value.
         *
         * C++ counterpart of .NET Int128 internal Upper property.
         */
        [[nodiscard]] uint64_t getUpperProperty() const noexcept {
            return static_cast<uint64_t>(static_cast<unsigned __int128>(value_) >> 64);
        }

        // ---------------------------------------------------------------
        // Equality and ordering
        // ---------------------------------------------------------------

        /**
         * @brief Determines whether the specified Int128 is equal to this instance.
         * @param other The Int128 to compare with the current instance.
         * @return true if @p other is equal to this instance; false otherwise.
         */
        [[nodiscard]] bool Equals(const Int128& other) const noexcept {
            return value_ == other.value_;
        }

        /**
         * @brief Returns the hash code for this instance.
         *
         * C++ counterpart of .NET Int128.GetHashCode().
         */
        [[nodiscard]] intcs GetHashCode() const noexcept {
            std::size_t seed = std::hash<uint64_t>{}(getLowerProperty());
            seed ^= std::hash<uint64_t>{}(getUpperProperty())
                  + 0x9e3779b9u + (seed << 6) + (seed >> 2);
            return static_cast<intcs>(seed);
        }

        /**
         * @brief Compares this instance to another Int128 value.
         * @param value The Int128 to compare with the current instance.
         * @return -1 if less than @p value; 0 if equal; 1 if greater than @p value.
         */
        [[nodiscard]] intcs CompareTo(const Int128& value) const noexcept {
            if (*this < value) return -1;
            if (*this > value) return  1;
            return 0;
        }

        // ---------------------------------------------------------------
        // String conversion
        // ---------------------------------------------------------------

        /**
         * @brief Converts the value of this Int128 to its equivalent string representation.
         *
         * C++ counterpart of .NET Int128.ToString().
         * @return A string containing the decimal representation of this value.
         */
        [[nodiscard]] std::string ToString() const {
            if (value_ == 0) return "0";
            bool neg = value_ < 0;
            unsigned __int128 v = neg ? static_cast<unsigned __int128>(-value_)
                                      : static_cast<unsigned __int128>(value_);
            std::string s;
            while (v > 0) { s += char('0' + int(v % 10)); v /= 10; }
            if (neg) s += '-';
            std::reverse(s.begin(), s.end());
            return s;
        }

        /**
         * @brief Converts the value of this Int128 to its equivalent string representation,
         * using the specified format.
         *
         * C++ counterpart of .NET Int128.ToString(string). Supports "X"/"x" (hexadecimal),
         * "D"/"d" (decimal), and "G"/"g" (general), each optionally followed by a minimum
         * field width (e.g. "X32"), zero-padded.
         * @param format The numeric format specifier.
         * @return A string containing the formatted representation of this value.
         */
        [[nodiscard]] std::string ToString(const std::string& format) const {
            if (format.empty()) return ToString();
            char type = format[0];
            int width = format.size() > 1 ? std::stoi(format.substr(1)) : 0;
            if (type == 'X' || type == 'x') {
                auto uv = static_cast<unsigned __int128>(value_);
                std::string s;
                if (uv == 0) {
                    s = "0";
                } else {
                    while (uv > 0) {
                        int digit = static_cast<int>(uv & 0xF);
                        s += static_cast<char>(digit < 10 ? '0' + digit : (type == 'X' ? 'A' : 'a') + (digit - 10));
                        uv >>= 4;
                    }
                    std::reverse(s.begin(), s.end());
                }
                while (static_cast<int>(s.size()) < width) s = "0" + s;
                return s;
            }
            if (type == 'D' || type == 'd') {
                std::string s = ToString();
                if (width > 0) {
                    bool neg = !s.empty() && s[0] == '-';
                    std::string digits = neg ? s.substr(1) : s;
                    while (static_cast<int>(digits.size()) < width) digits = "0" + digits;
                    return neg ? "-" + digits : digits;
                }
                return s;
            }
            return ToString();
        }

        /**
         * @brief Converts the string representation of a number to its Int128 equivalent.
         * @param s A string containing the number to convert.
         * @return An Int128 equivalent to @p s.
         * @throws System::FormatException if @p s is not a valid integer string.
         */
        static Int128 Parse(const std::string& s) {
            Int128 result;
            if (!TryParse(s, result))
                throw System::FormatException("Input string was not in a correct format: " + s);
            return result;
        }

        /**
         * @brief Tries to convert a string to its Int128 equivalent.
         * @param s      A string containing the number to convert.
         * @param result Set to the parsed value on success, or 0 on failure.
         * @return true if @p s was converted successfully; false otherwise.
         */
        static bool TryParse(const std::string& s, Int128& result) {
            result = Int128{};
            if (s.empty()) return false;
            std::size_t i = 0;
            bool neg = false;
            if (s[i] == '-') { neg = true; ++i; }
            else if (s[i] == '+') { ++i; }
            if (i == s.size()) return false;

            const unsigned __int128 absMax = neg
                ? (static_cast<unsigned __int128>(1) << 127)
                : ((static_cast<unsigned __int128>(1) << 127) - 1);

            unsigned __int128 val = 0;
            for (; i < s.size(); ++i) {
                if (s[i] < '0' || s[i] > '9') return false;
                unsigned int digit = static_cast<unsigned int>(s[i] - '0');
                if (val > (absMax - digit) / 10) return false;
                val = val * 10 + digit;
            }
            result = neg ? Int128(-static_cast<__int128>(val))
                        : Int128(static_cast<__int128>(val));
            return true;
        }

        // ---------------------------------------------------------------
        // Static math methods
        // ---------------------------------------------------------------

        /**
         * @brief Computes the absolute value of the specified Int128.
         *
         * C++ counterpart of .NET Int128.Abs(Int128) (INumberBase&lt;TSelf&gt;.Abs).
         * @param value The value whose absolute value is to be computed.
         * @return The absolute value of @p value.
         * @throws System::OverflowException if @p value is MinValue (its magnitude does not
         *         fit in a signed Int128).
         */
        [[nodiscard]] static Int128 Abs(const Int128& value) {
            if (value.value_ == MinValue().value_) throw System::OverflowException("Negating the minimum value of a twos complement number is invalid.");
            return value.value_ < 0 ? Int128(-value.value_) : value;
        }

        /**
         * @brief Produces the full product of two Int128 values.
         * @param left      The dividend.
         * @param right     The divisor.
         * @param remainder Set to the remainder of the division.
         * @return The quotient of dividing @p left by @p right.
         * @throws System::DivideByZeroException if @p right is zero (inherited from operator/).
         * @throws System::OverflowException if @p left is MinValue and @p right is -1
         *         (inherited from operator/ -- see its doc-comment for the underlying reason).
         */
        static Int128 DivRem(const Int128& left, const Int128& right, Int128& remainder) {
            Int128 quotient = left / right;
            remainder = left - quotient * right;
            return quotient;
        }

        /**
         * @brief Counts the number of leading zero bits in a value.
         * @param value The value whose leading zeros are to be counted.
         * @return The number of leading zero bits in @p value.
         */
        [[nodiscard]] static intcs LeadingZeroCount(const Int128& value) noexcept {
            uint64_t hi = value.getUpperProperty();
            if (hi != 0) return static_cast<intcs>(__builtin_clzll(hi));
            uint64_t lo = value.getLowerProperty();
            if (lo != 0) return static_cast<intcs>(64 + __builtin_clzll(lo));
            return 128;
        }

        /**
         * @brief Counts the number of trailing zero bits in a value.
         * @param value The value whose trailing zeros are to be counted.
         * @return The number of trailing zero bits in @p value.
         */
        [[nodiscard]] static intcs TrailingZeroCount(const Int128& value) noexcept {
            uint64_t lo = value.getLowerProperty();
            if (lo != 0) return static_cast<intcs>(__builtin_ctzll(lo));
            uint64_t hi = value.getUpperProperty();
            if (hi != 0) return static_cast<intcs>(64 + __builtin_ctzll(hi));
            return 128;
        }

        /**
         * @brief Computes the number of bits that are set in a value.
         * @param value The value whose set bits are to be counted.
         * @return The number of set bits in @p value.
         */
        [[nodiscard]] static intcs PopCount(const Int128& value) noexcept {
            return static_cast<intcs>(__builtin_popcountll(value.getLowerProperty())
                 + __builtin_popcountll(value.getUpperProperty()));
        }

        /**
         * @brief Rotates a value left by a given number of bits.
         * @param value        The value to rotate.
         * @param rotateAmount The number of bits to rotate by.
         * @return @p value rotated left by @p rotateAmount bits.
         */
        [[nodiscard]] static Int128 RotateLeft(const Int128& value, intcs rotateAmount) noexcept {
            rotateAmount &= 127;
            if (rotateAmount == 0) return value;
            auto uv = static_cast<unsigned __int128>(value.value_);
            return Int128(static_cast<__int128>(
                (uv << rotateAmount) | (uv >> (128 - rotateAmount))));
        }

        /**
         * @brief Rotates a value right by a given number of bits.
         * @param value        The value to rotate.
         * @param rotateAmount The number of bits to rotate by.
         * @return @p value rotated right by @p rotateAmount bits.
         */
        [[nodiscard]] static Int128 RotateRight(const Int128& value, intcs rotateAmount) noexcept {
            rotateAmount &= 127;
            if (rotateAmount == 0) return value;
            auto uv = static_cast<unsigned __int128>(value.value_);
            return Int128(static_cast<__int128>(
                (uv >> rotateAmount) | (uv << (128 - rotateAmount))));
        }

        /**
         * @brief Clamps a value between an inclusive minimum and maximum.
         *
         * C++ counterpart of .NET Int128.Clamp(Int128,Int128,Int128) (INumber&lt;TSelf&gt;.Clamp).
         * @param value The value to clamp.
         * @param min   The inclusive lower bound.
         * @param max   The inclusive upper bound.
         * @throws System::ArgumentException if @p min is greater than @p max.
         */
        [[nodiscard]] static Int128 Clamp(const Int128& value, const Int128& min, const Int128& max) {
            if (min > max) throw System::ArgumentException("min cannot be greater than max.");
            if (value < min) return min;
            if (value > max) return max;
            return value;
        }

        /** @brief Returns the larger of two values. C++ counterpart of .NET Int128.Max(Int128,Int128). */
        [[nodiscard]] static Int128 Max(const Int128& x, const Int128& y) noexcept { return x >= y ? x : y; }

        /** @brief Returns the smaller of two values. C++ counterpart of .NET Int128.Min(Int128,Int128). */
        [[nodiscard]] static Int128 Min(const Int128& x, const Int128& y) noexcept { return x <= y ? x : y; }

        /**
         * @brief Computes the sign of a value.
         *
         * C++ counterpart of .NET Int128.Sign(Int128) (INumber&lt;TSelf&gt;.Sign).
         * @return -1 if @p value is negative, 0 if zero, 1 if positive.
         */
        [[nodiscard]] static intcs Sign(const Int128& value) noexcept {
            if (value.value_ < 0) return -1;
            if (value.value_ > 0) return 1;
            return 0;
        }

        /** @brief Returns true if @p value is an even integer. C++ counterpart of .NET Int128.IsEvenInteger(Int128). */
        [[nodiscard]] static bool IsEvenInteger(const Int128& value) noexcept {
            return (value.getLowerProperty() & 1) == 0;
        }

        /** @brief Returns true if @p value is an odd integer. C++ counterpart of .NET Int128.IsOddInteger(Int128). */
        [[nodiscard]] static bool IsOddInteger(const Int128& value) noexcept {
            return (value.getLowerProperty() & 1) != 0;
        }

        /**
         * @brief Returns true if @p value is a power of two.
         *
         * C++ counterpart of .NET Int128.IsPow2(Int128) (IBinaryNumber&lt;TSelf&gt;.IsPow2).
         * Matches .NET: negative and zero values are never powers of two.
         */
        [[nodiscard]] static bool IsPow2(const Int128& value) noexcept {
            return PopCount(value) == 1 && value.value_ > 0;
        }

        /**
         * @brief Computes the base-2 logarithm (floor) of a value.
         *
         * C++ counterpart of .NET Int128.Log2(Int128) (IBinaryNumber&lt;TSelf&gt;.Log2).
         * Matches .NET: Log2(0) is 0, not an error.
         * @throws System::ArgumentOutOfRangeException if @p value is negative.
         */
        [[nodiscard]] static intcs Log2(const Int128& value) {
            if (value.value_ < 0) throw System::ArgumentOutOfRangeException("value", "value must be non-negative");
            uint64_t hi = value.getUpperProperty();
            if (hi != 0) return static_cast<intcs>(64 + 63 - __builtin_clzll(hi));
            uint64_t lo = value.getLowerProperty();
            if (lo == 0) return 0;
            return static_cast<intcs>(63 - __builtin_clzll(lo));
        }

        // ---------------------------------------------------------------
        // Arithmetic operators
        // ---------------------------------------------------------------

        /**
         * @brief Adds two Int128 values.
         *
         * Matches real .NET's default (unchecked) Int128 operator+, which wraps silently on
         * overflow rather than throwing (only the separate `checked` operator, not modeled by
         * this port, throws OverflowException) -- computed via unsigned __int128 arithmetic
         * (always well-defined modular wraparound) and converted back, since raw signed
         * __int128 overflow is genuine undefined behavior in C++ (confirmed via a standalone
         * UBSan repro before fixing), unlike C#'s defined-wraparound unchecked semantics.
         */
        Int128 operator+(const Int128& o) const noexcept {
            return Int128(static_cast<__int128>(
                static_cast<unsigned __int128>(value_) + static_cast<unsigned __int128>(o.value_)));
        }
        /** @brief Subtracts one Int128 from another. Wraps silently on overflow (see operator+'s doc-comment). */
        Int128 operator-(const Int128& o) const noexcept {
            return Int128(static_cast<__int128>(
                static_cast<unsigned __int128>(value_) - static_cast<unsigned __int128>(o.value_)));
        }
        /** @brief Multiplies two Int128 values. Wraps silently on overflow (see operator+'s doc-comment). */
        Int128 operator*(const Int128& o) const noexcept {
            return Int128(static_cast<__int128>(
                static_cast<unsigned __int128>(value_) * static_cast<unsigned __int128>(o.value_)));
        }
        /**
         * @brief Divides an Int128 by another.
         *
         * @throws System::DivideByZeroException if @p o is zero.
         * @throws System::OverflowException if this value is MinValue and @p o is -1 -- unlike
         * add/subtract/multiply, real .NET's Int128 operator/ explicitly checks for and throws on this specific
         * case even in its unchecked form (the magnitude of MaxValue+1 doesn't fit), rather than
         * wrapping. Confirmed via a standalone UBSan repro that plain __int128 division hits
         * real UB here too (a different failure mode than the wraparound cases above).
         */
        Int128 operator/(const Int128& o) const {
            if (o.value_ == 0) throw System::DivideByZeroException("Attempted to divide by zero.");
            if (value_ == MinValue().value_ && o.value_ == -1)
                throw System::OverflowException("Negating the minimum value of a twos complement number is invalid.");
            return Int128(value_ / o.value_);
        }
        /**
         * @brief Computes the remainder of dividing one Int128 by another.
         *
         * @throws System::DivideByZeroException if @p o is zero.
         * @throws System::OverflowException if this value is MinValue and @p o is -1 -- real
         * .NET's Int128 operator% is defined as `left - (left / right) * right`, so it inherits
         * operator/'s MinValue/-1 overflow check; matched here directly since plain __int128 `%`
         * hits the identical UB as `/` for this case (division and remainder share the same
         * underlying hardware/library operation).
         */
        Int128 operator%(const Int128& o) const {
            if (o.value_ == 0) throw System::DivideByZeroException("Attempted to divide by zero.");
            if (value_ == MinValue().value_ && o.value_ == -1)
                throw System::OverflowException("Negating the minimum value of a twos complement number is invalid.");
            return Int128(value_ % o.value_);
        }
        /** @brief Computes the bitwise AND of two Int128 values. */
        Int128 operator&(const Int128& o) const noexcept { return Int128(value_ & o.value_); }
        /** @brief Computes the bitwise OR of two Int128 values. */
        Int128 operator|(const Int128& o) const noexcept { return Int128(value_ | o.value_); }
        /** @brief Computes the bitwise XOR of two Int128 values. */
        Int128 operator^(const Int128& o) const noexcept { return Int128(value_ ^ o.value_); }
        /** @brief Returns the bitwise complement of an Int128. */
        Int128 operator~() const noexcept { return Int128(~value_); }
        /**
         * @brief Returns the arithmetic negation of an Int128.
         *
         * Matches real .NET's unchecked `operator -(Int128 value) => Zero - value`: negating
         * MinValue wraps back to MinValue itself (a classic two's-complement quirk, not an
         * error in the unchecked operator -- Int128::Abs() is the method that throws for this
         * case). Computed via unsigned negation, which is always well-defined, unlike negating
         * MinValue as a signed __int128 (confirmed real UB via a standalone UBSan repro).
         */
        Int128 operator-() const noexcept {
            return Int128(static_cast<__int128>(-static_cast<unsigned __int128>(value_)));
        }
        /**
         * @brief Shifts an Int128 left by n bits.
         *
         * C++ counterpart of .NET Int128 operator&lt;&lt;. Matches .NET: @p n is masked to
         * its low 7 bits (mod 128) rather than being undefined behavior out of range,
         * since the native @c __int128 shift operator does not auto-mask like C#'s does.
         */
        Int128 operator<<(intcs n) const noexcept { return Int128(value_ << (n & 127)); }
        /**
         * @brief Shifts an Int128 right by n bits (arithmetic/signed).
         *
         * C++ counterpart of .NET Int128 operator&gt;&gt;. Matches .NET: @p n is masked to
         * its low 7 bits (mod 128) rather than being undefined behavior out of range.
         */
        Int128 operator>>(intcs n) const noexcept { return Int128(value_ >> (n & 127)); }

        // ---------------------------------------------------------------
        // Comparison operators
        // ---------------------------------------------------------------

        /** @brief Returns true if two Int128 values are equal. */
        bool operator==(const Int128& o) const noexcept { return value_ == o.value_; }
        /** @brief Returns true if two Int128 values are not equal. */
        bool operator!=(const Int128& o) const noexcept { return value_ != o.value_; }
        /** @brief Returns true if this Int128 is less than another. */
        bool operator< (const Int128& o) const noexcept { return value_ <  o.value_; }
        /** @brief Returns true if this Int128 is less than or equal to another. */
        bool operator<=(const Int128& o) const noexcept { return value_ <= o.value_; }
        /** @brief Returns true if this Int128 is greater than another. */
        bool operator> (const Int128& o) const noexcept { return value_ >  o.value_; }
        /** @brief Returns true if this Int128 is greater than or equal to another. */
        bool operator>=(const Int128& o) const noexcept { return value_ >= o.value_; }

        // ---------------------------------------------------------------
        // Explicit conversions to primitive types
        // ---------------------------------------------------------------

        /** @brief Explicitly converts to long long (truncates to 64 bits). */
        explicit operator long long()          const noexcept { return static_cast<long long>(value_); }
        /** @brief Explicitly converts to unsigned long long (truncates to 64 bits). */
        explicit operator unsigned long long() const noexcept { return static_cast<unsigned long long>(value_); }
        /** @brief Explicitly converts to double (may lose precision). */
        explicit operator double()             const noexcept { return static_cast<double>(value_); }
        /** @brief Returns the underlying __int128 value. */
        explicit operator __int128()           const noexcept { return value_; }
    };

} // namespace System

// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>

#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/DivideByZeroException.hpp"
#include "System/FormatException.hpp"
#include "System/OverflowException.hpp"

#if defined(_MSC_VER)
#  error "UInt128 requires unsigned __int128 (GCC/Clang only). MSVC is not supported for this type."
#endif

namespace System {

    using SharpRuntime::intcs;

    /**
     * @brief 128-bit unsigned integer, mirroring .NET System.UInt128 (GCC/Clang only).
     *
     * C++ counterpart of .NET System.UInt128.
     * Arithmetic and bitwise operators, conversions, and ToString are provided.
     */
    class UInt128 {
        unsigned __int128 value_;

    public:
        /** @brief Constructs a UInt128 with value 0. */
        constexpr UInt128() : value_(0) {}
        /** @brief Constructs a UInt128 from a raw unsigned __int128 value. */
        constexpr explicit UInt128(unsigned __int128 v) : value_(v) {}
        /**
         * @brief Constructs a UInt128 from a 64-bit upper half and a 64-bit lower half.
         * @param upper High 64 bits.
         * @param lower Low 64 bits.
         */
        constexpr UInt128(uint64_t upper, uint64_t lower)
            : value_((static_cast<unsigned __int128>(upper) << 64) | lower) {}

        /** @brief Returns the minimum representable UInt128 value (0). */
        static const UInt128 MinValue() { return UInt128(static_cast<unsigned __int128>(0)); }
        /** @brief Returns the maximum representable UInt128 value. */
        static const UInt128 MaxValue() {
            return UInt128((static_cast<unsigned __int128>(0xFFFFFFFFFFFFFFFFULL) << 64) | 0xFFFFFFFFFFFFFFFFULL);
        }
        /** @brief Returns a UInt128 with value 0. */
        static const UInt128 Zero() { return UInt128(0); }
        /** @brief Returns a UInt128 with value 1. */
        static const UInt128 One()  { return UInt128(1); }

        /** @brief Returns the low 64 bits as an unsigned 64-bit integer. */
        [[nodiscard]] uint64_t getLowerProperty() const { return static_cast<uint64_t>(value_); }
        /** @brief Returns the high 64 bits as an unsigned 64-bit integer. */
        [[nodiscard]] uint64_t getUpperProperty() const { return static_cast<uint64_t>(value_ >> 64); }

        /** @brief Explicit conversion to unsigned long long (truncates to 64 bits). */
        explicit operator unsigned long long() const { return static_cast<unsigned long long>(value_); }
        /** @brief Explicit conversion to long long (truncates to 64 bits, may change sign). */
        explicit operator long long()          const { return static_cast<long long>(value_); }
        /** @brief Explicit conversion to double (may lose precision). */
        explicit operator double()             const { return static_cast<double>(value_); }
        /** @brief Explicit conversion to unsigned __int128. */
        explicit operator unsigned __int128()  const { return value_; }

        /** @brief Addition operator. */
        UInt128 operator+(const UInt128& o) const { return UInt128(value_ + o.value_); }
        /** @brief Subtraction operator. */
        UInt128 operator-(const UInt128& o) const { return UInt128(value_ - o.value_); }
        /** @brief Multiplication operator. */
        UInt128 operator*(const UInt128& o) const { return UInt128(value_ * o.value_); }
        /** @brief Division operator. @throws System::DivideByZeroException if @p o is zero. */
        UInt128 operator/(const UInt128& o) const {
            if (o.value_ == 0) throw System::DivideByZeroException("Attempted to divide by zero.");
            return UInt128(value_ / o.value_);
        }
        /** @brief Modulo operator. @throws System::DivideByZeroException if @p o is zero. */
        UInt128 operator%(const UInt128& o) const {
            if (o.value_ == 0) throw System::DivideByZeroException("Attempted to divide by zero.");
            return UInt128(value_ % o.value_);
        }
        /** @brief Bitwise AND operator. */
        UInt128 operator&(const UInt128& o) const { return UInt128(value_ & o.value_); }
        /** @brief Bitwise OR operator. */
        UInt128 operator|(const UInt128& o) const { return UInt128(value_ | o.value_); }
        /** @brief Bitwise XOR operator. */
        UInt128 operator^(const UInt128& o) const { return UInt128(value_ ^ o.value_); }
        /** @brief Bitwise NOT operator. */
        UInt128 operator~()               const { return UInt128(~value_); }
        /** @brief Left-shift operator. */
        UInt128 operator<<(int n)         const { return UInt128(value_ << n); }
        /** @brief Right-shift operator. */
        UInt128 operator>>(int n)         const { return UInt128(value_ >> n); }

        /** @brief Equality comparison. */
        bool operator==(const UInt128& o) const { return value_ == o.value_; }
        /** @brief Inequality comparison. */
        bool operator!=(const UInt128& o) const { return value_ != o.value_; }
        /** @brief Less-than comparison. */
        bool operator< (const UInt128& o) const { return value_ <  o.value_; }
        /** @brief Less-than-or-equal comparison. */
        bool operator<=(const UInt128& o) const { return value_ <= o.value_; }
        /** @brief Greater-than comparison. */
        bool operator> (const UInt128& o) const { return value_ >  o.value_; }
        /** @brief Greater-than-or-equal comparison. */
        bool operator>=(const UInt128& o) const { return value_ >= o.value_; }

        /** @brief Returns the decimal string representation of this UInt128. */
        [[nodiscard]] std::string ToString() const {
            if (value_ == 0) return "0";
            unsigned __int128 v = value_;
            std::string s;
            while (v > 0) { s += char('0' + (int)(v % 10)); v /= 10; }
            std::reverse(s.begin(), s.end());
            return s;
        }

        /**
         * @brief Converts the value of this UInt128 to its equivalent string representation,
         * using the specified format.
         *
         * C++ counterpart of .NET UInt128.ToString(string). Supports "X"/"x" (hexadecimal),
         * "D"/"d" (decimal), and "G"/"g" (general), each optionally followed by a minimum
         * field width (e.g. "X32"), zero-padded. Mirrors Int128::ToString(string) minus the
         * sign handling, since UInt128 is never negative.
         * @param format The numeric format specifier.
         * @return A string containing the formatted representation of this value.
         */
        [[nodiscard]] std::string ToString(const std::string& format) const {
            if (format.empty()) return ToString();
            char type = format[0];
            int width = format.size() > 1 ? std::stoi(format.substr(1)) : 0;
            if (type == 'X' || type == 'x') {
                unsigned __int128 uv = value_;
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
                while (static_cast<int>(s.size()) < width) s = "0" + s;
                return s;
            }
            return ToString();
        }

        /**
         * @brief Converts the string representation of a number to its UInt128 equivalent.
         *
         * C++ counterpart of .NET UInt128.Parse(string) with NumberStyles.Integer: a leading
         * '-' always overflows (unsigned types reject any negative sign, even "-0"), matching
         * the Parse convention already established for UInt16/UInt32/UInt64 in this codebase --
         * unlike Int128::Parse's simplified single-FormatException TryParse wrapper, this
         * distinguishes FormatException (bad characters/empty string) from OverflowException
         * (value out of [0, MaxValue] range or a leading '-'), matching real .NET's actual
         * UInt128 parsing behavior (Number.ParseUInt128 throws OverflowException specifically
         * on range violations).
         * @throws System::FormatException if @p s is not a valid integer string.
         * @throws System::OverflowException if @p s has a leading '-' or the value exceeds
         *         UInt128's range.
         */
        static UInt128 Parse(const std::string& s) {
            if (s.empty())
                throw System::FormatException("Input string was not in a correct format.");
            std::size_t i = 0;
            if (s[i] == '-')
                throw System::OverflowException("Value was either too large or too small for a UInt128.");
            if (s[i] == '+') ++i;
            if (i == s.size())
                throw System::FormatException("Input string was not in a correct format.");

            constexpr unsigned __int128 kMax = ~static_cast<unsigned __int128>(0);
            unsigned __int128 val = 0;
            for (; i < s.size(); ++i) {
                if (s[i] < '0' || s[i] > '9')
                    throw System::FormatException("Input string was not in a correct format.");
                unsigned int digit = static_cast<unsigned int>(s[i] - '0');
                if (val > (kMax - digit) / 10)
                    throw System::OverflowException("Value was either too large or too small for a UInt128.");
                val = val * 10 + digit;
            }
            return UInt128(val);
        }

        /**
         * @brief Tries to convert a string to its UInt128 equivalent.
         * @param s      A string containing the number to convert.
         * @param result Set to the parsed value on success, or 0 on failure.
         * @return true if @p s was converted successfully; false otherwise.
         */
        static bool TryParse(const std::string& s, UInt128& result) {
            try { result = Parse(s); return true; }
            catch (...) { result = UInt128(); return false; }
        }

        /**
         * @brief Compares this value to @p other.
         * C++ counterpart of .NET UInt128.CompareTo(UInt128).
         */
        [[nodiscard]] intcs CompareTo(const UInt128& other) const {
            return (value_ < other.value_) ? -1 : (value_ > other.value_) ? 1 : 0;
        }

        /** @brief Returns true if this value equals @p other. C++ counterpart of .NET UInt128.Equals(UInt128). */
        [[nodiscard]] bool Equals(const UInt128& other) const { return value_ == other.value_; }

        /**
         * @brief Returns a hash code for this value.
         * C++ counterpart of .NET UInt128.GetHashCode() (HashCode.Combine(lower, upper)).
         */
        [[nodiscard]] intcs GetHashCode() const {
            std::size_t seed = std::hash<uint64_t>{}(getLowerProperty());
            seed ^= std::hash<uint64_t>{}(getUpperProperty())
                  + 0x9e3779b9u + (seed << 6) + (seed >> 2);
            return static_cast<intcs>(seed);
        }

        /** @brief Returns the larger of @p x and @p y. C++ counterpart of .NET UInt128.Max(UInt128,UInt128). */
        [[nodiscard]] static UInt128 Max(const UInt128& x, const UInt128& y) { return (x >= y) ? x : y; }

        /** @brief Returns the smaller of @p x and @p y. C++ counterpart of .NET UInt128.Min(UInt128,UInt128). */
        [[nodiscard]] static UInt128 Min(const UInt128& x, const UInt128& y) { return (x <= y) ? x : y; }

        /** @brief Clamps @p value to [@p min, @p max]. C++ counterpart of .NET UInt128.Clamp(UInt128,UInt128,UInt128). */
        [[nodiscard]] static UInt128 Clamp(const UInt128& value, const UInt128& min, const UInt128& max) {
            return value < min ? min : (value > max ? max : value);
        }

        /** @brief Returns 0 if @p value is zero; otherwise 1. C++ counterpart of .NET UInt128.Sign(UInt128). */
        [[nodiscard]] static intcs Sign(const UInt128& value) { return value.value_ == 0 ? 0 : 1; }

        /** @brief Returns true if @p value is an even integer. C++ counterpart of .NET UInt128.IsEvenInteger(UInt128). */
        [[nodiscard]] static bool IsEvenInteger(const UInt128& value) { return (value.getLowerProperty() & 1) == 0; }

        /** @brief Returns true if @p value is an odd integer. C++ counterpart of .NET UInt128.IsOddInteger(UInt128). */
        [[nodiscard]] static bool IsOddInteger(const UInt128& value) { return (value.getLowerProperty() & 1) != 0; }

        /** @brief Returns true if @p value is a power of two. C++ counterpart of .NET UInt128.IsPow2(UInt128). */
        [[nodiscard]] static bool IsPow2(const UInt128& value) {
            return value.value_ != 0 && (value.value_ & (value.value_ - 1)) == 0;
        }
    };

} // namespace System

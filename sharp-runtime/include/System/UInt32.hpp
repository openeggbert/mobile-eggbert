// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <algorithm>
#include <bit>
#include <cctype>
#include <cstdint>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string>
#include <utility>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/DivideByZeroException.hpp"
#include "System/FormatException.hpp"
#include "System/Globalization/NumberStyles.hpp"
#include "System/OverflowException.hpp"
#include "System/detail/IntegerNumberStylesParser.hpp"

namespace System {

    class IFormatProvider;

    using SharpRuntime::uintcs;
    using SharpRuntime::intcs;

    /**
     * @brief Represents a 32-bit unsigned integer.
     *
     * C++ counterpart of .NET System.UInt32.
     * All members are static; the class cannot be instantiated.
     * The underlying C++ type is @c uint32_t (aliased as @c SharpRuntime::uintcs).
     */
    class UInt32 {
    public:
        UInt32() = delete;

        /** @brief The maximum value of a UInt32 (4 294 967 295). C++ counterpart of .NET UInt32.MaxValue. */
        static constexpr uintcs MaxValue = std::numeric_limits<uint32_t>::max();

        /** @brief The minimum value of a UInt32 (0). C++ counterpart of .NET UInt32.MinValue. */
        static constexpr uintcs MinValue = 0;

        /**
         * @brief Parses @p s as a decimal UInt32 value.
         *
         * C++ counterpart of .NET UInt32.Parse(string) with NumberStyles.Integer: leading/
         * trailing whitespace and a leading sign are tolerated, but a leading '-' always
         * overflows (unsigned types reject any negative sign, even "-0" -- verified against
         * real .NET's Number.Parsing.cs: `(!TInteger.IsSigned && number.IsNegative)` is an
         * overflow condition checked independent of magnitude) and any trailing non-whitespace
         * character is rejected. This previously called std::stoul(s) without capturing the
         * parse-end position (so trailing garbage like "5abc" was silently accepted) and
         * without rejecting a leading '-' explicitly. The latter is not just an "-0" edge
         * case here: on an LLP64 platform (e.g. Windows, where `unsigned long` is 32 bits,
         * the same width as UInt32), std::stoul("-1") wraps to ULONG_MAX == UInt32::MaxValue
         * exactly, which the old `v > MaxValue` check does NOT catch (`>`, not `>=`) --
         * Parse("-1") would have silently returned 4294967295 on such a platform.
         * @throws System::FormatException on bad format.
         * @throws System::OverflowException on overflow, or if the string has a leading '-'.
         */
        [[nodiscard]] static uintcs Parse(const std::string& s) {
            std::size_t start = s.find_first_not_of(" \t\n\r\f\v");
            if (start != std::string::npos && s[start] == '-')
                throw System::OverflowException("Value was either too large or too small for a UInt32.");
            std::size_t pos = 0;
            unsigned long v;
            try {
                v = std::stoul(s, &pos);
            }
            catch (const std::out_of_range&) {
                throw System::OverflowException("Value was either too large or too small for a UInt32.");
            }
            catch (...) {
                throw System::FormatException("Input string was not in a correct format.");
            }
            for (; pos < s.size(); ++pos) {
                if (!std::isspace(static_cast<unsigned char>(s[pos])))
                    throw System::FormatException("Input string was not in a correct format.");
            }
            if (v > MaxValue) throw System::OverflowException("Value was either too large or too small for a UInt32.");
            return static_cast<uint32_t>(v);
        }

        /**
         * @brief Attempts to parse @p s as a UInt32; returns false on failure.
         * C++ counterpart of .NET UInt32.TryParse(string, out uint).
         */
        static bool TryParse(const std::string& s, uintcs& result) noexcept {
            try { result = Parse(s); return true; }
            catch (...) { result = 0; return false; }
        }

        /**
         * @brief Converts the string representation of a number in the specified style to its
         * UInt32 equivalent.
         *
         * C++ counterpart of .NET UInt32.Parse(string, NumberStyles, IFormatProvider). @p
         * provider is accepted for API-surface parity but ignored. Supports
         * NumberStyles.Integer, .Number, .Currency, and .HexNumber -- see
         * include/System/detail/IntegerNumberStylesParser.hpp for the exact supported grammar.
         * @throws System::FormatException if the string is not in a correct format for @p style.
         * @throws System::OverflowException if the value exceeds UInt32 range.
         */
        [[nodiscard]] static uintcs Parse(const std::string& s, System::Globalization::NumberStyles style,
                                           const IFormatProvider* provider) {
            (void)provider;
            uintcs result;
            if (!TryParse(s, style, provider, result)) {
                using System::Globalization::NumberStyles;
                if ((style & NumberStyles::AllowHexSpecifier) != NumberStyles::None) {
                    uint64_t bits; bool tooManyDigits = false;
                    System::detail::IntegerNumberStylesParser::TryParseHexCore(s, style, bits, 8, tooManyDigits);
                    if (tooManyDigits)
                        throw System::OverflowException("Value was either too large or too small for a UInt32.");
                    throw System::FormatException("Input string was not in a correct format.");
                }
                if ((style & NumberStyles::AllowBinarySpecifier) != NumberStyles::None) {
                    uint64_t bits; bool tooManyDigits = false;
                    System::detail::IntegerNumberStylesParser::TryParseBinaryCore(s, style, bits, 32, tooManyDigits);
                    if (tooManyDigits)
                        throw System::OverflowException("Value was either too large or too small for a UInt32.");
                    throw System::FormatException("Input string was not in a correct format.");
                }
                SharpRuntime::ulongcs unsignedResult; bool overflowed = false;
                if (System::detail::IntegerNumberStylesParser::TryParseUnsignedCore(s, style, unsignedResult, overflowed) &&
                    (overflowed || unsignedResult > MaxValue))
                    throw System::OverflowException("Value was either too large or too small for a UInt32.");
                throw System::FormatException("Input string was not in a correct format.");
            }
            return result;
        }

        /**
         * @brief Tries to convert a string to a UInt32 using the specified style, without
         * throwing.
         *
         * C++ counterpart of .NET UInt32.TryParse(string, NumberStyles, IFormatProvider, out uint).
         */
        static bool TryParse(const std::string& s, System::Globalization::NumberStyles style,
                              const IFormatProvider* provider, uintcs& result) noexcept {
            (void)provider;
            using System::Globalization::NumberStyles;
            result = 0;
            if ((style & NumberStyles::AllowHexSpecifier) != NumberStyles::None) {
                uint64_t bits; bool tooManyDigits = false;
                if (!System::detail::IntegerNumberStylesParser::TryParseHexCore(s, style, bits, 8, tooManyDigits))
                    return false;
                result = static_cast<uintcs>(bits);
                return true;
            }
            if ((style & NumberStyles::AllowBinarySpecifier) != NumberStyles::None) {
                uint64_t bits; bool tooManyDigits = false;
                if (!System::detail::IntegerNumberStylesParser::TryParseBinaryCore(s, style, bits, 32, tooManyDigits))
                    return false;
                result = static_cast<uintcs>(bits);
                return true;
            }
            SharpRuntime::ulongcs unsignedResult; bool overflowed = false;
            if (!System::detail::IntegerNumberStylesParser::TryParseUnsignedCore(s, style, unsignedResult, overflowed))
                return false;
            if (overflowed || unsignedResult > MaxValue) return false;
            result = static_cast<uintcs>(unsignedResult);
            return true;
        }

        /**
         * @brief Converts @p value to its decimal string representation.
         * C++ counterpart of .NET UInt32.ToString().
         */
        [[nodiscard]] static std::string ToString(uintcs value) { return std::to_string(value); }

        /**
         * @brief Converts @p value to a string using the specified format specifier
         * ("X"/"x" hexadecimal, "D"/"d" zero-padded decimal, "G"/"g" general).
         *
         * C++ counterpart of .NET UInt32.ToString(string format) -- added 2026-07-14
         * (duplicated-implementation audit finding: UInt32 was the only one of the 8 integer
         * types missing this overload, mirrored here from the identical implementation on its
         * unsigned siblings UInt16/UInt64).
         */
        [[nodiscard]] static std::string ToString(uintcs value, const std::string& format) {
            if (format.empty()) return ToString(value);
            char type = format[0];
            int width = 0;
            if (format.size() > 1) {
                try {
                    width = std::stoi(format.substr(1));
                } catch (const std::exception&) {
                    throw System::FormatException("Format specifier was invalid.");
                }
            }
            std::ostringstream oss;
            oss.imbue(std::locale::classic());
            if (type == 'X') { oss << std::uppercase << std::hex << std::setfill('0') << std::setw(width) << value; return oss.str(); }
            if (type == 'x') { oss << std::hex << std::setfill('0') << std::setw(width) << value; return oss.str(); }
            if (type == 'D' || type == 'd') {
                std::string s = std::to_string(value);
                while (static_cast<int>(s.size()) < width) s = "0" + s;
                return s;
            }
            if (type == 'G' || type == 'g') return ToString(value);
            return ToString(value);
        }

        /**
         * @brief Compares @p a to @p b and returns a signed integer.
         * C++ counterpart of .NET UInt32.CompareTo(uint).
         */
        [[nodiscard]] static intcs CompareTo(uintcs a, uintcs b) noexcept {
            return (a < b) ? -1 : (a > b) ? 1 : 0;
        }

        /** @brief Returns true if @p a equals @p b. C++ counterpart of .NET UInt32.Equals(uint). */
        [[nodiscard]] static bool Equals(uintcs a, uintcs b) noexcept { return a == b; }

        /** @brief Returns a hash code for @p value. C++ counterpart of .NET UInt32.GetHashCode(). */
        [[nodiscard]] static intcs GetHashCode(uintcs value) noexcept { return static_cast<intcs>(value); }

        /** @brief Clamps @p value to [@p min, @p max]. C++ counterpart of .NET UInt32.Clamp(uint,uint,uint). */
        [[nodiscard]] static uintcs Clamp(uintcs value, uintcs min, uintcs max) noexcept {
            return std::clamp(value, min, max);
        }

        /** @brief Returns the larger of @p x and @p y. C++ counterpart of .NET UInt32.Max(uint,uint). */
        [[nodiscard]] static uintcs Max(uintcs x, uintcs y) noexcept { return x > y ? x : y; }

        /** @brief Returns the smaller of @p x and @p y. C++ counterpart of .NET UInt32.Min(uint,uint). */
        [[nodiscard]] static uintcs Min(uintcs x, uintcs y) noexcept { return x < y ? x : y; }

        /** @brief Returns 0 if @p value is zero; otherwise 1. C++ counterpart of .NET UInt32.Sign(uint). */
        [[nodiscard]] static intcs Sign(uintcs value) noexcept { return value == 0 ? 0 : 1; }

        /**
         * @brief Returns the quotient and remainder of @p left / @p right.
         * C++ counterpart of .NET UInt32.DivRem(uint,uint).
         * @throws System::DivideByZeroException if @p right is zero -- integer division
         *         by zero is undefined behavior in C++ (a hardware trap, not a catchable
         *         exception), unlike the CLR's div instruction which .NET surfaces as a
         *         managed DivideByZeroException; this must be checked explicitly.
         */
        [[nodiscard]] static std::pair<uintcs, uintcs> DivRem(uintcs left, uintcs right) {
            if (right == 0)
                throw System::DivideByZeroException();
            return {left / right, left % right};
        }

        /** @brief Returns true when @p value is even. C++ counterpart of .NET UInt32.IsEvenInteger(uint). */
        [[nodiscard]] static bool IsEvenInteger(uintcs value) noexcept { return (value & 1) == 0; }

        /** @brief Returns true when @p value is odd. C++ counterpart of .NET UInt32.IsOddInteger(uint). */
        [[nodiscard]] static bool IsOddInteger(uintcs value) noexcept { return (value & 1) != 0; }

        /** @brief Returns true when @p value is a power of two. C++ counterpart of .NET UInt32.IsPow2(uint). */
        [[nodiscard]] static bool IsPow2(uintcs value) noexcept {
            return value != 0 && (value & (value - 1)) == 0;
        }

        /** @brief Returns the number of leading zero bits. C++ counterpart of .NET UInt32.LeadingZeroCount(uint). */
        [[nodiscard]] static uintcs LeadingZeroCount(uintcs value) noexcept {
            return static_cast<uintcs>(std::countl_zero(value));
        }

        /** @brief Returns the number of set bits. C++ counterpart of .NET UInt32.PopCount(uint). */
        [[nodiscard]] static uintcs PopCount(uintcs value) noexcept {
            return static_cast<uintcs>(std::popcount(value));
        }

        /** @brief Returns the number of trailing zero bits. C++ counterpart of .NET UInt32.TrailingZeroCount(uint). */
        [[nodiscard]] static uintcs TrailingZeroCount(uintcs value) noexcept {
            if (value == 0) return 32;
            return static_cast<uintcs>(std::countr_zero(value));
        }

        /** @brief Rotates @p value left by @p rotateAmount bits. C++ counterpart of .NET UInt32.RotateLeft(uint,int). */
        [[nodiscard]] static uintcs RotateLeft(uintcs value, intcs rotateAmount) noexcept {
            return std::rotl(value, rotateAmount);
        }

        /** @brief Rotates @p value right by @p rotateAmount bits. C++ counterpart of .NET UInt32.RotateRight(uint,int). */
        [[nodiscard]] static uintcs RotateRight(uintcs value, intcs rotateAmount) noexcept {
            return std::rotr(value, rotateAmount);
        }

        /**
         * @brief Returns the floor of the base-2 logarithm of @p value.
         * C++ counterpart of .NET UInt32.Log2(uint) (via BitOperations.Log2).
         * Matches .NET's explicit "0 -> 0" contract: Log2(0) returns 0, it does not throw.
         */
        [[nodiscard]] static uintcs Log2(uintcs value) noexcept {
            if (value == 0) return 0;
            return static_cast<uintcs>(std::bit_width(value) - 1);
        }
    };

} // namespace System

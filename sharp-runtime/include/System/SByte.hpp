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
#include <stdexcept>
#include <string>
#include <utility>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/DivideByZeroException.hpp"
#include "System/FormatException.hpp"
#include "System/Globalization/NumberStyles.hpp"
#include "System/OverflowException.hpp"
#include "System/detail/IntegerNumberStylesParser.hpp"

namespace System {

    class IFormatProvider;

    using SharpRuntime::sbytecs;

    /**
     * @brief Represents an 8-bit signed integer.
     *
     * C++ counterpart of .NET System.SByte.
     * All members are static; the class cannot be instantiated.
     * The underlying C++ type is @c int8_t (aliased as @c SharpRuntime::sbytecs).
     */
    class SByte {
    public:
        SByte() = delete;

        /** @brief The maximum value of an SByte (127). C++ counterpart of .NET SByte.MaxValue. */
        static constexpr sbytecs MaxValue = std::numeric_limits<int8_t>::max();

        /** @brief The minimum value of an SByte (-128). C++ counterpart of .NET SByte.MinValue. */
        static constexpr sbytecs MinValue = std::numeric_limits<int8_t>::min();

        /**
         * @brief Parses @p s as a decimal SByte value.
         * C++ counterpart of .NET SByte.Parse(string).
         * @throws System::OverflowException if the value exceeds SByte range.
         * @throws System::FormatException if the string is not a valid integer.
         */
        [[nodiscard]] static sbytecs Parse(const std::string& s) {
            // Unlike Byte/Int16/Int32/Int64/UInt64::Parse, this previously called
            // std::stoi(s) without capturing the parse-end position, so trailing non-
            // whitespace garbage (e.g. "5abc") was silently ignored instead of rejected --
            // confirmed via a standalone repro that std::stoi("5abc") returns 5 without
            // throwing. Real .NET's NumberStyles.Integer parity requires the entire string
            // (aside from leading/trailing whitespace) to be consumed.
            std::size_t pos = 0;
            int v;
            try {
                v = std::stoi(s, &pos);
            } catch (const std::out_of_range&) {
                throw System::OverflowException("Value was either too large or too small for a signed byte.");
            } catch (...) {
                throw System::FormatException("Input string was not in a correct format.");
            }
            for (; pos < s.size(); ++pos) {
                if (!std::isspace(static_cast<unsigned char>(s[pos])))
                    throw System::FormatException("Input string was not in a correct format.");
            }
            if (v < MinValue || v > MaxValue)
                throw System::OverflowException("Value was either too large or too small for a signed byte.");
            return static_cast<int8_t>(v);
        }

        /**
         * @brief Attempts to parse @p s as an SByte; returns false on failure.
         * C++ counterpart of .NET SByte.TryParse(string, out sbyte).
         */
        static bool TryParse(const std::string& s, sbytecs& result) noexcept {
            try { result = Parse(s); return true; }
            catch (...) { result = 0; return false; }
        }

        /**
         * @brief Converts the string representation of a number in the specified style to its
         * SByte equivalent.
         *
         * C++ counterpart of .NET SByte.Parse(string, NumberStyles, IFormatProvider). @p
         * provider is accepted for API-surface parity but ignored. Supports
         * NumberStyles.Integer, .Number, .Currency, and .HexNumber (hex reinterpreted as a
         * two's-complement bit pattern) -- see
         * include/System/detail/IntegerNumberStylesParser.hpp for the exact supported grammar.
         * @throws System::FormatException if the string is not in a correct format for @p style.
         * @throws System::OverflowException if the value exceeds SByte range.
         */
        [[nodiscard]] static sbytecs Parse(const std::string& s, System::Globalization::NumberStyles style,
                                            const IFormatProvider* provider) {
            (void)provider;
            sbytecs result;
            if (!TryParse(s, style, provider, result)) {
                using System::Globalization::NumberStyles;
                if ((style & NumberStyles::AllowHexSpecifier) != NumberStyles::None) {
                    uint64_t bits; bool tooManyDigits = false;
                    System::detail::IntegerNumberStylesParser::TryParseHexCore(s, style, bits, 2, tooManyDigits);
                    if (tooManyDigits)
                        throw System::OverflowException("Value was either too large or too small for a signed byte.");
                    throw System::FormatException("Input string was not in a correct format.");
                }
                if ((style & NumberStyles::AllowBinarySpecifier) != NumberStyles::None) {
                    uint64_t bits; bool tooManyDigits = false;
                    System::detail::IntegerNumberStylesParser::TryParseBinaryCore(s, style, bits, 8, tooManyDigits);
                    if (tooManyDigits)
                        throw System::OverflowException("Value was either too large or too small for a signed byte.");
                    throw System::FormatException("Input string was not in a correct format.");
                }
                SharpRuntime::longcs signedResult; bool overflowed = false;
                if (System::detail::IntegerNumberStylesParser::TryParseSignedCore(s, style, signedResult, overflowed) &&
                    (overflowed || signedResult < MinValue || signedResult > MaxValue))
                    throw System::OverflowException("Value was either too large or too small for a signed byte.");
                throw System::FormatException("Input string was not in a correct format.");
            }
            return result;
        }

        /**
         * @brief Tries to convert a string to an SByte using the specified style, without
         * throwing.
         *
         * C++ counterpart of .NET SByte.TryParse(string, NumberStyles, IFormatProvider, out sbyte).
         */
        static bool TryParse(const std::string& s, System::Globalization::NumberStyles style,
                              const IFormatProvider* provider, sbytecs& result) noexcept {
            (void)provider;
            using System::Globalization::NumberStyles;
            result = 0;
            if ((style & NumberStyles::AllowHexSpecifier) != NumberStyles::None) {
                uint64_t bits; bool tooManyDigits = false;
                if (!System::detail::IntegerNumberStylesParser::TryParseHexCore(s, style, bits, 2, tooManyDigits))
                    return false;
                result = static_cast<sbytecs>(static_cast<uint8_t>(bits));
                return true;
            }
            if ((style & NumberStyles::AllowBinarySpecifier) != NumberStyles::None) {
                uint64_t bits; bool tooManyDigits = false;
                if (!System::detail::IntegerNumberStylesParser::TryParseBinaryCore(s, style, bits, 8, tooManyDigits))
                    return false;
                result = static_cast<sbytecs>(static_cast<uint8_t>(bits));
                return true;
            }
            SharpRuntime::longcs signedResult; bool overflowed = false;
            if (!System::detail::IntegerNumberStylesParser::TryParseSignedCore(s, style, signedResult, overflowed))
                return false;
            if (overflowed || signedResult < MinValue || signedResult > MaxValue) return false;
            result = static_cast<sbytecs>(signedResult);
            return true;
        }

        /** @brief Converts @p value to its decimal string representation. C++ counterpart of .NET SByte.ToString(). */
        [[nodiscard]] static std::string ToString(sbytecs value) {
            return std::to_string(static_cast<int>(value));
        }

        /** @brief Converts @p value to a string using format specifier ("X","x","D","d","G","g"). */
        [[nodiscard]] static std::string ToString(sbytecs value, const std::string& format) {
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
            if (type == 'X') {
                oss << std::uppercase << std::hex
                    << std::setfill('0') << std::setw(width)
                    << (static_cast<unsigned>(value) & 0xFFu);
                return oss.str();
            }
            if (type == 'x') {
                oss << std::hex << std::setfill('0') << std::setw(width)
                    << (static_cast<unsigned>(value) & 0xFFu);
                return oss.str();
            }
            if (type == 'D' || type == 'd') {
                bool neg = value < 0;
                std::string s = std::to_string(neg ? -static_cast<int>(value) : static_cast<int>(value));
                while (static_cast<int>(s.size()) < width) s = "0" + s;
                return neg ? "-" + s : s;
            }
            if (type == 'G' || type == 'g') return ToString(value);
            return ToString(value);
        }

        /** @brief Compares @p a to @p b. C++ counterpart of .NET SByte.CompareTo(sbyte). */
        [[nodiscard]] static SharpRuntime::intcs CompareTo(sbytecs a, sbytecs b) noexcept {
            return static_cast<SharpRuntime::intcs>(a) - static_cast<SharpRuntime::intcs>(b);
        }

        /** @brief Returns true if @p a equals @p b. C++ counterpart of .NET SByte.Equals(sbyte). */
        [[nodiscard]] static bool Equals(sbytecs a, sbytecs b) noexcept { return a == b; }

        /** @brief Returns a hash code for @p value. C++ counterpart of .NET SByte.GetHashCode(). */
        [[nodiscard]] static SharpRuntime::intcs GetHashCode(sbytecs value) noexcept { return static_cast<SharpRuntime::intcs>(value); }

        /** @brief Clamps @p value to [@p min, @p max]. C++ counterpart of .NET SByte.Clamp(sbyte,sbyte,sbyte). */
        [[nodiscard]] static sbytecs Clamp(sbytecs value, sbytecs min, sbytecs max) noexcept {
            return std::clamp(value, min, max);
        }

        /** @brief Returns the larger of @p x and @p y. C++ counterpart of .NET SByte.Max(sbyte,sbyte). */
        [[nodiscard]] static sbytecs Max(sbytecs x, sbytecs y) noexcept { return x > y ? x : y; }

        /** @brief Returns the smaller of @p x and @p y. C++ counterpart of .NET SByte.Min(sbyte,sbyte). */
        [[nodiscard]] static sbytecs Min(sbytecs x, sbytecs y) noexcept { return x < y ? x : y; }

        /** @brief Returns -1 if negative, 0 if zero, 1 if positive. C++ counterpart of .NET Math.Sign(sbyte). */
        [[nodiscard]] static SharpRuntime::intcs Sign(sbytecs value) noexcept {
            return (value > 0) - (value < 0);
        }

        /** @brief Returns the absolute value of @p value. C++ counterpart of .NET SByte.Abs(sbyte). */
        [[nodiscard]] static sbytecs Abs(sbytecs value) {
            if (value == MinValue)
                throw System::OverflowException("Negating SByte.MinValue would overflow.");
            return value < 0 ? static_cast<sbytecs>(-value) : value;
        }

        /**
         * @brief Returns a value with the magnitude of @p value and the sign of @p sign.
         * C++ counterpart of .NET SByte.CopySign(sbyte, sbyte).
         * @throws System::OverflowException if @p value is MinValue and @p sign is non-negative
         *         (its magnitude does not fit in a signed SByte).
         */
        [[nodiscard]] static sbytecs CopySign(sbytecs value, sbytecs sign) {
            sbytecs abs = value < 0 ? (value == MinValue ? value : static_cast<sbytecs>(-value)) : value;
            if (sign >= 0) {
                if (abs < 0) throw System::OverflowException("Negating MinValue is not representable.");
                return abs;
            }
            return static_cast<sbytecs>(-abs);
        }

        /** @brief Returns true if @p value is negative. C++ counterpart of .NET SByte.IsNegative(sbyte). */
        [[nodiscard]] static bool IsNegative(sbytecs value) noexcept { return value < 0; }

        /** @brief Returns true if @p value is positive (> 0). C++ counterpart of .NET SByte.IsPositive(sbyte). */
        [[nodiscard]] static bool IsPositive(sbytecs value) noexcept { return value > 0; }

        /**
         * @brief Returns the base-10 logarithm of @p value, truncated to sbyte.
         *
         * C++ counterpart of .NET SByte.Log10(sbyte). .NET throws only for a negative
         * @p value (ArgumentOutOfRangeException — "Non-negative number required.");
         * value == 0 is valid and returns 0 (SByte.cs delegates to uint.Log10, whose
         * documented convention is Log10(0) == 0), not an error.
         * @throws System::ArgumentOutOfRangeException if @p value is negative.
         */
        [[nodiscard]] static sbytecs Log10(sbytecs value) {
            if (value < 0) throw System::ArgumentOutOfRangeException("value", "value must be non-negative");
            sbytecs result = 0;
            while (value >= 10) { value /= 10; ++result; }
            return result;
        }

        /**
         * @brief Returns the value with greater magnitude; if magnitudes are equal, returns @p x.
         *
         * C++ counterpart of .NET SByte.MaxMagnitude(sbyte, sbyte). MinValue has no
         * representable positive magnitude, so it always wins, matching .NET (which
         * detects this via a wrapped-negation check; here via a direct equality check
         * to avoid relying on signed-overflow behavior).
         */
        [[nodiscard]] static sbytecs MaxMagnitude(sbytecs x, sbytecs y) noexcept {
            if (x == MinValue) return x;
            if (y == MinValue) return y;
            sbytecs ax = x < 0 ? static_cast<sbytecs>(-x) : x;
            sbytecs ay = y < 0 ? static_cast<sbytecs>(-y) : y;
            return ax >= ay ? x : y;
        }

        /**
         * @brief Returns the value with smaller magnitude; if magnitudes are equal, returns @p x.
         *
         * C++ counterpart of .NET SByte.MinMagnitude(sbyte, sbyte). MinValue has no
         * representable positive magnitude, so it always loses, matching .NET.
         */
        [[nodiscard]] static sbytecs MinMagnitude(sbytecs x, sbytecs y) noexcept {
            if (x == MinValue) return y;
            if (y == MinValue) return x;
            sbytecs ax = x < 0 ? static_cast<sbytecs>(-x) : x;
            sbytecs ay = y < 0 ? static_cast<sbytecs>(-y) : y;
            return ax <= ay ? x : y;
        }

        /**
         * @brief Rotates @p value left by @p rotateAmount bits within an 8-bit field.
         * C++ counterpart of .NET SByte.RotateLeft(sbyte, int).
         */
        [[nodiscard]] static sbytecs RotateLeft(sbytecs value, SharpRuntime::intcs rotateAmount) noexcept {
            uint8_t uv = static_cast<uint8_t>(value);
            int shift = rotateAmount & 7;
            return static_cast<sbytecs>((uv << shift) | (uv >> (8 - shift)));
        }

        /**
         * @brief Rotates @p value right by @p rotateAmount bits within an 8-bit field.
         * C++ counterpart of .NET SByte.RotateRight(sbyte, int).
         */
        [[nodiscard]] static sbytecs RotateRight(sbytecs value, SharpRuntime::intcs rotateAmount) noexcept {
            uint8_t uv = static_cast<uint8_t>(value);
            int shift = rotateAmount & 7;
            return static_cast<sbytecs>((uv >> shift) | (uv << (8 - shift)));
        }

        /**
         * @brief Returns the quotient and remainder of @p left / @p right.
         * C++ counterpart of .NET SByte.DivRem(sbyte,sbyte).
         * @throws System::DivideByZeroException if @p right is zero -- integer division
         *         by zero is undefined behavior in C++ (a hardware trap, not a catchable
         *         exception), unlike the CLR's div instruction which .NET surfaces as a
         *         managed DivideByZeroException; this must be checked explicitly. No
         *         MinValue/-1 overflow check is needed: sbyte operands promote to int in
         *         C++ arithmetic (matching the CLR's int32-width IL arithmetic for
         *         sbyte), so sbyte.MinValue/-1 does not overflow at the width the
         *         division runs at.
         */
        [[nodiscard]] static std::pair<sbytecs, sbytecs> DivRem(sbytecs left, sbytecs right) {
            if (right == 0)
                throw System::DivideByZeroException();
            return {static_cast<sbytecs>(left / right),
                    static_cast<sbytecs>(left % right)};
        }

        /** @brief Returns true when @p value is even. C++ counterpart of .NET SByte.IsEvenInteger(sbyte). */
        [[nodiscard]] static bool IsEvenInteger(sbytecs value) noexcept { return (value & 1) == 0; }

        /** @brief Returns true when @p value is odd. C++ counterpart of .NET SByte.IsOddInteger(sbyte). */
        [[nodiscard]] static bool IsOddInteger(sbytecs value) noexcept { return (value & 1) != 0; }

        /** @brief Returns true when @p value is a power of two. C++ counterpart of .NET SByte.IsPow2(sbyte). */
        [[nodiscard]] static bool IsPow2(sbytecs value) noexcept {
            return value > 0 && (value & (value - 1)) == 0;
        }

        /**
         * @brief Returns the number of leading zero bits, treating @p value as its raw 8-bit pattern.
         *
         * C++ counterpart of .NET SByte.LeadingZeroCount(sbyte) —
         * `(sbyte)(BitOperations.LeadingZeroCount((byte)value) - 24)`. Negative values are
         * reinterpreted as their unsigned 8-bit bit pattern, not treated as a "value <= 0"
         * special case: e.g. value == -1 (bit pattern 0xFF) has 0 leading zero bits, not 8.
         */
        [[nodiscard]] static sbytecs LeadingZeroCount(sbytecs value) noexcept {
            return static_cast<sbytecs>(
                std::countl_zero(static_cast<uint32_t>(static_cast<uint8_t>(value))) - 24);
        }

        /** @brief Returns the number of set bits. C++ counterpart of .NET SByte.PopCount(sbyte). */
        [[nodiscard]] static sbytecs PopCount(sbytecs value) noexcept {
            return static_cast<sbytecs>(
                std::popcount(static_cast<uint8_t>(value)));
        }

        /** @brief Returns the number of trailing zero bits. C++ counterpart of .NET SByte.TrailingZeroCount(sbyte). */
        [[nodiscard]] static sbytecs TrailingZeroCount(sbytecs value) noexcept {
            if (value == 0) return 8;
            return static_cast<sbytecs>(
                std::countr_zero(static_cast<uint32_t>(static_cast<uint8_t>(value))));
        }

        /**
         * @brief Returns the floor of the base-2 logarithm of @p value.
         *
         * C++ counterpart of .NET SByte.Log2(sbyte). .NET throws only for a negative
         * @p value (ArgumentOutOfRangeException — "Non-negative number required.");
         * value == 0 is valid and returns 0 (SByte.cs delegates to BitOperations.Log2,
         * whose documented convention is Log2(0) == 0), not an error.
         * @throws System::ArgumentOutOfRangeException if @p value is negative.
         */
        [[nodiscard]] static sbytecs Log2(sbytecs value) {
            if (value < 0) throw System::ArgumentOutOfRangeException("value", "value must be non-negative");
            if (value == 0) return 0;
            return static_cast<sbytecs>(
                std::bit_width(static_cast<uint32_t>(static_cast<uint8_t>(value))) - 1);
        }
    };

} // namespace System

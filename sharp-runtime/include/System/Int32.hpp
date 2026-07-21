// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <algorithm>
#include <bit>
#include <cctype>
#include <cstdint>
#include <cstdlib>
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

/**
 * @brief Static helper methods that mirror System.Int32 in .NET.
 *
 * C++ counterpart of .NET System.Int32.
 * Represents a 32-bit signed integer and exposes the static utility methods
 * of the .NET Int32 struct as static C++ methods.
 */
class Int32 {
public:
    /** @brief Maximum value of a 32-bit signed integer (2 147 483 647). */
    static constexpr SharpRuntime::intcs MaxValue = std::numeric_limits<int32_t>::max();
    /** @brief Minimum value of a 32-bit signed integer (-2 147 483 648). */
    static constexpr SharpRuntime::intcs MinValue = std::numeric_limits<int32_t>::min();

    // -----------------------------------------------------------------------
    // Parse / TryParse
    // -----------------------------------------------------------------------

    /**
     * @brief Converts the string representation of a number to its 32-bit
     * signed integer equivalent.
     *
     * C++ counterpart of .NET Int32.Parse(string) with NumberStyles.Integer:
     * leading/trailing whitespace and a leading sign are tolerated, but any
     * trailing non-whitespace character is rejected.
     * @param s String to parse.
     * @return Parsed int32 value.
     * @throws System::FormatException if the string is not a valid integer.
     * @throws System::OverflowException if the value exceeds Int32 range.
     */
    static SharpRuntime::intcs Parse(const std::string& s) {
        std::size_t pos = 0;
        long v;
        try {
            v = std::stol(s, &pos);
        } catch (const std::out_of_range&) {
            throw System::OverflowException("Value was either too large or too small for an Int32.");
        } catch (...) {
            throw System::FormatException("Input string was not in a correct format.");
        }
        for (; pos < s.size(); ++pos) {
            if (!std::isspace(static_cast<unsigned char>(s[pos])))
                throw System::FormatException("Input string was not in a correct format.");
        }
        if (v < MinValue || v > MaxValue)
            throw System::OverflowException("Value was either too large or too small for an Int32.");
        return static_cast<SharpRuntime::intcs>(v);
    }

    /**
     * @brief Tries to convert a string to a 32-bit signed integer without
     * throwing.
     *
     * C++ counterpart of .NET Int32.TryParse(string, out int).
     * @param s      String to parse.
     * @param result Receives the parsed value on success, or 0 on failure.
     * @return True if parsing succeeded; false otherwise.
     */
    static bool TryParse(const std::string& s, SharpRuntime::intcs& result) {
        try { result = Parse(s); return true; }
        catch (...) { result = 0; return false; }
    }

    /**
     * @brief Converts the string representation of a number in the specified style to its
     * 32-bit signed integer equivalent.
     *
     * C++ counterpart of .NET Int32.Parse(string, NumberStyles, IFormatProvider). @p provider
     * is accepted for API-surface parity but ignored (this port has no culture-aware number
     * formatting; separators/currency symbol use NumberFormatInfo.InvariantInfo's fixed
     * defaults). Supports NumberStyles.Integer, .Number, .Currency, and .HexNumber (hex is
     * reinterpreted as a two's-complement bit pattern, matching real .NET's actual semantics:
     * Parse("FFFFFFFF", NumberStyles.HexNumber) yields -1, not an overflow) -- see
     * include/System/detail/IntegerNumberStylesParser.hpp's own doc-comment for the exact
     * supported grammar and its one remaining scope gap (AllowExponent).
     * @throws System::FormatException if the string is not in a correct format for @p style.
     * @throws System::OverflowException if the value exceeds Int32 range.
     */
    static SharpRuntime::intcs Parse(const std::string& s, System::Globalization::NumberStyles style,
                                      const IFormatProvider* provider) {
        (void)provider;
        SharpRuntime::intcs result;
        if (!TryParse(s, style, provider, result)) {
            // Re-derive which failure occurred for an accurate exception type.
            using System::Globalization::NumberStyles;
            if ((style & NumberStyles::AllowHexSpecifier) != NumberStyles::None) {
                uint64_t bits; bool tooManyDigits = false;
                System::detail::IntegerNumberStylesParser::TryParseHexCore(s, style, bits, 8, tooManyDigits);
                if (tooManyDigits)
                    throw System::OverflowException("Value was either too large or too small for an Int32.");
                throw System::FormatException("Input string was not in a correct format.");
            }
            if ((style & NumberStyles::AllowBinarySpecifier) != NumberStyles::None) {
                uint64_t bits; bool tooManyDigits = false;
                System::detail::IntegerNumberStylesParser::TryParseBinaryCore(s, style, bits, 32, tooManyDigits);
                if (tooManyDigits)
                    throw System::OverflowException("Value was either too large or too small for an Int32.");
                throw System::FormatException("Input string was not in a correct format.");
            }
            SharpRuntime::longcs signedResult; bool overflowed = false;
            if (System::detail::IntegerNumberStylesParser::TryParseSignedCore(s, style, signedResult, overflowed) &&
                (overflowed || signedResult < MinValue || signedResult > MaxValue))
                throw System::OverflowException("Value was either too large or too small for an Int32.");
            throw System::FormatException("Input string was not in a correct format.");
        }
        return result;
    }

    /**
     * @brief Tries to convert a string to a 32-bit signed integer using the specified style,
     * without throwing.
     *
     * C++ counterpart of .NET Int32.TryParse(string, NumberStyles, IFormatProvider, out int).
     * @p provider is accepted for API-surface parity but ignored. See Parse(string,
     * NumberStyles, IFormatProvider) for the supported grammar subset.
     */
    static bool TryParse(const std::string& s, System::Globalization::NumberStyles style,
                          const IFormatProvider* provider, SharpRuntime::intcs& result) {
        (void)provider;
        using System::Globalization::NumberStyles;
        result = 0;
        if ((style & NumberStyles::AllowHexSpecifier) != NumberStyles::None) {
            uint64_t bits; bool tooManyDigits = false;
            if (!System::detail::IntegerNumberStylesParser::TryParseHexCore(s, style, bits, 8, tooManyDigits))
                return false;
            result = static_cast<SharpRuntime::intcs>(static_cast<uint32_t>(bits));
            return true;
        }
        if ((style & NumberStyles::AllowBinarySpecifier) != NumberStyles::None) {
            uint64_t bits; bool tooManyDigits = false;
            if (!System::detail::IntegerNumberStylesParser::TryParseBinaryCore(s, style, bits, 32, tooManyDigits))
                return false;
            result = static_cast<SharpRuntime::intcs>(static_cast<uint32_t>(bits));
            return true;
        }
        SharpRuntime::longcs signedResult; bool overflowed = false;
        if (!System::detail::IntegerNumberStylesParser::TryParseSignedCore(s, style, signedResult, overflowed))
            return false;
        if (overflowed || signedResult < MinValue || signedResult > MaxValue) return false;
        result = static_cast<SharpRuntime::intcs>(signedResult);
        return true;
    }

    // -----------------------------------------------------------------------
    // ToString
    // -----------------------------------------------------------------------

    /**
     * @brief Converts the 32-bit signed integer @p value to its decimal
     * string representation.
     *
     * C++ counterpart of .NET Int32.ToString().
     */
    static std::string ToString(SharpRuntime::intcs value) {
        return std::to_string(value);
    }

    /**
     * @brief Converts @p value to a string using the specified format specifier.
     *
     * C++ counterpart of .NET Int32.ToString(string format).
     * Supported specifiers:
     *   - @c "X" / @c "x" — hexadecimal (uppercase/lowercase), optional digit width
     *   - @c "D" / @c "d" — decimal with optional zero-padded minimum width
     *   - @c "G" / @c "g" — general (decimal)
     *   - @c "B" / @c "b" — binary, optional minimum digit width
     */
    static std::string ToString(SharpRuntime::intcs value, const std::string& format) {
        if (format.empty()) return std::to_string(value);
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
            oss << std::uppercase << std::hex;
            if (width > 0) oss << std::setfill('0') << std::setw(width);
            oss << static_cast<unsigned>(value);
            return oss.str();
        }
        if (type == 'x') {
            oss << std::hex;
            if (width > 0) oss << std::setfill('0') << std::setw(width);
            oss << static_cast<unsigned>(value);
            return oss.str();
        }
        if (type == 'D' || type == 'd') {
            if (width > 0) {
                std::string s = std::to_string(value);
                bool neg = value < 0;
                if (neg) s = s.substr(1);
                while (static_cast<int>(s.size()) < width) s = "0" + s;
                return neg ? "-" + s : s;
            }
            return std::to_string(value);
        }
        if (type == 'G' || type == 'g') return std::to_string(value);
        if (type == 'B' || type == 'b') {
            if (value == 0) {
                return std::string(width > 0 ? static_cast<size_t>(width) : 1u, '0');
            }
            std::string bits;
            uint32_t uv = static_cast<uint32_t>(value);
            while (uv > 0) { bits = static_cast<char>('0' + (uv & 1)) + bits; uv >>= 1; }
            while (static_cast<int>(bits.size()) < width) bits = "0" + bits;
            return bits;
        }
        return std::to_string(value);
    }

    // -----------------------------------------------------------------------
    // Comparison
    // -----------------------------------------------------------------------

    /**
     * @brief Compares @p a to @p b and returns a signed integer.
     *
     * C++ counterpart of .NET Int32.CompareTo(int).
     */
    [[nodiscard]] static SharpRuntime::intcs CompareTo(
            SharpRuntime::intcs a, SharpRuntime::intcs b) noexcept {
        return (a < b) ? -1 : (a > b) ? 1 : 0;
    }

    /**
     * @brief Returns true if @p a equals @p b.
     *
     * C++ counterpart of .NET Int32.Equals(int).
     */
    [[nodiscard]] static bool Equals(SharpRuntime::intcs a, SharpRuntime::intcs b) noexcept { return a == b; }

    /**
     * @brief Returns a hash code for @p value.
     *
     * C++ counterpart of .NET Int32.GetHashCode().
     */
    [[nodiscard]] static SharpRuntime::intcs GetHashCode(SharpRuntime::intcs value) noexcept { return value; }

    // -----------------------------------------------------------------------
    // Arithmetic helpers
    // -----------------------------------------------------------------------

    /**
     * @brief Produces the full 64-bit product of two 32-bit numbers.
     *
     * C++ counterpart of .NET Int32.BigMul(int, int).
     * @param left  The first number to multiply.
     * @param right The second number to multiply.
     * @return The 64-bit product.
     */
    [[nodiscard]] static SharpRuntime::longcs BigMul(
            SharpRuntime::intcs left, SharpRuntime::intcs right) noexcept {
        return static_cast<SharpRuntime::longcs>(left)
             * static_cast<SharpRuntime::longcs>(right);
    }

    /**
     * @brief Computes the quotient and remainder of two 32-bit integers.
     *
     * C++ counterpart of .NET Int32.DivRem(int, int).
     * @param left  The dividend.
     * @param right The divisor.
     * @return A pair {Quotient, Remainder}.
     * @throws System::DivideByZeroException if @p right is zero -- integer division by
     *         zero is undefined behavior in C++ (a hardware trap, not a catchable
     *         exception), unlike the CLR's div instruction which .NET surfaces as a
     *         managed DivideByZeroException; this must be checked explicitly.
     * @throws System::OverflowException if @p left is MinValue and @p right is -1
     *         (the mathematical result does not fit in Int32; the CLR's div
     *         instruction traps on this input and .NET surfaces it as OverflowException).
     */
    [[nodiscard]] static std::pair<SharpRuntime::intcs, SharpRuntime::intcs>
    DivRem(SharpRuntime::intcs left, SharpRuntime::intcs right) {
        if (right == 0)
            throw System::DivideByZeroException();
        if (left == MinValue && right == -1)
            throw System::OverflowException("Negating the minimum value of a twos complement number is invalid.");
        return { left / right, left % right };
    }

    /**
     * @brief Returns the absolute value of a 32-bit signed integer.
     *
     * C++ counterpart of .NET Int32.Abs(int).
     */
    [[nodiscard]] static SharpRuntime::intcs Abs(SharpRuntime::intcs value) {
        if (value == MinValue)
            throw System::OverflowException("Negating the minimum value of a twos complement number is invalid.");
        return value < 0 ? -value : value;
    }

    /**
     * @brief Clamps @p value to the inclusive range [@p min, @p max].
     *
     * C++ counterpart of .NET Int32.Clamp(int, int, int).
     */
    [[nodiscard]] static SharpRuntime::intcs Clamp(
            SharpRuntime::intcs value,
            SharpRuntime::intcs min,
            SharpRuntime::intcs max) noexcept {
        return std::clamp(value, min, max);
    }

    /**
     * @brief Copies the sign of @p sign to the magnitude of @p value.
     *
     * C++ counterpart of .NET Int32.CopySign(int, int).
     * Real .NET relies on unchecked negation of MinValue wrapping back to MinValue
     * (C# unchecked arithmetic is defined to wrap) to make the sign<0 branch a no-op for
     * MinValue. Negating INT32_MIN is undefined behavior in C++, so MinValue is special-cased
     * explicitly instead of relying on that wraparound.
     * @throws System::OverflowException if @p value is MinValue and @p sign is non-negative
     *         (there is no positive Int32 representation of MinValue's magnitude).
     */
    [[nodiscard]] static SharpRuntime::intcs CopySign(
            SharpRuntime::intcs value, SharpRuntime::intcs sign) {
        if (value == MinValue) {
            if (sign >= 0)
                throw System::OverflowException("Negating the minimum value of a twos complement number is invalid.");
            return MinValue;
        }
        int32_t absValue = value < 0 ? -value : value;
        return sign >= 0 ? absValue : -absValue;
    }

    /**
     * @brief Returns the larger of two 32-bit integers.
     *
     * C++ counterpart of .NET Int32.Max(int, int).
     */
    [[nodiscard]] static SharpRuntime::intcs Max(
            SharpRuntime::intcs x, SharpRuntime::intcs y) noexcept {
        return x > y ? x : y;
    }

    /**
     * @brief Returns the smaller of two 32-bit integers.
     *
     * C++ counterpart of .NET Int32.Min(int, int).
     */
    [[nodiscard]] static SharpRuntime::intcs Min(
            SharpRuntime::intcs x, SharpRuntime::intcs y) noexcept {
        return x < y ? x : y;
    }

    /**
     * @brief Returns the integer with the larger absolute value.
     *
     * C++ counterpart of .NET Int32.MaxMagnitude(int, int).
     * If both have equal magnitude, returns the positive one. MinValue has no
     * representable positive magnitude, so it always wins (matching .NET, which
     * special-cases it rather than comparing a wrapped-around magnitude).
     */
    [[nodiscard]] static SharpRuntime::intcs MaxMagnitude(
            SharpRuntime::intcs x, SharpRuntime::intcs y) noexcept {
        if (x == MinValue) return x;
        if (y == MinValue) return y;
        int32_t ax = x < 0 ? -x : x;
        int32_t ay = y < 0 ? -y : y;
        if (ax > ay) return x;
        if (ax == ay) return IsNegative(x) ? y : x;
        return y;
    }

    /**
     * @brief Returns the integer with the smaller absolute value.
     *
     * C++ counterpart of .NET Int32.MinMagnitude(int, int).
     * If both have equal magnitude, returns the negative one. MinValue has no
     * representable positive magnitude, so it always loses (matching .NET, which
     * special-cases it rather than comparing a wrapped-around magnitude).
     */
    [[nodiscard]] static SharpRuntime::intcs MinMagnitude(
            SharpRuntime::intcs x, SharpRuntime::intcs y) noexcept {
        if (x == MinValue) return y;
        if (y == MinValue) return x;
        int32_t ax = x < 0 ? -x : x;
        int32_t ay = y < 0 ? -y : y;
        if (ax < ay) return x;
        if (ax == ay) return IsNegative(x) ? x : y;
        return y;
    }

    /**
     * @brief Returns a value indicating the sign of a 32-bit signed integer.
     *
     * C++ counterpart of .NET Int32.Sign(int).
     * @return -1 if @p value is negative, 0 if zero, 1 if positive.
     */
    [[nodiscard]] static SharpRuntime::intcs Sign(
            SharpRuntime::intcs value) noexcept {
        return value < 0 ? -1 : (value > 0 ? 1 : 0);
    }

    // -----------------------------------------------------------------------
    // Bit operations
    // -----------------------------------------------------------------------

    /**
     * @brief Returns the number of leading zero bits in a 32-bit integer.
     *
     * C++ counterpart of .NET Int32.LeadingZeroCount(int).
     */
    [[nodiscard]] static SharpRuntime::intcs LeadingZeroCount(
            SharpRuntime::intcs value) noexcept {
        return static_cast<SharpRuntime::intcs>(
            std::countl_zero(static_cast<uint32_t>(value)));
    }

    /**
     * @brief Returns the number of trailing zero bits in a 32-bit integer.
     *
     * C++ counterpart of .NET Int32.TrailingZeroCount(int).
     */
    [[nodiscard]] static SharpRuntime::intcs TrailingZeroCount(
            SharpRuntime::intcs value) noexcept {
        return static_cast<SharpRuntime::intcs>(
            std::countr_zero(static_cast<uint32_t>(value)));
    }

    /**
     * @brief Returns the number of set bits (population count) in a 32-bit integer.
     *
     * C++ counterpart of .NET Int32.PopCount(int).
     */
    [[nodiscard]] static SharpRuntime::intcs PopCount(
            SharpRuntime::intcs value) noexcept {
        return static_cast<SharpRuntime::intcs>(
            std::popcount(static_cast<uint32_t>(value)));
    }

    /**
     * @brief Rotates the bits of a 32-bit integer left by @p rotateAmount positions.
     *
     * C++ counterpart of .NET Int32.RotateLeft(int, int).
     */
    [[nodiscard]] static SharpRuntime::intcs RotateLeft(
            SharpRuntime::intcs value, SharpRuntime::intcs rotateAmount) noexcept {
        return static_cast<SharpRuntime::intcs>(
            std::rotl(static_cast<uint32_t>(value),
                      static_cast<int>(rotateAmount)));
    }

    /**
     * @brief Rotates the bits of a 32-bit integer right by @p rotateAmount positions.
     *
     * C++ counterpart of .NET Int32.RotateRight(int, int).
     */
    [[nodiscard]] static SharpRuntime::intcs RotateRight(
            SharpRuntime::intcs value, SharpRuntime::intcs rotateAmount) noexcept {
        return static_cast<SharpRuntime::intcs>(
            std::rotr(static_cast<uint32_t>(value),
                      static_cast<int>(rotateAmount)));
    }

    /**
     * @brief Determines whether @p value is a power of two.
     *
     * C++ counterpart of .NET Int32.IsPow2(int).
     */
    [[nodiscard]] static bool IsPow2(SharpRuntime::intcs value) noexcept {
        return value > 0 && std::has_single_bit(static_cast<uint32_t>(value));
    }

    /**
     * @brief Returns the integer base-2 logarithm of a 32-bit integer.
     *
     * C++ counterpart of .NET Int32.Log2(int).
     * @throws System::ArgumentOutOfRangeException if @p value is negative.
     */
    [[nodiscard]] static SharpRuntime::intcs Log2(SharpRuntime::intcs value) {
        if (value < 0)
            throw System::ArgumentOutOfRangeException("value", "value must be non-negative");
        if (value == 0) return 0;
        return static_cast<SharpRuntime::intcs>(std::bit_width(static_cast<uint32_t>(value)) - 1);
    }

    // -----------------------------------------------------------------------
    // Classification predicates
    // -----------------------------------------------------------------------

    /**
     * @brief Returns true if @p value is an even integer.
     *
     * C++ counterpart of .NET Int32.IsEvenInteger(int).
     */
    [[nodiscard]] static bool IsEvenInteger(SharpRuntime::intcs value) noexcept {
        return (value & 1) == 0;
    }

    /**
     * @brief Returns true if @p value is an odd integer.
     *
     * C++ counterpart of .NET Int32.IsOddInteger(int).
     */
    [[nodiscard]] static bool IsOddInteger(SharpRuntime::intcs value) noexcept {
        return (value & 1) != 0;
    }

    /**
     * @brief Returns true if @p value is negative.
     *
     * C++ counterpart of .NET Int32.IsNegative(int).
     */
    [[nodiscard]] static bool IsNegative(SharpRuntime::intcs value) noexcept {
        return value < 0;
    }

    /**
     * @brief Returns true if @p value is zero or positive.
     *
     * C++ counterpart of .NET Int32.IsPositive(int).
     */
    [[nodiscard]] static bool IsPositive(SharpRuntime::intcs value) noexcept {
        return value >= 0;
    }
};

} // namespace System

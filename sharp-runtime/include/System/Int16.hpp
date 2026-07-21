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

/**
 * @brief Provides constants and static methods for working with 16-bit signed integers.
 *
 * C++ counterpart of .NET System.Int16.
 */
class Int16 {
public:
    /** @brief Maximum value of a 16-bit signed integer (32767). */
    static constexpr SharpRuntime::shortcs MaxValue = std::numeric_limits<int16_t>::max();
    /** @brief Minimum value of a 16-bit signed integer (-32768). */
    static constexpr SharpRuntime::shortcs MinValue = std::numeric_limits<int16_t>::min();

    /**
     * @brief Converts the string representation of a number to its 16-bit signed integer equivalent.
     * @param s String to parse.
     * @return Parsed int16 value.
     * @throws System::OverflowException if the value exceeds Int16 range.
     * @throws System::FormatException if the string is not a valid integer.
     */
    static SharpRuntime::shortcs Parse(const std::string& s) {
        std::size_t pos = 0;
        int v;
        try {
            v = std::stoi(s, &pos);
        } catch (const std::out_of_range&) {
            throw System::OverflowException("Value out of Int16 range.");
        } catch (...) {
            throw System::FormatException("Input string was not in a correct format.");
        }
        for (; pos < s.size(); ++pos) {
            if (!std::isspace(static_cast<unsigned char>(s[pos])))
                throw System::FormatException("Input string was not in a correct format.");
        }
        if (v < MinValue || v > MaxValue) throw System::OverflowException("Value out of Int16 range.");
        return static_cast<int16_t>(v);
    }

    /**
     * @brief Tries to convert a string to a 16-bit signed integer without throwing.
     * @param s String to parse.
     * @param result Receives the parsed value on success, or 0 on failure.
     * @return True if parsing succeeded; false otherwise.
     */
    static bool TryParse(const std::string& s, SharpRuntime::shortcs& result) {
        try { result = Parse(s); return true; }
        catch (...) { result = 0; return false; }
    }

    /**
     * @brief Converts the string representation of a number in the specified style to its
     * 16-bit signed integer equivalent.
     *
     * C++ counterpart of .NET Int16.Parse(string, NumberStyles, IFormatProvider). @p provider
     * is accepted for API-surface parity but ignored. Supports NumberStyles.Integer, .Number,
     * .Currency, and .HexNumber (hex reinterpreted as a two's-complement bit pattern) -- see
     * include/System/detail/IntegerNumberStylesParser.hpp for the exact supported grammar.
     * @throws System::FormatException if the string is not in a correct format for @p style.
     * @throws System::OverflowException if the value exceeds Int16 range.
     */
    static SharpRuntime::shortcs Parse(const std::string& s, System::Globalization::NumberStyles style,
                                        const IFormatProvider* provider) {
        (void)provider;
        SharpRuntime::shortcs result;
        if (!TryParse(s, style, provider, result)) {
            using System::Globalization::NumberStyles;
            if ((style & NumberStyles::AllowHexSpecifier) != NumberStyles::None) {
                uint64_t bits; bool tooManyDigits = false;
                System::detail::IntegerNumberStylesParser::TryParseHexCore(s, style, bits, 4, tooManyDigits);
                if (tooManyDigits)
                    throw System::OverflowException("Value out of Int16 range.");
                throw System::FormatException("Input string was not in a correct format.");
            }
            if ((style & NumberStyles::AllowBinarySpecifier) != NumberStyles::None) {
                uint64_t bits; bool tooManyDigits = false;
                System::detail::IntegerNumberStylesParser::TryParseBinaryCore(s, style, bits, 16, tooManyDigits);
                if (tooManyDigits)
                    throw System::OverflowException("Value out of Int16 range.");
                throw System::FormatException("Input string was not in a correct format.");
            }
            SharpRuntime::longcs signedResult; bool overflowed = false;
            if (System::detail::IntegerNumberStylesParser::TryParseSignedCore(s, style, signedResult, overflowed) &&
                (overflowed || signedResult < MinValue || signedResult > MaxValue))
                throw System::OverflowException("Value out of Int16 range.");
            throw System::FormatException("Input string was not in a correct format.");
        }
        return result;
    }

    /**
     * @brief Tries to convert a string to a 16-bit signed integer using the specified style,
     * without throwing.
     *
     * C++ counterpart of .NET Int16.TryParse(string, NumberStyles, IFormatProvider, out short).
     */
    static bool TryParse(const std::string& s, System::Globalization::NumberStyles style,
                          const IFormatProvider* provider, SharpRuntime::shortcs& result) {
        (void)provider;
        using System::Globalization::NumberStyles;
        result = 0;
        if ((style & NumberStyles::AllowHexSpecifier) != NumberStyles::None) {
            uint64_t bits; bool tooManyDigits = false;
            if (!System::detail::IntegerNumberStylesParser::TryParseHexCore(s, style, bits, 4, tooManyDigits))
                return false;
            result = static_cast<SharpRuntime::shortcs>(static_cast<uint16_t>(bits));
            return true;
        }
        if ((style & NumberStyles::AllowBinarySpecifier) != NumberStyles::None) {
            uint64_t bits; bool tooManyDigits = false;
            if (!System::detail::IntegerNumberStylesParser::TryParseBinaryCore(s, style, bits, 16, tooManyDigits))
                return false;
            result = static_cast<SharpRuntime::shortcs>(static_cast<uint16_t>(bits));
            return true;
        }
        SharpRuntime::longcs signedResult; bool overflowed = false;
        if (!System::detail::IntegerNumberStylesParser::TryParseSignedCore(s, style, signedResult, overflowed))
            return false;
        if (overflowed || signedResult < MinValue || signedResult > MaxValue) return false;
        result = static_cast<SharpRuntime::shortcs>(signedResult);
        return true;
    }

    /** @brief Converts the 16-bit signed integer @p value to its string representation. */
    static std::string ToString(SharpRuntime::shortcs value) { return std::to_string(value); }

    /** @brief Converts @p value to a string using format specifier ("X", "X4", "D", "D5", "G"). */
    static std::string ToString(SharpRuntime::shortcs value, const std::string& format) {
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
        if (type == 'X') { oss << std::uppercase << std::hex << std::setfill('0') << std::setw(width) << (static_cast<unsigned>(value) & 0xFFFFu); return oss.str(); }
        if (type == 'x') { oss << std::hex << std::setfill('0') << std::setw(width) << (static_cast<unsigned>(value) & 0xFFFFu); return oss.str(); }
        if (type == 'D' || type == 'd') {
            bool neg = value < 0;
            std::string s = std::to_string(neg ? -static_cast<int>(value) : static_cast<int>(value));
            while (static_cast<int>(s.size()) < width) s = "0" + s;
            return neg ? "-" + s : s;
        }
        if (type == 'G' || type == 'g') return ToString(value);
        return ToString(value);
    }

    /**
     * @brief Compares @p a to @p b and returns a signed integer.
     * C++ counterpart of .NET Int16.CompareTo(short).
     */
    [[nodiscard]] static SharpRuntime::intcs CompareTo(SharpRuntime::shortcs a, SharpRuntime::shortcs b) noexcept {
        return (a < b) ? -1 : (a > b) ? 1 : 0;
    }

    /** @brief Returns true if @p a equals @p b. C++ counterpart of .NET Int16.Equals(short). */
    [[nodiscard]] static bool Equals(SharpRuntime::shortcs a, SharpRuntime::shortcs b) noexcept { return a == b; }

    /** @brief Returns a hash code for @p value. C++ counterpart of .NET Int16.GetHashCode(). */
    [[nodiscard]] static SharpRuntime::intcs GetHashCode(SharpRuntime::shortcs value) noexcept { return static_cast<SharpRuntime::intcs>(value); }

    /**
     * @brief Returns the absolute value of @p value.
     * C++ counterpart of .NET Math.Abs(short).
     * @throws System::OverflowException if @p value is MinValue (its magnitude does not fit in Int16).
     */
    [[nodiscard]] static SharpRuntime::shortcs Abs(SharpRuntime::shortcs value) {
        if (value == MinValue) throw System::OverflowException("Negating the minimum value of a twos complement number is invalid.");
        return value < 0 ? static_cast<SharpRuntime::shortcs>(-value) : value;
    }

    /**
     * @brief Returns a value with the magnitude of @p value and the sign of @p sign.
     * C++ counterpart of .NET Int16.CopySign(short, short) -- added 2026-07-14
     * (duplicated-implementation audit finding: Int16 was the sole signed integer type
     * missing this and the four methods below, mirrored here from SByte's identical
     * implementation).
     * @throws System::OverflowException if @p value is MinValue and @p sign is non-negative
     *         (its magnitude does not fit in a signed Int16).
     */
    [[nodiscard]] static SharpRuntime::shortcs CopySign(SharpRuntime::shortcs value, SharpRuntime::shortcs sign) {
        SharpRuntime::shortcs abs = value < 0 ? (value == MinValue ? value : static_cast<SharpRuntime::shortcs>(-value)) : value;
        if (sign >= 0) {
            if (abs < 0) throw System::OverflowException("Negating MinValue is not representable.");
            return abs;
        }
        return static_cast<SharpRuntime::shortcs>(-abs);
    }

    /** @brief Returns true if @p value is negative. C++ counterpart of .NET Int16.IsNegative(short). */
    [[nodiscard]] static bool IsNegative(SharpRuntime::shortcs value) noexcept { return value < 0; }

    /** @brief Returns true if @p value is positive (> 0). C++ counterpart of .NET Int16.IsPositive(short). */
    [[nodiscard]] static bool IsPositive(SharpRuntime::shortcs value) noexcept { return value > 0; }

    /**
     * @brief Returns the value with greater magnitude; if magnitudes are equal, returns @p x.
     *
     * C++ counterpart of .NET Int16.MaxMagnitude(short, short). MinValue has no
     * representable positive magnitude, so it always wins, matching .NET (which detects
     * this via a wrapped-negation check; here via a direct equality check to avoid
     * relying on signed-overflow behavior).
     */
    [[nodiscard]] static SharpRuntime::shortcs MaxMagnitude(SharpRuntime::shortcs x, SharpRuntime::shortcs y) noexcept {
        if (x == MinValue) return x;
        if (y == MinValue) return y;
        SharpRuntime::shortcs ax = x < 0 ? static_cast<SharpRuntime::shortcs>(-x) : x;
        SharpRuntime::shortcs ay = y < 0 ? static_cast<SharpRuntime::shortcs>(-y) : y;
        return ax >= ay ? x : y;
    }

    /**
     * @brief Returns the value with smaller magnitude; if magnitudes are equal, returns @p x.
     *
     * C++ counterpart of .NET Int16.MinMagnitude(short, short). MinValue has no
     * representable positive magnitude, so it always loses, matching .NET.
     */
    [[nodiscard]] static SharpRuntime::shortcs MinMagnitude(SharpRuntime::shortcs x, SharpRuntime::shortcs y) noexcept {
        if (x == MinValue) return y;
        if (y == MinValue) return x;
        SharpRuntime::shortcs ax = x < 0 ? static_cast<SharpRuntime::shortcs>(-x) : x;
        SharpRuntime::shortcs ay = y < 0 ? static_cast<SharpRuntime::shortcs>(-y) : y;
        return ax <= ay ? x : y;
    }

    /** @brief Clamps @p value to [@p min, @p max]. C++ counterpart of .NET Int16.Clamp(short,short,short). */
    [[nodiscard]] static SharpRuntime::shortcs Clamp(SharpRuntime::shortcs value, SharpRuntime::shortcs min, SharpRuntime::shortcs max) noexcept {
        return std::clamp(value, min, max);
    }

    /** @brief Returns the larger of @p x and @p y. C++ counterpart of .NET Int16.Max(short,short). */
    [[nodiscard]] static SharpRuntime::shortcs Max(SharpRuntime::shortcs x, SharpRuntime::shortcs y) noexcept { return x > y ? x : y; }

    /** @brief Returns the smaller of @p x and @p y. C++ counterpart of .NET Int16.Min(short,short). */
    [[nodiscard]] static SharpRuntime::shortcs Min(SharpRuntime::shortcs x, SharpRuntime::shortcs y) noexcept { return x < y ? x : y; }

    /** @brief Returns -1 if negative, 0 if zero, 1 if positive. C++ counterpart of .NET Math.Sign(short). */
    [[nodiscard]] static SharpRuntime::intcs Sign(SharpRuntime::shortcs value) noexcept {
        return (value > 0) - (value < 0);
    }

    /**
     * @brief Returns the quotient and remainder of @p left / @p right.
     * C++ counterpart of .NET Int16.DivRem(short,short).
     * @throws System::DivideByZeroException if @p right is zero -- integer division by
     *         zero is undefined behavior in C++ (a hardware trap, not a catchable
     *         exception), unlike the CLR's div instruction which .NET surfaces as a
     *         managed DivideByZeroException; this must be checked explicitly. No
     *         MinValue/-1 overflow check is needed: short operands promote to int in
     *         C++ arithmetic (matching the CLR's int32-width IL arithmetic for short),
     *         so short.MinValue/-1 does not overflow at the width the division runs at.
     */
    [[nodiscard]] static std::pair<SharpRuntime::shortcs, SharpRuntime::shortcs> DivRem(SharpRuntime::shortcs left, SharpRuntime::shortcs right) {
        if (right == 0)
            throw System::DivideByZeroException();
        return {static_cast<SharpRuntime::shortcs>(left / right), static_cast<SharpRuntime::shortcs>(left % right)};
    }

    /** @brief Returns true when @p value is even. C++ counterpart of .NET Int16.IsEvenInteger(short). */
    [[nodiscard]] static bool IsEvenInteger(SharpRuntime::shortcs value) noexcept { return (value & 1) == 0; }

    /** @brief Returns true when @p value is odd. C++ counterpart of .NET Int16.IsOddInteger(short). */
    [[nodiscard]] static bool IsOddInteger(SharpRuntime::shortcs value) noexcept { return (value & 1) != 0; }

    /** @brief Returns true when @p value is a power of two. C++ counterpart of .NET Int16.IsPow2(short). */
    [[nodiscard]] static bool IsPow2(SharpRuntime::shortcs value) noexcept {
        return value > 0 && (value & (value - 1)) == 0;
    }

    /**
     * @brief Returns the number of leading zero bits (of the 16-bit value).
     * C++ counterpart of .NET Int16.LeadingZeroCount(short).
     * @note Real .NET's IBinaryInteger&lt;short&gt;.LeadingZeroCount returns @c short (not
     * @c int, unlike the wider integer types) -- matched here via @c shortcs rather than
     * @c intcs. The max possible result (16) fits comfortably.
     */
    [[nodiscard]] static SharpRuntime::shortcs LeadingZeroCount(SharpRuntime::shortcs value) noexcept {
        return static_cast<SharpRuntime::shortcs>(std::countl_zero(static_cast<uint16_t>(value)));
    }

    /**
     * @brief Returns the number of set bits.
     * C++ counterpart of .NET Int16.PopCount(short).
     * @note Real .NET's IBinaryInteger&lt;short&gt;.PopCount returns @c short (not @c int).
     */
    [[nodiscard]] static SharpRuntime::shortcs PopCount(SharpRuntime::shortcs value) noexcept {
        return static_cast<SharpRuntime::shortcs>(std::popcount(static_cast<uint16_t>(value)));
    }

    /**
     * @brief Returns the number of trailing zero bits (of the 16-bit value).
     * C++ counterpart of .NET Int16.TrailingZeroCount(short).
     * @note Real .NET's IBinaryInteger&lt;short&gt;.TrailingZeroCount returns @c short (not
     * @c int).
     */
    [[nodiscard]] static SharpRuntime::shortcs TrailingZeroCount(SharpRuntime::shortcs value) noexcept {
        if (value == 0) return 16;
        return static_cast<SharpRuntime::shortcs>(std::countr_zero(static_cast<uint16_t>(value)));
    }

    /** @brief Rotates @p value left by @p rotateAmount bits (within 16 bits). C++ counterpart of .NET Int16.RotateLeft(short,int). */
    [[nodiscard]] static SharpRuntime::shortcs RotateLeft(SharpRuntime::shortcs value, SharpRuntime::intcs rotateAmount) noexcept {
        return static_cast<SharpRuntime::shortcs>(
            std::rotl(static_cast<uint16_t>(value), rotateAmount));
    }

    /** @brief Rotates @p value right by @p rotateAmount bits (within 16 bits). C++ counterpart of .NET Int16.RotateRight(short,int). */
    [[nodiscard]] static SharpRuntime::shortcs RotateRight(SharpRuntime::shortcs value, SharpRuntime::intcs rotateAmount) noexcept {
        return static_cast<SharpRuntime::shortcs>(
            std::rotr(static_cast<uint16_t>(value), rotateAmount));
    }

    /**
     * @brief Returns the floor of the base-2 logarithm of @p value.
     * C++ counterpart of .NET Int16.Log2(short). Matches .NET: Log2(0) is 0, not an error.
     * @note Real .NET's IBinaryNumber&lt;short&gt;.Log2 returns @c short (not @c int).
     * @throws System::ArgumentOutOfRangeException if @p value is negative.
     */
    [[nodiscard]] static SharpRuntime::shortcs Log2(SharpRuntime::shortcs value) {
        if (value < 0) throw System::ArgumentOutOfRangeException("value", "value must be non-negative");
        if (value == 0) return 0;
        return static_cast<SharpRuntime::shortcs>(std::bit_width(static_cast<uint16_t>(value)) - 1);
    }
};

} // namespace System

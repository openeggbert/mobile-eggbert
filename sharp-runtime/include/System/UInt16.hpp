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

    using SharpRuntime::intcs;

    /**
     * @brief Provides static helper methods that mirror .NET System.UInt16.
     *
     * Represents a 16-bit unsigned integer (0 to 65535).
     */
    class UInt16 {
    public:
        /** @brief Maximum value of a 16-bit unsigned integer (65535). */
        static constexpr SharpRuntime::ushortcs MaxValue = std::numeric_limits<uint16_t>::max();
        /** @brief Minimum value of a 16-bit unsigned integer (0). */
        static constexpr SharpRuntime::ushortcs MinValue = 0;

        /**
         * @brief Converts the string representation of a number to its UInt16 equivalent.
         *
         * C++ counterpart of .NET UInt16.Parse(string) with NumberStyles.Integer: leading/
         * trailing whitespace and a leading sign are tolerated, but a leading '-' always
         * overflows (unsigned types reject any negative sign, even "-0" -- verified against
         * real .NET's Number.Parsing.cs: `(!TInteger.IsSigned && number.IsNegative)` is an
         * overflow condition checked independent of magnitude) and any trailing non-whitespace
         * character is rejected. This previously called std::stoul(s) without capturing the
         * parse-end position (so trailing garbage like "5abc" was silently accepted) and
         * without rejecting a leading '-' explicitly -- the latter was *usually* still caught
         * because std::stoul("-1") wraps to a huge unsigned long that exceeds UInt16::MaxValue,
         * but "-0" wraps to a literal 0 (confirmed via a standalone repro), which is <=
         * MaxValue and so silently succeeded instead of throwing.
         * @throws System::OverflowException if the value exceeds UInt16 range, or the string
         *         has a leading '-'.
         * @throws System::FormatException if the string is not a valid integer.
         */
        static SharpRuntime::ushortcs Parse(const std::string& s) {
            std::size_t start = s.find_first_not_of(" \t\n\r\f\v");
            if (start != std::string::npos && s[start] == '-')
                throw System::OverflowException("Value was either too large or too small for a UInt16.");
            std::size_t pos = 0;
            unsigned long v;
            try {
                v = std::stoul(s, &pos);
            } catch (const std::out_of_range&) {
                throw System::OverflowException("Value was either too large or too small for a UInt16.");
            } catch (...) {
                throw System::FormatException("Input string was not in a correct format.");
            }
            for (; pos < s.size(); ++pos) {
                if (!std::isspace(static_cast<unsigned char>(s[pos])))
                    throw System::FormatException("Input string was not in a correct format.");
            }
            if (v > MaxValue) throw System::OverflowException("Value was either too large or too small for a UInt16.");
            return static_cast<uint16_t>(v);
        }

        /**
         * @brief Tries to convert a string to a UInt16 without throwing.
         * @param result Receives the parsed value on success, or 0 on failure.
         * @return true if parsing succeeded; false otherwise.
         */
        static bool TryParse(const std::string& s, SharpRuntime::ushortcs& result) {
            try { result = Parse(s); return true; }
            catch (...) { result = 0; return false; }
        }

        /**
         * @brief Converts the string representation of a number in the specified style to its
         * UInt16 equivalent.
         *
         * C++ counterpart of .NET UInt16.Parse(string, NumberStyles, IFormatProvider). @p
         * provider is accepted for API-surface parity but ignored. Supports
         * NumberStyles.Integer, .Number, .Currency, and .HexNumber -- see
         * include/System/detail/IntegerNumberStylesParser.hpp for the exact supported grammar.
         * @throws System::FormatException if the string is not in a correct format for @p style.
         * @throws System::OverflowException if the value exceeds UInt16 range.
         */
        static SharpRuntime::ushortcs Parse(const std::string& s, System::Globalization::NumberStyles style,
                                             const IFormatProvider* provider) {
            (void)provider;
            SharpRuntime::ushortcs result;
            if (!TryParse(s, style, provider, result)) {
                using System::Globalization::NumberStyles;
                if ((style & NumberStyles::AllowHexSpecifier) != NumberStyles::None) {
                    uint64_t bits; bool tooManyDigits = false;
                    System::detail::IntegerNumberStylesParser::TryParseHexCore(s, style, bits, 4, tooManyDigits);
                    if (tooManyDigits)
                        throw System::OverflowException("Value was either too large or too small for a UInt16.");
                    throw System::FormatException("Input string was not in a correct format.");
                }
                if ((style & NumberStyles::AllowBinarySpecifier) != NumberStyles::None) {
                    uint64_t bits; bool tooManyDigits = false;
                    System::detail::IntegerNumberStylesParser::TryParseBinaryCore(s, style, bits, 16, tooManyDigits);
                    if (tooManyDigits)
                        throw System::OverflowException("Value was either too large or too small for a UInt16.");
                    throw System::FormatException("Input string was not in a correct format.");
                }
                SharpRuntime::ulongcs unsignedResult; bool overflowed = false;
                if (System::detail::IntegerNumberStylesParser::TryParseUnsignedCore(s, style, unsignedResult, overflowed) &&
                    (overflowed || unsignedResult > MaxValue))
                    throw System::OverflowException("Value was either too large or too small for a UInt16.");
                throw System::FormatException("Input string was not in a correct format.");
            }
            return result;
        }

        /**
         * @brief Tries to convert a string to a UInt16 using the specified style, without
         * throwing.
         *
         * C++ counterpart of .NET UInt16.TryParse(string, NumberStyles, IFormatProvider, out ushort).
         */
        static bool TryParse(const std::string& s, System::Globalization::NumberStyles style,
                              const IFormatProvider* provider, SharpRuntime::ushortcs& result) {
            (void)provider;
            using System::Globalization::NumberStyles;
            result = 0;
            if ((style & NumberStyles::AllowHexSpecifier) != NumberStyles::None) {
                uint64_t bits; bool tooManyDigits = false;
                if (!System::detail::IntegerNumberStylesParser::TryParseHexCore(s, style, bits, 4, tooManyDigits))
                    return false;
                result = static_cast<SharpRuntime::ushortcs>(bits);
                return true;
            }
            if ((style & NumberStyles::AllowBinarySpecifier) != NumberStyles::None) {
                uint64_t bits; bool tooManyDigits = false;
                if (!System::detail::IntegerNumberStylesParser::TryParseBinaryCore(s, style, bits, 16, tooManyDigits))
                    return false;
                result = static_cast<SharpRuntime::ushortcs>(bits);
                return true;
            }
            SharpRuntime::ulongcs unsignedResult; bool overflowed = false;
            if (!System::detail::IntegerNumberStylesParser::TryParseUnsignedCore(s, style, unsignedResult, overflowed))
                return false;
            if (overflowed || unsignedResult > MaxValue) return false;
            result = static_cast<SharpRuntime::ushortcs>(unsignedResult);
            return true;
        }

        /** @brief Converts the value to its decimal string representation. */
        static std::string ToString(SharpRuntime::ushortcs value) { return std::to_string(value); }

        /** @brief Converts value to a string using a format specifier ("X", "X4", "D", "D5", "G"). */
        static std::string ToString(SharpRuntime::ushortcs value, const std::string& format) {
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
            unsigned uv = static_cast<unsigned>(value);
            std::ostringstream oss;
            oss.imbue(std::locale::classic());
            if (type == 'X') { oss << std::uppercase << std::hex << std::setfill('0') << std::setw(width) << uv; return oss.str(); }
            if (type == 'x') { oss << std::hex << std::setfill('0') << std::setw(width) << uv; return oss.str(); }
            if (type == 'D' || type == 'd') {
                std::string s = std::to_string(uv);
                while (static_cast<int>(s.size()) < width) s = "0" + s;
                return s;
            }
            if (type == 'G' || type == 'g') return ToString(value);
            return ToString(value);
        }

        /** @brief Compares this value to another UInt16. Returns negative, zero, or positive. */
        static intcs CompareTo(SharpRuntime::ushortcs a, SharpRuntime::ushortcs b) {
            return (a < b) ? -1 : (a > b) ? 1 : 0;
        }

        /** @brief Returns true if the two values are equal. */
        static bool Equals(SharpRuntime::ushortcs a, SharpRuntime::ushortcs b) { return a == b; }

        /** @brief Returns a hash code for the value. */
        static intcs GetHashCode(SharpRuntime::ushortcs value) { return static_cast<intcs>(value); }

        /** @brief Returns the larger of two UInt16 values. */
        static SharpRuntime::ushortcs Max(SharpRuntime::ushortcs x, SharpRuntime::ushortcs y) {
            return std::max(x, y);
        }

        /** @brief Returns the smaller of two UInt16 values. */
        static SharpRuntime::ushortcs Min(SharpRuntime::ushortcs x, SharpRuntime::ushortcs y) {
            return std::min(x, y);
        }

        /** @brief Clamps value to the inclusive range [min, max]. */
        static SharpRuntime::ushortcs Clamp(SharpRuntime::ushortcs value,
                                            SharpRuntime::ushortcs min,
                                            SharpRuntime::ushortcs max) {
            return std::clamp(value, min, max);
        }

        /** @brief Returns 0 if value is zero; 1 otherwise. */
        static intcs Sign(SharpRuntime::ushortcs value) { return value == 0 ? 0 : 1; }

        /**
         * @brief Divides left by right and returns a (quotient, remainder) pair.
         * @throws System::DivideByZeroException if @p right is zero -- integer division
         *         by zero is undefined behavior in C++ (a hardware trap, not a catchable
         *         exception), unlike the CLR's div instruction which .NET surfaces as a
         *         managed DivideByZeroException; this must be checked explicitly.
         */
        static std::pair<SharpRuntime::ushortcs, SharpRuntime::ushortcs>
        DivRem(SharpRuntime::ushortcs left, SharpRuntime::ushortcs right) {
            if (right == 0)
                throw System::DivideByZeroException();
            return { static_cast<uint16_t>(left / right), static_cast<uint16_t>(left % right) };
        }

        /** @brief Returns true if value is an even integer. */
        static bool IsEvenInteger(SharpRuntime::ushortcs value) { return (value & 1) == 0; }

        /** @brief Returns true if value is an odd integer. */
        static bool IsOddInteger(SharpRuntime::ushortcs value) { return (value & 1) != 0; }

        /** @brief Returns true if value is a power of two. */
        static bool IsPow2(SharpRuntime::ushortcs value) {
            return value != 0 && (value & (value - 1)) == 0;
        }

        /**
         * @brief Returns the floor log base 2 of value.
         * Matches .NET's explicit "0 -> 0" contract: Log2(0) returns 0, it does not throw
         * or wrap around (bit_width(0) - 1 would otherwise underflow to 65535).
         */
        static SharpRuntime::ushortcs Log2(SharpRuntime::ushortcs value) {
            if (value == 0) return 0;
            return static_cast<uint16_t>(std::bit_width(static_cast<unsigned>(value)) - 1);
        }

        /** @brief Returns the number of leading zero bits in the 16-bit representation. */
        static SharpRuntime::ushortcs LeadingZeroCount(SharpRuntime::ushortcs value) {
            return static_cast<uint16_t>(std::countl_zero(static_cast<uint32_t>(value)) - 16);
        }

        /** @brief Returns the number of trailing zero bits. */
        static SharpRuntime::ushortcs TrailingZeroCount(SharpRuntime::ushortcs value) {
            if (value == 0) return 16;
            return static_cast<uint16_t>(std::countr_zero(static_cast<uint32_t>(value)));
        }

        /** @brief Returns the number of set bits (population count). */
        static SharpRuntime::ushortcs PopCount(SharpRuntime::ushortcs value) {
            return static_cast<uint16_t>(std::popcount(static_cast<uint32_t>(value)));
        }

        /** @brief Rotates value left by rotateAmount bits within a 16-bit field. */
        static SharpRuntime::ushortcs RotateLeft(SharpRuntime::ushortcs value, intcs rotateAmount) {
            rotateAmount &= 15;
            return static_cast<uint16_t>((value << rotateAmount) | (value >> (16 - rotateAmount)));
        }

        /** @brief Rotates value right by rotateAmount bits within a 16-bit field. */
        static SharpRuntime::ushortcs RotateRight(SharpRuntime::ushortcs value, intcs rotateAmount) {
            rotateAmount &= 15;
            return static_cast<uint16_t>((value >> rotateAmount) | (value << (16 - rotateAmount)));
        }
    };

} // namespace System

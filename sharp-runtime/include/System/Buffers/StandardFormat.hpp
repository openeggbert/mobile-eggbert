// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <cstdint>
#include <stdexcept>
#include <string>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/FormatException.hpp"

namespace System::Buffers {

    using SharpRuntime::intcs;

    /**
     * @brief Defines the format to use when formatting primitive values into text.
     *
     * C++ counterpart of .NET System.Buffers.StandardFormat.
     * A StandardFormat consists of a format symbol character (e.g. 'G', 'D', 'X')
     * and an optional precision value (0–99).
     */
    class StandardFormat {
        uint8_t format_    = 0;
        uint8_t precision_ = 0;

    public:
        /** @brief Sentinel value indicating that no precision is specified. */
        static constexpr uint8_t NoPrecision = 255;
        /** @brief Maximum allowed precision value. */
        static constexpr uint8_t MaxPrecision = 99;

        /**
         * @brief Constructs a default StandardFormat (Symbol='\0', Precision=0).
         *
         * C++ counterpart of .NET's true zero-initialized `default(StandardFormat)`
         * struct value — NOT the same as explicitly passing NoPrecision. This
         * matches .NET exactly: default(StandardFormat).HasPrecision is actually
         * true (Precision==0), and IsDefault checks for this same all-zero state.
         */
        StandardFormat() = default;
        /**
         * @brief Constructs a StandardFormat with the given format symbol and optional precision.
         * @param symbol    The format character (e.g. 'G', 'D', 'X', 'F').
         * @param precision The precision value (0–99), or NoPrecision if not specified.
         * @throws ArgumentOutOfRangeException if precision is not NoPrecision and exceeds MaxPrecision.
         */
        explicit StandardFormat(char symbol, uint8_t precision = NoPrecision)
            : format_(static_cast<uint8_t>(symbol)), precision_(precision) {
            if (precision != NoPrecision && precision > MaxPrecision)
                throw ArgumentOutOfRangeException("precision", "Precision cannot be larger than 99.");
        }

        /** @brief Returns the format symbol character. */
        [[nodiscard]] char    getSymbolProperty()       const { return static_cast<char>(format_); }
        /** @brief Returns the precision value, or NoPrecision if not set. */
        [[nodiscard]] uint8_t getPrecisionProperty()    const { return precision_; }
        /** @brief Returns true if a precision value has been specified. */
        [[nodiscard]] bool    getHasPrecisionProperty() const { return precision_ != NoPrecision; }
        /** @brief Returns true if this is the default (zero symbol, no precision) StandardFormat. */
        [[nodiscard]] bool    getIsDefaultProperty()    const { return format_ == 0 && precision_ == 0; }

        /** @brief Returns true if two StandardFormat values are equal. */
        bool operator==(const StandardFormat& o) const { return format_ == o.format_ && precision_ == o.precision_; }
        /** @brief Returns true if two StandardFormat values are not equal. */
        bool operator!=(const StandardFormat& o) const { return !(*this == o); }

        /** @brief Returns true if @p other has the same Symbol and Precision. C++ counterpart of .NET StandardFormat.Equals(StandardFormat). */
        [[nodiscard]] bool Equals(const StandardFormat& other) const { return *this == other; }

        /**
         * @brief Returns a hash code combining Symbol and Precision.
         * C++ counterpart of .NET StandardFormat.GetHashCode() (_format.GetHashCode() ^ _precision.GetHashCode(),
         * where .NET's byte.GetHashCode() is just the byte's value widened to int).
         */
        [[nodiscard]] intcs GetHashCode() const {
            return static_cast<intcs>(format_) ^ static_cast<intcs>(precision_);
        }

        /**
         * @brief Returns a string representation of this format, e.g. "G" or "D3".
         */
        [[nodiscard]] std::string ToString() const {
            std::string s(1, static_cast<char>(format_));
            if (precision_ != NoPrecision) s += std::to_string(precision_);
            return s;
        }

        /**
         * @brief Tries to parse a format string into a StandardFormat value.
         *
         * C++ counterpart of .NET StandardFormat.TryParse(ReadOnlySpan&lt;char&gt;, out StandardFormat).
         * @param format A format string such as "G", "D3", or "F2".
         * @param result Receives the parsed value on success.
         * @return true on success; false if the precision digits are invalid or too large.
         */
        static bool TryParse(const std::string& format, StandardFormat& result) {
            result = StandardFormat();
            if (format.empty()) return true;

            char symbol = format[0];
            uint8_t precision = NoPrecision;
            if (format.size() > 1) {
                uint32_t parsed = 0;
                for (std::size_t i = 1; i < format.size(); ++i) {
                    char c = format[i];
                    if (c < '0' || c > '9') return false;
                    parsed = parsed * 10 + static_cast<uint32_t>(c - '0');
                    if (parsed > MaxPrecision) return false;
                }
                precision = static_cast<uint8_t>(parsed);
            }
            result = StandardFormat(symbol, precision);
            return true;
        }

        /**
         * @brief Parses a format string into a StandardFormat value.
         * @param format A format string such as "G", "D3", or "F2".
         * @throws System::FormatException if the precision digits are invalid or too large.
         */
        static StandardFormat Parse(const std::string& format) {
            StandardFormat result;
            if (!TryParse(format, result))
                throw FormatException("Characters following the format symbol must be a number of 99 or less.");
            return result;
        }
    };

} // namespace System::Buffers

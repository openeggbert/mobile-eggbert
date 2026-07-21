// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <optional>
#include <string>
#include "System/DateTimeOffset.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Net::Http::Headers {

    /**
     * @brief Represents a value of the Warning header (e.g. '110 anderson/1.3.37 "Response is stale"').
     *
     * C++ counterpart of .NET System.Net.Http.Headers.WarningHeaderValue.
     *
     * @note Agent validation is the same practical host/token approximation used by
     * ViaHeaderValue::ReceivedBy (see its class doc note). Date parses via a small local RFC 1123
     * parser (see ContentDispositionHeaderValue's class doc note for why
     * System::DateTimeOffset::TryParse can't be used directly here).
     * @note .NET's internal GetWarningLength parsing helper is not ported. ICloneable is not
     * ported (no reflection-based virtual-clone story, per NEXT.md).
     */
    class WarningHeaderValue {
        SharpRuntime::intcs code_ = 0;
        std::string agent_;
        std::string text_; ///< Includes the surrounding quotes, e.g. "\"Response is stale\"".
        std::optional<System::DateTimeOffset> date_;

    public:
        /**
         * @brief Constructs a value with no date.
         * @param code A 3-digit warning code in [0, 999].
         * @param agent The warning agent (a host or token).
         * @param text The warning text, given as a quoted-string including its surrounding quotes.
         * @throws std::out_of_range if @p code is outside [0, 999].
         * @throws System::ArgumentException if @p agent is empty.
         * @throws System::FormatException if @p agent is not a valid token/host-like string, or
         * @p text is not a valid quoted-string.
         */
        WarningHeaderValue(SharpRuntime::intcs code, const std::string& agent, const std::string& text);

        /** @brief Constructs a value with a date. */
        WarningHeaderValue(SharpRuntime::intcs code, const std::string& agent, const std::string& text, System::DateTimeOffset date);

        /** @return The 3-digit warning code. */
        [[nodiscard]] SharpRuntime::intcs getCodeProperty() const { return code_; }
        /** @return The warning agent. */
        [[nodiscard]] const std::string& getAgentProperty() const { return agent_; }
        /** @return The warning text, including its surrounding quotes. */
        [[nodiscard]] const std::string& getTextProperty() const { return text_; }
        /** @return The warning date, or empty if not specified. */
        [[nodiscard]] std::optional<System::DateTimeOffset> getDateProperty() const { return date_; }

        /** @return "code agent text[ \"date\"]", with code zero-padded to 3 digits. */
        [[nodiscard]] std::string ToString() const;

        /** Equality: Code/Date compared exactly; Agent case-insensitive; Text case-sensitive (it's a quoted-string). */
        [[nodiscard]] bool Equals(const WarningHeaderValue& other) const;
        bool operator==(const WarningHeaderValue& o) const { return Equals(o); }
        bool operator!=(const WarningHeaderValue& o) const { return !Equals(o); }

        /** @return A hash code consistent with Equals(). */
        [[nodiscard]] SharpRuntime::intcs GetHashCode() const;

        /**
         * @brief Parses 'code agent "text"[ "date"]'.
         * @throws System::FormatException if @p input is not valid.
         */
        [[nodiscard]] static WarningHeaderValue Parse(const std::string& input);

        /** Attempts to parse @p input; returns false without throwing on failure. */
        [[nodiscard]] static bool TryParse(const std::string& input, WarningHeaderValue& parsedValue);

    private:
        WarningHeaderValue() = default;
    };

} // namespace System::Net::Http::Headers

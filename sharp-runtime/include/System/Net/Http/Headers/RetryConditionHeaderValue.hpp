// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <optional>
#include <string>
#include "System/DateTimeOffset.hpp"
#include "System/TimeSpan.hpp"

namespace System::Net::Http::Headers {

    /**
     * @brief Represents the value of the Retry-After header: either a date or a delta in
     * seconds, mutually exclusive.
     *
     * C++ counterpart of .NET System.Net.Http.Headers.RetryConditionHeaderValue.
     *
     * @note Date parses via a small local RFC 1123 parser (see ContentDispositionHeaderValue's
     * class doc note for why System::DateTimeOffset::TryParse can't be used directly).
     * @note .NET's internal GetRetryConditionLength parsing helper is not ported. ICloneable is
     * not ported (no reflection-based virtual-clone story, per NEXT.md).
     */
    class RetryConditionHeaderValue {
        std::optional<System::DateTimeOffset> date_;
        std::optional<System::TimeSpan> delta_;

        RetryConditionHeaderValue() = default;

    public:
        /** @brief Constructs a date-based value. */
        explicit RetryConditionHeaderValue(const System::DateTimeOffset& date);
        /**
         * @brief Constructs a delta-based value.
         * @throws std::out_of_range if @p delta's total seconds exceeds INT32_MAX.
         */
        explicit RetryConditionHeaderValue(const System::TimeSpan& delta);

        /** @return The date, or empty if this value is delta-based. */
        [[nodiscard]] std::optional<System::DateTimeOffset> getDateProperty() const { return date_; }
        /** @return The delta, or empty if this value is date-based. */
        [[nodiscard]] std::optional<System::TimeSpan> getDeltaProperty() const { return delta_; }

        /** @return The delta's whole seconds as a string, or the date formatted as RFC 1123. */
        [[nodiscard]] std::string ToString() const;

        /** Equality: compares whichever of Date/Delta is populated. */
        [[nodiscard]] bool Equals(const RetryConditionHeaderValue& other) const;
        bool operator==(const RetryConditionHeaderValue& o) const { return Equals(o); }
        bool operator!=(const RetryConditionHeaderValue& o) const { return !Equals(o); }

        /** @return A hash code consistent with Equals(). */
        [[nodiscard]] SharpRuntime::intcs GetHashCode() const;

        /**
         * @brief Parses a non-negative integer (seconds delta) or an RFC 1123 date.
         * @throws System::FormatException if @p input is not valid.
         */
        [[nodiscard]] static RetryConditionHeaderValue Parse(const std::string& input);

        /** Attempts to parse @p input; returns false without throwing on failure. */
        [[nodiscard]] static bool TryParse(const std::string& input, RetryConditionHeaderValue& parsedValue);
    };

} // namespace System::Net::Http::Headers

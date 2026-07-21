// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <optional>
#include <string>
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Net::Http::Headers {

    /**
     * @brief Represents a header value with an optional quality factor (e.g. for Accept-Encoding,
     * Accept-Language: "gzip; q=0.8").
     *
     * C++ counterpart of .NET System.Net.Http.Headers.StringWithQualityHeaderValue.
     *
     * @note .NET's internal GetStringWithQualityLength parsing helper is not ported; only the
     * public surface is. ICloneable is not ported (no reflection-based virtual-clone story, per
     * NEXT.md).
     */
    class StringWithQualityHeaderValue {
        std::string value_;
        std::optional<double> quality_;

    public:
        /**
         * @brief Constructs a value with no quality factor.
         * @throws System::ArgumentException if @p value is empty.
         * @throws System::FormatException if @p value is not a valid HTTP token.
         */
        explicit StringWithQualityHeaderValue(const std::string& value);

        /**
         * @brief Constructs a value with the given quality factor.
         * @throws System::ArgumentException if @p value is empty.
         * @throws System::FormatException if @p value is not a valid HTTP token.
         * @throws System::ArgumentOutOfRangeException if @p quality is outside [0.0, 1.0].
         */
        StringWithQualityHeaderValue(const std::string& value, double quality);

        /** @return The value (e.g. "gzip"). */
        [[nodiscard]] const std::string& getValueProperty() const { return value_; }
        /** @return The quality factor in [0.0, 1.0], or empty if not set. */
        [[nodiscard]] std::optional<double> getQualityProperty() const { return quality_; }

        /** @return "value; q=0.###" if a quality factor is set, otherwise just "value". */
        [[nodiscard]] std::string ToString() const;

        /** Equality: Value is case-insensitive; Quality must be exactly equal (no epsilon tolerance, matching .NET). */
        [[nodiscard]] bool Equals(const StringWithQualityHeaderValue& other) const;
        bool operator==(const StringWithQualityHeaderValue& o) const { return Equals(o); }
        bool operator!=(const StringWithQualityHeaderValue& o) const { return !Equals(o); }

        /** @return A hash code consistent with Equals(). */
        [[nodiscard]] SharpRuntime::intcs GetHashCode() const;

        /**
         * @brief Parses "value" or "value; q=quality".
         * @throws System::FormatException if @p input is not valid.
         */
        [[nodiscard]] static StringWithQualityHeaderValue Parse(const std::string& input);

        /** Attempts to parse @p input; returns false without throwing on failure. */
        [[nodiscard]] static bool TryParse(const std::string& input, StringWithQualityHeaderValue& parsedValue);
    };

} // namespace System::Net::Http::Headers

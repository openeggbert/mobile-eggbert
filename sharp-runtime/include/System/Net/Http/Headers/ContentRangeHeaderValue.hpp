// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <optional>
#include <string>
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Net::Http::Headers {

    /**
     * @brief Represents the value of the Content-Range header (e.g. "bytes 12-34/5678").
     *
     * C++ counterpart of .NET System.Net.Http.Headers.ContentRangeHeaderValue.
     *
     * @note .NET's internal GetContentRangeLength parsing helper is not ported; Parse/TryParse
     * here implement the same practical algorithm directly. ICloneable is not ported (no
     * reflection-based virtual-clone story, per NEXT.md).
     */
    class ContentRangeHeaderValue {
        std::string unit_ = "bytes";
        std::optional<SharpRuntime::longcs> from_;
        std::optional<SharpRuntime::longcs> to_;
        std::optional<SharpRuntime::longcs> length_;

        ContentRangeHeaderValue() = default;

    public:
        /**
         * @brief Constructs "bytes {from}-{to}/{length}".
         * @throws std::out_of_range if any of @p from, @p to, @p length is negative, if
         * @p from &gt; @p to, or if @p to &gt; @p length.
         */
        ContentRangeHeaderValue(SharpRuntime::longcs from, SharpRuntime::longcs to, SharpRuntime::longcs length);

        /**
         * @brief Constructs "bytes {*}/{length}".
         * @throws std::out_of_range if @p length is negative.
         */
        explicit ContentRangeHeaderValue(SharpRuntime::longcs length);

        /**
         * @brief Constructs "bytes {from}-{to}/{*}".
         * @throws std::out_of_range if @p from or @p to is negative, or @p from &gt; @p to.
         */
        ContentRangeHeaderValue(SharpRuntime::longcs from, SharpRuntime::longcs to);

        /** @return The range unit (e.g. "bytes"). */
        [[nodiscard]] const std::string& getUnitProperty() const { return unit_; }
        /**
         * Sets the range unit.
         * @throws System::FormatException if @p value is not a valid HTTP token.
         */
        void setUnitProperty(const std::string& value);

        /** @return The starting byte offset, or empty if this value has no range (e.g. "bytes * /1234"). */
        [[nodiscard]] std::optional<SharpRuntime::longcs> getFromProperty() const { return from_; }
        /** @return The ending byte offset, or empty if this value has no range. */
        [[nodiscard]] std::optional<SharpRuntime::longcs> getToProperty() const { return to_; }
        /** @return The total resource length, or empty if unknown (e.g. "bytes 12-34/ *"). */
        [[nodiscard]] std::optional<SharpRuntime::longcs> getLengthProperty() const { return length_; }

        /** @return true if a range (From/To) is present. */
        [[nodiscard]] bool getHasRangeProperty() const { return from_.has_value(); }
        /** @return true if a total length is present. */
        [[nodiscard]] bool getHasLengthProperty() const { return length_.has_value(); }

        /** @return "unit from-to/length", with "*" substituted for an absent range or length. */
        [[nodiscard]] std::string ToString() const;

        /** Equality: Unit is case-insensitive; From/To/Length must match exactly. */
        [[nodiscard]] bool Equals(const ContentRangeHeaderValue& other) const;
        bool operator==(const ContentRangeHeaderValue& o) const { return Equals(o); }
        bool operator!=(const ContentRangeHeaderValue& o) const { return !Equals(o); }

        /** @return A hash code consistent with Equals(). */
        [[nodiscard]] SharpRuntime::intcs GetHashCode() const;

        /**
         * @brief Parses "unit from-to/length" (each of the range and length may be "*").
         * @throws System::FormatException if @p input is not valid.
         */
        [[nodiscard]] static ContentRangeHeaderValue Parse(const std::string& input);

        /** Attempts to parse @p input; returns false without throwing on failure. */
        [[nodiscard]] static bool TryParse(const std::string& input, ContentRangeHeaderValue& parsedValue);
    };

} // namespace System::Net::Http::Headers

// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <optional>
#include <string>
#include <vector>
#include "System/Net/Http/Headers/RangeItemHeaderValue.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Net::Http::Headers {

    /**
     * @brief Represents the value of the Range header (e.g. "bytes=0-499, 1000-1499").
     *
     * C++ counterpart of .NET System.Net.Http.Headers.RangeHeaderValue.
     *
     * @note .NET's internal GetRangeLength/GetRangeItemListLength parsing helpers are not ported;
     * Parse/TryParse here implement the same algorithm directly (unit "=" comma-separated range
     * list, tolerating empty list segments like ",1-2, , 3-4,,"). ICloneable is not ported (no
     * reflection-based virtual-clone story, per NEXT.md).
     */
    class RangeHeaderValue {
        std::string unit_ = "bytes";
        std::vector<RangeItemHeaderValue> ranges_;

    public:
        RangeHeaderValue() = default;

        /** @brief Convenience constructor: "bytes={from}-{to}". */
        RangeHeaderValue(std::optional<SharpRuntime::longcs> from, std::optional<SharpRuntime::longcs> to);

        /** @return The range unit (e.g. "bytes"). */
        [[nodiscard]] const std::string& getUnitProperty() const { return unit_; }
        /**
         * Sets the range unit.
         * @throws System::FormatException if @p value is not a valid HTTP token.
         */
        void setUnitProperty(const std::string& value);

        /** @return The range list, mutable. */
        [[nodiscard]] std::vector<RangeItemHeaderValue>& getRangesProperty() { return ranges_; }
        [[nodiscard]] const std::vector<RangeItemHeaderValue>& getRangesProperty() const { return ranges_; }

        /** @return "unit=from1-to1, from2-to2, ...". */
        [[nodiscard]] std::string ToString() const;

        /** Equality: Unit is case-insensitive; Ranges compared order-independently. */
        [[nodiscard]] bool Equals(const RangeHeaderValue& other) const;
        bool operator==(const RangeHeaderValue& o) const { return Equals(o); }
        bool operator!=(const RangeHeaderValue& o) const { return !Equals(o); }

        /** @return A hash code consistent with Equals(). */
        [[nodiscard]] SharpRuntime::intcs GetHashCode() const;

        /**
         * @brief Parses "unit=from1-to1, from2-to2, ...".
         * @throws System::FormatException if @p input is not valid.
         */
        [[nodiscard]] static RangeHeaderValue Parse(const std::string& input);

        /** Attempts to parse @p input; returns false without throwing on failure. */
        [[nodiscard]] static bool TryParse(const std::string& input, RangeHeaderValue& parsedValue);
    };

} // namespace System::Net::Http::Headers

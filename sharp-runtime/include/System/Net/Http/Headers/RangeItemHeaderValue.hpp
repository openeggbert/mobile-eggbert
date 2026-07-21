// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <optional>
#include <string>
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Net::Http::Headers {

    /**
     * @brief Represents a single byte range item within a Range header (e.g. "1-2", "1-", "-2").
     *
     * C++ counterpart of .NET System.Net.Http.Headers.RangeItemHeaderValue.
     */
    class RangeItemHeaderValue {
        std::optional<SharpRuntime::longcs> from_;
        std::optional<SharpRuntime::longcs> to_;

    public:
        /**
         * @brief Constructs a range item. At least one of @p from / @p to must be present.
         * @throws System::ArgumentException if both @p from and @p to are empty.
         * @throws System::ArgumentOutOfRangeException if either value is negative, or @p from &gt; @p to.
         */
        RangeItemHeaderValue(std::optional<SharpRuntime::longcs> from, std::optional<SharpRuntime::longcs> to);

        /** @return The starting byte offset, or empty if this is a suffix range (e.g. "-500"). */
        [[nodiscard]] std::optional<SharpRuntime::longcs> getFromProperty() const { return from_; }
        /** @return The ending byte offset, or empty if this is an open-ended range (e.g. "500-"). */
        [[nodiscard]] std::optional<SharpRuntime::longcs> getToProperty() const { return to_; }

        /** @return "from-to", "from-", or "-to" depending on which bound(s) are present. */
        [[nodiscard]] std::string ToString() const;

        /** Equality: both From and To must match exactly. */
        [[nodiscard]] bool Equals(const RangeItemHeaderValue& other) const { return from_ == other.from_ && to_ == other.to_; }
        bool operator==(const RangeItemHeaderValue& o) const { return Equals(o); }
        bool operator!=(const RangeItemHeaderValue& o) const { return !Equals(o); }

        /** @return A hash code consistent with Equals(). */
        [[nodiscard]] SharpRuntime::intcs GetHashCode() const;
    };

} // namespace System::Net::Http::Headers

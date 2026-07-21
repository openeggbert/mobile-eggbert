// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <optional>
#include <string>
#include "System/Net/Http/Headers/EntityTagHeaderValue.hpp"
#include "System/DateTimeOffset.hpp"

namespace System::Net::Http::Headers {

    /**
     * @brief Represents the value of the If-Range header: either a date or an entity-tag,
     * mutually exclusive.
     *
     * C++ counterpart of .NET System.Net.Http.Headers.RangeConditionHeaderValue.
     *
     * @note Date parses via a small local RFC 1123 parser (see ContentDispositionHeaderValue's
     * class doc note for why System::DateTimeOffset::TryParse can't be used directly).
     * @note .NET's internal GetRangeConditionLength parsing helper is not ported. ICloneable is
     * not ported (no reflection-based virtual-clone story, per NEXT.md).
     */
    class RangeConditionHeaderValue {
        std::optional<System::DateTimeOffset> date_;
        std::optional<EntityTagHeaderValue> entityTag_;

        RangeConditionHeaderValue() = default;

    public:
        /** @brief Constructs a date-based value. */
        explicit RangeConditionHeaderValue(const System::DateTimeOffset& date);
        /** @brief Constructs an entity-tag-based value. */
        explicit RangeConditionHeaderValue(const EntityTagHeaderValue& entityTag);
        /**
         * @brief Constructs an entity-tag-based value from a quoted-string tag.
         * @throws System::FormatException if @p entityTag is not a valid quoted-string.
         */
        explicit RangeConditionHeaderValue(const std::string& entityTag);

        /** @return The date, or empty if this value is entity-tag-based. */
        [[nodiscard]] std::optional<System::DateTimeOffset> getDateProperty() const { return entityTag_.has_value() ? std::nullopt : date_; }
        /** @return The entity-tag, or empty if this value is date-based. */
        [[nodiscard]] const std::optional<EntityTagHeaderValue>& getEntityTagProperty() const { return entityTag_; }

        /** @return The entity-tag's ToString(), or the date formatted as RFC 1123. */
        [[nodiscard]] std::string ToString() const;

        /** Equality: compares whichever of Date/EntityTag is populated. */
        [[nodiscard]] bool Equals(const RangeConditionHeaderValue& other) const;
        bool operator==(const RangeConditionHeaderValue& o) const { return Equals(o); }
        bool operator!=(const RangeConditionHeaderValue& o) const { return !Equals(o); }

        /** @return A hash code consistent with Equals(). */
        [[nodiscard]] SharpRuntime::intcs GetHashCode() const;

        /**
         * @brief Parses an entity-tag ("*"/"\"tag\""/"W/\"tag\"") or an RFC 1123 date.
         * @throws System::FormatException if @p input is not valid.
         */
        [[nodiscard]] static RangeConditionHeaderValue Parse(const std::string& input);

        /** Attempts to parse @p input; returns false without throwing on failure. */
        [[nodiscard]] static bool TryParse(const std::string& input, RangeConditionHeaderValue& parsedValue);
    };

} // namespace System::Net::Http::Headers

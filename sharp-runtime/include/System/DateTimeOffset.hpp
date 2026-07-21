// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Created by robertvokac on 6/7/25.
//

#pragma once

#include "System/Object.hpp"
#include "System/DateTime.hpp"
#include "System/TimeSpan.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System {

    /**
     * @brief Represents a point in time, typically expressed as a date and time of day,
     * relative to Coordinated Universal Time (UTC).
     *
     * This is a partial C++ counterpart of the .NET System::DateTimeOffset structure.
     * It stores a "clock" DateTime value together with its UTC offset; the UTC instant
     * (used for identity, sorting, and subtraction) is derived as clock ticks minus offset.
     *
     * @note Status: Reduced to the subset actually exercised by this codebase (default
     *   construction, DateTimeOffset::getUtcNowProperty() to timestamp sensor readings,
     *   and equality comparison of those timestamps). See git history for the previously
     *   fuller .NET-parity surface (component accessors, arithmetic, Unix-time conversion,
     *   parsing/formatting, ordering comparison) if a future consumer needs it restored.
     */
    class DateTimeOffset : public Object {
    private:
        DateTime dateTime_;
        TimeSpan offset_;

    public:
        DateTimeOffset();
        DateTimeOffset(const DateTime& dateTime, const TimeSpan& offset);

        // Static factory
        /** @brief Gets a DateTimeOffset for the current UTC date and time, with a zero offset. */
        [[nodiscard]] static DateTimeOffset getUtcNowProperty();

        /** @return The number of ticks representing the date and time of this instance, in UTC. */
        [[nodiscard]] longcs getUtcTicksProperty() const;

        using Object::Equals;

        /** @brief Returns true if @p other represents the same point in time as this instance. */
        [[nodiscard]] bool Equals(const DateTimeOffset& other) const;

        bool operator==(const DateTimeOffset& other) const;

        GetTypeNameHPP()
    };

} // namespace System

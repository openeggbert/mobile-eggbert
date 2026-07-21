// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include "System/Object.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System {

    using SharpRuntime::longcs;

    /**
     * @brief Represents an instant in time, expressed as the number of 100-nanosecond
     * ticks since the .NET epoch (0001-01-01 00:00:00).
     *
     * Partial C++ counterpart of .NET System.DateTime.
     *
     * @note Status: Reduced to the subset actually exercised by this codebase (default
     *   construction, the tick-count constructor, and DateTime::getNowProperty(), used
     *   internally by System::DateTimeOffset::getUtcNowProperty() to timestamp sensor
     *   readings). See git history for the previously fuller .NET-parity surface
     *   (component accessors, arithmetic, Unix-time conversion, parsing/formatting,
     *   comparison) if a future consumer needs it restored.
     */
    class DateTime : public Object {
    public:
        static constexpr longcs TicksPerSecond      = 10000000LL;
        /** @brief Ticks from the .NET epoch (0001-01-01) to the Unix epoch (1970-01-01). */
        static constexpr longcs UnixEpochTicks      = 621355968000000000LL;
        /** @brief The maximum tick value representable (9999-12-31 23:59:59.9999999). */
        static constexpr longcs MaxTicks            = 3155378975999999999LL;

    private:
        longcs ticks_;

    public:
        /**
         * @brief Initializes a new instance with zero ticks (0001-01-01 00:00:00).
         */
        DateTime();

        /**
         * @brief Initializes a new instance with the specified number of ticks.
         *
         * @param ticks A date and time expressed in 100-nanosecond ticks since
         *              the .NET epoch (0001-01-01 00:00:00).
         * @throws System::ArgumentOutOfRangeException if @p ticks is negative or greater than MaxTicks.
         */
        explicit DateTime(longcs ticks);

        /**
         * @brief Gets the number of 100-nanosecond ticks since the .NET epoch.
         *
         * @return Tick count (0 = 0001-01-01 00:00:00).
         */
        [[nodiscard]] longcs getTicksProperty() const;

        /**
         * @brief Gets the current local date and time.
         *
         * @return Current local DateTime expressed in .NET-compatible ticks.
         * @note DateTimeKind is not stored; the value reflects UTC-based system time.
         */
        [[nodiscard]] static DateTime getNowProperty();

        GetTypeNameHPP()
    };

} // namespace System

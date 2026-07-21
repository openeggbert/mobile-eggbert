// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/DateTime.hpp"
#include "System/TimeSpan.hpp"

namespace System {

    /**
     * @brief Represents a time zone.
     *
     * C++ counterpart of .NET System.TimeZone (abstract base class).
     * Concrete subclasses implement GetUtcOffset and IsDaylightSavingTime.
     * Prefer TimeZoneInfo for new code; TimeZone is provided for compatibility.
     */
    class TimeZone {
    public:
        /** @brief Virtual destructor. */
        virtual ~TimeZone() = default;

        /**
         * @brief Returns the standard (non-daylight-saving) name of the time zone.
         *
         * C++ counterpart of .NET TimeZone.StandardName.
         */
        virtual const std::string& getStandardNameProperty()  const = 0;

        /**
         * @brief Returns the daylight-saving name of the time zone.
         *
         * C++ counterpart of .NET TimeZone.DaylightName.
         */
        virtual const std::string& getDaylightNameProperty()  const = 0;

        /**
         * @brief Returns the UTC offset for the given local time.
         *
         * C++ counterpart of .NET TimeZone.GetUtcOffset(DateTime).
         * @param time Local date/time whose offset is requested.
         */
        virtual TimeSpan GetUtcOffset(const DateTime& time)   const = 0;

        /**
         * @brief Returns true if the specified time falls within a daylight saving time period.
         *
         * C++ counterpart of .NET TimeZone.IsDaylightSavingTime(DateTime).
         * @param time Local date/time to test.
         */
        virtual bool IsDaylightSavingTime(const DateTime& time) const = 0;

        /**
         * @brief Returns the current local time zone.
         *
         * C++ counterpart of .NET TimeZone.CurrentTimeZone.
         */
        static const TimeZone& CurrentTimeZone();
    };

} // namespace System

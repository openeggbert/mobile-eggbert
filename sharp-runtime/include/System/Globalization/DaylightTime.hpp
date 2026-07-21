// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/DateTime.hpp"
#include "System/TimeSpan.hpp"

namespace System::Globalization {

/**
 * @brief Defines the period of daylight saving time, including the start, end, and time delta.
 *
 * C++ counterpart of .NET System.Globalization.DaylightTime.
 */
class DaylightTime {
    System::DateTime start_;
    System::DateTime end_;
    System::TimeSpan delta_;

public:
    /**
     * @brief Constructs a DaylightTime with the given start, end, and clock delta.
     *
     * C++ counterpart of .NET DaylightTime(DateTime, DateTime, TimeSpan).
     * @param start The date and time when daylight saving time begins.
     * @param end   The date and time when daylight saving time ends.
     * @param delta The clock offset applied during daylight saving time.
     */
    DaylightTime(const System::DateTime& start, const System::DateTime& end,
                 const System::TimeSpan& delta)
        : start_(start), end_(end), delta_(delta) {}

    /**
     * @brief Gets the date and time when daylight saving time begins.
     *
     * C++ counterpart of .NET DaylightTime.Start.
     * @return The start of daylight saving time.
     */
    [[nodiscard]] const System::DateTime& getStartProperty() const { return start_; }

    /**
     * @brief Gets the date and time when daylight saving time ends.
     *
     * C++ counterpart of .NET DaylightTime.End.
     * @return The end of daylight saving time.
     */
    [[nodiscard]] const System::DateTime& getEndProperty() const { return end_; }

    /**
     * @brief Gets the clock offset applied when daylight saving time is in effect.
     *
     * C++ counterpart of .NET DaylightTime.Delta.
     * @return The time delta for daylight saving time.
     */
    [[nodiscard]] const System::TimeSpan& getDeltaProperty() const { return delta_; }
};

} // namespace System::Globalization

// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Globalization {

/**
 * @brief Defines the different rules for determining the first week of the year.
 *
 * C++ counterpart of .NET System.Globalization.CalendarWeekRule.
 */
enum class CalendarWeekRule {
    FirstDay         = 0, ///< The first week begins on the first day of the year.
    FirstFullWeek    = 1, ///< The first week is the first week that contains seven full days.
    FirstFourDayWeek = 2, ///< The first week is the first week that contains at least four days.
};

} // namespace System::Globalization

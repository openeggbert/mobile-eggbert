// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Globalization {

/**
 * @brief Specifies whether a calendar is solar-based, lunar-based, or lunisolar-based.
 *
 * C++ counterpart of .NET System.Globalization.CalendarAlgorithmType.
 */
enum class CalendarAlgorithmType {
    Unknown          = 0, ///< An unknown calendar basis.
    SolarCalendar    = 1, ///< A solar-based calendar.
    LunarCalendar    = 2, ///< A lunar-based calendar.
    LunisolarCalendar = 3, ///< A lunisolar calendar.
};

} // namespace System::Globalization

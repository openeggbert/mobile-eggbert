// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Globalization {

/**
 * @brief Defines the Gregorian calendar types corresponding to the Arabic, Localized, or Transliterated types.
 *
 * C++ counterpart of .NET System.Globalization.GregorianCalendarTypes.
 * Passed to the GregorianCalendar(GregorianCalendarTypes) constructor to select
 * the variant used for localized or transliterated date display.
 */
enum class GregorianCalendarTypes {
    Localized             = 1,  ///< The calendar is localized in the language of the culture using it.
    USEnglish             = 2,  ///< The calendar is in US English.
    MiddleEastFrench      = 9,  ///< The calendar is in Middle East French.
    Arabic                = 10, ///< The calendar is in Arabic.
    TransliteratedEnglish = 11, ///< The calendar is a transliterated version of the English calendar.
    TransliteratedFrench  = 12, ///< The calendar is a transliterated version of the French calendar.
};

} // namespace System::Globalization

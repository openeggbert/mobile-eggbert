// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Globalization {

/**
 * @brief Defines the types of culture lists that can be retrieved using GetCultures.
 *
 * C++ counterpart of .NET System.Globalization.CultureTypes.
 * Values may be combined with the | operator.
 */
enum class CultureTypes {
    NeutralCultures        = 0x0001, ///< Cultures with only a language, no region (e.g. "en").
    SpecificCultures       = 0x0002, ///< Cultures specific to both a language and region (e.g. "en-US").
    InstalledWin32Cultures = 0x0004, ///< Cultures installed in the Windows operating system.
    AllCultures            = 0x0007, ///< All cultures recognized by .NET (NeutralCultures | SpecificCultures | InstalledWin32Cultures).
    UserCustomCulture      = 0x0008, ///< Custom cultures created by the user.
    ReplacementCultures    = 0x0010, ///< Custom cultures that replace shipped cultures.
    WindowsOnlyCultures    = 0x0020, ///< @deprecated Cultures that are specific to Windows and not cross-platform.
    FrameworkCultures      = 0x0040, ///< @deprecated Cultures that ship with the .NET Framework.
};

/**
 * @brief Combines two CultureTypes values with bitwise OR.
 * @param a First type set.
 * @param b Second type set.
 * @return The combined type set.
 */
inline CultureTypes operator|(CultureTypes a, CultureTypes b) {
    return static_cast<CultureTypes>(static_cast<int>(a) | static_cast<int>(b));
}

/**
 * @brief Masks two CultureTypes values with bitwise AND.
 * @param a First type set.
 * @param b Second type set.
 * @return The masked type set.
 */
inline CultureTypes operator&(CultureTypes a, CultureTypes b) {
    return static_cast<CultureTypes>(static_cast<int>(a) & static_cast<int>(b));
}

} // namespace System::Globalization

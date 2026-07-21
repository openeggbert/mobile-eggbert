// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Globalization {

/**
 * @brief Defines the formatting options that customize string parsing for date and time methods.
 *
 * C++ counterpart of .NET System.Globalization.DateTimeStyles.
 * Values may be combined with the | operator.
 */
enum class DateTimeStyles {
    None                 = 0x00000000, ///< Default formatting rules apply.
    AllowLeadingWhite    = 0x00000001, ///< Leading white-space characters are ignored during parsing.
    AllowTrailingWhite   = 0x00000002, ///< Trailing white-space characters are ignored during parsing.
    AllowInnerWhite      = 0x00000004, ///< Extra white-space in the middle of the string is ignored during parsing.
    AllowWhiteSpaces     = 0x00000007, ///< Allow leading, trailing, and inner white-space (AllowLeadingWhite | AllowTrailingWhite | AllowInnerWhite).
    NoCurrentDateDefault = 0x00000008, ///< If no date component is found, use January 1, year 1 instead of the current date.
    AdjustToUniversal    = 0x00000010, ///< Convert the parsed result to UTC.
    AssumeLocal          = 0x00000020, ///< Assume the string represents local time when no time zone is specified.
    AssumeUniversal      = 0x00000040, ///< Assume the string represents UTC when no time zone is specified.
    RoundtripKind        = 0x00000080, ///< Preserve the DateTimeKind field when converting a DateTime to a string and back.
};

/**
 * @brief Combines two DateTimeStyles values with bitwise OR.
 * @param a First style set.
 * @param b Second style set.
 * @return The combined style set.
 */
inline DateTimeStyles operator|(DateTimeStyles a, DateTimeStyles b) {
    return static_cast<DateTimeStyles>(static_cast<int>(a) | static_cast<int>(b));
}

/**
 * @brief Masks two DateTimeStyles values with bitwise AND.
 * @param a First style set.
 * @param b Second style set.
 * @return The masked style set.
 */
inline DateTimeStyles operator&(DateTimeStyles a, DateTimeStyles b) {
    return static_cast<DateTimeStyles>(static_cast<int>(a) & static_cast<int>(b));
}

} // namespace System::Globalization

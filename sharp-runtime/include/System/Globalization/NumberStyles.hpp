// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Globalization {

/**
 * @brief Defines the number style elements that can be present in a parsed numeric string.
 *
 * C++ counterpart of .NET System.Globalization.NumberStyles.
 * Values may be combined with the | operator to permit multiple style elements.
 */
enum class NumberStyles {
    None                 = 0x00000000, ///< No style elements are permitted.
    AllowLeadingWhite    = 0x00000001, ///< Leading white-space characters are permitted.
    AllowTrailingWhite   = 0x00000002, ///< Trailing white-space characters are permitted.
    AllowLeadingSign     = 0x00000004, ///< A leading sign (+ or -) is permitted.
    AllowTrailingSign    = 0x00000008, ///< A trailing sign (+ or -) is permitted.
    AllowParentheses     = 0x00000010, ///< Parentheses indicating a negative value are permitted.
    AllowDecimalPoint    = 0x00000020, ///< A decimal point character is permitted.
    AllowThousands       = 0x00000040, ///< Group separators (e.g. thousands) are permitted.
    AllowExponent        = 0x00000080, ///< An exponent (e.g. "E+10") is permitted.
    AllowCurrencySymbol  = 0x00000100, ///< A currency symbol is permitted.
    AllowHexSpecifier    = 0x00000200, ///< The string is parsed as a hexadecimal value.
    AllowBinarySpecifier = 0x00000400, ///< The string is parsed as a binary value.
    Integer    = 0x00000007, ///< AllowLeadingWhite | AllowTrailingWhite | AllowLeadingSign.
    HexNumber  = 0x00000203, ///< AllowLeadingWhite | AllowTrailingWhite | AllowHexSpecifier.
    BinaryNumber = 0x00000403, ///< AllowLeadingWhite | AllowTrailingWhite | AllowBinarySpecifier.
    Number     = 0x0000006F, ///< AllowLeadingWhite | AllowTrailingWhite | AllowLeadingSign | AllowTrailingSign | AllowDecimalPoint | AllowThousands.
    Float      = 0x000000A7, ///< AllowLeadingWhite | AllowTrailingWhite | AllowLeadingSign | AllowDecimalPoint | AllowExponent.
    HexFloat   = 0x000002A7, ///< AllowLeadingWhite | AllowTrailingWhite | AllowLeadingSign | AllowDecimalPoint | AllowExponent | AllowHexSpecifier.
    Currency   = 0x0000017F, ///< AllowLeadingWhite | AllowTrailingWhite | AllowLeadingSign | AllowTrailingSign | AllowParentheses | AllowDecimalPoint | AllowThousands | AllowCurrencySymbol.
    Any        = 0x000001FF, ///< Currency | AllowExponent — all styles except AllowHexSpecifier and AllowBinarySpecifier.
};

/**
 * @brief Combines two NumberStyles values with bitwise OR.
 * @param a First style set.
 * @param b Second style set.
 * @return The combined style set.
 */
inline NumberStyles operator|(NumberStyles a, NumberStyles b) {
    return static_cast<NumberStyles>(static_cast<int>(a) | static_cast<int>(b));
}

/**
 * @brief Masks two NumberStyles values with bitwise AND.
 * @param a First style set.
 * @param b Second style set.
 * @return The masked style set.
 */
inline NumberStyles operator&(NumberStyles a, NumberStyles b) {
    return static_cast<NumberStyles>(static_cast<int>(a) & static_cast<int>(b));
}

} // namespace System::Globalization

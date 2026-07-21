// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Globalization {

/**
 * @brief Defines the string comparison options to use with CompareInfo.
 *
 * C++ counterpart of .NET System.Globalization.CompareOptions.
 * Values may be combined with the | operator.
 */
enum class CompareOptions {
    None             = 0x00000000, ///< Use default comparison rules.
    IgnoreCase       = 0x00000001, ///< Ignore case during comparison.
    IgnoreNonSpace   = 0x00000002, ///< Ignore non-spacing combining characters.
    IgnoreSymbols    = 0x00000004, ///< Ignore symbols and punctuation.
    IgnoreKanaType   = 0x00000008, ///< Ignore the kana type (hiragana vs. katakana).
    IgnoreWidth      = 0x00000010, ///< Ignore character width (full-width vs. half-width).
    NumericOrdering  = 0x00000020, ///< Sort digit strings numerically rather than lexicographically.
    OrdinalIgnoreCase = 0x10000000, ///< Ordinal comparison, ignoring case.
    StringSort       = 0x20000000, ///< Sort punctuation as if it preceded alphanumeric characters.
    Ordinal          = 0x40000000, ///< Ordinal (byte-value) comparison.
};

/**
 * @brief Combines two CompareOptions values with bitwise OR.
 * @param a First option set.
 * @param b Second option set.
 * @return The combined option set.
 */
inline CompareOptions operator|(CompareOptions a, CompareOptions b) {
    return static_cast<CompareOptions>(static_cast<int>(a) | static_cast<int>(b));
}

/**
 * @brief Masks two CompareOptions values with bitwise AND.
 * @param a First option set.
 * @param b Second option set.
 * @return The masked option set.
 */
inline CompareOptions operator&(CompareOptions a, CompareOptions b) {
    return static_cast<CompareOptions>(static_cast<int>(a) & static_cast<int>(b));
}

} // namespace System::Globalization

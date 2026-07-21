// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System {

/**
 * @brief Specifies the casing style used in string comparison and conversion.
 *
 * C++ counterpart of .NET System.Casing (internal enum).
 * Used by globalization routines to distinguish upper-case from lower-case operations.
 */
enum class Casing : unsigned int {
    /** @brief Indicates upper-casing; outputs [ '0'..'9' ] and [ 'A'..'F' ]. */
    Upper = 0,
    /** @brief Indicates lower-casing; outputs [ '0'..'9' ] and [ 'a'..'f' ].
     *  The value 0x2020 is a bitmask used by HexConverter to OR with nibble
     *  outputs to shift A-F (0x41-0x46) to a-f (0x61-0x66). */
    Lower = 0x2020U,
};

} // namespace System

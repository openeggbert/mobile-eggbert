// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System {

    /**
     * @brief Specifies the culture, case, and sort rules to be used by certain overloads
     * of the String.Compare and String.Equals methods.
     */
    enum class StringComparison {
        /** @brief Compare strings using culture-sensitive sort rules and the current culture. */
        CurrentCulture           = 0,
        /** @brief Compare strings using culture-sensitive sort rules, the current culture, and ignoring the case of the strings being compared. */
        CurrentCultureIgnoreCase = 1,
        /** @brief Compare strings using culture-sensitive sort rules and the invariant culture. */
        InvariantCulture         = 2,
        /** @brief Compare strings using culture-sensitive sort rules, the invariant culture, and ignoring the case of the strings being compared. */
        InvariantCultureIgnoreCase = 3,
        /** @brief Compare strings using ordinal (binary) sort rules. */
        Ordinal                  = 4,
        /** @brief Compare strings using ordinal (binary) sort rules and ignoring the case of the strings being compared. */
        OrdinalIgnoreCase        = 5,
    };

} // namespace System

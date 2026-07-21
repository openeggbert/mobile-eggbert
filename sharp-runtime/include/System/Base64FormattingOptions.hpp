// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System {

/**
 * @brief Specifies whether relevant Convert.ToBase64CharArray and
 * Convert.ToBase64String methods insert line breaks in their output.
 *
 * C++ counterpart of .NET System.Base64FormattingOptions.
 */
enum class Base64FormattingOptions {
    /** @brief Does not insert line breaks after every 76 characters in the string representation. */
    None = 0,
    /** @brief Inserts line breaks after every 76 characters in the string representation. */
    InsertLineBreaks = 1,
};

} // namespace System

// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Globalization {

/**
 * @brief Specifies the shape of digits in a text string.
 *
 * C++ counterpart of .NET System.Globalization.DigitShapes.
 * Used to control whether digit characters appear as their native national
 * form or as ASCII Latin digits (0–9).
 */
enum class DigitShapes {
    Context        = 0, ///< Digit shape is determined by preceding text in the same output.
    None           = 1, ///< Do not change digit shapes; output uses traditional Latin digits (0–9).
    NativeNational = 2, ///< Use the digit shape that is native to the national script or language.
};

} // namespace System::Globalization

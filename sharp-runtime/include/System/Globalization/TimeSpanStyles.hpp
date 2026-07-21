// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Globalization {

/**
 * @brief Defines the formatting options that customize string parsing for TimeSpan.Parse/TryParse methods.
 *
 * C++ counterpart of .NET System.Globalization.TimeSpanStyles.
 * Values may be combined with the | operator.
 */
enum class TimeSpanStyles {
    None           = 0x00000000, ///< Default formatting rules apply.
    AssumeNegative = 0x00000001, ///< Treat the parsed string as a negative time span.
};

/**
 * @brief Combines two TimeSpanStyles values with bitwise OR.
 * @param a First style set.
 * @param b Second style set.
 * @return The combined style set.
 */
inline TimeSpanStyles operator|(TimeSpanStyles a, TimeSpanStyles b) {
    return static_cast<TimeSpanStyles>(static_cast<int>(a) | static_cast<int>(b));
}

/**
 * @brief Masks two TimeSpanStyles values with bitwise AND.
 * @param a First style set.
 * @param b Second style set.
 * @return The masked style set.
 */
inline TimeSpanStyles operator&(TimeSpanStyles a, TimeSpanStyles b) {
    return static_cast<TimeSpanStyles>(static_cast<int>(a) & static_cast<int>(b));
}

} // namespace System::Globalization

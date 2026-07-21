// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System {

    /**
     * @brief Specifies how mathematical rounding methods should process a number that
     * is midway between two numbers.
     *
     * C++ counterpart of .NET System.MidpointRounding.
     */
    enum class MidpointRounding {
        /** @brief Round to nearest even number (banker's rounding). */
        ToEven             = 0,
        /** @brief Round away from zero. */
        AwayFromZero       = 1,
        /** @brief Round toward zero (truncate). */
        ToZero             = 2,
        /** @brief Round toward negative infinity (floor). */
        ToNegativeInfinity = 3,
        /** @brief Round toward positive infinity (ceiling). */
        ToPositiveInfinity = 4,
    };

} // namespace System

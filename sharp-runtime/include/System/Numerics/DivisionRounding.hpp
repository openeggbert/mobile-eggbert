// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Numerics {

    /**
     * @brief Specifies the rounding strategy to use when performing division.
     *
     * C++ counterpart of .NET System.Numerics.DivisionRounding, used by IBinaryInteger<TSelf>'s
     * generic-math Divide(TSelf, TSelf, DivisionRounding) overload. Since IBinaryInteger is a
     * stub in this runtime (C++ templates cover the generic-math use case; see
     * GenericMathInterfaces.hpp), no concrete type currently implements the rounding-aware
     * overload — this enum exists for API name compatibility with code ported from .NET.
     */
    enum class DivisionRounding {
        /** Truncated division (rounding toward zero). */
        Truncate = 0,
        /** Floor division (rounding down). */
        Floor = 1,
        /** Ceiling division (rounding up). */
        Ceiling = 2,
        /** Division rounding away from zero. */
        AwayFromZero = 3,
        /** Euclidean division; always yields a non-negative remainder. */
        Euclidean = 4,
    };

} // namespace System::Numerics

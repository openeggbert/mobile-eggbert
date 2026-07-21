// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System {

    using SharpRuntime::intcs;

    /** Provides constants for multi-dimensional array rank limits. */
    class MDArray {
    public:
        /** The minimum rank (number of dimensions) for a multi-dimensional array. */
        static constexpr intcs MinRank = 1;
        /** The maximum rank (number of dimensions) for a multi-dimensional array. */
        static constexpr intcs MaxRank = 32;
    };

} // namespace System

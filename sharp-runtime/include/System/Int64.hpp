// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <cstdint>
#include <limits>
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System {

    using SharpRuntime::longcs;

    /**
     * @brief Represents a 64-bit signed integer.
     *
     * C++ counterpart of .NET System.Int64.
     * All members are static; the class cannot be instantiated.
     * The underlying C++ type is @c int64_t (aliased as @c SharpRuntime::longcs).
     */
    class Int64 {
    public:
        Int64() = delete;

        /** @brief The maximum value of an Int64 (9 223 372 036 854 775 807). C++ counterpart of .NET Int64.MaxValue. */
        static constexpr longcs MaxValue = std::numeric_limits<int64_t>::max();
    };

} // namespace System

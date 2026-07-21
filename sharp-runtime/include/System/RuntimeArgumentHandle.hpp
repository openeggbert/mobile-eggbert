// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System {

    /**
     * @brief Opaque handle representing a variable-argument list passed to a method.
     *
     * C++ counterpart of .NET System.RuntimeArgumentHandle.
     *
     * In .NET this is a `ref struct` used exclusively by ArgIterator to iterate
     * over `__arglist` parameters — a CLR feature with no equivalent in C++.
     * The struct is preserved so that code referencing the type compiles; it
     * carries no data and provides no operations.
     */
    struct RuntimeArgumentHandle {
    };

} // namespace System

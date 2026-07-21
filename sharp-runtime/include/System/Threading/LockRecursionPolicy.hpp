// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Threading {

    /** Specifies whether a lock supports recursive entry by the same thread. */
    enum class LockRecursionPolicy {
        /** The lock does not support recursive entry. */
        NoRecursion       = 0,
        /** The lock supports recursive entry. */
        SupportsRecursion = 1,
    };

} // namespace System::Threading

// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::IO {

    /** Specifies whether the underlying handle is inheritable by child processes. */
    enum class HandleInheritability {
        /** The handle is not inheritable. */
        None        = 0,
        /** The handle is inheritable. */
        Inheritable = 1,
    };

} // namespace System::IO

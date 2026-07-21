// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::IO {

    /** Specifies whether to search only the top-level directory or all subdirectories. */
    enum class SearchOption {
        /** Includes only the top directory. */
        TopDirectoryOnly = 0,
        /** Includes the top directory and all subdirectories. */
        AllDirectories   = 1,
    };

} // namespace System::IO

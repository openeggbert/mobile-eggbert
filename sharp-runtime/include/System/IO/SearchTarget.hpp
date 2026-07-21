// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::IO {

    /** Specifies the type of entries returned by a file-system enumeration. */
    enum class SearchTarget {
        /** Return only files. */
        Files       = 1,
        /** Return only directories. */
        Directories = 2,
        /** Return both files and directories. */
        Both        = 3,
    };

} // namespace System::IO

// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::IO {

    /** Specifies the match type used in file-system enumeration. */
    enum class MatchType {
        /** Uses simple wildcard matching (* and ?). */
        Simple = 0,
        /** Uses Win32 extended wildcard semantics. */
        Win32  = 1,
    };

} // namespace System::IO

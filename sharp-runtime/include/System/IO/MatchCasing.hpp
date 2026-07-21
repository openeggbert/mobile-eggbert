// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::IO {

    /** Controls case sensitivity when matching file names against a search pattern. */
    enum class MatchCasing {
        /** Use the case sensitivity rules of the current platform. */
        PlatformDefault  = 0,
        /** Match using case-sensitive comparison. */
        CaseSensitive    = 1,
        /** Match using case-insensitive comparison. */
        CaseInsensitive  = 2,
    };

} // namespace System::IO

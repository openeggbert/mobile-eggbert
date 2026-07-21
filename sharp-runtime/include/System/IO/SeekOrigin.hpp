// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::IO {

    /**
     * @brief Specifies the position in a stream to use for seeking.
     *
     * @note Status: Implemented
     */
    enum class SeekOrigin : int {
        /** Seek from the beginning of the stream. */
        Begin   = 0,
        /** Seek from the current position in the stream. */
        Current = 1,
        /** Seek from the end of the stream. */
        End     = 2
    };

} // namespace System::IO

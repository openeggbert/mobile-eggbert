// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Threading::Channels {

    /**
     * @brief Specifies the behavior to use when writing to a bounded channel that is already full.
     *
     * C++ counterpart of .NET System.Threading.Channels.BoundedChannelFullMode.
     */
    enum class BoundedChannelFullMode {
        /** Wait for space to be available in order to complete the write operation. */
        Wait,
        /** Remove and ignore the newest item in the channel to make room for the item being written. */
        DropNewest,
        /** Remove and ignore the oldest item in the channel to make room for the item being written. */
        DropOldest,
        /** Drop the item being written. */
        DropWrite,
    };

} // namespace System::Threading::Channels

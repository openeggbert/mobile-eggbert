// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Buffers {

    /** Indicates the status of an encoding or decoding operation. */
    enum class OperationStatus {
        /** The operation completed successfully. */
        Done                = 0,
        /** The output buffer was too small to hold the result. */
        DestinationTooSmall = 1,
        /** More input data is required to complete the operation. */
        NeedMoreData        = 2,
        /** The input data is invalid or malformed. */
        InvalidData         = 3,
    };

} // namespace System::Buffers

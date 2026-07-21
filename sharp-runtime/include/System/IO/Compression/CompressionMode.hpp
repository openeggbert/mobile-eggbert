// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::IO::Compression {

    /**
     * @brief Specifies whether to compress or decompress the underlying stream.
     *
     * Partial C++ counterpart of .NET System.IO.Compression.CompressionMode.
     *
     * @note Status: Implemented
     */
    enum class CompressionMode {
        /** Decompress the stream. */
        Decompress = 0,
        /** Compress the stream. */
        Compress   = 1
    };

} // namespace System::IO::Compression

// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::IO::Compression {

    /**
     * @brief Specifies the compression method used to compress an entry in a zip archive.
     *
     * The values correspond to the compression method values described in the ZIP File
     * Format Specification (APPNOTE.TXT section 4.4.5).
     *
     * C++ counterpart of .NET System.IO.Compression.ZipCompressionMethod.
     */
    enum class ZipCompressionMethod {
        /** The entry is stored (no compression). */
        Stored    = 0x0,
        /** The entry is compressed using the Deflate algorithm. */
        Deflate   = 0x8,
        /** The entry is compressed using the Deflate64 algorithm. */
        Deflate64 = 0x9,
    };

} // namespace System::IO::Compression

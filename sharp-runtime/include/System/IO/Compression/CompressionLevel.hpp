// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::IO::Compression {

    /**
     * @brief Specifies values that indicate whether a compression operation emphasizes
     * speed or compression size.
     *
     * This is an abstract concept and NOT the ZLib compression level. There may or may not
     * be any correspondence with a possible implementation-specific level parameter of the
     * deflater.
     *
     * C++ counterpart of .NET System.IO.Compression.CompressionLevel.
     */
    enum class CompressionLevel {
        /** The compression operation should balance compression speed and output size. */
        Optimal       = 0,
        /** The compression operation should complete as quickly as possible, even if the resulting file is not optimally compressed. */
        Fastest       = 1,
        /** No compression should be performed on the file. */
        NoCompression = 2,
        /** The compression operation should create output as small as possible, even if the operation takes a longer time to complete. */
        SmallestSize  = 3,
    };

} // namespace System::IO::Compression

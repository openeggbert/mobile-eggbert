// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::IO::Compression {

    /**
     * @brief Defines the compression algorithms that can be used for DeflateStream, GZipStream,
     * or ZLibStream.
     *
     * C++ counterpart of .NET System.IO.Compression.ZLibCompressionStrategy.
     */
    enum class ZLibCompressionStrategy {
        /** Used for normal data. */
        Default           = 0,
        /**
         * Used for data produced by a filter (or predictor). The effect of Filtered is to force
         * more Huffman coding and less string matching, intermediate between Default and
         * HuffmanOnly.
         */
        Filtered          = 1,
        /** Used to force Huffman encoding only (no string match). */
        HuffmanOnly       = 2,
        /** Used to limit match distances to one (run-length encoding); gives better compression for PNG image data. */
        RunLengthEncoding = 3,
        /** Prevents the use of dynamic Huffman codes, allowing for a simpler decoder for special applications. */
        Fixed             = 4,
    };

} // namespace System::IO::Compression

// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/IO/Compression/DeflateDecoder.hpp"

namespace System::IO::Compression {

    /**
     * @brief Provides methods to decode data compressed in the ZLib data format in a
     * streamless, non-allocating, and performant manner.
     *
     * C++ counterpart of .NET System.IO.Compression.ZLibDecoder. Internally composes a
     * DeflateDecoder configured for ZLib framing (zlib header/trailer).
     *
     * @note Status: IMPLEMENTED — requires system zlib (linked via ZLIB::ZLIB CMake target).
     */
    class ZLibDecoder {
    private:
        DeflateDecoder deflateDecoder_;
        bool disposed_ = false;

    public:
        /** Initializes a new ZLibDecoder. */
        ZLibDecoder();

        /** Frees and disposes unmanaged (zlib) resources. */
        void Dispose();

        /**
         * @brief Decompresses @p source into @p destination.
         * @return The status with which the operation finished.
         * @throws System::ObjectDisposedException if this decoder has been disposed.
         */
        OperationStatus Decompress(const bytecs* source, intcs sourceLength,
                                   bytecs* destination, intcs destinationLength,
                                   intcs& bytesConsumed, intcs& bytesWritten);

        /**
         * @brief Tries to decompress @p source into @p destination in one call using a
         * temporary ZLibDecoder.
         * @return true if decompression consumed all of @p source and completed successfully.
         */
        static bool TryDecompress(const bytecs* source, intcs sourceLength,
                                   bytecs* destination, intcs destinationLength,
                                   intcs& bytesWritten);
    };

} // namespace System::IO::Compression

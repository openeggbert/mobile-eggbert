// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/IO/Compression/DeflateEncoder.hpp"

namespace System::IO::Compression {

    /**
     * @brief Provides methods to encode data in a streamless, non-allocating, and performant
     * manner using the ZLib data format specification.
     *
     * C++ counterpart of .NET System.IO.Compression.ZLibEncoder. Internally composes a
     * DeflateEncoder configured for ZLib framing (zlib header/trailer).
     *
     * @note Status: IMPLEMENTED — requires system zlib (linked via ZLIB::ZLIB CMake target).
     */
    class ZLibEncoder {
    private:
        DeflateEncoder deflateEncoder_;
        bool disposed_ = false;

    public:
        /** Initializes a new ZLibEncoder using the default quality. */
        ZLibEncoder();
        /** Initializes a new ZLibEncoder using the specified quality (0-9, or -1 for default). */
        explicit ZLibEncoder(intcs quality);
        /** Initializes a new ZLibEncoder using the specified quality and window size. */
        ZLibEncoder(intcs quality, intcs windowLog);
        /** Initializes a new ZLibEncoder using the specified options. */
        explicit ZLibEncoder(const ZLibCompressionOptions& options);

        /** Frees and disposes unmanaged (zlib) resources. */
        void Dispose();

        /** Returns the maximum expected compressed length for the given input size. */
        static longcs GetMaxCompressedLength(longcs inputLength);

        /**
         * @brief Compresses @p source into @p destination.
         * @throws System::ObjectDisposedException if this encoder has been disposed.
         */
        OperationStatus Compress(const bytecs* source, intcs sourceLength,
                                 bytecs* destination, intcs destinationLength,
                                 intcs& bytesConsumed, intcs& bytesWritten, bool isFinalBlock);

        /** Flushes any pending output, ensuring output is produced for all processed input so far. */
        OperationStatus Flush(bytecs* destination, intcs destinationLength, intcs& bytesWritten);

        /** Tries to compress @p source into @p destination using the default quality and window size. */
        static bool TryCompress(const bytecs* source, intcs sourceLength,
                                 bytecs* destination, intcs destinationLength, intcs& bytesWritten);
        /** Tries to compress @p source into @p destination using the specified quality. */
        static bool TryCompress(const bytecs* source, intcs sourceLength,
                                 bytecs* destination, intcs destinationLength, intcs& bytesWritten,
                                 intcs quality);
        /** Tries to compress @p source into @p destination using the specified quality and window size. */
        static bool TryCompress(const bytecs* source, intcs sourceLength,
                                 bytecs* destination, intcs destinationLength, intcs& bytesWritten,
                                 intcs quality, intcs windowLog);
    };

} // namespace System::IO::Compression

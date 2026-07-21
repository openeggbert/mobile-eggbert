// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Buffers/OperationStatus.hpp"
#include "System/IO/Compression/ZLibCompressionOptions.hpp"

namespace System::IO::Compression {

    using SharpRuntime::bytecs;
    using SharpRuntime::intcs;
    using SharpRuntime::longcs;
    using System::Buffers::OperationStatus;

    struct ZLibDeflateEncoderState; ///< Opaque zlib deflate state; defined in DeflateEncoder.cpp.

    /**
     * @brief Provides methods to encode data in a streamless, non-allocating, and performant
     * manner using the Deflate data format specification.
     *
     * C++ counterpart of .NET System.IO.Compression.DeflateEncoder. Operates on raw
     * pointer+length buffers in place of .NET's ReadOnlySpan<byte>/Span<byte>.
     *
     * @note Status: IMPLEMENTED — requires system zlib (linked via ZLIB::ZLIB CMake target).
     */
    class DeflateEncoder {
    private:
        std::unique_ptr<ZLibDeflateEncoderState> state_;
        bool disposed_ = false;
        bool finished_ = false;

        static void ValidateQuality(intcs quality);
        static void ValidateWindowLog(intcs windowLog);

    public:
        /** Initializes a new DeflateEncoder using the default quality. */
        DeflateEncoder();
        /** Initializes a new DeflateEncoder using the specified quality (0-9, or -1 for default). */
        explicit DeflateEncoder(intcs quality);
        /** Initializes a new DeflateEncoder using the specified quality and window size. */
        DeflateEncoder(intcs quality, intcs windowLog);
        /** Initializes a new DeflateEncoder using the specified options. */
        explicit DeflateEncoder(const ZLibCompressionOptions& options);

        /**
         * @brief Initializes a new DeflateEncoder using the given zlib windowBits value and memLevel directly.
         *
         * Exposed publicly (unlike .NET's internal constructor) so GZipEncoder/ZLibEncoder can
         * compose a DeflateEncoder configured for their own framing; not part of the primary API.
         */
        DeflateEncoder(intcs quality, intcs windowBits, intcs memLevel, intcs strategy);

        ~DeflateEncoder();

        DeflateEncoder(const DeflateEncoder&) = delete;
        DeflateEncoder& operator=(const DeflateEncoder&) = delete;

        /** Frees and disposes unmanaged (zlib) resources. */
        void Dispose();

        /** Returns the maximum expected compressed length for the given input size. */
        static longcs GetMaxCompressedLength(longcs inputLength);

        /**
         * @brief Compresses @p source into @p destination.
         * @param isFinalBlock true to finalize the internal stream; false to allow postponing output.
         * @return The status with which the operation finished.
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

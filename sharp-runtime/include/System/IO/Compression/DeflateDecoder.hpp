// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Buffers/OperationStatus.hpp"

namespace System::IO::Compression {

    using SharpRuntime::bytecs;
    using SharpRuntime::intcs;
    using System::Buffers::OperationStatus;

    struct ZLibInflateState; ///< Opaque zlib inflate state; defined in DeflateDecoder.cpp.

    /**
     * @brief Provides methods to decode data compressed in the Deflate data format in a
     * streamless, non-allocating, and performant manner.
     *
     * C++ counterpart of .NET System.IO.Compression.DeflateDecoder. Operates on raw
     * pointer+length buffers in place of .NET's ReadOnlySpan<byte>/Span<byte>.
     *
     * @note Status: IMPLEMENTED — requires system zlib (linked via ZLIB::ZLIB CMake target).
     */
    class DeflateDecoder {
    private:
        std::unique_ptr<ZLibInflateState> state_;
        bool disposed_ = false;
        bool finished_ = false;

    public:
        /** Initializes a new DeflateDecoder using raw DEFLATE framing (no header/trailer). */
        DeflateDecoder();

        /**
         * @brief Initializes a new DeflateDecoder using the given zlib windowBits value directly.
         *
         * Exposed publicly (unlike .NET's internal constructor) so GZipDecoder/ZLibDecoder can
         * compose a DeflateDecoder configured for their own framing; not part of the primary API.
         */
        explicit DeflateDecoder(intcs windowBits);

        ~DeflateDecoder();

        DeflateDecoder(const DeflateDecoder&) = delete;
        DeflateDecoder& operator=(const DeflateDecoder&) = delete;

        /** Frees and disposes unmanaged (zlib) resources. */
        void Dispose();

        /**
         * @brief Decompresses @p source into @p destination.
         * @param source Pointer to the compressed source bytes.
         * @param sourceLength Number of bytes available at @p source.
         * @param destination Pointer to the destination buffer.
         * @param destinationLength Number of bytes available at @p destination.
         * @param bytesConsumed Set to the number of bytes read from @p source.
         * @param bytesWritten Set to the number of bytes written to @p destination.
         * @return The status with which the operation finished.
         * @throws System::ObjectDisposedException if this decoder has been disposed.
         */
        OperationStatus Decompress(const bytecs* source, intcs sourceLength,
                                   bytecs* destination, intcs destinationLength,
                                   intcs& bytesConsumed, intcs& bytesWritten);

        /**
         * @brief Tries to decompress @p source into @p destination in one call using a
         * temporary DeflateDecoder.
         * @return true if decompression consumed all of @p source and completed successfully.
         */
        static bool TryDecompress(const bytecs* source, intcs sourceLength,
                                   bytecs* destination, intcs destinationLength,
                                   intcs& bytesWritten);
    };

} // namespace System::IO::Compression

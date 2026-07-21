// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/IO/Stream.hpp"
#include "System/IO/Compression/CompressionMode.hpp"
#include <memory>

namespace System::IO::Compression {

    struct ZlibZLibState; ///< Opaque zlib state; defined in ZLibStream.cpp.

    /**
     * @brief Provides methods for compressing and decompressing streams using the ZLib format.
     *
     * Wraps a zlib inflate/deflate context. Unlike DeflateStream (raw DEFLATE) or GZipStream
     * (gzip framing), the ZLib format includes a 2-byte header and Adler-32 trailer.
     *
     * Partial C++ counterpart of .NET System.IO.Compression.ZLibStream.
     *
     * @note Status: IMPLEMENTED — requires system zlib (linked via ZLIB::ZLIB CMake target).
     */
    class ZLibStream : public Stream {
        Stream*                       inner_;
        CompressionMode               mode_;
        bool                          leaveOpen_;
        std::unique_ptr<ZlibZLibState> state_;

    public:
        /**
         * @brief Constructs a ZLibStream wrapping @p stream.
         *
         * @param stream    The underlying stream for compressed data.
         * @param mode      @c CompressionMode::Compress or @c ::Decompress.
         * @param leaveOpen When @c true the inner stream is not closed on destruction.
         *
         * @throws System::IO::IOException if zlib initialisation fails.
         */
        ZLibStream(Stream* stream, CompressionMode mode, bool leaveOpen = false);

        ~ZLibStream() override;

        /** @brief Returns @c true when mode is Decompress. */
        [[nodiscard]] bool getCanReadProperty()  const override;

        /** @brief Returns @c true when mode is Compress. */
        [[nodiscard]] bool getCanWriteProperty() const override;

        /** @brief Not supported — always throws NotSupportedException. */
        [[nodiscard]] SharpRuntime::intcs getLengthProperty() const override;

        /**
         * @brief Decompresses bytes from the inner stream into @p buffer.
         *
         * Only valid when mode is @c CompressionMode::Decompress.
         *
         * @param buffer Destination array.
         * @param offset Starting index in @p buffer.
         * @param count  Maximum number of bytes to read.
         * @return Number of bytes written; 0 when the compressed stream is exhausted.
         */
        SharpRuntime::intcs Read(SharpRuntime::bytecs* buffer,
                                 SharpRuntime::intcs   offset,
                                 SharpRuntime::intcs   count) override;

        /**
         * @brief Compresses @p count bytes from @p buffer and writes to the inner stream.
         *
         * Only valid when mode is @c CompressionMode::Compress.
         *
         * @param buffer Source array.
         * @param offset Starting index in @p buffer.
         * @param count  Number of bytes to compress.
         * @throws System::ArgumentNullException if buffer is null.
         * @throws System::ArgumentOutOfRangeException if offset or count is negative.
         */
        void Write(const SharpRuntime::bytecs* buffer,
                   SharpRuntime::intcs          offset,
                   SharpRuntime::intcs          count) override;

        /**
         * @brief Flushes any pending compressed data using @c Z_SYNC_FLUSH.
         *
         * No-op in Decompress mode.
         */
        void Flush() override;

        /**
         * @brief Finalises compression (@c Z_FINISH), writes remaining output, then
         *        optionally closes the inner stream.
         *
         * Safe to call multiple times.
         */
        void Close() override;
    };

} // namespace System::IO::Compression

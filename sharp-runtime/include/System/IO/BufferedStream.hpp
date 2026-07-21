// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <vector>
#include "System/ArgumentNullException.hpp"
#include "System/IO/Stream.hpp"

namespace System::IO {

    /**
     * @brief Adds buffering to reads and writes on another stream.
     *
     * C++ counterpart of .NET System.IO.BufferedStream. Maintains a single internal
     * byte buffer shared between reads and writes (never both at once) so that small,
     * frequent Read/Write calls on the wrapped stream are batched into fewer, larger
     * operations on the underlying stream.
     *
     * @note This port omits real .NET's "shadow buffer" write optimization (a transient,
     * up-to-2x-bufferSize allocation used purely to avoid an extra Write call in some
     * boundary cases) -- it is a performance nicety, not a correctness requirement, and
     * skipping it only costs one additional inner-stream Write call in the specific case
     * where a write is split across a buffer-full boundary. Async I/O is also out of
     * scope, matching this project's synchronous Stream design throughout.
     */
    class BufferedStream : public Stream {
        static constexpr SharpRuntime::intcs kDefaultBufferSize = 4096;

        Stream* inner_;
        bool    ownsStream_;
        SharpRuntime::intcs bufferSize_;
        bool    closed_ = false;

        // Buffer state is mutated by helpers called from const accessors (e.g. getLengthProperty()
        // must flush pending buffered writes before it can trust the inner stream's length), so it
        // is marked mutable rather than widening the base Stream contract's const accessors.
        mutable std::vector<SharpRuntime::bytecs> buffer_;
        mutable SharpRuntime::intcs readPos_  = 0;
        mutable SharpRuntime::intcs readLen_  = 0;
        mutable SharpRuntime::intcs writePos_ = 0;

        void ensureNotClosed() const;
        void ensureBufferAllocated() const;
        void flushWrite() const;
        void flushRead() const;
        void clearReadBufferBeforeWrite() const;
        SharpRuntime::intcs readFromBuffer(SharpRuntime::bytecs* dst, SharpRuntime::intcs offset, SharpRuntime::intcs count) const;

    public:
        /**
         * @brief Constructs a BufferedStream wrapping the given stream with the default 4096-byte buffer.
         * @throws System::ArgumentNullException if @p stream is null.
         */
        explicit BufferedStream(Stream* stream, bool ownsStream = false)
            : BufferedStream(stream, kDefaultBufferSize, ownsStream) {}

        /**
         * @brief Constructs a BufferedStream wrapping the given stream with a custom buffer size.
         * C++ counterpart of .NET BufferedStream(Stream, int).
         * @throws System::ArgumentNullException if @p stream is null.
         * @throws System::ArgumentOutOfRangeException if @p bufferSize is not positive.
         */
        BufferedStream(Stream* stream, SharpRuntime::intcs bufferSize, bool ownsStream = false);

        /** Destroys the BufferedStream, flushing any pending buffered writes and closing the inner stream when ownsStream is true. */
        ~BufferedStream() override;

        /** @brief Returns the size in bytes of the internal buffer. C++ counterpart of .NET BufferedStream.BufferSize. */
        [[nodiscard]] SharpRuntime::intcs getBufferSizeProperty() const { return bufferSize_; }
        /** @brief Returns the wrapped stream. C++ counterpart of .NET BufferedStream.UnderlyingStream. */
        [[nodiscard]] Stream* getUnderlyingStreamProperty() const { return inner_; }

        /** @brief Reads buffered/inner-stream bytes, batching small reads through the internal buffer. */
        SharpRuntime::intcs Read(SharpRuntime::bytecs buffer[],
                                 SharpRuntime::intcs  offset,
                                 SharpRuntime::intcs  count) override;

        /** @brief Writes bytes, batching small writes through the internal buffer. */
        void Write(const SharpRuntime::bytecs buffer[],
                   SharpRuntime::intcs         offset,
                   SharpRuntime::intcs         count) override;

        /** @brief Writes a single byte through the internal buffer. */
        void WriteByte(SharpRuntime::bytecs value) override;

        /** @brief Flushes any buffered write data to the inner stream, or discards/rewinds buffered read data. */
        void Flush() override;

        /** @brief Flushes pending buffered writes, then closes (and, if owned, disposes) the inner stream. */
        void Close() override;

        /** @brief Seeks the stream, correctly accounting for any buffered-but-unconsumed/unflushed data. */
        SharpRuntime::intcs Seek(SharpRuntime::intcs offset, SeekOrigin origin) override;

        /** @brief Sets the length of the inner stream, flushing any buffered writes first. */
        void SetLength(SharpRuntime::intcs value) override;

        /** @brief Returns the inner stream's length, flushing any buffered writes first so it is accurate. */
        [[nodiscard]] SharpRuntime::intcs getLengthProperty() const override;
        /** @brief Returns true if the inner stream supports reading. */
        [[nodiscard]] bool getCanReadProperty()  const override { return !closed_ && inner_->getCanReadProperty(); }
        /** @brief Returns true if the inner stream supports writing. */
        [[nodiscard]] bool getCanWriteProperty() const override { return !closed_ && inner_->getCanWriteProperty(); }
        /** @brief Returns true if the inner stream supports seeking. */
        [[nodiscard]] bool getCanSeekProperty() const override { return !closed_ && inner_->getCanSeekProperty(); }

        /** @brief Returns the logical position, accounting for buffered-but-unconsumed/unflushed data. */
        [[nodiscard]] SharpRuntime::intcs getPositionProperty() const override;
        /** @brief Seeks to the given absolute position. */
        void setPositionProperty(SharpRuntime::intcs value) override;
    };

} // namespace System::IO

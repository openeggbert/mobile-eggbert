// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <vector>
#include "System/IO/Stream.hpp"

namespace System::IO
{
    /**
     * @brief A stream backed by an in-memory byte buffer.
     *
     * Used to wrap data loaded from sources such as Android APK assets.
     *
     * @note Status: IMPLEMENTED
     */
    class MemoryStream : public Stream
    {
    private:
        std::vector<bytecs> data_;
        intcs position_;
        bool  writable_;
        bool  isOpen_ = true;

        /**
         * @brief Throws System::ObjectDisposedException if this stream has been closed.
         *
         * Matches real .NET's MemoryStream.EnsureNotClosed(), called from Read/Write/
         * the Length getter.
         */
        void ensureNotClosed() const;

    public:
        /**
         * @brief Creates a read-only MemoryStream over a byte buffer.
         *
         * @param buffer Pointer to the source bytes.
         * @param size   Number of bytes to copy.
         */
        MemoryStream(const bytecs* buffer, intcs size);

        ~MemoryStream() override = default;

        /** Reads up to count bytes into buffer at offset; returns bytes actually read. */
        intcs Read(bytecs buffer[], intcs offset, intcs count) override;
        /**
         * Writes count bytes from buffer at offset into the memory buffer.
         * @throws System::ArgumentNullException if buffer is null.
         * @throws System::ArgumentOutOfRangeException if offset or count is negative.
         */
        void  Write(const bytecs buffer[], intcs offset, intcs count) override;
        /**
         * @brief Marks this stream closed. Matches MemoryStream.Dispose(), which deliberately
         * leaves the buffer and position untouched -- but Read/Write/Length now correctly throw
         * System::ObjectDisposedException after this call, matching real .NET's _isOpen guard
         * (this port previously had no disposed-state tracking at all).
         */
        void  Close() override;

        /** Returns the length of the in-memory buffer in bytes. */
        [[nodiscard]] intcs getLengthProperty()   const override;
    };
}

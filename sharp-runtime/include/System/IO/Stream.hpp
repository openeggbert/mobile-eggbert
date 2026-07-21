// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <cstddef>

#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/IO/SeekOrigin.hpp"

namespace System::IO
{
    using SharpRuntime::bytecs;
    using SharpRuntime::intcs;

    /**
     * <summary>
     * Provides a generic view of a sequence of bytes.
     * 
     * Lightweight subset of the .NET Stream API intended for practical use
     * in source ports. Derived classes must implement Read(), Close(), and
     * getLengthProperty(); Write() and WriteByte() may optionally be overridden.
     * </summary>
     */
    class Stream
    {
    public:
        virtual ~Stream() = default;

        /**
         * Reads up to @p count bytes from the stream into @p buffer starting at @p offset.
         * @param buffer Destination byte buffer.
         * @param offset Byte offset within @p buffer at which to begin writing.
         * @param count Maximum number of bytes to read.
         * @return Number of bytes actually read; 0 at end-of-stream.
         */
        virtual intcs Read(bytecs buffer[], intcs offset, intcs count) = 0;

        /** Closes the stream and releases any associated resources. */
        virtual void Close() = 0;

        /** Returns the total length of the stream in bytes. */
        [[nodiscard]] virtual intcs getLengthProperty() const = 0;

        /**
         * Writes @p count bytes from @p buffer (starting at @p offset) to the stream.
         * Default implementation throws NotSupportedException; override in writable streams.
         * @param buffer Source byte buffer.
         * @param offset Byte offset within @p buffer to begin reading.
         * @param count Number of bytes to write.
         */
        virtual void Write(const bytecs buffer[], intcs offset, intcs count);

        /**
         * Writes a single byte @p value to the stream.
         * Default implementation throws NotSupportedException; override in writable streams.
         * @param value Byte to write.
         */
        virtual void WriteByte(bytecs value);

        /** Returns true if this stream supports writing. */
        [[nodiscard]] virtual bool getCanWriteProperty() const { return false; }

        /** Returns true if this stream supports reading. */
        [[nodiscard]] virtual bool getCanReadProperty()  const { return true; }

        /**
         * Returns the current position within the stream.
         * Default implementation throws NotSupportedException; override in seekable streams.
         */
        [[nodiscard]] virtual intcs getPositionProperty() const;

        /**
         * Sets the current position within the stream.
         * Default implementation throws NotSupportedException; override in seekable streams.
         * @param value The new position within the stream.
         */
        virtual void setPositionProperty(intcs value);

        /** Returns true if this stream supports seeking. */
        [[nodiscard]] virtual bool getCanSeekProperty() const { return false; }

        /**
         * Sets the position within the stream relative to @p origin.
         * Default implementation is expressed in terms of getPositionProperty()/setPositionProperty()/
         * getLengthProperty(), so it works automatically for any stream that supports Position;
         * streams that don't support seeking inherit the NotSupportedException thrown by those.
         * @param offset Byte offset relative to @p origin.
         * @param origin Reference point from which to seek.
         * @return The new position within the stream.
         */
        virtual intcs Seek(intcs offset, SeekOrigin origin);

        /**
         * Sets the length of the stream.
         * Default implementation throws NotSupportedException; override in streams that can resize.
         * @param value The desired length, in bytes.
         */
        virtual void SetLength(intcs value);

        /**
         * Reads a single byte from the stream and advances the position by one byte.
         * Default implementation is expressed in terms of Read(); override for a more efficient path.
         * @return The byte read, or -1 if at the end of the stream.
         */
        virtual intcs ReadByte();

        /** Flushes any buffered data to the underlying device. Default is a no-op. */
        virtual void Flush() {}
    };
}

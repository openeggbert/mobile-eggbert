// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <string>
#include <vector>

#include "System/IO/Stream.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::IO
{
    using SharpRuntime::bytecs;
    using SharpRuntime::intcs;
    using SharpRuntime::shortcs;
    using SharpRuntime::ushortcs;
    using SharpRuntime::longcs;
    using SharpRuntime::Single;

    /**
     * @brief Reads primitive data types from a Stream in binary, little-endian format.
     *
     * C++ counterpart of the .NET System.IO.BinaryReader class. This is a byte-oriented
     * subset: .NET's Encoding/Decoder-based char methods (ReadChar, PeekChar, ReadChars,
     * Read(char[])) are not implemented, since this codebase's Stream has no character
     * encoding layer — a deliberate simplification, not a silent gap.
     */
    class BinaryReader
    {
    private:
        Stream* stream_;
        bool leaveOpen_;
        bool disposed_ = false;

        void ThrowIfDisposed() const;
        void ReadBytesExact(bytecs* buf, intcs count);

    public:
        /** Initializes a new BinaryReader over @p stream, optionally leaving it open on Dispose. */
        explicit BinaryReader(Stream* stream, bool leaveOpen = false);
        virtual ~BinaryReader();

        /** Gets the underlying stream. */
        [[nodiscard]] Stream* getBaseStreamProperty() const { return stream_; }

        /** Reads one byte and advances the stream position. @throws System::IO::EndOfStreamException at end of stream. */
        [[nodiscard]] virtual bytecs ReadByte();

        /** Reads a signed byte. */
        [[nodiscard]] virtual int8_t ReadSByte();

        /** Reads a 2-byte signed integer (little-endian). */
        [[nodiscard]] virtual shortcs ReadInt16();

        /** Reads a 2-byte unsigned integer (little-endian). */
        [[nodiscard]] virtual ushortcs ReadUInt16();

        /** Reads a 4-byte signed integer (little-endian). */
        [[nodiscard]] virtual intcs ReadInt32();

        /** Reads a 4-byte unsigned integer (little-endian). */
        [[nodiscard]] virtual uint32_t ReadUInt32();

        /** Reads an 8-byte signed integer (little-endian). */
        [[nodiscard]] virtual longcs ReadInt64();

        /** Reads an 8-byte unsigned integer (little-endian). */
        [[nodiscard]] virtual uint64_t ReadUInt64();

        /** Reads a 4-byte IEEE 754 float (little-endian). */
        [[nodiscard]] virtual Single ReadSingle();

        /** Reads a 4-byte IEEE 754 double (little-endian). */
        [[nodiscard]] virtual double ReadDouble();

        /** Reads a boolean (one byte; non-zero = true). */
        [[nodiscard]] virtual bool ReadBoolean();

        /** Reads a UTF-8 string prefixed with its 7-bit encoded length. */
        [[nodiscard]] virtual std::string ReadString();

        /** Reads exactly count bytes into the caller-supplied buffer. */
        virtual intcs Read(bytecs buffer[], intcs offset, intcs count);

        /**
         * @brief Reads the specified number of bytes into a new array.
         *
         * C++ counterpart of .NET BinaryReader.ReadBytes(int).
         * If the end of the stream is reached before @p count bytes are read, the
         * returned vector is trimmed to the number of bytes actually read (no exception).
         * @param count The number of bytes to read.
         * @return A vector containing the bytes read (may be shorter than @p count at EOF).
         * @throws System::ArgumentOutOfRangeException if @p count is negative.
         */
        [[nodiscard]] virtual std::vector<bytecs> ReadBytes(intcs count);

        /**
         * @brief Reads a 32-bit integer encoded 7 bits at a time (little-endian-first, high bit = continuation).
         *
         * C++ counterpart of .NET BinaryReader.Read7BitEncodedInt().
         * @return The decoded 32-bit integer.
         * @throws System::FormatException if the encoding uses more bytes than a 32-bit value requires.
         */
        intcs Read7BitEncodedInt();

        /** Closes the reader and, unless leaveOpen was set, the underlying stream. */
        virtual void Close();
    };
}

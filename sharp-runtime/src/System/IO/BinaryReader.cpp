// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/IO/BinaryReader.hpp"

#include <cstring>

#include "System/ArgumentException.hpp"
#include "System/ArgumentNullException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/FormatException.hpp"
#include "System/ObjectDisposedException.hpp"
#include "System/IO/EndOfStreamException.hpp"
#include "System/IO/IOException.hpp"

namespace System::IO
{
    BinaryReader::BinaryReader(Stream* stream, bool leaveOpen)
        : stream_(stream), leaveOpen_(leaveOpen)
    {
        if (!stream_)
            throw System::ArgumentNullException("stream");
        if (!stream_->getCanReadProperty())
            throw System::ArgumentException("Stream was not readable.");
    }

    BinaryReader::~BinaryReader()
    {
        if (!disposed_ && !leaveOpen_ && stream_)
            stream_->Close();
    }

    void BinaryReader::ThrowIfDisposed() const
    {
        if (disposed_)
            throw System::ObjectDisposedException("BinaryReader", "Cannot access a closed file.");
    }

    void BinaryReader::ReadBytesExact(bytecs* buf, intcs count)
    {
        ThrowIfDisposed();
        intcs total = 0;
        while (total < count)
        {
            intcs n = stream_->Read(buf, total, count - total);
            if (n == 0) throw System::IO::EndOfStreamException();
            total += n;
        }
    }

    bytecs BinaryReader::ReadByte()
    {
        bytecs b;
        ReadBytesExact(&b, 1);
        return b;
    }

    int8_t BinaryReader::ReadSByte()
    {
        return static_cast<int8_t>(ReadByte());
    }

    shortcs BinaryReader::ReadInt16()
    {
        bytecs buf[2];
        ReadBytesExact(buf, 2);
        return static_cast<shortcs>(
            static_cast<uint16_t>(buf[0]) |
            (static_cast<uint16_t>(buf[1]) << 8));
    }

    ushortcs BinaryReader::ReadUInt16()
    {
        return static_cast<ushortcs>(ReadInt16());
    }

    intcs BinaryReader::ReadInt32()
    {
        bytecs buf[4];
        ReadBytesExact(buf, 4);
        return static_cast<intcs>(
            static_cast<uint32_t>(buf[0])        |
            (static_cast<uint32_t>(buf[1]) << 8) |
            (static_cast<uint32_t>(buf[2]) << 16)|
            (static_cast<uint32_t>(buf[3]) << 24));
    }

    uint32_t BinaryReader::ReadUInt32()
    {
        return static_cast<uint32_t>(ReadInt32());
    }

    longcs BinaryReader::ReadInt64()
    {
        uint64_t lo = ReadUInt32();
        uint64_t hi = ReadUInt32();
        return static_cast<longcs>((hi << 32) | lo);
    }

    uint64_t BinaryReader::ReadUInt64()
    {
        return static_cast<uint64_t>(ReadInt64());
    }

    Single BinaryReader::ReadSingle()
    {
        uint32_t raw = ReadUInt32();
        Single result;
        std::memcpy(&result, &raw, sizeof(Single));
        return result;
    }

    double BinaryReader::ReadDouble()
    {
        uint64_t raw = ReadUInt64();
        double result;
        std::memcpy(&result, &raw, sizeof(double));
        return result;
    }

    bool BinaryReader::ReadBoolean()
    {
        return ReadByte() != 0;
    }

    intcs BinaryReader::Read7BitEncodedInt()
    {
        // Unlike writing, we can't delegate to the 64-bit read: we want to stop
        // consuming bytes as soon as an overflow is detected, matching .NET exactly.
        uint32_t result = 0;
        bytecs byteReadJustNow;

        constexpr int MaxBytesWithoutOverflow = 4;
        for (int shift = 0; shift < MaxBytesWithoutOverflow * 7; shift += 7)
        {
            byteReadJustNow = ReadByte();
            result |= (static_cast<uint32_t>(byteReadJustNow) & 0x7Fu) << shift;

            if (byteReadJustNow <= 0x7Fu)
                return static_cast<intcs>(result);
        }

        // The 5th byte: we've already read 28 bits, so this byte must fit in 4 bits
        // and must not have the continuation bit set.
        byteReadJustNow = ReadByte();
        if (byteReadJustNow > 0b1111u)
            throw System::FormatException("Too many bytes in what should have been a 7-bit encoded integer.");

        result |= static_cast<uint32_t>(byteReadJustNow) << (MaxBytesWithoutOverflow * 7);
        return static_cast<intcs>(result);
    }

    std::string BinaryReader::ReadString()
    {
        // .NET BinaryWriter writes strings with a 7-bit-encoded length prefix.
        intcs length = Read7BitEncodedInt();
        if (length < 0)
            throw System::IO::IOException("Invalid string length.");
        if (length == 0) return {};
        std::string result(static_cast<std::size_t>(length), '\0');
        ReadBytesExact(reinterpret_cast<bytecs*>(result.data()), length);
        return result;
    }

    intcs BinaryReader::Read(bytecs buffer[], intcs offset, intcs count)
    {
        ThrowIfDisposed();
        System::ArgumentOutOfRangeException::ThrowIfNegative(offset, "offset");
        System::ArgumentOutOfRangeException::ThrowIfNegative(count, "count");
        return stream_->Read(buffer, offset, count);
    }

    std::vector<bytecs> BinaryReader::ReadBytes(intcs count)
    {
        ThrowIfDisposed();
        System::ArgumentOutOfRangeException::ThrowIfNegative(count, "count");
        if (count == 0) return {};

        std::vector<bytecs> result(static_cast<std::size_t>(count));
        intcs total = 0;
        while (total < count)
        {
            intcs n = stream_->Read(result.data(), total, count - total);
            if (n == 0) break; // .NET trims the result instead of throwing here.
            total += n;
        }
        if (total != count) result.resize(static_cast<std::size_t>(total));
        return result;
    }

    void BinaryReader::Close()
    {
        if (!disposed_)
        {
            if (!leaveOpen_ && stream_)
                stream_->Close();
            disposed_ = true;
        }
    }
}

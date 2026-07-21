// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/IO/UnmanagedMemoryAccessor.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentNullException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/NotSupportedException.hpp"
#include "System/ObjectDisposedException.hpp"

#include <cstring>

namespace System::IO {

    UnmanagedMemoryAccessor::UnmanagedMemoryAccessor(bytecs* buffer, longcs capacity)
    {
        Initialize(buffer, capacity, FileAccess::Read);
    }

    UnmanagedMemoryAccessor::UnmanagedMemoryAccessor(bytecs* buffer, longcs capacity, FileAccess access)
    {
        Initialize(buffer, capacity, access);
    }

    void UnmanagedMemoryAccessor::Initialize(bytecs* buffer, longcs capacity, FileAccess access)
    {
        if (buffer == nullptr) throw System::ArgumentNullException("buffer");
        if (capacity < 0) throw System::ArgumentOutOfRangeException("capacity", "Non-negative number required.");

        buffer_   = buffer;
        capacity_ = capacity;
        canRead_  = access == FileAccess::Read || access == FileAccess::ReadWrite;
        canWrite_ = access == FileAccess::Write || access == FileAccess::ReadWrite;
        isOpen_   = true;
    }

    void UnmanagedMemoryAccessor::EnsureReadable(longcs position, longcs size) const
    {
        if (!isOpen_) throw System::ObjectDisposedException("UnmanagedMemoryAccessor");
        if (!canRead_) throw System::NotSupportedException("Accessor does not support reading.");
        if (position < 0) throw System::ArgumentOutOfRangeException("position", "Non-negative number required.");
        if (position > capacity_ - size) {
            if (position >= capacity_)
                throw System::ArgumentOutOfRangeException("position", "The position may not be greater or equal to the capacity of the accessor.");
            throw System::ArgumentException("There are not enough bytes remaining in the accessor to read at this position.", "position");
        }
    }

    void UnmanagedMemoryAccessor::EnsureWritable(longcs position, longcs size) const
    {
        if (!isOpen_) throw System::ObjectDisposedException("UnmanagedMemoryAccessor");
        if (!canWrite_) throw System::NotSupportedException("Accessor does not support writing.");
        if (position < 0) throw System::ArgumentOutOfRangeException("position", "Non-negative number required.");
        if (position > capacity_ - size) {
            if (position >= capacity_)
                throw System::ArgumentOutOfRangeException("position", "The position may not be greater or equal to the capacity of the accessor.");
            throw System::ArgumentException("There are not enough bytes remaining in the accessor to write at this position.", "position");
        }
    }

    bytecs UnmanagedMemoryAccessor::ReadByte(longcs position) const
    {
        EnsureReadable(position, 1);
        return buffer_[position];
    }

    char16_t UnmanagedMemoryAccessor::ReadChar(longcs position) const
    {
        return static_cast<char16_t>(ReadInt16(position));
    }

    shortcs UnmanagedMemoryAccessor::ReadInt16(longcs position) const
    {
        EnsureReadable(position, 2);
        shortcs v;
        std::memcpy(&v, buffer_ + position, sizeof(v));
        return v;
    }

    intcs UnmanagedMemoryAccessor::ReadInt32(longcs position) const
    {
        EnsureReadable(position, 4);
        intcs v;
        std::memcpy(&v, buffer_ + position, sizeof(v));
        return v;
    }

    longcs UnmanagedMemoryAccessor::ReadInt64(longcs position) const
    {
        EnsureReadable(position, 8);
        longcs v;
        std::memcpy(&v, buffer_ + position, sizeof(v));
        return v;
    }

    float UnmanagedMemoryAccessor::ReadSingle(longcs position) const
    {
        EnsureReadable(position, 4);
        float v;
        std::memcpy(&v, buffer_ + position, sizeof(v));
        return v;
    }

    double UnmanagedMemoryAccessor::ReadDouble(longcs position) const
    {
        EnsureReadable(position, 8);
        double v;
        std::memcpy(&v, buffer_ + position, sizeof(v));
        return v;
    }

    void UnmanagedMemoryAccessor::Write(longcs position, bytecs value)
    {
        EnsureWritable(position, 1);
        buffer_[position] = value;
    }

    void UnmanagedMemoryAccessor::Write(longcs position, shortcs value)
    {
        EnsureWritable(position, 2);
        std::memcpy(buffer_ + position, &value, sizeof(value));
    }

    void UnmanagedMemoryAccessor::Write(longcs position, intcs value)
    {
        EnsureWritable(position, 4);
        std::memcpy(buffer_ + position, &value, sizeof(value));
    }

    void UnmanagedMemoryAccessor::Write(longcs position, longcs value)
    {
        EnsureWritable(position, 8);
        std::memcpy(buffer_ + position, &value, sizeof(value));
    }

    void UnmanagedMemoryAccessor::Write(longcs position, float value)
    {
        EnsureWritable(position, 4);
        std::memcpy(buffer_ + position, &value, sizeof(value));
    }

    void UnmanagedMemoryAccessor::Write(longcs position, double value)
    {
        EnsureWritable(position, 8);
        std::memcpy(buffer_ + position, &value, sizeof(value));
    }

} // namespace System::IO

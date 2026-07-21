// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/IO/UnmanagedMemoryStream.hpp"
#include "System/ArgumentNullException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/NotSupportedException.hpp"
#include "System/ObjectDisposedException.hpp"
#include "System/IO/IOException.hpp"

#include <algorithm>
#include <cstring>

namespace System::IO {

    UnmanagedMemoryStream::UnmanagedMemoryStream(bytecs* pointer, intcs length)
    {
        Initialize(pointer, length, length, FileAccess::Read);
    }

    UnmanagedMemoryStream::UnmanagedMemoryStream(bytecs* pointer, intcs length, intcs capacity, FileAccess access)
    {
        Initialize(pointer, length, capacity, access);
    }

    void UnmanagedMemoryStream::Initialize(bytecs* pointer, intcs length, intcs capacity, FileAccess access)
    {
        if (pointer == nullptr) throw System::ArgumentNullException("pointer");
        if (length < 0) throw System::ArgumentOutOfRangeException("length", "Non-negative number required.");
        if (capacity < 0) throw System::ArgumentOutOfRangeException("capacity", "Non-negative number required.");
        if (length > capacity) throw System::ArgumentOutOfRangeException("length", "The length cannot be greater than the capacity.");

        buffer_   = pointer;
        length_   = length;
        capacity_ = capacity;
        access_   = access;
        position_ = 0;
        isOpen_   = true;
    }

    namespace {
        void validateBufferArguments(const bytecs* buffer, intcs offset, intcs count)
        {
            if (buffer == nullptr) throw System::ArgumentNullException("buffer");
            if (offset < 0) throw System::ArgumentOutOfRangeException("offset", "Non-negative number required.");
            if (count < 0) throw System::ArgumentOutOfRangeException("count", "Non-negative number required.");
        }
    }

    // Verified against UnmanagedMemoryStream.cs's EnsureNotClosed()/EnsureReadable()/
    // EnsureWriteable(): real .NET checks isOpen FIRST, throwing ObjectDisposedException
    // ("Cannot access a closed Stream.") for a closed stream, and only checks CanRead/CanWrite
    // (throwing NotSupportedException) once it's confirmed still open. This port's
    // getCanReadProperty()/getCanWriteProperty() fold "is closed" into the same boolean as
    // "was never readable/writable", so Read()/Write() after Close() previously threw
    // NotSupportedException instead of ObjectDisposedException -- the wrong exception type for
    // a closed-object access, indistinguishable from a stream that was simply constructed
    // read-only or write-only.
    intcs UnmanagedMemoryStream::Read(bytecs buffer[], intcs offset, intcs count)
    {
        if (!isOpen_) throw System::ObjectDisposedException("Cannot access a closed Stream.");
        if (!getCanReadProperty()) throw System::NotSupportedException("Stream does not support reading.");
        validateBufferArguments(buffer, offset, count);

        const intcs remaining = length_ - position_;
        const intcs toRead = std::min(count, remaining);
        if (toRead <= 0) return 0;

        std::memcpy(buffer + offset, buffer_ + position_, static_cast<size_t>(toRead));
        position_ += toRead;
        return toRead;
    }

    void UnmanagedMemoryStream::Write(const bytecs buffer[], intcs offset, intcs count)
    {
        if (!isOpen_) throw System::ObjectDisposedException("Cannot access a closed Stream.");
        if (!getCanWriteProperty()) throw System::NotSupportedException("Stream does not support writing.");
        validateBufferArguments(buffer, offset, count);
        if (count == 0) return;
        // Position can legally be set arbitrarily far past the end (setPositionProperty only
        // rejects negative values, matching real .NET's own Position setter). That means
        // position_+count (both intcs/int32) can genuinely signed-overflow -- confirmed real UB
        // via a standalone UBSan repro on the identical pattern in Span<T>::Slice (ticket
        // 265/1487) -- and worse than "just UB": a wrapped negative sum would silently compare
        // as <= capacity_, bypassing the capacity check entirely, so the memcpy below would
        // write through buffer_+position_ with position_ wildly beyond the actual unmanaged
        // allocation -- a real out-of-bounds write, not just a wrong-answer bug. Computed here
        // in int64_t (always wide enough to hold the sum of two int32 values) to avoid the UB,
        // matching real .NET's own UnmanagedMemoryStream.WriteCore, which computes this exact
        // sum in `long` and explicitly checks for overflow before the separate capacity check.
        int64_t newPosition64 = static_cast<int64_t>(position_) + static_cast<int64_t>(count);
        if (newPosition64 > static_cast<int64_t>(capacity_)) {
            throw System::NotSupportedException("Unable to expand length of this stream beyond its capacity.");
        }

        std::memcpy(buffer_ + position_, buffer + offset, static_cast<size_t>(count));
        position_ = static_cast<intcs>(newPosition64);
        if (position_ > length_) length_ = position_;
    }

    void UnmanagedMemoryStream::WriteByte(bytecs value)
    {
        Write(&value, 0, 1);
    }

    void UnmanagedMemoryStream::setPositionProperty(intcs value)
    {
        if (value < 0) throw System::ArgumentOutOfRangeException("value", "Non-negative number required.");
        position_ = value;
    }

    void UnmanagedMemoryStream::SetLength(intcs value)
    {
        if (value < 0) throw System::ArgumentOutOfRangeException("value", "Non-negative number required.");
        if (!isOpen_) throw System::ObjectDisposedException("Cannot access a closed Stream.");
        if (!getCanWriteProperty()) throw System::NotSupportedException("Stream does not support writing.");
        if (value > capacity_) {
            throw IOException("Unable to expand length of this stream beyond its capacity.");
        }
        length_ = value;
        if (position_ > value) position_ = value;
    }

} // namespace System::IO

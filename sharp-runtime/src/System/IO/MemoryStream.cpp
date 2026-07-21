// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/IO/MemoryStream.hpp"
#include "System/ArgumentNullException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/IO/IOException.hpp"
#include "System/NotSupportedException.hpp"
#include "System/ObjectDisposedException.hpp"
#include <algorithm>

namespace System::IO
{
    MemoryStream::MemoryStream()
        : position_(0), writable_(true) {}

    MemoryStream::MemoryStream(const bytecs* buffer, intcs size)
        : data_(buffer, buffer + size), position_(0), writable_(false) {}

    void MemoryStream::ensureNotClosed() const
    {
        if (!isOpen_) throw System::ObjectDisposedException("Cannot access a closed Stream.");
    }

    // Verified against MemoryStream.cs's Read()/ValidateBufferArguments: real .NET throws
    // ArgumentNullException for a null buffer and ArgumentOutOfRangeException for a negative
    // offset/count, matching FileStream::Read's existing validation in this codebase. This
    // previously returned 0 (indistinguishable from "stream at EOF") for every one of these
    // cases instead of throwing -- a caller passing a negative count would silently get "0
    // bytes read" instead of an error.
    intcs MemoryStream::Read(bytecs buffer[], intcs offset, intcs count)
    {
        ensureNotClosed();
        if (buffer == nullptr) throw System::ArgumentNullException("buffer");
        if (offset < 0) throw System::ArgumentOutOfRangeException("offset", "Non-negative number required.");
        if (count < 0) throw System::ArgumentOutOfRangeException("count", "Non-negative number required.");
        const intcs remaining = static_cast<intcs>(data_.size()) - position_;
        const intcs toRead = std::min(count, remaining);
        if (toRead <= 0) return 0;
        std::copy(data_.begin() + position_, data_.begin() + position_ + toRead, buffer + offset);
        position_ += toRead;
        return toRead;
    }

    void MemoryStream::Write(const bytecs buffer[], intcs offset, intcs count)
    {
        ensureNotClosed();
        // Verified against MemoryStream.cs's Write()/EnsureWriteable(): real .NET throws
        // NotSupportedException when !CanWrite, matching this class's own SetLength(). This
        // port previously just returned, silently dropping every byte the caller thought it
        // had written.
        if (!writable_) throw System::NotSupportedException("Stream does not support writing.");
        // Verified against MemoryStream.cs's Write()/ValidateBufferArguments: real .NET throws
        // ArgumentNullException for a null buffer and ArgumentOutOfRangeException for a
        // negative offset/count, matching Read()'s validation above. This previously only
        // checked `buffer == nullptr || count <= 0` and silently returned for everything else
        // -- a negative offset reached std::copy(buffer + offset, ...) unchecked, reading
        // before the caller's array (confirmed real out-of-bounds read via a standalone ASan
        // repro), not just a silent no-op.
        if (buffer == nullptr) throw System::ArgumentNullException("buffer");
        if (offset < 0) throw System::ArgumentOutOfRangeException("offset", "Non-negative number required.");
        if (count < 0) throw System::ArgumentOutOfRangeException("count", "Non-negative number required.");
        if (count == 0) return;
        // Position can legally be set arbitrarily far past the end (setPositionProperty only
        // rejects negative values, matching real .NET's own Position setter, which allows
        // seeking past Length -- the resize just happens lazily on the next Write). That means
        // position_+count (both intcs/int32) can genuinely signed-overflow -- confirmed real UB
        // via a standalone UBSan repro on the identical pattern in Span<T>::Slice (ticket
        // 265/1487) -- and worse than "just UB": computing it directly as intcs and wrapping to
        // a negative sum would silently compare as <= data_.size(), skipping the resize, so the
        // std::copy below would write through data_.begin()+position_ with position_ wildly
        // beyond the vector's actual allocation -- a real heap buffer overflow. Computed here in
        // int64_t (always wide enough to hold the sum of two int32 values without overflow) to
        // avoid the UB entirely, matching real .NET's own MemoryStream.Write, which computes
        // this exact sum and explicitly checks for the overflow, throwing
        // IOException(SR.IO_StreamTooLong).
        int64_t newLength64 = static_cast<int64_t>(position_) + static_cast<int64_t>(count);
        if (newLength64 > static_cast<int64_t>(SharpRuntime::INTCS_MAX))
            throw System::IO::IOException("Stream was too long.");
        intcs newLength = static_cast<intcs>(newLength64);
        if (newLength > static_cast<intcs>(data_.size()))
            data_.resize(static_cast<size_t>(newLength));
        std::copy(buffer + offset, buffer + offset + count, data_.begin() + position_);
        position_ = newLength;
    }

    void MemoryStream::WriteByte(bytecs value)
    {
        ensureNotClosed();
        if (!writable_) throw System::NotSupportedException("Stream does not support writing.");
        if (position_ >= static_cast<intcs>(data_.size())) data_.push_back(value);
        else data_[static_cast<size_t>(position_)] = value;
        ++position_;
    }

    // Verified against MemoryStream.cs's Dispose(bool): real .NET explicitly does NOT clear
    // the underlying buffer on Close/Dispose -- "Don't set buffer to null - allow TryGetBuffer,
    // GetBuffer & ToArray to work" -- nor does it reset the position. This previously cleared
    // both, contradicting this method's own doc comment ("no-op for MemoryStream") and
    // destroying data real .NET deliberately preserves after Close(). Marks the stream closed
    // (ticket 1724) so Read/Write/Seek/Length/Position now correctly throw
    // ObjectDisposedException afterward, matching real .NET's _isOpen guard -- while
    // GetBuffer()/ToArray() deliberately stay unguarded, matching real .NET's own
    // MemoryStream.cs (neither calls EnsureNotClosed()).
    void MemoryStream::Close()
    {
        isOpen_ = false;
    }

    intcs MemoryStream::getLengthProperty() const
    {
        ensureNotClosed();
        return static_cast<intcs>(data_.size());
    }

    intcs MemoryStream::getPositionProperty() const
    {
        ensureNotClosed();
        return position_;
    }

    void MemoryStream::setPositionProperty(intcs value)
    {
        ensureNotClosed();
        if (value < 0)
            throw System::ArgumentOutOfRangeException("value", "Non-negative number required.");
        position_ = value;
    }

    void MemoryStream::SetLength(intcs value)
    {
        ensureNotClosed();
        if (value < 0)
            throw System::ArgumentOutOfRangeException("value", "Non-negative number required.");
        if (!writable_)
            throw System::NotSupportedException("Stream does not support writing.");
        data_.resize(static_cast<size_t>(value), bytecs{0});
        if (position_ > value) position_ = value;
    }
}

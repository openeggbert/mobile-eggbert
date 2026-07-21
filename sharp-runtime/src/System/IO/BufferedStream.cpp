// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/IO/BufferedStream.hpp"

#include <algorithm>
#include <cstring>

#include "System/ArgumentOutOfRangeException.hpp"
#include "System/NotSupportedException.hpp"
#include "System/ObjectDisposedException.hpp"

namespace System::IO {

using SharpRuntime::bytecs;
using SharpRuntime::intcs;

BufferedStream::BufferedStream(Stream* stream, intcs bufferSize, bool ownsStream)
    : inner_(stream), ownsStream_(ownsStream), bufferSize_(bufferSize) {
    if (!inner_) throw System::ArgumentNullException("stream");
    if (bufferSize <= 0)
        throw System::ArgumentOutOfRangeException("bufferSize", "Positive number required.");
}

BufferedStream::~BufferedStream() {
    try { Close(); } catch (...) { /* destructors must not throw */ }
}

void BufferedStream::ensureNotClosed() const {
    if (closed_) throw System::ObjectDisposedException("Cannot access a closed stream.");
}

void BufferedStream::ensureBufferAllocated() const {
    if (buffer_.empty()) buffer_.resize(static_cast<size_t>(bufferSize_));
}

// Called by Write paths to push pending buffered write data to the inner stream. Real .NET's
// FlushWrite also calls the inner stream's Flush() afterward -- preserved here so a caller relying
// on OS-level durability after a buffered write sequence gets the same guarantee.
void BufferedStream::flushWrite() const {
    if (writePos_ > 0) {
        inner_->Write(buffer_.data(), 0, writePos_);
        writePos_ = 0;
        inner_->Flush();
    }
}

// Rewinds the inner stream past any buffered-but-unconsumed read bytes so the inner stream's
// position matches this stream's logical position again. Requires the inner stream be seekable
// (callers check getCanSeekProperty() first, matching real .NET's FlushRead/ClearReadBufferBeforeWrite split).
void BufferedStream::flushRead() const {
    if (readPos_ != readLen_) {
        inner_->Seek(readPos_ - readLen_, SeekOrigin::Current);
    }
    readPos_ = readLen_ = 0;
}

void BufferedStream::clearReadBufferBeforeWrite() const {
    if (readPos_ == readLen_) {
        readPos_ = readLen_ = 0;
        return;
    }
    if (!inner_->getCanSeekProperty()) {
        throw System::NotSupportedException(
            "Cannot write to a BufferedStream while the read buffer still has unconsumed data "
            "and the underlying stream does not support seeking.");
    }
    flushRead();
}

intcs BufferedStream::readFromBuffer(bytecs* dst, intcs offset, intcs count) const {
    intcs available = readLen_ - readPos_;
    if (available <= 0) return 0;
    intcs n = std::min(available, count);
    std::memcpy(dst + offset, buffer_.data() + readPos_, static_cast<size_t>(n));
    readPos_ += n;
    return n;
}

intcs BufferedStream::Read(bytecs buffer[], intcs offset, intcs count) {
    ensureNotClosed();

    intcs fromBuffer = readFromBuffer(buffer, offset, count);
    if (fromBuffer == count) return fromBuffer;

    intcs already = fromBuffer;
    if (fromBuffer > 0) {
        offset += fromBuffer;
        count  -= fromBuffer;
    }

    readPos_ = readLen_ = 0;
    if (writePos_ > 0) flushWrite();

    // A request at least as large as the buffer itself gets no benefit from an extra copy --
    // read straight into the caller's buffer, matching real .NET's bypass threshold.
    if (count >= bufferSize_) {
        return inner_->Read(buffer, offset, count) + already;
    }

    ensureBufferAllocated();
    readLen_ = inner_->Read(buffer_.data(), 0, bufferSize_);
    fromBuffer = readFromBuffer(buffer, offset, count);
    return fromBuffer + already;
}

void BufferedStream::Write(const bytecs buffer[], intcs offset, intcs count) {
    ensureNotClosed();
    if (writePos_ == 0) clearReadBufferBeforeWrite();

    while (count > 0) {
        // Nothing buffered yet and this chunk alone is >= the buffer size: write it straight
        // through rather than copying it into the buffer first.
        if (writePos_ == 0 && count >= bufferSize_) {
            inner_->Write(buffer, offset, count);
            return;
        }

        intcs space = bufferSize_ - writePos_;
        intcs n = std::min(space, count);
        if (n > 0) {
            ensureBufferAllocated();
            std::memcpy(buffer_.data() + writePos_, buffer + offset, static_cast<size_t>(n));
            writePos_ += n;
            offset += n;
            count -= n;
        }

        if (writePos_ == bufferSize_) {
            inner_->Write(buffer_.data(), 0, writePos_);
            writePos_ = 0;
        }
    }
}

void BufferedStream::WriteByte(bytecs value) {
    ensureNotClosed();
    if (writePos_ == 0) clearReadBufferBeforeWrite();
    ensureBufferAllocated();
    if (writePos_ >= bufferSize_) {
        inner_->Write(buffer_.data(), 0, writePos_);
        writePos_ = 0;
    }
    buffer_[static_cast<size_t>(writePos_++)] = value;
}

void BufferedStream::Flush() {
    ensureNotClosed();

    if (writePos_ > 0) {
        flushWrite();
        return;
    }

    if (readPos_ < readLen_) {
        // If the inner stream can't seek, rewinding past the unconsumed read bytes (flushRead)
        // would throw -- real .NET opts to silently ignore the Flush in that case rather than
        // losing data or breaking a caller that didn't expect Flush() to ever throw.
        if (inner_->getCanSeekProperty()) flushRead();
        if (inner_->getCanWriteProperty()) inner_->Flush();
        return;
    }

    if (inner_->getCanWriteProperty()) inner_->Flush();
    writePos_ = readPos_ = readLen_ = 0;
}

void BufferedStream::Close() {
    if (closed_) return;
    try {
        Flush();
    } catch (...) {
        closed_ = true;
        if (ownsStream_ && inner_) inner_->Close();
        throw;
    }
    closed_ = true;
    if (ownsStream_ && inner_) inner_->Close();
}

intcs BufferedStream::Seek(intcs offset, SeekOrigin origin) {
    ensureNotClosed();
    if (!inner_->getCanSeekProperty())
        throw System::NotSupportedException("Stream does not support seeking.");

    if (writePos_ > 0) {
        flushWrite();
        return inner_->Seek(offset, origin);
    }

    if (readLen_ - readPos_ > 0 && origin == SeekOrigin::Current) {
        // Account for the difference between this stream's logical position and the inner
        // stream's actual position caused by data already read into the buffer.
        offset -= (readLen_ - readPos_);
    }

    intcs oldPos = getPositionProperty();
    intcs newPos = inner_->Seek(offset, origin);

    // If the seek destination still falls within the data already cached in the buffer, keep
    // using the buffer instead of discarding it.
    intcs readPosCandidate = newPos - (oldPos - readPos_);
    if (readPosCandidate >= 0 && readPosCandidate < readLen_) {
        readPos_ = readPosCandidate;
        inner_->Seek(readLen_ - readPos_, SeekOrigin::Current);
    } else {
        readPos_ = readLen_ = 0;
    }
    return newPos;
}

void BufferedStream::SetLength(intcs value) {
    if (value < 0)
        throw System::ArgumentOutOfRangeException("value", "Non-negative number required.");
    ensureNotClosed();
    if (!inner_->getCanSeekProperty())
        throw System::NotSupportedException("Stream does not support seeking.");
    if (!inner_->getCanWriteProperty())
        throw System::NotSupportedException("Stream does not support writing.");
    Flush();
    inner_->SetLength(value);
}

intcs BufferedStream::getLengthProperty() const {
    ensureNotClosed();
    if (writePos_ > 0) flushWrite();
    return inner_->getLengthProperty();
}

intcs BufferedStream::getPositionProperty() const {
    ensureNotClosed();
    if (!inner_->getCanSeekProperty())
        throw System::NotSupportedException("Stream does not support seeking.");
    return inner_->getPositionProperty() + (readPos_ - readLen_ + writePos_);
}

void BufferedStream::setPositionProperty(intcs value) {
    if (value < 0)
        throw System::ArgumentOutOfRangeException("value", "Non-negative number required.");
    Seek(value, SeekOrigin::Begin);
}

} // namespace System::IO

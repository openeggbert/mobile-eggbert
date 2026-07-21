// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/IO/Compression/DeflateStream.hpp"
#include "System/ArgumentNullException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/IO/IOException.hpp"
#include "System/IO/InvalidDataException.hpp"
#include "System/NotSupportedException.hpp"
#include <zlib.h>
#include <string>

namespace System::IO::Compression {

// ---------------------------------------------------------------------------
// Opaque zlib state (never exposed in the header)
// ---------------------------------------------------------------------------

struct ZlibDeflateState {
    z_stream zs{};
    bool     initialized = false;
    bool     finished    = false;
    static constexpr int BUFSIZE = 65536;
    uint8_t  inbuf[BUFSIZE];
    uint8_t  outbuf[BUFSIZE];
};

// ---------------------------------------------------------------------------
// Constructor / destructor
// ---------------------------------------------------------------------------

DeflateStream::DeflateStream(Stream* stream, CompressionMode mode, bool leaveOpen)
    : inner_(stream), mode_(mode), leaveOpen_(leaveOpen),
      state_(std::make_unique<ZlibDeflateState>())
{
    if (mode_ == CompressionMode::Decompress) {
        // -MAX_WBITS: raw deflate, no zlib wrapper
        if (inflateInit2(&state_->zs, -MAX_WBITS) != Z_OK)
            throw System::IO::IOException("DeflateStream: inflateInit2 failed");
    } else {
        if (deflateInit2(&state_->zs, Z_DEFAULT_COMPRESSION, Z_DEFLATED,
                         -MAX_WBITS, 8, Z_DEFAULT_STRATEGY) != Z_OK)
            throw System::IO::IOException("DeflateStream: deflateInit2 failed");
    }
    state_->initialized = true;
}

// Best-effort, non-throwing (audit finding A-02, 2026-07-14): Close() can propagate the inner
// stream's I/O failure (e.g. a broken pipe or full disk during the final deflate flush) -- the
// correct behavior for an EXPLICIT Close() call, but destructors are implicitly noexcept in
// C++, so letting that exception escape here calls std::terminate instead of allowing the
// surrounding scope to unwind normally, confirmed via a standalone repro (a Write()-throwing
// inner stream terminated the process instead of propagating a catchable exception). Swallowed
// here; callers who need to observe a close failure must call Close() explicitly.
DeflateStream::~DeflateStream() { try { Close(); } catch (...) {} }

// ---------------------------------------------------------------------------
// Property accessors
// ---------------------------------------------------------------------------

bool DeflateStream::getCanReadProperty()  const { return mode_ == CompressionMode::Decompress; }
bool DeflateStream::getCanWriteProperty() const { return mode_ == CompressionMode::Compress;   }

SharpRuntime::intcs DeflateStream::getLengthProperty() const {
    throw System::NotSupportedException("This operation is not supported.");
}

// ---------------------------------------------------------------------------
// Read (decompress)
// ---------------------------------------------------------------------------

SharpRuntime::intcs DeflateStream::Read(SharpRuntime::bytecs* buffer,
                                        SharpRuntime::intcs   offset,
                                        SharpRuntime::intcs   count)
{
    // Verified against real .NET's Stream.ValidateBufferArguments, matching Write()'s validation
    // above and MemoryStream::Read (audit finding A-03, 2026-07-14). This previously validated
    // nothing: a null buffer reached inflate() unchecked (confirmed reaching zlib and throwing
    // InvalidDataException instead of ArgumentNullException), and a negative count/offset just
    // silently returned 0 via `count <= 0` instead of throwing.
    if (buffer == nullptr) throw System::ArgumentNullException("buffer");
    if (offset < 0) throw System::ArgumentOutOfRangeException("offset", "Non-negative number required.");
    if (count < 0) throw System::ArgumentOutOfRangeException("count", "Non-negative number required.");
    if (!state_ || !state_->initialized || state_->finished || count == 0) return 0;

    auto& s = *state_;
    s.zs.next_out  = reinterpret_cast<Bytef*>(buffer + offset);
    s.zs.avail_out = static_cast<uInt>(count);

    while (s.zs.avail_out > 0 && !s.finished) {
        if (s.zs.avail_in == 0) {
            SharpRuntime::intcs n = inner_->Read(s.inbuf, 0, ZlibDeflateState::BUFSIZE);
            if (n <= 0) break;
            s.zs.next_in  = s.inbuf;
            s.zs.avail_in = static_cast<uInt>(n);
        }
        int ret = inflate(&s.zs, Z_NO_FLUSH);
        if (ret == Z_STREAM_END) { s.finished = true; break; }
        if (ret != Z_OK && ret != Z_BUF_ERROR)
            throw System::IO::InvalidDataException("DeflateStream: invalid or corrupt compressed data (zlib error " + std::to_string(ret) + ")");
    }
    return count - static_cast<SharpRuntime::intcs>(s.zs.avail_out);
}

// ---------------------------------------------------------------------------
// Write (compress)
// ---------------------------------------------------------------------------

void DeflateStream::Write(const SharpRuntime::bytecs* buffer,
                          SharpRuntime::intcs          offset,
                          SharpRuntime::intcs          count)
{
    // Verified against real .NET's Stream.ValidateBufferArguments (throws
    // ArgumentNullException/ArgumentOutOfRangeException before touching the buffer). This
    // previously only checked `count <= 0` -- no null-buffer or negative-offset check at all,
    // so a negative offset reached deflate()'s next_in = buffer + offset unchecked, a confirmed
    // out-of-bounds read via a standalone ASan repro (not just a silent no-op).
    if (buffer == nullptr) throw System::ArgumentNullException("buffer");
    if (offset < 0) throw System::ArgumentOutOfRangeException("offset", "Non-negative number required.");
    if (count < 0) throw System::ArgumentOutOfRangeException("count", "Non-negative number required.");
    if (!state_ || !state_->initialized || count == 0) return;

    auto& s = *state_;
    s.zs.next_in  = const_cast<Bytef*>(reinterpret_cast<const Bytef*>(buffer + offset));
    s.zs.avail_in = static_cast<uInt>(count);

    while (s.zs.avail_in > 0) {
        s.zs.next_out  = s.outbuf;
        s.zs.avail_out = ZlibDeflateState::BUFSIZE;
        int ret = deflate(&s.zs, Z_NO_FLUSH);
        if (ret != Z_OK && ret != Z_BUF_ERROR)
            throw System::IO::IOException("DeflateStream: deflate error " + std::to_string(ret));
        SharpRuntime::intcs produced =
            ZlibDeflateState::BUFSIZE - static_cast<SharpRuntime::intcs>(s.zs.avail_out);
        if (produced > 0)
            inner_->Write(s.outbuf, 0, produced);
    }
}

// ---------------------------------------------------------------------------
// Flush
// ---------------------------------------------------------------------------

void DeflateStream::Flush() {
    if (!state_ || !state_->initialized || mode_ != CompressionMode::Compress) return;

    auto& s = *state_;
    s.zs.next_in  = nullptr;
    s.zs.avail_in = 0;
    int ret;
    do {
        s.zs.next_out  = s.outbuf;
        s.zs.avail_out = ZlibDeflateState::BUFSIZE;
        ret = deflate(&s.zs, Z_SYNC_FLUSH);
        SharpRuntime::intcs produced =
            ZlibDeflateState::BUFSIZE - static_cast<SharpRuntime::intcs>(s.zs.avail_out);
        if (produced > 0 && inner_)
            inner_->Write(s.outbuf, 0, produced);
    } while (ret == Z_OK && s.zs.avail_out == 0);
}

// ---------------------------------------------------------------------------
// Close
// ---------------------------------------------------------------------------

void DeflateStream::Close() {
    if (!state_ || !state_->initialized) {
        if (!leaveOpen_ && inner_) { inner_->Close(); inner_ = nullptr; }
        return;
    }
    auto& s = *state_;
    // Mark not-initialized before the flush loop below, which can throw via inner_->Write (e.g.
    // the inner stream hits a broken pipe or a full disk) -- matches BufferedStream::Close()'s
    // "set closed state before the throwing operation" pattern. Without this, a second Close()
    // call (the destructor calls Close() unconditionally, with no try/catch) would re-enter the
    // same failing flush loop, permanently leaking zlib's deflate/inflate state and risking
    // std::terminate if it fires during this exception's own unwind.
    s.initialized = false;
    if (mode_ == CompressionMode::Compress) {
        s.zs.next_in  = nullptr;
        s.zs.avail_in = 0;
        try {
            int ret;
            do {
                s.zs.next_out  = s.outbuf;
                s.zs.avail_out = ZlibDeflateState::BUFSIZE;
                ret = deflate(&s.zs, Z_FINISH);
                SharpRuntime::intcs produced =
                    ZlibDeflateState::BUFSIZE - static_cast<SharpRuntime::intcs>(s.zs.avail_out);
                if (produced > 0 && inner_)
                    inner_->Write(s.outbuf, 0, produced);
            } while (ret == Z_OK);
        } catch (...) {
            deflateEnd(&s.zs);
            if (!leaveOpen_ && inner_) { inner_->Close(); inner_ = nullptr; }
            throw;
        }
        deflateEnd(&s.zs);
    } else {
        inflateEnd(&s.zs);
    }
    if (!leaveOpen_ && inner_) { inner_->Close(); inner_ = nullptr; }
}

} // namespace System::IO::Compression

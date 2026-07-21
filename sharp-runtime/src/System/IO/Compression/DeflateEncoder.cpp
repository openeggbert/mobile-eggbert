// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/IO/Compression/DeflateEncoder.hpp"
#include "System/IO/Compression/ZLibException.hpp"
#include "System/IO/IOException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/ObjectDisposedException.hpp"

#include <algorithm>
#include <zlib.h>

namespace System::IO::Compression {

    namespace {
        constexpr intcs MinQuality = 0;
        constexpr intcs MaxQuality = 9;
        constexpr intcs MinWindowLog = 8;
        constexpr intcs MaxWindowLog = 15;
        constexpr intcs DefaultWindowLog = MaxWindowLog;
        constexpr intcs DefaultQuality = 6;
        constexpr intcs Deflate_DefaultMemLevel = 8;
        constexpr intcs Deflate_NoCompressionMemLevel = 7;

        intcs ResolveDeflateWindowBits(intcs windowLog) {
            if (windowLog == -1) windowLog = DefaultWindowLog;
            windowLog = std::max(windowLog, static_cast<intcs>(9));
            return -windowLog;
        }
    }

    struct ZLibDeflateEncoderState {
        z_stream zs{};
    };

    void DeflateEncoder::ValidateQuality(intcs quality) {
        if (quality != -1) {
            if (quality < MinQuality) throw System::ArgumentOutOfRangeException("quality", "Value must be greater than or equal to 0.");
            if (quality > MaxQuality) throw System::ArgumentOutOfRangeException("quality", "Value must be less than or equal to 9.");
        }
    }

    void DeflateEncoder::ValidateWindowLog(intcs windowLog) {
        if (windowLog != -1) {
            if (windowLog < MinWindowLog) throw System::ArgumentOutOfRangeException("windowLog", "Value must be greater than or equal to 8.");
            if (windowLog > MaxWindowLog) throw System::ArgumentOutOfRangeException("windowLog", "Value must be less than or equal to 15.");
        }
    }

    DeflateEncoder::DeflateEncoder() : DeflateEncoder(DefaultQuality, DefaultWindowLog) {}

    DeflateEncoder::DeflateEncoder(intcs quality) : DeflateEncoder(quality, -1) {}

    DeflateEncoder::DeflateEncoder(intcs quality, intcs windowLog) {
        ValidateQuality(quality);
        ValidateWindowLog(windowLog);

        const intcs windowBits = ResolveDeflateWindowBits(windowLog);
        const intcs memLevel = quality == 0 ? Deflate_NoCompressionMemLevel : Deflate_DefaultMemLevel;
        const intcs resolvedQuality = quality == -1 ? Z_DEFAULT_COMPRESSION : quality;

        state_ = std::make_unique<ZLibDeflateEncoderState>();
        if (deflateInit2(&state_->zs, resolvedQuality, Z_DEFLATED, windowBits, memLevel, Z_DEFAULT_STRATEGY) != Z_OK) {
            throw IOException("DeflateEncoder: failed to initialize zlib deflate state.");
        }
    }

    DeflateEncoder::DeflateEncoder(const ZLibCompressionOptions& options)
        : DeflateEncoder(options.getCompressionLevelProperty(), options.getWindowLogProperty()) {}

    DeflateEncoder::DeflateEncoder(intcs quality, intcs windowBits, intcs memLevel, intcs strategy)
        : state_(std::make_unique<ZLibDeflateEncoderState>())
    {
        const intcs resolvedQuality = quality == -1 ? Z_DEFAULT_COMPRESSION : quality;
        if (deflateInit2(&state_->zs, resolvedQuality, Z_DEFLATED, windowBits, memLevel, strategy) != Z_OK) {
            throw IOException("DeflateEncoder: failed to initialize zlib deflate state.");
        }
    }

    DeflateEncoder::~DeflateEncoder() { Dispose(); }

    void DeflateEncoder::Dispose() {
        if (disposed_) return;
        disposed_ = true;
        if (state_) {
            deflateEnd(&state_->zs);
            state_.reset();
        }
    }

    longcs DeflateEncoder::GetMaxCompressedLength(longcs inputLength) {
        if (inputLength < 0) throw System::ArgumentOutOfRangeException("inputLength", "Non-negative number required.");

        // zlib's compressBound() takes a uLong, which is only 32 bits wide on platforms where
        // `unsigned long` is 32 bits (e.g. Windows' LLP64 model) even though longcs (.NET's
        // `long`) is always 64-bit. Casting an inputLength above 2^32 straight into a 32-bit
        // uLong silently truncates it, so compressBound would compute its bound from the wrong
        // (much smaller) size -- an undersized "max compressed length" that downstream buffer
        // allocation would then silently overflow into. Real .NET's own DeflateEncoder avoids
        // this exact hazard by only calling the native compressBound() for inputs up to 2^31 and
        // falling back to zlib-ng's managed quick-strategy formula (a proven safe upper bound)
        // above that -- mirrored here verbatim so the 32-bit-uLong platforms this port targets
        // (per CLAUDE.md's Windows support policy) never truncate.
        if (inputLength <= (longcs(1) << 31)) {
            return static_cast<longcs>(compressBound(static_cast<uLong>(inputLength)));
        }

        const auto sourceLength = static_cast<uint64_t>(inputLength);
        uint64_t maxCompressedLength = sourceLength
            + (sourceLength == 0 ? 1u : 0u)
            + (sourceLength < 9 ? 1u : 0u)
            + ((sourceLength + 7) >> 3)
            + 3   // DEFLATE_BLOCK_OVERHEAD: (3 + 15 + 6) >> 3
            + 6;  // ZLIB_WRAPLEN: zlib header (2 bytes) + Adler32 trailer (4 bytes)

        if (maxCompressedLength > static_cast<uint64_t>(INT64_MAX))
            throw System::ArgumentOutOfRangeException("inputLength");

        return static_cast<longcs>(maxCompressedLength);
    }

    OperationStatus DeflateEncoder::Compress(const bytecs* source, intcs sourceLength,
                                             bytecs* destination, intcs destinationLength,
                                             intcs& bytesConsumed, intcs& bytesWritten, bool isFinalBlock)
    {
        if (disposed_) throw System::ObjectDisposedException("DeflateEncoder");

        bytesConsumed = 0;
        bytesWritten = 0;

        if (finished_) return OperationStatus::Done;
        if (sourceLength == 0 && !isFinalBlock) return OperationStatus::Done;
        if (destinationLength == 0 && (sourceLength > 0 || isFinalBlock)) return OperationStatus::DestinationTooSmall;

        const int flushCode = isFinalBlock ? Z_FINISH : Z_NO_FLUSH;

        auto& zs = state_->zs;
        zs.next_in   = const_cast<Bytef*>(source);
        zs.avail_in  = static_cast<uInt>(sourceLength);
        zs.next_out  = destination;
        zs.avail_out = static_cast<uInt>(destinationLength);

        const int ret = deflate(&zs, flushCode);

        bytesConsumed = sourceLength - static_cast<intcs>(zs.avail_in);
        bytesWritten  = destinationLength - static_cast<intcs>(zs.avail_out);

        OperationStatus status;
        if (ret == Z_OK && isFinalBlock) {
            status = OperationStatus::DestinationTooSmall;
        } else if (ret == Z_OK) {
            status = zs.avail_in == 0 ? OperationStatus::Done : OperationStatus::DestinationTooSmall;
        } else if (ret == Z_STREAM_END) {
            status = OperationStatus::Done;
        } else if (ret == Z_BUF_ERROR) {
            status = zs.avail_out == 0 ? OperationStatus::DestinationTooSmall : OperationStatus::Done;
        } else {
            throw ZLibException("Unexpected error occurred during compression.", "deflate", ret, zs.msg ? zs.msg : "");
        }

        if (isFinalBlock && ret == Z_STREAM_END) finished_ = true;
        return status;
    }

    OperationStatus DeflateEncoder::Flush(bytecs* destination, intcs destinationLength, intcs& bytesWritten) {
        if (disposed_) throw System::ObjectDisposedException("DeflateEncoder");

        bytesWritten = 0;
        if (finished_) return OperationStatus::Done;

        auto& zs = state_->zs;
        zs.next_in   = nullptr;
        zs.avail_in  = 0;
        zs.next_out  = destination;
        zs.avail_out = static_cast<uInt>(destinationLength);

        const int ret = deflate(&zs, Z_SYNC_FLUSH);
        bytesWritten = destinationLength - static_cast<intcs>(zs.avail_out);

        if (ret == Z_OK) return zs.avail_out == 0 ? OperationStatus::DestinationTooSmall : OperationStatus::Done;
        if (ret == Z_STREAM_END) return OperationStatus::Done;
        if (ret == Z_BUF_ERROR) return zs.avail_out == 0 ? OperationStatus::DestinationTooSmall : OperationStatus::Done;
        throw ZLibException("Unexpected error occurred during compression.", "deflate", ret, zs.msg ? zs.msg : "");
    }

    bool DeflateEncoder::TryCompress(const bytecs* source, intcs sourceLength,
                                     bytecs* destination, intcs destinationLength, intcs& bytesWritten) {
        return TryCompress(source, sourceLength, destination, destinationLength, bytesWritten, -1, -1);
    }

    bool DeflateEncoder::TryCompress(const bytecs* source, intcs sourceLength,
                                     bytecs* destination, intcs destinationLength, intcs& bytesWritten,
                                     intcs quality) {
        return TryCompress(source, sourceLength, destination, destinationLength, bytesWritten, quality, -1);
    }

    bool DeflateEncoder::TryCompress(const bytecs* source, intcs sourceLength,
                                     bytecs* destination, intcs destinationLength, intcs& bytesWritten,
                                     intcs quality, intcs windowLog) {
        DeflateEncoder encoder(quality, windowLog);
        intcs consumed = 0;
        const OperationStatus status =
            encoder.Compress(source, sourceLength, destination, destinationLength, consumed, bytesWritten, true);

        const bool success = status == OperationStatus::Done && consumed == sourceLength;
        if (!success) bytesWritten = 0;
        return success;
    }

} // namespace System::IO::Compression

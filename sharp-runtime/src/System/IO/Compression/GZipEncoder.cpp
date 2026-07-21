// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/IO/Compression/GZipEncoder.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/ObjectDisposedException.hpp"

#include <algorithm>
#include <limits>
#include <zlib.h>

namespace System::IO::Compression {

    namespace {
        constexpr intcs MinQuality = 0;
        constexpr intcs MaxQuality = 9;
        constexpr intcs MinWindowLog = 8;
        constexpr intcs MaxWindowLog = 15;
        constexpr intcs DefaultWindowLog = MaxWindowLog;
        constexpr intcs Deflate_DefaultMemLevel = 8;
        constexpr intcs Deflate_NoCompressionMemLevel = 7;

        void ValidateQuality(intcs quality) {
            if (quality != -1) {
                if (quality < MinQuality) throw System::ArgumentOutOfRangeException("quality", "Value must be greater than or equal to 0.");
                if (quality > MaxQuality) throw System::ArgumentOutOfRangeException("quality", "Value must be less than or equal to 9.");
            }
        }

        void ValidateWindowLog(intcs windowLog) {
            if (windowLog != -1) {
                if (windowLog < MinWindowLog) throw System::ArgumentOutOfRangeException("windowLog", "Value must be greater than or equal to 8.");
                if (windowLog > MaxWindowLog) throw System::ArgumentOutOfRangeException("windowLog", "Value must be less than or equal to 15.");
            }
        }

        intcs ResolveGZipWindowBits(intcs windowLog) {
            if (windowLog == -1) windowLog = DefaultWindowLog;
            windowLog = std::max(windowLog, static_cast<intcs>(9));
            return windowLog + 16;
        }

        DeflateEncoder MakeDeflateEncoder(intcs quality, intcs windowLog) {
            ValidateQuality(quality);
            ValidateWindowLog(windowLog);
            const intcs memLevel = quality == 0 ? Deflate_NoCompressionMemLevel : Deflate_DefaultMemLevel;
            return DeflateEncoder(quality, ResolveGZipWindowBits(windowLog), memLevel, Z_DEFAULT_STRATEGY);
        }
    }

    GZipEncoder::GZipEncoder() : deflateEncoder_(MakeDeflateEncoder(-1, -1)) {}
    GZipEncoder::GZipEncoder(intcs quality) : deflateEncoder_(MakeDeflateEncoder(quality, -1)) {}
    GZipEncoder::GZipEncoder(intcs quality, intcs windowLog) : deflateEncoder_(MakeDeflateEncoder(quality, windowLog)) {}
    GZipEncoder::GZipEncoder(const ZLibCompressionOptions& options)
        : deflateEncoder_(MakeDeflateEncoder(options.getCompressionLevelProperty(), options.getWindowLogProperty())) {}

    void GZipEncoder::Dispose() {
        if (disposed_) return;
        disposed_ = true;
        deflateEncoder_.Dispose();
    }

    longcs GZipEncoder::GetMaxCompressedLength(longcs inputLength) {
        // Verified against GZipEncoder.cs: compressBound() (used by DeflateEncoder's bound)
        // returns the upper bound for zlib-wrapped deflate, which includes 6 bytes of zlib
        // overhead (2-byte header + 4-byte Adler32 trailer). The GZip format uses 18 bytes of
        // overhead (10-byte header + 8-byte CRC32/size trailer) -- 12 bytes more than zlib's,
        // which this port previously never added, understating the worst-case buffer size any
        // caller sizing a destination buffer from this value would allocate.
        longcs maxCompressedLength = DeflateEncoder::GetMaxCompressedLength(inputLength);
        if (maxCompressedLength > std::numeric_limits<longcs>::max() - 12)
            throw System::ArgumentOutOfRangeException("inputLength");
        return maxCompressedLength + 12;
    }

    OperationStatus GZipEncoder::Compress(const bytecs* source, intcs sourceLength,
                                          bytecs* destination, intcs destinationLength,
                                          intcs& bytesConsumed, intcs& bytesWritten, bool isFinalBlock)
    {
        if (disposed_) throw System::ObjectDisposedException("GZipEncoder");
        return deflateEncoder_.Compress(source, sourceLength, destination, destinationLength, bytesConsumed, bytesWritten, isFinalBlock);
    }

    OperationStatus GZipEncoder::Flush(bytecs* destination, intcs destinationLength, intcs& bytesWritten) {
        if (disposed_) throw System::ObjectDisposedException("GZipEncoder");
        return deflateEncoder_.Flush(destination, destinationLength, bytesWritten);
    }

    bool GZipEncoder::TryCompress(const bytecs* source, intcs sourceLength,
                                  bytecs* destination, intcs destinationLength, intcs& bytesWritten) {
        return TryCompress(source, sourceLength, destination, destinationLength, bytesWritten, -1, -1);
    }

    bool GZipEncoder::TryCompress(const bytecs* source, intcs sourceLength,
                                  bytecs* destination, intcs destinationLength, intcs& bytesWritten,
                                  intcs quality) {
        return TryCompress(source, sourceLength, destination, destinationLength, bytesWritten, quality, -1);
    }

    bool GZipEncoder::TryCompress(const bytecs* source, intcs sourceLength,
                                  bytecs* destination, intcs destinationLength, intcs& bytesWritten,
                                  intcs quality, intcs windowLog) {
        GZipEncoder encoder(quality, windowLog);
        intcs consumed = 0;
        const OperationStatus status =
            encoder.Compress(source, sourceLength, destination, destinationLength, consumed, bytesWritten, true);

        const bool success = status == OperationStatus::Done && consumed == sourceLength;
        if (!success) bytesWritten = 0;
        return success;
    }

} // namespace System::IO::Compression

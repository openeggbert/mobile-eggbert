// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/IO/Compression/ZLibEncoder.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/ObjectDisposedException.hpp"

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

        // Unlike Deflate/GZip, ZLib format does not clamp windowLog to a minimum of 9.
        intcs ResolveZLibWindowBits(intcs windowLog) {
            return windowLog == -1 ? DefaultWindowLog : windowLog;
        }

        DeflateEncoder MakeDeflateEncoder(intcs quality, intcs windowLog) {
            ValidateQuality(quality);
            ValidateWindowLog(windowLog);
            const intcs memLevel = quality == 0 ? Deflate_NoCompressionMemLevel : Deflate_DefaultMemLevel;
            return DeflateEncoder(quality, ResolveZLibWindowBits(windowLog), memLevel, Z_DEFAULT_STRATEGY);
        }
    }

    ZLibEncoder::ZLibEncoder() : deflateEncoder_(MakeDeflateEncoder(-1, -1)) {}
    ZLibEncoder::ZLibEncoder(intcs quality) : deflateEncoder_(MakeDeflateEncoder(quality, -1)) {}
    ZLibEncoder::ZLibEncoder(intcs quality, intcs windowLog) : deflateEncoder_(MakeDeflateEncoder(quality, windowLog)) {}
    ZLibEncoder::ZLibEncoder(const ZLibCompressionOptions& options)
        : deflateEncoder_(MakeDeflateEncoder(options.getCompressionLevelProperty(), options.getWindowLogProperty())) {}

    void ZLibEncoder::Dispose() {
        if (disposed_) return;
        disposed_ = true;
        deflateEncoder_.Dispose();
    }

    longcs ZLibEncoder::GetMaxCompressedLength(longcs inputLength) {
        return DeflateEncoder::GetMaxCompressedLength(inputLength);
    }

    OperationStatus ZLibEncoder::Compress(const bytecs* source, intcs sourceLength,
                                          bytecs* destination, intcs destinationLength,
                                          intcs& bytesConsumed, intcs& bytesWritten, bool isFinalBlock)
    {
        if (disposed_) throw System::ObjectDisposedException("ZLibEncoder");
        return deflateEncoder_.Compress(source, sourceLength, destination, destinationLength, bytesConsumed, bytesWritten, isFinalBlock);
    }

    OperationStatus ZLibEncoder::Flush(bytecs* destination, intcs destinationLength, intcs& bytesWritten) {
        if (disposed_) throw System::ObjectDisposedException("ZLibEncoder");
        return deflateEncoder_.Flush(destination, destinationLength, bytesWritten);
    }

    bool ZLibEncoder::TryCompress(const bytecs* source, intcs sourceLength,
                                  bytecs* destination, intcs destinationLength, intcs& bytesWritten) {
        return TryCompress(source, sourceLength, destination, destinationLength, bytesWritten, -1, -1);
    }

    bool ZLibEncoder::TryCompress(const bytecs* source, intcs sourceLength,
                                  bytecs* destination, intcs destinationLength, intcs& bytesWritten,
                                  intcs quality) {
        return TryCompress(source, sourceLength, destination, destinationLength, bytesWritten, quality, -1);
    }

    bool ZLibEncoder::TryCompress(const bytecs* source, intcs sourceLength,
                                  bytecs* destination, intcs destinationLength, intcs& bytesWritten,
                                  intcs quality, intcs windowLog) {
        ZLibEncoder encoder(quality, windowLog);
        intcs consumed = 0;
        const OperationStatus status =
            encoder.Compress(source, sourceLength, destination, destinationLength, consumed, bytesWritten, true);

        const bool success = status == OperationStatus::Done && consumed == sourceLength;
        if (!success) bytesWritten = 0;
        return success;
    }

} // namespace System::IO::Compression

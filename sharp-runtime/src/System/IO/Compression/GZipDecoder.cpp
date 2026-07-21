// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/IO/Compression/GZipDecoder.hpp"
#include "System/ObjectDisposedException.hpp"

namespace System::IO::Compression {

    namespace {
        constexpr intcs GZip_DefaultWindowBits = 31; // 15 (max window) + 16 (gzip header/trailer)
    }

    GZipDecoder::GZipDecoder() : deflateDecoder_(GZip_DefaultWindowBits) {}

    void GZipDecoder::Dispose() {
        if (disposed_) return;
        disposed_ = true;
        deflateDecoder_.Dispose();
    }

    OperationStatus GZipDecoder::Decompress(const bytecs* source, intcs sourceLength,
                                            bytecs* destination, intcs destinationLength,
                                            intcs& bytesConsumed, intcs& bytesWritten)
    {
        if (disposed_) throw System::ObjectDisposedException("GZipDecoder");
        return deflateDecoder_.Decompress(source, sourceLength, destination, destinationLength, bytesConsumed, bytesWritten);
    }

    bool GZipDecoder::TryDecompress(const bytecs* source, intcs sourceLength,
                                    bytecs* destination, intcs destinationLength,
                                    intcs& bytesWritten)
    {
        GZipDecoder decoder;
        intcs consumed = 0;
        const OperationStatus status =
            decoder.Decompress(source, sourceLength, destination, destinationLength, consumed, bytesWritten);

        const bool success = status == OperationStatus::Done && consumed == sourceLength;
        if (!success) bytesWritten = 0;
        return success;
    }

} // namespace System::IO::Compression

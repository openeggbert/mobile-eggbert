// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/IO/Compression/ZLibDecoder.hpp"
#include "System/ObjectDisposedException.hpp"

namespace System::IO::Compression {

    namespace {
        constexpr intcs ZLib_DefaultWindowBits = 15;
    }

    ZLibDecoder::ZLibDecoder() : deflateDecoder_(ZLib_DefaultWindowBits) {}

    void ZLibDecoder::Dispose() {
        if (disposed_) return;
        disposed_ = true;
        deflateDecoder_.Dispose();
    }

    OperationStatus ZLibDecoder::Decompress(const bytecs* source, intcs sourceLength,
                                            bytecs* destination, intcs destinationLength,
                                            intcs& bytesConsumed, intcs& bytesWritten)
    {
        if (disposed_) throw System::ObjectDisposedException("ZLibDecoder");
        return deflateDecoder_.Decompress(source, sourceLength, destination, destinationLength, bytesConsumed, bytesWritten);
    }

    bool ZLibDecoder::TryDecompress(const bytecs* source, intcs sourceLength,
                                    bytecs* destination, intcs destinationLength,
                                    intcs& bytesWritten)
    {
        ZLibDecoder decoder;
        intcs consumed = 0;
        const OperationStatus status =
            decoder.Decompress(source, sourceLength, destination, destinationLength, consumed, bytesWritten);

        const bool success = status == OperationStatus::Done && consumed == sourceLength;
        if (!success) bytesWritten = 0;
        return success;
    }

} // namespace System::IO::Compression

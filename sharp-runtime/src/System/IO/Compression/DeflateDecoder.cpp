// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/IO/Compression/DeflateDecoder.hpp"
#include "System/IO/IOException.hpp"
#include "System/ObjectDisposedException.hpp"

#include <zlib.h>

namespace System::IO::Compression {

    namespace {
        constexpr int Deflate_DefaultWindowBits = -15;
    }

    struct ZLibInflateState {
        z_stream zs{};
    };

    DeflateDecoder::DeflateDecoder() : DeflateDecoder(Deflate_DefaultWindowBits) {}

    DeflateDecoder::DeflateDecoder(intcs windowBits)
        : state_(std::make_unique<ZLibInflateState>())
    {
        if (inflateInit2(&state_->zs, windowBits) != Z_OK) {
            throw IOException("DeflateDecoder: failed to initialize zlib inflate state.");
        }
    }

    DeflateDecoder::~DeflateDecoder() { Dispose(); }

    void DeflateDecoder::Dispose() {
        if (disposed_) return;
        disposed_ = true;
        if (state_) {
            inflateEnd(&state_->zs);
            state_.reset();
        }
    }

    OperationStatus DeflateDecoder::Decompress(const bytecs* source, intcs sourceLength,
                                                bytecs* destination, intcs destinationLength,
                                                intcs& bytesConsumed, intcs& bytesWritten)
    {
        if (disposed_) throw System::ObjectDisposedException("DeflateDecoder");

        bytesConsumed = 0;
        bytesWritten = 0;

        if (finished_) return OperationStatus::Done;
        if (destinationLength == 0 && sourceLength > 0) return OperationStatus::DestinationTooSmall;

        auto& zs = state_->zs;
        zs.next_in   = const_cast<Bytef*>(source);
        zs.avail_in  = static_cast<uInt>(sourceLength);
        zs.next_out  = destination;
        zs.avail_out = static_cast<uInt>(destinationLength);

        const int ret = inflate(&zs, Z_NO_FLUSH);

        bytesConsumed = sourceLength - static_cast<intcs>(zs.avail_in);
        bytesWritten  = destinationLength - static_cast<intcs>(zs.avail_out);

        if (ret == Z_OK || ret == Z_BUF_ERROR) {
            return zs.avail_out == 0 ? OperationStatus::DestinationTooSmall : OperationStatus::NeedMoreData;
        }
        if (ret == Z_STREAM_END) {
            finished_ = true;
            return OperationStatus::Done;
        }
        return OperationStatus::InvalidData;
    }

    bool DeflateDecoder::TryDecompress(const bytecs* source, intcs sourceLength,
                                       bytecs* destination, intcs destinationLength,
                                       intcs& bytesWritten)
    {
        DeflateDecoder decoder;
        intcs consumed = 0;
        const OperationStatus status =
            decoder.Decompress(source, sourceLength, destination, destinationLength, consumed, bytesWritten);

        const bool success = status == OperationStatus::Done && consumed == sourceLength;
        if (!success) bytesWritten = 0;
        return success;
    }

} // namespace System::IO::Compression

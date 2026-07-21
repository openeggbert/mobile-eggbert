// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/IO/Hashing/XxHash32.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include <cstring>

namespace System::IO::Hashing {

    uintcs XxHash32::rotl32(uintcs v, int n) { return (v << n) | (v >> (32 - n)); }

    uintcs XxHash32::round(uintcs acc, uintcs input) {
        acc += input * Prime2;
        acc = rotl32(acc, 13);
        acc *= Prime1;
        return acc;
    }

    void XxHash32::initState() {
        v1_ = seed_ + Prime1 + Prime2;
        v2_ = seed_ + Prime2;
        v3_ = seed_;
        v4_ = seed_ - Prime1;
    }

    void XxHash32::processBlock(const bytecs* block) {
        uintcs lane;
        std::memcpy(&lane, block,      4); v1_ = round(v1_, lane);
        std::memcpy(&lane, block + 4,  4); v2_ = round(v2_, lane);
        std::memcpy(&lane, block + 8,  4); v3_ = round(v3_, lane);
        std::memcpy(&lane, block + 12, 4); v4_ = round(v4_, lane);
    }

    XxHash32::XxHash32(intcs seed)
        : NonCryptographicHashAlgorithm(Size), seed_(static_cast<uintcs>(seed))
    {
        initState();
    }

    XxHash32::XxHash32(uintcs seed, uintcs v1, uintcs v2, uintcs v3, uintcs v4,
                       SharpRuntime::ulongcs totalLength, const bytecs* buf, intcs bufLen)
        : NonCryptographicHashAlgorithm(Size), seed_(seed), v1_(v1), v2_(v2), v3_(v3), v4_(v4),
          totalLength_(totalLength), bufLen_(bufLen)
    {
        if (bufLen > 0) std::memcpy(buf_, buf, static_cast<size_t>(bufLen));
    }

    XxHash32 XxHash32::Clone() const {
        return XxHash32(seed_, v1_, v2_, v3_, v4_, totalLength_, buf_, bufLen_);
    }

    void XxHash32::Reset() { totalLength_ = 0; bufLen_ = 0; initState(); }

    void XxHash32::Append(const bytecs* source, intcs length) {
        if (length < 0)
            throw System::ArgumentOutOfRangeException("length", "Non-negative number required.");
        totalLength_ += static_cast<SharpRuntime::ulongcs>(length);
        const bytecs* p   = source;
        const bytecs* end = source + length;

        if (bufLen_ > 0) {
            const intcs need = 16 - bufLen_;
            const intcs copy = length < need ? length : need;
            std::memcpy(buf_ + bufLen_, p, static_cast<size_t>(copy));
            bufLen_ += copy;
            p += copy;
            if (bufLen_ < 16) return;
            processBlock(buf_);
            bufLen_ = 0;
        }

        while (p + 16 <= end) { processBlock(p); p += 16; }

        bufLen_ = static_cast<intcs>(end - p);
        if (bufLen_ > 0) std::memcpy(buf_, p, static_cast<size_t>(bufLen_));
    }

    void XxHash32::GetCurrentHashCore(bytecs* destination) const {
        const uintcs h32 = GetCurrentHashAsUInt32();
        destination[0] = static_cast<bytecs>(h32 >> 24);
        destination[1] = static_cast<bytecs>(h32 >> 16);
        destination[2] = static_cast<bytecs>(h32 >> 8);
        destination[3] = static_cast<bytecs>(h32);
    }

    uintcs XxHash32::GetCurrentHashAsUInt32() const {
        uintcs h32;
        if (totalLength_ >= 16) {
            h32 = rotl32(v1_, 1) + rotl32(v2_, 7) + rotl32(v3_, 12) + rotl32(v4_, 18);
        } else {
            h32 = seed_ + Prime5;
        }
        h32 += static_cast<uintcs>(totalLength_);

        const bytecs* p  = buf_;
        intcs remaining = bufLen_;
        while (remaining >= 4) {
            uintcs lane; std::memcpy(&lane, p, 4);
            h32 += lane * Prime3;
            h32 = rotl32(h32, 17) * Prime4;
            p += 4; remaining -= 4;
        }
        while (remaining > 0) {
            h32 += (*p) * Prime5;
            h32 = rotl32(h32, 11) * Prime1;
            ++p; --remaining;
        }

        h32 ^= h32 >> 15; h32 *= Prime2;
        h32 ^= h32 >> 13; h32 *= Prime3;
        h32 ^= h32 >> 16;

        return h32;
    }

    std::vector<bytecs> XxHash32::Hash(const bytecs* source, intcs length, intcs seed) {
        std::vector<bytecs> ret(static_cast<size_t>(Size));
        XxHash32 h(seed);
        h.Append(source, length);
        h.GetCurrentHashCore(ret.data());
        return ret;
    }

    intcs XxHash32::Hash(const bytecs* source, intcs length, bytecs* destination, intcs destinationLength, intcs seed) {
        if (destinationLength < Size) {
            throw System::ArgumentException("Destination is too short.", "destination");
        }
        XxHash32 h(seed);
        h.Append(source, length);
        h.GetCurrentHashCore(destination);
        return Size;
    }

    bool XxHash32::TryHash(const bytecs* source, intcs length, bytecs* destination, intcs destinationLength,
                           intcs& bytesWritten, intcs seed) {
        if (destinationLength < Size) {
            bytesWritten = 0;
            return false;
        }
        XxHash32 h(seed);
        h.Append(source, length);
        h.GetCurrentHashCore(destination);
        bytesWritten = Size;
        return true;
    }

    uintcs XxHash32::HashToUInt32(const bytecs* source, intcs length, intcs seed) {
        XxHash32 h(seed);
        h.Append(source, length);
        return h.GetCurrentHashAsUInt32();
    }

} // namespace System::IO::Hashing

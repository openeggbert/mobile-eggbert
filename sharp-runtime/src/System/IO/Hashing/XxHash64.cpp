// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/IO/Hashing/XxHash64.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include <cstring>

namespace System::IO::Hashing {

    ulongcs XxHash64::rotl64(ulongcs v, int n) { return (v << n) | (v >> (64 - n)); }

    ulongcs XxHash64::round(ulongcs acc, ulongcs input) {
        acc += input * Prime2;
        acc = rotl64(acc, 31);
        acc *= Prime1;
        return acc;
    }

    ulongcs XxHash64::mergeAccumulator(ulongcs h64, ulongcs acc) {
        h64 ^= round(0, acc);
        return h64 * Prime1 + Prime4;
    }

    void XxHash64::initState() {
        v1_ = seed_ + Prime1 + Prime2;
        v2_ = seed_ + Prime2;
        v3_ = seed_;
        v4_ = seed_ - Prime1;
    }

    void XxHash64::processBlock(const bytecs* block) {
        ulongcs lane;
        std::memcpy(&lane, block,      8); v1_ = round(v1_, lane);
        std::memcpy(&lane, block + 8,  8); v2_ = round(v2_, lane);
        std::memcpy(&lane, block + 16, 8); v3_ = round(v3_, lane);
        std::memcpy(&lane, block + 24, 8); v4_ = round(v4_, lane);
    }

    XxHash64::XxHash64(longcs seed)
        : NonCryptographicHashAlgorithm(Size), seed_(static_cast<ulongcs>(seed))
    {
        initState();
    }

    XxHash64::XxHash64(ulongcs seed, ulongcs v1, ulongcs v2, ulongcs v3, ulongcs v4,
                       ulongcs totalLength, const bytecs* buf, intcs bufLen)
        : NonCryptographicHashAlgorithm(Size), seed_(seed), v1_(v1), v2_(v2), v3_(v3), v4_(v4),
          totalLength_(totalLength), bufLen_(bufLen)
    {
        if (bufLen > 0) std::memcpy(buf_, buf, static_cast<size_t>(bufLen));
    }

    XxHash64 XxHash64::Clone() const {
        return XxHash64(seed_, v1_, v2_, v3_, v4_, totalLength_, buf_, bufLen_);
    }

    void XxHash64::Reset() { totalLength_ = 0; bufLen_ = 0; initState(); }

    void XxHash64::Append(const bytecs* source, intcs length) {
        if (length < 0)
            throw System::ArgumentOutOfRangeException("length", "Non-negative number required.");
        totalLength_ += static_cast<ulongcs>(length);
        const bytecs* p   = source;
        const bytecs* end = source + length;

        if (bufLen_ > 0) {
            const intcs need = 32 - bufLen_;
            const intcs copy = length < need ? length : need;
            std::memcpy(buf_ + bufLen_, p, static_cast<size_t>(copy));
            bufLen_ += copy;
            p += copy;
            if (bufLen_ < 32) return;
            processBlock(buf_);
            bufLen_ = 0;
        }

        while (p + 32 <= end) { processBlock(p); p += 32; }

        bufLen_ = static_cast<intcs>(end - p);
        if (bufLen_ > 0) std::memcpy(buf_, p, static_cast<size_t>(bufLen_));
    }

    void XxHash64::GetCurrentHashCore(bytecs* destination) const {
        const ulongcs h64 = GetCurrentHashAsUInt64();
        for (int i = 0; i < 8; ++i) destination[i] = static_cast<bytecs>(h64 >> (56 - 8 * i));
    }

    ulongcs XxHash64::GetCurrentHashAsUInt64() const {
        ulongcs h64;
        if (totalLength_ >= 32) {
            h64 = rotl64(v1_, 1) + rotl64(v2_, 7) + rotl64(v3_, 12) + rotl64(v4_, 18);
            h64 = mergeAccumulator(h64, v1_);
            h64 = mergeAccumulator(h64, v2_);
            h64 = mergeAccumulator(h64, v3_);
            h64 = mergeAccumulator(h64, v4_);
        } else {
            h64 = seed_ + Prime5;
        }
        h64 += totalLength_;

        const bytecs* p  = buf_;
        intcs remaining = bufLen_;
        while (remaining >= 8) {
            ulongcs lane; std::memcpy(&lane, p, 8);
            h64 ^= round(0, lane);
            h64 = rotl64(h64, 27) * Prime1 + Prime4;
            p += 8; remaining -= 8;
        }
        if (remaining >= 4) {
            SharpRuntime::uintcs lane; std::memcpy(&lane, p, 4);
            h64 ^= static_cast<ulongcs>(lane) * Prime1;
            h64 = rotl64(h64, 23) * Prime2 + Prime3;
            p += 4; remaining -= 4;
        }
        while (remaining > 0) {
            h64 ^= (*p) * Prime5;
            h64 = rotl64(h64, 11) * Prime1;
            ++p; --remaining;
        }

        h64 ^= h64 >> 33; h64 *= Prime2;
        h64 ^= h64 >> 29; h64 *= Prime3;
        h64 ^= h64 >> 32;

        return h64;
    }

    std::vector<bytecs> XxHash64::Hash(const bytecs* source, intcs length, longcs seed) {
        std::vector<bytecs> ret(static_cast<size_t>(Size));
        XxHash64 h(seed);
        h.Append(source, length);
        h.GetCurrentHashCore(ret.data());
        return ret;
    }

    intcs XxHash64::Hash(const bytecs* source, intcs length, bytecs* destination, intcs destinationLength, longcs seed) {
        if (destinationLength < Size) {
            throw System::ArgumentException("Destination is too short.", "destination");
        }
        XxHash64 h(seed);
        h.Append(source, length);
        h.GetCurrentHashCore(destination);
        return Size;
    }

    bool XxHash64::TryHash(const bytecs* source, intcs length, bytecs* destination, intcs destinationLength,
                           intcs& bytesWritten, longcs seed) {
        if (destinationLength < Size) {
            bytesWritten = 0;
            return false;
        }
        XxHash64 h(seed);
        h.Append(source, length);
        h.GetCurrentHashCore(destination);
        bytesWritten = Size;
        return true;
    }

    ulongcs XxHash64::HashToUInt64(const bytecs* source, intcs length, longcs seed) {
        XxHash64 h(seed);
        h.Append(source, length);
        return h.GetCurrentHashAsUInt64();
    }

} // namespace System::IO::Hashing

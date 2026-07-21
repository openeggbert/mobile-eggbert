// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Security/Cryptography/MD5.hpp"
#include <cstring>

namespace System::Security::Cryptography {

namespace {

    constexpr uint32_t kK[64] = {
        0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
        0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be, 0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
        0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa, 0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
        0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
        0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c, 0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
        0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
        0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
        0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1, 0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391,
    };

    constexpr uint32_t kS[64] = {
        7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
        5, 9,  14, 20, 5, 9,  14, 20, 5, 9,  14, 20, 5, 9,  14, 20,
        4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
        6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21,
    };

    inline uint32_t rotl(uint32_t x, uint32_t c) { return (x << c) | (x >> (32 - c)); }

} // namespace

void MD5::Initialize() {
    state_ = {0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476};
    bufferLen_ = 0;
    totalBits_ = 0;
}

void MD5::processBlock(const uint8_t* block) {
    uint32_t m[16];
    for (int i = 0; i < 16; ++i) {
        m[i] = static_cast<uint32_t>(block[i * 4]) | (static_cast<uint32_t>(block[i * 4 + 1]) << 8) |
               (static_cast<uint32_t>(block[i * 4 + 2]) << 16) | (static_cast<uint32_t>(block[i * 4 + 3]) << 24);
    }

    uint32_t a = state_[0], b = state_[1], c = state_[2], d = state_[3];

    for (uint32_t i = 0; i < 64; ++i) {
        uint32_t f;
        uint32_t g;
        if (i < 16) {
            f = (b & c) | (~b & d);
            g = i;
        } else if (i < 32) {
            f = (d & b) | (~d & c);
            g = (5 * i + 1) % 16;
        } else if (i < 48) {
            f = b ^ c ^ d;
            g = (3 * i + 5) % 16;
        } else {
            f = c ^ (b | ~d);
            g = (7 * i) % 16;
        }
        f = f + a + kK[i] + m[g];
        a = d;
        d = c;
        c = b;
        b = b + rotl(f, kS[i]);
    }

    state_[0] += a;
    state_[1] += b;
    state_[2] += c;
    state_[3] += d;
}

void MD5::HashCore(const std::vector<bytecs>& array, intcs offset, intcs count) {
    const auto* data = array.data() + offset;
    size_t remaining = static_cast<size_t>(count);
    totalBits_ += static_cast<uint64_t>(count) * 8;

    while (remaining > 0) {
        size_t take = std::min(remaining, buffer_.size() - bufferLen_);
        std::memcpy(buffer_.data() + bufferLen_, data, take);
        bufferLen_ += take;
        data += take;
        remaining -= take;

        if (bufferLen_ == buffer_.size()) {
            processBlock(buffer_.data());
            bufferLen_ = 0;
        }
    }
}

std::vector<bytecs> MD5::HashFinal() {
    uint64_t bitLen = totalBits_;
    size_t padLen = (bufferLen_ < 56) ? (56 - bufferLen_) : (120 - bufferLen_);

    uint8_t pad[64] = {0x80};
    for (size_t i = 0; i < padLen; ++i) {
        std::memcpy(buffer_.data() + bufferLen_, pad + i, 1);
        bufferLen_++;
        if (bufferLen_ == buffer_.size()) {
            processBlock(buffer_.data());
            bufferLen_ = 0;
        }
    }

    uint8_t lenBytes[8];
    for (int i = 0; i < 8; ++i) {
        lenBytes[i] = static_cast<uint8_t>((bitLen >> (i * 8)) & 0xFF);
    }
    std::memcpy(buffer_.data() + bufferLen_, lenBytes, 8);
    bufferLen_ += 8;
    processBlock(buffer_.data());
    bufferLen_ = 0;

    std::vector<bytecs> digest(16);
    for (int i = 0; i < 4; ++i) {
        digest[i * 4] = static_cast<bytecs>(state_[i] & 0xFF);
        digest[i * 4 + 1] = static_cast<bytecs>((state_[i] >> 8) & 0xFF);
        digest[i * 4 + 2] = static_cast<bytecs>((state_[i] >> 16) & 0xFF);
        digest[i * 4 + 3] = static_cast<bytecs>((state_[i] >> 24) & 0xFF);
    }
    return digest;
}

} // namespace System::Security::Cryptography

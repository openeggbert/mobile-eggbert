// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Security/Cryptography/SHA1.hpp"
#include <cstring>

namespace System::Security::Cryptography {

void SHA1::Initialize() {
    state_ = {0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0};
    bufferLen_ = 0;
    totalBits_ = 0;
}

void SHA1::processBlock(const uint8_t* block) {
    uint32_t w[80];
    for (int i = 0; i < 16; ++i) {
        w[i] = (static_cast<uint32_t>(block[i * 4]) << 24) | (static_cast<uint32_t>(block[i * 4 + 1]) << 16) |
               (static_cast<uint32_t>(block[i * 4 + 2]) << 8) | static_cast<uint32_t>(block[i * 4 + 3]);
    }
    for (int i = 16; i < 80; ++i) {
        uint32_t v = w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16];
        w[i] = (v << 1) | (v >> 31);
    }

    uint32_t a = state_[0], b = state_[1], c = state_[2], d = state_[3], e = state_[4];

    for (int i = 0; i < 80; ++i) {
        uint32_t f, k;
        if (i < 20) {
            f = (b & c) | ((~b) & d);
            k = 0x5A827999;
        } else if (i < 40) {
            f = b ^ c ^ d;
            k = 0x6ED9EBA1;
        } else if (i < 60) {
            f = (b & c) | (b & d) | (c & d);
            k = 0x8F1BBCDC;
        } else {
            f = b ^ c ^ d;
            k = 0xCA62C1D6;
        }

        uint32_t temp = ((a << 5) | (a >> 27)) + f + e + k + w[i];
        e = d;
        d = c;
        c = (b << 30) | (b >> 2);
        b = a;
        a = temp;
    }

    state_[0] += a;
    state_[1] += b;
    state_[2] += c;
    state_[3] += d;
    state_[4] += e;
}

void SHA1::HashCore(const std::vector<bytecs>& array, intcs offset, intcs count) {
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

std::vector<bytecs> SHA1::HashFinal() {
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
    for (int i = 7; i >= 0; --i) {
        lenBytes[7 - i] = static_cast<uint8_t>((bitLen >> (i * 8)) & 0xFF);
    }
    std::memcpy(buffer_.data() + bufferLen_, lenBytes, 8);
    bufferLen_ += 8;
    processBlock(buffer_.data());
    bufferLen_ = 0;

    std::vector<bytecs> digest(20);
    for (int i = 0; i < 5; ++i) {
        digest[i * 4] = static_cast<bytecs>((state_[i] >> 24) & 0xFF);
        digest[i * 4 + 1] = static_cast<bytecs>((state_[i] >> 16) & 0xFF);
        digest[i * 4 + 2] = static_cast<bytecs>((state_[i] >> 8) & 0xFF);
        digest[i * 4 + 3] = static_cast<bytecs>(state_[i] & 0xFF);
    }
    return digest;
}

} // namespace System::Security::Cryptography

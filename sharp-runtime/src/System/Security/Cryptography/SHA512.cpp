// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Security/Cryptography/SHA512.hpp"
#include <cstring>
#include "System/Security/Cryptography/detail/Sha512Core.hpp"

namespace System::Security::Cryptography {

void SHA512::Initialize() {
    state_ = {0x6a09e667f3bcc908ULL, 0xbb67ae8584caa73bULL, 0x3c6ef372fe94f82bULL, 0xa54ff53a5f1d36f1ULL,
              0x510e527fade682d1ULL, 0x9b05688c2b3e6c1fULL, 0x1f83d9abfb41bd6bULL, 0x5be0cd19137e2179ULL};
    bufferLen_ = 0;
    totalBits_ = 0;
}

void SHA512::HashCore(const std::vector<bytecs>& array, intcs offset, intcs count) {
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
            SharpRuntimeDetail::Sha512::compressBlock(state_, buffer_.data());
            bufferLen_ = 0;
        }
    }
}

std::vector<bytecs> SHA512::HashFinal() {
    uint64_t bitLen = totalBits_;
    size_t padLen = (bufferLen_ < 112) ? (112 - bufferLen_) : (240 - bufferLen_);

    uint8_t pad[128] = {0x80};
    for (size_t i = 0; i < padLen; ++i) {
        std::memcpy(buffer_.data() + bufferLen_, pad + i, 1);
        bufferLen_++;
        if (bufferLen_ == buffer_.size()) {
            SharpRuntimeDetail::Sha512::compressBlock(state_, buffer_.data());
            bufferLen_ = 0;
        }
    }

    // 128-bit big-endian bit length; high 64 bits are always zero (see class doc comment).
    uint8_t lenBytes[16] = {0};
    for (int i = 7; i >= 0; --i) {
        lenBytes[15 - i] = static_cast<uint8_t>((bitLen >> (i * 8)) & 0xFF);
    }
    std::memcpy(buffer_.data() + bufferLen_, lenBytes, 16);
    bufferLen_ += 16;
    SharpRuntimeDetail::Sha512::compressBlock(state_, buffer_.data());
    bufferLen_ = 0;

    std::vector<bytecs> digest(64);
    for (int i = 0; i < 8; ++i) {
        for (int b = 0; b < 8; ++b) {
            digest[i * 8 + b] = static_cast<bytecs>((state_[i] >> ((7 - b) * 8)) & 0xFF);
        }
    }
    return digest;
}

} // namespace System::Security::Cryptography

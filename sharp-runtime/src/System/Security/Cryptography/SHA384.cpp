// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Security/Cryptography/SHA384.hpp"
#include <cstring>
#include "System/Security/Cryptography/detail/Sha512Core.hpp"

namespace System::Security::Cryptography {

void SHA384::Initialize() {
    state_ = {0xcbbb9d5dc1059ed8ULL, 0x629a292a367cd507ULL, 0x9159015a3070dd17ULL, 0x152fecd8f70e5939ULL,
              0x67332667ffc00b31ULL, 0x8eb44a8768581511ULL, 0xdb0c2e0d64f98fa7ULL, 0x47b5481dbefa4fa4ULL};
    bufferLen_ = 0;
    totalBits_ = 0;
}

void SHA384::HashCore(const std::vector<bytecs>& array, intcs offset, intcs count) {
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

std::vector<bytecs> SHA384::HashFinal() {
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

    uint8_t lenBytes[16] = {0};
    for (int i = 7; i >= 0; --i) {
        lenBytes[15 - i] = static_cast<uint8_t>((bitLen >> (i * 8)) & 0xFF);
    }
    std::memcpy(buffer_.data() + bufferLen_, lenBytes, 16);
    bufferLen_ += 16;
    SharpRuntimeDetail::Sha512::compressBlock(state_, buffer_.data());
    bufferLen_ = 0;

    std::vector<bytecs> digest(48);
    for (int i = 0; i < 6; ++i) {
        for (int b = 0; b < 8; ++b) {
            digest[i * 8 + b] = static_cast<bytecs>((state_[i] >> ((7 - b) * 8)) & 0xFF);
        }
    }
    return digest;
}

} // namespace System::Security::Cryptography

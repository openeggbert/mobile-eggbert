// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Security/Cryptography/HMAC.hpp"

namespace System::Security::Cryptography {

void HMAC::derivePads() {
    std::vector<bytecs> keyPrime = keyValue_;
    if (static_cast<intcs>(keyPrime.size()) > blockSizeValue_) {
        auto hasher = createHash_();
        keyPrime = hasher->ComputeHash(keyPrime);
    }
    keyPrime.resize(static_cast<size_t>(blockSizeValue_), bytecs{0});

    innerPad_.assign(static_cast<size_t>(blockSizeValue_), bytecs{0});
    outerPad_.assign(static_cast<size_t>(blockSizeValue_), bytecs{0});
    for (size_t i = 0; i < keyPrime.size(); ++i) {
        innerPad_[i] = static_cast<bytecs>(keyPrime[i] ^ 0x36);
        outerPad_[i] = static_cast<bytecs>(keyPrime[i] ^ 0x5C);
    }
}

void HMAC::HashCore(const std::vector<bytecs>& array, intcs offset, intcs count) {
    pendingMessage_.insert(pendingMessage_.end(), array.begin() + offset, array.begin() + offset + count);
}

std::vector<bytecs> HMAC::HashFinal() {
    std::vector<bytecs> innerInput = innerPad_;
    innerInput.insert(innerInput.end(), pendingMessage_.begin(), pendingMessage_.end());
    auto inner = createHash_();
    std::vector<bytecs> innerDigest = inner->ComputeHash(innerInput);

    std::vector<bytecs> outerInput = outerPad_;
    outerInput.insert(outerInput.end(), innerDigest.begin(), innerDigest.end());
    auto outer = createHash_();
    return outer->ComputeHash(outerInput);
}

} // namespace System::Security::Cryptography

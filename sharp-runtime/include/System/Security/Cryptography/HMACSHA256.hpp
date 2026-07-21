// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Security/Cryptography/HMAC.hpp"
#include "System/Security/Cryptography/SHA256.hpp"

namespace System::Security::Cryptography {

    /** @brief Computes an HMAC-SHA256 (RFC 2104) message authentication code. */
    class HMACSHA256 : public HMAC {
    public:
        explicit HMACSHA256(std::vector<bytecs> key)
            : HMAC([]() -> std::unique_ptr<HashAlgorithm> { return std::make_unique<SHA256>(); }, 64, "SHA256",
                   std::move(key)) {
            hashSizeValue_ = 256;
        }
    };

} // namespace System::Security::Cryptography

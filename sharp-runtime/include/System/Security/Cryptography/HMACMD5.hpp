// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Security/Cryptography/HMAC.hpp"
#include "System/Security/Cryptography/MD5.hpp"

namespace System::Security::Cryptography {

    /** @brief Computes an HMAC-MD5 (RFC 2104) message authentication code. */
    class HMACMD5 : public HMAC {
    public:
        explicit HMACMD5(std::vector<bytecs> key)
            : HMAC([]() -> std::unique_ptr<HashAlgorithm> { return std::make_unique<MD5>(); }, 64, "MD5", std::move(key)) {
            hashSizeValue_ = 128;
        }
    };

} // namespace System::Security::Cryptography

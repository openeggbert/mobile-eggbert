// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Security/Cryptography/HMAC.hpp"
#include "System/Security/Cryptography/SHA1.hpp"

namespace System::Security::Cryptography {

    /** @brief Computes an HMAC-SHA1 (RFC 2104) message authentication code. */
    class HMACSHA1 : public HMAC {
    public:
        explicit HMACSHA1(std::vector<bytecs> key)
            : HMAC([]() -> std::unique_ptr<HashAlgorithm> { return std::make_unique<SHA1>(); }, 64, "SHA1", std::move(key)) {
            hashSizeValue_ = 160;
        }
    };

} // namespace System::Security::Cryptography

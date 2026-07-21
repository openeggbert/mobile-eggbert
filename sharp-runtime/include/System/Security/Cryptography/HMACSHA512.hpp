// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Security/Cryptography/HMAC.hpp"
#include "System/Security/Cryptography/SHA512.hpp"

namespace System::Security::Cryptography {

    /** @brief Computes an HMAC-SHA512 (RFC 2104) message authentication code. */
    class HMACSHA512 : public HMAC {
    public:
        explicit HMACSHA512(std::vector<bytecs> key)
            : HMAC([]() -> std::unique_ptr<HashAlgorithm> { return std::make_unique<SHA512>(); }, 128, "SHA512",
                   std::move(key)) {
            hashSizeValue_ = 512;
        }
    };

} // namespace System::Security::Cryptography

// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Security/Cryptography/HMAC.hpp"
#include "System/Security/Cryptography/SHA384.hpp"

namespace System::Security::Cryptography {

    /** @brief Computes an HMAC-SHA384 (RFC 2104) message authentication code. */
    class HMACSHA384 : public HMAC {
    public:
        explicit HMACSHA384(std::vector<bytecs> key)
            : HMAC([]() -> std::unique_ptr<HashAlgorithm> { return std::make_unique<SHA384>(); }, 128, "SHA384",
                   std::move(key)) {
            hashSizeValue_ = 384;
        }
    };

} // namespace System::Security::Cryptography

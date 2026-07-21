// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Security/Cryptography/HMAC.hpp"
#include "System/Security/Cryptography/SHA3_256.hpp"

namespace System::Security::Cryptography {

    /**
     * @brief Computes an HMAC-SHA3-256 (RFC 2104 / FIPS 202) message authentication code.
     *
     * The HMAC block size is the SHA3-256 sponge's rate (136 bytes), per FIPS 202's guidance for
     * constructing HMAC over Keccak-based hashes (which have no Merkle-Damgård "block size").
     */
    class HMACSHA3_256 : public HMAC {
    public:
        explicit HMACSHA3_256(std::vector<bytecs> key)
            : HMAC([]() -> std::unique_ptr<HashAlgorithm> { return std::make_unique<SHA3_256>(); }, 136, "SHA3-256",
                   std::move(key)) {
            hashSizeValue_ = 256;
        }
    };

} // namespace System::Security::Cryptography

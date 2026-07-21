// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Security/Cryptography/HMAC.hpp"
#include "System/Security/Cryptography/SHA3_512.hpp"

namespace System::Security::Cryptography {

    /**
     * @brief Computes an HMAC-SHA3-512 (RFC 2104 / FIPS 202) message authentication code.
     *
     * The HMAC block size is the SHA3-512 sponge's rate (72 bytes), per FIPS 202's guidance for
     * constructing HMAC over Keccak-based hashes (which have no Merkle-Damgård "block size").
     */
    class HMACSHA3_512 : public HMAC {
    public:
        explicit HMACSHA3_512(std::vector<bytecs> key)
            : HMAC([]() -> std::unique_ptr<HashAlgorithm> { return std::make_unique<SHA3_512>(); }, 72, "SHA3-512",
                   std::move(key)) {
            hashSizeValue_ = 512;
        }
    };

} // namespace System::Security::Cryptography

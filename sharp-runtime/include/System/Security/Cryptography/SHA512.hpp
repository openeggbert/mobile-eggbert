// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <array>
#include <cstdint>
#include "System/Security/Cryptography/HashAlgorithm.hpp"

namespace System::Security::Cryptography {

    /**
     * @brief Computes the SHA-512 hash (FIPS 180-4) for the input data.
     *
     * C++ counterpart of .NET System.Security.Cryptography.SHA512 (and the legacy SHA512Managed,
     * functionally identical in this runtime).
     *
     * @note The message-length field in the final padding block is the standard 128 bits per
     * FIPS 180-4, but this implementation only tracks the low 64 bits of it (the high 64 bits are
     * always written as zero) — correct for any input up to 2^64 bits (~2 exabytes), far beyond
     * anything this runtime will ever hash.
     */
    class SHA512 : public HashAlgorithm {
        std::array<uint64_t, 8> state_{};
        std::array<uint8_t, 128> buffer_{};
        size_t bufferLen_ = 0;
        uint64_t totalBits_ = 0;

    protected:
        void HashCore(const std::vector<bytecs>& array, intcs offset, intcs count) override;
        std::vector<bytecs> HashFinal() override;

    public:
        SHA512() { hashSizeValue_ = 512; Initialize(); }

        void Initialize() override;
    };

} // namespace System::Security::Cryptography

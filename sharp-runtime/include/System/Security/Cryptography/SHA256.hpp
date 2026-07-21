// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <array>
#include <cstdint>
#include "System/Security/Cryptography/HashAlgorithm.hpp"

namespace System::Security::Cryptography {

    /**
     * @brief Computes the SHA-256 hash (FIPS 180-4) for the input data.
     *
     * C++ counterpart of .NET System.Security.Cryptography.SHA256 (and the legacy SHA256Managed,
     * functionally identical in this runtime).
     */
    class SHA256 : public HashAlgorithm {
        std::array<uint32_t, 8> state_{};
        std::array<uint8_t, 64> buffer_{};
        size_t bufferLen_ = 0;
        uint64_t totalBits_ = 0;

        void processBlock(const uint8_t* block);

    protected:
        void HashCore(const std::vector<bytecs>& array, intcs offset, intcs count) override;
        std::vector<bytecs> HashFinal() override;

    public:
        SHA256() { hashSizeValue_ = 256; Initialize(); }

        void Initialize() override;
    };

} // namespace System::Security::Cryptography

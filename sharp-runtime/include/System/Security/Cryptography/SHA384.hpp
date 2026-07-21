// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <array>
#include <cstdint>
#include "System/Security/Cryptography/HashAlgorithm.hpp"

namespace System::Security::Cryptography {

    /**
     * @brief Computes the SHA-384 hash (FIPS 180-4) for the input data.
     *
     * C++ counterpart of .NET System.Security.Cryptography.SHA384 (and the legacy SHA384Managed,
     * functionally identical in this runtime). Identical algorithm to SHA-512 with a different
     * initial hash value and truncated to the first 48 of 64 output bytes.
     *
     * @note Same 128-bit-length-field simplification as SHA512 — see that class's doc comment.
     */
    class SHA384 : public HashAlgorithm {
        std::array<uint64_t, 8> state_{};
        std::array<uint8_t, 128> buffer_{};
        size_t bufferLen_ = 0;
        uint64_t totalBits_ = 0;

    protected:
        void HashCore(const std::vector<bytecs>& array, intcs offset, intcs count) override;
        std::vector<bytecs> HashFinal() override;

    public:
        SHA384() { hashSizeValue_ = 384; Initialize(); }

        void Initialize() override;
    };

} // namespace System::Security::Cryptography

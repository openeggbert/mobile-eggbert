// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Security/Cryptography/HashAlgorithm.hpp"
#include "System/Security/Cryptography/detail/Keccak.hpp"

namespace System::Security::Cryptography {

    /**
     * @brief Computes the SHA3-256 hash (FIPS 202) for the input data.
     *
     * C++ counterpart of .NET System.Security.Cryptography.SHA3_256.
     */
    class SHA3_256 : public HashAlgorithm {
        SharpRuntimeDetail::Keccak::Sponge sponge_{136, 0x06};

    protected:
        void HashCore(const std::vector<bytecs>& array, intcs offset, intcs count) override {
            sponge_.Absorb(reinterpret_cast<const uint8_t*>(array.data() + offset), static_cast<size_t>(count));
        }
        std::vector<bytecs> HashFinal() override {
            sponge_.FinishAbsorbing();
            std::vector<bytecs> digest(32);
            sponge_.Squeeze(reinterpret_cast<uint8_t*>(digest.data()), digest.size());
            return digest;
        }

    public:
        SHA3_256() { hashSizeValue_ = 256; Initialize(); }

        void Initialize() override { sponge_.Reset(); }
    };

} // namespace System::Security::Cryptography

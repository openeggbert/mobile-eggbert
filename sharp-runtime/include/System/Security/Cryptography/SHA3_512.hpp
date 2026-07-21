// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Security/Cryptography/HashAlgorithm.hpp"
#include "System/Security/Cryptography/detail/Keccak.hpp"

namespace System::Security::Cryptography {

    /**
     * @brief Computes the SHA3-512 hash (FIPS 202) for the input data.
     *
     * C++ counterpart of .NET System.Security.Cryptography.SHA3_512.
     */
    class SHA3_512 : public HashAlgorithm {
        SharpRuntimeDetail::Keccak::Sponge sponge_{72, 0x06};

    protected:
        void HashCore(const std::vector<bytecs>& array, intcs offset, intcs count) override {
            sponge_.Absorb(reinterpret_cast<const uint8_t*>(array.data() + offset), static_cast<size_t>(count));
        }
        std::vector<bytecs> HashFinal() override {
            sponge_.FinishAbsorbing();
            std::vector<bytecs> digest(64);
            sponge_.Squeeze(reinterpret_cast<uint8_t*>(digest.data()), digest.size());
            return digest;
        }

    public:
        SHA3_512() { hashSizeValue_ = 512; Initialize(); }

        void Initialize() override { sponge_.Reset(); }
    };

} // namespace System::Security::Cryptography

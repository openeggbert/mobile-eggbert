// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <array>
#include <cstdint>
#include "System/Security/Cryptography/HashAlgorithm.hpp"

namespace System::Security::Cryptography {

    /**
     * @brief Computes the MD5 hash (RFC 1321) for the input data.
     *
     * C++ counterpart of .NET System.Security.Cryptography.MD5 (and the legacy
     * MD5CryptoServiceProvider, which is functionally identical in this runtime — there is no
     * separate CSP-backed vs. managed implementation split here).
     *
     * @note MD5 is cryptographically broken (collision attacks are practical) and should not be
     * used for security purposes in new code; it is ported here only for API/data-format
     * compatibility (e.g. checksums, legacy protocols), matching .NET's own continued inclusion
     * of it for the same reason.
     */
    class MD5 : public HashAlgorithm {
        std::array<uint32_t, 4> state_{};
        std::array<uint8_t, 64> buffer_{};
        size_t bufferLen_ = 0;
        uint64_t totalBits_ = 0;

        void processBlock(const uint8_t* block);

    protected:
        void HashCore(const std::vector<bytecs>& array, intcs offset, intcs count) override;
        std::vector<bytecs> HashFinal() override;

    public:
        MD5() { hashSizeValue_ = 128; Initialize(); }

        void Initialize() override;
    };

} // namespace System::Security::Cryptography

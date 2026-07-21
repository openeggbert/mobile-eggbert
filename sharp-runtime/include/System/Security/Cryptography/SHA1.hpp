// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <array>
#include <cstdint>
#include "System/Security/Cryptography/HashAlgorithm.hpp"

namespace System::Security::Cryptography {

    /**
     * @brief Computes the SHA-1 hash (RFC 3174) for the input data.
     *
     * C++ counterpart of .NET System.Security.Cryptography.SHA1 (and the legacy SHA1Managed,
     * functionally identical in this runtime).
     *
     * @note SHA-1 is cryptographically weak (collision attacks are practical, per the 2017
     * SHAttered attack) and should not be used for new security-sensitive purposes; ported for
     * API/protocol compatibility, matching .NET's own continued inclusion of it. This core
     * algorithm was already implemented once this session for the WebSocket RFC 6455 handshake's
     * `Sec-WebSocket-Accept` digest (verified correct via a real end-to-end handshake test); this
     * is the same algorithm restructured to support incremental (streaming) hashing.
     */
    class SHA1 : public HashAlgorithm {
        std::array<uint32_t, 5> state_{};
        std::array<uint8_t, 64> buffer_{};
        size_t bufferLen_ = 0;
        uint64_t totalBits_ = 0;

        void processBlock(const uint8_t* block);

    protected:
        void HashCore(const std::vector<bytecs>& array, intcs offset, intcs count) override;
        std::vector<bytecs> HashFinal() override;

    public:
        SHA1() { hashSizeValue_ = 160; Initialize(); }

        void Initialize() override;
    };

} // namespace System::Security::Cryptography

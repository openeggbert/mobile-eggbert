// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>
#include <string>
#include <vector>
#include "System/Security/Cryptography/DeriveBytes.hpp"
#include "System/Security/Cryptography/HMAC.hpp"
#include "System/Security/Cryptography/HashAlgorithmName.hpp"

namespace System::Security::Cryptography {

    /**
     * @brief Implements PBKDF2 (RFC 2898 / RFC 8018) key derivation using an HMAC pseudo-random
     * function.
     *
     * C++ counterpart of .NET System.Security.Cryptography.Rfc2898DeriveBytes.
     *
     * @note Restricted to SHA1/SHA256/SHA384/SHA512 (MD5 deliberately excluded), matching .NET's
     * own restriction in `OpenHmac()`. SHA3-256/384/512 are also excluded here since this
     * runtime hasn't ported SHA-3 (not in `plan.sqlite3`'s current batch — Keccak/SHA-3 is a
     * different, unrelated construction from SHA-1/2, not a simple variant).
     */
    class Rfc2898DeriveBytes : public DeriveBytes {
        std::vector<bytecs> password_;
        std::vector<bytecs> salt_; // salt bytes followed by a 4-byte big-endian block counter
        uint32_t iterations_;
        HashAlgorithmName hashAlgorithm_;
        intcs blockSize_ = 0;
        std::vector<bytecs> buffer_;
        uint32_t block_ = 0;
        size_t startIndex_ = 0;
        size_t endIndex_ = 0;

        [[nodiscard]] std::unique_ptr<HMAC> createHmac() const;
        void func();
        void initialize();

    public:
        Rfc2898DeriveBytes(std::vector<bytecs> password, std::vector<bytecs> salt, intcs iterations,
                            HashAlgorithmName hashAlgorithm = HashAlgorithmName::SHA1);
        Rfc2898DeriveBytes(const std::string& password, std::vector<bytecs> salt, intcs iterations,
                            HashAlgorithmName hashAlgorithm = HashAlgorithmName::SHA1);

        /** @return The hash algorithm used for byte derivation. */
        [[nodiscard]] const HashAlgorithmName& getHashAlgorithmProperty() const { return hashAlgorithm_; }

        /** @return The number of PBKDF2 iterations. */
        [[nodiscard]] intcs getIterationCountProperty() const { return static_cast<intcs>(iterations_); }
        /** @brief Sets the number of PBKDF2 iterations and resets the derivation state. */
        void setIterationCountProperty(intcs value);

        /** @return A copy of the salt (without the internal block counter). */
        [[nodiscard]] std::vector<bytecs> getSaltProperty() const;
        /** @brief Sets the salt and resets the derivation state. */
        void setSaltProperty(std::vector<bytecs> value);

        [[nodiscard]] std::vector<bytecs> GetBytes(intcs cb) override;
        void Reset() override { initialize(); }
    };

} // namespace System::Security::Cryptography

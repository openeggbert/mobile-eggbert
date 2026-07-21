// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Security/Cryptography/HMAC.hpp"
#include "System/Security/Cryptography/HMACSHA1.hpp"
#include "System/Security/Cryptography/HMACSHA256.hpp"
#include "System/Security/Cryptography/HMACSHA384.hpp"
#include "System/Security/Cryptography/HMACSHA512.hpp"
#include "System/Security/Cryptography/HMACSHA3_256.hpp"
#include "System/Security/Cryptography/HMACSHA3_384.hpp"
#include "System/Security/Cryptography/HMACSHA3_512.hpp"
#include "System/Security/Cryptography/HashAlgorithmName.hpp"

namespace System::Security::Cryptography {

    using SharpRuntime::bytecs;
    using SharpRuntime::intcs;

    /**
     * @brief HMAC-based Extract-and-Expand Key Derivation Function (RFC 5869).
     *
     * C++ counterpart of .NET System.Security.Cryptography.HKDF. Builds entirely on this
     * codebase's existing HMAC-SHA* implementations -- no key material of its own beyond what a
     * caller supplies, no confidentiality guarantee to get wrong (same reasoning as this
     * project's existing PBKDF2/Rfc2898DeriveBytes port, per CLAUDE.md's hash-algorithm carve-out
     * from the broader crypto-out-of-scope decision).
     */
    class HKDF {
        static intcs hashLenForAlgorithm(const HashAlgorithmName& alg) {
            const auto& n = alg.getNameProperty();
            if (n == "SHA1") return 20;
            if (n == "SHA256") return 32;
            if (n == "SHA384") return 48;
            if (n == "SHA512") return 64;
            if (n == "SHA3-256") return 32;
            if (n == "SHA3-384") return 48;
            if (n == "SHA3-512") return 64;
            throw System::ArgumentException("Unsupported hash algorithm.", "hashAlgorithmName");
        }

        static std::unique_ptr<HMAC> createHmac(const HashAlgorithmName& alg, std::vector<bytecs> key) {
            const auto& n = alg.getNameProperty();
            if (n == "SHA1") return std::make_unique<HMACSHA1>(std::move(key));
            if (n == "SHA256") return std::make_unique<HMACSHA256>(std::move(key));
            if (n == "SHA384") return std::make_unique<HMACSHA384>(std::move(key));
            if (n == "SHA512") return std::make_unique<HMACSHA512>(std::move(key));
            if (n == "SHA3-256") return std::make_unique<HMACSHA3_256>(std::move(key));
            if (n == "SHA3-384") return std::make_unique<HMACSHA3_384>(std::move(key));
            if (n == "SHA3-512") return std::make_unique<HMACSHA3_512>(std::move(key));
            throw System::ArgumentException("Unsupported hash algorithm.", "hashAlgorithmName");
        }

    public:
        HKDF() = delete;

        /**
         * @brief HKDF-Extract: condenses @p ikm (input keying material) and @p salt into a
         * fixed-length pseudorandom key.
         * C++ counterpart of .NET HKDF.Extract(HashAlgorithmName, byte[], byte[]).
         */
        [[nodiscard]] static std::vector<bytecs> Extract(
            const HashAlgorithmName& hashAlgorithmName, const std::vector<bytecs>& ikm,
            const std::vector<bytecs>& salt = {}) {
            // RFC 5869 section 2.2: HMAC-Hash(salt, IKM); an empty/absent salt is replaced with
            // hashLen zero bytes.
            std::vector<bytecs> effectiveSalt = salt.empty() ? std::vector<bytecs>(static_cast<size_t>(hashLenForAlgorithm(hashAlgorithmName)), 0) : salt;
            auto hmac = createHmac(hashAlgorithmName, effectiveSalt);
            return hmac->ComputeHash(ikm);
        }

        /**
         * @brief HKDF-Expand: expands a pseudorandom key @p prk into @p outputLength bytes of
         * output keying material.
         * C++ counterpart of .NET HKDF.Expand(HashAlgorithmName, byte[], int, byte[]).
         * @throws ArgumentOutOfRangeException if @p outputLength is negative or exceeds
         * 255 * hashLen (RFC 5869's maximum expansion limit).
         */
        [[nodiscard]] static std::vector<bytecs> Expand(
            const HashAlgorithmName& hashAlgorithmName, const std::vector<bytecs>& prk, intcs outputLength,
            const std::vector<bytecs>& info = {}) {
            if (outputLength < 0)
                throw System::ArgumentOutOfRangeException("outputLength", "Non-negative number required.");
            intcs hashLen = hashLenForAlgorithm(hashAlgorithmName);
            if (outputLength > 255 * hashLen)
                throw System::ArgumentOutOfRangeException("outputLength", "The output length for HKDF-Expand is too large.");

            std::vector<bytecs> okm;
            okm.reserve(static_cast<size_t>(outputLength));
            std::vector<bytecs> t;
            for (uint8_t counter = 1; okm.size() < static_cast<size_t>(outputLength); ++counter) {
                std::vector<bytecs> input = t;
                input.insert(input.end(), info.begin(), info.end());
                input.push_back(static_cast<bytecs>(counter));
                auto hmac = createHmac(hashAlgorithmName, prk);
                t = hmac->ComputeHash(input);
                okm.insert(okm.end(), t.begin(), t.end());
            }
            okm.resize(static_cast<size_t>(outputLength));
            return okm;
        }

        /**
         * @brief Combines Extract and Expand into a single call, deriving @p outputLength bytes
         * of key material directly from @p ikm.
         * C++ counterpart of .NET HKDF.DeriveKey(HashAlgorithmName, byte[], int, byte[], byte[]).
         */
        [[nodiscard]] static std::vector<bytecs> DeriveKey(
            const HashAlgorithmName& hashAlgorithmName, const std::vector<bytecs>& ikm, intcs outputLength,
            const std::vector<bytecs>& salt = {}, const std::vector<bytecs>& info = {}) {
            std::vector<bytecs> prk = Extract(hashAlgorithmName, ikm, salt);
            return Expand(hashAlgorithmName, prk, outputLength, info);
        }
    };

} // namespace System::Security::Cryptography

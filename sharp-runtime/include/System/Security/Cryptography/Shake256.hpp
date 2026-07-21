// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Security/Cryptography/detail/Keccak.hpp"

namespace System::Security::Cryptography {

    using SharpRuntime::bytecs;
    using SharpRuntime::intcs;

    /**
     * @brief Computes a SHAKE256 (FIPS 202) extendable-output hash, whose digest length is
     * chosen by the caller rather than fixed like SHA3-256/384/512.
     *
     * C++ counterpart of .NET System.Security.Cryptography.Shake256. Reduced scope: no
     * Stream-based overloads, matching this codebase's established HashAlgorithm reduction (see
     * HashAlgorithm.hpp's class doc-comment).
     */
    class Shake256 {
        SharpRuntimeDetail::Keccak::Sponge sponge_{136, 0x1F};

        static void checkOutputLength(intcs outputLength) {
            if (outputLength < 0) throw ArgumentOutOfRangeException("outputLength", "Non-negative number required.");
        }

    public:
        Shake256() = default;

        /** @brief Appends more data to be hashed. */
        void AppendData(const std::vector<bytecs>& data) {
            sponge_.Absorb(reinterpret_cast<const uint8_t*>(data.data()), data.size());
        }

        /**
         * @brief Finalizes the hash (absorbing padding once) and squeezes @p outputLength bytes,
         * then resets the sponge for reuse.
         */
        [[nodiscard]] std::vector<bytecs> GetHashAndReset(intcs outputLength) {
            checkOutputLength(outputLength);
            if (!sponge_.IsSqueezing()) sponge_.FinishAbsorbing();
            std::vector<bytecs> out(static_cast<size_t>(outputLength));
            sponge_.Squeeze(reinterpret_cast<uint8_t*>(out.data()), out.size());
            sponge_.Reset();
            return out;
        }

        /**
         * @brief Finalizes the hash if needed and squeezes @p outputLength MORE bytes from the
         * current squeeze position, without resetting — repeated calls extend the output stream.
         */
        [[nodiscard]] std::vector<bytecs> Read(intcs outputLength) {
            checkOutputLength(outputLength);
            if (!sponge_.IsSqueezing()) sponge_.FinishAbsorbing();
            std::vector<bytecs> out(static_cast<size_t>(outputLength));
            sponge_.Squeeze(reinterpret_cast<uint8_t*>(out.data()), out.size());
            return out;
        }

        /** @brief Resets to the initial state, discarding any absorbed data. */
        void Reset() { sponge_.Reset(); }

        /** @brief One-shot: hashes @p source and returns @p outputLength bytes of digest. */
        [[nodiscard]] static std::vector<bytecs> HashData(const std::vector<bytecs>& source, intcs outputLength) {
            Shake256 shake;
            shake.AppendData(source);
            return shake.GetHashAndReset(outputLength);
        }
    };

} // namespace System::Security::Cryptography

// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include "System/Security/Cryptography/KeyedHashAlgorithm.hpp"

namespace System::Security::Cryptography {

    /**
     * @brief Computes a Hash-based Message Authentication Code (HMAC, RFC 2104) using a
     * configurable underlying hash algorithm.
     *
     * C++ counterpart of .NET System.Security.Cryptography.HMAC.
     *
     * @note .NET's own `HMAC` base class is a near-empty shell whose `HashCore`/`HashFinal`/
     * `Initialize` all throw `PlatformNotSupportedException` — the real HMAC construction lives
     * in an internal `HMACCommon` type used only by the concrete `HMACMD5`/`HMACSHA1`/etc.
     * subclasses (both marked `ignore`/not applicable in this port). Since splitting that
     * internal plumbing out as a separate C++ type would add complexity with no benefit, this
     * class implements the real RFC 2104 construction directly: `HMAC(K,m) =
     * H((K' ^ opad) || H((K' ^ ipad) || m))`, parameterized by an inner-hash factory and block
     * size supplied by each concrete subclass's constructor.
     */
    class HMAC : public KeyedHashAlgorithm {
        std::function<std::unique_ptr<HashAlgorithm>()> createHash_;
        intcs blockSizeValue_;
        std::vector<bytecs> innerPad_;
        std::vector<bytecs> outerPad_;
        std::vector<bytecs> pendingMessage_;
        std::string hashName_;

        void derivePads();

    protected:
        HMAC(std::function<std::unique_ptr<HashAlgorithm>()> createHash, intcs blockSizeValue, std::string hashName,
             std::vector<bytecs> key)
            : createHash_(std::move(createHash)), blockSizeValue_(blockSizeValue), hashName_(std::move(hashName)) {
            keyValue_ = std::move(key);
            derivePads();
        }

        void HashCore(const std::vector<bytecs>& array, intcs offset, intcs count) override;
        std::vector<bytecs> HashFinal() override;

    public:
        /** @return The block size, in bytes, of the underlying hash algorithm. */
        [[nodiscard]] intcs getBlockSizeValueProperty() const { return blockSizeValue_; }

        /** @return The name of the underlying hash algorithm (e.g. "SHA256"). */
        [[nodiscard]] const std::string& getHashNameProperty() const { return hashName_; }

        [[nodiscard]] std::vector<bytecs> getKeyProperty() const override { return keyValue_; }

        /** @brief Sets a new key, re-deriving the inner/outer pads. */
        void setKeyProperty(std::vector<bytecs> value) override {
            keyValue_ = std::move(value);
            derivePads();
        }

        void Initialize() override { pendingMessage_.clear(); }
    };

} // namespace System::Security::Cryptography

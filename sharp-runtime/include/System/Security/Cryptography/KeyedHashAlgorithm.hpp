// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <algorithm>
#include <vector>
#include "System/Security/Cryptography/HashAlgorithm.hpp"

namespace System::Security::Cryptography {

    /**
     * @brief Represents the abstract base class from which all implementations of keyed hash
     * algorithms must derive.
     *
     * C++ counterpart of .NET System.Security.Cryptography.KeyedHashAlgorithm.
     */
    class KeyedHashAlgorithm : public HashAlgorithm {
    protected:
        std::vector<bytecs> keyValue_;

    public:
        /** @return A copy of the key used by this instance. */
        [[nodiscard]] virtual std::vector<bytecs> getKeyProperty() const { return keyValue_; }

        /** @brief Sets the key used by this instance. */
        virtual void setKeyProperty(std::vector<bytecs> value) { keyValue_ = std::move(value); }

        /** @brief Zeroes the key and releases resources used by this instance. */
        void Dispose() override {
            std::fill(keyValue_.begin(), keyValue_.end(), bytecs{0});
            keyValue_.clear();
            HashAlgorithm::Dispose();
        }
    };

} // namespace System::Security::Cryptography

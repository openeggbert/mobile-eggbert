// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Security/Cryptography/RandomNumberGenerator.hpp"

namespace System::Security::Cryptography {

    /**
     * @brief Legacy CSP-backed random number generator — delegates to the same OS-backed
     * generator as `RandomNumberGenerator::Create()` in this runtime.
     *
     * C++ counterpart of .NET System.Security.Cryptography.RNGCryptoServiceProvider (obsolete in
     * .NET itself in favor of `RandomNumberGenerator`'s static members, but reproduced here for
     * API compatibility with ported code that still references it directly).
     */
    class RNGCryptoServiceProvider : public RandomNumberGenerator {
        std::shared_ptr<RandomNumberGenerator> inner_ = RandomNumberGenerator::Create();

    public:
        void GetBytes(std::vector<bytecs>& data) override { inner_->GetBytes(data); }
    };

} // namespace System::Security::Cryptography

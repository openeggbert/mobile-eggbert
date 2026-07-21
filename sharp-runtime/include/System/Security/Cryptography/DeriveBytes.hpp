// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Security::Cryptography {

    /**
     * @brief Represents the abstract base class from which all classes that derive byte
     * sequences of a specified length derive.
     *
     * C++ counterpart of .NET System.Security.Cryptography.DeriveBytes.
     */
    class DeriveBytes {
    public:
        virtual ~DeriveBytes() = default;

        /** @return @p cb newly derived bytes, continuing from where the previous call left off. */
        [[nodiscard]] virtual std::vector<SharpRuntime::bytecs> GetBytes(SharpRuntime::intcs cb) = 0;

        /** @brief Resets the state of the operation, so subsequent GetBytes() calls restart from the beginning. */
        virtual void Reset() = 0;

        /** @brief Releases resources used by this instance. */
        virtual void Dispose() {}
    };

} // namespace System::Security::Cryptography

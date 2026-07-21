// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/IO/IsolatedStorage/IsolatedStorageScope.hpp"

namespace System::IO::IsolatedStorage {

    /**
     * @brief Abstract base class for isolated storage implementations.
     *
     * Partial C++ counterpart of .NET System.IO.IsolatedStorage.IsolatedStorage.
     *
     * @note Status: Stub — concrete functionality is in IsolatedStorageFile.
     */
    class IsolatedStorage {
    protected:
        IsolatedStorageScope scope_ = IsolatedStorageScope::None;
        IsolatedStorage() = default;
    public:
        virtual ~IsolatedStorage() = default;

        /** Removes the isolated storage scope and all its contents. */
        virtual void Remove() = 0;
    };

} // namespace System::IO::IsolatedStorage

// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/IO/IsolatedStorage/IsolatedStorageScope.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::IO::IsolatedStorage {

    using SharpRuntime::longcs;

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

        /** Returns the isolation scope for this store. */
        [[nodiscard]] IsolatedStorageScope getScopeProperty() const { return scope_; }

        /** Returns the available free space in bytes (stub returns 0). */
        [[nodiscard]] virtual longcs getAvailableFreeSpaceProperty() const { return 0; }
        /** Returns the maximum size quota in bytes (stub returns 0). */
        [[nodiscard]] virtual longcs getQuotaProperty() const { return 0; }
        /** Returns the used space in bytes (stub returns 0). */
        [[nodiscard]] virtual longcs getUsedSizeProperty() const { return 0; }

        /** Removes the isolated storage scope and all its contents. */
        virtual void Remove() = 0;
        /** Closes the isolated storage. */
        virtual void Close()  {}

        /** Attempts to increase the quota, returning true on success. Default implementation always returns false. */
        [[nodiscard]] virtual bool IncreaseQuotaTo(longcs newQuotaSize) { (void)newQuotaSize; return false; }
    };

} // namespace System::IO::IsolatedStorage

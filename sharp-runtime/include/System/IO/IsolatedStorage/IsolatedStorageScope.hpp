// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::IO::IsolatedStorage {

    /**
     * @brief Enumerates the levels of isolated storage scope.
     *
     * Partial C++ counterpart of .NET System.IO.IsolatedStorage.IsolatedStorageScope.
     *
     * @note Status: Implemented
     */
    enum class IsolatedStorageScope {
        /** No isolation scope. */
        None         = 0x00,
        /** The isolated store is scoped to the current user. */
        User         = 0x01,
        /** The isolated store is scoped to the application domain. */
        Domain       = 0x02,
        /** The isolated store is scoped to the calling assembly. */
        Assembly     = 0x04,
        /** The isolated store is scoped to roaming user data. */
        Roaming      = 0x08,
        /** The isolated store is scoped to the machine. */
        Machine      = 0x10,
        /** The isolated store is scoped to the application. */
        Application  = 0x20
    };

    /** Returns the bitwise OR of two IsolatedStorageScope values. */
    inline IsolatedStorageScope operator|(IsolatedStorageScope a, IsolatedStorageScope b) {
        return static_cast<IsolatedStorageScope>(static_cast<int>(a) | static_cast<int>(b));
    }
    /** Returns the bitwise AND of two IsolatedStorageScope values. */
    inline IsolatedStorageScope operator&(IsolatedStorageScope a, IsolatedStorageScope b) {
        return static_cast<IsolatedStorageScope>(static_cast<int>(a) & static_cast<int>(b));
    }

} // namespace System::IO::IsolatedStorage

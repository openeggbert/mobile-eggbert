// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System {

    /**
     * @brief Provides a mechanism for releasing unmanaged resources.
     *
     * C++ counterpart of .NET System.IDisposable.
     *
     * Implement this interface when a class holds resources that must be
     * released deterministically (file handles, network connections, etc.).
     * Implementations must ensure that:
     *  - Dispose() is safe to call multiple times.
     *  - Dispose() releases all resources held by the instance.
     *  - Dispose() does not generally throw exceptions.
     *
     * In C#, the @c using statement calls Dispose() automatically at the end
     * of the block. In C++, use RAII or call Dispose() explicitly.
     */
    class IDisposable {
    public:
        /**
         * @brief Performs application-defined tasks associated with freeing,
         * releasing, or resetting unmanaged resources.
         *
         * C++ counterpart of .NET IDisposable.Dispose().
         * Implementations should be idempotent (safe to call more than once).
         */
        virtual void Dispose() = 0;

        /** @brief Virtual destructor for safe polymorphic destruction. */
        virtual ~IDisposable() = default;
    };

} // namespace System
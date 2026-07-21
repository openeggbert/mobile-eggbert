// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Threading/Tasks/ValueTask.hpp"

namespace System {

    /**
     * @brief Provides a mechanism for releasing unmanaged resources asynchronously.
     *
     * C++ counterpart of .NET System.IAsyncDisposable.
     * In this port DisposeAsync() runs synchronously and returns an already-completed
     * ValueTask — the async aspect is not modelled.
     */
    class IAsyncDisposable {
    public:
        /** @brief Virtual destructor for safe polymorphic destruction. */
        virtual ~IAsyncDisposable() = default;
        /**
         * @brief Releases resources asynchronously.
         *
         * C++ counterpart of .NET IAsyncDisposable.DisposeAsync().
         * Implementations run synchronously in this port; return
         * System::Threading::Tasks::ValueTask::CompletedTask() (or an equivalent
         * already-completed ValueTask) once cleanup is done.
         */
        virtual System::Threading::Tasks::ValueTask DisposeAsync() = 0;
    };

} // namespace System

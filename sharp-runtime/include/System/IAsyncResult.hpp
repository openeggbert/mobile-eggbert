// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <any>
#include "System/Threading/WaitHandle.hpp"

namespace System {

    /**
     * @brief Represents the status of an asynchronous operation.
     *
     * C++ counterpart of .NET System.IAsyncResult.
     * Implement this interface on types returned from Begin* asynchronous methods.
     */
    class IAsyncResult {
    public:
        /** @brief Virtual destructor for safe polymorphic destruction. */
        virtual ~IAsyncResult() = default;
        /** @brief Returns true if the asynchronous operation has completed. */
        [[nodiscard]] virtual bool getIsCompletedProperty() const = 0;
        /** @brief Returns true if the asynchronous operation completed synchronously. */
        [[nodiscard]] virtual bool getCompletedSynchronouslyProperty() const = 0;
        /**
         * @brief Gets a user-defined object that qualifies or contains information about
         * an asynchronous operation.
         *
         * C++ counterpart of .NET IAsyncResult.AsyncState. Uses std::any as the
         * C++ substitute for .NET's `object?`, since this port has no universal
         * base type.
         */
        [[nodiscard]] virtual const std::any& getAsyncStateProperty() const = 0;
        /**
         * @brief Gets a WaitHandle that is used to wait for an asynchronous operation to complete.
         *
         * C++ counterpart of .NET IAsyncResult.AsyncWaitHandle.
         */
        [[nodiscard]] virtual System::Threading::WaitHandle& getAsyncWaitHandleProperty() const = 0;
    };

} // namespace System

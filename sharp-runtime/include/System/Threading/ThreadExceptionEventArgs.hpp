// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <exception>
#include "System/EventArgs.hpp"

namespace System::Threading {

    /**
     * @brief Provides data for the ThreadException event.
     *
     * C++ counterpart of .NET System.Threading.ThreadExceptionEventArgs.
     * Carries the unhandled exception that caused the event.
     */
    class ThreadExceptionEventArgs : public System::EventArgs {
        std::exception_ptr exception_;
    public:
        /**
         * @brief Constructs ThreadExceptionEventArgs from the given exception pointer.
         * @param ex Pointer to the exception that caused the event.
         */
        explicit ThreadExceptionEventArgs(std::exception_ptr ex) : exception_(std::move(ex)) {}

        /**
         * @brief Returns the exception that caused the event.
         * @return The exception stored in this event argument.
         */
        [[nodiscard]] std::exception_ptr getExceptionProperty() const { return exception_; }
    };

} // namespace System::Threading

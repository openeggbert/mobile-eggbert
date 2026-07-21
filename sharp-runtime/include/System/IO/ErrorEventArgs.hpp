// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <exception>
#include "System/EventArgs.hpp"

namespace System::IO {

    /**
     * @brief Provides data for the FileSystemWatcher.Error event.
     *
     * C++ counterpart of .NET System.IO.ErrorEventArgs.
     */
    class ErrorEventArgs : public System::EventArgs {
        std::exception_ptr exception_;

    public:
        /**
         * @brief Initializes a new instance of the ErrorEventArgs class.
         * @param exception The exception that represents the error that occurred.
         */
        explicit ErrorEventArgs(std::exception_ptr exception) : exception_(std::move(exception)) {}

        /**
         * @brief Gets the Exception that represents the error that occurred.
         *
         * C++ counterpart of .NET ErrorEventArgs.GetException().
         * @return The exception that represents the error.
         */
        [[nodiscard]] virtual std::exception_ptr GetException() const { return exception_; }
    };

} // namespace System::IO

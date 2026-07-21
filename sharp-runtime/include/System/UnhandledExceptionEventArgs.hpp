// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <exception>
#include "System/EventArgs.hpp"

namespace System {

    /**
     * @brief Provides data for the event that is raised when there is an exception
     * that is not handled in any application domain.
     *
     * C++ counterpart of .NET System.UnhandledExceptionEventArgs.
     * Wraps a std::exception_ptr (instead of System.Object) because C++ does not
     * have a universal base class for throwable objects.
     */
    class UnhandledExceptionEventArgs : public EventArgs {
        std::exception_ptr exception_;
        bool               isTerminating_;

    public:
        /**
         * @brief Initializes a new instance with the specified exception and termination flag.
         *
         * C++ counterpart of .NET UnhandledExceptionEventArgs(object exception, bool isTerminating).
         * @param ex            An exception_ptr wrapping the unhandled exception (may be null).
         * @param isTerminating true if the runtime is terminating; false otherwise.
         */
        UnhandledExceptionEventArgs(std::exception_ptr ex, bool isTerminating)
            : exception_(ex), isTerminating_(isTerminating) {}

        /**
         * @brief Gets the unhandled exception object.
         *
         * C++ counterpart of .NET UnhandledExceptionEventArgs.ExceptionObject.
         * @return A std::exception_ptr to the exception, or nullptr if none was supplied.
         */
        [[nodiscard]] std::exception_ptr getExceptionObjectProperty() const {
            return exception_;
        }

        /**
         * @brief Gets a value that indicates whether the common language runtime is
         * terminating.
         *
         * C++ counterpart of .NET UnhandledExceptionEventArgs.IsTerminating.
         * @return true if the runtime is shutting down; false otherwise.
         */
        [[nodiscard]] bool getIsTerminatingProperty() const { return isTerminating_; }
    };

} // namespace System

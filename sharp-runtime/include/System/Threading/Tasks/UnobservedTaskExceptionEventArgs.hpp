// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include "System/AggregateException.hpp"
#include "System/EventArgs.hpp"

namespace System::Threading::Tasks {

    /**
     * @brief Provides data for the event that is raised when a faulted Task's exception goes unobserved.
     *
     * C++ counterpart of .NET System.Threading.Tasks.UnobservedTaskExceptionEventArgs. This runtime
     * does not currently raise TaskScheduler::UnobservedTaskException (no finalizer-driven unobserved-
     * exception detection), so no code constructs this type internally yet; it is provided for API
     * surface compatibility with ported code that references the type directly.
     */
    class UnobservedTaskExceptionEventArgs : public System::EventArgs {
        System::AggregateException exception_;
        bool observed_ = false;

    public:
        /**
         * @brief Initializes a new instance with the unobserved exception.
         * @param exception The AggregateException that has gone unobserved.
         */
        explicit UnobservedTaskExceptionEventArgs(System::AggregateException exception)
            : exception_(std::move(exception))
        {
        }

        /**
         * @brief Marks the exception as "observed," preventing exception escalation policy.
         *
         * C++ counterpart of .NET UnobservedTaskExceptionEventArgs.SetObserved().
         */
        void SetObserved() { observed_ = true; }

        /** @brief Gets whether this exception has been marked as "observed." */
        [[nodiscard]] bool getObservedProperty() const { return observed_; }

        /** @brief Gets the exception that went unobserved. */
        [[nodiscard]] const System::AggregateException& getExceptionProperty() const { return exception_; }
    };

} // namespace System::Threading::Tasks

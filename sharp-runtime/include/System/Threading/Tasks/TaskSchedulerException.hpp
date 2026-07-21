// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <exception>
#include <string>

#include "System/Exception.hpp"

namespace System::Threading::Tasks {

    /**
     * @brief Represents an exception used to communicate an invalid operation by a TaskScheduler.
     *
     * C++ counterpart of .NET System.Threading.Tasks.TaskSchedulerException.
     */
    class TaskSchedulerException : public System::Exception {
    public:
        /** @brief Initializes a new instance with the default message. */
        TaskSchedulerException();

        /** @brief Initializes a new instance with the specified error message. */
        explicit TaskSchedulerException(const std::string& message);

        /** @brief Initializes a new instance with the default message and an inner exception. */
        explicit TaskSchedulerException(std::exception_ptr innerException);

        /** @brief Initializes a new instance with a message and an inner exception. */
        TaskSchedulerException(const std::string& message, std::exception_ptr innerException);
    };

} // namespace System::Threading::Tasks

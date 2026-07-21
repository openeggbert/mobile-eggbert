// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Threading/Tasks/TaskSchedulerException.hpp"

namespace System::Threading::Tasks {

    namespace {
        constexpr const char* kDefaultMessage = "An exception was thrown by a TaskScheduler.";
    }

    TaskSchedulerException::TaskSchedulerException()
        : Exception(kDefaultMessage)
    {
    }

    TaskSchedulerException::TaskSchedulerException(const std::string& message)
        : Exception(message)
    {
    }

    TaskSchedulerException::TaskSchedulerException(std::exception_ptr innerException)
        : Exception(kDefaultMessage, std::move(innerException))
    {
    }

    TaskSchedulerException::TaskSchedulerException(const std::string& message, std::exception_ptr innerException)
        : Exception(message, std::move(innerException))
    {
    }

} // namespace System::Threading::Tasks

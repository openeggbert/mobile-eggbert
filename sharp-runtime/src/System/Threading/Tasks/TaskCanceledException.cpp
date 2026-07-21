// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Threading/Tasks/TaskCanceledException.hpp"

#include "System/Threading/Tasks/Task.hpp"

namespace System::Threading::Tasks {

    namespace {
        constexpr const char* kDefaultMessage = "A task was canceled.";
    }

    TaskCanceledException::TaskCanceledException()
        : OperationCanceledException(kDefaultMessage)
    {
    }

    TaskCanceledException::TaskCanceledException(const std::string& message)
        : OperationCanceledException(message)
    {
    }

    TaskCanceledException::TaskCanceledException(const std::string& message, std::exception_ptr innerException)
        : OperationCanceledException(message, std::move(innerException))
    {
    }

    TaskCanceledException::TaskCanceledException(const std::string& message, std::exception_ptr innerException,
                                                  const System::Threading::CancellationToken& token)
        : OperationCanceledException(message, std::move(innerException), token)
    {
    }

    TaskCanceledException::TaskCanceledException(const Task* task)
        : OperationCanceledException(kDefaultMessage,
                                      task != nullptr ? task->getCancellationTokenProperty()
                                                       : System::Threading::CancellationToken::None())
        , canceledTask_(task)
    {
    }

} // namespace System::Threading::Tasks

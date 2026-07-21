// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/SystemException.hpp"
#include "System/Threading/Mutex.hpp"
#include "System/Threading/WaitHandle.hpp"

namespace System::Threading {

    using SharpRuntime::intcs;

    /** The exception thrown when one thread acquires a Mutex object that another thread has abandoned by exiting. */
    class AbandonedMutexException : public System::SystemException {
        intcs mutexIndex_ = -1;
        Mutex* mutex_ = nullptr;

        void SetupException(intcs location, WaitHandle* handle) {
            mutexIndex_ = location;
            mutex_ = dynamic_cast<Mutex*>(handle);
        }

    public:
        /** Initializes an AbandonedMutexException with a default message. */
        AbandonedMutexException() : SystemException("The wait completed due to an abandoned mutex.") {}
        /** Initializes an AbandonedMutexException with the specified message. */
        explicit AbandonedMutexException(const std::string& message) : SystemException(message) {}
        /** Initializes an AbandonedMutexException with a message and an inner exception. */
        AbandonedMutexException(const std::string& message, std::exception_ptr inner)
            : SystemException(message, std::move(inner)) {}
        /** Initializes an AbandonedMutexException identifying the WaitAny index and abandoned handle. */
        AbandonedMutexException(intcs location, WaitHandle* handle)
            : SystemException("The wait completed due to an abandoned mutex.") { SetupException(location, handle); }
        /** Initializes an AbandonedMutexException with a message, plus the WaitAny index and abandoned handle. */
        AbandonedMutexException(const std::string& message, intcs location, WaitHandle* handle)
            : SystemException(message) { SetupException(location, handle); }
        /** Initializes an AbandonedMutexException with a message, inner exception, WaitAny index, and abandoned handle. */
        AbandonedMutexException(const std::string& message, std::exception_ptr inner, intcs location, WaitHandle* handle)
            : SystemException(message, std::move(inner)) { SetupException(location, handle); }

        /** Returns the abandoned Mutex, if the handle passed at construction was a Mutex; otherwise nullptr. */
        [[nodiscard]] Mutex* getMutexProperty() const { return mutex_; }
        /** Returns the index of the abandoned mutex in a WaitAny call, or -1 if not applicable. */
        [[nodiscard]] intcs getMutexIndexProperty() const { return mutexIndex_; }
    };

} // namespace System::Threading

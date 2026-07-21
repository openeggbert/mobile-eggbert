// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Exception.hpp"

namespace System::Threading {

    /**
     * @brief The exception thrown when the post-phase action of a Barrier fails.
     *
     * C++ counterpart of .NET System.Threading.BarrierPostPhaseException.
     */
    class BarrierPostPhaseException : public System::Exception {
    public:
        /** @brief Initializes a BarrierPostPhaseException with a default message. */
        BarrierPostPhaseException() : Exception("The postPhaseAction failed with an exception.") {}
        /** @brief Initializes a BarrierPostPhaseException with the specified message. */
        explicit BarrierPostPhaseException(const std::string& message) : Exception(message) {}
        /** @brief Initializes a BarrierPostPhaseException wrapping the given inner exception. */
        explicit BarrierPostPhaseException(std::exception_ptr innerException)
            : Exception("The postPhaseAction failed with an exception.", std::move(innerException)) {}
        /** @brief Initializes a BarrierPostPhaseException with a message and an inner exception. */
        BarrierPostPhaseException(const std::string& message, std::exception_ptr innerException)
            : Exception(message, std::move(innerException)) {}
    };

} // namespace System::Threading

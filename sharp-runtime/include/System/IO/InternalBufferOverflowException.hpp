// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include <utility>
#include "System/SystemException.hpp"

namespace System::IO {

    /** @brief The exception that is thrown when the internal buffer overflows. @note Status: Implemented */
    class InternalBufferOverflowException : public System::SystemException {
        static constexpr SharpRuntime::intcs InternalBufferOverflowHResult = static_cast<SharpRuntime::intcs>(0x80131905);
    public:
        /** Matches .NET's default constructor, which chains to the base SystemException() message ("System error.") rather than a custom string. */
        InternalBufferOverflowException() : System::SystemException() {
            setHResultProperty(InternalBufferOverflowHResult);
        }
        /** Initializes a new InternalBufferOverflowException with the specified message. */
        explicit InternalBufferOverflowException(const std::string& message) : System::SystemException(message) {
            setHResultProperty(InternalBufferOverflowHResult);
        }
        /** Initializes a new InternalBufferOverflowException with a message and an inner exception. */
        InternalBufferOverflowException(const std::string& message, std::exception_ptr inner)
            : System::SystemException(message, std::move(inner)) {
            setHResultProperty(InternalBufferOverflowHResult);
        }
    };

} // namespace System::IO

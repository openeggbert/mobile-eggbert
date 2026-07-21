// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <exception>
#include <string>
#include "System/SystemException.hpp"

namespace System {

    /**
     * @brief The exception that is thrown when there is an internal error in the
     * execution engine of the common language runtime.
     *
     * C++ counterpart of .NET System.ExecutionEngineException (sealed).
     *
     * @note In .NET this type is marked [Obsolete]: "ExecutionEngineException
     * previously indicated an unspecified fatal error in the runtime. The runtime
     * no longer raises this exception so this type is obsolete." It is retained
     * in sharp-runtime for API completeness when porting existing C# code.
     */
    class ExecutionEngineException final : public SystemException {
    public:
        /**
         * @brief Initializes a new instance with the default engine-error message.
         *
         * C++ counterpart of .NET ExecutionEngineException().
         */
        ExecutionEngineException()
            : SystemException("Internal error in the runtime.") {
            setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131506)); // COR_E_EXECUTIONENGINE
        }

        /**
         * @brief Initializes a new instance with the specified error message.
         *
         * C++ counterpart of .NET ExecutionEngineException(string).
         * @param message The error message that explains the reason for the exception.
         */
        explicit ExecutionEngineException(const std::string& message)
            : SystemException(message) {
            setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131506)); // COR_E_EXECUTIONENGINE
        }

        /**
         * @brief Initializes a new instance with a specified error message and a
         * reference to the inner exception that is the cause of this exception.
         *
         * C++ counterpart of .NET ExecutionEngineException(string, Exception).
         * @param message The error message that explains the reason for the exception.
         * @param inner   The exception that is the cause of the current exception.
         */
        ExecutionEngineException(const std::string& message, std::exception_ptr inner)
            : SystemException(message, std::move(inner)) {
            setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131506)); // COR_E_EXECUTIONENGINE
        }
    };

} // namespace System

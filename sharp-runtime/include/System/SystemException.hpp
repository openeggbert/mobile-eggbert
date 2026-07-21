// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Created by robertvokac on 5/26/25.
//

#pragma once
#include "Exception.hpp"
#include <string>

namespace System {

    /**
     * @brief Represents the base class for exceptions defined by the system.
     *
     * C++ counterpart of .NET System.SystemException. Derives from System::Exception
     * and serves as a base class for exceptions thrown by the runtime or framework.
     */
    class SystemException : public System::Exception {
    public:
        /** @brief Initializes a new instance with a default message. */
        SystemException();

        /** @brief Initializes a new instance with the specified message. */
        explicit SystemException(const char* str);

        /** @brief Initializes a new instance with the specified message. */
        explicit SystemException(const std::string& str);

        /** @brief Initializes a new instance with the specified message and inner exception. */
        SystemException(const std::string& str, std::exception_ptr inner);
    };

} // namespace System

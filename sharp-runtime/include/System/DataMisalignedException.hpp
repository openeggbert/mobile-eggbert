// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <exception>
#include "System/SystemException.hpp"

namespace System {

    /**
     * @brief The exception thrown when a data type misalignment is detected in a
     * load or store instruction.
     *
     * C++ counterpart of .NET System.DataMisalignedException.
     */
    class DataMisalignedException : public SystemException {
    public:
        /** @brief Initializes a new instance with the default misalignment message. */
        DataMisalignedException() : SystemException("A datatype misalignment was detected in a load or store instruction.") {}
        /** @brief Initializes a new instance with the specified error message. */
        explicit DataMisalignedException(const std::string& message) : SystemException(message) {}
        /** @brief Initializes a new instance with the specified message and inner exception. */
        DataMisalignedException(const std::string& message, std::exception_ptr inner)
            : SystemException(message, std::move(inner)) {}
    };

} // namespace System

// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/SystemException.hpp"

namespace System {

    /**
     * @brief The exception that is thrown for errors in an arithmetic, casting,
     * or conversion operation.
     *
     * C++ counterpart of .NET System.ArithmeticException.
     * Base class for OverflowException, DivideByZeroException, and
     * NotFiniteNumberException.
     */
    class ArithmeticException : public SystemException {
    public:
        /**
         * @brief Initializes a new instance of ArithmeticException with the
         * specified error message (from a C string).
         *
         * C++ convenience overload — no .NET equivalent.
         * @param message The error message.
         */
        explicit ArithmeticException(const char* message)
            : SystemException(message) {
            setHResultProperty(static_cast<SharpRuntime::intcs>(0x80070216)); // COR_E_ARITHMETIC
        }
    };

} // namespace System

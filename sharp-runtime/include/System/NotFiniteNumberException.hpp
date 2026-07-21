// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <exception>
#include "System/ArithmeticException.hpp"

namespace System {

    /**
     * @brief The exception thrown when a floating-point value is positive infinity,
     * negative infinity, or Not-a-Number (NaN).
     *
     * C++ counterpart of .NET System.NotFiniteNumberException.
     */
    class NotFiniteNumberException : public ArithmeticException {
        double offending_ = 0.0;
    public:
        /**
         * @brief Initializes a new instance with the default message.
         *
         * C++ counterpart of .NET NotFiniteNumberException().
         */
        NotFiniteNumberException()
            : ArithmeticException("The number encountered was not a finite quantity.") {
            setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131528)); // COR_E_NOTFINITENUMBER
        }

        /**
         * @brief Initializes a new instance with the non-finite value that triggered it.
         *
         * C++ counterpart of .NET NotFiniteNumberException(double). Matches .NET exactly:
         * this overload does NOT set the NotFiniteNumberException-specific default message -
         * it falls through to the base ArithmeticException's own default message
         * ("Overflow or underflow in the arithmetic operation."), since .NET's source for
         * this particular constructor has no `: base(...)` message argument.
         */
        explicit NotFiniteNumberException(double offendingNumber)
            : offending_(offendingNumber) {
            setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131528)); // COR_E_NOTFINITENUMBER
        }

        /**
         * @brief Initializes a new instance with the specified message.
         *
         * C++ counterpart of .NET NotFiniteNumberException(string).
         */
        explicit NotFiniteNumberException(const std::string& message)
            : ArithmeticException(message) {
            setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131528)); // COR_E_NOTFINITENUMBER
        }

        /**
         * @brief Initializes a new instance with a message and the offending value.
         *
         * C++ counterpart of .NET NotFiniteNumberException(string, double).
         */
        NotFiniteNumberException(const std::string& message, double offendingNumber)
            : ArithmeticException(message), offending_(offendingNumber) {
            setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131528)); // COR_E_NOTFINITENUMBER
        }

        /**
         * @brief Initializes a new instance with a message and an inner exception.
         *
         * C++ counterpart of .NET NotFiniteNumberException(string, Exception).
         */
        NotFiniteNumberException(const std::string& message, std::exception_ptr inner)
            : ArithmeticException(message, std::move(inner)) {
            setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131528)); // COR_E_NOTFINITENUMBER
        }

        /**
         * @brief Initializes a new instance with a message, offending value, and inner exception.
         *
         * C++ counterpart of .NET NotFiniteNumberException(string, double, Exception).
         */
        NotFiniteNumberException(const std::string& message, double offendingNumber,
                                 std::exception_ptr inner)
            : ArithmeticException(message, std::move(inner)), offending_(offendingNumber) {
            setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131528)); // COR_E_NOTFINITENUMBER
        }

        /**
         * @brief Returns the floating-point value (NaN or infinity) that caused the exception.
         *
         * C++ counterpart of .NET NotFiniteNumberException.OffendingNumber.
         */
        [[nodiscard]] double getOffendingNumberProperty() const noexcept { return offending_; }
    };

} // namespace System

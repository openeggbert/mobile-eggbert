// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/SystemException.hpp"
#include "System/Threading/CancellationToken.hpp"

namespace System {

    /**
     * @brief The exception that is thrown in a thread upon cancellation of an
     * operation that the thread was executing.
     *
     * C++ counterpart of .NET System.OperationCanceledException.
     */
    class OperationCanceledException : public SystemException {
        Threading::CancellationToken cancellationToken_;

    public:
        /** @brief Initializes a new instance with the default cancellation message. */
        OperationCanceledException();

        /** @brief Initializes a new instance with the specified C-string message. */
        explicit OperationCanceledException(const char* message);

        /** @brief Initializes a new instance with the specified message. */
        explicit OperationCanceledException(const std::string& message);

        /**
         * @brief Initializes a new instance with a message and an inner exception.
         *
         * C++ counterpart of .NET OperationCanceledException(string, Exception).
         */
        OperationCanceledException(const std::string& message, std::exception_ptr innerException);

        /**
         * @brief Initializes a new instance with the token whose cancellation caused the exception.
         *
         * C++ counterpart of .NET OperationCanceledException(CancellationToken).
         * @param token The cancellation token associated with the operation that was canceled.
         */
        explicit OperationCanceledException(const Threading::CancellationToken& token);

        /**
         * @brief Initializes a new instance with the specified message and cancellation token.
         *
         * C++ counterpart of .NET OperationCanceledException(string, CancellationToken).
         * @param message A description of the error.
         * @param token   The cancellation token associated with the operation that was canceled.
         */
        OperationCanceledException(const std::string& message, const Threading::CancellationToken& token);

        /**
         * @brief Initializes a new instance with the specified message, inner exception, and
         * cancellation token.
         *
         * C++ counterpart of .NET OperationCanceledException(string, Exception, CancellationToken).
         * @param message        A description of the error.
         * @param innerException The exception that caused this exception.
         * @param token          The cancellation token associated with the operation that was canceled.
         */
        OperationCanceledException(const std::string& message, std::exception_ptr innerException,
                                    const Threading::CancellationToken& token);

        /**
         * @brief Gets the cancellation token associated with this exception.
         *
         * C++ counterpart of .NET OperationCanceledException.CancellationToken.
         * Defaults to a non-cancelled token if none was supplied at construction.
         */
        [[nodiscard]] const Threading::CancellationToken& getCancellationTokenProperty() const {
            return cancellationToken_;
        }
    };

} // namespace System

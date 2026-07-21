// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <exception>

namespace System::Runtime::ExceptionServices {

    /**
     * @brief Separates exception dispatch details (stack trace, etc.) from the exception object,
     * so that an error can be tracked independently of the path it takes — in particular, to
     * propagate an exception caught on one thread and rethrow it on another while preserving its
     * original context.
     *
     * C++ counterpart of .NET System.Runtime.ExceptionServices.ExceptionDispatchInfo. Built
     * directly on `std::exception_ptr`/`std::rethrow_exception`, which already preserve the
     * original throw context in the same spirit as .NET's captured dispatch state.
     *
     * Known gap: real .NET also exposes static `SetCurrentStackTrace(Exception)` /
     * `SetRemoteStackTrace(Exception, string)` helpers that inject a stack trace string into an
     * *unthrown* exception instance before it is thrown or rethrown. This port's design is built
     * around `std::exception_ptr` (which requires an exception already in flight) rather than
     * `System::Exception` instances directly, and `System::Exception::getStackTraceProperty()`
     * has no public setter — porting these two methods would need a broader design change to
     * `System::Exception`'s stack-trace-capture story, out of scope for a single audit pass.
     */
    class ExceptionDispatchInfo {
        std::exception_ptr exception_;

        explicit ExceptionDispatchInfo(std::exception_ptr exception) : exception_(std::move(exception)) {}

    public:
        /** @brief Captures @p source for later rethrowing via Throw(). */
        [[nodiscard]] static ExceptionDispatchInfo Capture(std::exception_ptr source) {
            return ExceptionDispatchInfo(std::move(source));
        }

        /** @return The captured exception. */
        [[nodiscard]] std::exception_ptr getSourceExceptionProperty() const { return exception_; }

        /** @brief Rethrows the captured exception. */
        [[noreturn]] void Throw() const { std::rethrow_exception(exception_); }

        /** @brief Captures and immediately rethrows @p source. */
        [[noreturn]] static void Throw(std::exception_ptr source) { std::rethrow_exception(source); }
    };

} // namespace System::Runtime::ExceptionServices

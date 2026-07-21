// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Exception.hpp"

namespace System::ComponentModel {

    using SharpRuntime::intcs;

    /** Represents an exception thrown by a Win32 API call, carrying the native error code. */
    class Win32Exception : public System::Exception {
        intcs nativeErrorCode_;
    public:
        /**
         * Constructs a Win32Exception from a numeric Windows error code.
         * @param errorCode The Win32 error code (e.g. from GetLastError()).
         */
        explicit Win32Exception(intcs errorCode)
            : System::Exception("Win32 error " + std::to_string(errorCode)),
              nativeErrorCode_(errorCode) {}

        /**
         * Constructs a Win32Exception with a custom message and error code.
         * @param errorCode The Win32 error code.
         * @param message   A human-readable description of the error.
         */
        Win32Exception(intcs errorCode, const std::string& message)
            : System::Exception(message), nativeErrorCode_(errorCode) {}

        /** @return The underlying Win32 error code. */
        [[nodiscard]] intcs getNativeErrorCodeProperty() const { return nativeErrorCode_; }
    };

} // namespace System::ComponentModel

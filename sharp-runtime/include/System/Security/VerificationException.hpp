// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/SystemException.hpp"

namespace System::Security {

    class VerificationException : public System::SystemException {
    public:
        VerificationException() : System::SystemException("Verification failed.") {}
        explicit VerificationException(const std::string& message) : System::SystemException(message) {}
    };

} // namespace System::Security

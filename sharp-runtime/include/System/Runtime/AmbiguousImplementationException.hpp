// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/SystemException.hpp"

namespace System::Runtime {

    class AmbiguousImplementationException : public System::SystemException {
    public:
        AmbiguousImplementationException()
            : System::SystemException("Ambiguous implementation found.") {}
        explicit AmbiguousImplementationException(const std::string& message)
            : System::SystemException(message) {}
    };

} // namespace System::Runtime

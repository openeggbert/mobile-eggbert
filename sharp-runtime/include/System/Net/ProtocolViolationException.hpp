// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/InvalidOperationException.hpp"

namespace System::Net {

    /**
     * An exception class used when an attempt is made to use an invalid protocol.
     *
     * C++ counterpart of .NET System.Net.ProtocolViolationException.
     */
    class ProtocolViolationException : public System::InvalidOperationException {
    public:
        /** Creates a new instance with no message. */
        ProtocolViolationException() = default;

        /** Creates a new instance with the specified message. */
        explicit ProtocolViolationException(const std::string& message)
            : System::InvalidOperationException(message) {}
    };

} // namespace System::Net

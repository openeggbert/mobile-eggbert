// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/IO/IOException.hpp"
namespace System::IO {
    /** @brief The exception thrown when part of a file or directory cannot be found. @note Status: Implemented */
    class DirectoryNotFoundException : public IOException {
    public:
        /** Initializes a new DirectoryNotFoundException with the specified message. */
        explicit DirectoryNotFoundException(const std::string& message);
    };
} // namespace System::IO

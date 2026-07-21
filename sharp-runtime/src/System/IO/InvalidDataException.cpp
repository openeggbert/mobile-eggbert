// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/IO/InvalidDataException.hpp"
namespace System::IO {
    InvalidDataException::InvalidDataException() : System::SystemException("Found invalid data while decoding.") {}
    InvalidDataException::InvalidDataException(const char* message) : System::SystemException(message) {}
    InvalidDataException::InvalidDataException(const std::string& message) : System::SystemException(message) {}
} // namespace System::IO

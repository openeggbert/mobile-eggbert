// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/IO/IsolatedStorage/IsolatedStorageException.hpp"

namespace System::IO::IsolatedStorage
{
    IsolatedStorageException::IsolatedStorageException(const std::string& message)
        : System::Exception(message)
    {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131450)); // COR_E_ISOSTORE
    }
}
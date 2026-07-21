// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/IO/IsolatedStorage/IsolatedStorageException.hpp"
#include <utility>

namespace System::IO::IsolatedStorage
{
    namespace {
        constexpr const char* DefaultMessage = "An error occurred while accessing IsolatedStorage.";
    }

    IsolatedStorageException::IsolatedStorageException()
        : System::Exception(DefaultMessage)
    {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131450)); // COR_E_ISOSTORE
    }

    IsolatedStorageException::IsolatedStorageException(const std::string& message)
        : System::Exception(message)
    {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131450)); // COR_E_ISOSTORE
    }

    IsolatedStorageException::IsolatedStorageException(const std::string& message, std::exception_ptr inner)
        : System::Exception(message, std::move(inner))
    {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131450)); // COR_E_ISOSTORE
    }
}
// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/IO/DirectoryNotFoundException.hpp"
namespace System::IO {
    namespace {
        constexpr SharpRuntime::intcs CorEDirectoryNotFound = static_cast<SharpRuntime::intcs>(0x80070003); // COR_E_DIRECTORYNOTFOUND
    }
    DirectoryNotFoundException::DirectoryNotFoundException(const std::string& message) : IOException(message) {
        setHResultProperty(CorEDirectoryNotFound);
    }
} // namespace System::IO

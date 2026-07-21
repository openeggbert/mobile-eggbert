// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/IO/DirectoryNotFoundException.hpp"
namespace System::IO {
    namespace {
        constexpr SharpRuntime::intcs CorEDirectoryNotFound = static_cast<SharpRuntime::intcs>(0x80070003); // COR_E_DIRECTORYNOTFOUND
    }
    DirectoryNotFoundException::DirectoryNotFoundException() : IOException("Unable to find the specified directory.") {
        setHResultProperty(CorEDirectoryNotFound);
    }
    DirectoryNotFoundException::DirectoryNotFoundException(const char* message) : IOException(message) {
        setHResultProperty(CorEDirectoryNotFound);
    }
    DirectoryNotFoundException::DirectoryNotFoundException(const std::string& message) : IOException(message) {
        setHResultProperty(CorEDirectoryNotFound);
    }
    DirectoryNotFoundException::DirectoryNotFoundException(const std::string& message, std::exception_ptr inner)
        : IOException(message, std::move(inner)) {
        setHResultProperty(CorEDirectoryNotFound);
    }
    DirectoryNotFoundException::DirectoryNotFoundException(const std::string& message, const std::string& directoryPath)
        : IOException(message), directoryPath_(directoryPath) {
        setHResultProperty(CorEDirectoryNotFound);
    }
} // namespace System::IO

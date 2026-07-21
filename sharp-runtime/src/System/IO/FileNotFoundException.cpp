// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/IO/FileNotFoundException.hpp"

namespace System::IO {

    namespace {
        constexpr SharpRuntime::intcs CorEFileNotFound = static_cast<SharpRuntime::intcs>(0x80070002);
    }

    FileNotFoundException::FileNotFoundException(const std::string& message, const std::string& fileName)
        : IOException(message), fileName_(fileName) {
        setHResultProperty(CorEFileNotFound);
    }

} // namespace System::IO

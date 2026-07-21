// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <string>

#include "System/IO/IOException.hpp"

namespace System::IO {

    /**
     * @brief The exception that is thrown when an attempt to access a file
     * that does not exist on disk fails.
     *
     * @note Status: Implemented
     */
    class FileNotFoundException : public IOException {
    private:
        std::string fileName_;

    public:
        /** Initializes a FileNotFoundException with a message and the name of the file that was not found. */
        FileNotFoundException(const std::string& message, const std::string& fileName);
    };

} // namespace System::IO

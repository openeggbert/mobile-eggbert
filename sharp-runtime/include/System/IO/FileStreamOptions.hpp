// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/IO/FileMode.hpp"
#include "System/IO/FileAccess.hpp"
#include "System/IO/FileShare.hpp"
#include "System/IO/FileOptions.hpp"

namespace System::IO {

    using SharpRuntime::intcs;
    using SharpRuntime::longcs;

    /** Specifies options used for creating a FileStream. */
    class FileStreamOptions {
    public:
        /** How the operating system should open a file. */
        FileMode   Mode    = FileMode::Open;
        /** Defines the access mode for the file. */
        FileAccess Access  = FileAccess::Read;
        /** Controls how other FileStream objects can access the file. */
        FileShare  Share   = FileShare::Read;
        /** Additional options that apply when creating the FileStream. */
        FileOptions Options = FileOptions::None;
        /** Number of bytes to preallocate on disk; 0 means no preallocation. */
        longcs     PreallocationSize = 0;
        /** Size of the internal buffer in bytes. */
        intcs      BufferSize = 4096;

        /** Initializes FileStreamOptions with default values. */
        FileStreamOptions() = default;
    };

} // namespace System::IO

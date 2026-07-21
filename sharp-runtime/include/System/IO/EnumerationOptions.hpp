// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <climits>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/IO/FileAttributes.hpp"
#include "System/IO/MatchCasing.hpp"
#include "System/IO/MatchType.hpp"
#include "System/IO/SearchTarget.hpp"

namespace System::IO {

    using SharpRuntime::intcs;

    /** Options for controlling file-system enumeration behaviour. */
    class EnumerationOptions {
    public:
        /** When true, subdirectories are searched recursively. */
        bool RecurseSubdirectories      = false;
        /** When true, inaccessible directories are silently skipped. */
        bool IgnoreInaccessible         = true;
        /** When true, special entries such as "." and ".." are returned. */
        bool ReturnSpecialDirectories   = false;
        /** Maximum recursion depth; INT_MAX means unlimited. */
        intcs  MaxRecursionDepth        = INT_MAX;
        /** Suggested buffer size in bytes; 0 means no suggestion (platform default). */
        intcs  BufferSize               = 0;
        /** Controls case sensitivity when matching names. */
        MatchCasing  MatchCasingValue   = MatchCasing::PlatformDefault;
        /** Controls how search patterns are interpreted. */
        MatchType    MatchTypeValue      = MatchType::Simple;
        /** Specifies whether to return files, directories, or both. */
        SearchTarget SearchTargetValue  = SearchTarget::Files;
        /** File attributes that cause an entry to be skipped. Default is Hidden | System (0x6). */
        FileAttributes AttributesToSkip = static_cast<FileAttributes>(0x006); // Hidden (0x02) | System (0x04)
        /** File attributes that an entry must have to be returned. */
        FileAttributes AttributesToMatch = FileAttributes::Normal;

        /** Initializes EnumerationOptions with default values. */
        EnumerationOptions() = default;
    };

} // namespace System::IO

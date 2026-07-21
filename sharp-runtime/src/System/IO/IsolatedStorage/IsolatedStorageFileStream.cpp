// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/IO/IsolatedStorage/IsolatedStorageFileStream.hpp"

#include <filesystem>

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#endif

namespace System::IO::IsolatedStorage
{
    namespace {
        std::string PrepareFullPath(const std::filesystem::path& fullPath)
        {
            std::filesystem::create_directories(fullPath.parent_path());
            return fullPath.string();
        }
    }

    IsolatedStorageFileStream::IsolatedStorageFileStream(
        const std::filesystem::path& fullPath,
        System::IO::FileMode mode)
        : System::IO::FileStream(PrepareFullPath(fullPath), mode)
    {
    }

    void IsolatedStorageFileStream::Close()
    {
        System::IO::FileStream::Close();
#if defined(__EMSCRIPTEN__)
        // Flush in-memory IDBFS writes to IndexedDB so save data survives
        // page reloads.  The callback is fire-and-forget; errors are logged
        // to the browser console.
        EM_ASM(
            FS.syncfs(false, function(err) {
                if (err) { console.warn('IDBFS post-close sync failed:', err); }
            });
        );
#endif
    }
}

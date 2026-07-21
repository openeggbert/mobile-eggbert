// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <string>
#include <vector>

namespace System::IO {

    /**
     * @brief Exposes static methods for creating, moving, and enumerating
     * through directories and subdirectories.
     *
     * Partial C++ counterpart of .NET System.IO.Directory.
     *
     * @note Status: Partial — covers the common Exists/CreateDirectory/Delete/Move/GetFiles/
     * GetDirectories/GetCurrentDirectory/SetCurrentDirectory surface. Not ported: symbolic-link
     * operations (CreateSymbolicLink, ResolveLinkTarget), Enumerate* streaming variants (only the
     * eager vector-returning Get* forms exist), the static path-based timestamp accessors
     * (Directory.GetCreationTime/GetLastAccessTime/GetLastWriteTime and their setters — the
     * equivalent instance-level getCreationTimeProperty()/getLastWriteTimeProperty()/etc. exist
     * on System::IO::FileSystemInfo instead), GetLogicalDrives, GetDirectoryRoot, GetParent, and
     * recursive SearchOption/EnumerationOptions-based search overloads.
     */
    class Directory {
    public:
        /** Prevents instantiation — all members are static. */
        Directory() = delete;

        /** Returns true if the directory at path exists. */
        [[nodiscard]] static bool Exists(const std::string& path);
        /** Creates all directories in the specified path. */
        static void               CreateDirectory(const std::string& path);
        /** Deletes the directory at path; deletes recursively when recursive is true. */
        static void               Delete(const std::string& path, bool recursive = false);
        /** Moves a directory and its contents from src to dst. */
        static void               Move(const std::string& src, const std::string& dst);

        /** Returns the paths of all files in the given directory. */
        [[nodiscard]] static std::vector<std::string> GetFiles(const std::string& path);
        /** Returns the paths of files in the given directory that match searchPattern. */
        [[nodiscard]] static std::vector<std::string> GetFiles(const std::string& path,
                                                                const std::string& searchPattern);
        /** Returns the paths of all subdirectories in the given directory. */
        [[nodiscard]] static std::vector<std::string> GetDirectories(const std::string& path);

        /** Returns the process working directory. */
        [[nodiscard]] static std::string GetCurrentDirectory();
        /** Sets the process working directory to path. */
        static void                      SetCurrentDirectory(const std::string& path);
    };

} // namespace System::IO

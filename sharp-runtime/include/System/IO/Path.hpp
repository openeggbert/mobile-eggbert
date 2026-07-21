// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <string>
#include <vector>

namespace System::IO {

    /**
     * @brief Performs operations on String instances that contain file or
     * directory path information.
     *
     * Partial C++ counterpart of .NET System.IO.Path.
     *
     * @note Status: Implemented
     */
    class Path {
    public:
        /** Prevents instantiation — all members are static. */
        Path() = delete;

#ifdef _WIN32
        /** Platform directory separator character (backslash on Windows). */
        static inline const char DirectorySeparatorChar     = '\\';
        /** Alternate directory separator character (forward slash on Windows). */
        static inline const char AltDirectorySeparatorChar  = '/';
        /** Character used to separate path strings in environment variables (semicolon on Windows). */
        static inline const char PathSeparator              = ';';
#else
        /** Platform directory separator character (forward slash on POSIX). */
        static inline const char DirectorySeparatorChar     = '/';
        /** Alternate directory separator character (forward slash on POSIX). */
        static inline const char AltDirectorySeparatorChar  = '/';
        /** Character used to separate path strings in environment variables (colon on POSIX). */
        static inline const char PathSeparator              = ':';
#endif

        /** Combines two path strings into a single path. */
        [[nodiscard]] static std::string Combine(const std::string& path1, const std::string& path2);
        /** Combines three path strings into a single path. */
        [[nodiscard]] static std::string Combine(const std::string& path1, const std::string& path2,
                                                  const std::string& path3);

        /** Returns the file name and extension from the path. */
        [[nodiscard]] static std::string GetFileName(const std::string& path);
        /** Returns the file name from the path without the extension. */
        [[nodiscard]] static std::string GetFileNameWithoutExtension(const std::string& path);
        /** Returns the extension of the file (including the dot). */
        [[nodiscard]] static std::string GetExtension(const std::string& path);
        /** Returns the directory part of the path. */
        [[nodiscard]] static std::string GetDirectoryName(const std::string& path);
        /** Returns the absolute path for the specified path string. */
        [[nodiscard]] static std::string GetFullPath(const std::string& path);
        /** Returns the path of the system temporary directory. */
        [[nodiscard]] static std::string GetTempPath();
        /** Creates a uniquely named temporary file and returns its full path. */
        [[nodiscard]] static std::string GetTempFileName();
        /** Changes the extension of a path string. */
        [[nodiscard]] static std::string ChangeExtension(const std::string& path,
                                                          const std::string& extension);

        /** Returns true if path includes a file extension. */
        [[nodiscard]] static bool HasExtension(const std::string& path);
        /** Returns true if path contains a root. */
        [[nodiscard]] static bool IsPathRooted(const std::string& path);
    };

} // namespace System::IO

// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <string>
#include <vector>

#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::IO {

    /**
     * @brief Provides static methods for the creation, copying, deletion,
     * moving, and opening of files.
     *
     * Partial C++ counterpart of .NET System.IO.File.
     *
     * @note Status: Partial
     */
    class File {
    public:
        /** Prevents instantiation — all members are static. */
        File() = delete;

        /** Returns true if the specified file exists. */
        [[nodiscard]] static bool        Exists(const std::string& path);
        /** Deletes the specified file. */
        static void                      Delete(const std::string& path);
        /** Copies an existing file; overwrites the destination when overwrite is true. */
        static void                      Copy(const std::string& src, const std::string& dst, bool overwrite = false);
        /** Moves a specified file to a new location. */
        static void                      Move(const std::string& src, const std::string& dst);

        /** Opens a file, reads all text, then closes the file. */
        [[nodiscard]] static std::string         ReadAllText(const std::string& path);
        /** Creates a new file, writes the specified string, then closes the file. */
        static void                              WriteAllText(const std::string& path, const std::string& contents);
        /** Opens a file, reads all lines into a vector, then closes the file. */
        [[nodiscard]] static std::vector<std::string> ReadAllLines(const std::string& path);
        /** Creates a new file, writes the lines, then closes the file. */
        static void                              WriteAllLines(const std::string& path,
                                                               const std::vector<std::string>& lines);

        /** Opens a binary file, reads the contents into a byte vector, then closes the file. */
        [[nodiscard]] static std::vector<SharpRuntime::bytecs> ReadAllBytes(const std::string& path);
        /** Creates a new file, writes the byte array, then closes the file. */
        static void WriteAllBytes(const std::string& path,
                                  const std::vector<SharpRuntime::bytecs>& bytes);

        /** Appends the specified string to the file, creating it if it does not exist. */
        static void AppendAllText(const std::string& path, const std::string& contents);
    };

} // namespace System::IO

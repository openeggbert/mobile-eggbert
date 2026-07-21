// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/IO/Compression/CompressionLevel.hpp"
#include "System/IO/Compression/ZipArchive.hpp"

namespace System::IO::Compression {

    /**
     * @brief Provides static methods for creating, extracting, and opening zip archives.
     *
     * C++ counterpart of .NET System.IO.Compression.ZipFile.
     *
     * @note Status: PARTIAL — the Stream-based overloads and entryNameEncoding parameter from
     * .NET are not ported; only the path-based overloads are implemented.
     */
    class ZipFile {
    public:
        /** Prevents instantiation — all members are static. */
        ZipFile() = delete;

        /** Opens the zip archive at @p archiveFileName for reading. */
        [[nodiscard]] static ZipArchive OpenRead(const std::string& archiveFileName);

        /** Opens the zip archive at @p archiveFileName with the specified mode. */
        [[nodiscard]] static ZipArchive Open(const std::string& archiveFileName, ZipArchiveMode mode);

        /**
         * @brief Extracts all files in @p sourceArchiveFileName to @p destinationDirectoryName.
         * @throws System::IO::IOException if a destination file already exists.
         */
        static void ExtractToDirectory(const std::string& sourceArchiveFileName, const std::string& destinationDirectoryName);
        /** Extracts all files in @p sourceArchiveFileName to @p destinationDirectoryName, optionally overwriting existing files. */
        static void ExtractToDirectory(const std::string& sourceArchiveFileName, const std::string& destinationDirectoryName,
                                        bool overwriteFiles);

        /** Creates a zip archive at @p destinationArchiveFileName containing the files in @p sourceDirectoryName. */
        static void CreateFromDirectory(const std::string& sourceDirectoryName, const std::string& destinationArchiveFileName);
        /**
         * @brief Creates a zip archive at @p destinationArchiveFileName containing the files in @p sourceDirectoryName.
         * @param compressionLevel Compression level to apply to every entry.
         * @param includeBaseDirectory When true, includes the name of the source directory as the top-level
         *        directory of every entry within the archive.
         */
        static void CreateFromDirectory(const std::string& sourceDirectoryName, const std::string& destinationArchiveFileName,
                                         CompressionLevel compressionLevel, bool includeBaseDirectory);
    };

} // namespace System::IO::Compression

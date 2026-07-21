// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/IO/Compression/CompressionLevel.hpp"
#include "System/IO/Compression/ZipArchive.hpp"

namespace System::IO::Compression {

    /**
     * @brief Provides extension methods for the ZipArchive and ZipArchiveEntry classes.
     *
     * .NET implements these as C# extension methods on ZipArchive/ZipArchiveEntry; since C++
     * has no extension-method mechanism, they are static methods taking the target instance
     * as the first parameter — the same shape the C# compiler desugars extension methods to.
     *
     * C++ counterpart of .NET System.IO.Compression.ZipFileExtensions.
     */
    class ZipFileExtensions {
    public:
        /** Prevents instantiation — all members are static. */
        ZipFileExtensions() = delete;

        /**
         * @brief Creates an entry in @p archive with the given name from the file at @p sourceFileName.
         * @throws System::IO::FileNotFoundException if @p sourceFileName does not exist.
         */
        static ZipArchiveEntry CreateEntryFromFile(ZipArchive& archive, const std::string& sourceFileName,
                                                    const std::string& entryName);
        /** Creates an entry in @p archive using the specified compression level. */
        static ZipArchiveEntry CreateEntryFromFile(ZipArchive& archive, const std::string& sourceFileName,
                                                    const std::string& entryName, CompressionLevel compressionLevel);

        /** Extracts all entries in @p archive to @p destinationDirectoryName, throwing if a file would be overwritten. */
        static void ExtractToDirectory(ZipArchive& archive, const std::string& destinationDirectoryName);
        /** Extracts all entries in @p archive to @p destinationDirectoryName. */
        static void ExtractToDirectory(ZipArchive& archive, const std::string& destinationDirectoryName, bool overwriteFiles);

        /** Extracts @p entry to @p destinationFileName, throwing if the file already exists. */
        static void ExtractToFile(ZipArchiveEntry& entry, const std::string& destinationFileName);
        /** Extracts @p entry to @p destinationFileName. */
        static void ExtractToFile(ZipArchiveEntry& entry, const std::string& destinationFileName, bool overwrite);
    };

} // namespace System::IO::Compression

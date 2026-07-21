// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/IO/Compression/ZipFile.hpp"
#include "System/IO/Compression/ZipFileExtensions.hpp"
#include "System/IO/Directory.hpp"
#include "System/IO/DirectoryNotFoundException.hpp"

#include <filesystem>

namespace System::IO::Compression {

    ZipArchive ZipFile::OpenRead(const std::string& archiveFileName) {
        return ZipArchive(archiveFileName, ZipArchiveMode::Read);
    }

    ZipArchive ZipFile::Open(const std::string& archiveFileName, ZipArchiveMode mode) {
        return ZipArchive(archiveFileName, mode);
    }

    void ZipFile::ExtractToDirectory(const std::string& sourceArchiveFileName, const std::string& destinationDirectoryName) {
        ExtractToDirectory(sourceArchiveFileName, destinationDirectoryName, false);
    }

    void ZipFile::ExtractToDirectory(const std::string& sourceArchiveFileName, const std::string& destinationDirectoryName,
                                      bool overwriteFiles) {
        ZipArchive archive(sourceArchiveFileName, ZipArchiveMode::Read);
        ZipFileExtensions::ExtractToDirectory(archive, destinationDirectoryName, overwriteFiles);
    }

    void ZipFile::CreateFromDirectory(const std::string& sourceDirectoryName, const std::string& destinationArchiveFileName) {
        CreateFromDirectory(sourceDirectoryName, destinationArchiveFileName, CompressionLevel::Optimal, false);
    }

    void ZipFile::CreateFromDirectory(const std::string& sourceDirectoryName, const std::string& destinationArchiveFileName,
                                       CompressionLevel compressionLevel, bool includeBaseDirectory) {
        namespace fs = std::filesystem;

        if (!System::IO::Directory::Exists(sourceDirectoryName)) {
            throw System::IO::DirectoryNotFoundException(
                "Could not find a part of the path '" + sourceDirectoryName + "'.", sourceDirectoryName);
        }

        const fs::path sourceRoot = fs::path(sourceDirectoryName);
        const std::string entryPrefix = includeBaseDirectory ? sourceRoot.filename().string() + "/" : "";

        ZipArchive archive(destinationArchiveFileName, ZipArchiveMode::Create);

        std::error_code ec;
        for (const auto& dirEntry : fs::recursive_directory_iterator(sourceRoot, ec)) {
            if (!dirEntry.is_regular_file()) continue;

            const fs::path relative = fs::relative(dirEntry.path(), sourceRoot);
            std::string entryName = entryPrefix + relative.generic_string();

            ZipFileExtensions::CreateEntryFromFile(archive, dirEntry.path().string(), entryName, compressionLevel);
        }
    }

} // namespace System::IO::Compression

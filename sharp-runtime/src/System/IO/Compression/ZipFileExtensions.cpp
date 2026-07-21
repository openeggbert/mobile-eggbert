// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/IO/Compression/ZipFileExtensions.hpp"
#include "System/IO/Directory.hpp"
#include "System/IO/File.hpp"
#include "System/IO/FileMode.hpp"
#include "System/IO/FileStream.hpp"
#include "System/IO/IOException.hpp"
#include "System/IO/Path.hpp"

#include <memory>

namespace System::IO::Compression {

    ZipArchiveEntry ZipFileExtensions::CreateEntryFromFile(ZipArchive& archive, const std::string& sourceFileName,
                                                            const std::string& entryName) {
        return CreateEntryFromFile(archive, sourceFileName, entryName, CompressionLevel::Optimal);
    }

    ZipArchiveEntry ZipFileExtensions::CreateEntryFromFile(ZipArchive& archive, const std::string& sourceFileName,
                                                            const std::string& entryName, CompressionLevel compressionLevel) {
        auto entry = archive.CreateEntry(entryName, compressionLevel);

        auto data = System::IO::File::ReadAllBytes(sourceFileName);
        std::unique_ptr<System::IO::Stream> dest(entry.Open());
        if (!data.empty()) {
            dest->Write(data.data(), 0, static_cast<System::IO::intcs>(data.size()));
        }
        return entry;
    }

    void ZipFileExtensions::ExtractToDirectory(ZipArchive& archive, const std::string& destinationDirectoryName) {
        ExtractToDirectory(archive, destinationDirectoryName, false);
    }

    void ZipFileExtensions::ExtractToDirectory(ZipArchive& archive, const std::string& destinationDirectoryName,
                                                bool overwriteFiles) {
        // Real .NET's ExtractRelativeToDirectoryCheckIfFile resolves each entry's destination to a
        // full path and rejects it (IOException) unless it stays under the destination directory --
        // this port previously used Path::Combine directly with no such check, so a malicious entry
        // name like "../../etc/passwd" (a "Zip Slip" path) would write outside destinationDirectoryName
        // entirely. Mirrored here: resolve the destination directory to a full path once, then for
        // every entry resolve its full destination path and verify it is still prefixed by the
        // destination directory (with a trailing separator, so "dest-evil" can't falsely pass a
        // "dest" prefix check) before writing anything.
        System::IO::Directory::CreateDirectory(destinationDirectoryName);
        std::string destinationDirectoryFullPath = System::IO::Path::GetFullPath(destinationDirectoryName);
        if (destinationDirectoryFullPath.empty() ||
            destinationDirectoryFullPath.back() != System::IO::Path::DirectorySeparatorChar) {
            destinationDirectoryFullPath += System::IO::Path::DirectorySeparatorChar;
        }

        for (auto& entry : archive.getEntriesProperty()) {
            const std::string& fullName = entry.getFullNameProperty();
            // Entries whose name ends with a directory separator represent directories; skip them.
            if (!fullName.empty() && (fullName.back() == '/' || fullName.back() == '\\')) {
                continue;
            }

            const std::string destPath = System::IO::Path::Combine(destinationDirectoryName, fullName);
            const std::string destPathFull = System::IO::Path::GetFullPath(destPath);
            if (destPathFull.compare(0, destinationDirectoryFullPath.size(), destinationDirectoryFullPath) != 0) {
                throw System::IO::IOException(
                    "Extracting Zip entry would have resulted in a file outside the specified destination directory.");
            }

            const std::string destDir = System::IO::Path::GetDirectoryName(destPathFull);
            if (!destDir.empty() && !System::IO::Directory::Exists(destDir)) {
                System::IO::Directory::CreateDirectory(destDir);
            }

            ExtractToFile(entry, destPathFull, overwriteFiles);
        }
    }

    void ZipFileExtensions::ExtractToFile(ZipArchiveEntry& entry, const std::string& destinationFileName) {
        ExtractToFile(entry, destinationFileName, false);
    }

    void ZipFileExtensions::ExtractToFile(ZipArchiveEntry& entry, const std::string& destinationFileName, bool overwrite) {
        const System::IO::FileMode mode = overwrite ? System::IO::FileMode::Create : System::IO::FileMode::CreateNew;
        System::IO::FileStream out(destinationFileName, mode, System::IO::FileAccess::Write);

        std::unique_ptr<System::IO::Stream> source(entry.Open());
        System::IO::bytecs buffer[65536];
        System::IO::intcs n;
        while ((n = source->Read(buffer, 0, static_cast<System::IO::intcs>(sizeof(buffer)))) > 0) {
            out.Write(buffer, 0, n);
        }
        out.Close();
    }

} // namespace System::IO::Compression

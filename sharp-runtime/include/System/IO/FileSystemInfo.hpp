// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <filesystem>
#include <string>
#include "System/DateTime.hpp"

namespace System::IO {

    /**
     * @brief Provides the base class for both FileInfo and DirectoryInfo objects.
     *
     * Partial C++ counterpart of .NET System.IO.FileSystemInfo.
     *
     * @note Status: Partial — `UnixFileMode`, `LinkTarget`, `CreateAsSymbolicLink`, and
     *   `ResolveLinkTarget` are not implemented (documented gap, not silently wrong).
     *   `CreationTime`/`CreationTimeUtc` setters are not provided: reliably setting a
     *   file's birth time has no portable C++ standard library support and is not
     *   supported by most POSIX filesystems at all. `CreationTime` getters on POSIX use
     *   `st_ctime` (last inode-metadata-change time) as an approximation of birth time,
     *   since `<sys/stat.h>` on Linux has no portable birth-time field without a
     *   `statx()` feature-test — the same approximation real .NET itself documents for
     *   `File.GetCreationTimeUtc` on Linux prior to kernel/glibc `statx` support.
     */
    class FileSystemInfo {
    protected:
        std::filesystem::path fullPath_;
        std::string           originalPath_;

        /** @brief Constructs the base state from the path passed to a derived FileInfo/DirectoryInfo. */
        explicit FileSystemInfo(const std::string& path)
            : fullPath_(std::filesystem::absolute(path)), originalPath_(path) {}

    public:
        virtual ~FileSystemInfo() = default;

        /** @return The full absolute path of the file or directory. */
        [[nodiscard]] virtual std::string getFullNameProperty() const { return fullPath_.string(); }

        /** @return The extension of the file/directory name, including the leading dot, or empty if none. */
        [[nodiscard]] std::string getExtensionProperty() const { return fullPath_.extension().string(); }

        /** @return The name of the file or the last directory component (abstract in .NET; overridden by FileInfo/DirectoryInfo). */
        [[nodiscard]] virtual std::string getNameProperty() const = 0;

        /** @return True if the file or directory exists (abstract in .NET; overridden by FileInfo/DirectoryInfo). */
        [[nodiscard]] virtual bool getExistsProperty() const = 0;

        /** @brief Deletes the file or directory (abstract in .NET; overridden by FileInfo/DirectoryInfo). */
        virtual void Delete() = 0;

        /** @return The creation time of the file/directory, in local time (approximated via st_ctime on POSIX — see class doc comment). */
        [[nodiscard]] System::DateTime getCreationTimeProperty() const;
        /** @return The creation time of the file/directory, in UTC (approximated via st_ctime on POSIX — see class doc comment). */
        [[nodiscard]] System::DateTime getCreationTimeUtcProperty() const;

        /** @return The last access time of the file/directory, in local time. */
        [[nodiscard]] System::DateTime getLastAccessTimeProperty() const;
        /** @return The last access time of the file/directory, in UTC. */
        [[nodiscard]] System::DateTime getLastAccessTimeUtcProperty() const;

        /** @return The last write time of the file/directory, in local time. */
        [[nodiscard]] System::DateTime getLastWriteTimeProperty() const;
        /** @return The last write time of the file/directory, in UTC. */
        [[nodiscard]] System::DateTime getLastWriteTimeUtcProperty() const;

        /** @brief Sets the last write time of the file/directory, given in local time. */
        void setLastWriteTimeProperty(const System::DateTime& value);
        /** @brief Sets the last write time of the file/directory, given in UTC. */
        void setLastWriteTimeUtcProperty(const System::DateTime& value);

        /** @return The original path passed to the constructor (matches .NET's ToString(), which is NOT FullName). */
        [[nodiscard]] virtual std::string ToString() const { return originalPath_; }
    };

} // namespace System::IO

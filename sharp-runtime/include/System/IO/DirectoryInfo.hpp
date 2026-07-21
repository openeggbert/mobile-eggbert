// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <filesystem>
#include <string>
#include <vector>
#include "System/IO/DirectoryNotFoundException.hpp"
#include "System/IO/FileInfo.hpp"
#include "System/IO/FileSystemInfo.hpp"
#include "System/IO/IOException.hpp"

namespace System::IO {

    /**
     * @brief Exposes instance methods for creating, moving, and enumerating through directories and subdirectories.
     *
     * Wraps std::filesystem. Partial C++ counterpart of .NET System.IO.DirectoryInfo.
     *
     * @note Status: Partial — covers Create/Delete/MoveTo/GetFiles/GetDirectories and the common
     * FileSystemInfo properties. Not ported: CreateSubdirectory, the Enumerate*
     * (EnumerateFiles/EnumerateDirectories/EnumerateFileSystemInfos) streaming variants,
     * GetFileSystemInfos, the Root property, and searchPattern/SearchOption-based overloads of
     * GetFiles/GetDirectories. GetFileNames() is a port-added convenience method with no real
     * .NET equivalent (real .NET's GetFiles() already returns FileInfo[], not paths).
     */
    class DirectoryInfo : public FileSystemInfo {
    public:
        /** Constructs a DirectoryInfo for the specified path. */
        explicit DirectoryInfo(const std::string& path) : FileSystemInfo(path) {}

        /** Returns the last component of the directory path. */
        [[nodiscard]] std::string getNameProperty() const override { return fullPath_.filename().string(); }

        /** Returns true if the directory exists. Never throws, matching .NET. */
        [[nodiscard]] bool getExistsProperty() const override {
            std::error_code ec;
            bool isDir = std::filesystem::is_directory(fullPath_, ec);
            return !ec && isDir;
        }

        /** Returns the parent directory. */
        [[nodiscard]] DirectoryInfo getParentProperty() const {
            return DirectoryInfo(fullPath_.parent_path().string());
        }

        /** Creates the directory and all intermediate directories. */
        void Create() {
            std::error_code ec;
            std::filesystem::create_directories(fullPath_, ec);
            if (ec) throw IOException("Failed to create directory: " + ec.message());
        }

        /**
         * @brief Deletes this directory; deletes all contents recursively when recursive is true.
         * @throws System::IO::DirectoryNotFoundException if the directory does not exist.
         */
        void Delete(bool recursive) {
            if (!getExistsProperty())
                throw DirectoryNotFoundException("Could not find a part of the path '" + fullPath_.string() + "'.");
            std::error_code ec;
            if (recursive) std::filesystem::remove_all(fullPath_, ec);
            else           std::filesystem::remove(fullPath_, ec);
            if (ec) throw IOException("Failed to delete directory: " + ec.message());
        }

        /**
         * @brief Deletes this directory (non-recursive).
         * @throws System::IO::DirectoryNotFoundException if the directory does not exist.
         */
        void Delete() override { Delete(false); }

        /**
         * @brief Moves the directory to destDirName.
         * @throws System::IO::DirectoryNotFoundException if this directory does not exist.
         */
        void MoveTo(const std::string& destDirName) {
            if (!getExistsProperty())
                throw DirectoryNotFoundException("Could not find a part of the path '" + fullPath_.string() + "'.");
            // Real .NET's MoveTo does not replace an existing destination directory.
            if (std::filesystem::exists(destDirName))
                throw IOException("Cannot create '" + destDirName + "' because a file or directory with the same name already exists.");
            std::error_code ec;
            std::filesystem::rename(fullPath_, destDirName, ec);
            if (ec) throw IOException("Failed to move directory: " + ec.message());
            fullPath_ = std::filesystem::absolute(destDirName);
            originalPath_ = destDirName;
        }

        /**
         * @brief Returns a list of FileInfo objects for the files in this directory.
         * @throws System::IO::DirectoryNotFoundException if this directory does not exist.
         */
        [[nodiscard]] std::vector<FileInfo> GetFiles() const {
            if (!getExistsProperty())
                throw DirectoryNotFoundException("Could not find a part of the path '" + fullPath_.string() + "'.");
            std::vector<FileInfo> result;
            for (auto& e : std::filesystem::directory_iterator(fullPath_))
                if (e.is_regular_file()) result.emplace_back(e.path().string());
            return result;
        }

        /**
         * @brief Returns a list of file paths for the files in this directory.
         * @throws System::IO::DirectoryNotFoundException if this directory does not exist.
         */
        [[nodiscard]] std::vector<std::string> GetFileNames() const {
            if (!getExistsProperty())
                throw DirectoryNotFoundException("Could not find a part of the path '" + fullPath_.string() + "'.");
            std::vector<std::string> result;
            for (auto& e : std::filesystem::directory_iterator(fullPath_))
                if (e.is_regular_file()) result.push_back(e.path().string());
            return result;
        }

        /**
         * @brief Returns a list of DirectoryInfo objects for the subdirectories of this directory.
         * @throws System::IO::DirectoryNotFoundException if this directory does not exist.
         */
        [[nodiscard]] std::vector<DirectoryInfo> GetDirectories() const {
            if (!getExistsProperty())
                throw DirectoryNotFoundException("Could not find a part of the path '" + fullPath_.string() + "'.");
            std::vector<DirectoryInfo> result;
            for (auto& e : std::filesystem::directory_iterator(fullPath_))
                if (e.is_directory()) result.emplace_back(e.path().string());
            return result;
        }
    };

} // namespace System::IO

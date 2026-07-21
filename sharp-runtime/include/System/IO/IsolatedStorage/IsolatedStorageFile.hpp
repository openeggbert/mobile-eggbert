// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "System/IO/FileMode.hpp"
#include "System/IO/IsolatedStorage/IsolatedStorage.hpp"
#include "System/IO/IsolatedStorage/IsolatedStorageScope.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::IO::IsolatedStorage
{
    class IsolatedStorageFileStream;

    /**
     * @brief Represents a scope of isolated storage.
     *
     * Practical port of .NET System.IO.IsolatedStorage.IsolatedStorageFile.
     * All paths passed to methods are interpreted relative to the storage root.
     *
     * @note Status: DONE
     */
    class IsolatedStorageFile : public IsolatedStorage
    {
    private:
        std::filesystem::path rootDirectory_; ///< Root directory of this isolated storage scope.
        bool disposed_ = false;               ///< True after Close()/Dispose().

        /** Returns the full absolute path for a relative path inside the store. */
        [[nodiscard]] std::filesystem::path fullPath(const std::string& relativePath) const;

        /** @throws System::ObjectDisposedException if this store has been Close()d/Remove()d/Dispose()d. */
        void throwIfDisposed() const;

    public:
        /** Constructs an IsolatedStorageFile rooted at @p rootDirectory with the given scope. */
        explicit IsolatedStorageFile(const std::filesystem::path& rootDirectory,
                                      IsolatedStorageScope scope = IsolatedStorageScope::None);

        /** Returns an isolated storage scoped to the current application. */
        [[nodiscard]] static IsolatedStorageFile GetUserStoreForApplication();

        /** Returns an isolated storage scoped to the current assembly (same root as application). */
        [[nodiscard]] static IsolatedStorageFile GetUserStoreForAssembly();

        // --- File operations ---

        /** Returns true if the specified relative path exists as a file in isolated storage. */
        [[nodiscard]] bool FileExists(const std::string& relativePath) const;

        /** Opens a file inside isolated storage with the specified mode. */
        [[nodiscard]] IsolatedStorageFileStream OpenFile(
            const std::string& relativePath,
            System::IO::FileMode mode) const;

        /** Creates a new file (or truncates an existing one) and returns its stream. */
        [[nodiscard]] IsolatedStorageFileStream CreateFile(const std::string& relativePath) const;

        /** Deletes the specified file from isolated storage. */
        void DeleteFile(const std::string& relativePath) const;

        /** Copies @p sourceFileName to @p destinationFileName; does not overwrite by default. */
        void CopyFile(const std::string& sourceFileName, const std::string& destinationFileName) const;

        /** Copies @p sourceFileName to @p destinationFileName; overwrites if @p overwrite is true. */
        void CopyFile(const std::string& sourceFileName, const std::string& destinationFileName, bool overwrite) const;

        /** Moves (renames) a file within isolated storage. */
        void MoveFile(const std::string& sourceFileName, const std::string& destinationFileName) const;

        /** Returns names of all files matching @p searchPattern ("*" matches everything). */
        [[nodiscard]] std::vector<std::string> GetFileNames(const std::string& searchPattern = "*") const;

        // --- Directory operations ---

        /** Returns true if the specified relative path exists as a directory in isolated storage. */
        [[nodiscard]] bool DirectoryExists(const std::string& relativePath) const;

        /** Creates the specified directory (and any intermediate directories) inside isolated storage. */
        void CreateDirectory(const std::string& relativePath) const;

        /** Deletes the specified directory and all its contents from isolated storage. */
        void DeleteDirectory(const std::string& relativePath) const;

        /** Moves (renames) a directory within isolated storage. */
        void MoveDirectory(const std::string& sourceDirectoryName, const std::string& destinationDirectoryName) const;

        /** Returns names of all directories matching @p searchPattern ("*" matches everything). */
        [[nodiscard]] std::vector<std::string> GetDirectoryNames(const std::string& searchPattern = "*") const;

        // --- Store lifecycle ---

        /** Removes the entire isolated storage store and all its contents. */
        void Remove() override;

        /** Closes the isolated storage file (marks as disposed). */
        void Close() override;

        /** Releases all resources used by the isolated storage. */
        void Dispose();

        // --- Space properties ---

        /** Returns the available free space in the store's volume in bytes. */
        [[nodiscard]] SharpRuntime::longcs getAvailableFreeSpaceProperty() const override;

        /** Returns the approximate used size of the store in bytes (sum of all file sizes). */
        [[nodiscard]] SharpRuntime::longcs getUsedSizeProperty() const override;

        /** Returns the store's quota in bytes. This runtime does not enforce quotas, so this is always longcs's maximum value. */
        [[nodiscard]] SharpRuntime::longcs getQuotaProperty() const override;

        /** Returns the root directory path for this isolated storage scope. */
        [[nodiscard]] const std::filesystem::path& getRootDirectoryProperty() const;
    };
}

// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <experimental/filesystem>
#include <string>

#include "System/IO/FileMode.hpp"
#include "System/IO/IsolatedStorage/IsolatedStorage.hpp"
#include "System/IO/IsolatedStorage/IsolatedStorageScope.hpp"

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
        std::experimental::filesystem::path rootDirectory_; ///< Root directory of this isolated storage scope.
        bool disposed_ = false;               ///< True after Close()/Dispose().

        /** Returns the full absolute path for a relative path inside the store. */
        [[nodiscard]] std::experimental::filesystem::path fullPath(const std::string& relativePath) const;

        /** @throws System::ObjectDisposedException if this store has been Close()d/Remove()d/Dispose()d. */
        void throwIfDisposed() const;

    public:
        /** Constructs an IsolatedStorageFile rooted at @p rootDirectory with the given scope. */
        explicit IsolatedStorageFile(const std::experimental::filesystem::path& rootDirectory,
                                      IsolatedStorageScope scope = IsolatedStorageScope::None);

        /** Returns an isolated storage scoped to the current application. */
        [[nodiscard]] static IsolatedStorageFile GetUserStoreForApplication();

        // --- File operations ---

        /** Returns true if the specified relative path exists as a file in isolated storage. */
        [[nodiscard]] bool FileExists(const std::string& relativePath) const;

        /** Opens a file inside isolated storage with the specified mode. */
        [[nodiscard]] IsolatedStorageFileStream OpenFile(
            const std::string& relativePath,
            System::IO::FileMode mode) const;

        /** Deletes the specified file from isolated storage. */
        void DeleteFile(const std::string& relativePath) const;

        // --- Store lifecycle ---

        /** Removes the entire isolated storage store and all its contents. */
        void Remove() override;
    };
}

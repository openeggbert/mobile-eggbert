// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/IO/IsolatedStorage/IsolatedStorageFile.hpp"

#include <filesystem>

#include "SharpRuntime/Storage/StoragePaths.hpp"
#include "System/IO/FileMode.hpp"
#include "System/IO/IsolatedStorage/IsolatedStorageException.hpp"
#include "System/IO/IsolatedStorage/IsolatedStorageFileStream.hpp"
#include "System/ObjectDisposedException.hpp"

namespace System::IO::IsolatedStorage
{
    IsolatedStorageFile::IsolatedStorageFile(const std::filesystem::path& rootDirectory, IsolatedStorageScope scope)
        : rootDirectory_(rootDirectory)
    {
        scope_ = scope;
        std::filesystem::create_directories(rootDirectory_);
    }

    std::filesystem::path IsolatedStorageFile::fullPath(const std::string& relativePath) const
    {
        return rootDirectory_ / relativePath;
    }

    // Verified against IsolatedStorageFile.cs's EnsureStoreIsValid(): real .NET checks this at
    // the top of every file/directory operation. This port set disposed_ in Close()/Remove()/
    // Dispose() but never checked it anywhere, so every operation remained silently usable on a
    // closed/removed store.
    void IsolatedStorageFile::throwIfDisposed() const
    {
        if (disposed_)
            throw System::ObjectDisposedException("IsolatedStorageFile", "Store must be open for this operation.");
    }

    IsolatedStorageFile IsolatedStorageFile::GetUserStoreForApplication()
    {
        return IsolatedStorageFile(SharpRuntime::Storage::StoragePaths::GetIsolatedStorageRoot(),
                                    IsolatedStorageScope::Application | IsolatedStorageScope::User);
    }

    // --- File operations ---

    bool IsolatedStorageFile::FileExists(const std::string& relativePath) const
    {
        throwIfDisposed();
        const auto fp = fullPath(relativePath);
        return std::filesystem::exists(fp) && std::filesystem::is_regular_file(fp);
    }

    IsolatedStorageFileStream IsolatedStorageFile::OpenFile(
        const std::string& relativePath,
        System::IO::FileMode mode) const
    {
        throwIfDisposed();
        return IsolatedStorageFileStream(fullPath(relativePath), mode);
    }

    void IsolatedStorageFile::DeleteFile(const std::string& relativePath) const
    {
        throwIfDisposed();
        std::error_code ec;
        std::filesystem::remove(fullPath(relativePath), ec);
        if (ec)
            throw IsolatedStorageException("Failed to delete isolated storage file: " + relativePath);
    }

    // --- Store lifecycle ---

    void IsolatedStorageFile::Remove()
    {
        std::error_code ec;
        std::filesystem::remove_all(rootDirectory_, ec);
        disposed_ = true;
    }
}

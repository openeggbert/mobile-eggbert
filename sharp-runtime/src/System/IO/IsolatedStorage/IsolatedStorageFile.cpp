// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/IO/IsolatedStorage/IsolatedStorageFile.hpp"

#include <algorithm>
#include <filesystem>
#include <numeric>

#include <limits>

#include "SharpRuntime/Storage/StoragePaths.hpp"
#include "System/ArgumentException.hpp"
#include "System/IO/FileMode.hpp"
#include "System/IO/IsolatedStorage/IsolatedStorageException.hpp"
#include "System/IO/IsolatedStorage/IsolatedStorageFileStream.hpp"
#include "System/ObjectDisposedException.hpp"

namespace System::IO::IsolatedStorage
{
    // Simple glob: only '*' wildcard supported (matches any sequence of chars).
    static bool globMatch(const std::string& pattern, const std::string& name)
    {
        if (pattern == "*") return true;
        size_t p = 0, n = 0;
        size_t starP = std::string::npos, starN = 0;
        while (n < name.size()) {
            if (p < pattern.size() && (pattern[p] == name[n] || pattern[p] == '?')) {
                ++p; ++n;
            } else if (p < pattern.size() && pattern[p] == '*') {
                starP = p++; starN = n;
            } else if (starP != std::string::npos) {
                p = starP + 1; n = ++starN;
            } else {
                return false;
            }
        }
        while (p < pattern.size() && pattern[p] == '*') ++p;
        return p == pattern.size();
    }

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

    IsolatedStorageFile IsolatedStorageFile::GetUserStoreForAssembly()
    {
        return IsolatedStorageFile(SharpRuntime::Storage::StoragePaths::GetIsolatedStorageRoot(),
                                    IsolatedStorageScope::Assembly | IsolatedStorageScope::User);
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

    IsolatedStorageFileStream IsolatedStorageFile::CreateFile(const std::string& relativePath) const
    {
        return OpenFile(relativePath, System::IO::FileMode::Create);
    }

    void IsolatedStorageFile::DeleteFile(const std::string& relativePath) const
    {
        throwIfDisposed();
        std::error_code ec;
        std::filesystem::remove(fullPath(relativePath), ec);
        if (ec)
            throw IsolatedStorageException("Failed to delete isolated storage file: " + relativePath);
    }

    void IsolatedStorageFile::CopyFile(const std::string& src, const std::string& dst) const
    {
        CopyFile(src, dst, false);
    }

    void IsolatedStorageFile::CopyFile(const std::string& src, const std::string& dst, bool overwrite) const
    {
        System::ArgumentException::ThrowIfNullOrEmpty(src, "sourceFileName");
        System::ArgumentException::ThrowIfNullOrEmpty(dst, "destinationFileName");
        throwIfDisposed();
        auto opts = overwrite
            ? std::filesystem::copy_options::overwrite_existing
            : std::filesystem::copy_options::none;
        std::error_code ec;
        std::filesystem::copy_file(fullPath(src), fullPath(dst), opts, ec);
        if (ec)
            throw IsolatedStorageException("Failed to copy isolated storage file: " + src);
    }

    void IsolatedStorageFile::MoveFile(const std::string& src, const std::string& dst) const
    {
        System::ArgumentException::ThrowIfNullOrEmpty(src, "sourceFileName");
        System::ArgumentException::ThrowIfNullOrEmpty(dst, "destinationFileName");
        throwIfDisposed();
        std::error_code ec;
        std::filesystem::rename(fullPath(src), fullPath(dst), ec);
        if (ec)
            throw IsolatedStorageException("Failed to move isolated storage file: " + src);
    }

    std::vector<std::string> IsolatedStorageFile::GetFileNames(const std::string& searchPattern) const
    {
        throwIfDisposed();
        std::vector<std::string> names;
        if (!std::filesystem::exists(rootDirectory_)) return names;
        for (const auto& entry : std::filesystem::directory_iterator(rootDirectory_)) {
            if (!entry.is_regular_file()) continue;
            std::string name = entry.path().filename().string();
            if (globMatch(searchPattern, name))
                names.push_back(name);
        }
        std::sort(names.begin(), names.end());
        return names;
    }

    // --- Directory operations ---

    bool IsolatedStorageFile::DirectoryExists(const std::string& relativePath) const
    {
        throwIfDisposed();
        const auto fp = fullPath(relativePath);
        return std::filesystem::exists(fp) && std::filesystem::is_directory(fp);
    }

    void IsolatedStorageFile::CreateDirectory(const std::string& relativePath) const
    {
        throwIfDisposed();
        std::error_code ec;
        std::filesystem::create_directories(fullPath(relativePath), ec);
        if (ec)
            throw IsolatedStorageException("Failed to create isolated storage directory: " + relativePath);
    }

    void IsolatedStorageFile::DeleteDirectory(const std::string& relativePath) const
    {
        throwIfDisposed();
        // Verified against IsolatedStorageFile.cs's DeleteDirectory: real .NET calls
        // Directory.Delete(fullPath, recursive: false) -- non-recursive, fails on a non-empty
        // directory. This port previously used remove_all (recursive), silently deleting an
        // entire subtree instead of matching .NET's "must be empty" contract.
        std::error_code ec;
        std::filesystem::remove(fullPath(relativePath), ec);
        if (ec)
            throw IsolatedStorageException("Failed to delete isolated storage directory: " + relativePath);
    }

    void IsolatedStorageFile::MoveDirectory(const std::string& src, const std::string& dst) const
    {
        System::ArgumentException::ThrowIfNullOrEmpty(src, "sourceDirectoryName");
        System::ArgumentException::ThrowIfNullOrEmpty(dst, "destinationDirectoryName");
        throwIfDisposed();
        std::error_code ec;
        std::filesystem::rename(fullPath(src), fullPath(dst), ec);
        if (ec)
            throw IsolatedStorageException("Failed to move isolated storage directory: " + src);
    }

    std::vector<std::string> IsolatedStorageFile::GetDirectoryNames(const std::string& searchPattern) const
    {
        throwIfDisposed();
        std::vector<std::string> names;
        if (!std::filesystem::exists(rootDirectory_)) return names;
        for (const auto& entry : std::filesystem::directory_iterator(rootDirectory_)) {
            if (!entry.is_directory()) continue;
            std::string name = entry.path().filename().string();
            if (globMatch(searchPattern, name))
                names.push_back(name);
        }
        std::sort(names.begin(), names.end());
        return names;
    }

    // --- Store lifecycle ---

    void IsolatedStorageFile::Remove()
    {
        std::error_code ec;
        std::filesystem::remove_all(rootDirectory_, ec);
        disposed_ = true;
    }

    void IsolatedStorageFile::Close()
    {
        disposed_ = true;
    }

    void IsolatedStorageFile::Dispose()
    {
        Close();
    }

    // --- Space properties ---

    SharpRuntime::longcs IsolatedStorageFile::getAvailableFreeSpaceProperty() const
    {
        std::error_code ec;
        auto si = std::filesystem::space(rootDirectory_, ec);
        if (ec) return 0;
        return static_cast<SharpRuntime::longcs>(si.available);
    }

    SharpRuntime::longcs IsolatedStorageFile::getUsedSizeProperty() const
    {
        SharpRuntime::longcs total = 0;
        if (!std::filesystem::exists(rootDirectory_)) return total;
        for (const auto& entry : std::filesystem::recursive_directory_iterator(rootDirectory_)) {
            if (entry.is_regular_file()) {
                std::error_code ec;
                total += static_cast<SharpRuntime::longcs>(entry.file_size(ec));
            }
        }
        return total;
    }

    SharpRuntime::longcs IsolatedStorageFile::getQuotaProperty() const
    {
        // Verified against IsolatedStorageFile.cs's Quota property: real .NET does not enforce
        // quotas and always returns long.MaxValue. This port previously inherited the
        // IsolatedStorage base class's default of 0, which any caller checking "is there quota
        // remaining" against would read as "no space available" instead of "unlimited."
        return std::numeric_limits<SharpRuntime::longcs>::max();
    }

    const std::filesystem::path& IsolatedStorageFile::getRootDirectoryProperty() const
    {
        return rootDirectory_;
    }
}

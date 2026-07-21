// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/IO/FileStream.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentNullException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/NotSupportedException.hpp"
#include "System/ObjectDisposedException.hpp"
#include "System/IO/DirectoryNotFoundException.hpp"
#include "System/IO/FileNotFoundException.hpp"
#include "System/IO/IOException.hpp"

#include <filesystem>

namespace System::IO
{
    namespace {
        bool HasFlag(FileAccess access, FileAccess flag) {
            return (access & flag) == flag;
        }

        // Verified against Interop.IOErrors.cs's GetExceptionForIoErrno: real .NET throws
        // DirectoryNotFoundException (not FileNotFoundException, and not a generic IOException)
        // whenever the *parent* directory of the target path doesn't exist -- "For Windows
        // compatibility, throw DirectoryNotFoundException instead of FileNotFoundException when
        // the parent folder does not exist."
        bool ParentDirectoryExists(const std::string& path) {
            std::filesystem::path parent = std::filesystem::path(path).parent_path();
            if (parent.empty()) return true; // relative path with no directory component
            std::error_code ec;
            bool isDir = std::filesystem::is_directory(parent, ec);
            return !ec && isDir;
        }

        // .NET: FileStream(path, mode) defaults access to Write for Append, ReadWrite otherwise.
        FileAccess DefaultAccessFor(FileMode mode) {
            return mode == FileMode::Append ? FileAccess::Write : FileAccess::ReadWrite;
        }

        void ValidateModeAndAccess(FileMode mode, FileAccess access) {
            if (HasFlag(access, FileAccess::Read) && mode == FileMode::Append) {
                throw System::ArgumentException("FileMode.Append is invalid with FileAccess.Read.", "access");
            }
            if (!HasFlag(access, FileAccess::Write)) {
                if (mode == FileMode::Truncate || mode == FileMode::CreateNew ||
                    mode == FileMode::Create   || mode == FileMode::Append) {
                    throw System::ArgumentException(
                        "Combining FileMode and FileAccess values is invalid: this FileMode requires FileAccess.Write.",
                        "access");
                }
            }
        }
    }

    FileStream::FileStream(const std::string& path)
        : FileStream(path, FileMode::Open) {}

    FileStream::FileStream(const std::string& path, FileMode mode)
        : FileStream(path, mode, DefaultAccessFor(mode)) {}

    FileStream::FileStream(const std::string& path, FileMode mode, FileAccess access)
        : path_(path), length_(0), canRead_(false), canWrite_(false)
    {
        ValidateModeAndAccess(mode, access);

        std::error_code ec;
        bool exists = std::filesystem::is_regular_file(path, ec) && !ec;

        // Existence preconditions that std::fstream's open-mode flags can't express directly.
        if (mode == FileMode::CreateNew && exists) {
            throw IOException("Cannot create '" + path + "' because a file or directory with the same name already exists.");
        }
        if ((mode == FileMode::Open || mode == FileMode::Truncate) && !exists) {
            if (!ParentDirectoryExists(path)) {
                throw DirectoryNotFoundException("Could not find a part of the path '" + path + "'.");
            }
            throw FileNotFoundException("Unable to find the specified file.", path);
        }

        bool wantRead  = HasFlag(access, FileAccess::Read);
        bool wantWrite = HasFlag(access, FileAccess::Write);

        std::ios::openmode iosMode = std::ios::binary;
        if (wantRead)  iosMode |= std::ios::in;
        if (wantWrite) iosMode |= std::ios::out;

        switch (mode) {
            case FileMode::CreateNew:
            case FileMode::Create:
                iosMode |= std::ios::out | std::ios::trunc;
                break;
            case FileMode::Truncate:
                iosMode |= std::ios::out | std::ios::trunc;
                break;
            case FileMode::Append:
                iosMode |= std::ios::out | std::ios::app;
                break;
            case FileMode::OpenOrCreate:
                if (!exists) iosMode |= std::ios::out | std::ios::trunc; // create empty; nothing to preserve
                break;
            case FileMode::Open:
            default:
                break; // existence already verified above
        }

        file_.open(path, iosMode);
        if (!file_.is_open()) {
            if (!ParentDirectoryExists(path)) {
                throw DirectoryNotFoundException("Could not find a part of the path '" + path + "'.");
            }
            throw IOException("Failed to open file '" + path + "'.");
        }

        canRead_  = wantRead;
        canWrite_ = wantWrite;

        // Query length independently of the stream's own read position/access, matching
        // .NET's FileStream.Length (available regardless of CanRead).
        std::error_code sizeEc;
        auto size = std::filesystem::file_size(path, sizeEc);
        length_ = sizeEc ? 0 : static_cast<intcs>(size);
    }

    FileStream::~FileStream() { Close(); }

    intcs FileStream::Read(bytecs buffer[], intcs offset, intcs count)
    {
        if (!file_.is_open())
            throw System::ObjectDisposedException("Cannot access a closed file.");
        if (buffer == nullptr)
            throw System::ArgumentNullException("buffer");
        if (offset < 0)
            throw System::ArgumentOutOfRangeException("offset", "Non-negative number required.");
        if (count < 0)
            throw System::ArgumentOutOfRangeException("count", "Non-negative number required.");
        if (count == 0) return 0;
        file_.read(reinterpret_cast<char*>(buffer + offset), count);
        return static_cast<intcs>(file_.gcount());
    }

    void FileStream::Write(const bytecs buffer[], intcs offset, intcs count)
    {
        if (!file_.is_open())
            throw System::ObjectDisposedException("Cannot access a closed file.");
        if (buffer == nullptr)
            throw System::ArgumentNullException("buffer");
        if (offset < 0)
            throw System::ArgumentOutOfRangeException("offset", "Non-negative number required.");
        if (count < 0)
            throw System::ArgumentOutOfRangeException("count", "Non-negative number required.");
        if (count == 0) return;
        file_.write(reinterpret_cast<const char*>(buffer + offset),
                    static_cast<std::streamsize>(count));
    }

    void FileStream::WriteByte(bytecs value)
    {
        file_.put(static_cast<char>(value));
    }

    void FileStream::Flush()
    {
        if (file_.is_open()) file_.flush();
    }

    void FileStream::Close()
    {
        if (file_.is_open()) file_.close();
    }

    intcs FileStream::getLengthProperty() const {
        // Verified against FileStream.cs's Length getter: real .NET queries the underlying
        // file's current length live (accounting for any writes made so far through this
        // stream), not a value cached once and only ever updated by SetLength(). This port
        // previously only updated length_ from SetLength(), so a Write() that extended the
        // file left getLengthProperty() returning the stale construction-time length (often 0
        // for a freshly created file) until the stream was closed and reopened.
        if (file_.is_open() && canWrite_) {
            auto& f = const_cast<std::fstream&>(file_);
            f.flush();
        }
        std::error_code ec;
        auto size = std::filesystem::file_size(path_, ec);
        return ec ? length_ : static_cast<intcs>(size);
    }

    bool FileStream::IsOpen() const { return file_.is_open(); }

    intcs FileStream::getPositionProperty() const
    {
        auto& f = const_cast<std::fstream&>(file_);
        std::streampos pos = canRead_ ? f.tellg() : f.tellp();
        return static_cast<intcs>(pos);
    }

    void FileStream::setPositionProperty(intcs value)
    {
        if (value < 0)
            throw System::ArgumentOutOfRangeException("value", "Non-negative number required.");
        file_.clear();
        if (canRead_)  file_.seekg(static_cast<std::streamoff>(value));
        if (canWrite_) file_.seekp(static_cast<std::streamoff>(value));
    }

    void FileStream::SetLength(intcs value)
    {
        if (value < 0)
            throw System::ArgumentOutOfRangeException("value", "Non-negative number required.");
        // Check open state before canWrite_: canWrite_ reflects the access mode requested at
        // construction and is never reset by Close(), so without this check SetLength() after
        // Close() would still resize the file on disk via path_ below despite the stream
        // claiming to be closed.
        if (!file_.is_open())
            throw System::ObjectDisposedException("Cannot access a closed file.");
        if (!canWrite_)
            throw System::NotSupportedException("Stream does not support writing.");

        file_.flush();
        std::error_code ec;
        std::filesystem::resize_file(path_, static_cast<std::uintmax_t>(value), ec);
        if (ec) {
            throw IOException("Unable to set the length of file '" + path_ + "'.");
        }

        length_ = value;
        if (getPositionProperty() > value) setPositionProperty(value);
    }
}

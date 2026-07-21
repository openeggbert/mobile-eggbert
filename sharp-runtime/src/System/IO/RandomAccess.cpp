// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/IO/RandomAccess.hpp"
#include "System/IO/IOException.hpp"

#if defined(_WIN32)
#  include <windows.h>
#  include <io.h>       // _get_osfhandle, _chsize_s
namespace {
    // CRT fd → Win32 HANDLE
    inline HANDLE hOf(int fd) { return reinterpret_cast<HANDLE>(_get_osfhandle(fd)); }
}
#elif defined(__EMSCRIPTEN__)
#  include "System/PlatformNotSupportedException.hpp"
#else
#  include <unistd.h>   // pread, pwrite, lseek, ftruncate, fsync
#endif

namespace System::IO {

int64_t RandomAccess::GetLength(int fd) {
#if defined(__EMSCRIPTEN__)
    (void)fd;
    throw System::PlatformNotSupportedException("RandomAccess is not supported on Emscripten.");
#elif defined(_WIN32)
    LARGE_INTEGER sz{};
    if (!GetFileSizeEx(hOf(fd), &sz))
        throw IOException("RandomAccess::GetLength failed");
    return static_cast<int64_t>(sz.QuadPart);
#else
    off_t pos = lseek(fd, 0, SEEK_CUR);
    off_t len = lseek(fd, 0, SEEK_END);
    lseek(fd, pos, SEEK_SET);
    return static_cast<int64_t>(len);
#endif
}

void RandomAccess::SetLength(int fd, int64_t length) {
#if defined(__EMSCRIPTEN__)
    (void)fd; (void)length;
    throw System::PlatformNotSupportedException("RandomAccess is not supported on Emscripten.");
#elif defined(_WIN32)
    LARGE_INTEGER li{};
    li.QuadPart = length;
    if (!SetFilePointerEx(hOf(fd), li, nullptr, FILE_BEGIN) || !SetEndOfFile(hOf(fd)))
        throw IOException("RandomAccess::SetLength failed");
#else
    if (ftruncate(fd, static_cast<off_t>(length)) != 0)
        throw IOException("RandomAccess::SetLength failed");
#endif
}

intcs RandomAccess::Read(int fd, std::vector<bytecs>& buffer, int64_t fileOffset) {
    return Read(fd, buffer.data(), static_cast<intcs>(buffer.size()), fileOffset);
}

intcs RandomAccess::Read(int fd, bytecs* buffer, intcs count, int64_t fileOffset) {
#if defined(__EMSCRIPTEN__)
    (void)fd; (void)buffer; (void)count; (void)fileOffset;
    throw System::PlatformNotSupportedException("RandomAccess is not supported on Emscripten.");
#elif defined(_WIN32)
    OVERLAPPED ov{};
    ov.Offset     = static_cast<DWORD>(fileOffset & 0xFFFFFFFFLL);
    ov.OffsetHigh = static_cast<DWORD>(fileOffset >> 32);
    DWORD nRead = 0;
    if (!ReadFile(hOf(fd), buffer, static_cast<DWORD>(count), &nRead, &ov))
        throw IOException("RandomAccess::Read failed");
    return static_cast<intcs>(nRead);
#else
    ssize_t n = pread(fd, buffer, static_cast<size_t>(count), static_cast<off_t>(fileOffset));
    if (n < 0) throw IOException("RandomAccess::Read failed");
    return static_cast<intcs>(n);
#endif
}

void RandomAccess::Write(int fd, const std::vector<bytecs>& buffer, int64_t fileOffset) {
    Write(fd, buffer.data(), static_cast<intcs>(buffer.size()), fileOffset);
}

void RandomAccess::Write(int fd, const bytecs* buffer, intcs count, int64_t fileOffset) {
#if defined(__EMSCRIPTEN__)
    (void)fd; (void)buffer; (void)count; (void)fileOffset;
    throw System::PlatformNotSupportedException("RandomAccess is not supported on Emscripten.");
#elif defined(_WIN32)
    // WriteFile can also complete a "short write"; loop the same as the POSIX branch below.
    while (count > 0) {
        OVERLAPPED ov{};
        ov.Offset     = static_cast<DWORD>(fileOffset & 0xFFFFFFFFLL);
        ov.OffsetHigh = static_cast<DWORD>(fileOffset >> 32);
        DWORD nWritten = 0;
        if (!WriteFile(hOf(fd), buffer, static_cast<DWORD>(count), &nWritten, &ov))
            throw IOException("RandomAccess::Write failed");
        buffer += nWritten;
        count -= static_cast<intcs>(nWritten);
        fileOffset += nWritten;
    }
#else
    // Verified against RandomAccess.Unix.cs's WriteAtOffset: real .NET loops "while
    // (!buffer.IsEmpty)", advancing the buffer and file offset by however many bytes were
    // actually written each call, until the whole buffer has been written. pwrite() can
    // legitimately return fewer bytes than requested (a "short write" -- e.g. after a signal
    // interruption, or writing to certain non-regular files); this previously issued a single
    // pwrite() call and silently discarded any bytes it didn't cover, with no error and no way
    // for the void-returning caller to detect the loss.
    while (count > 0) {
        ssize_t n = pwrite(fd, buffer, static_cast<size_t>(count), static_cast<off_t>(fileOffset));
        if (n < 0) throw IOException("RandomAccess::Write failed");
        buffer += n;
        count -= static_cast<intcs>(n);
        fileOffset += n;
    }
#endif
}

void RandomAccess::FlushToDisk(int fd) {
#if defined(__EMSCRIPTEN__)
    (void)fd;
    throw System::PlatformNotSupportedException("RandomAccess is not supported on Emscripten.");
#elif defined(_WIN32)
    if (!FlushFileBuffers(hOf(fd)))
        throw IOException("RandomAccess::FlushToDisk failed");
#else
    if (fsync(fd) != 0) throw IOException("RandomAccess::FlushToDisk failed");
#endif
}

} // namespace System::IO

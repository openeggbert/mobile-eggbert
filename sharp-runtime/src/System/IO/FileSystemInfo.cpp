// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/IO/FileSystemInfo.hpp"
#include "System/IO/IOException.hpp"
#include "System/TimeZoneInfo.hpp"

#include <chrono>

#ifdef _WIN32
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <windows.h>
#  include <sys/stat.h>
#else
#  include <sys/stat.h>
#endif

namespace System::IO {

    using SharpRuntime::longcs;

    namespace {

        System::DateTime fromUnixTime(std::time_t seconds) {
            longcs ticks = System::DateTime::UnixEpochTicks + static_cast<longcs>(seconds) * System::DateTime::TicksPerSecond;
            return System::DateTime(ticks);
        }

        System::DateTime fromFileClock(std::filesystem::file_time_type ft) {
            // file_clock::to_sys() may return a finer-grained duration than
            // system_clock::time_point (e.g. nanoseconds on libc++/Emscripten vs.
            // microseconds on libstdc++); to_time_t() only accepts the exact
            // system_clock::time_point type, so cast explicitly.
            auto sysTime = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                std::chrono::file_clock::to_sys(ft));
            return fromUnixTime(std::chrono::system_clock::to_time_t(sysTime));
        }

        struct RawTimes {
            std::time_t creation;
            std::time_t access;
        };

        RawTimes statTimes(const std::filesystem::path& path) {
#ifdef _WIN32
            struct _stat64 st{};
            if (_wstat64(path.wstring().c_str(), &st) != 0)
                throw IOException("Failed to stat '" + path.string() + "'.");
            // On Windows, st_ctime from _stat64 reflects file creation time.
            return { st.st_ctime, st.st_atime };
#else
            struct stat st{};
            if (::stat(path.c_str(), &st) != 0)
                throw IOException("Failed to stat '" + path.string() + "'.");
            // POSIX has no portable birth-time field in <sys/stat.h>; st_ctime (inode
            // metadata change time) is the closest portable approximation, matching
            // real .NET's own documented Linux fallback behavior.
            return { st.st_ctime, st.st_atime };
#endif
        }

    } // namespace

    System::DateTime FileSystemInfo::getCreationTimeUtcProperty() const {
        return fromUnixTime(statTimes(fullPath_).creation);
    }

    // Verified against FileSystemInfo.cs: CreationTime/LastAccessTime/LastWriteTime are all
    // `Xxx => XxxUtc.ToLocalTime()` (and the LastWriteTime setter is `XxxUtc = value.
    // ToUniversalTime()`). This port's System::DateTime deliberately doesn't track
    // DateTimeKind and has no ToLocalTime()/ToUniversalTime() (a documented, separate
    // limitation -- see DateTime.hpp's doc comment), so these previously returned/consumed the
    // UTC value verbatim with no conversion at all -- silently wrong by the local UTC offset
    // whenever it's non-zero. Routed through the already-existing
    // TimeZoneInfo::ConvertTimeFromUtc/ConvertTimeToUtc using TimeZoneInfo::Local() instead.
    System::DateTime FileSystemInfo::getCreationTimeProperty() const {
        return System::TimeZoneInfo::ConvertTimeFromUtc(getCreationTimeUtcProperty(), System::TimeZoneInfo::Local());
    }

    System::DateTime FileSystemInfo::getLastAccessTimeUtcProperty() const {
        return fromUnixTime(statTimes(fullPath_).access);
    }

    System::DateTime FileSystemInfo::getLastAccessTimeProperty() const {
        return System::TimeZoneInfo::ConvertTimeFromUtc(getLastAccessTimeUtcProperty(), System::TimeZoneInfo::Local());
    }

    System::DateTime FileSystemInfo::getLastWriteTimeUtcProperty() const {
        std::error_code ec;
        auto ft = std::filesystem::last_write_time(fullPath_, ec);
        if (ec) throw IOException("Failed to get last write time of '" + fullPath_.string() + "': " + ec.message());
        return fromFileClock(ft);
    }

    System::DateTime FileSystemInfo::getLastWriteTimeProperty() const {
        return System::TimeZoneInfo::ConvertTimeFromUtc(getLastWriteTimeUtcProperty(), System::TimeZoneInfo::Local());
    }

    void FileSystemInfo::setLastWriteTimeUtcProperty(const System::DateTime& value) {
        longcs unixSeconds = (value.getTicksProperty() - System::DateTime::UnixEpochTicks) / System::DateTime::TicksPerSecond;
        auto sysTime = std::chrono::system_clock::from_time_t(static_cast<std::time_t>(unixSeconds));
        auto fileTime = std::chrono::file_clock::from_sys(sysTime);
        std::error_code ec;
        std::filesystem::last_write_time(fullPath_, fileTime, ec);
        if (ec) throw IOException("Failed to set last write time of '" + fullPath_.string() + "': " + ec.message());
    }

    void FileSystemInfo::setLastWriteTimeProperty(const System::DateTime& value) {
        setLastWriteTimeUtcProperty(System::TimeZoneInfo::ConvertTimeToUtc(value, System::TimeZoneInfo::Local()));
    }

} // namespace System::IO

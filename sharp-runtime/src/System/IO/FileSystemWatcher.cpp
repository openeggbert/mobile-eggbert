// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/IO/FileSystemWatcher.hpp"
#include "System/ArgumentException.hpp"
#include "System/IO/Directory.hpp"
#include "System/IO/IOException.hpp"
#include "System/PlatformNotSupportedException.hpp"

#if defined(__linux__)
#define SHARP_RUNTIME_FSW_LINUX 1
#include <cerrno>
#include <climits>
#include <cstdint>
#include <cstring>
#include <poll.h>
#include <regex>
#include <sys/eventfd.h>
#include <sys/inotify.h>
#include <system_error>
#include <unistd.h>
#include <unordered_map>
#endif

namespace System::IO {

    void FileSystemWatcher::CheckPathValidity(const std::string& path) {
        if (path.empty()) {
            throw System::ArgumentException("Empty path.", "path");
        }
        if (!Directory::Exists(path)) {
            throw System::ArgumentException("The directory name " + path + " does not exist.", "path");
        }
    }

    FileSystemWatcher::FileSystemWatcher(const std::string& path) {
        CheckPathValidity(path);
        directory_ = path;
    }

    FileSystemWatcher::FileSystemWatcher(const std::string& path, const std::string& filter) {
        CheckPathValidity(path);
        directory_ = path;
        setFilterProperty(filter);
    }

    FileSystemWatcher::~FileSystemWatcher() {
        stopWatchingIfRunning();
    }

    void FileSystemWatcher::setPathProperty(const std::string& value) {
        if (directory_ == value) return;
        if (value.empty()) {
            throw System::ArgumentException("Empty path.", "Path");
        }
        if (!Directory::Exists(value)) {
            throw System::ArgumentException("The directory name " + value + " does not exist.", "Path");
        }
        directory_ = value;
    }

    void FileSystemWatcher::setNotifyFilterProperty(NotifyFilters value) {
        if ((static_cast<int>(value) & ~ValidNotifyFiltersMask) != 0) {
            throw System::ArgumentException("The value of the NotifyFilter property is invalid.", "value");
        }
        notifyFilter_ = value;
    }

    void FileSystemWatcher::setEnableRaisingEventsProperty(bool value) {
        if (value == enabled_) return;
        enabled_ = value;
        if (value) startWatchingIfPossible();
        else stopWatchingIfRunning();
    }

#if defined(SHARP_RUNTIME_FSW_LINUX)

    namespace {
        // Mirrors Directory::GetFiles's glob-to-regex translation (including the "*.*" DOS-legacy
        // special case) so FileSystemWatcher's Filters behave the same way a caller would already
        // expect from GetFiles with the same pattern.
        bool matchesAnyFilter(const std::vector<std::string>& filters, const std::string& name) {
            if (filters.empty()) return true;
            for (const auto& rawPattern : filters) {
                std::string pattern = (rawPattern == "*.*") ? "*" : rawPattern;
                std::string regexPattern;
                for (char c : pattern) {
                    if (c == '*') regexPattern += ".*";
                    else if (c == '?') regexPattern += ".";
                    else if (std::string(".+^${}[]|()\\").find(c) != std::string::npos)
                        regexPattern += std::string("\\") + c;
                    else regexPattern += c;
                }
                std::regex rx(regexPattern, std::regex_constants::icase);
                if (std::regex_match(name, rx)) return true;
            }
            return false;
        }
    } // namespace

    void FileSystemWatcher::startWatchingIfPossible() {
        // No directory configured yet: preserve the original stub behavior (track the flag,
        // watch nothing) rather than throwing -- matches ported C# code that sets
        // EnableRaisingEvents before Path in some order real .NET itself does not forbid either.
        if (directory_.empty()) return;
        if (watchThread_.joinable()) return;

        inotifyFd_ = inotify_init1(IN_CLOEXEC);
        if (inotifyFd_ < 0) {
            enabled_ = false;
            if (!Error.empty()) {
                auto ex = std::make_exception_ptr(
                    System::IO::IOException("Unable to initialize inotify: " + std::string(std::strerror(errno))));
                System::IO::ErrorEventArgs args(ex);
                for (auto& handler : Error) handler(this, args);
            }
            return;
        }

        constexpr uint32_t mask = IN_CREATE | IN_DELETE | IN_MODIFY | IN_ATTRIB | IN_MOVED_FROM | IN_MOVED_TO;
        watchDescriptor_ = inotify_add_watch(inotifyFd_, directory_.c_str(), mask);
        if (watchDescriptor_ < 0) {
            ::close(inotifyFd_);
            inotifyFd_ = -1;
            enabled_ = false;
            if (!Error.empty()) {
                auto ex = std::make_exception_ptr(System::IO::IOException(
                    "Unable to watch directory '" + directory_ + "': " + std::string(std::strerror(errno))));
                System::IO::ErrorEventArgs args(ex);
                for (auto& handler : Error) handler(this, args);
            }
            return;
        }

        stopEventFd_ = eventfd(0, EFD_CLOEXEC);
        if (stopEventFd_ < 0) {
            inotify_rm_watch(inotifyFd_, watchDescriptor_);
            ::close(inotifyFd_);
            inotifyFd_ = -1;
            watchDescriptor_ = -1;
            enabled_ = false;
            return;
        }

        try {
            watchThread_ = std::thread(&FileSystemWatcher::watchLoop, this);
        } catch (const std::system_error&) {
            // std::thread's constructor can throw (e.g. thread/process resource exhaustion).
            // stopWatchingIfRunning()'s cleanup is gated on watchThread_.joinable(), which never
            // becomes true on this path -- without this catch, inotifyFd_/watchDescriptor_/
            // stopEventFd_ would all leak for the lifetime of this object. Mirrors the
            // eventfd-failure cleanup just above, plus firing Error for consistency with the
            // inotify_init1/inotify_add_watch failure paths above that.
            ::close(stopEventFd_);
            inotify_rm_watch(inotifyFd_, watchDescriptor_);
            ::close(inotifyFd_);
            inotifyFd_ = -1;
            watchDescriptor_ = -1;
            stopEventFd_ = -1;
            enabled_ = false;
            if (!Error.empty()) {
                auto ex = std::make_exception_ptr(
                    System::IO::IOException("Unable to start the file system watcher thread."));
                System::IO::ErrorEventArgs args(ex);
                for (auto& handler : Error) handler(this, args);
            }
        }
    }

    void FileSystemWatcher::stopWatchingIfRunning() {
        if (!watchThread_.joinable()) return;

        if (stopEventFd_ >= 0) {
            uint64_t one = 1;
            // Best-effort wakeup: watchLoop's poll() is blocked waiting on this fd (or on the
            // inotify fd) with no timeout, so writing to it is the only way to unblock the
            // thread promptly without ever detaching it -- eliminating the dangling-`this`
            // hazard this project has documented (but left unfixed) for other background-thread
            // callback types elsewhere (e.g. Socket, ClientWebSocket, Timer).
            ssize_t written = ::write(stopEventFd_, &one, sizeof(one));
            (void)written;
        }
        watchThread_.join();

        if (watchDescriptor_ >= 0 && inotifyFd_ >= 0) inotify_rm_watch(inotifyFd_, watchDescriptor_);
        if (inotifyFd_ >= 0) ::close(inotifyFd_);
        if (stopEventFd_ >= 0) ::close(stopEventFd_);
        inotifyFd_ = watchDescriptor_ = stopEventFd_ = -1;
    }

    void FileSystemWatcher::watchLoop() {
        constexpr size_t kEventBufSize = 16 * (sizeof(struct inotify_event) + NAME_MAX + 1);
        std::vector<char> buf(kEventBufSize);

        struct pollfd fds[2];
        fds[0].fd = inotifyFd_;   fds[0].events = POLLIN; fds[0].revents = 0;
        fds[1].fd = stopEventFd_; fds[1].events = POLLIN; fds[1].revents = 0;

        for (;;) {
            int pollResult = poll(fds, 2, -1);
            if (pollResult < 0) {
                if (errno == EINTR) continue;
                return;
            }
            if (fds[1].revents & POLLIN) return; // stop requested

            if (!(fds[0].revents & POLLIN)) continue;

            ssize_t len = read(inotifyFd_, buf.data(), buf.size());
            if (len <= 0) {
                if (len < 0 && errno == EINTR) continue;
                return;
            }

            // Tracks an in-flight IN_MOVED_FROM waiting for its paired IN_MOVED_TO (same cookie),
            // so a same-directory rename is reported as one Renamed event rather than a
            // Deleted+Created pair -- matching real .NET's rename semantics. Scoped per read()
            // batch: a paired rename's two halves are always delivered in the same batch, so
            // anything left unpaired at the end of this batch genuinely moved out of the watched
            // directory and is reported as Deleted, matching what the caller would observe.
            std::unordered_map<uint32_t, std::string> pendingMovedFrom;

            size_t i = 0;
            while (i + sizeof(struct inotify_event) <= static_cast<size_t>(len)) {
                auto* ev = reinterpret_cast<struct inotify_event*>(buf.data() + i);
                std::string name = ev->len > 0 ? std::string(ev->name) : std::string();

                if (name.empty() || matchesAnyFilter(filters_, name)) {
                    if (ev->mask & IN_MOVED_FROM) {
                        pendingMovedFrom[ev->cookie] = name;
                    } else if (ev->mask & IN_MOVED_TO) {
                        auto it = pendingMovedFrom.find(ev->cookie);
                        if (it != pendingMovedFrom.end()) {
                            RenamedEventArgs args(WatcherChangeTypes::Renamed, directory_, name, it->second);
                            for (auto& handler : Renamed) handler(this, args);
                            pendingMovedFrom.erase(it);
                        } else {
                            FileSystemEventArgs args(WatcherChangeTypes::Created, directory_, name);
                            for (auto& handler : Created) handler(this, args);
                        }
                    } else if (ev->mask & IN_CREATE) {
                        FileSystemEventArgs args(WatcherChangeTypes::Created, directory_, name);
                        for (auto& handler : Created) handler(this, args);
                    } else if (ev->mask & IN_DELETE) {
                        FileSystemEventArgs args(WatcherChangeTypes::Deleted, directory_, name);
                        for (auto& handler : Deleted) handler(this, args);
                    } else if (ev->mask & (IN_MODIFY | IN_ATTRIB)) {
                        FileSystemEventArgs args(WatcherChangeTypes::Changed, directory_, name);
                        for (auto& handler : Changed) handler(this, args);
                    }
                }

                i += sizeof(struct inotify_event) + ev->len;
            }

            for (const auto& [cookie, name] : pendingMovedFrom) {
                (void)cookie;
                FileSystemEventArgs args(WatcherChangeTypes::Deleted, directory_, name);
                for (auto& handler : Deleted) handler(this, args);
            }
        }
    }

#else // !SHARP_RUNTIME_FSW_LINUX

    // No OS-level watch backend implemented for this platform (see the class doc-comment).
    // Matches CLAUDE.md's platform-abstraction rule: on an unsupported platform, throw
    // System::PlatformNotSupportedException rather than silently degrading to a no-op -- a
    // silently-inert watcher that never fires a single event is a worse failure mode than a
    // loud, immediate exception at the point EnableRaisingEvents is actually turned on.
    void FileSystemWatcher::startWatchingIfPossible() {
        throw System::PlatformNotSupportedException(
            "FileSystemWatcher requires Linux inotify support; no watch backend is implemented "
            "for this platform.");
    }
    void FileSystemWatcher::stopWatchingIfRunning() {}
    void FileSystemWatcher::watchLoop() {}

#endif

} // namespace System::IO

// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include <thread>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/IO/ErrorEventHandler.hpp"
#include "System/IO/FileSystemEventHandler.hpp"
#include "System/IO/NotifyFilters.hpp"
#include "System/IO/RenamedEventHandler.hpp"

namespace System::IO {

    using SharpRuntime::intcs;

    /**
     * @brief Listens to the system directory change notifications and raises events when a
     * directory or file within a directory changes.
     *
     * C++ counterpart of .NET System.IO.FileSystemWatcher.
     *
     * @note Status: PARTIAL. On Linux, enabling the watcher (EnableRaisingEvents = true) starts
     * a real background thread backed by inotify that observes Created/Deleted/Changed/Renamed
     * events in the watched directory and invokes the registered handler vectors -- matching
     * real .NET's semantics for the common single-directory, non-recursive case. Deliberately
     * NOT implemented, matching this project's "document, don't rush" precedent for large
     * feature gaps: recursive subdirectory watching (IncludeSubdirectories is tracked but has
     * no effect -- would require walking the whole tree, adding a watch per subdirectory, and
     * handling directories created/removed while already watching), and any non-Linux backend
     * (FSEvents on macOS, ReadDirectoryChangesW on Windows) -- on those platforms, setting
     * EnableRaisingEvents = true throws System::PlatformNotSupportedException rather than
     * silently tracking the flag with no real monitoring, matching CLAUDE.md's
     * platform-abstraction policy (never silently fail on an unsupported platform). This is a
     * documented gap, not a silent one.
     */
    class FileSystemWatcher {
        std::string directory_;
        std::vector<std::string> filters_;
        NotifyFilters notifyFilter_ = NotifyFilters::LastWrite | NotifyFilters::FileName | NotifyFilters::DirectoryName;
        bool includeSubdirectories_ = false;
        bool enabled_ = false;
        unsigned int internalBufferSize_ = 8192;

        // Backing state for the real inotify-based watch thread (Linux only; unused elsewhere).
        // A FileSystemWatcher with a live watch thread can never be safely copied or moved (the
        // thread captures `this` for the lifetime of the watch), so copy/move are deleted below
        // rather than leaving a dangling-`this` hazard for a caller to discover the hard way.
        // Guarded to match FileSystemWatcher.cpp's own SHARP_RUNTIME_FSW_LINUX condition exactly --
        // on non-Linux targets (e.g. Emscripten, where startWatchingIfPossible() always throws)
        // these fields are never touched, and declaring them unconditionally left them dead code
        // there, tripping -Werror=unused-private-field.
#if defined(__linux__)
        int inotifyFd_ = -1;
        int watchDescriptor_ = -1;
        int stopEventFd_ = -1;
#endif
        std::thread watchThread_;

        void startWatchingIfPossible();
        void stopWatchingIfRunning();
        void watchLoop();

        static constexpr int ValidNotifyFiltersMask =
            static_cast<int>(NotifyFilters::Attributes)    | static_cast<int>(NotifyFilters::CreationTime) |
            static_cast<int>(NotifyFilters::DirectoryName) | static_cast<int>(NotifyFilters::FileName)     |
            static_cast<int>(NotifyFilters::LastAccess)    | static_cast<int>(NotifyFilters::LastWrite)    |
            static_cast<int>(NotifyFilters::Security)      | static_cast<int>(NotifyFilters::Size);

        static void CheckPathValidity(const std::string& path);

    public:
        /** Handler lists for each event; subscribers append via push_back, matching .NET's multicast delegate semantics. */
        std::vector<FileSystemEventHandler> Changed;
        std::vector<FileSystemEventHandler> Created;
        std::vector<FileSystemEventHandler> Deleted;
        std::vector<RenamedEventHandler>    Renamed;
        std::vector<ErrorEventHandler>      Error;

        /** Initializes a new instance of the FileSystemWatcher class with no directory set. */
        FileSystemWatcher() = default;

        /**
         * @brief Initializes a new instance of the FileSystemWatcher class, given the specified
         * directory to monitor.
         * @throws System::ArgumentException if @p path is empty or is not an existing directory.
         */
        explicit FileSystemWatcher(const std::string& path);

        /**
         * @brief Initializes a new instance of the FileSystemWatcher class, given the specified
         * directory and filter.
         * @throws System::ArgumentException if @p path is empty or is not an existing directory.
         */
        FileSystemWatcher(const std::string& path, const std::string& filter);

        FileSystemWatcher(const FileSystemWatcher&) = delete;
        FileSystemWatcher& operator=(const FileSystemWatcher&) = delete;
        FileSystemWatcher(FileSystemWatcher&&) = delete;
        FileSystemWatcher& operator=(FileSystemWatcher&&) = delete;

        /** Stops any active watch thread (joining it) before the watcher is destroyed. */
        ~FileSystemWatcher();

        /**
         * @brief Gets or sets the path of the directory to watch.
         * @throws System::ArgumentException if @p value is empty or is not an existing directory.
         */
        [[nodiscard]] const std::string& getPathProperty() const { return directory_; }
        void setPathProperty(const std::string& value);

        /** @brief Gets or sets the filter string used to determine which files are monitored (first entry of Filters, or "*" if empty). */
        [[nodiscard]] std::string getFilterProperty() const { return filters_.empty() ? "*" : filters_.front(); }
        void setFilterProperty(const std::string& value) {
            filters_.clear();
            filters_.push_back(value);
        }

        /** @brief Gets the collection of all filter strings used to determine which files are monitored. */
        [[nodiscard]] std::vector<std::string>& getFiltersProperty() { return filters_; }
        [[nodiscard]] const std::vector<std::string>& getFiltersProperty() const { return filters_; }

        /**
         * @brief Gets or sets the type of changes to watch for.
         * @throws System::ArgumentException if @p value contains bits outside the defined NotifyFilters values.
         */
        [[nodiscard]] NotifyFilters getNotifyFilterProperty() const { return notifyFilter_; }
        void setNotifyFilterProperty(NotifyFilters value);

        /** @brief Gets or sets whether subdirectories within the specified path should be monitored. */
        [[nodiscard]] bool getIncludeSubdirectoriesProperty() const { return includeSubdirectories_; }
        void setIncludeSubdirectoriesProperty(bool value) { includeSubdirectories_ = value; }

        /** @brief Gets or sets the size of the internal buffer, in bytes. Values below 4096 are clamped to 4096, matching .NET. */
        [[nodiscard]] intcs getInternalBufferSizeProperty() const { return static_cast<intcs>(internalBufferSize_); }
        void setInternalBufferSizeProperty(intcs value) {
            internalBufferSize_ = value < 4096 ? 4096u : static_cast<unsigned int>(value);
        }

        /**
         * @brief Gets or sets whether the component is enabled.
         *
         * On Linux, setting this to true starts a real inotify-backed watch thread for the
         * configured directory (see the class doc-comment for what is and isn't covered).
         * On other platforms this remains a documented stub: the flag is tracked faithfully so
         * ported C# code reading EnableRaisingEvents back gets the value it set, but no real
         * monitoring occurs.
         */
        [[nodiscard]] bool getEnableRaisingEventsProperty() const { return enabled_; }
        void setEnableRaisingEventsProperty(bool value);
    };

} // namespace System::IO

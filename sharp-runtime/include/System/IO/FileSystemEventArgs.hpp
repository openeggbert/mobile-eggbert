// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/EventArgs.hpp"
#include "System/IO/Path.hpp"
#include "System/IO/WatcherChangeTypes.hpp"

namespace System::IO {

    /**
     * @brief Provides data for the directory events: FileSystemWatcher.Changed, Created, Deleted.
     *
     * C++ counterpart of .NET System.IO.FileSystemEventArgs.
     */
    class FileSystemEventArgs : public System::EventArgs {
        WatcherChangeTypes changeType_;
        std::string name_;
        std::string fullPath_;

    protected:
        /**
         * @brief Combines a directory path and a relative file name into a single path.
         *
         * C++ counterpart of .NET FileSystemEventArgs.Combine(string, string).
         * Like Path::Combine, but without argument validation, and a separator is always
         * used even if @p name is empty. Protected so RenamedEventArgs can reuse it for
         * OldFullPath, matching .NET's internal visibility.
         */
        static std::string Combine(const std::string& directory, const std::string& name) {
            bool hasSeparator = !directory.empty() && directory.back() == Path::DirectorySeparatorChar;
            return hasSeparator ? directory + name
                                 : directory + Path::DirectorySeparatorChar + name;
        }

    public:
        /**
         * @brief Initializes a new instance of the FileSystemEventArgs class.
         * @param changeType The type of change (Created, Deleted, Changed, or Renamed).
         * @param directory  The directory containing the affected file or directory.
         * @param name       The name of the affected file or directory, relative to @p directory.
         */
        FileSystemEventArgs(WatcherChangeTypes changeType, const std::string& directory, const std::string& name)
            : changeType_(changeType), name_(name), fullPath_(Combine(directory, name)) {}

        /**
         * @brief Gets one of the WatcherChangeTypes values.
         *
         * C++ counterpart of .NET FileSystemEventArgs.ChangeType.
         */
        [[nodiscard]] WatcherChangeTypes getChangeTypeProperty() const { return changeType_; }

        /**
         * @brief Gets the fully qualified path of the affected file or directory.
         *
         * C++ counterpart of .NET FileSystemEventArgs.FullPath.
         */
        [[nodiscard]] const std::string& getFullPathProperty() const { return fullPath_; }

        /**
         * @brief Gets the name of the affected file or directory.
         *
         * C++ counterpart of .NET FileSystemEventArgs.Name.
         */
        [[nodiscard]] const std::string& getNameProperty() const { return name_; }
    };

} // namespace System::IO

// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/IO/FileSystemEventArgs.hpp"

namespace System::IO {

    /**
     * @brief Provides data for the FileSystemWatcher.Renamed event.
     *
     * C++ counterpart of .NET System.IO.RenamedEventArgs.
     */
    class RenamedEventArgs : public FileSystemEventArgs {
        std::string oldName_;
        std::string oldFullPath_;

    public:
        /**
         * @brief Initializes a new instance of the RenamedEventArgs class.
         * @param changeType The type of change (typically WatcherChangeTypes::Renamed).
         * @param directory  The directory containing the affected file or directory.
         * @param name       The new name of the affected file or directory.
         * @param oldName    The previous name of the affected file or directory.
         */
        RenamedEventArgs(WatcherChangeTypes changeType, const std::string& directory,
                          const std::string& name, const std::string& oldName)
            : FileSystemEventArgs(changeType, directory, name),
              oldName_(oldName), oldFullPath_(Combine(directory, oldName)) {}

        /**
         * @brief Gets the previous fully qualified path of the affected file or directory.
         *
         * C++ counterpart of .NET RenamedEventArgs.OldFullPath.
         */
        [[nodiscard]] const std::string& getOldFullPathProperty() const { return oldFullPath_; }

        /**
         * @brief Gets the previous name of the affected file or directory.
         *
         * C++ counterpart of .NET RenamedEventArgs.OldName.
         */
        [[nodiscard]] const std::string& getOldNameProperty() const { return oldName_; }
    };

} // namespace System::IO

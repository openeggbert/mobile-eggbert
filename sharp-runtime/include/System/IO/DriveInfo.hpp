// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include <vector>
#include "System/IO/Directory.hpp"

namespace System::IO {

    /** Specifies the type of a drive. */
    enum class DriveType {
        /** The type of drive is not known. */
        Unknown         = 0,
        /** The drive does not have a root directory. */
        NoRootDirectory = 1,
        /** The drive is a removable storage device such as a USB flash drive. */
        Removable       = 2,
        /** The drive is a fixed disk. */
        Fixed           = 3,
        /** The drive is a network drive. */
        Network         = 4,
        /** The drive is an optical disc device. */
        CDRom           = 5,
        /** The drive is a RAM disk. */
        Ram             = 6,
    };

    /** Provides access to information on a drive. */
    class DriveInfo {
        std::string name_;

    public:
        /** Constructs a DriveInfo for the specified drive name. */
        explicit DriveInfo(const std::string& driveName) : name_(driveName) {}

        /** Returns the name of the drive. */
        [[nodiscard]] const std::string& getNameProperty() const { return name_; }
        /** Returns true if the drive is ready (root directory exists). */
        [[nodiscard]] bool               getIsReadyProperty()    const { return Directory::Exists(name_); }
        /** Returns the drive type (always Fixed in this implementation). */
        [[nodiscard]] DriveType          getDriveTypeProperty()  const { return DriveType::Fixed; }
        /** Returns the amount of available free space in bytes (0 if the drive is not ready or space info is unavailable). */
        [[nodiscard]] long long          getAvailableFreeSpaceProperty()  const;
        /** Returns the total amount of free space in bytes (0 if the drive is not ready or space info is unavailable). */
        [[nodiscard]] long long          getTotalFreeSpaceProperty()      const;
        /** Returns the total size of the drive in bytes (0 if the drive is not ready or space info is unavailable). */
        [[nodiscard]] long long          getTotalSizeProperty()           const;
        /** Returns the volume label of the drive. */
        [[nodiscard]] std::string        getVolumeLabelProperty()         const { return name_; }
        /** Returns the name of the file system, such as NTFS or FAT32 (stub returns "Unknown"). */
        [[nodiscard]] std::string        getDriveFormatProperty()         const { return "Unknown"; }

        /** Returns the drive name as a string. */
        [[nodiscard]] std::string ToString() const { return name_; }

        /** Returns a collection of DriveInfo objects for all logical drives on the computer. */
        static std::vector<DriveInfo> GetDrives();
    };

} // namespace System::IO

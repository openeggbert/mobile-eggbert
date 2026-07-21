// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/IO/DriveInfo.hpp"

#include <filesystem>

#if defined(_WIN32)
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <windows.h>
#endif

namespace System::IO {

long long DriveInfo::getAvailableFreeSpaceProperty() const {
    std::error_code ec;
    auto info = std::filesystem::space(name_, ec);
    return ec ? 0LL : static_cast<long long>(info.available);
}

long long DriveInfo::getTotalFreeSpaceProperty() const {
    std::error_code ec;
    auto info = std::filesystem::space(name_, ec);
    return ec ? 0LL : static_cast<long long>(info.free);
}

long long DriveInfo::getTotalSizeProperty() const {
    std::error_code ec;
    auto info = std::filesystem::space(name_, ec);
    return ec ? 0LL : static_cast<long long>(info.capacity);
}

std::vector<DriveInfo> DriveInfo::GetDrives() {
#if defined(_WIN32)
    std::vector<DriveInfo> drives;
    DWORD mask = GetLogicalDrives();
    for (int i = 0; i < 26; ++i) {
        if (mask & (1u << i)) {
            std::string name(1, char('A' + i));
            name += ":\\";
            drives.emplace_back(name);
        }
    }
    return drives;
#else
    return { DriveInfo("/") };
#endif
}

} // namespace System::IO

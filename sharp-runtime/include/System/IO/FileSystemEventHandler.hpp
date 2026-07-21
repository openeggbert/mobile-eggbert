// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <functional>
#include "System/IO/FileSystemEventArgs.hpp"

namespace System::IO {

/**
 * @brief Represents the method that will handle the FileSystemWatcher.Changed, Created,
 * or Deleted event.
 *
 * C++ counterpart of .NET System.IO.FileSystemEventHandler.
 * .NET's delegate signature is (object? sender, FileSystemEventArgs e); the sender is passed
 * as an untyped void* here since sharp-runtime has no common object base to mirror it.
 */
using FileSystemEventHandler = std::function<void(void*, const FileSystemEventArgs&)>;

} // namespace System::IO

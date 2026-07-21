// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <functional>
#include "System/IO/ErrorEventArgs.hpp"

namespace System::IO {

/**
 * @brief Represents the method that will handle the FileSystemWatcher.Error event.
 *
 * C++ counterpart of .NET System.IO.ErrorEventHandler.
 * .NET's delegate signature is (object? sender, ErrorEventArgs e); the sender is passed
 * as an untyped void* here since sharp-runtime has no common object base to mirror it.
 */
using ErrorEventHandler = std::function<void(void*, const ErrorEventArgs&)>;

} // namespace System::IO

// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/IO/Stream.hpp"
#include "System/NotSupportedException.hpp"

namespace System::IO {

    void Stream::Write(const bytecs[], intcs, intcs) {
        throw System::NotSupportedException("Stream does not support writing.");
    }

} // namespace System::IO

// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Xml::Linq {

    /**
     * @brief Specifies load options when parsing an XDocument/XElement. [Flags]
     *
     * C++ counterpart of .NET System.Xml.Linq.LoadOptions.
     *
     * @note `SetBaseUri`/`SetLineInfo` are accepted but have no effect in this port — `XObject`
     * here does not track base URI or line/position info (no annotation system, see XObject's
     * doc comment).
     */
    enum class LoadOptions {
        None = 0,
        PreserveWhitespace = 1,
        SetBaseUri = 2,
        SetLineInfo = 4,
    };

    constexpr LoadOptions operator|(LoadOptions a, LoadOptions b) {
        return static_cast<LoadOptions>(static_cast<int>(a) | static_cast<int>(b));
    }
    constexpr LoadOptions operator&(LoadOptions a, LoadOptions b) {
        return static_cast<LoadOptions>(static_cast<int>(a) & static_cast<int>(b));
    }

} // namespace System::Xml::Linq

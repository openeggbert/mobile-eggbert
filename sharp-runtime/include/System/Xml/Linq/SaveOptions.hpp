// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Xml::Linq {

    /**
     * @brief Specifies serialization options. [Flags]
     *
     * C++ counterpart of .NET System.Xml.Linq.SaveOptions.
     */
    enum class SaveOptions {
        None = 0,
        DisableFormatting = 1,
        OmitDuplicateNamespaces = 2,
    };

    constexpr SaveOptions operator|(SaveOptions a, SaveOptions b) {
        return static_cast<SaveOptions>(static_cast<int>(a) | static_cast<int>(b));
    }
    constexpr SaveOptions operator&(SaveOptions a, SaveOptions b) {
        return static_cast<SaveOptions>(static_cast<int>(a) & static_cast<int>(b));
    }

} // namespace System::Xml::Linq

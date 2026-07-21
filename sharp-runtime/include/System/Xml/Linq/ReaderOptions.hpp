// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Xml::Linq {

    /**
     * @brief Specifies whether to omit duplicate namespaces when reading a tree via an XmlReader. [Flags]
     *
     * C++ counterpart of .NET System.Xml.Linq.ReaderOptions.
     *
     * @note This port has no `CreateReader()`/`XmlReader` integration (see `XNode`'s doc
     * comment), so this enum has no consumer here yet — reproduced for API completeness/future use.
     */
    enum class ReaderOptions {
        None = 0,
        OmitDuplicateNamespaces = 1,
    };

    constexpr ReaderOptions operator|(ReaderOptions a, ReaderOptions b) {
        return static_cast<ReaderOptions>(static_cast<int>(a) | static_cast<int>(b));
    }
    constexpr ReaderOptions operator&(ReaderOptions a, ReaderOptions b) {
        return static_cast<ReaderOptions>(static_cast<int>(a) & static_cast<int>(b));
    }

} // namespace System::Xml::Linq

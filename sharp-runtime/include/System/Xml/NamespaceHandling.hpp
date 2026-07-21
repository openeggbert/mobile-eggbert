// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Xml {

    /**
     * @brief Specifies how an XmlWriter handles namespace declarations. [Flags]
     *
     * C++ counterpart of .NET System.Xml.NamespaceHandling.
     */
    enum class NamespaceHandling {
        /** Default behavior: all namespace declarations are written as encountered. */
        Default = 0x0,
        /** Duplicate namespace declarations are omitted. */
        OmitDuplicates = 0x1,
    };

    /** @brief Bitwise OR for the [Flags] NamespaceHandling enum. */
    constexpr NamespaceHandling operator|(NamespaceHandling a, NamespaceHandling b) {
        return static_cast<NamespaceHandling>(static_cast<int>(a) | static_cast<int>(b));
    }

    /** @brief Bitwise AND for the [Flags] NamespaceHandling enum. */
    constexpr NamespaceHandling operator&(NamespaceHandling a, NamespaceHandling b) {
        return static_cast<NamespaceHandling>(static_cast<int>(a) & static_cast<int>(b));
    }

} // namespace System::Xml

// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Xml {

    /**
     * @brief Specifies whether an XmlReader or XmlWriter enforces well-formed XML fragment
     * or document conformance, or determines it automatically.
     *
     * C++ counterpart of .NET System.Xml.ConformanceLevel.
     */
    enum class ConformanceLevel {
        /** Automatically determines whether the input is an XML fragment or a full document. */
        Auto = 0,
        /** Conformance level for an XML fragment (a single XML declaration plus element-child content). */
        Fragment = 1,
        /** Conformance level for a full XML document per the XML 1.0 specification. */
        Document = 2,
    };

} // namespace System::Xml

// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Xml {

    /**
     * @brief Specifies the output method used by an XmlWriter (as configured by XmlWriterSettings).
     *
     * C++ counterpart of .NET System.Xml.XmlOutputMethod.
     */
    enum class XmlOutputMethod {
        /** Serialize using XML 1.0 rules. */
        Xml = 0,
        /** Serialize using HTML output rules. */
        Html = 1,
        /** Serialize text blocks only. */
        Text = 2,
        /** Choose between Xml and Html output at runtime. */
        AutoDetect = 3,
    };

} // namespace System::Xml

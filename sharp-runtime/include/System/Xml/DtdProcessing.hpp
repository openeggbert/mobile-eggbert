// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Xml {

    /**
     * @brief Specifies how an XmlReader handles DTDs (Document Type Definitions) in the XML document.
     *
     * C++ counterpart of .NET System.Xml.DtdProcessing.
     */
    enum class DtdProcessing {
        /** The reader throws on encountering a `<!DOCTYPE` markup declaration. */
        Prohibit = 0,
        /** The DTD is ignored; the DocumentType node is not reported. */
        Ignore = 1,
        /** The DTD is fully parsed and processed (entities expanded, default attributes added, etc.). */
        Parse = 2,
    };

} // namespace System::Xml

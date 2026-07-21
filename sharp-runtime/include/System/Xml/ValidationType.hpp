// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Xml {

    /**
     * @brief Specifies the type of validation to perform (legacy XmlValidatingReader / XmlReaderSettings).
     *
     * C++ counterpart of .NET System.Xml.ValidationType. This runtime does not implement real
     * DTD/XSD validation (see XmlValidatingReader's doc-comment) — this enum exists for API
     * name compatibility with ported code that references the values.
     */
    enum class ValidationType {
        /** No validation is performed. */
        None = 0,
        /** Deprecated in .NET; auto-detect DTD/XSD. Not implemented by this runtime. */
        Auto = 1,
        /** Validate against a DTD. Not implemented by this runtime. */
        DTD = 2,
        /** Deprecated in .NET (XDR). Not implemented by this runtime. */
        XDR = 3,
        /** Validate against W3C XSD schemas. Not implemented by this runtime. */
        Schema = 4,
    };

} // namespace System::Xml

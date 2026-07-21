// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Xml {

    /**
     * @brief Specifies the DTD/XSD-declared type of an attribute or entity token.
     *
     * C++ counterpart of .NET System.Xml.XmlTokenizedType.
     */
    enum class XmlTokenizedType {
        CDATA = 0,
        ID = 1,
        IDREF = 2,
        IDREFS = 3,
        ENTITY = 4,
        ENTITIES = 5,
        NMTOKEN = 6,
        NMTOKENS = 7,
        NOTATION = 8,
        ENUMERATION = 9,
        QName = 10,
        NCName = 11,
        /** No token type / not applicable. */
        None = 12,
    };

} // namespace System::Xml

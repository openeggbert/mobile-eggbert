// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Xml {

    /**
     * @brief Specifies the type of an XML node.
     *
     * C++ counterpart of .NET System.Xml.XmlNodeType.
     */
    enum class XmlNodeType {
        /** No node (e.g. a cursor not positioned on any node). */
        None = 0,
        /** An element, e.g. `<Name>`. */
        Element = 1,
        /** An attribute, e.g. `id='123'`. */
        Attribute = 2,
        /** The text content of an element or attribute. */
        Text = 3,
        /** A CDATA section, e.g. `<![CDATA[...]]>`. */
        CDATA = 4,
        /** A reference to an entity, e.g. `&foo;`. */
        EntityReference = 5,
        /** An entity declaration, e.g. `<!ENTITY ...>`. */
        Entity = 6,
        /** A processing instruction, e.g. `<?pi test?>`. */
        ProcessingInstruction = 7,
        /** A comment, e.g. `<!-- comment -->`. */
        Comment = 8,
        /** The root document node. */
        Document = 9,
        /** The document type declaration, e.g. `<!DOCTYPE ...>`. */
        DocumentType = 10,
        /** A document fragment. */
        DocumentFragment = 11,
        /** A notation in the document type declaration, e.g. `<!NOTATION ...>`. */
        Notation = 12,
        /** Whitespace between markup. */
        Whitespace = 13,
        /** Whitespace between markup in a mixed content model (xml:space="preserve" scope). */
        SignificantWhitespace = 14,
        /** The end tag of an element, e.g. `</foo>`. */
        EndElement = 15,
        /** Returned at the end of entity replacement content. */
        EndEntity = 16,
        /** The XML declaration node, e.g. `<?xml version='1.0'?>`. */
        XmlDeclaration = 17,
    };

} // namespace System::Xml

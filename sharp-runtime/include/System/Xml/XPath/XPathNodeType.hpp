// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Xml::XPath {

    /**
     * @brief Describes the type of an XPath node, as defined by the XPath/XQuery data model.
     *
     * C++ counterpart of .NET System.Xml.XPath.XPathNodeType. This is a smaller, XPath-specific
     * classification than System::Xml::XmlNodeType: e.g. CDATA sections collapse into Text, and
     * there is no XmlDeclaration/DocumentType/EntityReference/DocumentFragment/Notation
     * equivalent (those node kinds simply do not appear when navigating via XPathNavigator).
     */
    enum class XPathNodeType {
        /** The root node of the document (matches "/" in a location path). */
        Root,
        /** An element node. */
        Element,
        /** An attribute node. */
        Attribute,
        /** A namespace node. */
        Namespace,
        /** A text node. */
        Text,
        /** A whitespace node within an xml:space="preserve" scope. */
        SignificantWhitespace,
        /** A whitespace-only node outside of any xml:space="preserve" scope. */
        Whitespace,
        /** A processing instruction node (excluding the XML declaration). */
        ProcessingInstruction,
        /** A comment node. */
        Comment,
        /** Matches any node type; used as a wildcard node-type test. */
        All,
    };

} // namespace System::Xml::XPath

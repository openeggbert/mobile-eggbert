// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include "System/Xml/XmlNode.hpp"

namespace System::Xml {

    /**
     * @brief A lightweight container used to hold a subtree not yet attached to a document.
     *
     * C++ counterpart of .NET System.Xml.XmlDocumentFragment. When a fragment is inserted via
     * AppendChild/InsertBefore/InsertAfter on another node, its children are spliced in and the
     * fragment itself is emptied — matching .NET's DocumentFragment insertion behavior.
     * Backed internally by a scratch tinyxml2 element (owned by the same document, never itself
     * inserted into the visible tree) so ordinary child navigation "just works."
     */
    class XmlDocumentFragment : public XmlNode {
    public:
        XmlDocumentFragment(tinyxml2::XMLNode* native, XmlDocument* ownerDocument)
            : XmlNode(native, ownerDocument) {}

        [[nodiscard]] XmlNodeType getNodeTypeProperty() const override { return XmlNodeType::DocumentFragment; }
        [[nodiscard]] std::string getNameProperty() const override { return "#document-fragment"; }
        [[nodiscard]] std::string getLocalNameProperty() const override { return "#document-fragment"; }
    };

} // namespace System::Xml

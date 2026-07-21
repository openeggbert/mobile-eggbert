// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include "System/Xml/XmlNode.hpp"

namespace System::Xml {

    /**
     * @brief Base class for nodes that can have siblings (everything except XmlAttribute,
     * XmlDocument, and XmlDocumentFragment).
     *
     * C++ counterpart of .NET System.Xml.XmlLinkedNode. In real .NET this class exists to give
     * PreviousSibling/NextSibling a narrower declared return context; this runtime implements
     * sibling navigation uniformly on XmlNode itself, so XmlLinkedNode adds no members — it
     * exists purely so ported code that names the type still compiles.
     */
    class XmlLinkedNode : public XmlNode {
    protected:
        XmlLinkedNode() = default;
        XmlLinkedNode(tinyxml2::XMLNode* native, XmlDocument* ownerDocument)
            : XmlNode(native, ownerDocument) {}
    };

} // namespace System::Xml

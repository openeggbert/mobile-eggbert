// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <string>
#include <vector>

#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Xml {

    class XmlNode;

    /**
     * @brief A read-only, ordered collection of nodes accessible by name or by index.
     *
     * C++ counterpart of .NET System.Xml.XmlNamedNodeMap. Backs XmlDocumentType::Entities /
     * ::Notations; XmlElement::Attributes uses the richer XmlAttributeCollection subclass.
     * Holds a snapshot of non-owning node pointers (lifetime is owned by the document).
     */
    class XmlNamedNodeMap {
    protected:
        std::vector<XmlNode*> items_;

    public:
        virtual ~XmlNamedNodeMap() = default;

        /** @return The node with the given qualified name, or nullptr if none. */
        [[nodiscard]] virtual XmlNode* GetNamedItem(const std::string& name) const;
        /** @return The node with the given local name and namespace URI, or nullptr if none. */
        [[nodiscard]] virtual XmlNode* GetNamedItem(const std::string& localName, const std::string& namespaceURI) const;
        /** @return The number of nodes in the map. */
        [[nodiscard]] virtual SharpRuntime::intcs getCountProperty() const { return static_cast<SharpRuntime::intcs>(items_.size()); }
        /** @return The node at @p index, or nullptr if out of range. */
        [[nodiscard]] virtual XmlNode* Item(SharpRuntime::intcs index) const;
    };

} // namespace System::Xml

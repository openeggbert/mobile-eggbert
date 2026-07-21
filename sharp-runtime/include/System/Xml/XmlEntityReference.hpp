// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include "System/Xml/XmlLinkedNode.hpp"

namespace System::Xml {

    /**
     * @brief Represents a reference to an entity, e.g. `&foo;`.
     *
     * C++ counterpart of .NET System.Xml.XmlEntityReference. This runtime does not process DTDs
     * (see DtdProcessing/EntityHandling), so entity references are never produced by parsing —
     * this type exists only for XmlDocument::CreateEntityReference and stands detached from the
     * tinyxml2 tree (it has no children and cannot be resolved/expanded).
     */
    class XmlEntityReference : public XmlLinkedNode {
        std::string name_;

    public:
        XmlEntityReference(XmlDocument* ownerDocument, std::string name)
            : XmlLinkedNode(), name_(std::move(name)) { ownerDocument_ = ownerDocument; }

        [[nodiscard]] XmlNodeType getNodeTypeProperty() const override { return XmlNodeType::EntityReference; }
        [[nodiscard]] std::string getNameProperty() const override { return name_; }
        [[nodiscard]] std::string getLocalNameProperty() const override { return name_; }
    };

} // namespace System::Xml

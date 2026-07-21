// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include "System/Xml/XmlLinkedNode.hpp"

namespace System::Xml {

    /**
     * @brief Represents the document type declaration, e.g. `<!DOCTYPE root PUBLIC "..." "...">`.
     *
     * C++ counterpart of .NET System.Xml.XmlDocumentType. Backed by a tinyxml2 XMLUnknown node
     * (tinyxml2 has no dedicated DOCTYPE type); Name/PublicId/SystemId are parsed from the raw
     * declaration text on construction and cached, since tinyxml2's simple ">"-delimited parsing
     * cannot correctly handle a DOCTYPE with an internal subset containing ">" characters
     * (a known tinyxml2 limitation — not something this port can work around).
     */
    class XmlDocumentType : public XmlLinkedNode {
        std::string name_;
        std::string publicId_;
        std::string systemId_;

    public:
        XmlDocumentType(tinyxml2::XMLNode* native, XmlDocument* ownerDocument,
                        std::string name, std::string publicId, std::string systemId)
            : XmlLinkedNode(native, ownerDocument),
              name_(std::move(name)), publicId_(std::move(publicId)), systemId_(std::move(systemId)) {}

        [[nodiscard]] XmlNodeType getNodeTypeProperty() const override { return XmlNodeType::DocumentType; }
        [[nodiscard]] std::string getNameProperty() const override { return name_; }
        [[nodiscard]] std::string getLocalNameProperty() const override { return name_; }

        /** @return The public identifier, or "" if absent. */
        [[nodiscard]] const std::string& getPublicIdProperty() const { return publicId_; }
        /** @return The system identifier, or "" if absent. */
        [[nodiscard]] const std::string& getSystemIdProperty() const { return systemId_; }
        /** @return "" — the internal DTD subset is not parsed by this runtime. */
        [[nodiscard]] std::string getInternalSubsetProperty() const { return {}; }
    };

} // namespace System::Xml

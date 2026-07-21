// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/Xml/Linq/XNode.hpp"

namespace System::Xml::Linq {

    /**
     * @brief Represents an XML Document Type Definition (DTD).
     *
     * C++ counterpart of .NET System.Xml.Linq.XDocumentType.
     *
     * @note InternalSubset is always "" when parsed from XML — this runtime's tinyxml2-backed
     * DOM layer does not parse the internal DTD subset (see System::Xml::XmlDocumentType's doc
     * comment); settable/gettable here for API completeness and manual construction.
     */
    class XDocumentType : public XNode {
        std::string name_;
        std::string publicId_;
        std::string systemId_;
        std::string internalSubset_;

    public:
        /** @brief Initializes a new document type declaration. */
        XDocumentType(const std::string& name, const std::string& publicId,
                      const std::string& systemId, const std::string& internalSubset)
            : name_(name), publicId_(publicId), systemId_(systemId), internalSubset_(internalSubset) {}

        [[nodiscard]] System::Xml::XmlNodeType getNodeTypeProperty() const override { return System::Xml::XmlNodeType::DocumentType; }

        /** @return The name of this DTD. */
        [[nodiscard]] const std::string& getNameProperty() const { return name_; }
        /** @brief Sets the name of this DTD. */
        void setNameProperty(const std::string& name) { name_ = name; }

        /** @return The public identifier, or "" if absent. */
        [[nodiscard]] const std::string& getPublicIdProperty() const { return publicId_; }
        /** @brief Sets the public identifier. */
        void setPublicIdProperty(const std::string& publicId) { publicId_ = publicId; }

        /** @return The system identifier, or "" if absent. */
        [[nodiscard]] const std::string& getSystemIdProperty() const { return systemId_; }
        /** @brief Sets the system identifier. */
        void setSystemIdProperty(const std::string& systemId) { systemId_ = systemId; }

        /** @return The internal subset, or "" if absent/not parsed (see class doc-comment). */
        [[nodiscard]] const std::string& getInternalSubsetProperty() const { return internalSubset_; }
        /** @brief Sets the internal subset. */
        void setInternalSubsetProperty(const std::string& internalSubset) { internalSubset_ = internalSubset; }

        void WriteTo(System::Xml::XmlWriter& writer) const override;
        [[nodiscard]] SharpRuntime::intcs GetDeepHashCode() const override {
            return static_cast<SharpRuntime::intcs>(std::hash<std::string>{}(name_)) ^
                   static_cast<SharpRuntime::intcs>(std::hash<std::string>{}(publicId_)) ^
                   static_cast<SharpRuntime::intcs>(std::hash<std::string>{}(systemId_)) ^
                   static_cast<SharpRuntime::intcs>(std::hash<std::string>{}(internalSubset_));
        }

    protected:
        void SerializeTo(std::ostream& os, int depth, bool indent) const override;
        [[nodiscard]] bool DeepEqualsCore(const XNode& other) const override {
            const auto& o = static_cast<const XDocumentType&>(other);
            return name_ == o.name_ && publicId_ == o.publicId_ && systemId_ == o.systemId_ && internalSubset_ == o.internalSubset_;
        }
    };

} // namespace System::Xml::Linq

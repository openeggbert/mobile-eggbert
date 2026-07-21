// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <memory>
#include <string>

#include "System/Xml/XmlNameTable.hpp"
#include "System/Xml/XmlNamespaceManager.hpp"
#include "System/Xml/XmlSpace.hpp"

namespace System::Xml {

    /**
     * @brief Provides context information required by XmlTextReader to parse an XML fragment
     * that has document-type or namespace information defined outside of the fragment.
     *
     * C++ counterpart of .NET System.Xml.XmlParserContext. This runtime's XmlTextReader parses
     * whole documents rather than DTD-context-dependent fragments, so this type is currently a
     * plain data holder for API-name compatibility rather than something consulted during
     * parsing — a documented practical-subset limitation, not a silent gap.
     */
    class XmlParserContext {
    public:
        XmlParserContext(std::shared_ptr<XmlNameTable> nameTable,
                          std::shared_ptr<XmlNamespaceManager> namespaceManager,
                          std::string docTypeName, std::string pubId, std::string sysId,
                          std::string internalSubset, std::string baseURI,
                          std::string xmlLang, XmlSpace xmlSpace, std::string encoding = {})
            : nameTable_(std::move(nameTable)), namespaceManager_(std::move(namespaceManager)),
              docTypeName_(std::move(docTypeName)), pubId_(std::move(pubId)), sysId_(std::move(sysId)),
              internalSubset_(std::move(internalSubset)), baseURI_(std::move(baseURI)),
              xmlLang_(std::move(xmlLang)), xmlSpace_(xmlSpace), encoding_(std::move(encoding)) {}

        [[nodiscard]] std::shared_ptr<XmlNameTable> getNameTableProperty() const { return nameTable_; }
        void setNameTableProperty(std::shared_ptr<XmlNameTable> value) { nameTable_ = std::move(value); }

        [[nodiscard]] std::shared_ptr<XmlNamespaceManager> getNamespaceManagerProperty() const { return namespaceManager_; }
        void setNamespaceManagerProperty(std::shared_ptr<XmlNamespaceManager> value) { namespaceManager_ = std::move(value); }

        [[nodiscard]] const std::string& getDocTypeNameProperty() const { return docTypeName_; }
        void setDocTypeNameProperty(std::string value) { docTypeName_ = std::move(value); }

        [[nodiscard]] const std::string& getPublicIdProperty() const { return pubId_; }
        void setPublicIdProperty(std::string value) { pubId_ = std::move(value); }

        [[nodiscard]] const std::string& getSystemIdProperty() const { return sysId_; }
        void setSystemIdProperty(std::string value) { sysId_ = std::move(value); }

        [[nodiscard]] const std::string& getInternalSubsetProperty() const { return internalSubset_; }
        void setInternalSubsetProperty(std::string value) { internalSubset_ = std::move(value); }

        [[nodiscard]] const std::string& getBaseURIProperty() const { return baseURI_; }
        void setBaseURIProperty(std::string value) { baseURI_ = std::move(value); }

        [[nodiscard]] const std::string& getXmlLangProperty() const { return xmlLang_; }
        void setXmlLangProperty(std::string value) { xmlLang_ = std::move(value); }

        [[nodiscard]] XmlSpace getXmlSpaceProperty() const { return xmlSpace_; }
        void setXmlSpaceProperty(XmlSpace value) { xmlSpace_ = value; }

        [[nodiscard]] const std::string& getEncodingProperty() const { return encoding_; }
        void setEncodingProperty(std::string value) { encoding_ = std::move(value); }

    private:
        std::shared_ptr<XmlNameTable> nameTable_;
        std::shared_ptr<XmlNamespaceManager> namespaceManager_;
        std::string docTypeName_;
        std::string pubId_;
        std::string sysId_;
        std::string internalSubset_;
        std::string baseURI_;
        std::string xmlLang_;
        XmlSpace xmlSpace_;
        std::string encoding_;
    };

} // namespace System::Xml

// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Xml/XPath/XPathDocument.hpp"

#include "System/Xml/XPath/XPathNavigator.hpp"
#include "System/Xml/XmlDocument.hpp"

namespace System::Xml::XPath {

    XPathDocument::XPathDocument() : doc_(std::make_unique<System::Xml::XmlDocument>()) {}

    XPathDocument::XPathDocument(const std::string& uri) : doc_(std::make_unique<System::Xml::XmlDocument>()) {
        doc_->Load(uri);
    }

    XPathDocument::~XPathDocument() = default;

    void XPathDocument::LoadXml(const std::string& xml) {
        doc_->LoadXml(xml);
    }

    XPathNavigator* XPathDocument::CreateNavigator() const {
        return doc_->CreateNavigator();
    }

} // namespace System::Xml::XPath

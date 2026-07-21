// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Xml/XmlAttribute.hpp"

#include <tinyxml2/tinyxml2.h>

#include "System/Xml/XmlDocument.hpp"

namespace System::Xml {

    std::string XmlAttribute::getValueProperty() const {
        if (ownerElementNative_) {
            const char* v = ownerElementNative_->Attribute(name_.c_str());
            return v ? v : "";
        }
        return localValue_;
    }

    void XmlAttribute::setValueProperty(const std::string& value) {
        if (ownerElementNative_) {
            ownerElementNative_->SetAttribute(name_.c_str(), value.c_str());
        } else {
            localValue_ = value;
        }
    }

    void XmlAttribute::AttachTo(XmlElement* elementWrapper, tinyxml2::XMLElement* elementNative) {
        ownerElementWrapper_ = elementWrapper;
        ownerElementNative_ = elementNative;
        elementNative->SetAttribute(name_.c_str(), localValue_.c_str());
    }

    void XmlAttribute::BindExisting(XmlElement* elementWrapper, tinyxml2::XMLElement* elementNative) {
        ownerElementWrapper_ = elementWrapper;
        ownerElementNative_ = elementNative;
    }

    void XmlAttribute::Detach() {
        if (ownerElementNative_) {
            const char* v = ownerElementNative_->Attribute(name_.c_str());
            localValue_ = v ? v : "";
        }
        ownerElementWrapper_ = nullptr;
        ownerElementNative_ = nullptr;
    }

    // Verified against XmlAttribute.cs's NamespaceURI: real .NET captures an attribute's
    // namespace at creation time; this port instead resolves it dynamically by walking ancestor
    // elements for the nearest matching xmlns:prefix declaration, consistent with how
    // XmlNode::getNamespaceURIProperty() already does this for other node types. The previous
    // implementation inherited that base method unmodified, which walks starting from `native_`
    // -- a field XmlAttribute never sets (it tracks its owning element separately via
    // ownerElementNative_), so the walk never even started and always returned "". Unlike
    // elements, an unprefixed attribute has no namespace at all: the XML Namespaces spec (and
    // real .NET) do not apply an ancestor's default `xmlns="..."` declaration to unprefixed
    // attributes, only prefixed ones via `xmlns:prefix="..."`.
    std::string XmlAttribute::getNamespaceURIProperty() const {
        std::string prefix = getPrefixProperty();
        if (prefix.empty()) return {};
        if (prefix == "xml") return "http://www.w3.org/XML/1998/namespace";
        std::string attrName = "xmlns:" + prefix;
        for (tinyxml2::XMLNode* n = ownerElementNative_; n; n = n->Parent()) {
            if (auto* el = n->ToElement()) {
                const char* v = el->Attribute(attrName.c_str());
                if (v) return v;
            }
        }
        return {};
    }

    // Verified against XmlAttribute.cs's CloneNode(bool): real .NET always performs a full
    // clone regardless of the deep parameter (an attribute's value has no meaningful shallow
    // form) via doc.CreateAttribute(...) followed by copying its content. This port previously
    // inherited XmlNode's default CloneNode(), which early-returns nullptr whenever native_ is
    // null -- true for every XmlAttribute, since (as above) attributes never set native_.
    XmlNode* XmlAttribute::CloneNode(bool /*deep*/) const {
        XmlDocument* doc = GetDocument();
        if (!doc) return nullptr;
        XmlAttribute* clone = doc->CreateAttribute(name_);
        clone->setValueProperty(getValueProperty());
        return clone;
    }

} // namespace System::Xml

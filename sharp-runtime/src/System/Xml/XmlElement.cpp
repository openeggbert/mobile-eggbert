// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Xml/XmlElement.hpp"

#include <tinyxml2/tinyxml2.h>

#include "System/Xml/XmlDocument.hpp"
#include "System/Xml/XmlWriter.hpp"

namespace System::Xml {

    tinyxml2::XMLElement* XmlElement::nativeElement() const {
        return native_ ? native_->ToElement() : nullptr;
    }

    XmlAttribute* XmlElement::GetOrCreateAttrWrapper(const std::string& name) {
        auto it = attrCache_.find(name);
        if (it != attrCache_.end()) return it->second.get();
        auto wrapper = std::make_unique<XmlAttribute>(ownerDocument_, name);
        wrapper->BindExisting(const_cast<XmlElement*>(this), nativeElement());
        auto* raw = wrapper.get();
        attrCache_.emplace(name, std::move(wrapper));
        return raw;
    }

    XmlAttributeCollection* XmlElement::getAttributesProperty() const {
        std::vector<XmlNode*> attrs;
        auto* el = nativeElement();
        if (el) {
            for (const tinyxml2::XMLAttribute* a = el->FirstAttribute(); a; a = a->Next()) {
                std::string name = a->Name() ? a->Name() : "";
                attrs.push_back(const_cast<XmlElement*>(this)->GetOrCreateAttrWrapper(name));
            }
        }
        attrsSnapshot_ = std::make_unique<XmlAttributeCollection>(const_cast<XmlElement*>(this), std::move(attrs));
        return attrsSnapshot_.get();
    }

    bool XmlElement::getHasAttributesProperty() const {
        auto* el = nativeElement();
        return el && el->FirstAttribute() != nullptr;
    }

    std::string XmlElement::GetAttribute(const std::string& name) const {
        auto* el = nativeElement();
        if (!el) return {};
        const char* v = el->Attribute(name.c_str());
        return v ? v : "";
    }

    void XmlElement::SetAttribute(const std::string& name, const std::string& value) {
        auto* el = nativeElement();
        if (!el) return;
        el->SetAttribute(name.c_str(), value.c_str());
        GetOrCreateAttrWrapper(name); // ensure a wrapper exists for identity stability
    }

    void XmlElement::RemoveAttribute(const std::string& name) {
        auto* el = nativeElement();
        if (!el) return;
        el->DeleteAttribute(name.c_str());
        auto it = attrCache_.find(name);
        if (it != attrCache_.end()) { it->second->Detach(); attrCache_.erase(it); }
    }

    XmlAttribute* XmlElement::GetAttributeNode(const std::string& name) {
        auto* el = nativeElement();
        if (!el || !el->Attribute(name.c_str())) return nullptr;
        return GetOrCreateAttrWrapper(name);
    }

    XmlAttribute* XmlElement::SetAttributeNode(XmlAttribute* newAttr) {
        if (!newAttr) return nullptr;
        std::string name = newAttr->getNameProperty();
        std::string value = newAttr->getValueProperty();
        XmlAttribute* replaced = nullptr;
        auto it = attrCache_.find(name);
        if (it != attrCache_.end() && it->second.get() != newAttr) {
            replaced = it->second.release();
            replaced->Detach();
            attrCache_.erase(it);
        }
        newAttr->AttachTo(this, nativeElement());
        newAttr->setValueProperty(value);
        // If newAttr came from XmlDocument::CreateAttribute and was never attached before, the
        // document is holding it in unattachedNodes_ -- release that ownership now that
        // attrCache_ below is taking over, or the document would double-own (and eventually
        // double-delete) it. No-op if newAttr wasn't tracked there (e.g. it's being moved from
        // another element's attrCache_ instead).
        if (auto* doc = GetDocument()) doc->ReleaseUnattachedNode(newAttr);
        attrCache_[name] = std::unique_ptr<XmlAttribute>(newAttr);
        return replaced;
    }

    XmlAttribute* XmlElement::RemoveAttributeNode(XmlAttribute* oldAttr) {
        if (!oldAttr) return nullptr;
        std::string name = oldAttr->getNameProperty();
        auto it = attrCache_.find(name);
        if (it == attrCache_.end() || it->second.get() != oldAttr) return nullptr;
        auto* el = nativeElement();
        if (el) el->DeleteAttribute(name.c_str());
        it->second.release();
        attrCache_.erase(it);
        oldAttr->Detach();
        return oldAttr;
    }

    bool XmlElement::HasAttribute(const std::string& name) const {
        auto* el = nativeElement();
        return el && el->Attribute(name.c_str()) != nullptr;
    }

    XmlNode* XmlElement::RemoveAttributeAt(SharpRuntime::intcs i) {
        auto* el = nativeElement();
        if (!el) return nullptr;
        SharpRuntime::intcs idx = 0;
        for (const tinyxml2::XMLAttribute* a = el->FirstAttribute(); a; a = a->Next(), ++idx) {
            if (idx == i) {
                std::string name = a->Name() ? a->Name() : "";
                auto* wrapper = GetOrCreateAttrWrapper(name);
                return RemoveAttributeNode(wrapper);
            }
        }
        return nullptr;
    }

    void XmlElement::RemoveAllAttributes() {
        for (auto& [name, wrapper] : attrCache_) wrapper->Detach();
        attrCache_.clear();
        auto* el = nativeElement();
        if (!el) return;
        while (el->FirstAttribute())
            el->DeleteAttribute(el->FirstAttribute()->Name());
    }

    void XmlElement::RemoveAll() {
        XmlNode::RemoveAll();
        RemoveAllAttributes();
    }

    namespace {
        void CollectElementsByName(tinyxml2::XMLNode* node, const std::string& name,
                                   XmlDocument* doc, std::vector<XmlNode*>& out) {
            for (tinyxml2::XMLNode* child = node->FirstChild(); child; child = child->NextSibling()) {
                if (auto* el = child->ToElement()) {
                    if (name == "*" || (el->Name() && name == el->Name()))
                        out.push_back(doc->WrapNode(child));
                    CollectElementsByName(child, name, doc, out);
                }
            }
        }
    }

    XmlNodeList* XmlElement::GetElementsByTagName(const std::string& name) const {
        std::vector<XmlNode*> results;
        CollectElementsByName(native_, name, ownerDocument_, results);
        elementsByTagNameSnapshot_ = std::make_unique<XmlNodeList>(std::move(results));
        return elementsByTagNameSnapshot_.get();
    }

    std::string XmlElement::getInnerXmlProperty() const {
        return XmlNode::getInnerXmlProperty();
    }

    void XmlElement::setInnerXmlProperty(const std::string& xml) {
        XmlNode::setInnerXmlProperty(xml);
    }

    void XmlElement::WriteTo(XmlWriter& writer) const {
        auto* el = nativeElement();
        if (!el) return;
        writer.WriteStartElement(el->Name() ? el->Name() : "");
        for (const tinyxml2::XMLAttribute* a = el->FirstAttribute(); a; a = a->Next())
            writer.WriteAttributeString(a->Name() ? a->Name() : "", a->Value() ? a->Value() : "");
        WriteContentTo(writer);
        writer.WriteEndElement();
    }

    void XmlElement::WriteContentTo(XmlWriter& writer) const {
        XmlNode::WriteContentTo(writer);
    }

} // namespace System::Xml

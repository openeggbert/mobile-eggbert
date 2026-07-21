// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Xml/XmlAttributeCollection.hpp"

#include <algorithm>

#include "System/Xml/XmlElement.hpp"

namespace System::Xml {

    XmlNode* XmlAttributeCollection::SetNamedItem(XmlNode* node) {
        auto* attr = static_cast<XmlAttribute*>(node);
        return owner_->SetAttributeNode(attr);
    }

    XmlNode* XmlAttributeCollection::RemoveNamedItem(const std::string& name) {
        auto* attr = owner_->GetAttributeNode(name);
        if (!attr) return nullptr;
        return owner_->RemoveAttributeNode(attr);
    }

    XmlAttribute* XmlAttributeCollection::Prepend(XmlAttribute* node) {
        return Append(node); // tinyxml2 attributes have no ordering API to prepend by; append is the practical equivalent.
    }

    XmlAttribute* XmlAttributeCollection::Append(XmlAttribute* node) {
        owner_->SetAttributeNode(node);
        return node;
    }

    XmlAttribute* XmlAttributeCollection::InsertBefore(XmlAttribute* newNode, XmlAttribute* /*refNode*/) {
        return Append(newNode); // see Prepend: tinyxml2 has no attribute ordering to insert within.
    }

    XmlAttribute* XmlAttributeCollection::InsertAfter(XmlAttribute* newNode, XmlAttribute* /*refNode*/) {
        return Append(newNode);
    }

    XmlAttribute* XmlAttributeCollection::Remove(XmlAttribute* node) {
        if (!node) return nullptr;
        return owner_->RemoveAttributeNode(node);
    }

    XmlAttribute* XmlAttributeCollection::RemoveAt(SharpRuntime::intcs i) {
        auto* node = static_cast<XmlAttribute*>(Item(i));
        if (!node) return nullptr;
        return owner_->RemoveAttributeNode(node);
    }

    void XmlAttributeCollection::RemoveAll() {
        owner_->RemoveAllAttributes();
    }

} // namespace System::Xml

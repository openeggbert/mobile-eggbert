// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Xml/XmlNamedNodeMap.hpp"

#include "System/Xml/XmlNode.hpp"

namespace System::Xml {

    XmlNode* XmlNamedNodeMap::GetNamedItem(const std::string& name) const {
        for (XmlNode* item : items_)
            if (item->getNameProperty() == name) return item;
        return nullptr;
    }

    XmlNode* XmlNamedNodeMap::GetNamedItem(const std::string& localName, const std::string& namespaceURI) const {
        for (XmlNode* item : items_)
            if (item->getLocalNameProperty() == localName && item->getNamespaceURIProperty() == namespaceURI)
                return item;
        return nullptr;
    }

    XmlNode* XmlNamedNodeMap::Item(SharpRuntime::intcs index) const {
        if (index < 0 || static_cast<size_t>(index) >= items_.size()) return nullptr;
        return items_[static_cast<size_t>(index)];
    }

} // namespace System::Xml

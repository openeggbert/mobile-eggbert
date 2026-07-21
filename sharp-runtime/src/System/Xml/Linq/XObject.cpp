// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Xml/Linq/XObject.hpp"
#include "System/Xml/Linq/XContainer.hpp"
#include "System/Xml/Linq/XElement.hpp"
#include "System/Xml/Linq/XDocument.hpp"

namespace System::Xml::Linq {

    XElement* XObject::getParentProperty() const {
        if (parent_ != nullptr && parent_->getNodeTypeProperty() == System::Xml::XmlNodeType::Element) {
            return static_cast<XElement*>(parent_);
        }
        return nullptr;
    }

    XDocument* XObject::getDocumentProperty() const {
        const XObject* o = this;
        while (o->parent_ != nullptr) {
            o = o->parent_;
        }
        if (o->getNodeTypeProperty() == System::Xml::XmlNodeType::Document) {
            return const_cast<XDocument*>(static_cast<const XDocument*>(o));
        }
        return nullptr;
    }

} // namespace System::Xml::Linq

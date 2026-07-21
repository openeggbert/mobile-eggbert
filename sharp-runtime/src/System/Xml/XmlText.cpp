// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Xml/XmlText.hpp"

#include "System/ArgumentOutOfRangeException.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/Xml/XmlDocument.hpp"

namespace System::Xml {

    XmlText* XmlText::SplitText(SharpRuntime::intcs offset) {
        auto* parent = getParentNodeProperty();
        SharpRuntime::intcs length = getLengthProperty();
        if (offset > length) throw System::ArgumentOutOfRangeException("offset");
        if (!parent)
            throw System::InvalidOperationException(
                "The 'Text' node is not connected in the DOM live tree. No 'SplitText' operation could be performed.");

        std::string data = getDataProperty();
        std::string tail = data.substr(static_cast<size_t>(offset));
        setDataProperty(data.substr(0, static_cast<size_t>(offset)));

        auto* newNode = ownerDocument_->CreateTextNode(tail);
        parent->InsertAfter(newNode, this);
        return newNode;
    }

} // namespace System::Xml

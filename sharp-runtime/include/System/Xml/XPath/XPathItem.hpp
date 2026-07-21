// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <string>

namespace System::Xml::XPath {

    /**
     * @brief Base class for XPathNavigator (and, in real .NET, the atomic-value type
     * XmlAtomicValue).
     *
     * C++ counterpart of .NET System.Xml.XPath.XPathItem, scoped down: this runtime's XPath
     * subset only ever produces node items (backed by a DOM node), never standalone
     * schema-typed atomic values, so IsNode is always true and the schema-typed accessors
     * (XmlType/TypedValue/ValueType/ValueAs*) from real .NET are not provided — this runtime has
     * no XML Schema type system (see CLAUDE.md's reflection/schema permanent-deviation note).
     * Use XPathNavigator::getValueProperty() and the ValueAsBoolean/Double-style helpers instead
     * for untyped value access.
     */
    class XPathItem {
    public:
        virtual ~XPathItem() = default;

        /** @return true if this item is a node (always true in this runtime's XPath subset). */
        [[nodiscard]] virtual bool getIsNodeProperty() const { return true; }

        /** @return The string value of this item. */
        [[nodiscard]] virtual std::string getValueProperty() const = 0;
    };

} // namespace System::Xml::XPath

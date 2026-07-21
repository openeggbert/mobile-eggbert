// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include "System/Xml/XmlAttribute.hpp"
#include "System/Xml/XmlNamedNodeMap.hpp"

namespace System::Xml {

    /**
     * @brief An ordered collection of an element's attributes.
     *
     * C++ counterpart of .NET System.Xml.XmlAttributeCollection. A fresh collection reflecting
     * the owning element's current attributes is produced each time XmlElement::Attributes() is
     * called; individual XmlAttribute* identities remain stable via XmlElement's attribute cache
     * (see XmlElement's doc-comment), even though the collection object itself is not cached.
     */
    class XmlAttributeCollection : public XmlNamedNodeMap {
        XmlElement* owner_;

    public:
        explicit XmlAttributeCollection(XmlElement* owner, std::vector<XmlNode*> attrs)
            : owner_(owner) { items_ = std::move(attrs); }

        /** @return The attribute at @p i. */
        [[nodiscard]] XmlAttribute* operator[](SharpRuntime::intcs i) const {
            return static_cast<XmlAttribute*>(Item(i));
        }
        /** @return The attribute named @p name, or nullptr if none. */
        [[nodiscard]] XmlAttribute* operator[](const std::string& name) const {
            return static_cast<XmlAttribute*>(GetNamedItem(name));
        }

        /** @brief Sets or replaces the attribute matching @p node's name. @return The replaced node, or nullptr if none. */
        XmlNode* SetNamedItem(XmlNode* node);
        /** @brief Removes and returns the attribute named @p name, or nullptr if none. */
        XmlNode* RemoveNamedItem(const std::string& name);

        /** @brief Inserts @p node as the first attribute. @return @p node. */
        XmlAttribute* Prepend(XmlAttribute* node);
        /** @brief Inserts @p node as the last attribute. @return @p node. */
        XmlAttribute* Append(XmlAttribute* node);
        /** @brief Inserts @p newNode immediately before @p refNode (or at the end if @p refNode is nullptr). @return @p newNode. */
        XmlAttribute* InsertBefore(XmlAttribute* newNode, XmlAttribute* refNode);
        /** @brief Inserts @p newNode immediately after @p refNode (or at the start if @p refNode is nullptr). @return @p newNode. */
        XmlAttribute* InsertAfter(XmlAttribute* newNode, XmlAttribute* refNode);
        /** @brief Removes @p node from this collection. @return @p node, or nullptr if it wasn't a member. */
        XmlAttribute* Remove(XmlAttribute* node);
        /** @brief Removes the attribute at index @p i. @return The removed attribute, or nullptr if out of range. */
        XmlAttribute* RemoveAt(SharpRuntime::intcs i);
        /** @brief Removes all attributes. */
        void RemoveAll();
    };

} // namespace System::Xml

// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <functional>
#include "System/Xml/Linq/XObjectChangeEventArgs.hpp"
#include "System/Xml/XmlNodeType.hpp"

namespace System::Xml::Linq {

    class XContainer;
    class XElement;
    class XDocument;

    /**
     * @brief Represents the method that handles the XObject::Changed/Changing events.
     *
     * C++ counterpart of .NET's `EventHandler<XObjectChangeEventArgs>` as used by XObject.
     */
    using XObjectChangeEventHandler = std::function<void(void* sender, const XObjectChangeEventArgs& e)>;

    /**
     * @brief Represents a node or an attribute in an XML tree. Abstract base of the whole
     * System.Xml.Linq node hierarchy (XNode, XContainer, XElement, XDocument, ...) and of XAttribute.
     *
     * C++ counterpart of .NET System.Xml.Linq.XObject.
     *
     * @note Deliberately out of scope for this port (documented, not silent):
     * - Annotations (AddAnnotation/Annotation/Annotations/RemoveAnnotations) — .NET's generic
     *   per-object `object?` annotation bag has no clean C++ equivalent without reflection/RTTI
     *   plumbing this runtime otherwise avoids (see CLAUDE.md's reflection policy); ported game
     *   code realistically never depends on it.
     * - BaseUri / IXmlLineInfo (HasLineInfo/LineNumber/LinePosition) — depend on the annotation
     *   system above; LoadOptions::SetBaseUri/SetLineInfo already document that they're no-ops.
     * - Changed/Changing events — real change notification would require every mutating method
     *   in the whole hierarchy to walk up the tree and invoke handlers; this port exposes the
     *   add/remove accessors for API compatibility but they register nothing, matching this
     *   codebase's existing no-op event-accessor convention (e.g. NetworkChange).
     */
    class XObject {
        friend class XContainer;

    protected:
        /** Nearest containing XContainer (XElement or XDocument), or nullptr if this object is detached/root. Non-owning. */
        XContainer* parent_ = nullptr;

    public:
        XObject() = default;
        virtual ~XObject() = default;

        // Nodes/containers are always used via shared_ptr and track a raw parent_ back-pointer;
        // value-copying an XObject would silently duplicate that back-pointer without updating
        // the (former) parent's child list, so copy/move are disabled hierarchy-wide. XAttribute
        // provides its own explicit copy constructor (which starts the copy detached, not a
        // base-class copy) for the one case that needs it (see XAttribute's doc comment).
        XObject(const XObject&) = delete;
        XObject& operator=(const XObject&) = delete;
        XObject(XObject&&) = delete;
        XObject& operator=(XObject&&) = delete;

        /** @return The node type of this object. */
        [[nodiscard]] virtual System::Xml::XmlNodeType getNodeTypeProperty() const = 0;

        /**
         * @return The parent XElement of this object, or nullptr if this object has no parent
         * or its parent is an XDocument (matches .NET: `parent as XElement`).
         */
        [[nodiscard]] XElement* getParentProperty() const;

        /** @return The XDocument that owns this object (walking up to the root), or nullptr if the root is not an XDocument. */
        [[nodiscard]] XDocument* getDocumentProperty() const;

        /**
         * @brief Stub — event registration is not functional; provided for API compatibility (see class doc-comment).
         * C++ counterpart of .NET XObject.Changed event add accessor.
         */
        void add_Changed(const XObjectChangeEventHandler& /*handler*/) {}
        /** @brief Stub — see add_Changed. C++ counterpart of .NET XObject.Changed event remove accessor. */
        void remove_Changed(const XObjectChangeEventHandler& /*handler*/) {}
        /** @brief Stub — event registration is not functional; provided for API compatibility (see class doc-comment). */
        void add_Changing(const XObjectChangeEventHandler& /*handler*/) {}
        /** @brief Stub — see add_Changing. */
        void remove_Changing(const XObjectChangeEventHandler& /*handler*/) {}
    };

} // namespace System::Xml::Linq

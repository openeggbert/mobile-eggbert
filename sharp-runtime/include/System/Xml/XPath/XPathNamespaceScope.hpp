// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Xml::XPath {

    /**
     * @brief Specifies the scope of namespaces returned by XPathNavigator::MoveToFirstNamespace /
     * MoveToNextNamespace (the namespace axis).
     *
     * C++ counterpart of .NET System.Xml.XPath.XPathNamespaceScope. Distinct from (but
     * numerically identical in meaning to) System::Xml::XmlNamespaceScope, which is used by
     * IXmlNamespaceResolver::GetNamespacesInScope instead — matching the two separate, same-
     * shaped enums that exist in real .NET.
     */
    enum class XPathNamespaceScope {
        /** All namespaces in scope, including the implicit "xml" prefix. */
        All,
        /** All namespaces in scope, excluding the implicit "xml" prefix. */
        ExcludeXml,
        /** Only namespaces declared directly on the current element. */
        Local,
    };

} // namespace System::Xml::XPath

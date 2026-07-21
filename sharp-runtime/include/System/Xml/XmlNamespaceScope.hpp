// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Xml {

    /**
     * @brief Specifies the scope of namespaces returned by XmlNamespaceManager::GetNamespacesInScope.
     *
     * C++ counterpart of .NET System.Xml.XmlNamespaceScope.
     */
    enum class XmlNamespaceScope {
        /** All namespaces in scope, including the implicit "xml" prefix. */
        All = 0,
        /** All namespaces in scope, excluding the implicit "xml" prefix. */
        ExcludeXml = 1,
        /** Only namespaces declared directly on the current scope. */
        Local = 2,
    };

} // namespace System::Xml

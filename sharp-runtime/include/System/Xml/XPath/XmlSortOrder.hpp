// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Xml::XPath {

    /**
     * @brief Specifies the sort order used by XPathExpression::AddSort.
     *
     * C++ counterpart of .NET System.Xml.XPath.XmlSortOrder.
     */
    enum class XmlSortOrder {
        /** Sort in ascending order. */
        Ascending = 1,
        /** Sort in descending order. */
        Descending = 2,
    };

} // namespace System::Xml::XPath

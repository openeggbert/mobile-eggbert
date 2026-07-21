// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Xml::XPath {

    /**
     * @brief Describes the return type of an XPath expression.
     *
     * C++ counterpart of .NET System.Xml.XPath.XPathResultType. Numeric values match .NET
     * exactly, including the historical quirk that Navigator shares String's value (1) — .NET
     * never actually produces Navigator from expression evaluation, it is XPathResultType's
     * unused/reserved member.
     */
    enum class XPathResultType {
        /** A numeric (double) result. */
        Number = 0,
        /** A string result. */
        String = 1,
        /** A boolean result. */
        Boolean = 2,
        /** A node-set result. */
        NodeSet = 3,
        /** Reserved; shares String's numeric value (1) in real .NET and is never produced. */
        Navigator = 1,
        /** The result type could not be determined statically (e.g. an unrecognized function). */
        Any = 5,
        /** The expression is invalid. */
        Error = 6,
    };

} // namespace System::Xml::XPath

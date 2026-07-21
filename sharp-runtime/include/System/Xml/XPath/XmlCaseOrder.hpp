// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Xml::XPath {

    /**
     * @brief Specifies how to order nodes whose sort keys are equal ignoring case, used by
     * XPathExpression::AddSort.
     *
     * C++ counterpart of .NET System.Xml.XPath.XmlCaseOrder.
     *
     * @note This runtime accepts and stores XmlCaseOrder but does not special-case it: string
     * sort-key comparisons are always ordinal (case-sensitive). Documented gap, not a silent one.
     */
    enum class XmlCaseOrder {
        /** No special case ordering. */
        None = 0,
        /** Uppercase sorts before lowercase. */
        UpperFirst = 1,
        /** Lowercase sorts before uppercase. */
        LowerFirst = 2,
    };

} // namespace System::Xml::XPath

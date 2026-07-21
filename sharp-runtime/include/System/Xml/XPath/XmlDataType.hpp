// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Xml::XPath {

    /**
     * @brief Specifies how sort keys are compared, used by XPathExpression::AddSort.
     *
     * C++ counterpart of .NET System.Xml.XPath.XmlDataType.
     */
    enum class XmlDataType {
        /** Sort keys are compared as strings. */
        Text = 1,
        /** Sort keys are compared as numbers. */
        Number = 2,
    };

} // namespace System::Xml::XPath

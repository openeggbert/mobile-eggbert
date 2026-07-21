// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <string>

#include "System/SystemException.hpp"

namespace System::Xml::XPath {

    /**
     * @brief The exception that is thrown when there is an error processing an XPath expression.
     *
     * C++ counterpart of .NET System.Xml.XPath.XPathException.
     */
    class XPathException : public System::SystemException {
    public:
        /** @brief Initializes a new instance with a default message. */
        XPathException();
        /** @brief Initializes a new instance with the specified error message. */
        explicit XPathException(const std::string& message);
        /** @brief Initializes a new instance with the specified message and inner exception. */
        XPathException(const std::string& message, std::exception_ptr innerException);
    };

} // namespace System::Xml::XPath

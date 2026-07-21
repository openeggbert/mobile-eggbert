// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Xml/XPath/XPathException.hpp"

#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Xml::XPath {

    namespace {
        constexpr const char* DefaultMsg = "An XPath error has occurred.";
        constexpr SharpRuntime::intcs HResultXmlXPath = static_cast<SharpRuntime::intcs>(0x80131943); // HResults.XmlXPath
    }

    XPathException::XPathException()
        : SystemException(DefaultMsg) {
        setHResultProperty(HResultXmlXPath);
    }

    XPathException::XPathException(const std::string& message)
        : SystemException(message) {
        setHResultProperty(HResultXmlXPath);
    }

    XPathException::XPathException(const std::string& message, std::exception_ptr innerException)
        : SystemException(message, std::move(innerException)) {
        setHResultProperty(HResultXmlXPath);
    }

} // namespace System::Xml::XPath

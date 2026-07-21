// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Xml/XmlException.hpp"

namespace System::Xml {

    namespace {
        constexpr const char* DefaultMsg = "An XML error has occurred.";
        constexpr SharpRuntime::intcs HResultXml = static_cast<SharpRuntime::intcs>(0x80131940); // HResults.Xml
    }

    XmlException::XmlException()
        : SystemException(DefaultMsg) {
        setHResultProperty(HResultXml);
    }

    XmlException::XmlException(const std::string& message)
        : SystemException(message) {
        setHResultProperty(HResultXml);
    }

    XmlException::XmlException(const std::string& message, std::exception_ptr inner)
        : SystemException(message, std::move(inner)) {
        setHResultProperty(HResultXml);
    }

    XmlException::XmlException(const std::string& message, std::exception_ptr inner,
                                SharpRuntime::intcs lineNumber, SharpRuntime::intcs linePosition)
        : SystemException(message, std::move(inner)), lineNumber_(lineNumber), linePosition_(linePosition) {
        setHResultProperty(HResultXml);
    }

    XmlException::XmlException(const std::string& message, SharpRuntime::intcs lineNumber, SharpRuntime::intcs linePosition)
        : SystemException(message), lineNumber_(lineNumber), linePosition_(linePosition) {
        setHResultProperty(HResultXml);
    }

    XmlException::XmlException(const std::string& message, SharpRuntime::intcs lineNumber,
                                SharpRuntime::intcs linePosition, const std::string& sourceUri)
        : SystemException(message), lineNumber_(lineNumber), linePosition_(linePosition), sourceUri_(sourceUri) {
        setHResultProperty(HResultXml);
    }

} // namespace System::Xml

// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <string>

#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/SystemException.hpp"

namespace System::Xml {

    /**
     * @brief The exception thrown when an error occurs while parsing or processing XML.
     *
     * C++ counterpart of .NET System.Xml.XmlException.
     */
    class XmlException : public System::SystemException {
        SharpRuntime::intcs lineNumber_ = 0;
        SharpRuntime::intcs linePosition_ = 0;
        std::string sourceUri_;

    public:
        /** @brief Initializes a new instance with a default message. */
        XmlException();
        /** @brief Initializes a new instance with the specified error message. */
        explicit XmlException(const std::string& message);
        /** @brief Initializes a new instance with the specified message and inner exception. */
        XmlException(const std::string& message, std::exception_ptr inner);
        /** @brief Initializes a new instance with the specified message, inner exception, and source position. */
        XmlException(const std::string& message, std::exception_ptr inner,
                     SharpRuntime::intcs lineNumber, SharpRuntime::intcs linePosition);
        /** @brief Initializes a new instance with the specified message and source position (no inner exception, no source URI). */
        XmlException(const std::string& message, SharpRuntime::intcs lineNumber, SharpRuntime::intcs linePosition);
        /** @brief Initializes a new instance with message, source position, and source URI. */
        XmlException(const std::string& message, SharpRuntime::intcs lineNumber,
                     SharpRuntime::intcs linePosition, const std::string& sourceUri);

        /** @return The line number where the error occurred, or 0 if unavailable. */
        [[nodiscard]] SharpRuntime::intcs getLineNumberProperty() const { return lineNumber_; }
        /** @return The line position where the error occurred, or 0 if unavailable. */
        [[nodiscard]] SharpRuntime::intcs getLinePositionProperty() const { return linePosition_; }
        /** @return The URI of the input that caused the error, or an empty string if unavailable. */
        [[nodiscard]] const std::string& getSourceUriProperty() const { return sourceUri_; }
    };

} // namespace System::Xml

// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <optional>
#include <string>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Exception.hpp"

namespace System::Text::Json {

    using SharpRuntime::longcs;

    /**
     * @brief The exception thrown when invalid JSON text is encountered, when the defined maximum
     * depth is passed, or the JSON text is not compatible with the type of a property on an object.
     *
     * C++ counterpart of .NET System.Text.Json.JsonException.
     */
    class JsonException : public System::Exception {
        std::optional<longcs> lineNumber_;
        std::optional<longcs> bytePositionInLine_;
        std::optional<std::string> path_;

    public:
        JsonException() = default;
        explicit JsonException(const std::string& message) : System::Exception(message) {}
        JsonException(const std::string& message, std::exception_ptr innerException)
            : System::Exception(message, innerException) {}
        JsonException(const std::string& message, const std::optional<std::string>& path,
                       std::optional<longcs> lineNumber, std::optional<longcs> bytePositionInLine)
            : System::Exception(message), lineNumber_(lineNumber), bytePositionInLine_(bytePositionInLine),
              path_(path) {}
        JsonException(const std::string& message, const std::optional<std::string>& path,
                       std::optional<longcs> lineNumber, std::optional<longcs> bytePositionInLine,
                       std::exception_ptr innerException)
            : System::Exception(message, innerException), lineNumber_(lineNumber),
              bytePositionInLine_(bytePositionInLine), path_(path) {}

        /** @return The number of lines read so far before the exception (starting at 0), if known. */
        [[nodiscard]] std::optional<longcs> getLineNumberProperty() const { return lineNumber_; }
        /** @return The number of bytes read within the current line before the exception (starting at 0), if known. */
        [[nodiscard]] std::optional<longcs> getBytePositionInLineProperty() const { return bytePositionInLine_; }
        /** @return The path within the JSON where the exception was encountered, if known. */
        [[nodiscard]] std::optional<std::string> getPathProperty() const { return path_; }

        void setLineNumberProperty(std::optional<longcs> v) { lineNumber_ = v; }
        void setBytePositionInLineProperty(std::optional<longcs> v) { bytePositionInLine_ = v; }
        void setPathProperty(std::optional<std::string> v) { path_ = v; }
    };

} // namespace System::Text::Json

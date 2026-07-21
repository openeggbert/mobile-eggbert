// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/Attribute.hpp"

namespace System {

    /** Marks a program element as obsolete, mirroring .NET System.ObsoleteAttribute. */
    class ObsoleteAttribute : public Attribute {
        std::string message_;
        bool isError_ = false;
        std::string diagnosticId_;
        std::string urlFormat_;

    public:
        /** Constructs an ObsoleteAttribute with an empty message and isError = false. */
        ObsoleteAttribute() = default;
        /**
         * @brief Constructs an ObsoleteAttribute with the given informational message.
         * @param message Human-readable explanation of the obsolescence.
         */
        explicit ObsoleteAttribute(const std::string& message) : message_(message) {}
        /**
         * @brief Constructs an ObsoleteAttribute with a message and an error flag.
         * @param message Human-readable explanation of the obsolescence.
         * @param isError If true, using the marked element is a compile-time error.
         */
        ObsoleteAttribute(const std::string& message, bool isError) : message_(message), isError_(isError) {}

        /** Returns the informational message describing the obsolescence. */
        [[nodiscard]] const std::string& getMessageProperty()      const { return message_; }
        /** Returns true if usage of the marked element is treated as a compile error. */
        [[nodiscard]] bool               getIsErrorProperty()      const { return isError_; }
        /** Returns the optional diagnostic ID associated with this obsolescence. */
        [[nodiscard]] const std::string& getDiagnosticIdProperty() const { return diagnosticId_; }
        /** Returns the optional URL format string for further documentation. */
        [[nodiscard]] const std::string& getUrlFormatProperty()    const { return urlFormat_; }

        /** Sets the diagnostic ID associated with this obsolescence. */
        void setDiagnosticIdProperty(const std::string& v) { diagnosticId_ = v; }
        /** Sets the URL format string pointing to further documentation. */
        void setUrlFormatProperty(const std::string& v)    { urlFormat_ = v; }
    };

} // namespace System

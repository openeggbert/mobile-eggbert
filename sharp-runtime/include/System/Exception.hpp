// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Created by robertvokac on 5/26/25.
//

#pragma once

#include <exception>
#include <map>
#include <memory>
#include <string>
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System {

    /**
     * @brief Represents the base class for application exceptions.
     *
     * C++ reimplementation of the .NET System.Exception type.
     * Stores an error message and exposes it both through the std::exception
     * what() interface and the .NET-like getMessageProperty() accessor.
     *
     * GetBaseException() and ToString() from the .NET surface are intentionally not provided:
     * GetBaseException() would require cloning `*this` through the base class, which slices any
     * derived exception type back down to plain Exception (there is no virtual-clone mechanism in
     * this port); ToString() would require a GetType()-equivalent class name, which is out of scope
     * per the project's no-reflection deviation. TargetSite and GetObjectData are out of scope for
     * the same reflection/serialization deviations.
     */
    class Exception : public std::exception {
    private:
        std::string message_;
        std::exception_ptr innerException_;
        mutable std::map<std::string, std::string> data_;
        std::string source_;
        std::string helpLink_;
        SharpRuntime::intcs hResult_ = static_cast<SharpRuntime::intcs>(0x80131500u); // COR_E_EXCEPTION

    public:
        /** @brief Initializes a new instance of the Exception class with an empty message. */
        Exception();

        ~Exception() override = default;

        /**
         * @brief Initializes a new instance with the specified C-string message.
         * @param msg A null-terminated string describing the error; nullptr stores an empty message.
         */
        explicit Exception(const char* msg);

        /**
         * @brief Initializes a new instance with the specified message.
         * @param msg A string describing the error.
         */
        explicit Exception(const std::string& msg);

        /**
         * @brief Initializes a new instance with a message and a reference to an inner exception.
         *
         * C++ counterpart of .NET Exception(string, Exception).
         */
        Exception(const std::string& msg, std::exception_ptr inner);

        /**
         * @brief Gets the explanatory message associated with this exception.
         *
         * C++ counterpart of .NET Exception.Message.
         * @return Const reference to the stored error message string.
         */
        [[nodiscard]] virtual const std::string& getMessageProperty() const;

        /**
         * @brief Returns the exception that caused the current exception.
         *
         * C++ counterpart of .NET Exception.InnerException.
         * @return The inner exception pointer, or nullptr if none.
         */
        [[nodiscard]] std::exception_ptr getInnerExceptionProperty() const;

        /**
         * @brief Returns an empty string.
         *
         * C++ counterpart of .NET Exception.StackTrace.
         * Stack traces are not captured in C++ exceptions.
         */
        [[nodiscard]] const std::string& getStackTraceProperty() const;

        /**
         * @brief Returns a mutable key/value collection of additional user-defined data.
         *
         * C++ counterpart of .NET Exception.Data.
         */
        [[nodiscard]] std::map<std::string, std::string>& getDataProperty();
        [[nodiscard]] const std::map<std::string, std::string>& getDataProperty() const;

        /**
         * @brief Gets or sets a link to the help file associated with this exception.
         *
         * C++ counterpart of .NET Exception.HelpLink.
         */
        [[nodiscard]] virtual const std::string& getHelpLinkProperty() const;
        virtual void setHelpLinkProperty(const std::string& value);

        /**
         * @brief Gets or sets the name of the application or object that caused the error.
         *
         * C++ counterpart of .NET Exception.Source.
         */
        [[nodiscard]] virtual const std::string& getSourceProperty() const;
        virtual void setSourceProperty(const std::string& value);

        /**
         * @brief Gets or sets a coded numerical value assigned to this exception.
         *
         * C++ counterpart of .NET Exception.HResult. Defaults to COR_E_EXCEPTION (0x80131500),
         * matching .NET's default for the base Exception type.
         */
        [[nodiscard]] SharpRuntime::intcs getHResultProperty() const;
        void setHResultProperty(SharpRuntime::intcs value);

        /**
         * @brief Returns the explanatory message as a null-terminated C string.
         *
         * The pointer remains valid for the lifetime of the exception object.
         */
        [[nodiscard]] const char* what() const noexcept override;
    };

} // namespace System

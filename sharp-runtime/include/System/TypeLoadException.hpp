// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <exception>
#include <string>
#include "System/SystemException.hpp"

namespace System {

/**
 * @brief The exception that is thrown when type-loading failures occur.
 *
 * C++ counterpart of .NET System.TypeLoadException.
 * Stores an optional type name that identifies which type failed to load.
 */
class TypeLoadException : public SystemException {
public:
    /**
     * @brief Initializes a new instance of TypeLoadException with a default message.
     *
     * C++ counterpart of .NET TypeLoadException().
     */
    TypeLoadException()
        : SystemException("Failure has occurred while loading a type.") {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131522)); // COR_E_TYPELOAD
    }

    /**
     * @brief Initializes a new instance of TypeLoadException with a specified error message.
     *
     * C++ counterpart of .NET TypeLoadException(string).
     * @param message The message that describes the error.
     */
    explicit TypeLoadException(const char* message)
        : SystemException(message) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131522)); // COR_E_TYPELOAD
    }

    /**
     * @brief Initializes a new instance of TypeLoadException with a specified error message.
     *
     * C++ counterpart of .NET TypeLoadException(string).
     * @param message The message that describes the error.
     */
    explicit TypeLoadException(const std::string& message)
        : SystemException(message) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131522)); // COR_E_TYPELOAD
    }

    /**
     * @brief Initializes a new instance of TypeLoadException with a message and inner exception.
     *
     * C++ counterpart of .NET TypeLoadException(string, Exception).
     * @param message The error message.
     * @param inner   The exception that is the cause of the current exception.
     */
    TypeLoadException(const std::string& message, std::exception_ptr inner)
        : SystemException(message, std::move(inner)) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131522)); // COR_E_TYPELOAD
    }

    /**
     * @brief Initializes a new instance of TypeLoadException with a message and type name.
     *
     * C++ counterpart of the internal .NET TypeLoadException(string, string) constructor.
     * @param message  The error message.
     * @param typeName The fully qualified name of the type that failed to load.
     */
    TypeLoadException(const std::string& message, const std::string& typeName)
        : SystemException(message), typeName_(typeName) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131522)); // COR_E_TYPELOAD
    }

    /**
     * @brief Gets the fully qualified name of the type that caused the exception.
     *
     * C++ counterpart of .NET TypeLoadException.TypeName.
     * @return The type name, or an empty string if none was provided.
     */
    [[nodiscard]] std::string getTypeNameProperty() const { return typeName_; }

private:
    std::string typeName_;
};

} // namespace System

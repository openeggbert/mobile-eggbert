// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Created by robertvokac on 6/7/25.
//

#include "System/ObjectDisposedException.hpp"

namespace System {

    namespace {
        constexpr const char* DefaultObjectDisposedMessage = "Cannot access a disposed object.";
    }

    std::string ObjectDisposedException::BuildMessage(const std::string& objectName, const std::string& message)
    {
        if (objectName.empty()) {
            return message;
        }

        return message + "\nObject name: '" + objectName + "'.";
    }

    ObjectDisposedException::ObjectDisposedException()
        : InvalidOperationException(DefaultObjectDisposedMessage),
          objectName_("") {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131622)); // COR_E_OBJECTDISPOSED
    }

    ObjectDisposedException::ObjectDisposedException(const char* objectName)
        : InvalidOperationException(BuildMessage(objectName != nullptr ? std::string(objectName) : std::string(),
                                                 DefaultObjectDisposedMessage)),
          objectName_(objectName != nullptr ? objectName : "") {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131622)); // COR_E_OBJECTDISPOSED
    }

    ObjectDisposedException::ObjectDisposedException(const std::string& objectName)
        : InvalidOperationException(BuildMessage(objectName, DefaultObjectDisposedMessage)),
          objectName_(objectName) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131622)); // COR_E_OBJECTDISPOSED
    }

    ObjectDisposedException::ObjectDisposedException(const char* objectName, const char* message)
        : InvalidOperationException(BuildMessage(objectName != nullptr ? std::string(objectName) : std::string(),
                                                 message != nullptr ? std::string(message) : std::string())),
          objectName_(objectName != nullptr ? objectName : "") {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131622)); // COR_E_OBJECTDISPOSED
    }

    ObjectDisposedException::ObjectDisposedException(const std::string& objectName, const std::string& message)
        : InvalidOperationException(BuildMessage(objectName, message)),
          objectName_(objectName) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131622)); // COR_E_OBJECTDISPOSED
    }

    ObjectDisposedException::ObjectDisposedException(const std::string& message, std::exception_ptr inner)
        : InvalidOperationException(message, std::move(inner)),
          objectName_("") {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131622)); // COR_E_OBJECTDISPOSED
    }

    const std::string& ObjectDisposedException::getObjectNameProperty() const
    {
        return objectName_;
    }

    void ObjectDisposedException::ThrowIf(bool condition, const char* objectName)
    {
        if (condition) {
            throw ObjectDisposedException(objectName);
        }
    }

    void ObjectDisposedException::ThrowIf(bool condition, const std::string& objectName)
    {
        if (condition) {
            throw ObjectDisposedException(objectName);
        }
    }

} // namespace System
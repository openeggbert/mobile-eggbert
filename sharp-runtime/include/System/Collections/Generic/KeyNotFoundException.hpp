// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/SystemException.hpp"

namespace System::Collections::Generic {

/**
 * @brief The exception that is thrown when the key specified for accessing an element
 *        in a collection does not match any key in the collection.
 *
 * C++ counterpart of .NET System.Collections.Generic.KeyNotFoundException.
 */
class KeyNotFoundException : public System::SystemException {
public:
    /**
     * @brief Initializes a new instance with the default message.
     *
     * C++ counterpart of .NET KeyNotFoundException().
     */
    KeyNotFoundException()
        : SystemException("The given key was not present in the dictionary.") {}

    /**
     * @brief Initializes a new instance with the specified error message.
     *
     * C++ counterpart of .NET KeyNotFoundException(string).
     * @param message A message that describes the error.
     */
    explicit KeyNotFoundException(const std::string& message)
        : SystemException(message) {}

    /**
     * @brief Initializes a new instance with a specified error message and a reference
     *        to the inner exception that is the cause of this exception.
     *
     * C++ counterpart of .NET KeyNotFoundException(string, Exception).
     * @param message A message that describes the error.
     * @param inner   The exception that is the cause of the current exception.
     */
    KeyNotFoundException(const std::string& message, std::exception_ptr inner)
        : SystemException(message, std::move(inner)) {}
};

} // namespace System::Collections::Generic

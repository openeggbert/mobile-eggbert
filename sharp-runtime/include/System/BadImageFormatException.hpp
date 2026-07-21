// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/SystemException.hpp"

namespace System {

/**
 * @brief The exception that is thrown when the file image of a dynamic-link
 * library (DLL) or an executable program is invalid.
 *
 * C++ counterpart of .NET System.BadImageFormatException.
 */
class BadImageFormatException : public SystemException {
    std::string fileName_;
public:
    /**
     * @brief Initializes a new instance with the default invalid-image-format message.
     *
     * C++ counterpart of .NET BadImageFormatException().
     */
    BadImageFormatException()
        : SystemException("Format of the executable (.exe) or library (.dll) is invalid.") {}

    /**
     * @brief Initializes a new instance with the specified message.
     *
     * C++ counterpart of .NET BadImageFormatException(string).
     * @param message The error message.
     */
    explicit BadImageFormatException(const std::string& message)
        : SystemException(message) {}

    /**
     * @brief Initializes a new instance with a message and an inner exception.
     *
     * C++ counterpart of .NET BadImageFormatException(string, Exception).
     * @param message The error message.
     * @param inner   The exception that is the cause of this exception.
     */
    BadImageFormatException(const std::string& message, std::exception_ptr inner)
        : SystemException(message, std::move(inner)) {}

    /**
     * @brief Initializes a new instance with a message and a file name.
     *
     * C++ counterpart of .NET BadImageFormatException(string, string).
     * @param message  The error message.
     * @param fileName The name of the file with the invalid image.
     */
    BadImageFormatException(const std::string& message, const std::string& fileName)
        : SystemException(message), fileName_(fileName) {}

    /**
     * @brief Initializes a new instance with a message, file name, and inner exception.
     *
     * C++ counterpart of .NET BadImageFormatException(string, string, Exception).
     * @param message  The error message.
     * @param fileName The name of the file with the invalid image.
     * @param inner    The exception that is the cause of this exception.
     */
    BadImageFormatException(const std::string& message, const std::string& fileName,
                            std::exception_ptr inner)
        : SystemException(message, std::move(inner)), fileName_(fileName) {}

    /**
     * @brief Gets the name of the file that caused this exception.
     *
     * C++ counterpart of .NET BadImageFormatException.FileName.
     * @return The file name, or empty string if none was provided.
     */
    [[nodiscard]] const std::string& getFileNameProperty() const noexcept { return fileName_; }

    /**
     * @brief Gets the log file that describes why assembly loading failed.
     *
     * C++ counterpart of .NET BadImageFormatException.FusionLog.
     * Always returns empty string — fusion logging is not available outside the CLR.
     */
    [[nodiscard]] std::string getFusionLogProperty() const { return {}; }
};

} // namespace System

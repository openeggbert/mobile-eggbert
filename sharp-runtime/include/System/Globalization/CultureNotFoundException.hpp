// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <cstdio>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentException.hpp"

namespace System::Globalization {

    using SharpRuntime::intcs;

/**
 * @brief The exception thrown when a culture identifier is not available on the current platform.
 *
 * C++ counterpart of .NET System.Globalization.CultureNotFoundException.
 * Extends ArgumentException to carry the invalid culture name or ID
 * that triggered the exception.
 */
class CultureNotFoundException : public System::ArgumentException {
    std::string invalidCultureName_;
    intcs invalidCultureId_ = -1;

    /** @brief Formats an LCID as real .NET's "{0} (0x{0:x4})" -- e.g. 99 -> "99 (0x0063)". */
    static std::string formatCultureId(intcs id) {
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%d (0x%04x)", id, id);
        return buf;
    }

    /**
     * @brief Composes the full exception message, matching real .NET's Message override:
     * base message (with the "(Parameter 'x')" marker already appended, if @p paramName is
     * non-empty) followed by a newline and "{value} is an invalid culture identifier.",
     * where {value} is the invalid culture name or a formatted LCID.
     */
    static std::string composeMessage(const std::string& message, const std::string& paramName,
                                       const std::string& invalidCultureValue) {
        std::string base = paramName.empty() ? message : AppendParamNameSuffix(message, paramName);
        return base + "\n" + invalidCultureValue + " is an invalid culture identifier.";
    }

public:
    /**
     * @brief Constructs a CultureNotFoundException with a default message.
     *
     * C++ counterpart of .NET CultureNotFoundException().
     */
    CultureNotFoundException() : ArgumentException("Culture is not supported.") {}

    /**
     * @brief Constructs a CultureNotFoundException with the specified message.
     *
     * C++ counterpart of .NET CultureNotFoundException(string).
     * @param message The error message.
     */
    explicit CultureNotFoundException(const std::string& message)
        : ArgumentException(message) {}

    /**
     * @brief Constructs a CultureNotFoundException with a parameter name and message.
     *
     * C++ counterpart of .NET CultureNotFoundException(string paramName, string message).
     * @param paramName The name of the parameter that caused the exception.
     * @param message   The error message.
     */
    CultureNotFoundException(const std::string& paramName, const std::string& message)
        : ArgumentException(message, paramName) {}

    /**
     * @brief Constructs a CultureNotFoundException with a message and inner exception.
     *
     * C++ counterpart of .NET CultureNotFoundException(string, Exception).
     * @param message The error message.
     * @param inner   The inner exception.
     */
    CultureNotFoundException(const std::string& message, std::exception_ptr inner)
        : ArgumentException(message, std::move(inner)) {}

    /**
     * @brief Constructs a CultureNotFoundException with a parameter name, invalid culture name, and message.
     *
     * C++ counterpart of .NET CultureNotFoundException(string paramName, string invalidCultureName, string message).
     * @param paramName          The name of the parameter that caused the exception.
     * @param invalidCultureName The culture name that could not be found.
     * @param message            The error message.
     */
    CultureNotFoundException(const std::string& paramName, const std::string& invalidCultureName,
                             const std::string& message)
        : ArgumentException(composeMessage(message, paramName, invalidCultureName), paramName,
                             AlreadyComposedTag{}),
          invalidCultureName_(invalidCultureName) {}

    /**
     * @brief Constructs a CultureNotFoundException with a message, invalid culture name, and inner exception.
     *
     * C++ counterpart of .NET CultureNotFoundException(string message, string invalidCultureName, Exception innerException).
     * @param message            The error message.
     * @param invalidCultureName The culture name that could not be found.
     * @param inner              The inner exception.
     */
    CultureNotFoundException(const std::string& message, const std::string& invalidCultureName,
                             std::exception_ptr inner)
        : ArgumentException(composeMessage(message, "", invalidCultureName), std::move(inner)),
          invalidCultureName_(invalidCultureName) {}

    /**
     * @brief Constructs a CultureNotFoundException with a message and invalid LCID.
     *
     * C++ counterpart of .NET CultureNotFoundException(string, int, Exception).
     * @param message          The error message.
     * @param invalidCultureId The LCID that could not be found.
     * @param inner            The inner exception (may be nullptr).
     */
    CultureNotFoundException(const std::string& message, intcs invalidCultureId,
                             std::exception_ptr inner)
        : ArgumentException(composeMessage(message, "", formatCultureId(invalidCultureId)), std::move(inner)),
          invalidCultureId_(invalidCultureId) {}

    /**
     * @brief Constructs a CultureNotFoundException with a param name, invalid LCID, and message.
     *
     * C++ counterpart of .NET CultureNotFoundException(string, int, string).
     * @param paramName        The parameter name.
     * @param invalidCultureId The LCID that could not be found.
     * @param message          The error message.
     */
    CultureNotFoundException(const std::string& paramName, intcs invalidCultureId,
                             const std::string& message)
        : ArgumentException(composeMessage(message, paramName, formatCultureId(invalidCultureId)),
                             paramName, AlreadyComposedTag{}),
          invalidCultureId_(invalidCultureId) {}

    /**
     * @brief Gets the culture name that triggered this exception.
     *
     * C++ counterpart of .NET CultureNotFoundException.InvalidCultureName.
     * @return The invalid culture name, or an empty string if not set.
     */
    [[nodiscard]] const std::string& getInvalidCultureNameProperty() const {
        return invalidCultureName_;
    }

    /**
     * @brief Gets the LCID that triggered this exception.
     *
     * C++ counterpart of .NET CultureNotFoundException.InvalidCultureId.
     * @return The invalid culture ID, or -1 if not set.
     */
    [[nodiscard]] intcs getInvalidCultureIdProperty() const { return invalidCultureId_; }
};

} // namespace System::Globalization

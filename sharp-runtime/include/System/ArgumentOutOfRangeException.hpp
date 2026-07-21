// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <string>
#include "System/ArgumentException.hpp"

namespace System {

/**
 * @brief The exception that is thrown when the value of an argument is outside
 *        the allowable range of values as defined by the invoked method.
 *
 * C++ counterpart of .NET System.ArgumentOutOfRangeException.
 * Derives from ArgumentException and adds an optional actual-value string that
 * identifies the out-of-range value.
 */
class ArgumentOutOfRangeException : public ArgumentException {
public:
    /**
     * @brief Initializes a new instance with the name of the parameter that caused the exception.
     *
     * C++ counterpart of .NET ArgumentOutOfRangeException(string paramName).
     * @param paramName The name of the parameter that caused the exception.
     */
    explicit ArgumentOutOfRangeException(const char* paramName);

    /**
     * @brief Initializes a new instance with the parameter name and a custom message.
     *
     * C++ counterpart of .NET ArgumentOutOfRangeException(string paramName, string message).
     * @param paramName The name of the parameter that caused the exception.
     * @param message   A message that describes the error.
     */
    ArgumentOutOfRangeException(const std::string& paramName,
                                const std::string& message);

    /**
     * @brief Initializes a new instance with a parameter name, actual value, and message.
     *
     * C++ counterpart of .NET ArgumentOutOfRangeException(string, object, string).
     * Matches .NET's Message property override, which appends "Actual value was X."
     * after the "(Parameter 'x')" marker whenever an actual value is supplied.
     * @param paramName   The name of the parameter that caused the exception.
     * @param actualValue The value of the argument that caused the exception (as string).
     * @param message     The error message.
     */
    ArgumentOutOfRangeException(const std::string& paramName,
                                const std::string& actualValue,
                                const std::string& message);

    // -----------------------------------------------------------------------
    // Static guard helpers  (C++ counterparts of .NET ThrowIf* methods)
    // -----------------------------------------------------------------------
    //
    // KNOWN MINOR GAP (audited, not fixed): real .NET's ThrowIfXxx helpers embed the actual
    // value directly in the PRIMARY message text too (e.g. ThrowIfZero's exact resource string
    // is "{paramName} ('{value}') must be a non-zero value."), and ThrowIfNegativeOrZero/
    // ThrowIfEqual/ThrowIfNotEqual use noticeably different prose than the messages below (e.g.
    // "must be a non-negative and non-zero value." vs this port's "must be a positive value.").
    // The information itself is NOT missing here -- paramName is present via
    // getParamNameProperty(), the value via getActualValueProperty() and (after this ticket's
    // fix) the "Actual value was X." message suffix -- this is a pure wording/paraphrase
    // difference in the primary sentence, not a structural gap, so left as-is matching this
    // project's established practice of not chasing verbatim message-text parity for its own
    // sake (see CLAUDE.md's parity philosophy).

    /**
     * @brief Throws ArgumentOutOfRangeException if @p value is negative.
     *
     * C++ counterpart of .NET ArgumentOutOfRangeException.ThrowIfNegative.
     * @tparam T  An arithmetic type.
     * @param value     The value to check.
     * @param paramName The name of the parameter.
     */
    template<typename T>
    static void ThrowIfNegative(T value, const std::string& paramName = "")
    {
        if (value < T{})
            throw ArgumentOutOfRangeException(paramName, std::to_string(value),
                "'" + paramName + "' must be a non-negative value.");
    }

    /**
     * @brief Throws ArgumentOutOfRangeException if @p value is greater than or equal to @p other.
     *
     * C++ counterpart of .NET ArgumentOutOfRangeException.ThrowIfGreaterThanOrEqual.
     * @tparam T  A totally ordered type.
     * @param value     The value to check.
     * @param other     The exclusive upper bound.
     * @param paramName The name of the parameter.
     */
    template<typename T>
    static void ThrowIfGreaterThanOrEqual(T value, T other, const std::string& paramName = "")
    {
        if (value >= other)
            throw ArgumentOutOfRangeException(paramName, std::to_string(value),
                "'" + paramName + "' must be less than " + std::to_string(other) + ".");
    }

    /**
     * @brief Throws ArgumentOutOfRangeException if @p value is less than @p other.
     *
     * C++ counterpart of .NET ArgumentOutOfRangeException.ThrowIfLessThan.
     * @tparam T  A totally ordered type.
     * @param value     The value to check.
     * @param other     The lower bound (exclusive).
     * @param paramName The name of the parameter.
     */
    template<typename T>
    static void ThrowIfLessThan(T value, T other, const std::string& paramName = "")
    {
        if (value < other)
            throw ArgumentOutOfRangeException(paramName, std::to_string(value),
                "'" + paramName + "' must be greater than or equal to " + std::to_string(other) + ".");
    }

private:
    std::string actualValue_;
};

} // namespace System

// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <exception>
#include <string>
#include "System/TimeSpan.hpp"
#include "System/TimeoutException.hpp"

namespace System::Text::RegularExpressions {

    /**
     * @brief Thrown when the execution time of a regular expression pattern-matching method
     * exceeds its time-out interval.
     *
     * C++ counterpart of .NET System.Text.RegularExpressions.RegexMatchTimeoutException.
     *
     * @note This runtime's `std::regex`-backed Regex has no matching time-out mechanism (no way
     * to interrupt `std::regex_search` mid-match), so this type is never actually thrown by
     * Regex itself here — it's reproduced for API compatibility with ported code that catches or
     * constructs it directly.
     */
    class RegexMatchTimeoutException : public System::TimeoutException {
        std::string input_;
        std::string pattern_;
        System::TimeSpan matchTimeout_ = System::TimeSpan::FromTicks(-1);

    public:
        RegexMatchTimeoutException() : System::TimeoutException("The RegEx engine has timed out while trying to match a pattern to an input string.") {}
        explicit RegexMatchTimeoutException(const std::string& message) : System::TimeoutException(message) {}
        RegexMatchTimeoutException(const std::string& message, std::exception_ptr innerException)
            : System::TimeoutException(message, innerException) {}
        RegexMatchTimeoutException(std::string regexInput, std::string regexPattern, System::TimeSpan matchTimeout)
            : System::TimeoutException("The RegEx engine has timed out while trying to match a pattern to an input string."),
              input_(std::move(regexInput)), pattern_(std::move(regexPattern)), matchTimeout_(matchTimeout) {}

        /** @return The input text being processed when the time-out occurred. */
        [[nodiscard]] const std::string& getInputProperty() const { return input_; }
        /** @return The regular expression pattern in use when the time-out occurred. */
        [[nodiscard]] const std::string& getPatternProperty() const { return pattern_; }
        /** @return The configured time-out interval. */
        [[nodiscard]] System::TimeSpan getMatchTimeoutProperty() const { return matchTimeout_; }
    };

} // namespace System::Text::RegularExpressions

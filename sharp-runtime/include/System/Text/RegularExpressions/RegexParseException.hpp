// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include <string>
#include "System/ArgumentException.hpp"
#include "System/Text/RegularExpressions/RegexParseError.hpp"

namespace System::Text::RegularExpressions {

    /**
     * @brief Thrown when a regular expression parsing error occurs.
     *
     * C++ counterpart of .NET System.Text.RegularExpressions.RegexParseException, which
     * additionally carries an `Offset` (the zero-based character offset in the pattern where
     * the parse error occurred). This runtime is backed by `std::regex`, whose
     * `std::regex_error` does not expose a comparable position, so `Offset` defaults to 0
     * unless the caller constructing this exception knows the real offset.
     */
    class RegexParseException : public System::ArgumentException {
        RegexParseError error_;
        SharpRuntime::intcs offset_;

    public:
        RegexParseException(RegexParseError error, const std::string& message, SharpRuntime::intcs offset = 0)
            : ArgumentException(message), error_(error), offset_(offset) {}

        /** @return The specific reason parsing failed. */
        [[nodiscard]] RegexParseError getErrorProperty() const { return error_; }

        /**
         * @brief The zero-based character offset in the pattern where the parse error
         * occurred, or 0 if unknown (see class doc comment).
         */
        [[nodiscard]] SharpRuntime::intcs getOffsetProperty() const { return offset_; }
    };

} // namespace System::Text::RegularExpressions

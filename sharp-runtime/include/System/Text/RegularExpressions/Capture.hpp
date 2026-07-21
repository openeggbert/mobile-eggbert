// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Text::RegularExpressions {

    using SharpRuntime::intcs;

    /**
     * @brief Represents the results from a single successful subexpression capture.
     *
     * C++ counterpart of .NET System.Text.RegularExpressions.Capture.
     */
    class Capture {
    protected:
        std::string value_;
        intcs index_ = 0;
        intcs length_ = 0;

    public:
        Capture() = default;
        Capture(std::string value, intcs index, intcs length)
            : value_(std::move(value)), index_(index), length_(length) {}
        virtual ~Capture() = default;

        /** @return The zero-based starting position of the captured substring in the original string. */
        [[nodiscard]] intcs getIndexProperty() const { return index_; }
        /** @return The length of the captured substring. */
        [[nodiscard]] intcs getLengthProperty() const { return length_; }
        /** @return The captured substring. */
        [[nodiscard]] const std::string& getValueProperty() const { return value_; }

        /** @return The captured substring (same as getValueProperty()). */
        [[nodiscard]] virtual std::string ToString() const { return value_; }
    };

} // namespace System::Text::RegularExpressions

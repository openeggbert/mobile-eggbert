// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <utility>

#include "System/InvalidOperationException.hpp"

namespace System
{
    /**
     * @brief A value type that can represent a value or the absence of one.
     *
     * C++ counterpart of .NET System.Nullable<T> (the C# `T?` shorthand). Introduced as the
     * C++14-compatible replacement for std::optional<T> (C++17) throughout the XNA-facing API
     * surface, where the original C# signature uses a nullable value type
     * (e.g. `Rectangle? sourceRectangle`, `char? DefaultCharacter`).
     */
    template <typename T>
    class Nullable
    {
    public:
        /** @brief Initializes a new instance with no value (C# equivalent: `null`). */
        constexpr Nullable() noexcept : hasValue_(false), value_() {}

        /** @brief Initializes a new instance holding @p value. */
        constexpr Nullable(const T& value) noexcept : hasValue_(true), value_(value) {} // NOLINT(*-explicit-constructor)

        /** @brief Gets whether the current instance has a value. */
        [[nodiscard]] constexpr bool getHasValueProperty() const noexcept { return hasValue_; }

        /**
         * @brief Gets the value of the current instance.
         *
         * @throws System::InvalidOperationException if getHasValueProperty() is false.
         */
        [[nodiscard]] const T& getValueProperty() const
        {
            if (!hasValue_)
            {
                throw InvalidOperationException("Nullable object must have a value.");
            }
            return value_;
        }

        /** @brief Gets the value if present, otherwise @p defaultValue. */
        [[nodiscard]] constexpr T GetValueOrDefault(const T& defaultValue) const
        {
            return hasValue_ ? value_ : defaultValue;
        }

        [[nodiscard]] constexpr bool operator==(const Nullable& other) const
        {
            return hasValue_ == other.hasValue_ && (!hasValue_ || value_ == other.value_);
        }

        [[nodiscard]] constexpr bool operator!=(const Nullable& other) const
        {
            return !(*this == other);
        }

    private:
        bool hasValue_;
        T value_;
    };
} // namespace System

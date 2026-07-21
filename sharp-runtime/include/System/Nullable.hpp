// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <concepts>
#include <functional>
#include <optional>
#include <sstream>
#include <string>
#include <type_traits>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/InvalidOperationException.hpp"

namespace System {

    using SharpRuntime::intcs;

    /**
     * @brief Represents a value type that can be assigned null.
     *
     * C++ counterpart of .NET System.Nullable<T> struct.
     * Backed by std::optional<T>.
     *
     * @tparam T The underlying value type of the Nullable generic type.
     */
    template<typename T>
    class Nullable {
        std::optional<T> value_;

    public:
        /**
         * @brief Initializes a new instance of the Nullable<T> structure with
         * no value (HasValue is false).
         *
         * C++ counterpart of the implicit null-assignment of .NET Nullable<T>.
         */
        Nullable() = default;

        /**
         * @brief Initializes a new instance of the Nullable<T> structure to the
         * specified value.
         *
         * C++ counterpart of .NET Nullable<T>(T value).
         * Implicit to mirror the implicit C# conversion from T to T?.
         * @param value The value to wrap.
         */
        Nullable(const T& value) : value_(value) {}  // NOLINT(google-explicit-constructor)

        /** @brief Move-constructing overload for Nullable<T>(T). */
        Nullable(T&& value) : value_(std::move(value)) {}  // NOLINT(google-explicit-constructor)

        // -----------------------------------------------------------------------
        // Properties
        // -----------------------------------------------------------------------

        /**
         * @brief Gets a value indicating whether the current Nullable<T> object
         * has a valid value of its underlying type.
         *
         * C++ counterpart of .NET Nullable<T>.HasValue.
         * @return true if the current object has a value; false if it is null.
         */
        [[nodiscard]] bool getHasValueProperty() const noexcept {
            return value_.has_value();
        }

        /**
         * @brief Gets the value of the current Nullable<T> object if it has
         * been assigned a valid underlying value.
         *
         * C++ counterpart of .NET Nullable<T>.Value.
         * @return The value of the current Nullable<T> object.
         * @throws InvalidOperationException if HasValue is false.
         */
        [[nodiscard]] const T& getValueProperty() const {
            if (!value_)
                throw InvalidOperationException("Nullable object must have a value.");
            return *value_;
        }

        // -----------------------------------------------------------------------
        // Methods
        // -----------------------------------------------------------------------

        /**
         * @brief Retrieves the value of the current Nullable<T> object, or the
         * default value of the underlying type.
         *
         * C++ counterpart of .NET Nullable<T>.GetValueOrDefault().
         * @return The value of the Value property if HasValue is true; otherwise
         *         the default value of type T.
         */
        [[nodiscard]] T GetValueOrDefault() const {
            return value_.value_or(T{});
        }

        /**
         * @brief Retrieves the value of the current Nullable<T> object, or the
         * specified default value.
         *
         * C++ counterpart of .NET Nullable<T>.GetValueOrDefault(T).
         * @param defaultValue The value to return if HasValue is false.
         * @return The value of the Value property if HasValue is true; otherwise
         *         @p defaultValue.
         */
        [[nodiscard]] T GetValueOrDefault(const T& defaultValue) const {
            return value_.value_or(defaultValue);
        }

        /**
         * @brief Indicates whether the current Nullable<T> object is equal to
         * a specified object.
         *
         * C++ counterpart of .NET Nullable<T>.Equals(object).
         * @param other A Nullable<T> object.
         * @return true if both have no value, or both have the same value.
         */
        [[nodiscard]] bool Equals(const Nullable<T>& other) const noexcept {
            return value_ == other.value_;
        }

        /**
         * @brief Retrieves the hash code of the object returned by the Value
         * property, or zero if HasValue is false.
         *
         * C++ counterpart of .NET Nullable<T>.GetHashCode().
         * @return The hash code of the wrapped value, or 0 if HasValue is false.
         */
        [[nodiscard]] intcs GetHashCode() const noexcept {
            if (!value_) return 0;
            return static_cast<intcs>(std::hash<T>{}(*value_));
        }

        /**
         * @brief Returns the text representation of the value of the current
         * Nullable<T> object.
         *
         * C++ counterpart of .NET Nullable<T>.ToString(). Prefers a T::ToString()
         * member (this port's usual convention) if T has one, falling back to
         * stream insertion (operator<<) otherwise.
         * @return The string representation of the value if HasValue is true;
         *         an empty string if HasValue is false.
         */
        [[nodiscard]] std::string ToString() const {
            if (!value_) return "";
            if constexpr (requires (const T& t) { { t.ToString() } -> std::convertible_to<std::string>; }) {
                return value_->ToString();
            } else {
                std::ostringstream oss;
                oss.imbue(std::locale::classic());
                oss << *value_;
                return oss.str();
            }
        }

        // -----------------------------------------------------------------------
        // Operators
        // -----------------------------------------------------------------------

        /** @brief Returns true if this Nullable has a value. */
        explicit operator bool() const noexcept { return value_.has_value(); }

        /**
         * @brief Extracts the wrapped value.
         *
         * C++ counterpart of .NET's explicit operator T(T? value) => value.Value.
         * @throws InvalidOperationException if HasValue is false.
         */
        explicit operator T() const { return getValueProperty(); }

        /** @brief Returns true if both Nullable<T> instances are equal. */
        bool operator==(const Nullable<T>& other) const noexcept {
            return value_ == other.value_;
        }
        /** @brief Returns true if the two Nullable<T> instances are not equal. */
        bool operator!=(const Nullable<T>& other) const noexcept {
            return value_ != other.value_;
        }

        /** @brief Returns true if this Nullable has no value. */
        bool operator==(std::nullopt_t) const noexcept { return !value_.has_value(); }
        /** @brief Returns true if this Nullable has a value. */
        bool operator!=(std::nullopt_t) const noexcept { return value_.has_value(); }
    };

    // ---------------------------------------------------------------------------

    /**
     * @brief Provides static helper methods for Nullable<T> values.
     *
     * C++ counterpart of .NET System.Nullable (non-generic static class).
     * Named NullableHelper to avoid a name conflict with Nullable<T>.
     */
    struct NullableHelper {
        NullableHelper() = delete;

        /**
         * @brief Compares the relative values of two Nullable<T> objects.
         *
         * C++ counterpart of .NET Nullable.Compare<T>(T?, T?).
         * @return Negative if n1 < n2, zero if equal, positive if n1 > n2.
         *         A null value is considered less than any non-null value.
         */
        template<typename T>
        [[nodiscard]] static int Compare(const Nullable<T>& n1, const Nullable<T>& n2) {
            if (n1.getHasValueProperty()) {
                if (n2.getHasValueProperty()) {
                    const T& v1 = n1.getValueProperty();
                    const T& v2 = n2.getValueProperty();
                    if (v1 < v2) return -1;
                    if (v2 < v1) return  1;
                    return 0;
                }
                return 1;   // n1 has value, n2 is null → n1 > n2
            }
            return n2.getHasValueProperty() ? -1 : 0;
        }

        /**
         * @brief Indicates whether two specified Nullable<T> objects are equal.
         *
         * C++ counterpart of .NET Nullable.Equals<T>(T?, T?).
         * @return true if both are null or both hold the same value.
         */
        template<typename T>
        [[nodiscard]] static bool Equals(const Nullable<T>& n1, const Nullable<T>& n2) noexcept {
            return n1 == n2;
        }
    };

} // namespace System

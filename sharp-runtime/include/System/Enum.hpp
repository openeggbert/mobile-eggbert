// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include <type_traits>
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System {

    /**
     * @brief Provides the base class for enumerations and static utility methods
     * for working with C++ enum types.
     *
     * C++ counterpart of .NET System.Enum.
     * Full .NET Enum reflection (GetNames, GetValues, Parse, IsDefined, etc.)
     * requires runtime type information that is not available in C++, and is
     * permanently out of scope per this project's reflection deviation policy.
     * This class provides working, non-reflection template helpers (ToString,
     * ToUnderlying, ToInt32, ToObject, HasFlag) as a practical substitute --
     * these ARE fully implemented, not stubs.
     *
     * @note Status: Partial — reflection-dependent members (GetNames/GetValues/Parse/
     * IsDefined/etc.) are permanently out of scope; the non-reflection helpers above
     * are complete.
     */
    class Enum {
    public:
        Enum() = delete;

        /**
         * @brief Converts the underlying integer value of an enum constant to a
         * string using its numeric representation.
         *
         * C++ counterpart of the numeric overload of .NET Enum.ToString().
         * @tparam TEnum A C++ enum or enum class type.
         * @param value The enum value to convert.
         * @return The decimal string representation of the underlying integer value.
         */
        template<typename TEnum>
        [[nodiscard]] static std::string ToString(TEnum value) {
            return std::to_string(static_cast<std::underlying_type_t<TEnum>>(value));
        }

        /**
         * @brief Returns the underlying integer value of an enum constant.
         *
         * C++ counterpart of .NET (int)someEnumValue or Convert.ToInt32(value).
         * @tparam TEnum A C++ enum or enum class type.
         * @param value The enum value to convert.
         * @return The underlying integer value.
         */
        template<typename TEnum>
        [[nodiscard]] static std::underlying_type_t<TEnum> ToUnderlying(TEnum value) noexcept {
            return static_cast<std::underlying_type_t<TEnum>>(value);
        }

        /**
         * @brief Returns the underlying integer value cast to int.
         *
         * C++ counterpart of .NET (int)someEnumValue.
         * @tparam TEnum A C++ enum or enum class type.
         * @param value The enum value to convert.
         * @return The value cast to int.
         */
        template<typename TEnum>
        [[nodiscard]] static SharpRuntime::intcs ToInt32(TEnum value) noexcept {
            return static_cast<SharpRuntime::intcs>(value);
        }

        /**
         * @brief Converts an integer to the specified enum type.
         *
         * C++ counterpart of .NET (TEnum)intValue or Enum.ToObject(typeof(TEnum), value).
         * No range or validity check is performed.
         * @tparam TEnum A C++ enum or enum class type.
         * @param value The integer to convert.
         * @return The enum constant with the specified underlying value.
         */
        template<typename TEnum>
        [[nodiscard]] static TEnum ToObject(SharpRuntime::intcs value) noexcept {
            return static_cast<TEnum>(value);
        }

        /**
         * @brief Returns true if the specified flag or combination of flags is set in @p value.
         *
         * C++ counterpart of .NET Enum.HasFlag(Enum flag).
         * @tparam TEnum A C++ enum or enum class type whose underlying type is integral.
         * @param value The composite flags value.
         * @param flag  The flag or combination of flags to test.
         * @return true if all bits in @p flag are set in @p value; otherwise false.
         */
        template<typename TEnum>
        [[nodiscard]] static bool HasFlag(TEnum value, TEnum flag) noexcept {
            using U = std::underlying_type_t<TEnum>;
            return (static_cast<U>(value) & static_cast<U>(flag)) == static_cast<U>(flag);
        }
    };

} // namespace System

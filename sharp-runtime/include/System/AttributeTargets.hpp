// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System {

    /**
     * @brief Specifies the application elements on which a custom attribute is valid.
     *
     * This is a flags enum — values can be combined with the bitwise OR operator.
     */
    enum class AttributeTargets {
        /** @brief The attribute can be applied to an assembly. */
        Assembly         = 0x0001,
        /** @brief The attribute can be applied to a module. */
        Module           = 0x0002,
        /** @brief The attribute can be applied to a class. */
        Class            = 0x0004,
        /** @brief The attribute can be applied to a struct. */
        Struct           = 0x0008,
        /** @brief The attribute can be applied to an enum. */
        Enum             = 0x0010,
        /** @brief The attribute can be applied to a constructor. */
        Constructor      = 0x0020,
        /** @brief The attribute can be applied to a method. */
        Method           = 0x0040,
        /** @brief The attribute can be applied to a property. */
        Property         = 0x0080,
        /** @brief The attribute can be applied to a field. */
        Field            = 0x0100,
        /** @brief The attribute can be applied to an event. */
        Event            = 0x0200,
        /** @brief The attribute can be applied to an interface. */
        Interface        = 0x0400,
        /** @brief The attribute can be applied to a parameter. */
        Parameter        = 0x0800,
        /** @brief The attribute can be applied to a delegate. */
        Delegate         = 0x1000,
        /** @brief The attribute can be applied to a return value. */
        ReturnValue      = 0x2000,
        /** @brief The attribute can be applied to a generic type parameter. */
        GenericParameter = 0x4000,
        /** @brief The attribute can be applied to any application element. */
        All = Assembly | Module | Class | Struct | Enum | Constructor |
              Method | Property | Field | Event | Interface | Parameter |
              Delegate | ReturnValue | GenericParameter
    };

    /** @brief Bitwise OR operator for combining AttributeTargets flags. */
    inline AttributeTargets operator|(AttributeTargets a, AttributeTargets b) {
        return static_cast<AttributeTargets>(static_cast<int>(a) | static_cast<int>(b));
    }
    /** @brief Bitwise AND operator for testing AttributeTargets flags. */
    inline AttributeTargets operator&(AttributeTargets a, AttributeTargets b) {
        return static_cast<AttributeTargets>(static_cast<int>(a) & static_cast<int>(b));
    }

} // namespace System

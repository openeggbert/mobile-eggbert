// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System {

    /**
     * @brief Identifies the runtime category of a type token.
     *
     * C++ counterpart of the internal .NET System.RuntimeType enumeration.
     * In the CLR this is used internally to distinguish value types, reference
     * types, and special runtime constructs. The values here match the documented
     * internal constants used in CoreCLR.
     */
    enum class RuntimeType {
        /** @brief No runtime type or unknown. */
        None = 0,
        /** @brief The type is a primitive value type (int, bool, etc.). */
        Primitive = 1,
        /** @brief The type is a value type (struct). */
        ValueType = 2,
        /** @brief The type is a reference type (class). */
        ReferenceType = 3,
        /** @brief The type is an array. */
        Array = 4,
        /** @brief The type is a generic type parameter. */
        GenericParameter = 5,
    };

} // namespace System

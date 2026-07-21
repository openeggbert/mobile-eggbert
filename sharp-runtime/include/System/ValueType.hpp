// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <cstdint>
#include <string>

#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System {

    using SharpRuntime::intcs;

/**
 * @brief Provides the base class for value types.
 *
 * C++ counterpart of .NET System.ValueType. In .NET, ValueType is the implicit
 * base of all struct types; it overrides Equals and GetHashCode to perform
 * field-by-field comparison via reflection. In C++ there is no reflection, so
 * the defaults here fall back to object identity. Concrete value-type structs
 * should override Equals() and GetHashCode() to match .NET field-equality semantics.
 */
class ValueType {
public:
    virtual ~ValueType() = default;

    /**
     * @brief Indicates whether this instance equals another object.
     *
     * C++ counterpart of .NET ValueType.Equals(object).
     * Default: identity comparison. Override in concrete types for field equality.
     */
    virtual bool Equals(const ValueType& other) const { return this == &other; }

    /**
     * @brief Returns a hash code for this instance.
     *
     * C++ counterpart of .NET ValueType.GetHashCode().
     * Default: address-based hash. Override in concrete types.
     */
    virtual intcs GetHashCode() const {
        return static_cast<intcs>(reinterpret_cast<std::uintptr_t>(this));
    }

    /**
     * @brief Returns a string representation of this instance.
     *
     * C++ counterpart of .NET ValueType.ToString().
     */
    virtual std::string ToString() const { return "System.ValueType"; }
};

} // namespace System

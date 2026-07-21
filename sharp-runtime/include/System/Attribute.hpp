// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <cstddef>
#include <functional>
#include <typeinfo>

namespace System {

/**
 * @brief Base class for all custom attributes.
 *
 * C++ counterpart of .NET System.Attribute.
 *
 * In .NET, Attribute is abstract with a protected constructor and uses
 * reflection to implement field-by-field Equals/GetHashCode. In C++:
 * - The constructor is public so that the class can be instantiated in tests;
 *   treat it as logically abstract — derive from it, do not instantiate directly.
 * - Equals() and GetHashCode() default to object identity; subclasses should
 *   override them if they need value equality.
 * - TypeId returns the std::type_info of the most-derived type.
 */
class Attribute {
public:
    /** @brief Virtual destructor for safe polymorphic destruction. */
    virtual ~Attribute() = default;

    // -------------------------------------------------------------------------

    /**
     * @brief Determines whether this instance is equal to another attribute.
     *
     * C++ counterpart of .NET Attribute.Equals(object). Default implementation
     * uses object identity. Subclasses should override this to compare fields
     * when value equality is required.
     * @param other The attribute to compare with.
     * @return true if the two attributes are the same object instance.
     */
    virtual bool Equals(const Attribute& other) const { return this == &other; }

    /**
     * @brief Returns a hash code for this attribute.
     *
     * C++ counterpart of .NET Attribute.GetHashCode(). Default implementation
     * hashes the object address. Subclasses that override Equals() should also
     * override GetHashCode() to maintain the equals/hashCode contract.
     * @return A hash value derived from this object's address.
     */
    virtual int GetHashCode() const {
        return static_cast<int>(std::hash<const Attribute*>{}(this));
    }

    /**
     * @brief Gets a unique identifier for this attribute.
     *
     * C++ counterpart of .NET Attribute.TypeId (returns object/Type).
     * Here the std::type_info of the most-derived type is returned, which
     * allows distinguishing different attribute types at runtime.
     * @return The std::type_info of the most-derived concrete type.
     */
    [[nodiscard]] virtual const std::type_info& getTypeIdProperty() const {
        return typeid(*this);
    }

    /**
     * @brief Returns true if this is the default instance of the attribute type.
     *
     * C++ counterpart of .NET Attribute.IsDefaultAttribute().
     * Default implementation returns false; subclasses may override.
     */
    virtual bool getIsDefaultAttributeProperty() const { return false; }

    /**
     * @brief Returns true if this attribute matches the specified attribute.
     *
     * C++ counterpart of .NET Attribute.Match(object). Default implementation
     * delegates to Equals(). Subclasses may override to provide a weaker
     * notion of equality (e.g., same type regardless of field values).
     * @param obj The attribute to match against.
     * @return The result of Equals(obj).
     */
    virtual bool Match(const Attribute& obj) const { return Equals(obj); }
};

} // namespace System

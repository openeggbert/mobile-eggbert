// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <string>
#include <cstddef>
#include <typeinfo>
#include "System/Type.hpp"

/**
 * @def GetTypeNameHPP()
 * @brief Declares the GetTypeName() override in the class body.
 *
 * Place this macro inside a class that derives from System::Object to
 * declare the virtual GetTypeName() method. Pair with GetTypeNameCPP()
 * in the corresponding .cpp file.
 */
#define GetTypeNameHPP() [[nodiscard]] virtual const std::string& GetTypeName() const override;

/**
 * @def GetTypeNameCPP(CLASS, NAME)
 * @brief Defines the GetTypeName() override in the .cpp file.
 *
 * @param CLASS The fully-qualified class name (e.g. System::Foo).
 * @param NAME  A quoted string literal with the fully-qualified .NET type name
 *              (e.g. "System.Foo"). Must be a string literal — do not pass an
 *              unquoted identifier, as NAME is used directly without stringization.
 */
#define GetTypeNameCPP(CLASS, NAME) \
const std::string& CLASS ::GetTypeName() const\
        {\
            const static std::string type_name = NAME;\
            return type_name;\
        }

namespace System
{
    /**
     * @brief The root base class for all objects in the sharp-runtime class hierarchy.
     *
     * C++ counterpart of .NET System.Object.
     * Provides the default implementations of ToString(), Equals(), GetHashCode(),
     * and the static helpers Equals(objA, objB) and ReferenceEquals(objA, objB).
     *
     * In this port the class is abstract because C++ has no universal root type;
     * the pure virtual GetTypeName() ensures every concrete derived class provides
     * a type-name string (used by the default ToString() implementation).
     */
    class Object
    {
    public:
        /** @brief Constructs a new Object instance. C++ counterpart of .NET Object(). */
        Object();

        /** @brief Virtual destructor for safe polymorphic destruction. */
        virtual ~Object();

        /**
         * @brief Returns a string that represents the current object.
         *
         * C++ counterpart of .NET Object.ToString().
         * The default implementation returns GetTypeName(). Derived classes
         * should override this to return a human-readable representation.
         * @return A string representation of this object.
         */
        [[nodiscard]] virtual std::string ToString() const;

        /**
         * @brief Determines whether the specified object is equal to the current object.
         *
         * C++ counterpart of .NET Object.Equals(object).
         * The default implementation uses reference identity (pointer equality).
         * Derived classes should override this to implement value equality.
         * @param obj The object to compare with the current object; may be nullptr.
         * @return true if @p obj is the same instance as this object; otherwise false.
         */
        virtual bool Equals(const Object* obj) const;

        /**
         * @brief Determines whether two object instances are considered equal.
         *
         * C++ counterpart of .NET Object.Equals(object, object).
         * Returns true if both pointers are the same, or if neither is nullptr and
         * the first object's Equals() returns true for the second.
         * @param objA The first object; may be nullptr.
         * @param objB The second object; may be nullptr.
         * @return true if the objects are considered equal; otherwise false.
         */
        static bool Equals(const Object* objA, const Object* objB);

        /**
         * @brief Determines whether the specified object instances are the same instance.
         *
         * C++ counterpart of .NET Object.ReferenceEquals(object, object).
         * Returns true only when both pointers compare equal (including both nullptr).
         * @param objA The first object; may be nullptr.
         * @param objB The second object; may be nullptr.
         * @return true if @p objA and @p objB refer to the same instance; otherwise false.
         */
        static bool ReferenceEquals(const Object* objA, const Object* objB);

        /**
         * @brief Serves as the default hash function.
         *
         * C++ counterpart of .NET Object.GetHashCode().
         * The default implementation derives a non-negative hash from this object's
         * address. Derived classes that override Equals() should also override
         * GetHashCode() to satisfy the equals/hashCode contract.
         * @return A hash code for this object.
         */
        [[nodiscard]] virtual int GetHashCode() const;

        /**
         * @brief Gets a Type object that describes the runtime type of this object.
         *
         * C++ counterpart of .NET Object.GetType().
         * Uses C++ RTTI (typeid(*this)) so the returned Type correctly reflects
         * the most-derived concrete class even when called through a base pointer.
         * This method is not virtual — like its .NET counterpart it cannot be
         * overridden, but RTTI ensures the dynamic type is always returned.
         * @return A Type instance representing the runtime type of this object.
         */
        [[nodiscard]] Type GetType() const
        {
            return Type::FromTypeInfo(typeid(*this));
        }

        /**
         * @brief Returns the runtime type name of this object.
         *
         * Project-specific extension (no direct .NET equivalent).
         * Every concrete class must override this and return a stable string
         * identifying the type. The GetTypeNameHPP() / GetTypeNameCPP() macros
         * provide a convenient way to implement this.
         * @return The type name string for this object.
         */
        [[nodiscard]] virtual const std::string& GetTypeName() const = 0;
    };
}

// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <functional>
#include <string>
#include <typeinfo>

namespace System
{
    /**
     * @brief Represents a type declaration — class, interface, array, value type, or enumeration.
     *
     * Minimal C++ counterpart of the .NET System.Type class, sufficient for
     * use as a service key in GameServiceContainer and similar patterns.
     *
     * Unlike .NET reflection, this implementation identifies types by their
     * C++ RTTI std::type_info, accessible via the static From&lt;T&gt;() factory.
     *
     * @note Status: STUB (permanent deviation — see CLAUDE.md's "Known permanent
     *   deviations"; reflection is out of scope, this is the intended end state, not a gap
     *   to close). All boolean predicates (IsClass, IsValueType, IsAbstract, IsSealed,
     *   IsInterface) return a fixed value regardless of what T actually is — C++ RTTI cannot
     *   determine any of these. They are **not mutually consistent**: e.g. `Type::From<int>()`
     *   reports both IsClass()==true and IsValueType()==false simultaneously, which would be
     *   contradictory for a real .NET type (int is IsValueType==true, IsClass==false). Do not
     *   branch on these predicates to distinguish value types from class types; they exist
     *   only so ported code that calls `type.IsClass` etc. compiles, not to answer the
     *   question correctly.
     */
    class Type
    {
    private:
        const std::type_info* info_;

        explicit Type(const std::type_info& info) : info_(&info) {}

    public:
        /**
         * @brief Constructs a null Type (no type information).
         *
         * Equivalent to a default-constructed type handle; all property
         * accessors return empty / false for this instance.
         */
        Type() : info_(nullptr) {}

        // -----------------------------------------------------------------------
        // Factory
        // -----------------------------------------------------------------------

        /**
         * @brief Creates a Type instance representing the template parameter T.
         *
         * C++ counterpart of .NET typeof(T) or obj.GetType().
         * @tparam T The C++ type to represent.
         * @return A Type wrapping the RTTI information for T.
         */
        template<typename T>
        static Type From()
        {
            return Type(typeid(T));
        }

        /**
         * @brief Creates a Type instance from a std::type_info reference.
         *
         * Used internally by Object::GetType() to construct a Type from
         * the result of typeid(*this) on a polymorphic object.
         * @param info The RTTI descriptor to wrap.
         * @return A Type wrapping @p info.
         */
        static Type FromTypeInfo(const std::type_info& info)
        {
            return Type(info);
        }

        // -----------------------------------------------------------------------
        // Name properties
        // -----------------------------------------------------------------------

        /**
         * @brief Returns the name of the type as reported by C++ RTTI.
         *
         * On most compilers the name is mangled; prefer getNameProperty() for
         * display purposes. This method exists for backward compatibility with
         * early sharp-runtime code.
         */
        [[nodiscard]] std::string getName() const
        {
            return info_ ? info_->name() : "(null)";
        }

        /**
         * @brief Gets the name of the current type.
         *
         * C++ counterpart of .NET Type.Name.
         * Returns the C++ RTTI name (possibly mangled). Returns "(null)" for
         * a default-constructed Type.
         */
        [[nodiscard]] std::string getNameProperty() const
        {
            return getName();
        }

        /**
         * @brief Gets the fully qualified name of the type.
         *
         * C++ counterpart of .NET Type.FullName.
         * Returns the same RTTI name as getNameProperty() — namespace
         * separation is not available through RTTI in this port.
         */
        [[nodiscard]] std::string getFullNameProperty() const
        {
            return getName();
        }

        // -----------------------------------------------------------------------
        // Object overrides
        // -----------------------------------------------------------------------

        /**
         * @brief Returns a string representation of the type.
         *
         * C++ counterpart of .NET Type.ToString().
         * Returns the same value as getNameProperty().
         */
        [[nodiscard]] std::string ToString() const
        {
            return getName();
        }

        /**
         * @brief Returns a hash code for this type.
         *
         * C++ counterpart of .NET Type.GetHashCode().
         * Based on std::type_info::hash_code().
         */
        [[nodiscard]] std::size_t GetHashCode() const noexcept
        {
            return info_ ? info_->hash_code() : 0;
        }

        // -----------------------------------------------------------------------
        // Stub predicates
        // -----------------------------------------------------------------------

        /**
         * @brief Gets a value indicating whether the type is a value type.
         *
         * C++ counterpart of .NET Type.IsValueType.
         * Always returns false in this stub — C++ RTTI cannot determine this.
         */
        [[nodiscard]] bool getIsValueTypeProperty() const noexcept { return false; }

        /**
         * @brief Gets a value indicating whether the type is a class.
         *
         * C++ counterpart of .NET Type.IsClass.
         * Always returns true in this stub.
         */
        [[nodiscard]] bool getIsClassProperty() const noexcept { return true; }

        /**
         * @brief Gets a value indicating whether the type is abstract.
         *
         * C++ counterpart of .NET Type.IsAbstract.
         * Always returns false in this stub.
         */
        [[nodiscard]] bool getIsAbstractProperty() const noexcept { return false; }

        /**
         * @brief Gets a value indicating whether the type is sealed (cannot be inherited).
         *
         * C++ counterpart of .NET Type.IsSealed.
         * Always returns false in this stub.
         */
        [[nodiscard]] bool getIsSealedProperty() const noexcept { return false; }

        /**
         * @brief Gets a value indicating whether the type is an interface.
         *
         * C++ counterpart of .NET Type.IsInterface.
         * Always returns false in this stub.
         */
        [[nodiscard]] bool getIsInterfaceProperty() const noexcept { return false; }

        // -----------------------------------------------------------------------
        // Comparison operators
        // -----------------------------------------------------------------------

        /**
         * @brief Returns true if both Type instances refer to the same C++ type.
         *
         * C++ counterpart of .NET Type equality (reference identity in .NET,
         * RTTI equality here).
         */
        [[nodiscard]] bool operator==(const Type& other) const noexcept
        {
            if (!info_ || !other.info_) return info_ == other.info_;
            return *info_ == *other.info_;
        }

        /**
         * @brief Returns true if the two Type instances refer to different types.
         */
        [[nodiscard]] bool operator!=(const Type& other) const noexcept
        {
            return !(*this == other);
        }

        // -----------------------------------------------------------------------
        // Low-level access
        // -----------------------------------------------------------------------

        /**
         * @brief Returns the underlying std::type_info pointer.
         *
         * Useful for constructing std::unordered_map keys or interoperating
         * with other RTTI-based code.
         * @return Pointer to the RTTI descriptor, or nullptr for a null Type.
         */
        [[nodiscard]] const std::type_info* getTypeInfo() const noexcept { return info_; }
    };

} // namespace System

namespace std
{
    template<>
    struct hash<System::Type>
    {
        std::size_t operator()(const System::Type& t) const noexcept
        {
            return t.GetHashCode();
        }
    };
}

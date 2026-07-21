// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <typeinfo>
#include "System/NotSupportedException.hpp"

namespace System {

    /**
     * @brief Describes objects that contain both a managed pointer to a location
     * and a runtime representation of the type that may be stored at that location.
     *
     * C++ counterpart of .NET System.TypedReference.
     *
     * **Status: STUB** — TypedReference is a CLR `ref struct` whose construction and
     * destruction depend on C# compiler intrinsics (`__makeref`, `__reftype`,
     * `__refvalue`) and runtime services (`FieldInfo`, `RuntimeTypeHandle`,
     * `Unsafe.NullRef`). None of these are available in a C++ environment.
     *
     * The public API is exposed so that code referencing the type compiles; all
     * non-trivial methods throw NotSupportedException at runtime.
     *
     * @note Not CLS-compliant (mirrors `[CLSCompliant(false)]` in .NET).
     */
    struct TypedReference {
        /**
         * @brief Returns a hash code for this TypedReference.
         *
         * C++ counterpart of .NET TypedReference.GetHashCode().
         * Always returns 0 because the CLR type token is not available.
         */
        [[nodiscard]] int GetHashCode() const noexcept { return 0; }

        /**
         * @brief Not supported — throws NotSupportedException.
         *
         * C++ counterpart of .NET TypedReference.Equals(object).
         * Throws because equality comparison for TypedReference is not supported
         * by the .NET runtime itself.
         */
        [[noreturn]] bool Equals(const TypedReference&) const {
            throw NotSupportedException(
                "TypedReference.Equals is not supported.");
        }

        /**
         * @brief Not supported — throws NotSupportedException.
         *
         * C++ counterpart of .NET TypedReference.MakeTypedReference(object, FieldInfo[]).
         * Requires CLR reflection (FieldInfo) and compiler intrinsics that are not
         * available in C++.
         */
        [[noreturn]] static TypedReference MakeTypedReference() {
            throw NotSupportedException(
                "TypedReference.MakeTypedReference requires CLR reflection and is not supported in sharp-runtime.");
        }

        /**
         * @brief Not supported — throws NotSupportedException.
         *
         * C++ counterpart of .NET TypedReference.SetTypedReference(TypedReference, object).
         */
        [[noreturn]] static void SetTypedReference(TypedReference, const void*) {
            throw NotSupportedException(
                "TypedReference.SetTypedReference is not supported in sharp-runtime.");
        }

        /**
         * @brief Not supported — throws NotSupportedException.
         *
         * C++ counterpart of .NET TypedReference.GetTargetType(TypedReference).
         * Would return a System.Type, which requires CLR reflection.
         */
        [[noreturn]] static const std::type_info& GetTargetType(TypedReference) {
            throw NotSupportedException(
                "TypedReference.GetTargetType requires CLR reflection and is not supported in sharp-runtime.");
        }
    };

} // namespace System

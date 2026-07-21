// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include "System/NotSupportedException.hpp"
#include "System/RuntimeArgumentHandle.hpp"
#include "System/TypedReference.hpp"

namespace System {

    /**
     * @brief Enumerates the arguments in a variable-length argument list.
     *
     * C++ counterpart of .NET System.ArgIterator.
     *
     * **Status: STUB** — ArgIterator depends on the CLR `__arglist` keyword,
     * `RuntimeArgumentHandle`, and `TypedReference`, none of which exist in C++.
     * All methods throw NotSupportedException at runtime.
     */
    struct ArgIterator {
        /**
         * @brief Constructs an ArgIterator over the given variable-argument handle.
         *
         * Always throws NotSupportedException; __arglist is not available in C++.
         * @param arglist Handle to the variable argument list.
         */
        [[noreturn]] explicit ArgIterator(RuntimeArgumentHandle /*arglist*/) {
            throw NotSupportedException(
                "ArgIterator requires CLR __arglist support and is not available in sharp-runtime.");
        }

        /**
         * @brief Constructs an ArgIterator with an explicit pointer to the first argument.
         *
         * Always throws NotSupportedException.
         * @param arglist Handle to the variable argument list.
         * @param ptr     Pointer to the first argument descriptor.
         */
        [[noreturn]] ArgIterator(RuntimeArgumentHandle /*arglist*/, void* /*ptr*/) {
            throw NotSupportedException(
                "ArgIterator requires CLR __arglist support and is not available in sharp-runtime.");
        }

        /**
         * @brief Concludes processing of the argument list.
         *
         * No-op in this implementation (there is no list to finalize).
         */
        void End() noexcept {}

        /**
         * @brief Returns false — ArgIterator stubs are never equal.
         * @return false.
         */
        [[nodiscard]] bool Equals(const ArgIterator& /*other*/) const noexcept { return false; }

        /**
         * @brief Returns a hash code for this ArgIterator.
         * @return 0 (stub).
         */
        [[nodiscard]] int GetHashCode() const noexcept { return 0; }

        /**
         * @brief Returns the next argument in the variable-argument list.
         *
         * Always throws NotSupportedException.
         */
        [[noreturn]] TypedReference GetNextArg() {
            throw NotSupportedException(
                "ArgIterator.GetNextArg requires CLR __arglist support and is not available in sharp-runtime.");
        }

        /**
         * @brief Returns the next argument constrained to the specified runtime type.
         *
         * Always throws NotSupportedException.
         */
        [[noreturn]] TypedReference GetNextArgType() {
            throw NotSupportedException(
                "ArgIterator.GetNextArgType requires CLR __arglist support and is not available in sharp-runtime.");
        }

        /**
         * @brief Returns the number of arguments remaining in the list.
         *
         * Always throws NotSupportedException.
         * @return Never returns.
         */
        [[noreturn]] int GetRemainingCount() {
            throw NotSupportedException(
                "ArgIterator.GetRemainingCount requires CLR __arglist support and is not available in sharp-runtime.");
        }
    };

} // namespace System

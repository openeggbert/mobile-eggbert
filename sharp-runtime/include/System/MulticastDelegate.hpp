// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Delegate.hpp"

namespace System {

    /**
     * @brief Provides the root class for multicast delegates.
     *
     * C++ counterpart of .NET System.MulticastDelegate. In .NET, MulticastDelegate
     * derives from Delegate and is the abstract base of all compiler-generated delegate
     * types (Action, Func, EventHandler, etc.). All user-defined delegate types must
     * derive from MulticastDelegate in the .NET object hierarchy.
     *
     * In sharp-runtime the full multicast machinery (Combine, Remove, GetInvocationList,
     * invocation-list–based Equals, GetHashCode) lives in Delegate. MulticastDelegate
     * adds only:
     *   - A named type that satisfies C++ is-a checks for ported code that tests
     *     whether an object is a MulticastDelegate.
     *   - Clone() returning a MulticastDelegate rather than a plain Delegate.
     *
     * Usage: derive from MulticastDelegate (not Delegate) when porting code whose
     * delegate types are declared as `delegate` in C#.
     */
    class MulticastDelegate : public Delegate {
    public:
        /** @brief Constructs an empty (no-op) multicast delegate. */
        MulticastDelegate() = default;

        /**
         * @brief Constructs a single-target multicast delegate wrapping the given callable.
         *
         * C++ counterpart of the implicit single-entry constructor used by the C#
         * compiler when creating delegate instances from method groups or lambdas.
         * @param invoke Type-erased callable to invoke via Invoke().
         */
        explicit MulticastDelegate(ErasedInvoke invoke)
            : Delegate(std::move(invoke)) {}

        virtual ~MulticastDelegate() = default;

        // -----------------------------------------------------------------------
        // Clone — returns MulticastDelegate rather than plain Delegate
        // -----------------------------------------------------------------------

        /**
         * @brief Creates a shallow copy of this MulticastDelegate.
         *
         * C++ counterpart of .NET object.MemberwiseClone() as invoked from
         * Delegate.Clone(). Returns a shared_ptr<Delegate> whose dynamic type
         * is MulticastDelegate.
         */
        [[nodiscard]] std::shared_ptr<Delegate> Clone() const override {
            return std::make_shared<MulticastDelegate>(*this);
        }
    };

} // namespace System

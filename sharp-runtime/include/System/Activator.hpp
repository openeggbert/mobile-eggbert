// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>

namespace System {

    /**
     * @brief Contains methods to create types of objects locally or obtain references to existing remote objects.
     *
     * C++ partial counterpart of .NET System.Activator.
     * Reflection-based overloads (CreateInstance(Type), CreateInstanceFrom, etc.) are not available
     * because sharp-runtime does not implement System.Reflection.
     * Template overloads cover all constructible types at compile time.
     */
    class Activator {
    public:
        /**
         * @brief Creates an instance of type @p T using its default constructor.
         *
         * C++ counterpart of .NET Activator.CreateInstance&lt;T&gt;().
         * @tparam T Type to instantiate. Must be default-constructible.
         * @return New value-constructed instance of @p T.
         */
        template<typename T>
        [[nodiscard]] static T CreateInstance() {
            return T{};
        }

        /**
         * @brief Creates an instance of type @p T forwarding the given constructor arguments.
         *
         * C++ counterpart of .NET Activator.CreateInstance(Type, object[]).
         * @tparam T    Type to instantiate.
         * @tparam Args Constructor argument types.
         * @param  args Arguments forwarded to @p T's constructor.
         * @return New instance of @p T constructed with @p args.
         */
        template<typename T, typename... Args>
        [[nodiscard]] static T CreateInstance(Args&&... args) {
            return T{std::forward<Args>(args)...};
        }

        /**
         * @brief Creates a heap-allocated instance of type @p T using its default constructor.
         *
         * C++ extension (no direct .NET equivalent).
         * @tparam T Type to instantiate. Must be default-constructible.
         * @return std::unique_ptr owning the new instance.
         */
        template<typename T>
        [[nodiscard]] static std::unique_ptr<T> CreateInstancePtr() {
            return std::make_unique<T>();
        }

        /**
         * @brief Creates a heap-allocated instance of type @p T forwarding the given constructor arguments.
         *
         * C++ extension (no direct .NET equivalent).
         * @tparam T    Type to instantiate.
         * @tparam Args Constructor argument types.
         * @param  args Arguments forwarded to @p T's constructor.
         * @return std::unique_ptr owning the new instance.
         */
        template<typename T, typename... Args>
        [[nodiscard]] static std::unique_ptr<T> CreateInstancePtr(Args&&... args) {
            return std::make_unique<T>(std::forward<Args>(args)...);
        }
    };

} // namespace System

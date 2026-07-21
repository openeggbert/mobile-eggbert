// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System {

    /**
     * @brief Defines a generalized equality comparison method for objects of the same type.
     *
     * This template is a C++ counterpart of the .NET IEquatable<T> interface.
     * It allows a class or struct to define how it should be compared for equality
     * with another instance of the same type.
     *
     * @tparam T The type of object to compare with.
     */
    template <typename T>
    class IEquatable {
    public:
        /**
         * @brief Determines whether the current instance is equal to another instance of the same type.
         *
         * @param other Another object of the same type.
         * @return true if the current instance is equal to @p other, otherwise false.
         */
        virtual bool Equals(const T& other) const = 0;

        /**
         * @brief Virtual destructor for safe polymorphic destruction.
         */
        virtual ~IEquatable() = default;
    };

} // namespace System
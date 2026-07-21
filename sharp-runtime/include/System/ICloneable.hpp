// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>

namespace System {

    /**
     * @brief Supports cloning, which creates a new instance of a class with the
     * same value as an existing instance.
     *
     * C++ counterpart of .NET System.ICloneable.
     */
    class ICloneable {
    public:
        /** @brief Virtual destructor for safe polymorphic destruction. */
        virtual ~ICloneable() = default;
        /** @brief Creates a new object that is a copy of the current instance. */
        [[nodiscard]] virtual std::shared_ptr<ICloneable> Clone() const = 0;
    };

} // namespace System

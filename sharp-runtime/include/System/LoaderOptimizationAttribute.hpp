// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <cstdint>
#include "System/Attribute.hpp"
#include "System/LoaderOptimization.hpp"

namespace System {

    /**
     * @brief Allows developers to specify the particular optimization that
     * the loader uses when loading an assembly for an application.
     *
     * C++ counterpart of .NET System.LoaderOptimizationAttribute.
     * This attribute targets methods ([AttributeUsage(AttributeTargets.Method)]).
     */
    class LoaderOptimizationAttribute final : public Attribute {
    public:
        /**
         * @brief Initializes a new instance with the specified byte value.
         *
         * C++ counterpart of .NET LoaderOptimizationAttribute(byte).
         * The byte is cast to LoaderOptimization.
         */
        explicit LoaderOptimizationAttribute(uint8_t value)
            : value_(static_cast<LoaderOptimization>(value)) {}

        /**
         * @brief Initializes a new instance with the specified optimization value.
         *
         * C++ counterpart of .NET LoaderOptimizationAttribute(LoaderOptimization).
         */
        explicit LoaderOptimizationAttribute(LoaderOptimization value)
            : value_(value) {}

        /**
         * @brief Gets the loader optimization value.
         *
         * C++ counterpart of .NET LoaderOptimizationAttribute.Value.
         */
        [[nodiscard]] LoaderOptimization getValueProperty() const { return value_; }

    private:
        LoaderOptimization value_;
    };

} // namespace System

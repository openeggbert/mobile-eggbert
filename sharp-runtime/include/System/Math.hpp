// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <cmath>
#include <cstdint>
#include <limits>
#include <utility>
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System
{
    using SharpRuntime::intcs;
    using SharpRuntime::longcs;

    /**
     * @brief Provides constants and static methods for trigonometric,
     * logarithmic, and other common mathematical functions.
     *
     * C++ counterpart of .NET System.Math.
     */
    class Math
    {
    public:
        Math() = delete;
        ~Math() = delete;

        /** @brief The ratio of a circle's circumference to its diameter, π ≈ 3.14159. */
        static constexpr double PI  = 3.14159'26535'89793'23846;

        /**
         * @brief Returns the sine of the specified angle.
         *
         * The angle is specified in radians.
         *
         * @param value Angle in radians.
         * @return Sine of the angle.
         *
         */
        [[nodiscard]] static double Sin(double value);

        /**
         * @brief Returns the cosine of the specified angle.
         *
         * The angle is specified in radians.
         *
         * @param value Angle in radians.
         * @return Cosine of the angle.
         *
         */
        [[nodiscard]] static double Cos(double value);

        /**
         * @brief Returns the smaller of two 32-bit signed integers.
         *
         * @param a First value.
         * @param b Second value.
         * @return Smaller value.
         *
         */
        [[nodiscard]] static intcs Min(intcs a, intcs b);

        /**
         * @brief Returns the smaller of two double-precision numbers.
         *
         * @param a First value.
         * @param b Second value.
         * @return Smaller value.
         *
         */
        [[nodiscard]] static double Min(double a, double b);

        /**
         * @brief Returns the larger of two 32-bit signed integers.
         *
         * @param a First value.
         * @param b Second value.
         * @return Larger value.
         *
         */
        [[nodiscard]] static intcs Max(intcs a, intcs b);

        /**
         * @brief Returns the larger of two double-precision numbers.
         *
         * @param a First value.
         * @param b Second value.
         * @return Larger value.
         *
         */
        [[nodiscard]] static double Max(double a, double b);
    };
}

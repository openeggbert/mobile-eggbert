// SPDX-License-Identifier: MS-PL

#pragma once

#include "CNA/CNAHelper.hpp"

namespace Microsoft::Xna::Framework
{
    /** @brief Contains commonly used precalculated values and mathematical operations. */
    class MathHelper final
    {
    public:
        /** @brief Deleted; MathHelper is a static utility class. */
        NOXNA MathHelper() = delete;

        /** @brief Mathematical constant e. */
        static constexpr float E = 2.71828175f;

        /** @brief Base-10 logarithm of e. */
        static constexpr float Log10E = 0.4342945f;

        /** @brief Base-2 logarithm of e. */
        static constexpr float Log2E = 1.442695f;

        /** @brief The value of pi. */
        static constexpr float Pi = 3.14159274f;

        /** @brief Pi divided by two. */
        static constexpr float PiOver2 = 1.57079637f;

        /** @brief Pi divided by four. */
        static constexpr float PiOver4 = 0.7853982f;

        /** @brief Pi multiplied by two. */
        static constexpr float TwoPi = 6.28318548f;

        // FNA marks the four members below as 'internal'; C++ has no assembly-scope visibility,
        // so they are public here. Other framework types (e.g. Vector2, Matrix) rely on them.

        /** @brief Float epsilon used by framework-level approximate comparisons. */
        static const float MachineEpsilonFloat;

        /**
         * @brief Restricts a floating-point value to the inclusive range [min, max].
         *
         * @param value The value to clamp.
         * @param min The minimum bound.
         * @param max The maximum bound.
         * @return The clamped value.
         */
        static float Clamp(float value, float min, float max);

        /**
         * @brief Returns true when two floats are closer than the framework epsilon.
         *
         * @param floatA The first float value.
         * @param floatB The second float value.
         * @return @c true if the values are within epsilon of each other; @c false otherwise.
         */
        static bool WithinEpsilon(float floatA, float floatB);

    private:
        static float GetMachineEpsilonFloat();
    };
}

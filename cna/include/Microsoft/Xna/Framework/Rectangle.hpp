// SPDX-License-Identifier: MS-PL

#pragma once

#include <string>

#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace Microsoft::Xna::Framework
{
    using SharpRuntime::intcs;

    /** @brief Describes a rectangle in two-dimensional integer space. */
    struct Rectangle
    {
        /** @brief X coordinate of the top-left corner. */
        intcs X;

        /** @brief Y coordinate of the top-left corner. */
        intcs Y;

        /** @brief Width of the rectangle. */
        intcs Width;

        /** @brief Height of the rectangle. */
        intcs Height;

        /** @brief Creates an empty rectangle at 0, 0. */
        Rectangle();

        /**
         * @brief Creates a rectangle from position and size.
         *
         * @param x The X coordinate of the top-left corner.
         * @param y The Y coordinate of the top-left corner.
         * @param width The width of the rectangle.
         * @param height The height of the rectangle.
         */
        Rectangle(intcs x, intcs y, intcs width, intcs height);

        /**
         * @brief Gets the x coordinate of the left edge.
         *
         * @return The x coordinate of the left edge.
         */
        [[nodiscard]] intcs getLeftProperty() const;

        /**
         * @brief Gets the y coordinate of the top edge.
         *
         * @return The y coordinate of the top edge.
         */
        [[nodiscard]] intcs getTopProperty() const;

        /**
         * @brief Returns true when the specified coordinates are inside this rectangle.
         *
         * @param x The X coordinate to test.
         * @param y The Y coordinate to test.
         * @return @c true if the point is inside this rectangle; @c false otherwise.
         */
        [[nodiscard]] bool Contains(intcs x, intcs y) const;

        /**
         * @brief Returns true when all four fields are equal.
         *
         * @param value1 Left-hand rectangle.
         * @param value2 Right-hand rectangle.
         * @return @c true if the rectangles are equal; @c false otherwise.
         */
        friend bool operator==(Rectangle value1, Rectangle value2);

        /**
         * @brief Returns true when any field differs.
         *
         * @param value1 Left-hand rectangle.
         * @param value2 Right-hand rectangle.
         * @return @c true if the rectangles are not equal; @c false otherwise.
         */
        friend bool operator!=(Rectangle value1, Rectangle value2);

    private:
        [[nodiscard]] std::string getDebugDisplayStringProperty() const;
    };
}

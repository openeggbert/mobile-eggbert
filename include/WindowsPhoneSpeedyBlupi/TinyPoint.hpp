/**
 * @file TinyPoint.hpp
 * @brief Declaration of the TinyPoint lightweight 2D integer point structure.
 * @details TinyPoint is the primary screen-coordinate carrier throughout the
 *          game.  It stores a horizontal (@c X) and vertical (@c Y) pixel
 *          position using the same @c intcs integer type used by the original
 *          C# codebase.
 */

#pragma once

#include <string>
#include <sstream>
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace WindowsPhoneSpeedyBlupi
{
    using SharpRuntime::intcs;

    /**
     * @struct TinyPoint
     * @brief Lightweight 2D integer point (X, Y).
     *
     * @details This structure is a C++ port of the original C# @c TinyPoint
     *          struct from WindowsPhoneSpeedyBlupi.  It stores the horizontal
     *          and vertical screen position in the fields @c X and @c Y.
     *
     *          TinyPoint is a plain aggregate — no heap allocation, no virtual
     *          functions, no hidden state — and is cheap to copy.
     *
     * @note The field names @c X and @c Y match the C# originals exactly so
     *       that ported code remains readable alongside the original source.
     * @note Status: Ported
     */
    struct TinyPoint
    {
        intcs X; ///< @brief Horizontal (column) coordinate in pixels.
        intcs Y; ///< @brief Vertical (row) coordinate in pixels.

        /**
         * @brief Constructs a point at the origin (0, 0).
         *
         * @details Value-initialises both fields to zero.  Added in the C++
         *          port to support default construction; the original C# struct
         *          provided this behaviour implicitly.
         *
         * @note Status: Ported
         */
        TinyPoint()
            : X(0), Y(0)
        {
        }

        /**
         * @brief Constructs a point with explicit coordinates.
         *
         * @param[in] x Horizontal coordinate.
         * @param[in] y Vertical coordinate.
         *
         * @note Status: Ported
         */
        TinyPoint(intcs x, intcs y)
            : X(x), Y(y)
        {
        }

        /**
         * @brief Returns a human-readable representation of the point.
         *
         * @details Produces the string @c "X;Y", e.g. @c "10;20" for
         *          @c TinyPoint(10, 20).  Useful for logging and debugging.
         *
         * @return String in the format @c "X;Y".
         *
         * @note Status: Ported
         */
        [[nodiscard]] std::string ToString() const
        {
            std::ostringstream oss;
            oss << X << ";" << Y;
            return oss.str();
        }
    };
}

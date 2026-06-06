/**
 * @file TinyRect.hpp
 * @brief Declaration of the TinyRect lightweight integer rectangle structure.
 *
 * @warning NON-STANDARD FIELD ORDER.
 *          TinyRect stores its edges as @b Left, @b Right, @b Top, @b Bottom —
 *          NOT the more common Left, Top, Right, Bottom order used by Win32
 *          RECT, SDL_Rect, or most other rectangle types.  This order is
 *          preserved from the original C# WindowsPhoneSpeedyBlupi codebase.
 *          Always verify argument order when constructing a TinyRect by value.
 */

#pragma once

#include <string>
#include <sstream>

#include "TinyPoint.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace WindowsPhoneSpeedyBlupi
{
    using SharpRuntime::intcs;

    /**
     * @struct TinyRect
     * @brief Lightweight integer rectangle defined by four edge coordinates.
     *
     * @details This structure is a C++ port of the original C# @c TinyRect
     *          struct from WindowsPhoneSpeedyBlupi.  It is the primary
     *          screen-region type used by sprite-draw and hit-test code.
     *
     *          Width  = Right  - Left  (may be zero or negative for degenerate rects).
     *          Height = Bottom - Top   (may be zero or negative for degenerate rects).
     *
     * @warning NON-STANDARD FIELD ORDER: the four fields are declared (and the
     *          four-argument constructor accepts them) in the order
     *          @b Left, @b Right, @b Top, @b Bottom.  This differs from the
     *          conventional Left, Top, Right, Bottom order.  Passing arguments
     *          in the wrong order produces silent geometry bugs.
     *
     * @note Status: Ported
     *
     * @see TinyPoint
     */
    struct TinyRect
    {
        intcs Left;   ///< @brief Left edge (minimum X) of the rectangle in pixels.
        intcs Right;  ///< @brief Right edge (maximum X) of the rectangle in pixels.
        intcs Top;    ///< @brief Top edge (minimum Y) of the rectangle in pixels.
        intcs Bottom; ///< @brief Bottom edge (maximum Y) of the rectangle in pixels.

        /**
         * @brief Constructs a zero-sized rectangle at the origin.
         *
         * @details Value-initialises all four edges to zero.  Equivalent to
         *          the C# expression @c default(TinyRect).
         *
         * @note Status: Ported
         */
        TinyRect()
            : Left(0), Right(0), Top(0), Bottom(0)
        {
        }

        /**
         * @brief Constructs a rectangle from explicit edge coordinates.
         *
         * @warning Parameter order is @b Left, @b Right, @b Top, @b Bottom —
         *          NOT the conventional Left, Top, Right, Bottom order.
         *          This unusual order replicates the original C# struct layout.
         *
         * @param[in] left   Left edge (minimum X).
         * @param[in] right  Right edge (maximum X).
         * @param[in] top    Top edge (minimum Y).
         * @param[in] bottom Bottom edge (maximum Y).
         *
         * @note Status: IMPLEMENTED
         */
        TinyRect(const intcs left, const intcs right, const intcs top, const intcs bottom)
            : Left(left), Right(right), Top(top), Bottom(bottom)
        {
        }

        /**
         * @brief Constructs a zero-sized rectangle located at @p point.
         *
         * @details Sets Left = Right = point.X and Top = Bottom = point.Y,
         *          producing a degenerate (zero-area) rectangle anchored at the
         *          given position.  Used by icon-drawing APIs where a zero-area
         *          TinyRect signals "use the icon's default dimensions".
         *
         * @param[in] point Position of the zero-sized rectangle.
         *
         * @note Status: IMPLEMENTED
         */
        explicit TinyRect(TinyPoint point);

        /**
         * @brief Returns the width of the rectangle.
         *
         * @details Computed as @c Right - Left.  Returns zero or a negative
         *          value for degenerate rectangles.
         *
         * @return Rectangle width in pixels.
         *
         * @note Status: Ported
         */
        [[nodiscard]] intcs getWidthProperty() const;

        /**
         * @brief Returns the height of the rectangle.
         *
         * @details Computed as @c Bottom - Top.  Returns zero or a negative
         *          value for degenerate rectangles.
         *
         * @return Rectangle height in pixels.
         *
         * @note Status: Ported
         */
        [[nodiscard]] intcs getHeightProperty() const;

        /**
         * @brief Returns a human-readable representation of the rectangle.
         *
         * @details Produces the string @c "Left;Top;Right;Bottom", e.g.
         *          @c "0;0;100;50".  Useful for logging and debugging.
         *
         * @return String in the format @c "Left;Top;Right;Bottom".
         *
         * @note Status: Ported
         */
        [[nodiscard]] std::string ToString() const
        {
            std::ostringstream oss;
            oss << Left << ";" << Top << ";" << Right << ";" << Bottom;
            return oss.str();
        }
    };
}

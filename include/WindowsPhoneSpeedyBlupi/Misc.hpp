/**
 * @file Misc.hpp
 * @brief Declarations for the Misc static utility class.
 * @details Provides 2D geometry helpers: rectangle intersection/union,
 *          point rotation using the standard rotation matrix, and
 *          angle/direction conversion utilities.  All methods are pure
 *          functions with no side effects.
 */

#pragma once

#include "TinyPoint.hpp"
#include "TinyRect.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace WindowsPhoneSpeedyBlupi
{
    using SharpRuntime::intcs;
    using WindowsPhoneSpeedyBlupi::TinyPoint;

    /**
     * @class Misc
     * @brief Static utility class for 2D geometry operations used throughout the game.
     *
     * @details This class is a C++ port of the original C# static class @c Misc from
     *          WindowsPhoneSpeedyBlupi.  It provides:
     *          - Point rotation around an arbitrary centre via the standard sin/cos matrix.
     *          - Degree-to-radian conversion.
     *          - Integer approach (smooth step toward a target value).
     *          - Normalised-speed-to-integer conversion with guaranteed non-zero result.
     *          - Rectangle inflation, inside test, intersection, and union.
     *
     * @note All methods are static; the class cannot be instantiated or destroyed.
     * @note Status: Ported
     */
    class Misc
    {
    public:
        /**
         * @brief Deleted default constructor — this is a static utility class.
         * @note Status: Ported
         */
        Misc() = delete;

        /**
         * @brief Deleted destructor — this is a static utility class.
         * @note Status: Ported
         */
        ~Misc() = delete;

        /**
         * @brief Adjusts a rectangle's top-left position after rotation around its centre.
         *
         * @details Computes the half-size centre of @p rect, rotates that point by
         *          @p angle radians around the origin, and shifts the rectangle's
         *          top-left corner by the resulting offset so that the visual centre
         *          remains correct after the rotation.  Width and height are unchanged.
         *
         * @param[in] rect  Rectangle whose position should be adjusted.
         * @param[in] angle Rotation angle in radians (counter-clockwise positive).
         * @return A new rectangle with the same dimensions as @p rect but a
         *         corrected top-left position.
         *
         * @note Status: Ported
         */
        [[nodiscard]] static Microsoft::Xna::Framework::Rectangle RotateAdjust(
            const Microsoft::Xna::Framework::Rectangle& rect,
            double angle);

        /**
         * @brief Rotates a point around the origin (0, 0).
         *
         * @details Convenience overload that delegates to the three-argument form
         *          with a zero-initialised centre point.
         *
         * @param[in] angle Rotation angle in radians (counter-clockwise positive).
         * @param[in] p     Point to rotate.
         * @return Rotated point (fractional values truncated toward zero).
         *
         * @see RotatePointRad(const TinyPoint&, double, const TinyPoint&)
         * @note Status: Ported
         */
        [[nodiscard]] static TinyPoint RotatePointRad(double angle, const TinyPoint& p);

        /**
         * @brief Rotates a point around an arbitrary centre using the standard 2D
         *        rotation matrix.
         *
         * @details Translates @p point by -@p center, applies the rotation matrix
         *          @f$\begin{pmatrix}\cos\theta & -\sin\theta \\ \sin\theta & \cos\theta\end{pmatrix}@f$,
         *          then translates back by +@p center.  Fractional pixel values are
         *          truncated toward zero via static_cast<int>.
         *
         * @param[in] center Centre of rotation.
         * @param[in] angle  Rotation angle in radians (counter-clockwise positive).
         * @param[in] point  Point to rotate.
         * @return Rotated point (fractional values truncated toward zero).
         *
         * @note Status: Ported
         */
        [[nodiscard]] static TinyPoint RotatePointRad(
            const TinyPoint& center,
            double angle,
            const TinyPoint& point);

        /**
         * @brief Converts an angle from degrees to radians.
         *
         * @details Computes @p angle * PI / 180.
         *
         * @param[in] angle Angle in degrees.
         * @return Equivalent angle in radians.
         *
         * @note Status: Ported
         */
        [[nodiscard]] static double DegToRad(double angle);

        /**
         * @brief Moves an integer value toward a target value by a fixed step.
         *
         * @details If @p actual < @p final, increases @p actual by @p step, clamped
         *          to @p final.  If @p actual > @p final, decreases @p actual by
         *          @p step, clamped to @p final.  If already equal, returns unchanged.
         *
         * @param[in] actual Current value.
         * @param[in] final  Target value.
         * @param[in] step   Step size (should be positive for correct behaviour).
         * @return Value after one approach step toward @p final.
         *
         * @note Status: Ported
         */
        [[nodiscard]] static intcs Approach(intcs actual, intcs final, intcs step);

        /**
         * @brief Converts a floating-point speed factor to a guaranteed non-zero
         *        integer speed (unless the input is exactly zero).
         *
         * @details Multiplies @p speed by @p max and truncates to integer.  Ensures
         *          the result is at least 1 for positive inputs and at most -1 for
         *          negative inputs, preventing a zero-movement stall caused by
         *          truncation of small floating-point values.
         *
         * @param[in] speed Normalised speed factor (positive or negative).
         * @param[in] max   Maximum magnitude scale factor.
         * @return Integer speed with the same sign as @p speed, or 0 if @p speed is 0.
         *
         * @note Status: Ported
         */
        [[nodiscard]] static intcs Speed(double speed, intcs max);

        /**
         * @brief Returns a copy of @p rect enlarged by @p value in every direction.
         *
         * @details Subtracts @p value from Left and Top; adds @p value to Right and
         *          Bottom.  Pass a negative @p value to shrink the rectangle.
         *
         * @param[in] rect  Source rectangle.
         * @param[in] value Inflation amount in pixels (negative to deflate).
         * @return Inflated rectangle.
         *
         * @note Status: Ported
         */
        [[nodiscard]] static TinyRect Inflate(const TinyRect& rect, intcs value);

        /**
         * @brief Tests whether a point lies inside or on the border of a rectangle.
         *
         * @details All boundary comparisons are inclusive (>= and <=).
         *
         * @param[in] rect Rectangle to test against.
         * @param[in] p    Point to test.
         * @return @c true if @p p is within the inclusive bounds of @p rect;
         *         @c false otherwise.
         *
         * @note Status: Ported
         */
        [[nodiscard]] static bool IsInside(const TinyRect& rect, const TinyPoint& p);

        /**
         * @brief Computes the intersection of two rectangles.
         *
         * @details Fills @p dst with the overlapping area (max of lefts/tops,
         *          min of rights/bottoms).  If the rectangles do not overlap,
         *          @p dst is set to a zero-sized rectangle.
         *
         * @param[out] dst  Receives the intersection rectangle.
         * @param[in]  src1 First source rectangle.
         * @param[in]  src2 Second source rectangle.
         * @return @c true if the resulting rectangle is non-empty; @c false otherwise.
         *
         * @note Status: Ported
         */
        static bool IntersectRect(TinyRect& dst, const TinyRect& src1, const TinyRect& src2);

        /**
         * @brief Computes the bounding rectangle that covers both source rectangles.
         *
         * @details Fills @p dst with the smallest axis-aligned rectangle that
         *          contains both @p src1 and @p src2 (min of lefts/tops, max of
         *          rights/bottoms).
         *
         * @param[out] dst  Receives the union rectangle.
         * @param[in]  src1 First source rectangle.
         * @param[in]  src2 Second source rectangle.
         * @return @c true if the resulting rectangle is non-empty; @c false otherwise.
         *
         * @note Status: Ported
         */
        static bool UnionRect(TinyRect& dst, const TinyRect& src1, const TinyRect& src2);

    private:
        /**
         * @brief Tests whether a rectangle has zero or negative area.
         *
         * @details A rectangle is considered empty when its width (Right - Left) or
         *          height (Bottom - Top) is less than or equal to zero.
         *
         * @param[in] rect Rectangle to test.
         * @return @c true if the rectangle is empty; @c false otherwise.
         *
         * @note Status: Ported
         */
        [[nodiscard]] static bool IsRectEmpty(const TinyRect& rect);
    };
}

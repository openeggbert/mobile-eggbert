/**
 * @file TinyRect.cpp
 * @brief Implementations of non-inline TinyRect methods.
 *
 * @warning NON-STANDARD FIELD ORDER.
 *          TinyRect stores and accepts its edges in the order
 *          @b Left, @b Right, @b Top, @b Bottom — NOT the conventional
 *          Left, Top, Right, Bottom order.  This order is preserved from the
 *          original C# WindowsPhoneSpeedyBlupi codebase.  Code that constructs
 *          or reads TinyRect values must account for this difference to avoid
 *          silent geometry bugs.
 */

#include "WindowsPhoneSpeedyBlupi/TinyRect.hpp"

namespace WindowsPhoneSpeedyBlupi
{
    TinyRect::TinyRect(TinyPoint point)
        : Left(point.X), Right(point.X), Top(point.Y), Bottom(point.Y)
    {
    }

    intcs TinyRect::getWidthProperty() const
    {
        return Right - Left;
    }

    intcs TinyRect::getHeightProperty() const
    {
        return Bottom - Top;
    }
}

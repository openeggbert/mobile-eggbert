#include "WindowsPhoneSpeedyBlupi/TinyRect.hpp"

namespace WindowsPhoneSpeedyBlupi
{
    intcs TinyRect::getWidthProperty() const
    {
        return Right - Left;
    }

    intcs TinyRect::getHeightProperty() const
    {
        return Bottom - Top;
    }
}
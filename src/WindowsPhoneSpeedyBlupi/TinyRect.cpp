#include "WindowsPhoneSpeedyBlupi/TinyRect.hpp"
namespace WindowsPhoneSpeedyBlupi {

    int TinyRect::getWidthProperty() const { return RightX - LeftX;  }
    int TinyRect::getHeightProperty() const { return BottomY - TopY; }
}
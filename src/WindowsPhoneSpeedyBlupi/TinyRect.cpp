#include "WindowsPhoneSpeedyBlupi/TinyRect.h"
namespace WindowsPhoneSpeedyBlupi {

    int TinyRect::getWidth() const { return RightX - LeftX;  }
    int TinyRect::getHeight() const { return BottomY - TopY; }
}
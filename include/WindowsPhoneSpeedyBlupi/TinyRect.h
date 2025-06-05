//todo: rename members: remove Left, Right, Top, Bottom

#ifndef TINYRECT_H
#define TINYRECT_H

#include <string>
#include <sstream>

namespace WindowsPhoneSpeedyBlupi
{
    struct TinyRect
    {
        int LeftX;
        int RightX;
        int TopY;
        int BottomY;

        TinyRect()
            : LeftX(0), RightX(0), TopY(0), BottomY(0)
        {}

        TinyRect(int leftX, int rightX, int topY, int bottomY)
            : LeftX(leftX), RightX(rightX), TopY(topY), BottomY(bottomY)
        {

        }

        public: [[nodiscard]] int getWidthProperty() const;
        public: [[nodiscard]] int getHeightProperty() const;

        std::string ToString() const
        {;
            std::ostringstream oss;
            oss << LeftX << ";" << TopY << ";" << RightX << ";" << BottomY;
            return oss.str();
        }
    };
}

#endif // TINYRECT_H

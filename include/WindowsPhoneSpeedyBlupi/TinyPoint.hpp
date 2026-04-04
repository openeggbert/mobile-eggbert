#ifndef TINYPOINT_H
#define TINYPOINT_H

#include <string>
#include <sstream>

namespace WindowsPhoneSpeedyBlupi
{
    struct TinyPoint
    {
        int X;
        int Y;

        TinyPoint()
            : X(0), Y(0)
        {
        }
        TinyPoint(int x, int y)
            : X(x), Y(y)
        {
        }

        std::string ToString() const
        {
            std::ostringstream oss;
            oss << X << ";" << Y;
            return oss.str();
        }
    };
}

#endif // TINYPOINT_H

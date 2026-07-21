// SPDX-License-Identifier: MS-PL

#include "Microsoft/Xna/Framework/Rectangle.hpp"

#include <sstream>

namespace Microsoft::Xna::Framework
{
    Rectangle::Rectangle()
        : Rectangle(0, 0, 0, 0)
    {
    }

    Rectangle::Rectangle(intcs x, intcs y, intcs width, intcs height)
        : X(x),
          Y(y),
          Width(width),
          Height(height)
    {
    }

    intcs Rectangle::getLeftProperty() const
    {
        return X;
    }

    intcs Rectangle::getTopProperty() const
    {
        return Y;
    }

    bool Rectangle::Contains(intcs x, intcs y) const
    {
        return X <= x &&
            x < X + Width &&
            Y <= y &&
            y < Y + Height;
    }

    std::string Rectangle::getDebugDisplayStringProperty() const
    {
        std::ostringstream stream;
        stream << X << " " << Y << " " << Width << " " << Height;
        return stream.str();
    }

    bool operator==(Rectangle value1, Rectangle value2)
    {
        return value1.X == value2.X &&
            value1.Y == value2.Y &&
            value1.Width == value2.Width &&
            value1.Height == value2.Height;
    }

    bool operator!=(Rectangle value1, Rectangle value2)
    {
        return !(value1 == value2);
    }
}

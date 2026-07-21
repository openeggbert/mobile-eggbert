// SPDX-License-Identifier: MS-PL

#include "Microsoft/Xna/Framework/Quaternion.hpp"

#include <cmath>
#include <sstream>
#include <stdexcept>

namespace Microsoft::Xna::Framework
{
    const Quaternion Quaternion::Identity(0.0f, 0.0f, 0.0f, 1.0f);

    Quaternion::Quaternion(float x, float y, float z, float w)
        : X(x), Y(y), Z(z), W(w)
    {
    }

    Quaternion::Quaternion(Vector3 vectorPart, float scalarPart)
        : X(vectorPart.X), Y(vectorPart.Y), Z(vectorPart.Z), W(scalarPart)
    {
    }

    bool Quaternion::Equals(const Quaternion& other) const
    {
        return X == other.X &&
            Y == other.Y &&
            Z == other.Z &&
            W == other.W;
    }

    std::string Quaternion::getDebugDisplayStringProperty() const
    {
        if (*this == Quaternion::Identity)
        {
            return "Identity";
        }

        std::ostringstream stream;
        stream << X << " " << Y << " " << Z << " " << W;
        return stream.str();
    }

    void Quaternion::CheckForNaNs() const
    {
#if !defined(NDEBUG)
        if (std::isnan(X) || std::isnan(Y) || std::isnan(Z) || std::isnan(W))
        {
            throw std::logic_error("Quaternion contains NaNs!");
        }
#endif
    }

    bool operator==(Quaternion quaternion1, Quaternion quaternion2)
    {
        return quaternion1.Equals(quaternion2);
    }
}

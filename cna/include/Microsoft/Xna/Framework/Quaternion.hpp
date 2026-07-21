// SPDX-License-Identifier: MS-PL

#pragma once

#include <string>

#include "Microsoft/Xna/Framework/Vector3.hpp"

namespace Microsoft::Xna::Framework
{
    /** @brief Efficient value type used to represent 3D rotations. */
    struct Quaternion
    {
        /** @brief Quaternion that represents no rotation. */
        static const Quaternion Identity;

        /** @brief X component of the quaternion vector part. */
        float X;

        /** @brief Y component of the quaternion vector part. */
        float Y;

        /** @brief Z component of the quaternion vector part. */
        float Z;

        /** @brief Scalar rotation component. */
        float W;

        /**
         * @brief Constructs a quaternion from four scalar components.
         *
         * @param x The X component.
         * @param y The Y component.
         * @param z The Z component.
         * @param w The W (scalar) component.
         */
        Quaternion(float x, float y, float z, float w);

        /**
         * @brief Constructs a quaternion from a vector part and a scalar part.
         *
         * @param vectorPart The vector part (X, Y, Z components).
         * @param scalarPart The scalar part (W component).
         */
        Quaternion(Vector3 vectorPart, float scalarPart);

        /**
         * @brief Compares this quaternion with another quaternion.
         *
         * @param other The quaternion to compare against.
         * @return @c true if the quaternions are equal; @c false otherwise.
         */
        [[nodiscard]] bool Equals(const Quaternion& other) const;

        /**
         * @brief Returns true when all four components are equal.
         *
         * @param quaternion1 Left-hand quaternion.
         * @param quaternion2 Right-hand quaternion.
         * @return @c true if the quaternions are equal; @c false otherwise.
         */
        friend bool operator==(Quaternion quaternion1, Quaternion quaternion2);

    private:
        [[nodiscard]] std::string getDebugDisplayStringProperty() const;
        void CheckForNaNs() const;
    };
}

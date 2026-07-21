// SPDX-License-Identifier: MS-PL

#pragma once

#include <string>
#include <vector>

#include "CNA/CNAHelper.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

namespace Microsoft::Xna::Framework
{
    struct Matrix;

    /** @brief Describes a three-component floating point vector. */
    struct Vector3
    {
        /** @brief Vector with all components set to zero. */
        static const Vector3 Zero;
        /** @brief Vector with all components set to one. */
        static const Vector3 One;
        /** @brief Unit vector pointing along the positive X axis. */
        static const Vector3 UnitX;
        /** @brief Unit vector pointing along the positive Y axis. */
        static const Vector3 UnitY;
        /** @brief Unit vector pointing along the positive Z axis. */
        static const Vector3 UnitZ;
        /** @brief Unit vector pointing up (0, 1, 0). */
        static const Vector3 Up;
        /** @brief Unit vector pointing down (0, -1, 0). */
        static const Vector3 Down;
        /** @brief Unit vector pointing right (1, 0, 0). */
        static const Vector3 Right;
        /** @brief Unit vector pointing left (-1, 0, 0). */
        static const Vector3 Left;
        /** @brief Unit vector pointing forward in a right-handed coordinate system (0, 0, -1). */
        static const Vector3 Forward;
        /** @brief Unit vector pointing backward in a right-handed coordinate system (0, 0, 1). */
        static const Vector3 Backward;

        /** @brief X component. */
        float X;

        /** @brief Y component. */
        float Y;

        /** @brief Z component. */
        float Z;

        /** @brief Creates a zero vector. */
        Vector3();

        /**
         * @brief Creates a vector from three component values.
         *
         * @param x The X component.
         * @param y The Y component.
         * @param z The Z component.
         */
        Vector3(float x, float y, float z);

        /**
         * @brief Creates a vector with all components set to the same value.
         *
         * @param value The value to assign to all components.
         */
        explicit Vector3(float value);

        /**
         * @brief Creates a vector from a Vector2 and a Z value.
         *
         * @param value The X and Y components.
         * @param z The Z component.
         */
        Vector3(Vector2 value, float z);

        /**
         * @brief Compares this vector with another for equality.
         *
         * @param other The vector to compare against.
         * @return @c true if the vectors are equal; @c false otherwise.
         */
        [[nodiscard]] bool Equals(const Vector3& other) const;

        /**
         * @brief Returns a hash code for this vector.
         *
         * @return Hash code of this vector.
         */
        [[nodiscard]] int GetHashCode() const;

        /**
         * @brief Returns the length of this vector.
         *
         * @return The length of this vector.
         */
        [[nodiscard]] float Length() const;

        /**
         * @brief Returns the squared length of this vector.
         *
         * @return The squared length of this vector.
         */
        [[nodiscard]] float LengthSquared() const;

        /** @brief Turns this vector into a unit vector with the same direction. */
        void Normalize();

        /**
         * @brief Returns a string in the form {X:... Y:... Z:...}.
         *
         * @return String representation of this vector.
         */
        [[nodiscard]] std::string ToString() const;

        /**
         * @brief Adds another vector to this vector in place.
         *
         * @param vector3 The vector to add.
         * @return Reference to this vector after addition.
         */
        NOXNA Vector3& operator+=(const Vector3& vector3);

        /**
         * @brief Subtracts another vector from this vector in place.
         *
         * @param vector3 The vector to subtract.
         * @return Reference to this vector after subtraction.
         */
        NOXNA Vector3& operator-=(const Vector3& vector3);

        /**
         * @brief Adds two vectors.
         *
         * @param value1 Source vector.
         * @param value2 Source vector.
         * @return Sum of the vectors.
         */
        [[nodiscard]] static Vector3 Add(Vector3 value1, Vector3 value2);

        /**
         * @brief Stores the sum of two vectors in an output parameter.
         *
         * @param value1 Source vector.
         * @param value2 Source vector.
         * @param result Output vector that receives the sum.
         */
        static void Add(const Vector3& value1, const Vector3& value2, Vector3& result);

        /**
         * @brief Returns the cross product of two vectors.
         *
         * @param vector1 Source vector.
         * @param vector2 Source vector.
         * @return The cross product of the two vectors.
         */
        [[nodiscard]] static Vector3 Cross(Vector3 vector1, Vector3 vector2);

        /**
         * @brief Stores the cross product of two vectors in an output parameter.
         *
         * @param vector1 Source vector.
         * @param vector2 Source vector.
         * @param result Output vector that receives the cross product.
         */
        static void Cross(const Vector3& vector1, const Vector3& vector2, Vector3& result);

        /**
         * @brief Divides a vector component-wise by another vector.
         *
         * @param value1 Source vector.
         * @param value2 Divisor vector.
         * @return The component-wise quotient.
         */
        [[nodiscard]] static Vector3 Divide(Vector3 value1, Vector3 value2);

        /**
         * @brief Stores the component-wise division result in an output parameter.
         *
         * @param value1 Source vector.
         * @param value2 Divisor vector.
         * @param result Output vector that receives the quotient.
         */
        static void Divide(const Vector3& value1, const Vector3& value2, Vector3& result);

        /**
         * @brief Divides a vector by a scalar.
         *
         * @param value1 Source vector.
         * @param divider Divisor scalar.
         * @return The scaled vector.
         */
        [[nodiscard]] static Vector3 Divide(Vector3 value1, float divider);

        /**
         * @brief Stores the scalar division result in an output parameter.
         *
         * @param value1 Source vector.
         * @param divider Divisor scalar.
         * @param result Output vector that receives the result.
         */
        static void Divide(const Vector3& value1, float divider, Vector3& result);

        /**
         * @brief Multiplies vectors component-wise.
         *
         * @param value1 Source vector.
         * @param value2 Source vector.
         * @return The component-wise product.
         */
        [[nodiscard]] static Vector3 Multiply(Vector3 value1, Vector3 value2);

        /**
         * @brief Stores the component-wise product in an output parameter.
         *
         * @param value1 Source vector.
         * @param value2 Source vector.
         * @param result Output vector that receives the product.
         */
        static void Multiply(const Vector3& value1, const Vector3& value2, Vector3& result);

        /**
         * @brief Multiplies a vector by a scalar.
         *
         * @param value1 Source vector.
         * @param scaleFactor Scalar multiplier.
         * @return The scaled vector.
         */
        [[nodiscard]] static Vector3 Multiply(Vector3 value1, float scaleFactor);

        /**
         * @brief Stores the scalar product in an output parameter.
         *
         * @param value1 Source vector.
         * @param scaleFactor Scalar multiplier.
         * @param result Output vector that receives the product.
         */
        static void Multiply(const Vector3& value1, float scaleFactor, Vector3& result);

        /**
         * @brief Negates all vector components.
         *
         * @param value Source vector.
         * @return A vector with negated components.
         */
        [[nodiscard]] static Vector3 Negate(Vector3 value);

        /**
         * @brief Stores the negated vector in an output parameter.
         *
         * @param value Source vector.
         * @param result Output vector that receives the negated components.
         */
        static void Negate(const Vector3& value, Vector3& result);

        /**
         * @brief Returns a normalized copy of a vector.
         *
         * @param value Source vector.
         * @return The normalized vector.
         */
        [[nodiscard]] static Vector3 Normalize(Vector3 value);

        /**
         * @brief Stores the normalized vector in an output parameter.
         *
         * @param value Source vector.
         * @param result Output vector that receives the normalized result.
         */
        static void Normalize(const Vector3& value, Vector3& result);

        /**
         * @brief Subtracts one vector from another.
         *
         * @param value1 Source vector.
         * @param value2 Vector to subtract.
         * @return The difference vector.
         */
        [[nodiscard]] static Vector3 Subtract(Vector3 value1, Vector3 value2);

        /**
         * @brief Stores the difference of two vectors in an output parameter.
         *
         * @param value1 Source vector.
         * @param value2 Vector to subtract.
         * @param result Output vector that receives the difference.
         */
        static void Subtract(const Vector3& value1, const Vector3& value2, Vector3& result);

        /**
         * @brief Transforms a position by a matrix.
         *
         * @param position Source position vector.
         * @param matrix The transformation matrix.
         * @return The transformed position.
         */
        [[nodiscard]] static Vector3 Transform(Vector3 position, const Matrix& matrix);

        /**
         * @brief Stores the matrix-transformed position in an output parameter.
         *
         * @param position Source position vector.
         * @param matrix The transformation matrix.
         * @param result Output vector that receives the transformed position.
         */
        static void Transform(const Vector3& position, const Matrix& matrix, Vector3& result);

        /**
         * @brief Transforms an array of positions by a matrix.
         *
         * @param sourceArray Source array of vectors.
         * @param matrix The transformation matrix.
         * @param destinationArray Output array that receives the transformed vectors.
         */
        static void Transform(const std::vector<Vector3>& sourceArray, const Matrix& matrix,
                              std::vector<Vector3>& destinationArray);

        /**
         * @brief Transforms a range of positions in an array by a matrix.
         *
         * @param sourceArray Source array of vectors.
         * @param sourceIndex Starting index in the source array.
         * @param matrix The transformation matrix.
         * @param destinationArray Output array that receives the transformed vectors.
         * @param destinationIndex Starting index in the destination array.
         * @param length Number of elements to transform.
         */
        static void Transform(const std::vector<Vector3>& sourceArray, int sourceIndex, const Matrix& matrix,
                              std::vector<Vector3>& destinationArray, int destinationIndex, int length);

        /**
         * @brief Returns true when all components are equal.
         *
         * @param value1 Left-hand vector.
         * @param value2 Right-hand vector.
         * @return @c true if the vectors are equal; @c false otherwise.
         */
        friend bool operator==(Vector3 value1, Vector3 value2);

        /**
         * @brief Returns true when any component differs.
         *
         * @param value1 Left-hand vector.
         * @param value2 Right-hand vector.
         * @return @c true if the vectors are not equal; @c false otherwise.
         */
        friend bool operator!=(Vector3 value1, Vector3 value2);

        /**
         * @brief Adds two vectors component-wise.
         *
         * @param value1 Left-hand vector.
         * @param value2 Right-hand vector.
         * @return The component-wise sum.
         */
        friend Vector3 operator+(Vector3 value1, Vector3 value2);

        /**
         * @brief Negates all components of a vector.
         *
         * @param value Source vector.
         * @return The negated vector.
         */
        friend Vector3 operator-(Vector3 value);

        /**
         * @brief Subtracts one vector from another component-wise.
         *
         * @param value1 Left-hand vector.
         * @param value2 Right-hand vector.
         * @return The component-wise difference.
         */
        friend Vector3 operator-(Vector3 value1, Vector3 value2);

        /**
         * @brief Multiplies two vectors component-wise.
         *
         * @param value1 Left-hand vector.
         * @param value2 Right-hand vector.
         * @return The component-wise product.
         */
        friend Vector3 operator*(Vector3 value1, Vector3 value2);

        /**
         * @brief Multiplies all components of a vector by a scalar.
         *
         * @param value Source vector.
         * @param scaleFactor Scalar multiplier.
         * @return The scaled vector.
         */
        friend Vector3 operator*(Vector3 value, float scaleFactor);

        /**
         * @brief Multiplies all components of a vector by a scalar.
         *
         * @param scaleFactor Scalar multiplier.
         * @param value Source vector.
         * @return The scaled vector.
         */
        friend Vector3 operator*(float scaleFactor, Vector3 value);

        /**
         * @brief Divides one vector by another component-wise.
         *
         * @param value1 Left-hand vector.
         * @param value2 Right-hand vector.
         * @return The component-wise quotient.
         */
        friend Vector3 operator/(Vector3 value1, Vector3 value2);

        /**
         * @brief Divides all components of a vector by a scalar.
         *
         * @param value1 Source vector.
         * @param divider Divisor scalar.
         * @return The scaled vector.
         */
        friend Vector3 operator/(Vector3 value1, float divider);

    private:
        [[nodiscard]] std::string getDebugDisplayStringProperty() const;
        void CheckForNaNs() const;
    };
}

// SPDX-License-Identifier: MS-PL

#pragma once

#include <string>
#include <vector>

namespace Microsoft::Xna::Framework
{
    struct Matrix;

    /** @brief Describes a two-component floating point vector. */
    struct Vector2
    {
        /** @brief Vector with both components set to zero. */
        static const Vector2 Zero;

        /** @brief Vector with both components set to one. */
        static const Vector2 One;

        /** @brief Unit vector on the X axis. */
        static const Vector2 UnitX;

        /** @brief Unit vector on the Y axis. */
        static const Vector2 UnitY;

        /** @brief X component. */
        float X;

        /** @brief Y component. */
        float Y;

        /** @brief Creates a zero vector. */
        Vector2();

        /**
         * @brief Creates a vector from two component values.
         *
         * @param x The X component.
         * @param y The Y component.
         */
        Vector2(float x, float y);

        /**
         * @brief Creates a vector whose components are both set to the same value.
         *
         * @param value The value to assign to both components.
         */
        explicit Vector2(float value);

        /**
         * @brief Compares this vector with another vector.
         *
         * @param other The vector to compare against.
         * @return @c true if the vectors are equal; @c false otherwise.
         */
        [[nodiscard]] bool Equals(const Vector2& other) const;

        /**
         * @brief Returns a hash code for this vector.
         *
         * @return Hash code of this vector.
         */
        [[nodiscard]] int GetHashCode() const;

        /**
         * @brief Returns the vector length.
         *
         * @return The length of this vector.
         */
        [[nodiscard]] float Length() const;

        /**
         * @brief Returns the squared vector length.
         *
         * @return The squared length of this vector.
         */
        [[nodiscard]] float LengthSquared() const;

        /** @brief Normalizes this vector in place. */
        void Normalize();

        /**
         * @brief Returns a string in the form {X:... Y:...}.
         *
         * @return String representation of this vector.
         */
        [[nodiscard]] std::string ToString() const;

        /**
         * @brief Adds two vectors.
         *
         * @param value1 Source vector.
         * @param value2 Source vector.
         * @return Sum of the vectors.
         */
        [[nodiscard]] static Vector2 Add(Vector2 value1, Vector2 value2);

        /**
         * @brief Stores the sum of two vectors in an output parameter.
         *
         * @param value1 Source vector.
         * @param value2 Source vector.
         * @param result Output vector that receives the sum.
         */
        static void Add(const Vector2& value1, const Vector2& value2, Vector2& result);

        /**
         * @brief Divides a vector component-wise by another vector.
         *
         * @param value1 Source vector.
         * @param value2 Divisor vector.
         * @return The component-wise quotient.
         */
        [[nodiscard]] static Vector2 Divide(Vector2 value1, Vector2 value2);

        /**
         * @brief Stores the component-wise division result in an output parameter.
         *
         * @param value1 Source vector.
         * @param value2 Divisor vector.
         * @param result Output vector that receives the quotient.
         */
        static void Divide(const Vector2& value1, const Vector2& value2, Vector2& result);

        /**
         * @brief Divides a vector by a scalar.
         *
         * @param value1 Source vector.
         * @param divider Divisor scalar.
         * @return The scaled vector.
         */
        [[nodiscard]] static Vector2 Divide(Vector2 value1, float divider);

        /**
         * @brief Stores the scalar division result in an output parameter.
         *
         * @param value1 Source vector.
         * @param divider Divisor scalar.
         * @param result Output vector that receives the result.
         */
        static void Divide(const Vector2& value1, float divider, Vector2& result);

        /**
         * @brief Multiplies vectors component-wise.
         *
         * @param value1 Source vector.
         * @param value2 Source vector.
         * @return The component-wise product.
         */
        [[nodiscard]] static Vector2 Multiply(Vector2 value1, Vector2 value2);

        /**
         * @brief Stores the component-wise product in an output parameter.
         *
         * @param value1 Source vector.
         * @param value2 Source vector.
         * @param result Output vector that receives the product.
         */
        static void Multiply(const Vector2& value1, const Vector2& value2, Vector2& result);

        /**
         * @brief Multiplies a vector by a scalar.
         *
         * @param value1 Source vector.
         * @param scaleFactor Scalar multiplier.
         * @return The scaled vector.
         */
        [[nodiscard]] static Vector2 Multiply(Vector2 value1, float scaleFactor);

        /**
         * @brief Stores the scalar product in an output parameter.
         *
         * @param value1 Source vector.
         * @param scaleFactor Scalar multiplier.
         * @param result Output vector that receives the product.
         */
        static void Multiply(const Vector2& value1, float scaleFactor, Vector2& result);

        /**
         * @brief Negates both vector components.
         *
         * @param value Source vector.
         * @return A vector with negated components.
         */
        [[nodiscard]] static Vector2 Negate(Vector2 value);

        /**
         * @brief Stores the negated vector in an output parameter.
         *
         * @param value Source vector.
         * @param result Output vector that receives the negated components.
         */
        static void Negate(const Vector2& value, Vector2& result);

        /**
         * @brief Returns a normalized copy of a vector.
         *
         * @param value Source vector.
         * @return The normalized vector.
         */
        [[nodiscard]] static Vector2 Normalize(Vector2 value);

        /**
         * @brief Stores the normalized vector in an output parameter.
         *
         * @param value Source vector.
         * @param result Output vector that receives the normalized result.
         */
        static void Normalize(const Vector2& value, Vector2& result);

        /**
         * @brief Subtracts one vector from another.
         *
         * @param value1 Source vector.
         * @param value2 Vector to subtract.
         * @return The difference vector.
         */
        [[nodiscard]] static Vector2 Subtract(Vector2 value1, Vector2 value2);

        /**
         * @brief Stores the difference of two vectors in an output parameter.
         *
         * @param value1 Source vector.
         * @param value2 Vector to subtract.
         * @param result Output vector that receives the difference.
         */
        static void Subtract(const Vector2& value1, const Vector2& value2, Vector2& result);

        /**
         * @brief Transforms a position by a matrix.
         *
         * @param position Source position vector.
         * @param matrix The transformation matrix.
         * @return The transformed position.
         */
        [[nodiscard]] static Vector2 Transform(Vector2 position, const Matrix& matrix);

        /**
         * @brief Stores the matrix-transformed position in an output parameter.
         *
         * @param position Source position vector.
         * @param matrix The transformation matrix.
         * @param result Output vector that receives the transformed position.
         */
        static void Transform(const Vector2& position, const Matrix& matrix, Vector2& result);

        /**
         * @brief Transforms an array of positions by a matrix.
         *
         * @param sourceArray Source array of vectors.
         * @param matrix The transformation matrix.
         * @param destinationArray Output array that receives the transformed vectors.
         */
        static void Transform(const std::vector<Vector2>& sourceArray, const Matrix& matrix,
                              std::vector<Vector2>& destinationArray);

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
        static void Transform(const std::vector<Vector2>& sourceArray, int sourceIndex, const Matrix& matrix,
                              std::vector<Vector2>& destinationArray, int destinationIndex, int length);

        /**
         * @brief Negates all components of a vector.
         *
         * @param value Source vector.
         * @return The negated vector.
         */
        friend Vector2 operator-(Vector2 value);

        /**
         * @brief Returns true when both vectors have equal components.
         *
         * @param value1 Left-hand vector.
         * @param value2 Right-hand vector.
         * @return @c true if the vectors are equal; @c false otherwise.
         */
        friend bool operator==(Vector2 value1, Vector2 value2);

        /**
         * @brief Returns true when any component differs.
         *
         * @param value1 Left-hand vector.
         * @param value2 Right-hand vector.
         * @return @c true if the vectors are not equal; @c false otherwise.
         */
        friend bool operator!=(Vector2 value1, Vector2 value2);

        /**
         * @brief Adds two vectors component-wise.
         *
         * @param value1 Left-hand vector.
         * @param value2 Right-hand vector.
         * @return The component-wise sum.
         */
        friend Vector2 operator+(Vector2 value1, Vector2 value2);

        /**
         * @brief Subtracts one vector from another component-wise.
         *
         * @param value1 Left-hand vector.
         * @param value2 Right-hand vector.
         * @return The component-wise difference.
         */
        friend Vector2 operator-(Vector2 value1, Vector2 value2);

        /**
         * @brief Multiplies two vectors component-wise.
         *
         * @param value1 Left-hand vector.
         * @param value2 Right-hand vector.
         * @return The component-wise product.
         */
        friend Vector2 operator*(Vector2 value1, Vector2 value2);

        /**
         * @brief Multiplies all components of a vector by a scalar.
         *
         * @param value Source vector.
         * @param scaleFactor Scalar multiplier.
         * @return The scaled vector.
         */
        friend Vector2 operator*(Vector2 value, float scaleFactor);

        /**
         * @brief Multiplies all components of a vector by a scalar.
         *
         * @param scaleFactor Scalar multiplier.
         * @param value Source vector.
         * @return The scaled vector.
         */
        friend Vector2 operator*(float scaleFactor, Vector2 value);

        /**
         * @brief Divides one vector by another component-wise.
         *
         * @param value1 Left-hand vector.
         * @param value2 Right-hand vector.
         * @return The component-wise quotient.
         */
        friend Vector2 operator/(Vector2 value1, Vector2 value2);

        /**
         * @brief Divides all components of a vector by a scalar.
         *
         * @param value1 Source vector.
         * @param divider Divisor scalar.
         * @return The scaled vector.
         */
        friend Vector2 operator/(Vector2 value1, float divider);

    private:
        [[nodiscard]] std::string getDebugDisplayStringProperty() const;
        void CheckForNaNs() const;
    };
}

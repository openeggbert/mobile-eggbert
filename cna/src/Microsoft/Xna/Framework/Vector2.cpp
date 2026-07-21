// SPDX-License-Identifier: MS-PL

#include "Microsoft/Xna/Framework/Vector2.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <sstream>
#include <stdexcept>

#include "Microsoft/Xna/Framework/Matrix.hpp"

namespace Microsoft::Xna::Framework
{
    namespace
    {
        int FloatHash(float value)
        {
            std::uint32_t bits = 0;
            static_assert(sizeof(bits) == sizeof(value));
            std::memcpy(&bits, &value, sizeof(value));
            return static_cast<int>(bits);
        }

        void CheckArrayRange(std::size_t sourceSize, int sourceIndex, std::size_t destinationSize, int destinationIndex,
                             int length)
        {
            if (sourceIndex < 0 || destinationIndex < 0 || length < 0)
            {
                throw std::out_of_range("array index and length values must be non-negative");
            }
            if (static_cast<std::size_t>(sourceIndex + length) > sourceSize)
            {
                throw std::out_of_range("source range exceeds source array size");
            }
            if (static_cast<std::size_t>(destinationIndex + length) > destinationSize)
            {
                throw std::out_of_range("destination range exceeds destination array size");
            }
        }
    }

    const Vector2 Vector2::Zero(0.0f, 0.0f);
    const Vector2 Vector2::One(1.0f, 1.0f);
    const Vector2 Vector2::UnitX(1.0f, 0.0f);
    const Vector2 Vector2::UnitY(0.0f, 1.0f);

    Vector2::Vector2() : X(0.0f), Y(0.0f)
    {
    }

    Vector2::Vector2(float x, float y) : X(x), Y(y)
    {
    }

    Vector2::Vector2(float value) : X(value), Y(value)
    {
    }

    bool Vector2::Equals(const Vector2& other) const { return X == other.X && Y == other.Y; }
    int Vector2::GetHashCode() const
    {
        // Unsigned wraparound avoids signed-overflow UB (UBSan, INPUT-BUILD-006); result unchanged.
        return static_cast<int>(static_cast<unsigned>(FloatHash(X)) + static_cast<unsigned>(FloatHash(Y)));
    }
    float Vector2::Length() const { return std::sqrt((X * X) + (Y * Y)); }
    float Vector2::LengthSquared() const { return (X * X) + (Y * Y); }

    void Vector2::Normalize()
    {
        const float val = 1.0f / std::sqrt((X * X) + (Y * Y));
        X *= val;
        Y *= val;
    }

    std::string Vector2::ToString() const
    {
        std::ostringstream s;
        s << "{X:" << X << " Y:" << Y << "}";
        return s.str();
    }

    std::string Vector2::getDebugDisplayStringProperty() const
    {
        std::ostringstream s;
        s << X << " " << Y;
        return s.str();
    }

    void Vector2::CheckForNaNs() const
    {
        if (std::isnan(X) || std::isnan(Y)) throw std::logic_error("Vector2 contains NaNs!");
    }

    Vector2 Vector2::Add(Vector2 value1, Vector2 value2)
    {
        value1.X += value2.X;
        value1.Y += value2.Y;
        return value1;
    }

    void Vector2::Add(const Vector2& value1, const Vector2& value2, Vector2& result)
    {
        result.X = value1.X + value2.X;
        result.Y = value1.Y + value2.Y;
    }

    Vector2 Vector2::Divide(Vector2 value1, Vector2 value2)
    {
        value1.X /= value2.X;
        value1.Y /= value2.Y;
        return value1;
    }

    void Vector2::Divide(const Vector2& value1, const Vector2& value2, Vector2& result)
    {
        result.X = value1.X / value2.X;
        result.Y = value1.Y / value2.Y;
    }

    Vector2 Vector2::Divide(Vector2 value1, float divider)
    {
        value1.X /= divider;
        value1.Y /= divider;
        return value1;
    }

    void Vector2::Divide(const Vector2& value1, float divider, Vector2& result)
    {
        result.X = value1.X / divider;
        result.Y = value1.Y / divider;
    }

    Vector2 Vector2::Multiply(Vector2 value1, Vector2 value2)
    {
        value1.X *= value2.X;
        value1.Y *= value2.Y;
        return value1;
    }

    void Vector2::Multiply(const Vector2& value1, const Vector2& value2, Vector2& result)
    {
        result.X = value1.X * value2.X;
        result.Y = value1.Y * value2.Y;
    }

    Vector2 Vector2::Multiply(Vector2 value1, float scaleFactor)
    {
        value1.X *= scaleFactor;
        value1.Y *= scaleFactor;
        return value1;
    }

    void Vector2::Multiply(const Vector2& value1, float scaleFactor, Vector2& result)
    {
        result.X = value1.X * scaleFactor;
        result.Y = value1.Y * scaleFactor;
    }

    Vector2 Vector2::Negate(Vector2 value) { return Vector2(-value.X, -value.Y); }
    void Vector2::Negate(const Vector2& value, Vector2& result) { result = Negate(value); }

    Vector2 Vector2::Normalize(Vector2 value)
    {
        value.Normalize();
        return value;
    }

    void Vector2::Normalize(const Vector2& value, Vector2& result) { result = Normalize(value); }

    Vector2 Vector2::Subtract(Vector2 value1, Vector2 value2)
    {
        value1.X -= value2.X;
        value1.Y -= value2.Y;
        return value1;
    }

    void Vector2::Subtract(const Vector2& value1, const Vector2& value2, Vector2& result)
    {
        result.X = value1.X - value2.X;
        result.Y = value1.Y - value2.Y;
    }

    Vector2 Vector2::Transform(Vector2 position, const Matrix& matrix)
    {
        Vector2 result;
        Transform(position, matrix, result);
        return result;
    }

    void Vector2::Transform(const Vector2& position, const Matrix& matrix, Vector2& result)
    {
        const float x = (position.X * matrix.M11) + (position.Y * matrix.M21) + matrix.M41;
        const float y = (position.X * matrix.M12) + (position.Y * matrix.M22) + matrix.M42;
        result.X = x;
        result.Y = y;
    }

    void Vector2::Transform(const std::vector<Vector2>& sourceArray, const Matrix& matrix,
                            std::vector<Vector2>& destinationArray)
    {
        Transform(sourceArray, 0, matrix, destinationArray, 0, static_cast<int>(sourceArray.size()));
    }

    void Vector2::Transform(const std::vector<Vector2>& sourceArray, int sourceIndex, const Matrix& matrix,
                            std::vector<Vector2>& destinationArray, int destinationIndex, int length)
    {
        CheckArrayRange(sourceArray.size(), sourceIndex, destinationArray.size(), destinationIndex, length);
        for (int i = 0; i < length; ++i) Transform(sourceArray[sourceIndex + i], matrix,
                                                   destinationArray[destinationIndex + i]);
    }

    Vector2 operator-(Vector2 value) { return Vector2::Negate(value); }
    bool operator==(Vector2 value1, Vector2 value2) { return value1.Equals(value2); }
    bool operator!=(Vector2 value1, Vector2 value2) { return !value1.Equals(value2); }
    Vector2 operator+(Vector2 value1, Vector2 value2) { return Vector2::Add(value1, value2); }
    Vector2 operator-(Vector2 value1, Vector2 value2) { return Vector2::Subtract(value1, value2); }
    Vector2 operator*(Vector2 value1, Vector2 value2) { return Vector2::Multiply(value1, value2); }
    Vector2 operator*(Vector2 value, float scaleFactor) { return Vector2::Multiply(value, scaleFactor); }
    Vector2 operator*(float scaleFactor, Vector2 value) { return Vector2::Multiply(value, scaleFactor); }
    Vector2 operator/(Vector2 value1, Vector2 value2) { return Vector2::Divide(value1, value2); }
    Vector2 operator/(Vector2 value1, float divider) { return Vector2::Divide(value1, divider); }
}

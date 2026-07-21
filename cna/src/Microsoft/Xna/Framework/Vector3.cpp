// SPDX-License-Identifier: MS-PL

#include "Microsoft/Xna/Framework/Vector3.hpp"
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

    const Vector3 Vector3::Zero(0.0f, 0.0f, 0.0f);
    const Vector3 Vector3::One(1.0f, 1.0f, 1.0f);
    const Vector3 Vector3::UnitX(1.0f, 0.0f, 0.0f);
    const Vector3 Vector3::UnitY(0.0f, 1.0f, 0.0f);
    const Vector3 Vector3::UnitZ(0.0f, 0.0f, 1.0f);
    const Vector3 Vector3::Up(0.0f, 1.0f, 0.0f);
    const Vector3 Vector3::Down(0.0f, -1.0f, 0.0f);
    const Vector3 Vector3::Right(1.0f, 0.0f, 0.0f);
    const Vector3 Vector3::Left(-1.0f, 0.0f, 0.0f);
    const Vector3 Vector3::Forward(0.0f, 0.0f, -1.0f);
    const Vector3 Vector3::Backward(0.0f, 0.0f, 1.0f);

    Vector3::Vector3() : X(0.0f), Y(0.0f), Z(0.0f)
    {
    }

    Vector3::Vector3(float x, float y, float z) : X(x), Y(y), Z(z)
    {
    }

    Vector3::Vector3(float value) : X(value), Y(value), Z(value)
    {
    }

    Vector3::Vector3(Vector2 value, float z) : X(value.X), Y(value.Y), Z(z)
    {
    }

    bool Vector3::Equals(const Vector3& other) const { return X == other.X && Y == other.Y && Z == other.Z; }
    int Vector3::GetHashCode() const { return FloatHash(X) + FloatHash(Y) + FloatHash(Z); }
    float Vector3::Length() const { return std::sqrt((X * X) + (Y * Y) + (Z * Z)); }
    float Vector3::LengthSquared() const { return (X * X) + (Y * Y) + (Z * Z); }

    void Vector3::Normalize()
    {
        const float factor = 1.0f / std::sqrt((X * X) + (Y * Y) + (Z * Z));
        X *= factor;
        Y *= factor;
        Z *= factor;
    }

    std::string Vector3::ToString() const
    {
        std::ostringstream s;
        s << "{X:" << X << " Y:" << Y << " Z:" << Z << "}";
        return s.str();
    }

    Vector3& Vector3::operator+=(const Vector3& vector3)
    {
        X += vector3.X;
        Y += vector3.Y;
        Z += vector3.Z;
        return *this;
    }

    Vector3& Vector3::operator-=(const Vector3& vector3)
    {
        X -= vector3.X;
        Y -= vector3.Y;
        Z -= vector3.Z;
        return *this;
    }

    std::string Vector3::getDebugDisplayStringProperty() const
    {
        std::ostringstream s;
        s << X << " " << Y << " " << Z;
        return s.str();
    }

    void Vector3::CheckForNaNs() const
    {
        if (std::isnan(X) || std::isnan(Y) || std::isnan(Z)) throw std::logic_error("Vector3 contains NaNs!");
    }

    Vector3 Vector3::Add(Vector3 value1, Vector3 value2)
    {
        value1.X += value2.X;
        value1.Y += value2.Y;
        value1.Z += value2.Z;
        return value1;
    }

    void Vector3::Add(const Vector3& value1, const Vector3& value2, Vector3& result)
    {
        result.X = value1.X + value2.X;
        result.Y = value1.Y + value2.Y;
        result.Z = value1.Z + value2.Z;
    }

    Vector3 Vector3::Cross(Vector3 vector1, Vector3 vector2)
    {
        Vector3 result;
        Cross(vector1, vector2, result);
        return result;
    }

    void Vector3::Cross(const Vector3& vector1, const Vector3& vector2, Vector3& result)
    {
        const float x = (vector1.Y * vector2.Z) - (vector2.Y * vector1.Z);
        const float y = -((vector1.X * vector2.Z) - (vector2.X * vector1.Z));
        const float z = (vector1.X * vector2.Y) - (vector2.X * vector1.Y);
        result.X = x;
        result.Y = y;
        result.Z = z;
    }

    Vector3 Vector3::Divide(Vector3 value1, Vector3 value2)
    {
        value1.X /= value2.X;
        value1.Y /= value2.Y;
        value1.Z /= value2.Z;
        return value1;
    }

    void Vector3::Divide(const Vector3& value1, const Vector3& value2, Vector3& result)
    {
        result.X = value1.X / value2.X;
        result.Y = value1.Y / value2.Y;
        result.Z = value1.Z / value2.Z;
    }

    Vector3 Vector3::Divide(Vector3 value1, float divider)
    {
        value1.X /= divider;
        value1.Y /= divider;
        value1.Z /= divider;
        return value1;
    }

    void Vector3::Divide(const Vector3& value1, float divider, Vector3& result)
    {
        result.X = value1.X / divider;
        result.Y = value1.Y / divider;
        result.Z = value1.Z / divider;
    }

    Vector3 Vector3::Multiply(Vector3 value1, Vector3 value2)
    {
        value1.X *= value2.X;
        value1.Y *= value2.Y;
        value1.Z *= value2.Z;
        return value1;
    }

    void Vector3::Multiply(const Vector3& value1, const Vector3& value2, Vector3& result)
    {
        result.X = value1.X * value2.X;
        result.Y = value1.Y * value2.Y;
        result.Z = value1.Z * value2.Z;
    }

    Vector3 Vector3::Multiply(Vector3 value1, float scaleFactor)
    {
        value1.X *= scaleFactor;
        value1.Y *= scaleFactor;
        value1.Z *= scaleFactor;
        return value1;
    }

    void Vector3::Multiply(const Vector3& value1, float scaleFactor, Vector3& result)
    {
        result.X = value1.X * scaleFactor;
        result.Y = value1.Y * scaleFactor;
        result.Z = value1.Z * scaleFactor;
    }

    Vector3 Vector3::Negate(Vector3 value) { return Vector3(-value.X, -value.Y, -value.Z); }
    void Vector3::Negate(const Vector3& value, Vector3& result) { result = Negate(value); }

    Vector3 Vector3::Normalize(Vector3 value)
    {
        value.Normalize();
        return value;
    }

    void Vector3::Normalize(const Vector3& value, Vector3& result) { result = Normalize(value); }

    Vector3 Vector3::Subtract(Vector3 value1, Vector3 value2)
    {
        value1.X -= value2.X;
        value1.Y -= value2.Y;
        value1.Z -= value2.Z;
        return value1;
    }

    void Vector3::Subtract(const Vector3& value1, const Vector3& value2, Vector3& result)
    {
        result.X = value1.X - value2.X;
        result.Y = value1.Y - value2.Y;
        result.Z = value1.Z - value2.Z;
    }

    Vector3 Vector3::Transform(Vector3 position, const Matrix& matrix)
    {
        Vector3 result;
        Transform(position, matrix, result);
        return result;
    }

    void Vector3::Transform(const Vector3& position, const Matrix& matrix, Vector3& result)
    {
        const float x = (position.X * matrix.M11) + (position.Y * matrix.M21) + (position.Z * matrix.M31) + matrix.M41;
        const float y = (position.X * matrix.M12) + (position.Y * matrix.M22) + (position.Z * matrix.M32) + matrix.M42;
        const float z = (position.X * matrix.M13) + (position.Y * matrix.M23) + (position.Z * matrix.M33) + matrix.M43;
        result.X = x;
        result.Y = y;
        result.Z = z;
    }

    void Vector3::Transform(const std::vector<Vector3>& sourceArray, const Matrix& matrix,
                            std::vector<Vector3>& destinationArray)
    {
        Transform(sourceArray, 0, matrix, destinationArray, 0, static_cast<int>(sourceArray.size()));
    }

    void Vector3::Transform(const std::vector<Vector3>& sourceArray, int sourceIndex, const Matrix& matrix,
                            std::vector<Vector3>& destinationArray, int destinationIndex, int length)
    {
        CheckArrayRange(sourceArray.size(), sourceIndex, destinationArray.size(), destinationIndex, length);
        for (int i = 0; i < length; ++i) Transform(sourceArray[sourceIndex + i], matrix,
                                                   destinationArray[destinationIndex + i]);
    }

    bool operator==(Vector3 value1, Vector3 value2) { return value1.Equals(value2); }
    bool operator!=(Vector3 value1, Vector3 value2) { return !value1.Equals(value2); }
    Vector3 operator+(Vector3 value1, Vector3 value2) { return Vector3::Add(value1, value2); }
    Vector3 operator-(Vector3 value) { return Vector3::Negate(value); }
    Vector3 operator-(Vector3 value1, Vector3 value2) { return Vector3::Subtract(value1, value2); }
    Vector3 operator*(Vector3 value1, Vector3 value2) { return Vector3::Multiply(value1, value2); }
    Vector3 operator*(Vector3 value, float scaleFactor) { return Vector3::Multiply(value, scaleFactor); }
    Vector3 operator*(float scaleFactor, Vector3 value) { return Vector3::Multiply(value, scaleFactor); }
    Vector3 operator/(Vector3 value1, Vector3 value2) { return Vector3::Divide(value1, value2); }
    Vector3 operator/(Vector3 value1, float divider) { return Vector3::Divide(value1, divider); }
}

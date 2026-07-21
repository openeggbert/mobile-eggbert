// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <cmath>
#include <locale>
#include <sstream>
#include <string>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentOutOfRangeException.hpp"

namespace System::Numerics {

// Forward declarations
struct Matrix3x2;
struct Matrix4x4;

/** A structure encapsulating two single-precision floating-point values (X, Y). */
struct Vector2 {
    float X{0.0f}; ///< The X component.
    float Y{0.0f}; ///< The Y component.

    /** Constructs a zero vector. */
    Vector2() = default;
    /** Constructs a vector with both components set to @p value. */
    explicit Vector2(float value) : X(value), Y(value) {}
    /** Constructs a vector from individual components. */
    Vector2(float x, float y) : X(x), Y(y) {}

    /** @return The vector (0, 0). */
    static Vector2 Zero()  { return {0.0f, 0.0f}; }
    /** @return The vector (1, 1). */
    static Vector2 One()   { return {1.0f, 1.0f}; }
    /** @return The unit vector along the X axis (1, 0). */
    static Vector2 UnitX() { return {1.0f, 0.0f}; }
    /** @return The unit vector along the Y axis (0, 1). */
    static Vector2 UnitY() { return {0.0f, 1.0f}; }

    /** @return The Euclidean length of this vector. */
    [[nodiscard]] float Length()        const { return std::sqrt(LengthSquared()); }
    /** @return The squared Euclidean length, avoiding a sqrt call. */
    [[nodiscard]] float LengthSquared() const { return X * X + Y * Y; }

    /**
     * Copies the components into a caller-provided array of at least 2 floats.
     * @param arr  Destination array; must have room for at least 2 elements.
     */
    void CopyTo(float* arr) const { arr[0] = X; arr[1] = Y; }

    /** @return True if this vector is component-wise equal to @p other. */
    [[nodiscard]] bool Equals(const Vector2& other) const { return X == other.X && Y == other.Y; }

    /** @return A string of the form "&lt;X, Y&gt;" using the invariant locale. */
    std::string ToString() const {
        std::ostringstream ss;
        ss.imbue(std::locale::classic());
        ss << "<" << X << ", " << Y << ">";
        return ss.str();
    }

    /** @return The component at @p index (0 = X, 1 = Y). @throws System::ArgumentOutOfRangeException on invalid index. */
    float operator[](SharpRuntime::intcs index) const {
        if (index == 0) return X;
        if (index == 1) return Y;
        throw System::ArgumentOutOfRangeException("index");
    }
    /** @return A reference to the component at @p index (0 = X, 1 = Y). @throws System::ArgumentOutOfRangeException on invalid index. */
    float& operator[](SharpRuntime::intcs index) {
        if (index == 0) return X;
        if (index == 1) return Y;
        throw System::ArgumentOutOfRangeException("index");
    }

    /** Component-wise addition. */
    Vector2 operator+(const Vector2& r) const { return {X + r.X, Y + r.Y}; }
    /** Component-wise subtraction. */
    Vector2 operator-(const Vector2& r) const { return {X - r.X, Y - r.Y}; }
    /** Component-wise multiplication. */
    Vector2 operator*(const Vector2& r) const { return {X * r.X, Y * r.Y}; }
    /** Scalar multiplication. */
    Vector2 operator*(float s)          const { return {X * s,   Y * s};   }
    /** Component-wise division. */
    Vector2 operator/(const Vector2& r) const { return {X / r.X, Y / r.Y}; }
    /** Scalar division. */
    Vector2 operator/(float s)          const { return {X / s,   Y / s};   }
    /** Unary negation. */
    Vector2 operator-()                 const { return {-X, -Y}; }
    /** Equality comparison (exact floating-point). */
    bool    operator==(const Vector2& r) const { return Equals(r); }
    /** Inequality comparison. */
    bool    operator!=(const Vector2& r) const { return !Equals(r); }
    /** Scalar * vector multiplication (commutative). */
    friend Vector2 operator*(float s, const Vector2& v) { return v * s; }

    /** @return Component-wise absolute value of @p v. */
    static Vector2 Abs(Vector2 v)                          { return {std::abs(v.X), std::abs(v.Y)}; }
    /** @return Component-wise sum a + b. */
    static Vector2 Add(Vector2 a, Vector2 b)               { return a + b; }
    /** @return Component-wise difference a - b. */
    static Vector2 Subtract(Vector2 a, Vector2 b)          { return a - b; }
    /** @return Component-wise product a * b. */
    static Vector2 Multiply(Vector2 a, Vector2 b)          { return a * b; }
    /** @return Vector @p v scaled by scalar @p s. */
    static Vector2 Multiply(Vector2 v, float s)            { return v * s; }
    /** @return Scalar @p s multiplied by vector @p v. */
    static Vector2 Multiply(float s, Vector2 v)            { return v * s; }
    /** @return Component-wise quotient a / b. */
    static Vector2 Divide(Vector2 a, Vector2 b)            { return a / b; }
    /** @return Vector @p v divided by scalar @p s. */
    static Vector2 Divide(Vector2 v, float s)              { return v / s; }
    /** @return Negated vector -v. */
    static Vector2 Negate(Vector2 v)                       { return -v; }

    /** @return Dot product a·b = aX*bX + aY*bY. */
    static float   Dot(Vector2 a, Vector2 b)               { return a.X * b.X + a.Y * b.Y; }
    /** @return 2-D cross product (scalar): aX*bY - aY*bX. */
    static float   Cross(Vector2 a, Vector2 b)             { return a.X * b.Y - a.Y * b.X; }
    /** @return Euclidean distance between @p a and @p b. */
    static float   Distance(Vector2 a, Vector2 b)          { return (a - b).Length(); }
    /** @return Squared Euclidean distance (avoids sqrt). */
    static float   DistanceSquared(Vector2 a, Vector2 b)   { return (a - b).LengthSquared(); }

    /** @return Unit vector in the direction of @p v; returns @p v unchanged if its length is zero. */
    static Vector2 Normalize(Vector2 v) {
        float len = v.Length();
        return len > 0 ? v / len : v;
    }
    /** @return Reflection of @p v about @p normal. */
    static Vector2 Reflect(Vector2 v, Vector2 normal) {
        return v - 2.0f * Dot(v, normal) * normal;
    }
    /** @return Component-wise minimum of @p a and @p b. */
    static Vector2 Min(Vector2 a, Vector2 b)               { return {std::min(a.X,b.X), std::min(a.Y,b.Y)}; }
    /** @return Component-wise maximum of @p a and @p b. */
    static Vector2 Max(Vector2 a, Vector2 b)               { return {std::max(a.X,b.X), std::max(a.Y,b.Y)}; }
    /** @return @p v clamped component-wise to [@p mn, @p mx]. */
    static Vector2 Clamp(Vector2 v, Vector2 mn, Vector2 mx){ return Min(Max(v,mn),mx); }
    /** @return Linear interpolation a + (b-a)*t; t=0 returns a, t=1 returns b. */
    static Vector2 Lerp(Vector2 a, Vector2 b, float t)     { return a + (b - a) * t; }
    /** @return Component-wise square root of @p v. */
    static Vector2 SquareRoot(Vector2 v)                   { return {std::sqrt(v.X), std::sqrt(v.Y)}; }

    /** Transforms a position vector by a 3×2 matrix. */
    static Vector2 Transform(Vector2 pos, const Matrix3x2& m);
    /** Transforms a position vector by a 4×4 matrix. */
    static Vector2 Transform(Vector2 pos, const Matrix4x4& m);
    /** Transforms a direction (normal) vector by a 3×2 matrix, ignoring translation. */
    static Vector2 TransformNormal(Vector2 n, const Matrix3x2& m);
    /** Transforms a direction (normal) vector by a 4×4 matrix, ignoring translation. */
    static Vector2 TransformNormal(Vector2 n, const Matrix4x4& m);
};

} // namespace System::Numerics

// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <cmath>
#include <sstream>
#include <string>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Numerics/Vector2.hpp"

namespace System::Numerics {

struct Matrix4x4;
struct Quaternion;

/** A structure encapsulating three single-precision floating-point values (X, Y, Z). */
struct Vector3 {
    float X{0.0f}; ///< The X component.
    float Y{0.0f}; ///< The Y component.
    float Z{0.0f}; ///< The Z component.

    /** Constructs a zero vector. */
    Vector3() = default;
    /** Constructs a vector with all three components set to @p value. */
    explicit Vector3(float value) : X(value), Y(value), Z(value) {}
    /** Constructs a vector from individual components. */
    Vector3(float x, float y, float z) : X(x), Y(y), Z(z) {}
    /** Constructs a vector from a Vector2 (XY) and a Z component. */
    Vector3(Vector2 xy, float z) : X(xy.X), Y(xy.Y), Z(z) {}

    /** @return The vector (0, 0, 0). */
    static Vector3 Zero()  { return {0.0f,0.0f,0.0f}; }
    /** @return The vector (1, 1, 1). */
    static Vector3 One()   { return {1.0f,1.0f,1.0f}; }
    /** @return The unit vector along the X axis (1, 0, 0). */
    static Vector3 UnitX() { return {1.0f,0.0f,0.0f}; }
    /** @return The unit vector along the Y axis (0, 1, 0). */
    static Vector3 UnitY() { return {0.0f,1.0f,0.0f}; }
    /** @return The unit vector along the Z axis (0, 0, 1). */
    static Vector3 UnitZ() { return {0.0f,0.0f,1.0f}; }

    /** @return The Euclidean length of this vector. */
    [[nodiscard]] float Length()        const { return std::sqrt(LengthSquared()); }
    /** @return The squared Euclidean length, avoiding a sqrt call. */
    [[nodiscard]] float LengthSquared() const { return X*X + Y*Y + Z*Z; }

    /** @return True if this vector is component-wise equal to @p o. */
    [[nodiscard]] bool Equals(const Vector3& o) const { return X==o.X && Y==o.Y && Z==o.Z; }

    /** @return A string of the form "&lt;X, Y, Z&gt;" using the invariant locale. */
    std::string ToString() const {
        std::ostringstream ss; ss.imbue(std::locale::classic());
        ss << "<" << X << ", " << Y << ", " << Z << ">"; return ss.str();
    }

    /** @return The component at @p i (0=X, 1=Y, 2=Z). @throws System::ArgumentOutOfRangeException on invalid index. */
    float operator[](SharpRuntime::intcs i) const {
        if (i==0) return X;
        if (i==1) return Y;
        if (i==2) return Z;
        throw System::ArgumentOutOfRangeException("index");
    }
    /** @return A reference to the component at @p i (0=X, 1=Y, 2=Z). @throws System::ArgumentOutOfRangeException on invalid index. */
    float& operator[](SharpRuntime::intcs i) {
        if (i==0) return X;
        if (i==1) return Y;
        if (i==2) return Z;
        throw System::ArgumentOutOfRangeException("index");
    }

    /** Component-wise addition. */
    Vector3 operator+(const Vector3& r) const { return {X+r.X,Y+r.Y,Z+r.Z}; }
    /** Component-wise subtraction. */
    Vector3 operator-(const Vector3& r) const { return {X-r.X,Y-r.Y,Z-r.Z}; }
    /** Component-wise multiplication. */
    Vector3 operator*(const Vector3& r) const { return {X*r.X,Y*r.Y,Z*r.Z}; }
    /** Scalar multiplication. */
    Vector3 operator*(float s)          const { return {X*s,  Y*s,  Z*s};   }
    /** Component-wise division. */
    Vector3 operator/(const Vector3& r) const { return {X/r.X,Y/r.Y,Z/r.Z}; }
    /** Scalar division. */
    Vector3 operator/(float s)          const { return {X/s,  Y/s,  Z/s};   }
    /** Unary negation. */
    Vector3 operator-()                 const { return {-X,-Y,-Z}; }
    /** Equality comparison (exact floating-point). */
    bool    operator==(const Vector3& r) const { return Equals(r); }
    /** Inequality comparison. */
    bool    operator!=(const Vector3& r) const { return !Equals(r); }
    /** Scalar * vector multiplication (commutative). */
    friend Vector3 operator*(float s, const Vector3& v) { return v * s; }

    /** @return Dot product a·b = aX*bX + aY*bY + aZ*bZ. */
    static float   Dot(Vector3 a, Vector3 b)               { return a.X*b.X + a.Y*b.Y + a.Z*b.Z; }
    /** @return Cross product a × b (perpendicular to both a and b). */
    static Vector3 Cross(Vector3 a, Vector3 b)             { return {a.Y*b.Z-a.Z*b.Y, a.Z*b.X-a.X*b.Z, a.X*b.Y-a.Y*b.X}; }
    /** @return Euclidean distance between @p a and @p b. */
    static float   Distance(Vector3 a, Vector3 b)          { return (a-b).Length(); }
    /** @return Squared Euclidean distance (avoids sqrt). */
    static float   DistanceSquared(Vector3 a, Vector3 b)   { return (a-b).LengthSquared(); }
    /** @return Unit vector in the direction of @p v; returns @p v unchanged if its length is zero. */
    static Vector3 Normalize(Vector3 v)                    { float l=v.Length(); return l>0?v/l:v; }
    /** @return Reflection of @p v about surface normal @p n. */
    static Vector3 Reflect(Vector3 v, Vector3 n)           { return v - 2.0f*Dot(v,n)*n; }
    /** @return Component-wise absolute value of @p v. */
    static Vector3 Abs(Vector3 v)                          { return {std::abs(v.X),std::abs(v.Y),std::abs(v.Z)}; }
    /** @return Component-wise minimum of @p a and @p b. */
    static Vector3 Min(Vector3 a, Vector3 b)               { return {std::min(a.X,b.X),std::min(a.Y,b.Y),std::min(a.Z,b.Z)}; }
    /** @return Component-wise maximum of @p a and @p b. */
    static Vector3 Max(Vector3 a, Vector3 b)               { return {std::max(a.X,b.X),std::max(a.Y,b.Y),std::max(a.Z,b.Z)}; }
    /** @return @p v clamped component-wise to [@p mn, @p mx]. */
    static Vector3 Clamp(Vector3 v, Vector3 mn, Vector3 mx){ return Min(Max(v,mn),mx); }
    /** @return Linear interpolation a + (b-a)*t; t=0 returns a, t=1 returns b. */
    static Vector3 Lerp(Vector3 a, Vector3 b, float t)     { return a+(b-a)*t; }
    /** @return Component-wise square root of @p v. */
    static Vector3 SquareRoot(Vector3 v)                   { return {std::sqrt(v.X),std::sqrt(v.Y),std::sqrt(v.Z)}; }
    /** @return Component-wise sum a + b. */
    static Vector3 Add(Vector3 a,Vector3 b)                { return a+b; }
    /** @return Component-wise difference a - b. */
    static Vector3 Subtract(Vector3 a,Vector3 b)           { return a-b; }
    /** @return Component-wise product a * b. */
    static Vector3 Multiply(Vector3 a,Vector3 b)           { return a*b; }
    /** @return Vector @p v scaled by scalar @p s. */
    static Vector3 Multiply(Vector3 v,float s)             { return v*s; }
    /** @return Component-wise quotient a / b. */
    static Vector3 Divide(Vector3 a,Vector3 b)             { return a/b; }
    /** @return Vector @p v divided by scalar @p s. */
    static Vector3 Divide(Vector3 v,float s)               { return v/s; }
    /** @return Negated vector -v. */
    static Vector3 Negate(Vector3 v)                       { return -v; }

    /** Transforms a position vector by a 4×4 matrix. */
    static Vector3 Transform(Vector3 pos, const Matrix4x4& m);
    /** Transforms a direction (normal) vector by a 4×4 matrix, ignoring translation. */
    static Vector3 TransformNormal(Vector3 n, const Matrix4x4& m);
    /** Rotates @p v by quaternion @p q. */
    static Vector3 Transform(Vector3 v, const Quaternion& q);
};

} // namespace System::Numerics

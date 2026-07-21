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
#include "System/Numerics/Vector3.hpp"

namespace System::Numerics {

struct Matrix4x4;
struct Quaternion;

/** A structure encapsulating four single-precision floating-point values (X, Y, Z, W). */
struct Vector4 {
    float X{0.0f}; ///< The X component.
    float Y{0.0f}; ///< The Y component.
    float Z{0.0f}; ///< The Z component.
    float W{0.0f}; ///< The W component.

    /** Constructs a zero vector. */
    Vector4() = default;
    /** Constructs a vector with all four components set to @p value. */
    explicit Vector4(float value) : X(value), Y(value), Z(value), W(value) {}
    /** Constructs a vector from individual components. */
    Vector4(float x, float y, float z, float w) : X(x), Y(y), Z(z), W(w) {}
    /** Constructs a vector from a Vector2 (XY) plus Z and W components. */
    Vector4(Vector2 xy, float z, float w) : X(xy.X), Y(xy.Y), Z(z), W(w) {}
    /** Constructs a vector from a Vector3 (XYZ) plus a W component. */
    Vector4(Vector3 xyz, float w)         : X(xyz.X), Y(xyz.Y), Z(xyz.Z), W(w) {}

    /** @return The vector (0, 0, 0, 0). */
    static Vector4 Zero()  { return {0,0,0,0}; }
    /** @return The vector (1, 1, 1, 1). */
    static Vector4 One()   { return {1,1,1,1}; }
    /** @return The unit vector along the X axis (1, 0, 0, 0). */
    static Vector4 UnitX() { return {1,0,0,0}; }
    /** @return The unit vector along the Y axis (0, 1, 0, 0). */
    static Vector4 UnitY() { return {0,1,0,0}; }
    /** @return The unit vector along the Z axis (0, 0, 1, 0). */
    static Vector4 UnitZ() { return {0,0,1,0}; }
    /** @return The unit vector along the W axis (0, 0, 0, 1). */
    static Vector4 UnitW() { return {0,0,0,1}; }

    /** @return The Euclidean length of this vector. */
    [[nodiscard]] float Length()        const { return std::sqrt(LengthSquared()); }
    /** @return The squared Euclidean length, avoiding a sqrt call. */
    [[nodiscard]] float LengthSquared() const { return X*X+Y*Y+Z*Z+W*W; }

    /** @return True if this vector is component-wise equal to @p o. */
    [[nodiscard]] bool Equals(const Vector4& o) const { return X==o.X&&Y==o.Y&&Z==o.Z&&W==o.W; }

    /** @return A string of the form "&lt;X, Y, Z, W&gt;" using the invariant locale. */
    std::string ToString() const {
        std::ostringstream ss; ss.imbue(std::locale::classic());
        ss<<"<"<<X<<", "<<Y<<", "<<Z<<", "<<W<<">"; return ss.str();
    }

    /** @return The component at @p i (0=X,1=Y,2=Z,3=W). @throws System::ArgumentOutOfRangeException on invalid index. */
    float operator[](SharpRuntime::intcs i) const {
        if (i==0) return X;
        if (i==1) return Y;
        if (i==2) return Z;
        if (i==3) return W;
        throw System::ArgumentOutOfRangeException("index");
    }
    /** @return A reference to the component at @p i (0=X,1=Y,2=Z,3=W). @throws System::ArgumentOutOfRangeException on invalid index. */
    float& operator[](SharpRuntime::intcs i) {
        if (i==0) return X;
        if (i==1) return Y;
        if (i==2) return Z;
        if (i==3) return W;
        throw System::ArgumentOutOfRangeException("index");
    }

    /** Component-wise addition. */
    Vector4 operator+(const Vector4& r) const { return {X+r.X,Y+r.Y,Z+r.Z,W+r.W}; }
    /** Component-wise subtraction. */
    Vector4 operator-(const Vector4& r) const { return {X-r.X,Y-r.Y,Z-r.Z,W-r.W}; }
    /** Component-wise multiplication. */
    Vector4 operator*(const Vector4& r) const { return {X*r.X,Y*r.Y,Z*r.Z,W*r.W}; }
    /** Scalar multiplication. */
    Vector4 operator*(float s)          const { return {X*s,Y*s,Z*s,W*s}; }
    /** Component-wise division. */
    Vector4 operator/(const Vector4& r) const { return {X/r.X,Y/r.Y,Z/r.Z,W/r.W}; }
    /** Scalar division. */
    Vector4 operator/(float s)          const { return {X/s,Y/s,Z/s,W/s}; }
    /** Unary negation. */
    Vector4 operator-()                 const { return {-X,-Y,-Z,-W}; }
    /** Equality comparison (exact floating-point). */
    bool    operator==(const Vector4& r) const { return Equals(r); }
    /** Inequality comparison. */
    bool    operator!=(const Vector4& r) const { return !Equals(r); }
    /** Scalar * vector multiplication (commutative). */
    friend Vector4 operator*(float s, const Vector4& v) { return v * s; }

    /** @return Dot product a·b = aX*bX + aY*bY + aZ*bZ + aW*bW. */
    static float   Dot(Vector4 a, Vector4 b)               { return a.X*b.X+a.Y*b.Y+a.Z*b.Z+a.W*b.W; }
    /** @return Euclidean distance between @p a and @p b. */
    static float   Distance(Vector4 a, Vector4 b)          { return (a-b).Length(); }
    /** @return Squared Euclidean distance (avoids sqrt). */
    static float   DistanceSquared(Vector4 a, Vector4 b)   { return (a-b).LengthSquared(); }
    /** @return Unit vector in the direction of @p v; returns @p v unchanged if its length is zero. */
    static Vector4 Normalize(Vector4 v)                    { float l=v.Length(); return l>0?v/l:v; }
    /** @return Component-wise absolute value of @p v. */
    static Vector4 Abs(Vector4 v)                          { return {std::abs(v.X),std::abs(v.Y),std::abs(v.Z),std::abs(v.W)}; }
    /** @return Component-wise minimum of @p a and @p b. */
    static Vector4 Min(Vector4 a, Vector4 b)               { return {std::min(a.X,b.X),std::min(a.Y,b.Y),std::min(a.Z,b.Z),std::min(a.W,b.W)}; }
    /** @return Component-wise maximum of @p a and @p b. */
    static Vector4 Max(Vector4 a, Vector4 b)               { return {std::max(a.X,b.X),std::max(a.Y,b.Y),std::max(a.Z,b.Z),std::max(a.W,b.W)}; }
    /** @return @p v clamped component-wise to [@p mn, @p mx]. */
    static Vector4 Clamp(Vector4 v, Vector4 mn, Vector4 mx){ return Min(Max(v,mn),mx); }
    /** @return Linear interpolation a + (b-a)*t; t=0 returns a, t=1 returns b. */
    static Vector4 Lerp(Vector4 a, Vector4 b, float t)     { return a+(b-a)*t; }
    /** @return Component-wise square root of @p v. */
    static Vector4 SquareRoot(Vector4 v)                   { return {std::sqrt(v.X),std::sqrt(v.Y),std::sqrt(v.Z),std::sqrt(v.W)}; }
    /** @return Component-wise sum a + b. */
    static Vector4 Add(Vector4 a,Vector4 b)                { return a+b; }
    /** @return Component-wise difference a - b. */
    static Vector4 Subtract(Vector4 a,Vector4 b)           { return a-b; }
    /** @return Component-wise product a * b. */
    static Vector4 Multiply(Vector4 a,Vector4 b)           { return a*b; }
    /** @return Vector @p v scaled by scalar @p s. */
    static Vector4 Multiply(Vector4 v,float s)             { return v*s; }
    /** @return Component-wise quotient a / b. */
    static Vector4 Divide(Vector4 a,Vector4 b)             { return a/b; }
    /** @return Vector @p v divided by scalar @p s. */
    static Vector4 Divide(Vector4 v,float s)               { return v/s; }
    /** @return Negated vector -v. */
    static Vector4 Negate(Vector4 v)                       { return -v; }

    /** Transforms a 2-D position by a 4×4 matrix (Z=0, W=1). */
    static Vector4 Transform(Vector2 pos, const Matrix4x4& m);
    /** Transforms a 3-D position by a 4×4 matrix (W=1). */
    static Vector4 Transform(Vector3 pos, const Matrix4x4& m);
    /** Transforms a 4-D vector by a 4×4 matrix. */
    static Vector4 Transform(Vector4 v,   const Matrix4x4& m);
    /** Rotates a 4-D vector by quaternion @p q (XYZ components only; W is passed through). */
    static Vector4 Transform(Vector4 v,   const Quaternion& q);
};

} // namespace System::Numerics

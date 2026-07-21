// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <cmath>
#include <sstream>
#include <string>
#include "System/Numerics/Vector3.hpp"

namespace System::Numerics {

struct Matrix4x4;

/**
 * Represents a rotation in 3-D space encoded as four floats X, Y, Z, W.
 * 
 * The quaternion (X, Y, Z, W) satisfies ||q|| = 1 for unit quaternions,
 * which represent pure rotations. Use Normalize() to ensure unit length
 * after arithmetic operations that may drift.
 */
struct Quaternion {
    float X{0.0f}; ///< The X (imaginary i) component.
    float Y{0.0f}; ///< The Y (imaginary j) component.
    float Z{0.0f}; ///< The Z (imaginary k) component.
    float W{1.0f}; ///< The W (real) component.

    /** Constructs the identity quaternion (0, 0, 0, 1). */
    Quaternion() = default;
    /** Constructs a quaternion from individual components. */
    Quaternion(float x, float y, float z, float w) : X(x), Y(y), Z(z), W(w) {}
    /** Constructs a quaternion from a vector part @p v and real part @p w. */
    Quaternion(Vector3 v, float w) : X(v.X), Y(v.Y), Z(v.Z), W(w) {}

    /** @return The identity quaternion (0, 0, 0, 1) — represents no rotation. */
    static Quaternion Identity() { return {0,0,0,1}; }
    /** @return The zero quaternion (0, 0, 0, 0). */
    static Quaternion Zero()     { return {0,0,0,0}; }

    /** @return True if this quaternion equals the identity (0, 0, 0, 1) exactly. */
    [[nodiscard]] bool getIsIdentityProperty() const { return X==0&&Y==0&&Z==0&&W==1; }
    /** @return The Euclidean norm ||q||. */
    [[nodiscard]] float Length()        const { return std::sqrt(LengthSquared()); }
    /** @return The squared norm ||q||², avoiding a sqrt call. */
    [[nodiscard]] float LengthSquared() const { return X*X+Y*Y+Z*Z+W*W; }

    /** @return True if all components compare equal to the corresponding components of @p o. */
    [[nodiscard]] bool Equals(const Quaternion& o) const {
        return X==o.X&&Y==o.Y&&Z==o.Z&&W==o.W;
    }

    /** @return A string of the form "{X:x Y:y Z:z W:w}" using the invariant locale. */
    std::string ToString() const {
        std::ostringstream ss; ss.imbue(std::locale::classic());
        ss<<"{X:"<<X<<" Y:"<<Y<<" Z:"<<Z<<" W:"<<W<<"}"; return ss.str();
    }

    /** Component-wise addition. */
    Quaternion operator+(const Quaternion& r) const { return {X+r.X,Y+r.Y,Z+r.Z,W+r.W}; }
    /** Component-wise subtraction. */
    Quaternion operator-(const Quaternion& r) const { return {X-r.X,Y-r.Y,Z-r.Z,W-r.W}; }
    /** Unary negation (negates all components). */
    Quaternion operator-()                   const { return {-X,-Y,-Z,-W}; }
    /** Scalar multiplication. */
    Quaternion operator*(float s)            const { return {X*s,Y*s,Z*s,W*s}; }
    /** Scalar division. */
    Quaternion operator/(float s)            const { return {X/s,Y/s,Z/s,W/s}; }
    /** Divides this quaternion by @p q (i.e. *this * Inverse(q)). */
    Quaternion operator/(const Quaternion& q) const { return *this * Inverse(q); }
    /** Equality comparison (exact floating-point). */
    bool operator==(const Quaternion& r)    const { return Equals(r); }
    /** Inequality comparison. */
    bool operator!=(const Quaternion& r)    const { return !Equals(r); }

    /** Hamilton product — composes two rotations (not commutative). */
    Quaternion operator*(const Quaternion& q) const {
        return {
            W*q.X + X*q.W + Y*q.Z - Z*q.Y,
            W*q.Y - X*q.Z + Y*q.W + Z*q.X,
            W*q.Z + X*q.Y - Y*q.X + Z*q.W,
            W*q.W - X*q.X - Y*q.Y - Z*q.Z
        };
    }

    /** @return Dot product a·b of the quaternions treated as 4-D vectors. */
    static float       Dot(Quaternion a, Quaternion b)         { return a.X*b.X+a.Y*b.Y+a.Z*b.Z+a.W*b.W; }
    /**
     * @return Unit quaternion in the direction of @p q (normalised by length).
     * @note Matches .NET exactly: an unconditional q/Length(), including for a zero-length
     * @p q, which produces a NaN-component quaternion (not a caught/guarded case in .NET's
     * own implementation either — see Quaternion.cs).
     */
    static Quaternion  Normalize(Quaternion q)                 { return q/q.Length(); }
    /** @return Conjugate q* = (-X, -Y, -Z, W) — inverse rotation for unit quaternions. */
    static Quaternion  Conjugate(Quaternion q)                 { return {-q.X,-q.Y,-q.Z,q.W}; }
    /**
     * @return Multiplicative inverse of @p q; equivalent to Conjugate for unit quaternions.
     * Returns Zero() when LengthSquared() is at or below .NET's float epsilon threshold
     * (1.192092896e-7f, matching Quaternion.cs's Inverse), not just when it is exactly zero —
     * otherwise a near-zero-but-nonzero length would blow up to huge/near-infinite components.
     */
    static Quaternion  Inverse(Quaternion q) {
        constexpr float epsilon = 1.192092896e-7f;
        float n = q.LengthSquared();
        return n > epsilon ? Conjugate(q)/n : Zero();
    }
    /** @return Negated quaternion -q (all components negated). */
    static Quaternion  Negate(Quaternion q)                    { return -q; }
    /** @return Component-wise sum a + b. */
    static Quaternion  Add(Quaternion a,Quaternion b)          { return a+b; }
    /** @return Component-wise difference a - b. */
    static Quaternion  Subtract(Quaternion a,Quaternion b)     { return a-b; }
    /** @return Hamilton product a * b (rotation composition). */
    static Quaternion  Multiply(Quaternion a,Quaternion b)     { return a*b; }
    /** @return Quaternion @p q scaled by scalar @p s. */
    static Quaternion  Multiply(Quaternion q, float s)         { return q*s; }
    /** @return @p a composed with the inverse of @p b (i.e. a * Inverse(b)). */
    static Quaternion  Divide(Quaternion a, Quaternion b)      { return a*Inverse(b); }
    /** @return @p value1 rotated by @p value2, applied in @p value1-then-@p value2 order (i.e. value2 * value1). */
    static Quaternion  Concatenate(Quaternion value1, Quaternion value2) { return value2*value1; }

    /**
     * Creates a unit quaternion from an axis-angle representation.
     * @param axis   Rotation axis (should be normalised).
     * @param angle  Rotation angle in radians.
     */
    static Quaternion CreateFromAxisAngle(Vector3 axis, float angle) {
        float half=angle*0.5f, s=std::sin(half), c=std::cos(half);
        return {axis.X*s, axis.Y*s, axis.Z*s, c};
    }

    /**
     * Creates a quaternion from intrinsic Euler angles (yaw-pitch-roll, ZYX order).
     * @param yaw    Rotation around Y in radians.
     * @param pitch  Rotation around X in radians.
     * @param roll   Rotation around Z in radians.
     */
    static Quaternion CreateFromYawPitchRoll(float yaw, float pitch, float roll) {
        float cy=std::cos(yaw*0.5f), sy=std::sin(yaw*0.5f);
        float cp=std::cos(pitch*0.5f), sp=std::sin(pitch*0.5f);
        float cr=std::cos(roll*0.5f), sr=std::sin(roll*0.5f);
        return {
            cy*sp*cr + sy*cp*sr,
            sy*cp*cr - cy*sp*sr,
            cy*cp*sr - sy*sp*cr,
            cy*cp*cr + sy*sp*sr
        };
    }

    /**
     * Spherical linear interpolation between unit quaternions @p a and @p b.
     * @param t  Interpolation factor in [0, 1]; 0 returns a, 1 returns b.
     */
    static Quaternion Slerp(Quaternion a, Quaternion b, float t) {
        constexpr float slerpEpsilon = 1e-6f; // matches Quaternion.cs's SlerpEpsilon
        float cosAngle = Dot(a, b);
        if (cosAngle < 0) { b = -b; cosAngle = -cosAngle; }
        float k0, k1;
        if (cosAngle > (1.0f - slerpEpsilon)) {
            k0 = 1.0f - t; k1 = t;
        } else {
            float angle = std::acos(cosAngle);
            float inv = 1.0f / std::sin(angle);
            k0 = std::sin((1.0f-t)*angle)*inv;
            k1 = std::sin(t*angle)*inv;
        }
        return {a.X*k0+b.X*k1, a.Y*k0+b.Y*k1, a.Z*k0+b.Z*k1, a.W*k0+b.W*k1};
    }

    /**
     * Normalised linear interpolation (cheaper than Slerp; slightly less accurate for large angles).
     * @param t  Interpolation factor in [0, 1].
     */
    static Quaternion Lerp(Quaternion a, Quaternion b, float t) {
        if (Dot(a,b) < 0) b = -b;
        return Normalize({a.X+t*(b.X-a.X), a.Y+t*(b.Y-a.Y), a.Z+t*(b.Z-a.Z), a.W+t*(b.W-a.W)});
    }

    /**
     * Creates a quaternion from an orthogonal rotation matrix.
     * @param m  A rotation matrix (upper-left 3×3 must be orthonormal).
     */
    static Quaternion CreateFromRotationMatrix(const Matrix4x4& m);
};

} // namespace System::Numerics

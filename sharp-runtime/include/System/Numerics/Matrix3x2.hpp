// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <cmath>
#include <limits>
#include <sstream>
#include <string>
#include "System/Numerics/Vector2.hpp"

namespace System::Numerics {

/** <summary>Represents a 3x2 matrix used for 2D transformations.</summary> */
struct Matrix3x2 {
    float M11{}; ///< Row 1, column 1 element.
    float M12{}; ///< Row 1, column 2 element.
    float M21{}; ///< Row 2, column 1 element.
    float M22{}; ///< Row 2, column 2 element.
    float M31{}; ///< Row 3, column 1 element (X translation).
    float M32{}; ///< Row 3, column 2 element (Y translation).

    /** Default constructor — all elements zero. */
    Matrix3x2() = default;

    /**
     * Constructs a matrix from six explicit element values.
     * @param m11  Row 1, column 1.
     * @param m12  Row 1, column 2.
     * @param m21  Row 2, column 1.
     * @param m22  Row 2, column 2.
     * @param m31  Row 3, column 1 (X translation).
     * @param m32  Row 3, column 2 (Y translation).
     */
    Matrix3x2(float m11,float m12,float m21,float m22,float m31,float m32)
        : M11(m11),M12(m12),M21(m21),M22(m22),M31(m31),M32(m32) {}

    /** Returns the multiplicative identity matrix. */
    static Matrix3x2 Identity() { return {1,0, 0,1, 0,0}; }

    /** Returns true when this matrix equals the identity matrix. */
    [[nodiscard]] bool getIsIdentityProperty() const {
        return M11==1&&M12==0&&M21==0&&M22==1&&M31==0&&M32==0;
    }

    /** Returns the translation component as a Vector2 (M31, M32). */
    [[nodiscard]] Vector2 getTranslationProperty() const { return {M31,M32}; }

    /** Sets the translation component (M31, M32) from @p t. */
    void setTranslationProperty(Vector2 t) { M31=t.X; M32=t.Y; }

    /** Returns the determinant of the 2x2 rotation/scale sub-matrix. */
    [[nodiscard]] float GetDeterminant() const { return M11*M22 - M12*M21; }

    /** Returns true when all corresponding elements of this matrix and @p o are equal. */
    bool Equals(const Matrix3x2& o) const {
        return M11==o.M11&&M12==o.M12&&M21==o.M21&&M22==o.M22&&M31==o.M31&&M32==o.M32;
    }

    /** Equality comparison. */
    bool operator==(const Matrix3x2& o) const { return Equals(o); }

    /** Inequality comparison. */
    bool operator!=(const Matrix3x2& o) const { return !Equals(o); }

    /** Adds two matrices element-wise. */
    Matrix3x2 operator+(const Matrix3x2& r) const {
        return {M11+r.M11,M12+r.M12,M21+r.M21,M22+r.M22,M31+r.M31,M32+r.M32};
    }

    /** Subtracts @p r from this matrix element-wise. */
    Matrix3x2 operator-(const Matrix3x2& r) const {
        return {M11-r.M11,M12-r.M12,M21-r.M21,M22-r.M22,M31-r.M31,M32-r.M32};
    }

    /** Negates all elements. */
    Matrix3x2 operator-() const { return {-M11,-M12,-M21,-M22,-M31,-M32}; }

    /** Multiplies every element by scalar @p s. */
    Matrix3x2 operator*(float s) const { return {M11*s,M12*s,M21*s,M22*s,M31*s,M32*s}; }

    /** Concatenates two 2D transformation matrices (this * @p b). */
    Matrix3x2 operator*(const Matrix3x2& b) const {
        return {
            M11*b.M11+M12*b.M21,
            M11*b.M12+M12*b.M22,
            M21*b.M11+M22*b.M21,
            M21*b.M12+M22*b.M22,
            M31*b.M11+M32*b.M21+b.M31,
            M31*b.M12+M32*b.M22+b.M32
        };
    }

    /** Adds two matrices element-wise. */
    static Matrix3x2 Add(Matrix3x2 a, Matrix3x2 b)      { return a+b; }

    /** Subtracts @p b from @p a element-wise. */
    static Matrix3x2 Subtract(Matrix3x2 a, Matrix3x2 b) { return a-b; }

    /** Concatenates two 2D transformation matrices. */
    static Matrix3x2 Multiply(Matrix3x2 a, Matrix3x2 b) { return a*b; }

    /** Multiplies every element of @p a by scalar @p s. */
    static Matrix3x2 Multiply(Matrix3x2 a, float s)     { return a*s; }

    /** Negates all elements of @p m. */
    static Matrix3x2 Negate(Matrix3x2 m)                { return -m; }

    /**
     * Attempts to invert @p m, writing the result to @p result.
     * @param m       The matrix to invert.
     * @param result  Receives the inverse on success, or a zero matrix on failure.
     * @return True if the matrix is invertible (non-zero determinant).
     */
    static bool Invert(const Matrix3x2& m, Matrix3x2& result) {
        float det = m.GetDeterminant();
        if (std::abs(det) < std::numeric_limits<float>::denorm_min()) {
            constexpr float nan = std::numeric_limits<float>::quiet_NaN();
            result = {nan, nan, nan, nan, nan, nan};
            return false;
        }
        float inv = 1.0f / det;
        result = {m.M22*inv, -m.M12*inv, -m.M21*inv, m.M11*inv,
                  (m.M21*m.M32-m.M22*m.M31)*inv, (m.M12*m.M31-m.M11*m.M32)*inv};
        return true;
    }

    /**
     * Creates a 2D translation matrix.
     * @param x  Translation along the X axis.
     * @param y  Translation along the Y axis.
     */
    static Matrix3x2 CreateTranslation(float x, float y) { return {1,0, 0,1, x,y}; }

    /** Creates a 2D translation matrix from a Vector2. */
    static Matrix3x2 CreateTranslation(Vector2 t)        { return CreateTranslation(t.X,t.Y); }

    /**
     * Creates a 2D non-uniform scale matrix.
     * @param x  Scale factor along the X axis.
     * @param y  Scale factor along the Y axis.
     */
    static Matrix3x2 CreateScale(float x, float y)       { return {x,0, 0,y, 0,0}; }

    /** Creates a 2D non-uniform scale matrix from a Vector2. */
    static Matrix3x2 CreateScale(Vector2 s)              { return CreateScale(s.X,s.Y); }

    /** Creates a 2D uniform scale matrix. */
    static Matrix3x2 CreateScale(float s)                { return CreateScale(s,s); }

    /**
     * Creates a 2D counter-clockwise rotation matrix.
     * @param radians  Angle in radians.
     */
    static Matrix3x2 CreateRotation(float radians) {
        float c=std::cos(radians), s=std::sin(radians);
        return {c,s, -s,c, 0,0};
    }

    /**
     * Creates a 2D skew (shear) matrix.
     * @param radiansX  Shear angle along the X axis, in radians.
     * @param radiansY  Shear angle along the Y axis, in radians.
     */
    static Matrix3x2 CreateSkew(float radiansX, float radiansY) {
        return {1,std::tan(radiansY), std::tan(radiansX),1, 0,0};
    }

    /** Returns a human-readable representation of all six elements. */
    std::string ToString() const {
        std::ostringstream ss;
        ss.imbue(std::locale::classic());
        ss<<"{ {M11:"<<M11<<" M12:"<<M12<<"} {M21:"<<M21<<" M22:"<<M22<<"} {M31:"<<M31<<" M32:"<<M32<<"} }";
        return ss.str();
    }
};

// --- Deferred Vector2::Transform for Matrix3x2 ---

/** Transforms point @p p by matrix @p m (applies translation). */
inline Vector2 Vector2::Transform(Vector2 p, const Matrix3x2& m) {
    return {p.X*m.M11+p.Y*m.M21+m.M31, p.X*m.M12+p.Y*m.M22+m.M32};
}

/** Transforms direction @p n by matrix @p m (ignores translation). */
inline Vector2 Vector2::TransformNormal(Vector2 n, const Matrix3x2& m) {
    return {n.X*m.M11+n.Y*m.M21, n.X*m.M12+n.Y*m.M22};
}

} // namespace System::Numerics

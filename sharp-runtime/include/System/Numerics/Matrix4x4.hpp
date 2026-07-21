// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <cmath>
#include <limits>
#include <numbers>
#include <sstream>
#include <string>
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/HashCode.hpp"
#include "System/Numerics/Vector3.hpp"
#include "System/Numerics/Vector4.hpp"
#include "System/Numerics/Quaternion.hpp"

namespace System::Numerics {

/**
 * <summary>Represents a 4x4 matrix.</summary>
 *
 * This port implements the core arithmetic/factory surface (Add/Subtract/Multiply/Negate,
 * Transpose/Invert/GetDeterminant, CreateTranslation/CreateScale/CreateRotationX-Y-Z,
 * CreateFromQuaternion/CreateFromYawPitchRoll, CreateLookAt, CreatePerspectiveFieldOfView,
 * CreateOrthographic) but deliberately omits a number of real .NET Matrix4x4 members not yet
 * needed by this project's consumers: the "LeftHanded" variant of every camera/projection
 * factory, CreateBillboard/CreateBillboardLeftHanded, CreateConstrainedBillboard(LeftHanded),
 * CreateFromAxisAngle, CreateOrthographicOffCenter(LeftHanded), CreatePerspective(LeftHanded)/
 * CreatePerspectiveOffCenter(LeftHanded), CreateReflection, CreateRotationX/Y/Z's centerPoint
 * overload, CreateScale's centerPoint overloads, CreateShadow, CreateViewport(LeftHanded),
 * CreateWorld, Decompose (scale/rotation/translation extraction), and Transform(Matrix4x4,
 * Quaternion). Add on demand rather than porting the full surface speculatively.
 */
struct Matrix4x4 {
    float M11{}; ///< Row 1, column 1 element.
    float M12{}; ///< Row 1, column 2 element.
    float M13{}; ///< Row 1, column 3 element.
    float M14{}; ///< Row 1, column 4 element.
    float M21{}; ///< Row 2, column 1 element.
    float M22{}; ///< Row 2, column 2 element.
    float M23{}; ///< Row 2, column 3 element.
    float M24{}; ///< Row 2, column 4 element.
    float M31{}; ///< Row 3, column 1 element.
    float M32{}; ///< Row 3, column 2 element.
    float M33{}; ///< Row 3, column 3 element.
    float M34{}; ///< Row 3, column 4 element.
    float M41{}; ///< Row 4, column 1 element (X translation).
    float M42{}; ///< Row 4, column 2 element (Y translation).
    float M43{}; ///< Row 4, column 3 element (Z translation).
    float M44{}; ///< Row 4, column 4 element.

    /** Default constructor — all elements zero. */
    Matrix4x4() = default;

    /** Constructs a matrix from sixteen explicit element values (row-major order). */
    Matrix4x4(float m11,float m12,float m13,float m14,
              float m21,float m22,float m23,float m24,
              float m31,float m32,float m33,float m34,
              float m41,float m42,float m43,float m44)
        : M11(m11),M12(m12),M13(m13),M14(m14)
        , M21(m21),M22(m22),M23(m23),M24(m24)
        , M31(m31),M32(m32),M33(m33),M34(m34)
        , M41(m41),M42(m42),M43(m43),M44(m44) {}

    /** Returns the multiplicative identity matrix. */
    static Matrix4x4 Identity() {
        return {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
    }

    /** Returns true when this matrix equals the identity matrix. */
    [[nodiscard]] bool getIsIdentityProperty() const {
        return M11==1&&M22==1&&M33==1&&M44==1
            && M12==0&&M13==0&&M14==0
            && M21==0&&M23==0&&M24==0
            && M31==0&&M32==0&&M34==0
            && M41==0&&M42==0&&M43==0;
    }

    /** Returns the translation component as a Vector3 (M41, M42, M43). */
    [[nodiscard]] Vector3 getTranslationProperty() const { return {M41,M42,M43}; }

    /** Sets the translation component (M41, M42, M43) from @p t. */
    void setTranslationProperty(Vector3 t) { M41=t.X; M42=t.Y; M43=t.Z; }

    /** Returns true when all corresponding elements of this matrix and @p o are equal. */
    bool Equals(const Matrix4x4& o) const {
        return M11==o.M11&&M12==o.M12&&M13==o.M13&&M14==o.M14
            && M21==o.M21&&M22==o.M22&&M23==o.M23&&M24==o.M24
            && M31==o.M31&&M32==o.M32&&M33==o.M33&&M34==o.M34
            && M41==o.M41&&M42==o.M42&&M43==o.M43&&M44==o.M44;
    }

    /** Equality comparison. */
    bool operator==(const Matrix4x4& o) const { return Equals(o); }

    /** Inequality comparison. */
    bool operator!=(const Matrix4x4& o) const { return !Equals(o); }

    /** Returns a hash code combining all sixteen elements, so that equal matrices hash equal. */
    [[nodiscard]] int GetHashCode() const {
        int row1 = System::HashCode::Combine(M11,M12,M13,M14);
        int row2 = System::HashCode::Combine(M21,M22,M23,M24);
        int row3 = System::HashCode::Combine(M31,M32,M33,M34);
        int row4 = System::HashCode::Combine(M41,M42,M43,M44);
        return System::HashCode::Combine(row1,row2,row3,row4);
    }

    /** Adds two matrices element-wise. */
    Matrix4x4 operator+(const Matrix4x4& r) const {
        return {M11+r.M11,M12+r.M12,M13+r.M13,M14+r.M14,
                M21+r.M21,M22+r.M22,M23+r.M23,M24+r.M24,
                M31+r.M31,M32+r.M32,M33+r.M33,M34+r.M34,
                M41+r.M41,M42+r.M42,M43+r.M43,M44+r.M44};
    }

    /** Subtracts @p r from this matrix element-wise. */
    Matrix4x4 operator-(const Matrix4x4& r) const {
        return {M11-r.M11,M12-r.M12,M13-r.M13,M14-r.M14,
                M21-r.M21,M22-r.M22,M23-r.M23,M24-r.M24,
                M31-r.M31,M32-r.M32,M33-r.M33,M34-r.M34,
                M41-r.M41,M42-r.M42,M43-r.M43,M44-r.M44};
    }

    /** Negates all elements. */
    Matrix4x4 operator-() const {
        return {-M11,-M12,-M13,-M14,-M21,-M22,-M23,-M24,
                -M31,-M32,-M33,-M34,-M41,-M42,-M43,-M44};
    }

    /** Multiplies every element by scalar @p s. */
    Matrix4x4 operator*(float s) const {
        return {M11*s,M12*s,M13*s,M14*s, M21*s,M22*s,M23*s,M24*s,
                M31*s,M32*s,M33*s,M34*s, M41*s,M42*s,M43*s,M44*s};
    }

    /** Concatenates two 4x4 transformation matrices (this * @p b). */
    Matrix4x4 operator*(const Matrix4x4& b) const {
        return {
            M11*b.M11+M12*b.M21+M13*b.M31+M14*b.M41,
            M11*b.M12+M12*b.M22+M13*b.M32+M14*b.M42,
            M11*b.M13+M12*b.M23+M13*b.M33+M14*b.M43,
            M11*b.M14+M12*b.M24+M13*b.M34+M14*b.M44,
            M21*b.M11+M22*b.M21+M23*b.M31+M24*b.M41,
            M21*b.M12+M22*b.M22+M23*b.M32+M24*b.M42,
            M21*b.M13+M22*b.M23+M23*b.M33+M24*b.M43,
            M21*b.M14+M22*b.M24+M23*b.M34+M24*b.M44,
            M31*b.M11+M32*b.M21+M33*b.M31+M34*b.M41,
            M31*b.M12+M32*b.M22+M33*b.M32+M34*b.M42,
            M31*b.M13+M32*b.M23+M33*b.M33+M34*b.M43,
            M31*b.M14+M32*b.M24+M33*b.M34+M34*b.M44,
            M41*b.M11+M42*b.M21+M43*b.M31+M44*b.M41,
            M41*b.M12+M42*b.M22+M43*b.M32+M44*b.M42,
            M41*b.M13+M42*b.M23+M43*b.M33+M44*b.M43,
            M41*b.M14+M42*b.M24+M43*b.M34+M44*b.M44
        };
    }

    /** Adds two matrices element-wise. */
    static Matrix4x4 Add(Matrix4x4 a, Matrix4x4 b)      { return a+b; }

    /** Subtracts @p b from @p a element-wise. */
    static Matrix4x4 Subtract(Matrix4x4 a, Matrix4x4 b) { return a-b; }

    /** Concatenates two 4x4 transformation matrices. */
    static Matrix4x4 Multiply(Matrix4x4 a, Matrix4x4 b) { return a*b; }

    /** Multiplies every element of @p a by scalar @p s. */
    static Matrix4x4 Multiply(Matrix4x4 a, float s)     { return a*s; }

    /** Negates all elements of @p m. */
    static Matrix4x4 Negate(Matrix4x4 m)                { return -m; }

    /**
     * Performs a linear interpolation between two matrices, element-wise.
     * @param a       The first matrix (returned when @p amount is 0).
     * @param b       The second matrix (returned when @p amount is 1).
     * @param amount  The relative weighting of @p b.
     */
    static Matrix4x4 Lerp(const Matrix4x4& a, const Matrix4x4& b, float amount) {
        return {
            a.M11+(b.M11-a.M11)*amount, a.M12+(b.M12-a.M12)*amount,
            a.M13+(b.M13-a.M13)*amount, a.M14+(b.M14-a.M14)*amount,
            a.M21+(b.M21-a.M21)*amount, a.M22+(b.M22-a.M22)*amount,
            a.M23+(b.M23-a.M23)*amount, a.M24+(b.M24-a.M24)*amount,
            a.M31+(b.M31-a.M31)*amount, a.M32+(b.M32-a.M32)*amount,
            a.M33+(b.M33-a.M33)*amount, a.M34+(b.M34-a.M34)*amount,
            a.M41+(b.M41-a.M41)*amount, a.M42+(b.M42-a.M42)*amount,
            a.M43+(b.M43-a.M43)*amount, a.M44+(b.M44-a.M44)*amount
        };
    }

    /** Returns the transpose of @p m (rows become columns). */
    static Matrix4x4 Transpose(const Matrix4x4& m) {
        return {m.M11,m.M21,m.M31,m.M41, m.M12,m.M22,m.M32,m.M42,
                m.M13,m.M23,m.M33,m.M43, m.M14,m.M24,m.M34,m.M44};
    }

    /** Returns the determinant via cofactor expansion along the first row. */
    [[nodiscard]] float GetDeterminant() const {
        float a = M11*(M22*(M33*M44-M34*M43)-M23*(M32*M44-M34*M42)+M24*(M32*M43-M33*M42));
        float b = M12*(M21*(M33*M44-M34*M43)-M23*(M31*M44-M34*M41)+M24*(M31*M43-M33*M41));
        float c = M13*(M21*(M32*M44-M34*M42)-M22*(M31*M44-M34*M41)+M24*(M31*M42-M32*M41));
        float d = M14*(M21*(M32*M43-M33*M42)-M22*(M31*M43-M33*M41)+M23*(M31*M42-M32*M41));
        return a - b + c - d;
    }

    /**
     * Attempts to invert @p m, writing the result to @p result.
     * @param m       The matrix to invert.
     * @param result  Receives the inverse on success, or a zero matrix on failure.
     * @return True if the matrix is invertible (non-zero determinant).
     */
    static bool Invert(const Matrix4x4& m, Matrix4x4& result) {
        // Cofactor expansion
        float a=m.M11,b=m.M12,c=m.M13,d=m.M14;
        float e=m.M21,f=m.M22,g=m.M23,h=m.M24;
        float i=m.M31,j=m.M32,k=m.M33,l=m.M34;
        float mn=m.M41,n=m.M42,o=m.M43,p=m.M44;

        float kp_lo=k*p-l*o, jp_ln=j*p-l*n, jo_kn=j*o-k*n;
        float ip_lm=i*p-l*mn, io_km=i*o-k*mn, in_jm=i*n-j*mn;
        float gp_ho=g*p-h*o, fp_hn=f*p-h*n, fo_gn=f*o-g*n;
        float ep_hm=e*p-h*mn, eo_gm=e*o-g*mn, en_fm=e*n-f*mn;
        float gl_hk=g*l-h*k, fl_hj=f*l-h*j, fk_gj=f*k-g*j;
        float el_hi=e*l-h*i, ek_gi=e*k-g*i, ej_fi=e*j-f*i;

        float det = a*(f*(kp_lo)-g*(jp_ln)+h*(jo_kn))
                  - b*(e*(kp_lo)-g*(ip_lm)+h*(io_km))
                  + c*(e*(jp_ln)-f*(ip_lm)+h*(in_jm))
                  - d*(e*(jo_kn)-f*(io_km)+g*(in_jm));
        if (std::abs(det) < std::numeric_limits<float>::denorm_min()) {
            constexpr float nan = std::numeric_limits<float>::quiet_NaN();
            result = Matrix4x4(nan, nan, nan, nan,
                                nan, nan, nan, nan,
                                nan, nan, nan, nan,
                                nan, nan, nan, nan);
            return false;
        }
        float inv = 1.0f / det;
        result.M11 = (f*(kp_lo)-g*(jp_ln)+h*(jo_kn))*inv;
        result.M21 = -(e*(kp_lo)-g*(ip_lm)+h*(io_km))*inv;
        result.M31 = (e*(jp_ln)-f*(ip_lm)+h*(in_jm))*inv;
        result.M41 = -(e*(jo_kn)-f*(io_km)+g*(in_jm))*inv;
        result.M12 = -(b*(kp_lo)-c*(jp_ln)+d*(jo_kn))*inv;
        result.M22 = (a*(kp_lo)-c*(ip_lm)+d*(io_km))*inv;
        result.M32 = -(a*(jp_ln)-b*(ip_lm)+d*(in_jm))*inv;
        result.M42 = (a*(jo_kn)-b*(io_km)+c*(in_jm))*inv;
        result.M13 = (b*(gp_ho)-c*(fp_hn)+d*(fo_gn))*inv;
        result.M23 = -(a*(gp_ho)-c*(ep_hm)+d*(eo_gm))*inv;
        result.M33 = (a*(fp_hn)-b*(ep_hm)+d*(en_fm))*inv;
        result.M43 = -(a*(fo_gn)-b*(eo_gm)+c*(en_fm))*inv;
        result.M14 = -(b*(gl_hk)-c*(fl_hj)+d*(fk_gj))*inv;
        result.M24 = (a*(gl_hk)-c*(el_hi)+d*(ek_gi))*inv;
        result.M34 = -(a*(fl_hj)-b*(el_hi)+d*(ej_fi))*inv;
        result.M44 = (a*(fk_gj)-b*(ek_gi)+c*(ej_fi))*inv;
        return true;
    }

    // --- Factory methods ---

    /**
     * Creates a translation matrix.
     * @param x  Translation along the X axis.
     * @param y  Translation along the Y axis.
     * @param z  Translation along the Z axis.
     */
    static Matrix4x4 CreateTranslation(float x, float y, float z) {
        auto m = Identity(); m.M41=x; m.M42=y; m.M43=z; return m;
    }

    /** Creates a translation matrix from a Vector3. */
    static Matrix4x4 CreateTranslation(Vector3 t) { return CreateTranslation(t.X,t.Y,t.Z); }

    /**
     * Creates a non-uniform scale matrix.
     * @param x  Scale factor along the X axis.
     * @param y  Scale factor along the Y axis.
     * @param z  Scale factor along the Z axis.
     */
    static Matrix4x4 CreateScale(float x, float y, float z) {
        auto m = Identity(); m.M11=x; m.M22=y; m.M33=z; return m;
    }

    /** Creates a non-uniform scale matrix from a Vector3. */
    static Matrix4x4 CreateScale(Vector3 s) { return CreateScale(s.X,s.Y,s.Z); }

    /** Creates a uniform scale matrix. */
    static Matrix4x4 CreateScale(float s)   { return CreateScale(s,s,s); }

    /**
     * Creates a rotation matrix for a rotation around the X axis.
     * @param radians  Angle in radians.
     */
    static Matrix4x4 CreateRotationX(float radians) {
        float c=std::cos(radians), s=std::sin(radians);
        auto m = Identity(); m.M22=c; m.M23=s; m.M32=-s; m.M33=c; return m;
    }

    /**
     * Creates a rotation matrix for a rotation around the Y axis.
     * @param radians  Angle in radians.
     */
    static Matrix4x4 CreateRotationY(float radians) {
        float c=std::cos(radians), s=std::sin(radians);
        auto m = Identity(); m.M11=c; m.M13=-s; m.M31=s; m.M33=c; return m;
    }

    /**
     * Creates a rotation matrix for a rotation around the Z axis.
     * @param radians  Angle in radians.
     */
    static Matrix4x4 CreateRotationZ(float radians) {
        float c=std::cos(radians), s=std::sin(radians);
        auto m = Identity(); m.M11=c; m.M12=s; m.M21=-s; m.M22=c; return m;
    }

    /** Creates a rotation matrix from a unit quaternion. */
    static Matrix4x4 CreateFromQuaternion(const Quaternion& q) {
        float xx=q.X*q.X, yy=q.Y*q.Y, zz=q.Z*q.Z;
        float xy=q.X*q.Y, xz=q.X*q.Z, yz=q.Y*q.Z;
        float wx=q.W*q.X, wy=q.W*q.Y, wz=q.W*q.Z;
        return {
            1-2*(yy+zz), 2*(xy+wz),   2*(xz-wy),   0,
            2*(xy-wz),   1-2*(xx+zz), 2*(yz+wx),   0,
            2*(xz+wy),   2*(yz-wx),   1-2*(xx+yy), 0,
            0,           0,           0,           1
        };
    }

    /**
     * Creates a rotation matrix from yaw, pitch, and roll angles (in radians).
     * @param yaw    Rotation around the Y axis.
     * @param pitch  Rotation around the X axis.
     * @param roll   Rotation around the Z axis.
     */
    static Matrix4x4 CreateFromYawPitchRoll(float yaw, float pitch, float roll) {
        return CreateFromQuaternion(Quaternion::CreateFromYawPitchRoll(yaw,pitch,roll));
    }

    /**
     * Creates a view matrix for a camera looking from @p eye toward @p target.
     * @param eye     Camera position in world space.
     * @param target  The point the camera looks at.
     * @param up      World-space up vector (usually Y+).
     */
    static Matrix4x4 CreateLookAt(Vector3 eye, Vector3 target, Vector3 up) {
        Vector3 zAxis = Vector3::Normalize(eye - target);
        Vector3 xAxis = Vector3::Normalize(Vector3::Cross(up, zAxis));
        Vector3 yAxis = Vector3::Cross(zAxis, xAxis);
        return {
            xAxis.X,             yAxis.X,             zAxis.X,             0,
            xAxis.Y,             yAxis.Y,             zAxis.Y,             0,
            xAxis.Z,             yAxis.Z,             zAxis.Z,             0,
            -Vector3::Dot(xAxis,eye), -Vector3::Dot(yAxis,eye), -Vector3::Dot(zAxis,eye), 1
        };
    }

    /**
     * Creates a perspective projection matrix from a vertical field-of-view angle.
     * @param fov     Vertical field-of-view angle in radians.
     * @param aspect  Viewport width divided by height.
     * @param near    Distance to the near clip plane.
     * @param far     Distance to the far clip plane.
     */
    static Matrix4x4 CreatePerspectiveFieldOfView(float fov, float aspect, float near, float far) {
        System::ArgumentOutOfRangeException::ThrowIfLessThanOrEqual(fov, 0.0f, "fieldOfView");
        System::ArgumentOutOfRangeException::ThrowIfGreaterThanOrEqual(fov, std::numbers::pi_v<float>, "fieldOfView");
        System::ArgumentOutOfRangeException::ThrowIfLessThanOrEqual(near, 0.0f, "nearPlaneDistance");
        System::ArgumentOutOfRangeException::ThrowIfLessThanOrEqual(far, 0.0f, "farPlaneDistance");
        System::ArgumentOutOfRangeException::ThrowIfGreaterThanOrEqual(near, far, "nearPlaneDistance");
        float ys = 1.0f/std::tan(fov*0.5f);
        float xs = ys/aspect;
        // Real .NET special-cases an infinite far plane to -1 rather than evaluating
        // far/(near-far), which would divide inf by -inf and yield NaN.
        float zn = std::isinf(far) ? -1.0f : far/(near-far);
        return {xs,0,0,0, 0,ys,0,0, 0,0,zn,-1, 0,0,near*zn,0};
    }

    /**
     * Creates an orthographic projection matrix.
     * @param w     Viewport width.
     * @param h     Viewport height.
     * @param near  Distance to the near clip plane.
     * @param far   Distance to the far clip plane.
     */
    static Matrix4x4 CreateOrthographic(float w, float h, float near, float far) {
        return {2/w,0,0,0, 0,2/h,0,0, 0,0,1/(near-far),0, 0,0,near/(near-far),1};
    }

    /** Returns a human-readable representation of all sixteen elements. */
    std::string ToString() const {
        std::ostringstream ss;
        ss.imbue(std::locale::classic());
        ss << "{ {M11:"<<M11<<" M12:"<<M12<<" M13:"<<M13<<" M14:"<<M14<<"} "
           << "{M21:"<<M21<<" M22:"<<M22<<" M23:"<<M23<<" M24:"<<M24<<"} "
           << "{M31:"<<M31<<" M32:"<<M32<<" M33:"<<M33<<" M34:"<<M34<<"} "
           << "{M41:"<<M41<<" M42:"<<M42<<" M43:"<<M43<<" M44:"<<M44<<"} }";
        return ss.str();
    }
};

// --- Deferred Transform implementations ---

/** Transforms point @p p by the upper-left 2x2 of @p m plus the translation row. */
inline Vector2 Vector2::Transform(Vector2 p, const Matrix4x4& m) {
    return {p.X*m.M11+p.Y*m.M21+m.M41, p.X*m.M12+p.Y*m.M22+m.M42};
}

/** Transforms direction @p n by the upper-left 2x2 of @p m (ignores translation). */
inline Vector2 Vector2::TransformNormal(Vector2 n, const Matrix4x4& m) {
    return {n.X*m.M11+n.Y*m.M21, n.X*m.M12+n.Y*m.M22};
}

/** Transforms point @p p by @p m (applies translation from row 4). */
inline Vector3 Vector3::Transform(Vector3 p, const Matrix4x4& m) {
    return {p.X*m.M11+p.Y*m.M21+p.Z*m.M31+m.M41,
            p.X*m.M12+p.Y*m.M22+p.Z*m.M32+m.M42,
            p.X*m.M13+p.Y*m.M23+p.Z*m.M33+m.M43};
}

/** Transforms direction @p n by @p m (ignores translation row). */
inline Vector3 Vector3::TransformNormal(Vector3 n, const Matrix4x4& m) {
    return {n.X*m.M11+n.Y*m.M21+n.Z*m.M31,
            n.X*m.M12+n.Y*m.M22+n.Z*m.M32,
            n.X*m.M13+n.Y*m.M23+n.Z*m.M33};
}

/** Rotates @p v by unit quaternion @p q. */
inline Vector3 Vector3::Transform(Vector3 v, const Quaternion& q) {
    Vector3 u{q.X,q.Y,q.Z};
    float s = q.W;
    return 2.0f*Vector3::Dot(u,v)*u + (s*s - Vector3::Dot(u,u))*v + 2.0f*s*Vector3::Cross(u,v);
}

/** Transforms 2D point @p p by @p m, producing a full Vector4. */
inline Vector4 Vector4::Transform(Vector2 p, const Matrix4x4& m) {
    return {p.X*m.M11+p.Y*m.M21+m.M41,
            p.X*m.M12+p.Y*m.M22+m.M42,
            p.X*m.M13+p.Y*m.M23+m.M43,
            p.X*m.M14+p.Y*m.M24+m.M44};
}

/** Transforms 3D point @p p by @p m, producing a full Vector4. */
inline Vector4 Vector4::Transform(Vector3 p, const Matrix4x4& m) {
    return {p.X*m.M11+p.Y*m.M21+p.Z*m.M31+m.M41,
            p.X*m.M12+p.Y*m.M22+p.Z*m.M32+m.M42,
            p.X*m.M13+p.Y*m.M23+p.Z*m.M33+m.M43,
            p.X*m.M14+p.Y*m.M24+p.Z*m.M34+m.M44};
}

/** Transforms Vector4 @p v by @p m. */
inline Vector4 Vector4::Transform(Vector4 v, const Matrix4x4& m) {
    return {v.X*m.M11+v.Y*m.M21+v.Z*m.M31+v.W*m.M41,
            v.X*m.M12+v.Y*m.M22+v.Z*m.M32+v.W*m.M42,
            v.X*m.M13+v.Y*m.M23+v.Z*m.M33+v.W*m.M43,
            v.X*m.M14+v.Y*m.M24+v.Z*m.M34+v.W*m.M44};
}

/** Rotates Vector4 @p v by unit quaternion @p q (W component is preserved). */
inline Vector4 Vector4::Transform(Vector4 v, const Quaternion& q) {
    Vector3 u{q.X,q.Y,q.Z};
    Vector3 vxyz{v.X,v.Y,v.Z};
    Vector3 r = Vector3::Transform(vxyz,q);
    return {r.X,r.Y,r.Z,v.W};
}

/** Constructs a unit quaternion from a rotation matrix @p m. */
inline Quaternion Quaternion::CreateFromRotationMatrix(const Matrix4x4& m) {
    float trace = m.M11+m.M22+m.M33;
    if (trace > 0) {
        float s = 0.5f/std::sqrt(trace+1.0f);
        return {(m.M23-m.M32)*s,(m.M31-m.M13)*s,(m.M12-m.M21)*s,0.25f/s};
    } else if (m.M11>=m.M22 && m.M11>=m.M33) {
        float s = 2.0f*std::sqrt(1.0f+m.M11-m.M22-m.M33);
        return {0.25f*s,(m.M12+m.M21)/s,(m.M31+m.M13)/s,(m.M23-m.M32)/s};
    } else if (m.M22>m.M33) {
        float s = 2.0f*std::sqrt(1.0f+m.M22-m.M11-m.M33);
        return {(m.M12+m.M21)/s,0.25f*s,(m.M23+m.M32)/s,(m.M31-m.M13)/s};
    } else {
        float s = 2.0f*std::sqrt(1.0f+m.M33-m.M11-m.M22);
        return {(m.M31+m.M13)/s,(m.M23+m.M32)/s,0.25f*s,(m.M12-m.M21)/s};
    }
}

} // namespace System::Numerics

// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include <gtest/gtest.h>
#include <cmath>
#include <limits>
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Numerics/Vector2.hpp"
#include "System/Numerics/Vector3.hpp"
#include "System/Numerics/Vector4.hpp"
#include "System/Numerics/Matrix3x2.hpp"
#include "System/Numerics/Matrix4x4.hpp"
#include "System/Numerics/Quaternion.hpp"
#include "System/Numerics/Plane.hpp"

using namespace System::Numerics;

static bool near(float a, float b, float eps = 1e-5f) { return std::abs(a - b) < eps; }

// --- Vector2 ---
TEST(Vector2Tests, BasicArithmetic) {
    Vector2 a{1,2}, b{3,4};
    EXPECT_EQ((a+b), Vector2(4,6));
    EXPECT_EQ((a-b), Vector2(-2,-2));
    EXPECT_EQ((a*b), Vector2(3,8));
    EXPECT_EQ((a*2.0f), Vector2(2,4));
    EXPECT_EQ((a/2.0f), Vector2(0.5f,1.0f));
}
TEST(Vector2Tests, Dot) { EXPECT_FLOAT_EQ(Vector2::Dot({1,0},{0,1}), 0.0f); }
TEST(Vector2Tests, Length) { EXPECT_TRUE(near(Vector2(3,4).Length(), 5.0f)); }
TEST(Vector2Tests, Normalize) {
    auto n = Vector2::Normalize({3,4});
    EXPECT_TRUE(near(n.Length(), 1.0f));
}
TEST(Vector2Tests, Distance) { EXPECT_TRUE(near(Vector2::Distance({0,0},{3,4}), 5.0f)); }
TEST(Vector2Tests, Lerp) { EXPECT_EQ(Vector2::Lerp({0,0},{10,10},0.5f), Vector2(5,5)); }
TEST(Vector2Tests, Zero)  { EXPECT_EQ(Vector2::Zero(),  Vector2(0,0)); }
TEST(Vector2Tests, One)   { EXPECT_EQ(Vector2::One(),   Vector2(1,1)); }
TEST(Vector2Tests, UnitX) { EXPECT_EQ(Vector2::UnitX(), Vector2(1,0)); }

// --- Vector3 ---
TEST(Vector3Tests, Cross) {
    auto c = Vector3::Cross({1,0,0},{0,1,0});
    EXPECT_TRUE(near(c.X,0)); EXPECT_TRUE(near(c.Y,0)); EXPECT_TRUE(near(c.Z,1));
}
TEST(Vector3Tests, Normalize) {
    auto n = Vector3::Normalize({1,2,3});
    EXPECT_TRUE(near(n.Length(), 1.0f));
}
TEST(Vector3Tests, Lerp) { EXPECT_EQ(Vector3::Lerp({0,0,0},{2,4,6},0.5f), Vector3(1,2,3)); }

// --- Vector4 ---
TEST(Vector4Tests, BasicArithmetic) {
    Vector4 a{1,2,3,4}, b{5,6,7,8};
    EXPECT_EQ((a+b), Vector4(6,8,10,12));
}
TEST(Vector4Tests, Length) { EXPECT_TRUE(near(Vector4(1,1,1,1).Length(), 2.0f)); }

// --- Matrix3x2 ---
TEST(Matrix3x2Tests, Identity) {
    EXPECT_TRUE(Matrix3x2::Identity().getIsIdentityProperty());
}
TEST(Matrix3x2Tests, Multiply) {
    auto t = Matrix3x2::CreateTranslation(3,4);
    auto p = Vector2::Transform({1,1}, t);
    EXPECT_TRUE(near(p.X, 4.0f));
    EXPECT_TRUE(near(p.Y, 5.0f));
}
TEST(Matrix3x2Tests, Invert) {
    auto m = Matrix3x2::CreateTranslation(5,3);
    Matrix3x2 inv;
    EXPECT_TRUE(Matrix3x2::Invert(m, inv));
    auto combined = m * inv;
    EXPECT_TRUE(combined.getIsIdentityProperty());
}
TEST(Matrix3x2Tests, CreateRotation) {
    auto m = Matrix3x2::CreateRotation(0.0f);
    EXPECT_TRUE(m.getIsIdentityProperty());
}
TEST(Matrix3x2Tests, Invert_SingularMatrix_ReturnsFalseAndNaN) {
    Matrix3x2 singular{1, 2, 2, 4, 0, 0}; // determinant == 0
    Matrix3x2 inv;
    EXPECT_FALSE(Matrix3x2::Invert(singular, inv));
    EXPECT_TRUE(std::isnan(inv.M11));
    EXPECT_TRUE(std::isnan(inv.M12));
    EXPECT_TRUE(std::isnan(inv.M21));
    EXPECT_TRUE(std::isnan(inv.M22));
    EXPECT_TRUE(std::isnan(inv.M31));
    EXPECT_TRUE(std::isnan(inv.M32));
}

// --- Matrix4x4 ---
TEST(Matrix4x4Tests, Identity) {
    EXPECT_TRUE(Matrix4x4::Identity().getIsIdentityProperty());
}
TEST(Matrix4x4Tests, Multiply) {
    auto a = Matrix4x4::Identity();
    auto b = Matrix4x4::CreateTranslation(1,2,3);
    auto c = a * b;
    EXPECT_TRUE(near(c.M41, 1.0f));
    EXPECT_TRUE(near(c.M42, 2.0f));
    EXPECT_TRUE(near(c.M43, 3.0f));
}
TEST(Matrix4x4Tests, Invert) {
    auto m = Matrix4x4::CreateTranslation(5,6,7);
    Matrix4x4 inv;
    EXPECT_TRUE(Matrix4x4::Invert(m, inv));
    auto r = m * inv;
    EXPECT_TRUE(r.getIsIdentityProperty());
}
TEST(Matrix4x4Tests, Invert_SingularMatrix_ReturnsFalseAndNaN) {
    Matrix4x4 singular{}; // all-zero matrix: determinant == 0
    Matrix4x4 inv;
    EXPECT_FALSE(Matrix4x4::Invert(singular, inv));
    EXPECT_TRUE(std::isnan(inv.M11));
    EXPECT_TRUE(std::isnan(inv.M44));
}
TEST(Matrix4x4Tests, CreatePerspectiveFieldOfView_NonPositiveFov_Throws) {
    EXPECT_THROW(Matrix4x4::CreatePerspectiveFieldOfView(0.0f, 1.0f, 0.1f, 100.0f), System::ArgumentOutOfRangeException);
    EXPECT_THROW(Matrix4x4::CreatePerspectiveFieldOfView(-1.0f, 1.0f, 0.1f, 100.0f), System::ArgumentOutOfRangeException);
}
TEST(Matrix4x4Tests, CreatePerspectiveFieldOfView_FovTooLarge_Throws) {
    EXPECT_THROW(Matrix4x4::CreatePerspectiveFieldOfView(3.2f, 1.0f, 0.1f, 100.0f), System::ArgumentOutOfRangeException);
}
TEST(Matrix4x4Tests, CreatePerspectiveFieldOfView_NonPositiveNearOrFar_Throws) {
    EXPECT_THROW(Matrix4x4::CreatePerspectiveFieldOfView(1.0f, 1.0f, 0.0f, 100.0f), System::ArgumentOutOfRangeException);
    EXPECT_THROW(Matrix4x4::CreatePerspectiveFieldOfView(1.0f, 1.0f, 0.1f, 0.0f), System::ArgumentOutOfRangeException);
}
TEST(Matrix4x4Tests, CreatePerspectiveFieldOfView_NearGreaterThanFar_Throws) {
    EXPECT_THROW(Matrix4x4::CreatePerspectiveFieldOfView(1.0f, 1.0f, 100.0f, 0.1f), System::ArgumentOutOfRangeException);
}
TEST(Matrix4x4Tests, CreatePerspectiveFieldOfView_ValidArgs_NoThrow) {
    EXPECT_NO_THROW(Matrix4x4::CreatePerspectiveFieldOfView(1.0f, 1.0f, 0.1f, 100.0f));
}
TEST(Matrix4x4Tests, CreatePerspectiveFieldOfView_InfiniteFar_ProducesFiniteRangeNotNaN) {
    // Real .NET special-cases farPlaneDistance == +Infinity to range = -1 rather than
    // evaluating far/(near-far), which would divide inf by -inf and yield NaN.
    auto m = Matrix4x4::CreatePerspectiveFieldOfView(1.0f, 1.0f, 0.1f, std::numeric_limits<float>::infinity());
    EXPECT_FALSE(std::isnan(m.M33));
    EXPECT_FALSE(std::isnan(m.M43));
    EXPECT_TRUE(near(m.M33, -1.0f));
    EXPECT_TRUE(near(m.M43, -0.1f));
}
TEST(Matrix4x4Tests, TransformVector3) {
    auto m = Matrix4x4::CreateTranslation(10,0,0);
    auto v = Vector3::Transform({0,0,0}, m);
    EXPECT_TRUE(near(v.X, 10.0f));
}
TEST(Matrix4x4Tests, CreateRotationX) {
    auto m = Matrix4x4::CreateRotationX(0.0f);
    EXPECT_TRUE(m.getIsIdentityProperty());
}
TEST(Matrix4x4Tests, Transpose) {
    auto m = Matrix4x4{1,2,3,4, 5,6,7,8, 9,10,11,12, 13,14,15,16};
    auto t = Matrix4x4::Transpose(m);
    EXPECT_FLOAT_EQ(t.M12, 5.0f);
    EXPECT_FLOAT_EQ(t.M21, 2.0f);
}
TEST(Matrix4x4Tests, GetHashCode_EqualMatrices_SameHash) {
    auto a = Matrix4x4{1,2,3,4, 5,6,7,8, 9,10,11,12, 13,14,15,16};
    auto b = Matrix4x4{1,2,3,4, 5,6,7,8, 9,10,11,12, 13,14,15,16};
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}
TEST(Matrix4x4Tests, GetHashCode_DifferentMatrices_DifferentHash) {
    auto a = Matrix4x4::Identity();
    auto b = Matrix4x4::CreateTranslation(1,2,3);
    EXPECT_NE(a.GetHashCode(), b.GetHashCode());
}
TEST(Matrix4x4Tests, Lerp_ZeroAmount_ReturnsFirst) {
    auto a = Matrix4x4::Identity();
    auto b = Matrix4x4::CreateTranslation(10,20,30);
    auto r = Matrix4x4::Lerp(a, b, 0.0f);
    EXPECT_TRUE(r.Equals(a));
}
TEST(Matrix4x4Tests, Lerp_OneAmount_ReturnsSecond) {
    auto a = Matrix4x4::Identity();
    auto b = Matrix4x4::CreateTranslation(10,20,30);
    auto r = Matrix4x4::Lerp(a, b, 1.0f);
    EXPECT_TRUE(r.Equals(b));
}
TEST(Matrix4x4Tests, Lerp_HalfAmount_Midpoint) {
    auto a = Matrix4x4::CreateTranslation(0,0,0);
    auto b = Matrix4x4::CreateTranslation(10,20,30);
    auto r = Matrix4x4::Lerp(a, b, 0.5f);
    EXPECT_TRUE(near(r.M41, 5.0f));
    EXPECT_TRUE(near(r.M42, 10.0f));
    EXPECT_TRUE(near(r.M43, 15.0f));
}

// --- Quaternion ---
TEST(QuaternionTests, Identity) {
    EXPECT_TRUE(Quaternion::Identity().getIsIdentityProperty());
}
TEST(QuaternionTests, Normalize) {
    auto q = Quaternion::Normalize({1,2,3,4});
    EXPECT_TRUE(near(q.Length(), 1.0f));
}
TEST(QuaternionTests, Conjugate) {
    auto q = Quaternion{1,2,3,4};
    auto c = Quaternion::Conjugate(q);
    EXPECT_FLOAT_EQ(c.X, -1.0f);
    EXPECT_FLOAT_EQ(c.W,  4.0f);
}
TEST(QuaternionTests, CreateFromAxisAngle) {
    auto q = Quaternion::CreateFromAxisAngle({0,0,1}, 0.0f);
    EXPECT_TRUE(near(q.W, 1.0f));
}
TEST(QuaternionTests, RoundtripMatrix) {
    auto q = Quaternion::CreateFromYawPitchRoll(0.3f, 0.2f, 0.1f);
    auto m = Matrix4x4::CreateFromQuaternion(q);
    auto q2 = Quaternion::CreateFromRotationMatrix(m);
    EXPECT_TRUE(near(std::abs(Quaternion::Dot(q,q2)), 1.0f, 1e-4f));
}
TEST(QuaternionTests, Inverse_ZeroQuaternion_ReturnsZero) {
    auto q = Quaternion::Inverse(Quaternion::Zero());
    EXPECT_EQ(q, Quaternion::Zero());
}
TEST(QuaternionTests, Inverse_NearZeroLength_ReturnsZero) {
    // Regression: previously only exact-zero length short-circuited to a safe result;
    // a near-zero-but-nonzero length blew up to huge/near-infinite components instead of
    // matching .NET's epsilon-guarded Zero() fallback (Quaternion.cs Inverse).
    Quaternion q{1e-5f, 0, 0, 0};
    ASSERT_LE(q.LengthSquared(), 1.192092896e-7f);
    EXPECT_EQ(Quaternion::Inverse(q), Quaternion::Zero());
}
TEST(QuaternionTests, Inverse_UnitQuaternion_MatchesConjugate) {
    auto q = Quaternion::Normalize({1,2,3,4});
    auto inv = Quaternion::Inverse(q);
    auto conj = Quaternion::Conjugate(q);
    EXPECT_TRUE(near(inv.X, conj.X));
    EXPECT_TRUE(near(inv.W, conj.W));
}
TEST(QuaternionTests, Normalize_ZeroQuaternion_ProducesNaN) {
    // Matches .NET exactly: Normalize is an unconditional value/Length() with no zero
    // guard (Quaternion.cs), so a zero-length input propagates NaN rather than being
    // silently caught.
    auto q = Quaternion::Normalize(Quaternion::Zero());
    EXPECT_TRUE(std::isnan(q.X));
}
TEST(QuaternionTests, Slerp_TightlyAlignedButNotWithinLooseThreshold_UsesSphericalPath) {
    // Regression: the old 0.9999f threshold took the linear-interpolation shortcut for
    // quaternion pairs .NET still slerps via the full spherical formula (SlerpEpsilon is
    // 1e-6f in Quaternion.cs, i.e. threshold 0.999999f). Construct a pair whose dot product
    // falls strictly between the two thresholds and confirm the interpolated result still
    // lands on the unit sphere (both paths preserve this, so instead assert the two
    // thresholds actually disagree on which branch a nearby-but-not-coincident pair takes).
    auto a = Quaternion::Identity();
    auto b = Quaternion::Normalize(Quaternion::CreateFromAxisAngle({0,0,1}, 0.01f));
    float cosAngle = Quaternion::Dot(a, b);
    ASSERT_GT(cosAngle, 0.9999f);
    ASSERT_LE(cosAngle, 1.0f - 1e-6f);
    auto slerped = Quaternion::Slerp(a, b, 0.5f);
    EXPECT_TRUE(near(slerped.Length(), 1.0f));
}
TEST(QuaternionTests, DivideOperator_MatchesStaticDivide) {
    auto a = Quaternion::Normalize({1,2,3,4});
    auto b = Quaternion::Normalize({4,3,2,1});
    auto viaOperator = a / b;
    auto viaStatic = Quaternion::Divide(a, b);
    EXPECT_TRUE(near(viaOperator.X, viaStatic.X));
    EXPECT_TRUE(near(viaOperator.W, viaStatic.W));
}
TEST(QuaternionTests, Concatenate_MatchesReverseMultiplyOrder) {
    auto a = Quaternion::CreateFromAxisAngle({0,0,1}, 0.3f);
    auto b = Quaternion::CreateFromAxisAngle({1,0,0}, 0.2f);
    auto viaConcatenate = Quaternion::Concatenate(a, b);
    auto viaMultiply = b * a;
    EXPECT_TRUE(near(viaConcatenate.X, viaMultiply.X));
    EXPECT_TRUE(near(viaConcatenate.W, viaMultiply.W));
}

// --- Plane ---
TEST(PlaneTests, DotCoordinate) {
    Plane p{{0,1,0}, -1.0f};
    EXPECT_FLOAT_EQ(Plane::DotCoordinate(p, {0,1,0}), 0.0f);
}
TEST(PlaneTests, Normalize) {
    Plane p{2,0,0,-4};
    auto n = Plane::Normalize(p);
    EXPECT_TRUE(near(n.Normal.Length(), 1.0f));
    EXPECT_TRUE(near(n.D, -2.0f));
}
TEST(PlaneTests, CreateFromVertices) {
    auto p = Plane::CreateFromVertices({0,0,0},{1,0,0},{0,1,0});
    EXPECT_TRUE(near(std::abs(p.Normal.Z), 1.0f));
}

// --- Colors ---
#include "System/Numerics/Colors/Colors.hpp"
using namespace System::Numerics::Colors;
TEST(ColorsTests, ArgbRoundtrip) {
    Argb<uint8_t> a{255,10,20,30};
    auto r = a.ToRgba();
    EXPECT_EQ(r.R, 10); EXPECT_EQ(r.G, 20); EXPECT_EQ(r.B, 30); EXPECT_EQ(r.A, 255);
    auto back = r.ToArgb();
    EXPECT_TRUE(back.Equals(a));
}

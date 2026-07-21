// SPDX-License-Identifier: MS-PL

#include "Microsoft/Xna/Framework/Matrix.hpp"

#include <cmath>
#include <sstream>
#include <stdexcept>

namespace Microsoft::Xna::Framework
{
    const Matrix Matrix::getIdentityProperty()
    {
        static const Matrix identity = Matrix(
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        );
        return identity;
    }

    Matrix::Matrix()
        : M11(0.0f), M12(0.0f), M13(0.0f), M14(0.0f),
          M21(0.0f), M22(0.0f), M23(0.0f), M24(0.0f),
          M31(0.0f), M32(0.0f), M33(0.0f), M34(0.0f),
          M41(0.0f), M42(0.0f), M43(0.0f), M44(0.0f)
    {
    }

    Matrix::Matrix(
        float m11, float m12, float m13, float m14,
        float m21, float m22, float m23, float m24,
        float m31, float m32, float m33, float m34,
        float m41, float m42, float m43, float m44
    )
        : M11(m11), M12(m12), M13(m13), M14(m14),
          M21(m21), M22(m22), M23(m23), M24(m24),
          M31(m31), M32(m32), M33(m33), M34(m34),
          M41(m41), M42(m42), M43(m43), M44(m44)
    {
    }

    Vector3 Matrix::getTranslationProperty() const { return Vector3(M41, M42, M43); }

    bool Matrix::Equals(const Matrix& other) const
    {
        return M11 == other.M11 && M12 == other.M12 && M13 == other.M13 && M14 == other.M14 &&
            M21 == other.M21 && M22 == other.M22 && M23 == other.M23 && M24 == other.M24 &&
            M31 == other.M31 && M32 == other.M32 && M33 == other.M33 && M34 == other.M34 &&
            M41 == other.M41 && M42 == other.M42 && M43 == other.M43 && M44 == other.M44;
    }

    std::string Matrix::getDebugDisplayStringProperty() const
    {
        std::ostringstream stream;
        stream << "( " << M11 << " " << M12 << " " << M13 << " " << M14 << " ) \r\n"
            << "( " << M21 << " " << M22 << " " << M23 << " " << M24 << " ) \r\n"
            << "( " << M31 << " " << M32 << " " << M33 << " " << M34 << " ) \r\n"
            << "( " << M41 << " " << M42 << " " << M43 << " " << M44 << " )";
        return stream.str();
    }

    void Matrix::CheckForNaNs() const
    {
#if !defined(NDEBUG)
        if (std::isnan(M11) || std::isnan(M12) || std::isnan(M13) || std::isnan(M14) ||
            std::isnan(M21) || std::isnan(M22) || std::isnan(M23) || std::isnan(M24) ||
            std::isnan(M31) || std::isnan(M32) || std::isnan(M33) || std::isnan(M34) ||
            std::isnan(M41) || std::isnan(M42) || std::isnan(M43) || std::isnan(M44))
        {
            throw std::logic_error("Matrix contains NaNs!");
        }
#endif
    }

    Matrix Matrix::Invert(Matrix matrix)
    {
        Matrix result;
        Invert(matrix, result);
        return result;
    }

    void Matrix::Invert(const Matrix& matrix, Matrix& result)
    {
        /*
         * Use Laplace expansion theorem to calculate the inverse of a 4x4 matrix.
         *
         * 1. Calculate the 2x2 determinants needed the 4x4 determinant based on
         *    the 2x2 determinants.
         * 3. Create the adjugate matrix, which satisfies: A * adj(A) = det(A) * I.
         * 4. Divide adjugate matrix with the determinant to find the inverse.
         */
        // FNA casts each operand to double before multiplying to gain extra precision;
        // CNA uses plain float arithmetic for simplicity (no observable difference in practice).
        const float num1 = matrix.M11;
        const float num2 = matrix.M12;
        const float num3 = matrix.M13;
        const float num4 = matrix.M14;
        const float num5 = matrix.M21;
        const float num6 = matrix.M22;
        const float num7 = matrix.M23;
        const float num8 = matrix.M24;
        const float num9 = matrix.M31;
        const float num10 = matrix.M32;
        const float num11 = matrix.M33;
        const float num12 = matrix.M34;
        const float num13 = matrix.M41;
        const float num14 = matrix.M42;
        const float num15 = matrix.M43;
        const float num16 = matrix.M44;

        const float num17 = num11 * num16 - num12 * num15;
        const float num18 = num10 * num16 - num12 * num14;
        const float num19 = num10 * num15 - num11 * num14;
        const float num20 = num9 * num16 - num12 * num13;
        const float num21 = num9 * num15 - num11 * num13;
        const float num22 = num9 * num14 - num10 * num13;
        const float num23 = num6 * num17 - num7 * num18 + num8 * num19;
        const float num24 = -(num5 * num17 - num7 * num20 + num8 * num21);
        const float num25 = num5 * num18 - num6 * num20 + num8 * num22;
        const float num26 = -(num5 * num19 - num6 * num21 + num7 * num22);
        const float num27 = 1.0f / (num1 * num23 + num2 * num24 + num3 * num25 + num4 * num26);

        result.M11 = num23 * num27;
        result.M21 = num24 * num27;
        result.M31 = num25 * num27;
        result.M41 = num26 * num27;
        result.M12 = -(num2 * num17 - num3 * num18 + num4 * num19) * num27;
        result.M22 = (num1 * num17 - num3 * num20 + num4 * num21) * num27;
        result.M32 = -(num1 * num18 - num2 * num20 + num4 * num22) * num27;
        result.M42 = (num1 * num19 - num2 * num21 + num3 * num22) * num27;

        const float num28 = num7 * num16 - num8 * num15;
        const float num29 = num6 * num16 - num8 * num14;
        const float num30 = num6 * num15 - num7 * num14;
        const float num31 = num5 * num16 - num8 * num13;
        const float num32 = num5 * num15 - num7 * num13;
        const float num33 = num5 * num14 - num6 * num13;

        result.M13 = (num2 * num28 - num3 * num29 + num4 * num30) * num27;
        result.M23 = -(num1 * num28 - num3 * num31 + num4 * num32) * num27;
        result.M33 = (num1 * num29 - num2 * num31 + num4 * num33) * num27;
        result.M43 = -(num1 * num30 - num2 * num32 + num3 * num33) * num27;

        const float num34 = num7 * num12 - num8 * num11;
        const float num35 = num6 * num12 - num8 * num10;
        const float num36 = num6 * num11 - num7 * num10;
        const float num37 = num5 * num12 - num8 * num9;
        const float num38 = num5 * num11 - num7 * num9;
        const float num39 = num5 * num10 - num6 * num9;

        result.M14 = -(num2 * num34 - num3 * num35 + num4 * num36) * num27;
        result.M24 = (num1 * num34 - num3 * num37 + num4 * num38) * num27;
        result.M34 = -(num1 * num35 - num2 * num37 + num4 * num39) * num27;
        result.M44 = (num1 * num36 - num2 * num38 + num3 * num39) * num27;
    }

    Matrix Matrix::Multiply(Matrix matrix1, Matrix matrix2)
    {
        Matrix result;
        Multiply(matrix1, matrix2, result);
        return result;
    }

    void Matrix::Multiply(const Matrix& matrix1, const Matrix& matrix2, Matrix& result)
    {
        result.M11 = matrix1.M11 * matrix2.M11 + matrix1.M12 * matrix2.M21 + matrix1.M13 * matrix2.M31 + matrix1.M14 *
            matrix2.M41;
        result.M12 = matrix1.M11 * matrix2.M12 + matrix1.M12 * matrix2.M22 + matrix1.M13 * matrix2.M32 + matrix1.M14 *
            matrix2.M42;
        result.M13 = matrix1.M11 * matrix2.M13 + matrix1.M12 * matrix2.M23 + matrix1.M13 * matrix2.M33 + matrix1.M14 *
            matrix2.M43;
        result.M14 = matrix1.M11 * matrix2.M14 + matrix1.M12 * matrix2.M24 + matrix1.M13 * matrix2.M34 + matrix1.M14 *
            matrix2.M44;

        result.M21 = matrix1.M21 * matrix2.M11 + matrix1.M22 * matrix2.M21 + matrix1.M23 * matrix2.M31 + matrix1.M24 *
            matrix2.M41;
        result.M22 = matrix1.M21 * matrix2.M12 + matrix1.M22 * matrix2.M22 + matrix1.M23 * matrix2.M32 + matrix1.M24 *
            matrix2.M42;
        result.M23 = matrix1.M21 * matrix2.M13 + matrix1.M22 * matrix2.M23 + matrix1.M23 * matrix2.M33 + matrix1.M24 *
            matrix2.M43;
        result.M24 = matrix1.M21 * matrix2.M14 + matrix1.M22 * matrix2.M24 + matrix1.M23 * matrix2.M34 + matrix1.M24 *
            matrix2.M44;

        result.M31 = matrix1.M31 * matrix2.M11 + matrix1.M32 * matrix2.M21 + matrix1.M33 * matrix2.M31 + matrix1.M34 *
            matrix2.M41;
        result.M32 = matrix1.M31 * matrix2.M12 + matrix1.M32 * matrix2.M22 + matrix1.M33 * matrix2.M32 + matrix1.M34 *
            matrix2.M42;
        result.M33 = matrix1.M31 * matrix2.M13 + matrix1.M32 * matrix2.M23 + matrix1.M33 * matrix2.M33 + matrix1.M34 *
            matrix2.M43;
        result.M34 = matrix1.M31 * matrix2.M14 + matrix1.M32 * matrix2.M24 + matrix1.M33 * matrix2.M34 + matrix1.M34 *
            matrix2.M44;

        result.M41 = matrix1.M41 * matrix2.M11 + matrix1.M42 * matrix2.M21 + matrix1.M43 * matrix2.M31 + matrix1.M44 *
            matrix2.M41;
        result.M42 = matrix1.M41 * matrix2.M12 + matrix1.M42 * matrix2.M22 + matrix1.M43 * matrix2.M32 + matrix1.M44 *
            matrix2.M42;
        result.M43 = matrix1.M41 * matrix2.M13 + matrix1.M42 * matrix2.M23 + matrix1.M43 * matrix2.M33 + matrix1.M44 *
            matrix2.M43;
        result.M44 = matrix1.M41 * matrix2.M14 + matrix1.M42 * matrix2.M24 + matrix1.M43 * matrix2.M34 + matrix1.M44 *
            matrix2.M44;
    }

    Matrix Matrix::Transpose(Matrix matrix)
    {
        Matrix result;
        Transpose(matrix, result);
        return result;
    }

    void Matrix::Transpose(const Matrix& matrix, Matrix& result)
    {
        result.M11 = matrix.M11;
        result.M12 = matrix.M21;
        result.M13 = matrix.M31;
        result.M14 = matrix.M41;
        result.M21 = matrix.M12;
        result.M22 = matrix.M22;
        result.M23 = matrix.M32;
        result.M24 = matrix.M42;
        result.M31 = matrix.M13;
        result.M32 = matrix.M23;
        result.M33 = matrix.M33;
        result.M34 = matrix.M43;
        result.M41 = matrix.M14;
        result.M42 = matrix.M24;
        result.M43 = matrix.M34;
        result.M44 = matrix.M44;
    }

    bool operator!=(Matrix matrix1, Matrix matrix2) { return !matrix1.Equals(matrix2); }
    Matrix operator*(Matrix matrix1, Matrix matrix2) { return Matrix::Multiply(matrix1, matrix2); }

    void Matrix::ToColumnMajor(float out[16]) const
    {
        out[0] = M11;
        out[1] = M12;
        out[2] = M13;
        out[3] = M14;
        out[4] = M21;
        out[5] = M22;
        out[6] = M23;
        out[7] = M24;
        out[8] = M31;
        out[9] = M32;
        out[10] = M33;
        out[11] = M34;
        out[12] = M41;
        out[13] = M42;
        out[14] = M43;
        out[15] = M44;
    }
}

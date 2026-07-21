// SPDX-License-Identifier: MS-PL

#pragma once

#include <string>

#include "CNA/CNAHelper.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

namespace Microsoft::Xna::Framework
{
    /** @brief Represents a right-handed 4x4 matrix storing translation, scale and rotation data. */
    struct Matrix
    {
        /**
         * @brief Returns the identity matrix.
         *
         * @return The identity matrix.
         */
        static const Matrix getIdentityProperty();

        /** @brief A first row and first column value. */
        float M11;
        /** @brief A first row and second column value. */
        float M12;
        /** @brief A first row and third column value. */
        float M13;
        /** @brief A first row and fourth column value. */
        float M14;

        /** @brief A second row and first column value. */
        float M21;
        /** @brief A second row and second column value. */
        float M22;
        /** @brief A second row and third column value. */
        float M23;
        /** @brief A second row and fourth column value. */
        float M24;

        /** @brief A third row and first column value. */
        float M31;
        /** @brief A third row and second column value. */
        float M32;
        /** @brief A third row and third column value. */
        float M33;
        /** @brief A third row and fourth column value. */
        float M34;

        /** @brief A fourth row and first column value. */
        float M41;
        /** @brief A fourth row and second column value. */
        float M42;
        /** @brief A fourth row and third column value. */
        float M43;
        /** @brief A fourth row and fourth column value. */
        float M44;

        /** @brief Constructs a zero-filled matrix. */
        Matrix();

        /**
         * @brief Constructs a matrix from all 16 row-major field values.
         *
         * @param m11 Value for row 1, column 1.
         * @param m12 Value for row 1, column 2.
         * @param m13 Value for row 1, column 3.
         * @param m14 Value for row 1, column 4.
         * @param m21 Value for row 2, column 1.
         * @param m22 Value for row 2, column 2.
         * @param m23 Value for row 2, column 3.
         * @param m24 Value for row 2, column 4.
         * @param m31 Value for row 3, column 1.
         * @param m32 Value for row 3, column 2.
         * @param m33 Value for row 3, column 3.
         * @param m34 Value for row 3, column 4.
         * @param m41 Value for row 4, column 1.
         * @param m42 Value for row 4, column 2.
         * @param m43 Value for row 4, column 3.
         * @param m44 Value for row 4, column 4.
         */
        Matrix(
            float m11, float m12, float m13, float m14,
            float m21, float m22, float m23, float m24,
            float m31, float m32, float m33, float m34,
            float m41, float m42, float m43, float m44
        );

        /**
         * @brief Gets the translation stored in this matrix.
         *
         * @return The translation vector.
         */
        [[nodiscard]] Vector3 getTranslationProperty() const;

        /**
         * @brief Compares this matrix with another matrix without tolerance.
         *
         * @param other The matrix to compare against.
         * @return @c true if the matrices are equal; @c false otherwise.
         */
        [[nodiscard]] bool Equals(const Matrix& other) const;

        /**
         * @brief Returns the inverse of a matrix.
         *
         * @param matrix The matrix to invert.
         * @return The inverted matrix.
         */
        [[nodiscard]] static Matrix Invert(Matrix matrix);

        /**
         * @brief Returns the inverse of a matrix in an output parameter.
         *
         * @param matrix The matrix to invert.
         * @param result Output matrix that receives the inverse.
         */
        static void Invert(const Matrix& matrix, Matrix& result);

        /**
         * @brief Multiplies two matrices.
         *
         * @param matrix1 Left-hand matrix.
         * @param matrix2 Right-hand matrix.
         * @return The matrix product.
         */
        [[nodiscard]] static Matrix Multiply(Matrix matrix1, Matrix matrix2);

        /**
         * @brief Multiplies two matrices in an output parameter.
         *
         * @param matrix1 Left-hand matrix.
         * @param matrix2 Right-hand matrix.
         * @param result Output matrix that receives the product.
         */
        static void Multiply(const Matrix& matrix1, const Matrix& matrix2, Matrix& result);

        /**
         * @brief Transposes a matrix.
         *
         * @param matrix Source matrix.
         * @return The transposed matrix.
         */
        [[nodiscard]] static Matrix Transpose(Matrix matrix);

        /**
         * @brief Transposes a matrix in an output parameter.
         *
         * @param matrix Source matrix.
         * @param result Output matrix that receives the transposed result.
         */
        static void Transpose(const Matrix& matrix, Matrix& result);

        /**
         * @brief Returns true when any element differs between the two matrices.
         *
         * @param matrix1 Left-hand matrix.
         * @param matrix2 Right-hand matrix.
         * @return @c true if the matrices are not equal; @c false otherwise.
         */
        friend bool operator!=(Matrix matrix1, Matrix matrix2);

        /**
         * @brief Multiplies two matrices using standard matrix multiplication.
         *
         * @param matrix1 Left-hand matrix.
         * @param matrix2 Right-hand matrix.
         * @return The matrix product.
         */
        friend Matrix operator*(Matrix matrix1, Matrix matrix2);

        /**
         * @brief Returns this matrix in the transposed column-major form expected by OpenGL-style uniform uploads.
         *
         * @param out Array of 16 floats that receives the column-major matrix data.
         */
        NOXNA void ToColumnMajor(float out[16]) const;

    private:
        [[nodiscard]] std::string getDebugDisplayStringProperty() const;
        void CheckForNaNs() const;
    };
}

// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <cstdint>
#include <cstring>
#include <locale>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentException.hpp"
#include "System/BitConverter.hpp"
#include "System/Buffers/Binary/BinaryPrimitives.hpp"
#include "System/HashCode.hpp"

namespace System::Numerics::Colors {

template<typename T> struct Rgba;

/** Represents a color in ARGB (Alpha, Red, Green, Blue) order. */
template<typename T>
struct Argb {
    T A{}; ///< Alpha channel.
    T R{}; ///< Red channel.
    T G{}; ///< Green channel.
    T B{}; ///< Blue channel.

    /** Default constructor — all channels zero-initialised. */
    Argb() = default;
    /**
     * @brief Constructs an Argb from individual channel values.
     * @param a Alpha value.
     * @param r Red value.
     * @param g Green value.
     * @param b Blue value.
     */
    Argb(T a, T r, T g, T b) : A(a), R(r), G(g), B(b) {}
    /**
     * @brief Constructs an Argb from a vector with at least 4 elements (A, R, G, B).
     * @param values Source vector; must have at least 4 elements.
     * @throws System::ArgumentException if @p values has fewer than 4 elements.
     */
    Argb(const std::vector<T>& values) {
        if (values.size() < 4) throw System::ArgumentException("The span is too short to contain a color.", "values");
        A = values[0]; R = values[1]; G = values[2]; B = values[3];
    }

    /**
     * @brief Copies the four channels into @p destination (must have at least 4 elements).
     * @param destination Target vector; must have at least 4 elements.
     * @throws System::ArgumentException if @p destination has fewer than 4 elements.
     */
    void CopyTo(std::vector<T>& destination) const {
        if (destination.size() < 4) throw System::ArgumentException("The span is too short to contain a color.", "destination");
        destination[0] = A; destination[1] = R; destination[2] = G; destination[3] = B;
    }

    /** Returns true if all four channels of @p other equal this instance's channels. */
    bool Equals(const Argb<T>& other) const {
        return A == other.A && R == other.R && G == other.G && B == other.B;
    }
    /** Returns true if all channels are equal to @p other. */
    bool operator==(const Argb<T>& other) const { return Equals(other); }
    /** Returns true if any channel differs from @p other. */
    bool operator!=(const Argb<T>& other) const { return !Equals(other); }

    /** Default hash function for this color, combining all four channels. */
    SharpRuntime::intcs GetHashCode() const { return System::HashCode::Combine(A, R, G, B); }

    /** Returns a string representation in the form "<A, R, G, B>". */
    std::string ToString() const {
        std::ostringstream ss;
        ss.imbue(std::locale::classic());
        ss << "<" << +A << ", " << +R << ", " << +G << ", " << +B << ">";
        return ss.str();
    }

    /** Converts this Argb to the equivalent Rgba representation. */
    inline Rgba<T> ToRgba() const;

    /**
     * @brief Creates an Argb<byte> color from a big-endian uint32_t (0xAARRGGBB byte order).
     * C++ counterpart of .NET's static Argb.CreateBigEndian(uint).
     */
    static Argb<SharpRuntime::bytecs> CreateBigEndian(uint32_t color) {
        if (System::BitConverter::IsLittleEndian) {
            color = System::Buffers::Binary::BinaryPrimitives::ReverseEndianness(color);
        }
        return bitsToArgb(color);
    }

    /**
     * @brief Creates an Argb<byte> color from a little-endian uint32_t (0xBBGGRRAA byte order).
     * C++ counterpart of .NET's static Argb.CreateLittleEndian(uint).
     */
    static Argb<SharpRuntime::bytecs> CreateLittleEndian(uint32_t color) {
        if (!System::BitConverter::IsLittleEndian) {
            color = System::Buffers::Binary::BinaryPrimitives::ReverseEndianness(color);
        }
        return bitsToArgb(color);
    }

    /**
     * @brief Converts an Argb<byte> color to a big-endian uint32_t (0xAARRGGBB byte order).
     * C++ counterpart of .NET's static Argb.ToUInt32BigEndian(Argb<byte>).
     */
    static uint32_t ToUInt32BigEndian(const Argb<SharpRuntime::bytecs>& color) {
        uint32_t bits = argbToBits(color);
        return System::BitConverter::IsLittleEndian
                   ? System::Buffers::Binary::BinaryPrimitives::ReverseEndianness(bits)
                   : bits;
    }

    /**
     * @brief Converts an Argb<byte> color to a little-endian uint32_t (0xBBGGRRAA byte order).
     * C++ counterpart of .NET's static Argb.ToUInt32LittleEndian(Argb<byte>).
     */
    static uint32_t ToUInt32LittleEndian(const Argb<SharpRuntime::bytecs>& color) {
        uint32_t bits = argbToBits(color);
        return System::BitConverter::IsLittleEndian
                   ? bits
                   : System::Buffers::Binary::BinaryPrimitives::ReverseEndianness(bits);
    }

private:
    // Raw memory reinterpretation between a native uint32_t and the four A,R,G,B bytes, in
    // struct-declaration (address) order — matches .NET's Unsafe.BitCast<uint, Argb<byte>> over
    // a [StructLayout(Sequential)] A,R,G,B struct exactly (host-endian dependent by design; the
    // CreateBigEndian/CreateLittleEndian/ToUInt32*Endian wrappers apply ReverseEndianness around
    // this to normalize for the caller's requested byte order).
    static Argb<SharpRuntime::bytecs> bitsToArgb(uint32_t bits) {
        SharpRuntime::bytecs raw[4];
        std::memcpy(raw, &bits, 4);
        return Argb<SharpRuntime::bytecs>(raw[0], raw[1], raw[2], raw[3]);
    }
    static uint32_t argbToBits(const Argb<SharpRuntime::bytecs>& color) {
        SharpRuntime::bytecs raw[4] = {color.A, color.R, color.G, color.B};
        uint32_t bits;
        std::memcpy(&bits, raw, 4);
        return bits;
    }
};

/** Represents a color in RGBA (Red, Green, Blue, Alpha) order. */
template<typename T>
struct Rgba {
    T R{}; ///< Red channel.
    T G{}; ///< Green channel.
    T B{}; ///< Blue channel.
    T A{}; ///< Alpha channel.

    /** Default constructor — all channels zero-initialised. */
    Rgba() = default;
    /**
     * @brief Constructs an Rgba from individual channel values.
     * @param r Red value.
     * @param g Green value.
     * @param b Blue value.
     * @param a Alpha value.
     */
    Rgba(T r, T g, T b, T a) : R(r), G(g), B(b), A(a) {}
    /**
     * @brief Constructs an Rgba from a vector with at least 4 elements (R, G, B, A).
     * @param values Source vector; must have at least 4 elements.
     * @throws System::ArgumentException if @p values has fewer than 4 elements.
     */
    Rgba(const std::vector<T>& values) {
        if (values.size() < 4) throw System::ArgumentException("The span is too short to contain a color.", "values");
        R = values[0]; G = values[1]; B = values[2]; A = values[3];
    }

    /**
     * @brief Copies the four channels into @p destination (must have at least 4 elements).
     * @param destination Target vector; must have at least 4 elements.
     * @throws System::ArgumentException if @p destination has fewer than 4 elements.
     */
    void CopyTo(std::vector<T>& destination) const {
        if (destination.size() < 4) throw System::ArgumentException("The span is too short to contain a color.", "destination");
        destination[0] = R; destination[1] = G; destination[2] = B; destination[3] = A;
    }

    /** Returns true if all four channels of @p other equal this instance's channels. */
    bool Equals(const Rgba<T>& other) const {
        return R == other.R && G == other.G && B == other.B && A == other.A;
    }
    /** Returns true if all channels are equal to @p other. */
    bool operator==(const Rgba<T>& other) const { return Equals(other); }
    /** Returns true if any channel differs from @p other. */
    bool operator!=(const Rgba<T>& other) const { return !Equals(other); }

    /** Default hash function for this color, combining all four channels. */
    SharpRuntime::intcs GetHashCode() const { return System::HashCode::Combine(R, G, B, A); }

    /** Returns a string representation in the form "<R, G, B, A>". */
    std::string ToString() const {
        std::ostringstream ss;
        ss.imbue(std::locale::classic());
        ss << "<" << +R << ", " << +G << ", " << +B << ", " << +A << ">";
        return ss.str();
    }

    /** Converts this Rgba to the equivalent Argb representation. */
    Argb<T> ToArgb() const { return Argb<T>(A, R, G, B); }

    /**
     * @brief Creates an Rgba<byte> color from a big-endian uint32_t (0xRRGGBBAA byte order).
     * C++ counterpart of .NET's static Rgba.CreateBigEndian(uint).
     */
    static Rgba<SharpRuntime::bytecs> CreateBigEndian(uint32_t color) {
        if (System::BitConverter::IsLittleEndian) {
            color = System::Buffers::Binary::BinaryPrimitives::ReverseEndianness(color);
        }
        return bitsToRgba(color);
    }

    /**
     * @brief Creates an Rgba<byte> color from a little-endian uint32_t (0xAABBGGRR byte order).
     * C++ counterpart of .NET's static Rgba.CreateLittleEndian(uint).
     */
    static Rgba<SharpRuntime::bytecs> CreateLittleEndian(uint32_t color) {
        if (!System::BitConverter::IsLittleEndian) {
            color = System::Buffers::Binary::BinaryPrimitives::ReverseEndianness(color);
        }
        return bitsToRgba(color);
    }

    /**
     * @brief Converts an Rgba<byte> color to a big-endian uint32_t (0xRRGGBBAA byte order).
     * C++ counterpart of .NET's static Rgba.ToUInt32BigEndian(Rgba<byte>).
     */
    static uint32_t ToUInt32BigEndian(const Rgba<SharpRuntime::bytecs>& color) {
        uint32_t bits = rgbaToBits(color);
        return System::BitConverter::IsLittleEndian
                   ? System::Buffers::Binary::BinaryPrimitives::ReverseEndianness(bits)
                   : bits;
    }

    /**
     * @brief Converts an Rgba<byte> color to a little-endian uint32_t (0xAABBGGRR byte order).
     * C++ counterpart of .NET's static Rgba.ToUInt32LittleEndian(Rgba<byte>).
     */
    static uint32_t ToUInt32LittleEndian(const Rgba<SharpRuntime::bytecs>& color) {
        uint32_t bits = rgbaToBits(color);
        return System::BitConverter::IsLittleEndian
                   ? bits
                   : System::Buffers::Binary::BinaryPrimitives::ReverseEndianness(bits);
    }

private:
    // Raw memory reinterpretation between a native uint32_t and the four R,G,B,A bytes, in
    // struct-declaration (address) order — matches .NET's Unsafe.BitCast<uint, Rgba<byte>> over
    // a [StructLayout(Sequential)] R,G,B,A struct exactly (host-endian dependent by design; the
    // CreateBigEndian/CreateLittleEndian/ToUInt32*Endian wrappers apply ReverseEndianness around
    // this to normalize for the caller's requested byte order).
    static Rgba<SharpRuntime::bytecs> bitsToRgba(uint32_t bits) {
        SharpRuntime::bytecs raw[4];
        std::memcpy(raw, &bits, 4);
        return Rgba<SharpRuntime::bytecs>(raw[0], raw[1], raw[2], raw[3]);
    }
    static uint32_t rgbaToBits(const Rgba<SharpRuntime::bytecs>& color) {
        SharpRuntime::bytecs raw[4] = {color.R, color.G, color.B, color.A};
        uint32_t bits;
        std::memcpy(&bits, raw, 4);
        return bits;
    }
};

template<typename T>
Rgba<T> Argb<T>::ToRgba() const { return Rgba<T>(R, G, B, A); }

} // namespace System::Numerics::Colors

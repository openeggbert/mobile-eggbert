// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Span.hpp"
// System::Int128/UInt128 themselves #error unconditionally on MSVC (require GCC/Clang's
// __int128), so including them here must stay conditional -- otherwise this file, which is
// otherwise fully portable, would silently become MSVC-unsupported too just from adding the
// Int128/UInt128 overloads below.
#if !defined(_MSC_VER)
#  include "System/Int128.hpp"
#  include "System/UInt128.hpp"
#endif

namespace System::Buffers::Binary {

    using SharpRuntime::intcs;

    /**
     * @brief Reads and writes primitive values from and to byte spans in a specific byte order.
     *
     * C++ counterpart of .NET System.Buffers.Binary.BinaryPrimitives.
     * All methods are static. Little-endian and big-endian variants are provided for
     * int16, uint16, int32, uint32, int64, uint64, float, and double types, each with
     * both a throwing (Read/Write) and a non-throwing (TryRead/TryWrite) form.
     */
    class BinaryPrimitives {
    public:
        BinaryPrimitives() = delete;

        // -----------------------------------------------------------------------
        // Read — little-endian
        // -----------------------------------------------------------------------

        /** @brief Reads a little-endian int16 from the first 2 bytes of @p source. */
        [[nodiscard]] static int16_t ReadInt16LittleEndian(System::ReadOnlySpan<uint8_t> source) {
            checkSize(source.getLengthProperty(), 2, "source");
            int16_t v; std::memcpy(&v, source.getPointer(), 2); return fromLE16(v);
        }
        /** @brief Reads a little-endian uint16 from the first 2 bytes of @p source. */
        [[nodiscard]] static uint16_t ReadUInt16LittleEndian(System::ReadOnlySpan<uint8_t> source) {
            checkSize(source.getLengthProperty(), 2, "source");
            uint16_t v; std::memcpy(&v, source.getPointer(), 2); return fromLE16u(v);
        }
        /** @brief Reads a little-endian int32 from the first 4 bytes of @p source. */
        [[nodiscard]] static int32_t ReadInt32LittleEndian(System::ReadOnlySpan<uint8_t> source) {
            checkSize(source.getLengthProperty(), 4, "source");
            int32_t v; std::memcpy(&v, source.getPointer(), 4); return fromLE32(v);
        }
        /** @brief Reads a little-endian uint32 from the first 4 bytes of @p source. */
        [[nodiscard]] static uint32_t ReadUInt32LittleEndian(System::ReadOnlySpan<uint8_t> source) {
            checkSize(source.getLengthProperty(), 4, "source");
            uint32_t v; std::memcpy(&v, source.getPointer(), 4); return fromLE32u(v);
        }
        /** @brief Reads a little-endian int64 from the first 8 bytes of @p source. */
        [[nodiscard]] static int64_t ReadInt64LittleEndian(System::ReadOnlySpan<uint8_t> source) {
            checkSize(source.getLengthProperty(), 8, "source");
            int64_t v; std::memcpy(&v, source.getPointer(), 8); return fromLE64(v);
        }
        /** @brief Reads a little-endian uint64 from the first 8 bytes of @p source. */
        [[nodiscard]] static uint64_t ReadUInt64LittleEndian(System::ReadOnlySpan<uint8_t> source) {
            checkSize(source.getLengthProperty(), 8, "source");
            uint64_t v; std::memcpy(&v, source.getPointer(), 8); return fromLE64u(v);
        }
        /** @brief Reads a little-endian float from the first 4 bytes of @p source. */
        [[nodiscard]] static float ReadSingleLittleEndian(System::ReadOnlySpan<uint8_t> source) {
            uint32_t bits = ReadUInt32LittleEndian(source);
            float v; std::memcpy(&v, &bits, 4); return v;
        }
        /** @brief Reads a little-endian double from the first 8 bytes of @p source. */
        [[nodiscard]] static double ReadDoubleLittleEndian(System::ReadOnlySpan<uint8_t> source) {
            uint64_t bits = ReadUInt64LittleEndian(source);
            double v; std::memcpy(&v, &bits, 8); return v;
        }

        // -----------------------------------------------------------------------
        // Read — big-endian
        // -----------------------------------------------------------------------

        /** @brief Reads a big-endian int16 from the first 2 bytes of @p source. */
        [[nodiscard]] static int16_t ReadInt16BigEndian(System::ReadOnlySpan<uint8_t> source) {
            checkSize(source.getLengthProperty(), 2, "source");
            int16_t v; std::memcpy(&v, source.getPointer(), 2); return fromBE16(v);
        }
        /** @brief Reads a big-endian uint16 from the first 2 bytes of @p source. */
        [[nodiscard]] static uint16_t ReadUInt16BigEndian(System::ReadOnlySpan<uint8_t> source) {
            checkSize(source.getLengthProperty(), 2, "source");
            uint16_t v; std::memcpy(&v, source.getPointer(), 2); return fromBE16u(v);
        }
        /** @brief Reads a big-endian int32 from the first 4 bytes of @p source. */
        [[nodiscard]] static int32_t ReadInt32BigEndian(System::ReadOnlySpan<uint8_t> source) {
            checkSize(source.getLengthProperty(), 4, "source");
            int32_t v; std::memcpy(&v, source.getPointer(), 4); return fromBE32(v);
        }
        /** @brief Reads a big-endian uint32 from the first 4 bytes of @p source. */
        [[nodiscard]] static uint32_t ReadUInt32BigEndian(System::ReadOnlySpan<uint8_t> source) {
            checkSize(source.getLengthProperty(), 4, "source");
            uint32_t v; std::memcpy(&v, source.getPointer(), 4); return fromBE32u(v);
        }
        /** @brief Reads a big-endian int64 from the first 8 bytes of @p source. */
        [[nodiscard]] static int64_t ReadInt64BigEndian(System::ReadOnlySpan<uint8_t> source) {
            checkSize(source.getLengthProperty(), 8, "source");
            int64_t v; std::memcpy(&v, source.getPointer(), 8); return fromBE64(v);
        }
        /** @brief Reads a big-endian uint64 from the first 8 bytes of @p source. */
        [[nodiscard]] static uint64_t ReadUInt64BigEndian(System::ReadOnlySpan<uint8_t> source) {
            checkSize(source.getLengthProperty(), 8, "source");
            uint64_t v; std::memcpy(&v, source.getPointer(), 8); return fromBE64u(v);
        }
        /** @brief Reads a big-endian float from the first 4 bytes of @p source. */
        [[nodiscard]] static float ReadSingleBigEndian(System::ReadOnlySpan<uint8_t> source) {
            uint32_t bits = ReadUInt32BigEndian(source);
            float v; std::memcpy(&v, &bits, 4); return v;
        }
        /** @brief Reads a big-endian double from the first 8 bytes of @p source. */
        [[nodiscard]] static double ReadDoubleBigEndian(System::ReadOnlySpan<uint8_t> source) {
            uint64_t bits = ReadUInt64BigEndian(source);
            double v; std::memcpy(&v, &bits, 8); return v;
        }

        // -----------------------------------------------------------------------
        // TryRead — little-endian
        // -----------------------------------------------------------------------

        /** @brief Attempts to read a little-endian int16; returns false if @p source is too small. */
        [[nodiscard]] static bool TryReadInt16LittleEndian(System::ReadOnlySpan<uint8_t> source, int16_t& value) {
            if (source.getLengthProperty() < 2) { value = 0; return false; }
            std::memcpy(&value, source.getPointer(), 2); value = fromLE16(value); return true;
        }
        /** @brief Attempts to read a little-endian uint16; returns false if @p source is too small. */
        [[nodiscard]] static bool TryReadUInt16LittleEndian(System::ReadOnlySpan<uint8_t> source, uint16_t& value) {
            if (source.getLengthProperty() < 2) { value = 0; return false; }
            std::memcpy(&value, source.getPointer(), 2); value = fromLE16u(value); return true;
        }
        /** @brief Attempts to read a little-endian int32; returns false if @p source is too small. */
        [[nodiscard]] static bool TryReadInt32LittleEndian(System::ReadOnlySpan<uint8_t> source, int32_t& value) {
            if (source.getLengthProperty() < 4) { value = 0; return false; }
            std::memcpy(&value, source.getPointer(), 4); value = fromLE32(value); return true;
        }
        /** @brief Attempts to read a little-endian uint32; returns false if @p source is too small. */
        [[nodiscard]] static bool TryReadUInt32LittleEndian(System::ReadOnlySpan<uint8_t> source, uint32_t& value) {
            if (source.getLengthProperty() < 4) { value = 0; return false; }
            std::memcpy(&value, source.getPointer(), 4); value = fromLE32u(value); return true;
        }
        /** @brief Attempts to read a little-endian int64; returns false if @p source is too small. */
        [[nodiscard]] static bool TryReadInt64LittleEndian(System::ReadOnlySpan<uint8_t> source, int64_t& value) {
            if (source.getLengthProperty() < 8) { value = 0; return false; }
            std::memcpy(&value, source.getPointer(), 8); value = fromLE64(value); return true;
        }
        /** @brief Attempts to read a little-endian uint64; returns false if @p source is too small. */
        [[nodiscard]] static bool TryReadUInt64LittleEndian(System::ReadOnlySpan<uint8_t> source, uint64_t& value) {
            if (source.getLengthProperty() < 8) { value = 0; return false; }
            std::memcpy(&value, source.getPointer(), 8); value = fromLE64u(value); return true;
        }
        /** @brief Attempts to read a little-endian float; returns false if @p source is too small. */
        [[nodiscard]] static bool TryReadSingleLittleEndian(System::ReadOnlySpan<uint8_t> source, float& value) {
            uint32_t bits;
            if (!TryReadUInt32LittleEndian(source, bits)) { value = 0.0f; return false; }
            std::memcpy(&value, &bits, 4); return true;
        }
        /** @brief Attempts to read a little-endian double; returns false if @p source is too small. */
        [[nodiscard]] static bool TryReadDoubleLittleEndian(System::ReadOnlySpan<uint8_t> source, double& value) {
            uint64_t bits;
            if (!TryReadUInt64LittleEndian(source, bits)) { value = 0.0; return false; }
            std::memcpy(&value, &bits, 8); return true;
        }

        // -----------------------------------------------------------------------
        // TryRead — big-endian
        // -----------------------------------------------------------------------

        /** @brief Attempts to read a big-endian int16; returns false if @p source is too small. */
        [[nodiscard]] static bool TryReadInt16BigEndian(System::ReadOnlySpan<uint8_t> source, int16_t& value) {
            if (source.getLengthProperty() < 2) { value = 0; return false; }
            std::memcpy(&value, source.getPointer(), 2); value = fromBE16(value); return true;
        }
        /** @brief Attempts to read a big-endian uint16; returns false if @p source is too small. */
        [[nodiscard]] static bool TryReadUInt16BigEndian(System::ReadOnlySpan<uint8_t> source, uint16_t& value) {
            if (source.getLengthProperty() < 2) { value = 0; return false; }
            std::memcpy(&value, source.getPointer(), 2); value = fromBE16u(value); return true;
        }
        /** @brief Attempts to read a big-endian int32; returns false if @p source is too small. */
        [[nodiscard]] static bool TryReadInt32BigEndian(System::ReadOnlySpan<uint8_t> source, int32_t& value) {
            if (source.getLengthProperty() < 4) { value = 0; return false; }
            std::memcpy(&value, source.getPointer(), 4); value = fromBE32(value); return true;
        }
        /** @brief Attempts to read a big-endian uint32; returns false if @p source is too small. */
        [[nodiscard]] static bool TryReadUInt32BigEndian(System::ReadOnlySpan<uint8_t> source, uint32_t& value) {
            if (source.getLengthProperty() < 4) { value = 0; return false; }
            std::memcpy(&value, source.getPointer(), 4); value = fromBE32u(value); return true;
        }
        /** @brief Attempts to read a big-endian int64; returns false if @p source is too small. */
        [[nodiscard]] static bool TryReadInt64BigEndian(System::ReadOnlySpan<uint8_t> source, int64_t& value) {
            if (source.getLengthProperty() < 8) { value = 0; return false; }
            std::memcpy(&value, source.getPointer(), 8); value = fromBE64(value); return true;
        }
        /** @brief Attempts to read a big-endian uint64; returns false if @p source is too small. */
        [[nodiscard]] static bool TryReadUInt64BigEndian(System::ReadOnlySpan<uint8_t> source, uint64_t& value) {
            if (source.getLengthProperty() < 8) { value = 0; return false; }
            std::memcpy(&value, source.getPointer(), 8); value = fromBE64u(value); return true;
        }
        /** @brief Attempts to read a big-endian float; returns false if @p source is too small. */
        [[nodiscard]] static bool TryReadSingleBigEndian(System::ReadOnlySpan<uint8_t> source, float& value) {
            uint32_t bits;
            if (!TryReadUInt32BigEndian(source, bits)) { value = 0.0f; return false; }
            std::memcpy(&value, &bits, 4); return true;
        }
        /** @brief Attempts to read a big-endian double; returns false if @p source is too small. */
        [[nodiscard]] static bool TryReadDoubleBigEndian(System::ReadOnlySpan<uint8_t> source, double& value) {
            uint64_t bits;
            if (!TryReadUInt64BigEndian(source, bits)) { value = 0.0; return false; }
            std::memcpy(&value, &bits, 8); return true;
        }

        // -----------------------------------------------------------------------
        // Write — little-endian
        // -----------------------------------------------------------------------

        /** @brief Writes @p value as little-endian int16 into the first 2 bytes of @p destination. */
        static void WriteInt16LittleEndian(System::Span<uint8_t> destination, int16_t value) {
            checkSize(destination.getLengthProperty(), 2, "destination");
            int16_t v = toLE16(value); std::memcpy(destination.getPointer(), &v, 2);
        }
        /** @brief Writes @p value as little-endian uint16 into the first 2 bytes of @p destination. */
        static void WriteUInt16LittleEndian(System::Span<uint8_t> destination, uint16_t value) {
            checkSize(destination.getLengthProperty(), 2, "destination");
            uint16_t v = toLE16u(value); std::memcpy(destination.getPointer(), &v, 2);
        }
        /** @brief Writes @p value as little-endian int32 into the first 4 bytes of @p destination. */
        static void WriteInt32LittleEndian(System::Span<uint8_t> destination, int32_t value) {
            checkSize(destination.getLengthProperty(), 4, "destination");
            int32_t v = toLE32(value); std::memcpy(destination.getPointer(), &v, 4);
        }
        /** @brief Writes @p value as little-endian uint32 into the first 4 bytes of @p destination. */
        static void WriteUInt32LittleEndian(System::Span<uint8_t> destination, uint32_t value) {
            checkSize(destination.getLengthProperty(), 4, "destination");
            uint32_t v = toLE32u(value); std::memcpy(destination.getPointer(), &v, 4);
        }
        /** @brief Writes @p value as little-endian int64 into the first 8 bytes of @p destination. */
        static void WriteInt64LittleEndian(System::Span<uint8_t> destination, int64_t value) {
            checkSize(destination.getLengthProperty(), 8, "destination");
            int64_t v = toLE64(value); std::memcpy(destination.getPointer(), &v, 8);
        }
        /** @brief Writes @p value as little-endian uint64 into the first 8 bytes of @p destination. */
        static void WriteUInt64LittleEndian(System::Span<uint8_t> destination, uint64_t value) {
            checkSize(destination.getLengthProperty(), 8, "destination");
            uint64_t v = toLE64u(value); std::memcpy(destination.getPointer(), &v, 8);
        }
        /** @brief Writes @p value as little-endian float into the first 4 bytes of @p destination. */
        static void WriteSingleLittleEndian(System::Span<uint8_t> destination, float value) {
            uint32_t bits; std::memcpy(&bits, &value, 4);
            WriteUInt32LittleEndian(destination, bits);
        }
        /** @brief Writes @p value as little-endian double into the first 8 bytes of @p destination. */
        static void WriteDoubleLittleEndian(System::Span<uint8_t> destination, double value) {
            uint64_t bits; std::memcpy(&bits, &value, 8);
            WriteUInt64LittleEndian(destination, bits);
        }

        // -----------------------------------------------------------------------
        // Write — big-endian
        // -----------------------------------------------------------------------

        /** @brief Writes @p value as big-endian int16 into the first 2 bytes of @p destination. */
        static void WriteInt16BigEndian(System::Span<uint8_t> destination, int16_t value) {
            checkSize(destination.getLengthProperty(), 2, "destination");
            int16_t v = toBE16(value); std::memcpy(destination.getPointer(), &v, 2);
        }
        /** @brief Writes @p value as big-endian uint16 into the first 2 bytes of @p destination. */
        static void WriteUInt16BigEndian(System::Span<uint8_t> destination, uint16_t value) {
            checkSize(destination.getLengthProperty(), 2, "destination");
            uint16_t v = toBE16u(value); std::memcpy(destination.getPointer(), &v, 2);
        }
        /** @brief Writes @p value as big-endian int32 into the first 4 bytes of @p destination. */
        static void WriteInt32BigEndian(System::Span<uint8_t> destination, int32_t value) {
            checkSize(destination.getLengthProperty(), 4, "destination");
            int32_t v = toBE32(value); std::memcpy(destination.getPointer(), &v, 4);
        }
        /** @brief Writes @p value as big-endian uint32 into the first 4 bytes of @p destination. */
        static void WriteUInt32BigEndian(System::Span<uint8_t> destination, uint32_t value) {
            checkSize(destination.getLengthProperty(), 4, "destination");
            uint32_t v = toBE32u(value); std::memcpy(destination.getPointer(), &v, 4);
        }
        /** @brief Writes @p value as big-endian int64 into the first 8 bytes of @p destination. */
        static void WriteInt64BigEndian(System::Span<uint8_t> destination, int64_t value) {
            checkSize(destination.getLengthProperty(), 8, "destination");
            int64_t v = toBE64(value); std::memcpy(destination.getPointer(), &v, 8);
        }
        /** @brief Writes @p value as big-endian uint64 into the first 8 bytes of @p destination. */
        static void WriteUInt64BigEndian(System::Span<uint8_t> destination, uint64_t value) {
            checkSize(destination.getLengthProperty(), 8, "destination");
            uint64_t v = toBE64u(value); std::memcpy(destination.getPointer(), &v, 8);
        }
        /** @brief Writes @p value as big-endian float into the first 4 bytes of @p destination. */
        static void WriteSingleBigEndian(System::Span<uint8_t> destination, float value) {
            uint32_t bits; std::memcpy(&bits, &value, 4);
            WriteUInt32BigEndian(destination, bits);
        }
        /** @brief Writes @p value as big-endian double into the first 8 bytes of @p destination. */
        static void WriteDoubleBigEndian(System::Span<uint8_t> destination, double value) {
            uint64_t bits; std::memcpy(&bits, &value, 8);
            WriteUInt64BigEndian(destination, bits);
        }

        // -----------------------------------------------------------------------
        // TryWrite — little-endian
        // -----------------------------------------------------------------------

        /** @brief Attempts to write @p value as little-endian int16; returns false if @p destination is too small. */
        [[nodiscard]] static bool TryWriteInt16LittleEndian(System::Span<uint8_t> destination, int16_t value) {
            if (destination.getLengthProperty() < 2) return false;
            int16_t v = toLE16(value); std::memcpy(destination.getPointer(), &v, 2); return true;
        }
        /** @brief Attempts to write @p value as little-endian uint16; returns false if @p destination is too small. */
        [[nodiscard]] static bool TryWriteUInt16LittleEndian(System::Span<uint8_t> destination, uint16_t value) {
            if (destination.getLengthProperty() < 2) return false;
            uint16_t v = toLE16u(value); std::memcpy(destination.getPointer(), &v, 2); return true;
        }
        /** @brief Attempts to write @p value as little-endian int32; returns false if @p destination is too small. */
        [[nodiscard]] static bool TryWriteInt32LittleEndian(System::Span<uint8_t> destination, int32_t value) {
            if (destination.getLengthProperty() < 4) return false;
            int32_t v = toLE32(value); std::memcpy(destination.getPointer(), &v, 4); return true;
        }
        /** @brief Attempts to write @p value as little-endian uint32; returns false if @p destination is too small. */
        [[nodiscard]] static bool TryWriteUInt32LittleEndian(System::Span<uint8_t> destination, uint32_t value) {
            if (destination.getLengthProperty() < 4) return false;
            uint32_t v = toLE32u(value); std::memcpy(destination.getPointer(), &v, 4); return true;
        }
        /** @brief Attempts to write @p value as little-endian int64; returns false if @p destination is too small. */
        [[nodiscard]] static bool TryWriteInt64LittleEndian(System::Span<uint8_t> destination, int64_t value) {
            if (destination.getLengthProperty() < 8) return false;
            int64_t v = toLE64(value); std::memcpy(destination.getPointer(), &v, 8); return true;
        }
        /** @brief Attempts to write @p value as little-endian uint64; returns false if @p destination is too small. */
        [[nodiscard]] static bool TryWriteUInt64LittleEndian(System::Span<uint8_t> destination, uint64_t value) {
            if (destination.getLengthProperty() < 8) return false;
            uint64_t v = toLE64u(value); std::memcpy(destination.getPointer(), &v, 8); return true;
        }
        /** @brief Attempts to write @p value as little-endian float; returns false if @p destination is too small. */
        [[nodiscard]] static bool TryWriteSingleLittleEndian(System::Span<uint8_t> destination, float value) {
            uint32_t bits; std::memcpy(&bits, &value, 4);
            return TryWriteUInt32LittleEndian(destination, bits);
        }
        /** @brief Attempts to write @p value as little-endian double; returns false if @p destination is too small. */
        [[nodiscard]] static bool TryWriteDoubleLittleEndian(System::Span<uint8_t> destination, double value) {
            uint64_t bits; std::memcpy(&bits, &value, 8);
            return TryWriteUInt64LittleEndian(destination, bits);
        }

        // -----------------------------------------------------------------------
        // TryWrite — big-endian
        // -----------------------------------------------------------------------

        /** @brief Attempts to write @p value as big-endian int16; returns false if @p destination is too small. */
        [[nodiscard]] static bool TryWriteInt16BigEndian(System::Span<uint8_t> destination, int16_t value) {
            if (destination.getLengthProperty() < 2) return false;
            int16_t v = toBE16(value); std::memcpy(destination.getPointer(), &v, 2); return true;
        }
        /** @brief Attempts to write @p value as big-endian uint16; returns false if @p destination is too small. */
        [[nodiscard]] static bool TryWriteUInt16BigEndian(System::Span<uint8_t> destination, uint16_t value) {
            if (destination.getLengthProperty() < 2) return false;
            uint16_t v = toBE16u(value); std::memcpy(destination.getPointer(), &v, 2); return true;
        }
        /** @brief Attempts to write @p value as big-endian int32; returns false if @p destination is too small. */
        [[nodiscard]] static bool TryWriteInt32BigEndian(System::Span<uint8_t> destination, int32_t value) {
            if (destination.getLengthProperty() < 4) return false;
            int32_t v = toBE32(value); std::memcpy(destination.getPointer(), &v, 4); return true;
        }
        /** @brief Attempts to write @p value as big-endian uint32; returns false if @p destination is too small. */
        [[nodiscard]] static bool TryWriteUInt32BigEndian(System::Span<uint8_t> destination, uint32_t value) {
            if (destination.getLengthProperty() < 4) return false;
            uint32_t v = toBE32u(value); std::memcpy(destination.getPointer(), &v, 4); return true;
        }
        /** @brief Attempts to write @p value as big-endian int64; returns false if @p destination is too small. */
        [[nodiscard]] static bool TryWriteInt64BigEndian(System::Span<uint8_t> destination, int64_t value) {
            if (destination.getLengthProperty() < 8) return false;
            int64_t v = toBE64(value); std::memcpy(destination.getPointer(), &v, 8); return true;
        }
        /** @brief Attempts to write @p value as big-endian uint64; returns false if @p destination is too small. */
        [[nodiscard]] static bool TryWriteUInt64BigEndian(System::Span<uint8_t> destination, uint64_t value) {
            if (destination.getLengthProperty() < 8) return false;
            uint64_t v = toBE64u(value); std::memcpy(destination.getPointer(), &v, 8); return true;
        }
        /** @brief Attempts to write @p value as big-endian float; returns false if @p destination is too small. */
        [[nodiscard]] static bool TryWriteSingleBigEndian(System::Span<uint8_t> destination, float value) {
            uint32_t bits; std::memcpy(&bits, &value, 4);
            return TryWriteUInt32BigEndian(destination, bits);
        }
        /** @brief Attempts to write @p value as big-endian double; returns false if @p destination is too small. */
        [[nodiscard]] static bool TryWriteDoubleBigEndian(System::Span<uint8_t> destination, double value) {
            uint64_t bits; std::memcpy(&bits, &value, 8);
            return TryWriteUInt64BigEndian(destination, bits);
        }

#if !defined(_MSC_VER)
        // -----------------------------------------------------------------------
        // Int128 / UInt128 (LE/BE, split into two 64-bit halves via System::Int128/UInt128's
        // getUpperProperty()/getLowerProperty() and (upper,lower) constructor -- both types were
        // missing from this file entirely even though they're fully ported elsewhere; real .NET
        // has had Read/Write/TryRead/TryWriteInt128/UInt128*Endian and ReverseEndianness(Int128/
        // UInt128) since .NET 7. For LE, the low 64 bits occupy the first 8 bytes and the high 64
        // bits the last 8 (matches MemoryMarshal.Read<Int128> on a little-endian host, which is
        // what real .NET's own implementation reduces to); BE reverses both which half comes
        // first AND each half's own byte order -- verified against BinaryPrimitives.Read*Endian.cs.
        // ReverseEndianness(Half)/(BFloat16) don't exist in real .NET either (both are always
        // stored/read as a raw 2-byte pattern via the existing 16-bit primitives, no endian-swap
        // concept beyond that), so Half/BFloat16 support isn't added here -- IntPtr/UIntPtr
        // (nint/nuint) are also missing, deliberately deferred: their width is platform-dependent
        // (4 or 8 bytes), which needs a scope decision on which native width this port targets,
        // not a mechanical add like Int128/UInt128 was.
        // -----------------------------------------------------------------------

        /** @brief Reads a little-endian Int128 from the first 16 bytes of @p source. */
        [[nodiscard]] static System::Int128 ReadInt128LittleEndian(System::ReadOnlySpan<uint8_t> source) {
            checkSize(source.getLengthProperty(), 16, "source");
            uint64_t lower = ReadUInt64LittleEndian(source.Slice(0, 8));
            uint64_t upper = ReadUInt64LittleEndian(source.Slice(8, 8));
            return System::Int128(upper, lower);
        }
        /** @brief Reads a little-endian UInt128 from the first 16 bytes of @p source. */
        [[nodiscard]] static System::UInt128 ReadUInt128LittleEndian(System::ReadOnlySpan<uint8_t> source) {
            checkSize(source.getLengthProperty(), 16, "source");
            uint64_t lower = ReadUInt64LittleEndian(source.Slice(0, 8));
            uint64_t upper = ReadUInt64LittleEndian(source.Slice(8, 8));
            return System::UInt128(upper, lower);
        }
        /** @brief Reads a big-endian Int128 from the first 16 bytes of @p source. */
        [[nodiscard]] static System::Int128 ReadInt128BigEndian(System::ReadOnlySpan<uint8_t> source) {
            checkSize(source.getLengthProperty(), 16, "source");
            uint64_t upper = ReadUInt64BigEndian(source.Slice(0, 8));
            uint64_t lower = ReadUInt64BigEndian(source.Slice(8, 8));
            return System::Int128(upper, lower);
        }
        /** @brief Reads a big-endian UInt128 from the first 16 bytes of @p source. */
        [[nodiscard]] static System::UInt128 ReadUInt128BigEndian(System::ReadOnlySpan<uint8_t> source) {
            checkSize(source.getLengthProperty(), 16, "source");
            uint64_t upper = ReadUInt64BigEndian(source.Slice(0, 8));
            uint64_t lower = ReadUInt64BigEndian(source.Slice(8, 8));
            return System::UInt128(upper, lower);
        }

        /** @brief Attempts to read a little-endian Int128; returns false if @p source is too small. */
        [[nodiscard]] static bool TryReadInt128LittleEndian(System::ReadOnlySpan<uint8_t> source, System::Int128& value) {
            if (source.getLengthProperty() < 16) { value = System::Int128(); return false; }
            value = ReadInt128LittleEndian(source);
            return true;
        }
        /** @brief Attempts to read a little-endian UInt128; returns false if @p source is too small. */
        [[nodiscard]] static bool TryReadUInt128LittleEndian(System::ReadOnlySpan<uint8_t> source, System::UInt128& value) {
            if (source.getLengthProperty() < 16) { value = System::UInt128(); return false; }
            value = ReadUInt128LittleEndian(source);
            return true;
        }
        /** @brief Attempts to read a big-endian Int128; returns false if @p source is too small. */
        [[nodiscard]] static bool TryReadInt128BigEndian(System::ReadOnlySpan<uint8_t> source, System::Int128& value) {
            if (source.getLengthProperty() < 16) { value = System::Int128(); return false; }
            value = ReadInt128BigEndian(source);
            return true;
        }
        /** @brief Attempts to read a big-endian UInt128; returns false if @p source is too small. */
        [[nodiscard]] static bool TryReadUInt128BigEndian(System::ReadOnlySpan<uint8_t> source, System::UInt128& value) {
            if (source.getLengthProperty() < 16) { value = System::UInt128(); return false; }
            value = ReadUInt128BigEndian(source);
            return true;
        }

        /** @brief Writes @p value as little-endian Int128 into the first 16 bytes of @p destination. */
        static void WriteInt128LittleEndian(System::Span<uint8_t> destination, System::Int128 value) {
            checkSize(destination.getLengthProperty(), 16, "destination");
            WriteUInt64LittleEndian(destination.Slice(0, 8), value.getLowerProperty());
            WriteUInt64LittleEndian(destination.Slice(8, 8), value.getUpperProperty());
        }
        /** @brief Writes @p value as little-endian UInt128 into the first 16 bytes of @p destination. */
        static void WriteUInt128LittleEndian(System::Span<uint8_t> destination, System::UInt128 value) {
            checkSize(destination.getLengthProperty(), 16, "destination");
            WriteUInt64LittleEndian(destination.Slice(0, 8), value.getLowerProperty());
            WriteUInt64LittleEndian(destination.Slice(8, 8), value.getUpperProperty());
        }
        /** @brief Writes @p value as big-endian Int128 into the first 16 bytes of @p destination. */
        static void WriteInt128BigEndian(System::Span<uint8_t> destination, System::Int128 value) {
            checkSize(destination.getLengthProperty(), 16, "destination");
            WriteUInt64BigEndian(destination.Slice(0, 8), value.getUpperProperty());
            WriteUInt64BigEndian(destination.Slice(8, 8), value.getLowerProperty());
        }
        /** @brief Writes @p value as big-endian UInt128 into the first 16 bytes of @p destination. */
        static void WriteUInt128BigEndian(System::Span<uint8_t> destination, System::UInt128 value) {
            checkSize(destination.getLengthProperty(), 16, "destination");
            WriteUInt64BigEndian(destination.Slice(0, 8), value.getUpperProperty());
            WriteUInt64BigEndian(destination.Slice(8, 8), value.getLowerProperty());
        }

        /** @brief Attempts to write @p value as little-endian Int128; returns false if @p destination is too small. */
        [[nodiscard]] static bool TryWriteInt128LittleEndian(System::Span<uint8_t> destination, System::Int128 value) {
            if (destination.getLengthProperty() < 16) return false;
            WriteInt128LittleEndian(destination, value);
            return true;
        }
        /** @brief Attempts to write @p value as little-endian UInt128; returns false if @p destination is too small. */
        [[nodiscard]] static bool TryWriteUInt128LittleEndian(System::Span<uint8_t> destination, System::UInt128 value) {
            if (destination.getLengthProperty() < 16) return false;
            WriteUInt128LittleEndian(destination, value);
            return true;
        }
        /** @brief Attempts to write @p value as big-endian Int128; returns false if @p destination is too small. */
        [[nodiscard]] static bool TryWriteInt128BigEndian(System::Span<uint8_t> destination, System::Int128 value) {
            if (destination.getLengthProperty() < 16) return false;
            WriteInt128BigEndian(destination, value);
            return true;
        }
        /** @brief Attempts to write @p value as big-endian UInt128; returns false if @p destination is too small. */
        [[nodiscard]] static bool TryWriteUInt128BigEndian(System::Span<uint8_t> destination, System::UInt128 value) {
            if (destination.getLengthProperty() < 16) return false;
            WriteUInt128BigEndian(destination, value);
            return true;
        }
#endif // !defined(_MSC_VER)

        // -----------------------------------------------------------------------
        // ReverseEndianness
        // -----------------------------------------------------------------------

        /** @brief Returns @p value with its bytes in reversed order (no-op for sbyte). */
        static int8_t   ReverseEndianness(int8_t value)   { return value; }
        /** @brief Returns @p value with its bytes in reversed order (no-op for byte). */
        static uint8_t  ReverseEndianness(uint8_t value)  { return value; }
        /** @brief Returns @p value with its bytes reversed. */
        static int16_t  ReverseEndianness(int16_t value)  {
            uint16_t u; std::memcpy(&u, &value, 2); u = bswap16(u);
            int16_t r; std::memcpy(&r, &u, 2); return r;
        }
        /** @brief Returns @p value with its bytes reversed. */
        static uint16_t ReverseEndianness(uint16_t value) { return bswap16(value); }
        /** @brief Returns @p value with its bytes reversed. */
        static int32_t  ReverseEndianness(int32_t value)  {
            uint32_t u; std::memcpy(&u, &value, 4); u = bswap32(u);
            int32_t r; std::memcpy(&r, &u, 4); return r;
        }
        /** @brief Returns @p value with its bytes reversed. */
        static uint32_t ReverseEndianness(uint32_t value) { return bswap32(value); }
        /** @brief Returns @p value with its bytes reversed. */
        static int64_t  ReverseEndianness(int64_t value)  {
            uint64_t u; std::memcpy(&u, &value, 8); u = bswap64(u);
            int64_t r; std::memcpy(&r, &u, 8); return r;
        }
        /** @brief Returns @p value with its bytes reversed. */
        static uint64_t ReverseEndianness(uint64_t value) { return bswap64(value); }
#if !defined(_MSC_VER)
        /** @brief Returns @p value with its bytes reversed (the two 64-bit halves swap AND each reverses). */
        static System::Int128 ReverseEndianness(System::Int128 value) {
            return System::Int128(bswap64(value.getLowerProperty()), bswap64(value.getUpperProperty()));
        }
        /** @brief Returns @p value with its bytes reversed (the two 64-bit halves swap AND each reverses). */
        static System::UInt128 ReverseEndianness(System::UInt128 value) {
            return System::UInt128(bswap64(value.getLowerProperty()), bswap64(value.getUpperProperty()));
        }
#endif // !defined(_MSC_VER)

    private:
        static void checkSize(intcs len, intcs needed, const char* paramName) {
            if (len < needed) throw System::ArgumentOutOfRangeException(paramName);
        }

        // Little-endian helpers (no-op on LE architectures; byte-swap on BE)
        static int16_t  fromLE16(int16_t v)   { return leToHost16s(v); }
        static uint16_t fromLE16u(uint16_t v) { return leToHost16u(v); }
        static int32_t  fromLE32(int32_t v)   { return leToHost32s(v); }
        static uint32_t fromLE32u(uint32_t v) { return leToHost32u(v); }
        static int64_t  fromLE64(int64_t v)   { return leToHost64s(v); }
        static uint64_t fromLE64u(uint64_t v) { return leToHost64u(v); }

        static int16_t  toLE16(int16_t v)    { return leToHost16s(v); }
        static uint16_t toLE16u(uint16_t v)  { return leToHost16u(v); }
        static int32_t  toLE32(int32_t v)    { return leToHost32s(v); }
        static uint32_t toLE32u(uint32_t v)  { return leToHost32u(v); }
        static int64_t  toLE64(int64_t v)    { return leToHost64s(v); }
        static uint64_t toLE64u(uint64_t v)  { return leToHost64u(v); }

        // Big-endian helpers
        static int16_t  fromBE16(int16_t v)   { return beToHost16s(v); }
        static uint16_t fromBE16u(uint16_t v) { return beToHost16u(v); }
        static int32_t  fromBE32(int32_t v)   { return beToHost32s(v); }
        static uint32_t fromBE32u(uint32_t v) { return beToHost32u(v); }
        static int64_t  fromBE64(int64_t v)   { return beToHost64s(v); }
        static uint64_t fromBE64u(uint64_t v) { return beToHost64u(v); }

        static int16_t  toBE16(int16_t v)   { return beToHost16s(v); }
        static uint16_t toBE16u(uint16_t v) { return beToHost16u(v); }
        static int32_t  toBE32(int32_t v)   { return beToHost32s(v); }
        static uint32_t toBE32u(uint32_t v) { return beToHost32u(v); }
        static int64_t  toBE64(int64_t v)   { return beToHost64s(v); }
        static uint64_t toBE64u(uint64_t v) { return beToHost64u(v); }

        // Portable byte-swap
        static uint16_t bswap16(uint16_t v) {
            return static_cast<uint16_t>((v >> 8) | (v << 8));
        }
        static uint32_t bswap32(uint32_t v) {
            return ((v & 0xFF000000u) >> 24) | ((v & 0x00FF0000u) >> 8)
                 | ((v & 0x0000FF00u) << 8)  | ((v & 0x000000FFu) << 24);
        }
        static uint64_t bswap64(uint64_t v) {
            return ((uint64_t)bswap32(static_cast<uint32_t>(v)) << 32)
                 |  (uint64_t)bswap32(static_cast<uint32_t>(v >> 32));
        }

#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        static int16_t  leToHost16s(int16_t v)  { uint16_t u; std::memcpy(&u,&v,2); u=bswap16(u); int16_t r; std::memcpy(&r,&u,2); return r; }
        static uint16_t leToHost16u(uint16_t v) { return bswap16(v); }
        static int32_t  leToHost32s(int32_t v)  { uint32_t u; std::memcpy(&u,&v,4); u=bswap32(u); int32_t r; std::memcpy(&r,&u,4); return r; }
        static uint32_t leToHost32u(uint32_t v) { return bswap32(v); }
        static int64_t  leToHost64s(int64_t v)  { uint64_t u; std::memcpy(&u,&v,8); u=bswap64(u); int64_t r; std::memcpy(&r,&u,8); return r; }
        static uint64_t leToHost64u(uint64_t v) { return bswap64(v); }

        static int16_t  beToHost16s(int16_t v)  { return v; }
        static uint16_t beToHost16u(uint16_t v) { return v; }
        static int32_t  beToHost32s(int32_t v)  { return v; }
        static uint32_t beToHost32u(uint32_t v) { return v; }
        static int64_t  beToHost64s(int64_t v)  { return v; }
        static uint64_t beToHost64u(uint64_t v) { return v; }
#else
        // Little-endian (x86/x64/ARM default) — memcpy is already correct
        static int16_t  leToHost16s(int16_t v)  { return v; }
        static uint16_t leToHost16u(uint16_t v) { return v; }
        static int32_t  leToHost32s(int32_t v)  { return v; }
        static uint32_t leToHost32u(uint32_t v) { return v; }
        static int64_t  leToHost64s(int64_t v)  { return v; }
        static uint64_t leToHost64u(uint64_t v) { return v; }

        static int16_t  beToHost16s(int16_t v)  { uint16_t u; std::memcpy(&u,&v,2); u=bswap16(u); int16_t r; std::memcpy(&r,&u,2); return r; }
        static uint16_t beToHost16u(uint16_t v) { return bswap16(v); }
        static int32_t  beToHost32s(int32_t v)  { uint32_t u; std::memcpy(&u,&v,4); u=bswap32(u); int32_t r; std::memcpy(&r,&u,4); return r; }
        static uint32_t beToHost32u(uint32_t v) { return bswap32(v); }
        static int64_t  beToHost64s(int64_t v)  { uint64_t u; std::memcpy(&u,&v,8); u=bswap64(u); int64_t r; std::memcpy(&r,&u,8); return r; }
        static uint64_t beToHost64u(uint64_t v) { return bswap64(v); }
#endif
    };

} // namespace System::Buffers::Binary

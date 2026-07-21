// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/IO/FileAccess.hpp"

namespace System::IO {

    using SharpRuntime::bytecs;
    using SharpRuntime::sbytecs;
    using SharpRuntime::shortcs;
    using SharpRuntime::ushortcs;
    using SharpRuntime::intcs;
    using SharpRuntime::uintcs;
    using SharpRuntime::longcs;
    using SharpRuntime::ulongcs;

    /**
     * @brief Provides random access to unmanaged blocks of memory from managed code.
     *
     * Lightweight C++ counterpart of .NET System.IO.UnmanagedMemoryAccessor: wraps a raw,
     * caller-owned byte buffer (in place of .NET's SafeBuffer) and lets primitive values be
     * read/written at arbitrary offsets. Generic Read<T>/Write<T>/ReadArray/WriteArray and
     * ReadDecimal/Write(decimal) are not implemented — they rely on .NET's blittable-struct
     * marshalling, which has no C++ equivalent here.
     *
     * @note Status: PARTIAL
     */
    class UnmanagedMemoryAccessor {
    private:
        bytecs* buffer_   = nullptr;
        longcs  capacity_ = 0;
        bool    canRead_  = false;
        bool    canWrite_ = false;
        bool    isOpen_   = false;

        void EnsureReadable(longcs position, longcs size) const;
        void EnsureWritable(longcs position, longcs size) const;

    protected:
        /** Initializes an accessor with no backing buffer; derived types must call Initialize(). */
        UnmanagedMemoryAccessor() = default;

        /** Initializes the accessor over @p buffer with the given capacity and access. */
        void Initialize(bytecs* buffer, longcs capacity, FileAccess access);

        /** Returns true if the accessor has not been disposed. */
        [[nodiscard]] bool getIsOpenProperty() const { return isOpen_; }

    public:
        /** Constructs an accessor over @p buffer with Read access, matching .NET's 3-arg (buffer, offset, capacity) constructor default. */
        UnmanagedMemoryAccessor(bytecs* buffer, longcs capacity);
        /** Constructs an accessor over @p buffer with the specified access. */
        UnmanagedMemoryAccessor(bytecs* buffer, longcs capacity, FileAccess access);

        virtual ~UnmanagedMemoryAccessor() = default;

        /** Returns the capacity of the accessor, in bytes. */
        [[nodiscard]] longcs getCapacityProperty() const { return capacity_; }
        /** Returns true if the accessor supports reading. */
        [[nodiscard]] bool getCanReadProperty() const { return isOpen_ && canRead_; }
        /** Returns true if the accessor supports writing. */
        [[nodiscard]] bool getCanWriteProperty() const { return isOpen_ && canWrite_; }

        /** Marks the accessor as closed; further reads/writes throw ObjectDisposedException. */
        void Dispose() { isOpen_ = false; }

        /** Reads a bool at the given byte position. */
        [[nodiscard]] bool     ReadBoolean(longcs position) const { return ReadByte(position) != 0; }
        /** Reads a byte at the given byte position. */
        [[nodiscard]] bytecs   ReadByte(longcs position) const;
        /** Reads a UTF-16 char (2 bytes) at the given byte position. */
        [[nodiscard]] char16_t ReadChar(longcs position) const;
        /** Reads a 16-bit signed integer at the given byte position. */
        [[nodiscard]] shortcs  ReadInt16(longcs position) const;
        /** Reads a 32-bit signed integer at the given byte position. */
        [[nodiscard]] intcs    ReadInt32(longcs position) const;
        /** Reads a 64-bit signed integer at the given byte position. */
        [[nodiscard]] longcs   ReadInt64(longcs position) const;
        /** Reads a single-precision float at the given byte position. */
        [[nodiscard]] float    ReadSingle(longcs position) const;
        /** Reads a double-precision float at the given byte position. */
        [[nodiscard]] double   ReadDouble(longcs position) const;
        /** Reads a signed byte at the given byte position. */
        [[nodiscard]] sbytecs  ReadSByte(longcs position) const { return static_cast<sbytecs>(ReadByte(position)); }
        /** Reads an unsigned 16-bit integer at the given byte position. */
        [[nodiscard]] ushortcs ReadUInt16(longcs position) const { return static_cast<ushortcs>(ReadInt16(position)); }
        /** Reads an unsigned 32-bit integer at the given byte position. */
        [[nodiscard]] uintcs   ReadUInt32(longcs position) const { return static_cast<uintcs>(ReadInt32(position)); }
        /** Reads an unsigned 64-bit integer at the given byte position. */
        [[nodiscard]] ulongcs  ReadUInt64(longcs position) const { return static_cast<ulongcs>(ReadInt64(position)); }

        /** Writes a bool at the given byte position. */
        void Write(longcs position, bool value)     { Write(position, static_cast<bytecs>(value ? 1 : 0)); }
        /** Writes a byte at the given byte position. */
        void Write(longcs position, bytecs value);
        /** Writes a UTF-16 char (2 bytes) at the given byte position. */
        void Write(longcs position, char16_t value) { Write(position, static_cast<shortcs>(value)); }
        /** Writes a 16-bit signed integer at the given byte position. */
        void Write(longcs position, shortcs value);
        /** Writes a 32-bit signed integer at the given byte position. */
        void Write(longcs position, intcs value);
        /** Writes a 64-bit signed integer at the given byte position. */
        void Write(longcs position, longcs value);
        /** Writes a single-precision float at the given byte position. */
        void Write(longcs position, float value);
        /** Writes a double-precision float at the given byte position. */
        void Write(longcs position, double value);
        /** Writes a signed byte at the given byte position. */
        void Write(longcs position, sbytecs value)  { Write(position, static_cast<bytecs>(value)); }
        /** Writes an unsigned 16-bit integer at the given byte position. */
        void Write(longcs position, ushortcs value) { Write(position, static_cast<shortcs>(value)); }
        /** Writes an unsigned 32-bit integer at the given byte position. */
        void Write(longcs position, uintcs value)   { Write(position, static_cast<intcs>(value)); }
        /** Writes an unsigned 64-bit integer at the given byte position. */
        void Write(longcs position, ulongcs value)  { Write(position, static_cast<longcs>(value)); }
    };

} // namespace System::IO

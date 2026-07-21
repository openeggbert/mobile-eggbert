// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Memory.hpp"
#include "System/Span.hpp"

namespace System::Buffers {

    using SharpRuntime::intcs;

    /**
     * @brief Represents an output sink into which T data can be written.
     *
     * C++ counterpart of .NET System.Buffers.IBufferWriter&lt;T&gt;.
     * Implementers provide a mutable Span&lt;T&gt; or Memory&lt;T&gt; to be filled,
     * then call Advance() to record how many elements were actually written.
     *
     * @tparam T The type of the items in the IBufferWriter.
     */
    template<typename T>
    class IBufferWriter {
    public:
        /** @brief Virtual destructor for safe polymorphic destruction. */
        virtual ~IBufferWriter() = default;

        /**
         * @brief Notifies the writer that @p count elements have been written to the output.
         * @param count The number of elements written.
         */
        virtual void Advance(intcs count) = 0;

        /**
         * @brief Returns a Memory&lt;T&gt; with at least @p sizeHint elements of available space.
         * @param sizeHint Minimum number of elements desired (0 means no preference).
         * @return A writable Memory covering the available output buffer.
         */
        virtual System::Memory<T> GetMemory(intcs sizeHint = 0) = 0;

        /**
         * @brief Returns a Span&lt;T&gt; with at least @p sizeHint elements of available space.
         * @param sizeHint Minimum number of elements desired (0 means no preference).
         * @return A writable Span covering the available output buffer.
         */
        virtual System::Span<T> GetSpan(intcs sizeHint = 0) = 0;
    };

} // namespace System::Buffers

// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <stdexcept>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/NotSupportedException.hpp"
#include "System/Span.hpp"
#include "System/Buffers/IMemoryOwner.hpp"
#include "System/Buffers/IPinnable.hpp"

namespace System::Buffers {

    using SharpRuntime::intcs;

/**
 * @brief Abstract base for custom Memory&lt;T&gt; managers.
 *
 * C++ counterpart of .NET System.Buffers.MemoryManager&lt;T&gt;.
 * Implements both IMemoryOwner&lt;T&gt; and IPinnable. Concrete subclasses must
 * provide GetSpan(), Pin(), and Unpin().
 *
 * @note In real .NET, Memory&lt;T&gt;.Memory / CreateMemory() construct a
 * Memory&lt;T&gt; backed directly by this manager (Memory&lt;T&gt;(MemoryManager&lt;T&gt;, int)),
 * so writes through the returned Memory&lt;T&gt; observe the manager's live
 * storage. This port's System::Memory&lt;T&gt; only supports std::vector-backed
 * storage, so a manager-backed Memory&lt;T&gt; cannot be constructed here;
 * getMemoryProperty()/CreateMemory() throw NotSupportedException. Use GetSpan()
 * to access the underlying memory instead.
 *
 * @tparam T The element type.
 */
template<typename T>
class MemoryManager : public IMemoryOwner<T>, public IPinnable {
public:
    virtual ~MemoryManager() = default;

    /**
     * @brief Returns a Span&lt;T&gt; wrapping the underlying memory region.
     *
     * C++ counterpart of .NET MemoryManager&lt;T&gt;.GetSpan().
     */
    virtual System::Span<T> GetSpan() = 0;

    /**
     * @brief Pins the memory at the given element offset and returns a handle.
     *
     * C++ counterpart of .NET MemoryManager&lt;T&gt;.Pin(int elementIndex = 0).
     */
    MemoryHandle Pin(intcs elementIndex = 0) override = 0;

    /**
     * @brief Unpins the memory, allowing the GC to move it.
     *
     * C++ counterpart of .NET MemoryManager&lt;T&gt;.Unpin().
     */
    void Unpin() override = 0;

    /**
     * @brief Implements IDisposable. Default no-op; override to release resources.
     *
     * C++ counterpart of .NET MemoryManager&lt;T&gt;.Dispose(bool).
     */
    void Dispose() override {}

    /**
     * @brief Returns a Memory&lt;T&gt; wrapping the underlying memory region.
     *
     * C++ counterpart of .NET MemoryManager&lt;T&gt;.Memory (which returns
     * `new Memory<T>(this, GetSpan().Length)`).
     * @throws System::NotSupportedException always — see the class-level note.
     */
    System::Memory<T> getMemoryProperty() override {
        throw NotSupportedException(
            "MemoryManager<T>.Memory requires a manager-backed Memory<T>, which "
            "this port's Memory<T> does not support. Use GetSpan() instead.");
    }

protected:
    /**
     * @brief Returns a Memory&lt;T&gt; of the given length for this manager.
     * C++ counterpart of .NET MemoryManager&lt;T&gt;.CreateMemory(int).
     * @throws System::NotSupportedException always — see the class-level note.
     */
    System::Memory<T> CreateMemory(intcs /*length*/) {
        throw NotSupportedException(
            "MemoryManager<T>.CreateMemory requires a manager-backed Memory<T>, "
            "which this port's Memory<T> does not support. Use GetSpan() instead.");
    }

    /**
     * @brief Returns a Memory&lt;T&gt; over [start, start+length) for this manager.
     * C++ counterpart of .NET MemoryManager&lt;T&gt;.CreateMemory(int, int).
     * @throws System::NotSupportedException always — see the class-level note.
     */
    System::Memory<T> CreateMemory(intcs /*start*/, intcs /*length*/) {
        throw NotSupportedException(
            "MemoryManager<T>.CreateMemory requires a manager-backed Memory<T>, "
            "which this port's Memory<T> does not support. Use GetSpan() instead.");
    }
};

} // namespace System::Buffers

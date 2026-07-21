// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>
#include <vector>
#include "System/IDisposable.hpp"
#include "System/Memory.hpp"

namespace System::Buffers {

    /** Identifies an owner of a memory buffer, providing access to and lifetime management for the buffer. */
    template<typename T>
    class IMemoryOwner : public System::IDisposable {
    public:
        /** Destroys the memory owner and releases associated resources. */
        virtual ~IMemoryOwner() = default;
        /**
         * @brief Returns the memory buffer owned by this instance.
         * C++ counterpart of .NET IMemoryOwner&lt;T&gt;.Memory.
         */
        virtual System::Memory<T> getMemoryProperty() = 0;
    };

} // namespace System::Buffers

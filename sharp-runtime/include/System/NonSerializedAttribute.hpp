// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Attribute.hpp"

namespace System {
    /**
     * @brief Indicates that a field of a serializable class should not be serialized.
     *
     * C++ counterpart of .NET System.NonSerializedAttribute.
     *
     * @note Status: STUB (permanent deviation) — see SerializableAttribute.hpp's
     *   doc-comment for the full rationale; the same "no reflection, no BinaryFormatter, so
     *   this has no observable effect" reasoning applies here.
     */
    class NonSerializedAttribute : public Attribute {};
} // namespace System

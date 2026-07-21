// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Attribute.hpp"

namespace System {
    /**
     * @brief Indicates that a class can be serialized.
     *
     * C++ counterpart of .NET System.SerializableAttribute.
     *
     * @note Status: STUB (permanent deviation — see CLAUDE.md's "Known permanent
     *   deviations"). In .NET this attribute is discovered via reflection by the (now
     *   obsolete) BinaryFormatter and related legacy serializers; since sharp-runtime has no
     *   reflection (System::Type cannot enumerate a class's attributes) and no
     *   BinaryFormatter, applying this attribute to a ported type has no observable effect
     *   at all. It exists purely so ported C# code with a `[Serializable]` class attribute
     *   compiles without modification, not because anything in this port inspects it.
     */
    class SerializableAttribute : public Attribute {};
} // namespace System

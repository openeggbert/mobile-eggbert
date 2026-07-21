// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>

namespace System::Runtime::Serialization {

    /**
     * @brief Stores all the data needed to serialize or deserialize an object.
     *
     * Minimal C++ stub of .NET System.Runtime.Serialization.SerializationInfo.
     *
     * @note Status: STUB (permanent deviation — see CLAUDE.md's "Known permanent
     *   deviations": `[Serializable]`/SerializationInfo are ignored, .NET's legacy binary
     *   serialization infrastructure is not implemented, and is not needed for game code).
     *   No type in sharp-runtime currently takes a SerializationInfo parameter — in
     *   particular, exception classes do *not* have the .NET
     *   `Exception(SerializationInfo, StreamingContext)` protected constructor, since .NET
     *   itself has marked that pattern obsolete since .NET 8 (SYSLIB0051) and it is not
     *   reachable without a working BinaryFormatter. This type exists purely so that ported
     *   C# code referencing the SerializationInfo *type* (e.g. in an unused method
     *   signature) still compiles, not because anything in this port constructs or reads one.
     */
    class SerializationInfo {
    public:
        /** @brief Default constructor. */
        SerializationInfo() = default;
        virtual ~SerializationInfo() = default;
    };

} // namespace System::Runtime::Serialization

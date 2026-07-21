// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Runtime::Serialization {

    /**
     * @brief Describes the source and destination of a serialized stream.
     *
     * Minimal C++ stub of .NET System.Runtime.Serialization.StreamingContext.
     *
     * @note Status: STUB (permanent deviation — see SerializationInfo.hpp's doc-comment,
     *   which this type is always used alongside, for the full rationale). No type in
     *   sharp-runtime currently takes a StreamingContext parameter; it exists purely for
     *   source-level compatibility with ported C# code that references the type.
     */
    struct StreamingContext {
    };

} // namespace System::Runtime::Serialization

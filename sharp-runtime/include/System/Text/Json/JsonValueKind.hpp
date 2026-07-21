// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Text::Json {

    /** Specifies the type of a JSON value. */
    enum class JsonValueKind {
        /** No value has been set. */
        Undefined = 0,
        /** A JSON object (key-value pairs). */
        Object    = 1,
        /** A JSON array. */
        Array     = 2,
        /** A JSON string. */
        String    = 3,
        /** A JSON number. */
        Number    = 4,
        /** The JSON literal true. */
        True      = 5,
        /** The JSON literal false. */
        False     = 6,
        /** The JSON literal null. */
        Null      = 7,
    };

} // namespace System::Text::Json

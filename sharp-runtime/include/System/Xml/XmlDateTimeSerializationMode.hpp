// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Xml {

    /**
     * @brief Specifies how DateTime values are serialized/deserialized by XmlConvert.
     *
     * C++ counterpart of .NET System.Xml.XmlDateTimeSerializationMode.
     */
    enum class XmlDateTimeSerializationMode {
        /** Treat as local time / convert to local time. */
        Local = 0,
        /** Treat as UTC / convert to UTC. */
        Utc = 1,
        /** No time-zone information is included. */
        Unspecified = 2,
        /** Preserve the original DateTimeKind information (round-trip). */
        RoundtripKind = 3,
    };

} // namespace System::Xml

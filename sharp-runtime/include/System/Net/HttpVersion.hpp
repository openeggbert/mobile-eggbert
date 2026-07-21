// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Version.hpp"

namespace System::Net {

    /**
     * @brief Defines the version numbers that are supported for the HTTP protocol.
     *
     * C++ counterpart of .NET System.Net.HttpVersion.
     */
    class HttpVersion {
    public:
        HttpVersion() = delete;

        /** Indicates an unknown version of HTTP. */
        static const System::Version Unknown;
        /** HTTP 1.0. */
        static const System::Version Version10;
        /** HTTP 1.1. */
        static const System::Version Version11;
        /** HTTP 2.0. */
        static const System::Version Version20;
        /** HTTP 3.0. */
        static const System::Version Version30;
    };

    inline const System::Version HttpVersion::Unknown   {0, 0};
    inline const System::Version HttpVersion::Version10 {1, 0};
    inline const System::Version HttpVersion::Version11 {1, 1};
    inline const System::Version HttpVersion::Version20 {2, 0};
    inline const System::Version HttpVersion::Version30 {3, 0};

} // namespace System::Net

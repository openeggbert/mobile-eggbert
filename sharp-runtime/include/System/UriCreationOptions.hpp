// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System {

    /**
     * @brief Options for URI creation to control canonicalization behavior.
     *
     * C++ counterpart of .NET System.UriCreationOptions (introduced in .NET 6).
     */
    struct UriCreationOptions {
        /**
         * @brief When true, disables path and query canonicalization.
         *
         * Equivalent of .NET UriCreationOptions.DangerousDisablePathAndQueryCanonicalization.
         * Not enforced in this stub — present for API compatibility only.
         */
        bool DangerousDisablePathAndQueryCanonicalization = false;
    };

} // namespace System

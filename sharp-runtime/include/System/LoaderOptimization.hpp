// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System {

    /**
     * @brief Specifies how the loader optimizes the loading of an executable.
     *
     * C++ counterpart of .NET System.LoaderOptimization.
     * Note: multi-domain and domain-mask values are legacy; modern .NET uses a
     * single AppDomain.
     */
    enum class LoaderOptimization {
        /** @brief No optimization is specified. */
        NotSpecified    = 0,
        /** @brief The application will probably have a single domain. */
        SingleDomain    = 1,
        /** @brief The application will probably have multiple domains using the same code. */
        MultiDomain     = 2,
        /**
         * @brief The application will probably have multiple domains using different code.
         *
         * C++ counterpart of .NET LoaderOptimization.MultiDomainHost.
         */
        MultiDomainHost = 3,
        /**
         * @brief Deprecated alias for MultiDomainHost.
         *
         * C++ counterpart of .NET LoaderOptimization.DomainMask (deprecated).
         * @deprecated Use MultiDomainHost instead.
         */
        DomainMask      = 3,
        /**
         * @brief Disallow binding redirects.
         *
         * C++ counterpart of .NET LoaderOptimization.DisallowBindings (deprecated).
         * @deprecated This value is not supported.
         */
        DisallowBindings = 4,
    };

} // namespace System

// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#pragma once

#include <filesystem>

namespace SharpRuntime::Storage
{
    /**
     * @brief Provides filesystem paths for persistent storage used by the application.
     *
     * @note Status: IMPLEMENTED
     */
    class StoragePaths
    {
    public:
        StoragePaths() = delete;
        ~StoragePaths() = delete;

        /**
         * @brief Gets the root directory for isolated storage.
         *
         * The directory is created if it does not already exist.
         *
         * @return Filesystem path to isolated storage root.
         *
         * @note Status: IMPLEMENTED
         */
        [[nodiscard]] static std::filesystem::path GetIsolatedStorageRoot();
    };
}
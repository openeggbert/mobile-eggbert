// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "SharpRuntime/Storage/StoragePaths.hpp"

#include <filesystem>

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#endif

#if defined(__ANDROID__)
#include <SDL3/SDL.h>
#endif

namespace SharpRuntime::Storage
{
    std::filesystem::path StoragePaths::GetIsolatedStorageRoot()
    {
#if defined(__EMSCRIPTEN__)
        // On Emscripten, persist save data under /save which is mounted as
        // IDBFS by the application startup code so data survives page reloads.
        const std::filesystem::path root = std::filesystem::path("/save") / ".cna_isolated_storage";
#elif defined(__ANDROID__)
        // On Android the working directory is not writable.
        // Use SDL_GetPrefPath to obtain the app's private internal storage.
        // SDL_GetPrefPath returns a path like /data/data/<package>/files/<org>/<app>/
        // which persists across app restarts but is cleared on uninstall.
        char* prefPath = SDL_GetPrefPath("org.openeggbert", "speedyblupi");
        std::filesystem::path root;
        if (prefPath) {
            root = std::filesystem::path(prefPath) / ".cna_isolated_storage";
            SDL_free(prefPath);
        } else {
            // Fallback: use the Android internal storage path directly
            const char* internalPath = SDL_GetAndroidInternalStoragePath();
            if (internalPath) {
                root = std::filesystem::path(internalPath) / ".cna_isolated_storage";
            } else {
                root = std::filesystem::path("/data/local/tmp") / ".cna_isolated_storage";
            }
        }
#else
        const std::filesystem::path root = std::filesystem::current_path() / ".cna_isolated_storage";
#endif
        std::filesystem::create_directories(root);
        return root;
    }
}

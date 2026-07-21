// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <algorithm>
#include <cctype>
#include <memory>
#include <string>
#include "System/PlatformID.hpp"
#include "System/Version.hpp"

namespace System {

    /**
     * @brief Represents information about an operating system, such as the platform and version.
     *
     * C++ counterpart of .NET System.OperatingSystem.
     *
     * The static IsXxx() predicate methods are resolved at compile time from preprocessor macros.
     * The IsXxxVersionAtLeast() methods return true when running on the matching platform
     * (actual version detection is not implemented; the assumption is that the build host
     * is recent enough).
     */
    class OperatingSystem {
        PlatformID  platform_;
        Version     version_;
        std::string servicePack_;

        static std::string toLower(std::string s) {
            std::transform(s.begin(), s.end(), s.begin(),
                [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
            return s;
        }

    public:
        /**
         * @brief Constructs an OperatingSystem with the specified platform identifier and version.
         *
         * C++ counterpart of .NET OperatingSystem(PlatformID, Version).
         * @param platform Platform identifier (e.g. PlatformID::Unix).
         * @param version  OS version.
         */
        OperatingSystem(PlatformID platform, const Version& version)
            : platform_(platform), version_(version) {}

        // ---- Properties ----

        /**
         * @brief Gets the platform identifier for this operating system.
         *
         * C++ counterpart of .NET OperatingSystem.Platform.
         */
        [[nodiscard]] PlatformID         getPlatformProperty()    const { return platform_; }

        /**
         * @brief Gets the version of this operating system.
         *
         * C++ counterpart of .NET OperatingSystem.Version.
         */
        [[nodiscard]] const Version&     getVersionProperty()     const { return version_; }

        /**
         * @brief Gets the service pack string (always empty in this port).
         *
         * C++ counterpart of .NET OperatingSystem.ServicePack.
         */
        [[nodiscard]] const std::string& getServicePackProperty() const { return servicePack_; }

        /**
         * @brief Gets a human-readable version string such as "Unix 5.15.0".
         *
         * C++ counterpart of .NET OperatingSystem.VersionString.
         */
        [[nodiscard]] std::string getVersionStringProperty() const {
            // Real .NET's VersionString switches on every PlatformID value (OperatingSystem.cs)
            // -- this previously only handled Win32NT/Unix/MacOSX, falling through to "Unknown "
            // for the rest. PlatformID::Other in particular is not a dead/unused case here:
            // Environment::getOSVersionProperty() constructs it for Emscripten builds, so this
            // was a real, reachable divergence ("Unknown 0.0" instead of real .NET's "Other
            // 0.0") for that platform, not just a theoretical gap for the legacy Win32S/
            // Win32Windows/WinCE/Xbox values.
            std::string name;
            switch (platform_) {
                case PlatformID::Win32S:       name = "Microsoft Win32S "; break;
                case PlatformID::Win32Windows:
                    name = (version_.Major > 4 || (version_.Major == 4 && version_.Minor > 0))
                        ? "Microsoft Windows 98 " : "Microsoft Windows 95 ";
                    break;
                case PlatformID::Win32NT:      name = "Microsoft Windows NT "; break;
                case PlatformID::WinCE:        name = "Microsoft Windows CE "; break;
                case PlatformID::Unix:         name = "Unix "; break;
                case PlatformID::Xbox:         name = "Xbox "; break;
                case PlatformID::MacOSX:       name = "Mac OS X "; break;
                case PlatformID::Other:        name = "Other "; break;
                default:                       name = "<unknown> "; break;
            }
            return name + version_.ToString();
        }

        // ---- Instance methods ----

        /**
         * @brief Returns a copy of this OperatingSystem instance.
         *
         * C++ counterpart of .NET OperatingSystem.Clone().
         */
        [[nodiscard]] std::shared_ptr<OperatingSystem> Clone() const {
            return std::make_shared<OperatingSystem>(platform_, version_);
        }

        /**
         * @brief Returns the same string as getVersionStringProperty().
         *
         * C++ counterpart of .NET OperatingSystem.ToString().
         */
        [[nodiscard]] std::string ToString() const { return getVersionStringProperty(); }

        // ---- Static platform predicates ----

        /**
         * @brief Returns true when compiled for Windows (_WIN32 or _WIN64 defined).
         *
         * C++ counterpart of .NET OperatingSystem.IsWindows().
         */
        static bool IsWindows() {
#if defined(_WIN32) || defined(_WIN64)
            return true;
#else
            return false;
#endif
        }

        /**
         * @brief Returns true when compiled for Linux (__linux__ defined, excluding Android).
         *
         * C++ counterpart of .NET OperatingSystem.IsLinux(). Android also defines __linux__
         * (its kernel is Linux-based), but real .NET treats Android as a distinct OSPlatform
         * from generic Linux, so it's excluded here — mutually exclusive with IsAndroid(),
         * matching IsMacOS()'s existing exclusion of iOS via __IPHONE_OS_VERSION_MIN_REQUIRED.
         */
        static bool IsLinux() {
#if defined(__linux__) && !defined(__ANDROID__)
            return true;
#else
            return false;
#endif
        }

        /**
         * @brief Returns true when compiled for macOS (__APPLE__ and !__IPHONE_OS_VERSION_MIN_REQUIRED defined).
         *
         * C++ counterpart of .NET OperatingSystem.IsMacOS().
         */
        static bool IsMacOS() {
#if defined(__APPLE__) && !defined(__IPHONE_OS_VERSION_MIN_REQUIRED)
            return true;
#else
            return false;
#endif
        }

        /**
         * @brief Returns true when compiled for FreeBSD (__FreeBSD__ defined).
         *
         * C++ counterpart of .NET OperatingSystem.IsFreeBSD().
         */
        static bool IsFreeBSD() {
#if defined(__FreeBSD__)
            return true;
#else
            return false;
#endif
        }

        /**
         * @brief Returns true when compiled for WebAssembly via Emscripten (__EMSCRIPTEN__ defined).
         *
         * C++ counterpart of .NET OperatingSystem.IsBrowser().
         */
        static bool IsBrowser() {
#if defined(__EMSCRIPTEN__)
            return true;
#else
            return false;
#endif
        }

        /**
         * @brief Returns false — WASI is not supported in this port.
         *
         * C++ counterpart of .NET OperatingSystem.IsWasi().
         */
        static bool IsWasi() { return false; }

        /**
         * @brief Returns true when compiled for Android (__ANDROID__ defined).
         *
         * C++ counterpart of .NET OperatingSystem.IsAndroid(). Was previously hardcoded
         * false, in direct contradiction to SharpRuntime/Storage/StoragePaths.cpp's own
         * real __ANDROID__-guarded code path — this port does have Android-specific
         * handling elsewhere, this predicate just never reported it.
         */
        static bool IsAndroid() {
#if defined(__ANDROID__)
            return true;
#else
            return false;
#endif
        }

        /**
         * @brief Returns true when compiled for iOS (__APPLE__ and __IPHONE_OS_VERSION_MIN_REQUIRED defined).
         *
         * C++ counterpart of .NET OperatingSystem.IsIOS(). Uses the same
         * __IPHONE_OS_VERSION_MIN_REQUIRED convention IsMacOS() already relies on to
         * exclude iOS from itself.
         */
        static bool IsIOS() {
#if defined(__APPLE__) && defined(__IPHONE_OS_VERSION_MIN_REQUIRED)
            return true;
#else
            return false;
#endif
        }

        /**
         * @brief Returns false — tvOS is not supported in this port.
         *
         * C++ counterpart of .NET OperatingSystem.IsTvOS().
         */
        static bool IsTvOS() { return false; }

        /**
         * @brief Returns false — watchOS is not supported in this port.
         *
         * C++ counterpart of .NET OperatingSystem.IsWatchOS().
         */
        static bool IsWatchOS() { return false; }

        /**
         * @brief Returns false — Mac Catalyst is not supported in this port.
         *
         * C++ counterpart of .NET OperatingSystem.IsMacCatalyst().
         */
        static bool IsMacCatalyst() { return false; }

        /**
         * @brief Indicates whether the current OS platform matches the specified name.
         *
         * C++ counterpart of .NET OperatingSystem.IsOSPlatform(string).
         * Comparison is case-insensitive. Recognised values: "windows", "linux", "macos",
         * "freebsd", "browser", "android", "ios", "tvos", "watchos", "maccatalyst", "wasi".
         * @param platform The platform name to check.
         * @return true if the current OS matches the given name.
         */
        static bool IsOSPlatform(const std::string& platform) {
            const std::string p = toLower(platform);
            if (p == "windows")     return IsWindows();
            if (p == "linux")       return IsLinux();
            if (p == "macos")       return IsMacOS();
            if (p == "freebsd")     return IsFreeBSD();
            if (p == "browser")     return IsBrowser();
            if (p == "android")     return IsAndroid();
            if (p == "ios")         return IsIOS();
            if (p == "tvos")        return IsTvOS();
            if (p == "watchos")     return IsWatchOS();
            if (p == "maccatalyst") return IsMacCatalyst();
            if (p == "wasi")        return IsWasi();
            return false;
        }

        // ---- Version-at-least predicates ----
        // These return true when running on the matching platform.
        // Actual OS version detection is not implemented; the platform presence
        // is used as a proxy (suitable for game-dev porting where the host is always recent).

        /**
         * @brief Returns true if running on Windows.
         *
         * C++ counterpart of .NET OperatingSystem.IsWindowsVersionAtLeast(int, int, int, int).
         * Version comparison is not implemented; returns IsWindows().
         */
        static bool IsWindowsVersionAtLeast(SharpRuntime::intcs /*major*/, SharpRuntime::intcs /*minor*/ = 0,
                                             SharpRuntime::intcs /*build*/ = 0, SharpRuntime::intcs /*revision*/ = 0) {
            return IsWindows();
        }

        /**
         * @brief Returns true if running on macOS.
         *
         * C++ counterpart of .NET OperatingSystem.IsMacOSVersionAtLeast(int, int, int).
         * Version comparison is not implemented; returns IsMacOS().
         */
        static bool IsMacOSVersionAtLeast(SharpRuntime::intcs /*major*/, SharpRuntime::intcs /*minor*/ = 0, SharpRuntime::intcs /*build*/ = 0) {
            return IsMacOS();
        }

        /**
         * @brief Returns true if running on FreeBSD.
         *
         * C++ counterpart of .NET OperatingSystem.IsFreeBSDVersionAtLeast(int, int, int, int).
         * Version comparison is not implemented; returns IsFreeBSD().
         */
        static bool IsFreeBSDVersionAtLeast(SharpRuntime::intcs /*major*/, SharpRuntime::intcs /*minor*/ = 0,
                                             SharpRuntime::intcs /*build*/ = 0, SharpRuntime::intcs /*revision*/ = 0) {
            return IsFreeBSD();
        }

        /**
         * @brief Always returns false — Android is not supported in this port.
         *
         * C++ counterpart of .NET OperatingSystem.IsAndroidVersionAtLeast(int, int, int, int).
         */
        static bool IsAndroidVersionAtLeast(SharpRuntime::intcs /*major*/, SharpRuntime::intcs /*minor*/ = 0,
                                             SharpRuntime::intcs /*build*/ = 0, SharpRuntime::intcs /*revision*/ = 0) {
            return false;
        }

        /**
         * @brief Always returns false — iOS is not supported in this port.
         *
         * C++ counterpart of .NET OperatingSystem.IsIOSVersionAtLeast(int, int, int).
         */
        static bool IsIOSVersionAtLeast(SharpRuntime::intcs /*major*/, SharpRuntime::intcs /*minor*/ = 0, SharpRuntime::intcs /*build*/ = 0) {
            return false;
        }

        /**
         * @brief Always returns false — tvOS is not supported in this port.
         *
         * C++ counterpart of .NET OperatingSystem.IsTvOSVersionAtLeast(int, int, int).
         */
        static bool IsTvOSVersionAtLeast(SharpRuntime::intcs /*major*/, SharpRuntime::intcs /*minor*/ = 0, SharpRuntime::intcs /*build*/ = 0) {
            return false;
        }

        /**
         * @brief Always returns false — watchOS is not supported in this port.
         *
         * C++ counterpart of .NET OperatingSystem.IsWatchOSVersionAtLeast(int, int, int).
         */
        static bool IsWatchOSVersionAtLeast(SharpRuntime::intcs /*major*/, SharpRuntime::intcs /*minor*/ = 0, SharpRuntime::intcs /*build*/ = 0) {
            return false;
        }

        /**
         * @brief Always returns false — Mac Catalyst is not supported in this port.
         *
         * C++ counterpart of .NET OperatingSystem.IsMacCatalystVersionAtLeast(int, int, int).
         */
        static bool IsMacCatalystVersionAtLeast(SharpRuntime::intcs /*major*/, SharpRuntime::intcs /*minor*/ = 0,
                                                 SharpRuntime::intcs /*build*/ = 0) {
            return false;
        }

        /**
         * @brief Returns true if the current OS platform matches the name and is assumed to meet
         * the specified version requirement.
         *
         * C++ counterpart of .NET OperatingSystem.IsOSPlatformVersionAtLeast(string, int, int, int, int).
         * Version comparison is not implemented; delegates to IsOSPlatform(platform).
         * @param platform The platform name (case-insensitive).
         */
        static bool IsOSPlatformVersionAtLeast(const std::string& platform,
                                               SharpRuntime::intcs /*major*/, SharpRuntime::intcs /*minor*/ = 0,
                                               SharpRuntime::intcs /*build*/ = 0, SharpRuntime::intcs /*revision*/ = 0) {
            return IsOSPlatform(platform);
        }
    };

} // namespace System

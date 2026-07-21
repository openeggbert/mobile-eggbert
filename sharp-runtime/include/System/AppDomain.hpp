// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <functional>
#include <string>
#include "System/EventArgs.hpp"
#include "System/MarshalByRefObject.hpp"
#include "System/UnhandledExceptionEventHandler.hpp"
#include "System/Environment.hpp"

namespace System {

    /**
     * @brief Represents an application domain — the isolated execution environment
     * for an application. This class cannot be inherited.
     *
     * C++ counterpart of .NET System.AppDomain (sealed).
     * Only the singleton CurrentDomain() is supported. Implements the subset of
     * the .NET AppDomain API needed for game-engine porting: friendly name,
     * base directory, identity properties, and event/data stubs.
     *
     * Assembly loading, reflection, and code-access security are not implemented.
     */
    class AppDomain final : public MarshalByRefObject {
        std::string friendlyName_  = "DefaultDomain";
        std::string baseDirectory_;

        AppDomain();

    public:
        // -----------------------------------------------------------------------
        // Singleton access
        // -----------------------------------------------------------------------

        /**
         * @brief Gets the current application domain for the current thread.
         *
         * C++ counterpart of .NET AppDomain.CurrentDomain.
         * Returns the single process-wide AppDomain (there is no multi-domain
         * support in this port).
         */
        static AppDomain& CurrentDomain() {
            static AppDomain domain;
            return domain;
        }

        // -----------------------------------------------------------------------
        // Properties
        // -----------------------------------------------------------------------

        /**
         * @brief Gets the friendly name of this application domain.
         *
         * C++ counterpart of .NET AppDomain.FriendlyName.
         * Always returns @c "DefaultDomain" in this port.
         */
        [[nodiscard]] const std::string& getFriendlyNameProperty() const {
            return friendlyName_;
        }

        /**
         * @brief Gets the base directory that the assembly resolver uses to probe
         * for assemblies.
         *
         * C++ counterpart of .NET AppDomain.BaseDirectory.
         * The directory containing the executable, with a trailing '/': on Linux (and other
         * POSIX platforms besides macOS), resolved from /proc/self/exe; on macOS, from
         * _NSGetExecutablePath; on Windows, from GetModuleFileNameW. Falls back to "./" if
         * detection fails, and always "./" on Emscripten (no real executable path to resolve
         * -- runs against the virtual filesystem root).
         */
        [[nodiscard]] const std::string& getBaseDirectoryProperty() const {
            return baseDirectory_;
        }

        /**
         * @brief Gets the path under the base directory where the assembly resolver
         * probes for private assemblies.
         *
         * C++ counterpart of .NET AppDomain.RelativeSearchPath.
         * Always returns an empty string in this port (no dynamic probing).
         */
        [[nodiscard]] std::string getRelativeSearchPathProperty() const { return {}; }

        /**
         * @brief Gets the directory that the assembly resolver uses to probe for
         * dynamically created assemblies.
         *
         * C++ counterpart of .NET AppDomain.DynamicDirectory.
         * Always returns an empty string in this port.
         */
        [[nodiscard]] std::string getDynamicDirectoryProperty() const { return {}; }

        /**
         * @brief Gets a unique integer identifier for this application domain.
         *
         * C++ counterpart of .NET AppDomain.Id.
         * Always returns 1 (there is only one domain in this port).
         */
        [[nodiscard]] int getIdProperty() const noexcept { return 1; }

        /**
         * @brief Gets a value indicating whether the current application domain is
         * fully trusted.
         *
         * C++ counterpart of .NET AppDomain.IsFullyTrusted.
         * Always returns true in this port.
         */
        [[nodiscard]] bool getIsFullyTrustedProperty() const noexcept { return true; }

        /**
         * @brief Gets a value indicating whether the current application domain is
         * homogeneous.
         *
         * C++ counterpart of .NET AppDomain.IsHomogenous.
         * Always returns true in this port.
         */
        [[nodiscard]] bool getIsHomogenousProperty() const noexcept { return true; }

        // -----------------------------------------------------------------------
        // Methods
        // -----------------------------------------------------------------------

        /**
         * @brief Returns a value indicating whether this is the default application
         * domain for the current process.
         *
         * C++ counterpart of .NET AppDomain.IsDefaultAppDomain().
         * Always returns true in this port.
         */
        [[nodiscard]] bool IsDefaultAppDomain() const noexcept { return true; }

        /**
         * @brief Returns a value indicating whether this application domain is
         * being unloaded.
         *
         * C++ counterpart of .NET AppDomain.IsFinalizingForUnload().
         * Always returns false in this port (domains cannot be unloaded).
         */
        [[nodiscard]] bool IsFinalizingForUnload() const noexcept { return false; }

        /**
         * @brief Returns the policy-mapped assembly display name for a given
         * assembly name.
         *
         * C++ counterpart of .NET AppDomain.ApplyPolicy(string).
         * Returns @p assemblyName unchanged (no policy engine in this port).
         * @param assemblyName The assembly display name to map.
         * @return @p assemblyName unchanged.
         */
        [[nodiscard]] std::string ApplyPolicy(const std::string& assemblyName) const {
            return assemblyName;
        }

        /**
         * @brief Returns a string representation of this application domain.
         *
         * C++ counterpart of .NET AppDomain.ToString().
         */
        [[nodiscard]] std::string ToString() const {
            return "Name:" + friendlyName_;
        }

        // -----------------------------------------------------------------------
        // Data store (stubs)
        // -----------------------------------------------------------------------

        /**
         * @brief Assigns a value to the named data element of the current domain.
         *
         * C++ counterpart of .NET AppDomain.SetData(string, object).
         * Stub — stores nothing; provided for API compatibility.
         * @param name The name of the data element.
         * @param data A pointer to the value (ignored).
         */
        void SetData(const std::string& /*name*/, void* /*data*/) {}

        /**
         * @brief Gets the value stored in the current domain under the given name.
         *
         * C++ counterpart of .NET AppDomain.GetData(string).
         * Stub — always returns nullptr; provided for API compatibility.
         * @param name The name of the data element.
         * @return Always nullptr in this port.
         */
        void* GetData(const std::string& /*name*/) { return nullptr; }

        // -----------------------------------------------------------------------
        // Events (stubs)
        // -----------------------------------------------------------------------

        /**
         * @brief Stub — event registration is not functional; provided for API
         * compatibility.
         *
         * C++ counterpart of .NET AppDomain.UnhandledException event add accessor.
         */
        void add_UnhandledException(const UnhandledExceptionEventHandler& /*handler*/) {}

        /**
         * @brief Stub — event registration is not functional; provided for API
         * compatibility.
         *
         * C++ counterpart of .NET AppDomain.UnhandledException event remove accessor.
         */
        void remove_UnhandledException(const UnhandledExceptionEventHandler& /*handler*/) {}

        /**
         * @brief Stub — event registration is not functional; provided for API
         * compatibility.
         *
         * C++ counterpart of .NET AppDomain.ProcessExit event add accessor.
         */
        void add_ProcessExit(const std::function<void(void*, EventArgs&)>& /*handler*/) {}

        /**
         * @brief Stub — event registration is not functional; provided for API
         * compatibility.
         *
         * C++ counterpart of .NET AppDomain.ProcessExit event remove accessor.
         */
        void remove_ProcessExit(const std::function<void(void*, EventArgs&)>& /*handler*/) {}

        /**
         * @brief Stub — event registration is not functional; provided for API
         * compatibility.
         *
         * C++ counterpart of .NET AppDomain.DomainUnload event add accessor.
         */
        void add_DomainUnload(const std::function<void(void*, EventArgs&)>& /*handler*/) {}

        /**
         * @brief Stub — event registration is not functional; provided for API
         * compatibility.
         *
         * C++ counterpart of .NET AppDomain.DomainUnload event remove accessor.
         */
        void remove_DomainUnload(const std::function<void(void*, EventArgs&)>& /*handler*/) {}

        // -----------------------------------------------------------------------
        // Additional properties
        // -----------------------------------------------------------------------

        /**
         * @brief Gets a value indicating whether shadow copying of files is enabled.
         *
         * C++ counterpart of .NET AppDomain.ShadowCopyFiles.
         * Always returns false in this port.
         */
        [[nodiscard]] bool getShadowCopyFilesProperty() const noexcept { return false; }

        /**
         * @brief Determines whether a compatibility switch is set.
         *
         * C++ counterpart of .NET AppDomain.IsCompatibilitySwitchSet(string).
         * Always returns false in this port (no switch registry).
         * @param value The name of the compatibility switch.
         */
        [[nodiscard]] bool IsCompatibilitySwitchSet(const std::string& /*value*/) const noexcept {
            return false;
        }

        // -----------------------------------------------------------------------
        // Additional static methods
        // -----------------------------------------------------------------------

        /**
         * @brief Gets an integer that uniquely identifies the current managed thread.
         *
         * C++ counterpart of .NET AppDomain.GetCurrentThreadId().
         * @deprecated In .NET this method is deprecated; prefer Thread.ManagedThreadId.
         *             Delegates to Environment::getCurrentManagedThreadIdProperty().
         */
        [[nodiscard]] static SharpRuntime::intcs GetCurrentThreadId() {
            return Environment::getCurrentManagedThreadIdProperty();
        }

        // -----------------------------------------------------------------------
        // Deprecated no-op stub methods
        // -----------------------------------------------------------------------

        /**
         * @brief No-op stub.
         *
         * C++ counterpart of .NET AppDomain.SetDynamicBase(string) [Obsolete].
         */
        void SetDynamicBase(const std::string& /*path*/) {}

        /**
         * @brief No-op stub.
         *
         * C++ counterpart of .NET AppDomain.AppendPrivatePath(string) [Obsolete].
         */
        void AppendPrivatePath(const std::string& /*path*/) {}

        /**
         * @brief No-op stub.
         *
         * C++ counterpart of .NET AppDomain.ClearPrivatePath() [Obsolete].
         */
        void ClearPrivatePath() {}

        /**
         * @brief No-op stub.
         *
         * C++ counterpart of .NET AppDomain.ClearShadowCopyPath() [Obsolete].
         */
        void ClearShadowCopyPath() {}

        /**
         * @brief No-op stub.
         *
         * C++ counterpart of .NET AppDomain.SetShadowCopyFiles() [Obsolete].
         */
        void SetShadowCopyFiles() {}

        /**
         * @brief No-op stub.
         *
         * C++ counterpart of .NET AppDomain.SetShadowCopyPath(string) [Obsolete].
         */
        void SetShadowCopyPath(const std::string& /*path*/) {}

        /**
         * @brief No-op stub.
         *
         * C++ counterpart of .NET AppDomain.SetCachePath(string) [Obsolete].
         */
        void SetCachePath(const std::string& /*path*/) {}
    };

} // namespace System

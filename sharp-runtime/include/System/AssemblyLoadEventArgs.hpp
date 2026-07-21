// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <functional>
#include <string>
#include "System/EventArgs.hpp"

namespace System {

/**
 * @brief Provides data for the event that is raised when an assembly is loaded.
 *
 * C++ counterpart of .NET System.AssemblyLoadEventArgs.
 * Since sharp-runtime has no reflection, the loaded assembly is represented
 * by its name string rather than a System.Reflection.Assembly instance.
 */
class AssemblyLoadEventArgs : public EventArgs {
    std::string assemblyName_;
public:
    /**
     * @brief Initializes a new instance with the name of the loaded assembly.
     *
     * C++ counterpart of .NET AssemblyLoadEventArgs(Assembly loadedAssembly).
     * @param assemblyName The name of the assembly that was loaded.
     */
    explicit AssemblyLoadEventArgs(const std::string& assemblyName)
        : assemblyName_(assemblyName) {}

    /**
     * @brief Gets the name of the assembly that was loaded.
     *
     * C++ counterpart of .NET AssemblyLoadEventArgs.LoadedAssembly.
     * @return The assembly name string.
     */
    [[nodiscard]] const std::string& getLoadedAssemblyProperty() const { return assemblyName_; }
};

/**
 * @brief Delegate type for assembly-load event handlers.
 *
 * C++ counterpart of .NET System.AssemblyLoadEventHandler.
 */
using AssemblyLoadEventHandler = std::function<void(void*, AssemblyLoadEventArgs&)>;

} // namespace System

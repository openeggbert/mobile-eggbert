// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <mutex>
#include <string>
#include <unordered_map>
#include "System/AppDomain.hpp"
#include "System/ArgumentException.hpp"

namespace System {

    /**
     * @brief Provides members for setting and retrieving data about an application's context.
     *
     * C++ counterpart of .NET System.AppContext static class.
     * Supports the named-data store, compatibility switches, and the base directory.
     */
    class AppContext {
    public:
        AppContext() = delete;

        // -----------------------------------------------------------------------
        // Properties
        // -----------------------------------------------------------------------

        /**
         * @brief Gets the file path of the base directory that the assembly resolver
         * uses to probe for assemblies.
         *
         * C++ counterpart of .NET AppContext.BaseDirectory.
         * Delegates to AppDomain.CurrentDomain().BaseDirectory.
         * @return The base directory path (ends with a directory separator).
         */
        static const std::string& getBaseDirectoryProperty() {
            return AppDomain::CurrentDomain().getBaseDirectoryProperty();
        }

        /**
         * @brief Gets the name of the framework version targeted by the current application.
         *
         * C++ counterpart of .NET AppContext.TargetFrameworkName.
         * Always returns an empty string in sharp-runtime (CLR reflection not available).
         * @return An empty string.
         */
        static std::string getTargetFrameworkNameProperty() {
            return {};
        }

        // -----------------------------------------------------------------------
        // Data store
        // -----------------------------------------------------------------------

        /**
         * @brief Returns the value of the named data element assigned to the current application.
         *
         * C++ counterpart of .NET AppContext.GetData(string).
         * @param name The name of the data element.
         * @return A pointer to the stored value, or nullptr if @p name is not found.
         */
        static void* GetData(const std::string& name) {
            std::lock_guard<std::mutex> lock(mutex_());
            auto& store = dataStore_();
            auto it = store.find(name);
            return (it != store.end()) ? it->second : nullptr;
        }

        /**
         * @brief Assigns a value to the named data element of the current application context.
         *
         * C++ counterpart of .NET AppContext.SetData(string, object).
         * @param name The name of the data element.
         * @param data A pointer to the value to associate with @p name.
         */
        static void SetData(const std::string& name, void* data) {
            std::lock_guard<std::mutex> lock(mutex_());
            dataStore_()[name] = data;
        }

        // -----------------------------------------------------------------------
        // Compatibility switches
        // -----------------------------------------------------------------------

        /**
         * @brief Tries to get the value of the named compatibility switch.
         *
         * C++ counterpart of .NET AppContext.TryGetSwitch(string, out bool).
         * @param switchName The name of the switch.
         * @param isEnabled  Set to the switch value on success, or false on failure.
         * @return true if the switch was found and its value was assigned to @p isEnabled;
         *         false if the switch has not been set.
         * @throws System::ArgumentException if @p switchName is empty.
         */
        static bool TryGetSwitch(const std::string& switchName, bool& isEnabled) {
            if (switchName.empty())
                throw System::ArgumentException("The value cannot be an empty string.", "switchName");
            std::lock_guard<std::mutex> lock(mutex_());
            auto& sw = switches_();
            auto it = sw.find(switchName);
            if (it != sw.end()) {
                isEnabled = it->second;
                return true;
            }
            isEnabled = false;
            return false;
        }

        /**
         * @brief Sets the value of the named compatibility switch.
         *
         * C++ counterpart of .NET AppContext.SetSwitch(string, bool).
         * @param switchName The name of the switch.
         * @param isEnabled  The value to assign to the switch.
         * @throws System::ArgumentException if @p switchName is empty.
         */
        static void SetSwitch(const std::string& switchName, bool isEnabled) {
            if (switchName.empty())
                throw System::ArgumentException("The value cannot be an empty string.", "switchName");
            std::lock_guard<std::mutex> lock(mutex_());
            switches_()[switchName] = isEnabled;
        }

    private:
        static std::mutex& mutex_() {
            static std::mutex m;
            return m;
        }
        static std::unordered_map<std::string, void*>& dataStore_() {
            static std::unordered_map<std::string, void*> s;
            return s;
        }
        static std::unordered_map<std::string, bool>& switches_() {
            static std::unordered_map<std::string, bool> s;
            return s;
        }
    };

} // namespace System

// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <string>
#include <vector>

#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Diagnostics {

    using SharpRuntime::intcs;

    /**
     * @brief Specifies a set of values used when starting a process.
     *
     * Partial C++ counterpart of .NET System.Diagnostics.ProcessStartInfo, scoped to the
     * arguments-based launch model (@ref getArgumentListProperty is the safe, unambiguous way to
     * pass arguments containing spaces or special characters -- no shell/OS quoting rules apply).
     * @ref getArgumentsProperty is provided for convenience but only splits on ASCII whitespace,
     * NOT real .NET's full Win32-style argument re-quoting grammar (which itself only matters on
     * Windows; on POSIX, argv is always a plain string array with no re-quoting step at all) --
     * use @ref getArgumentListProperty directly for anything containing spaces or quotes.
     *
     * @note Status: Partial. UseShellExecute, redirected standard input, environment-variable
     * overrides, and window-style/verb properties (all Windows-shell-specific or otherwise out of
     * scope for a POSIX-only game-runtime launcher) are not implemented -- see
     * System::Diagnostics::Process's class doc-comment for the full list of deliberately deferred
     * surface.
     */
    class ProcessStartInfo {
        std::string fileName_;
        std::string arguments_;
        std::vector<std::string> argumentList_;
        std::string workingDirectory_;
        bool redirectStandardOutput_ = false;
        bool redirectStandardError_ = false;

    public:
        ProcessStartInfo() = default;

        /** @brief Initializes a new instance with the specified executable path. */
        explicit ProcessStartInfo(const std::string& fileName) : fileName_(fileName) {}

        /** @brief Initializes a new instance with the specified executable path and argument string. */
        ProcessStartInfo(const std::string& fileName, const std::string& arguments)
            : fileName_(fileName), arguments_(arguments) {}

        /** @brief Gets or sets the application to start. */
        [[nodiscard]] const std::string& getFileNameProperty() const { return fileName_; }
        void setFileNameProperty(const std::string& value) { fileName_ = value; }

        /**
         * @brief Gets or sets a single string of command-line arguments, split on ASCII whitespace
         * when the process is started. See the class doc-comment for why @ref
         * getArgumentListProperty is preferred whenever an argument may contain spaces or quotes.
         */
        [[nodiscard]] const std::string& getArgumentsProperty() const { return arguments_; }
        void setArgumentsProperty(const std::string& value) { arguments_ = value; }

        /** @brief Gets the collection of command-line arguments, passed to the child process verbatim (no shell quoting). */
        [[nodiscard]] std::vector<std::string>& getArgumentListProperty() { return argumentList_; }
        [[nodiscard]] const std::vector<std::string>& getArgumentListProperty() const { return argumentList_; }

        /** @brief Gets or sets the working directory for the process. Empty means inherit the caller's current directory. */
        [[nodiscard]] const std::string& getWorkingDirectoryProperty() const { return workingDirectory_; }
        void setWorkingDirectoryProperty(const std::string& value) { workingDirectory_ = value; }

        /** @brief Gets or sets whether the process's standard output is redirected and captured. */
        [[nodiscard]] bool getRedirectStandardOutputProperty() const { return redirectStandardOutput_; }
        void setRedirectStandardOutputProperty(bool value) { redirectStandardOutput_ = value; }

        /** @brief Gets or sets whether the process's standard error is redirected and captured. */
        [[nodiscard]] bool getRedirectStandardErrorProperty() const { return redirectStandardError_; }
        void setRedirectStandardErrorProperty(bool value) { redirectStandardError_ = value; }
    };

} // namespace System::Diagnostics

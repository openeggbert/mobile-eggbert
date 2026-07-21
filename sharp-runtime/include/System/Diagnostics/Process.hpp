// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <memory>
#include <string>
#include <thread>

#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Diagnostics/ProcessStartInfo.hpp"

namespace System::Diagnostics {

    using SharpRuntime::intcs;

    /**
     * @brief Provides access to launching, waiting on, and terminating a local external process.
     *
     * Partial C++ counterpart of .NET System.Diagnostics.Process, scoped to the core
     * launch/wait/exit-code/kill lifecycle a game runtime typically needs (e.g. running an
     * external updater/installer, invoking a build/asset-processing tool, opening a URL via a
     * platform "open" helper) rather than real .NET's full systems-programming surface.
     *
     * @note Status: Partial, POSIX-only (uses fork()/execvp()/waitpid() in the .cpp body, guarded
     * behind #ifdef so the public header stays portable; throws
     * System::PlatformNotSupportedException on Emscripten). Implemented: Start (instance and the
     * three static overloads), WaitForExit (blocking and timeout forms), Kill (single process and
     * process-tree via killpg), ExitCode, HasExited, Id, GetCurrentProcess, and optional
     * captured-text stdout/stderr redirection (a deliberate simplification of real .NET's
     * Stream-based StandardOutput/StandardError -- see getStandardOutputTextProperty). Explicitly
     * NOT implemented, all deliberately out of scope for this pass: process enumeration
     * (GetProcesses/GetProcessById), memory/CPU/priority/module/thread introspection properties,
     * the Exited/OutputDataReceived/ErrorDataReceived event-based async I/O model, UseShellExecute
     * (Windows-shell-specific), and the .NET 10 Run/RunAsync/RunAndCaptureText helper family.
     */
    class Process {
    public:
        struct Impl;
    private:
        std::unique_ptr<Impl> impl_;

        Process();
        static void reapIfNeeded(Impl& impl);

    public:
        ~Process();
        Process(const Process&) = delete;
        Process& operator=(const Process&) = delete;
        Process(Process&&) noexcept;
        Process& operator=(Process&&) noexcept;

        /** @brief Gets or sets the properties to pass to the Start method. */
        [[nodiscard]] const ProcessStartInfo& getStartInfoProperty() const;
        void setStartInfoProperty(const ProcessStartInfo& value);

        /** @brief Starts (or restarts) the process using getStartInfoProperty(). @return true if a process resource was started. */
        bool Start();

        /**
         * @brief Gets the native process identifier.
         * @throws System::InvalidOperationException if the process has not been started.
         */
        [[nodiscard]] intcs getIdProperty() const;

        /** @brief Gets a value indicating whether the associated process has terminated. */
        [[nodiscard]] bool getHasExitedProperty() const;

        /**
         * @brief Gets the exit code returned by the associated process.
         * @throws System::InvalidOperationException if the process has not exited yet.
         */
        [[nodiscard]] intcs getExitCodeProperty() const;

        /**
         * @brief Gets the captured standard output text.
         * @throws System::InvalidOperationException unless RedirectStandardOutput was set before Start().
         */
        [[nodiscard]] const std::string& getStandardOutputTextProperty() const;

        /**
         * @brief Gets the captured standard error text.
         * @throws System::InvalidOperationException unless RedirectStandardError was set before Start().
         */
        [[nodiscard]] const std::string& getStandardErrorTextProperty() const;

        /** @brief Immediately stops the associated process by sending SIGKILL. */
        void Kill();

        /** @brief Immediately stops the associated process, and optionally its full process tree, via SIGKILL. */
        void Kill(bool entireProcessTree);

        /** @brief Blocks the calling thread until the associated process terminates. */
        void WaitForExit();

        /** @brief Blocks up to @p milliseconds for the process to terminate. @return true if the process exited before the timeout. */
        bool WaitForExit(intcs milliseconds);

        /** @brief Starts a process resource using the specified start info. */
        [[nodiscard]] static Process Start(const ProcessStartInfo& startInfo);
        /** @brief Starts a process resource for the specified executable path. */
        [[nodiscard]] static Process Start(const std::string& fileName);
        /** @brief Starts a process resource for the specified executable path and argument string. */
        [[nodiscard]] static Process Start(const std::string& fileName, const std::string& arguments);

        /** @brief Returns a Process wrapping the currently running process. WaitForExit/Kill are unsupported on it (it is not a child of itself). */
        [[nodiscard]] static Process GetCurrentProcess();
    };

} // namespace System::Diagnostics

// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Diagnostics/Process.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/PlatformNotSupportedException.hpp"

#include <chrono>
#include <cstdlib>
#include <sstream>

#if !defined(_WIN32) && !defined(__EMSCRIPTEN__)
#  include <sys/wait.h>
#  include <sys/types.h>
#  include <unistd.h>
#  include <signal.h>
#  include <fcntl.h>
#  include <cerrno>
#  include <cstring>
#  define SHARP_RUNTIME_PROCESS_POSIX 1
#endif

namespace System::Diagnostics {

struct Process::Impl {
    ProcessStartInfo startInfo;
#if defined(SHARP_RUNTIME_PROCESS_POSIX)
    pid_t pid = -1;
#endif
    bool started = false;
    bool hasExited = false;
    bool isCurrentProcess = false;
    intcs exitCode = 0;
    std::string stdoutText;
    std::string stderrText;
    std::thread stdoutReader;
    std::thread stderrReader;

    ~Impl() {
        if (stdoutReader.joinable()) stdoutReader.join();
        if (stderrReader.joinable()) stderrReader.join();
    }
};

Process::Process() : impl_(std::make_unique<Impl>()) {}
Process::~Process() = default;
Process::Process(Process&&) noexcept = default;
Process& Process::operator=(Process&&) noexcept = default;

const ProcessStartInfo& Process::getStartInfoProperty() const { return impl_->startInfo; }
void Process::setStartInfoProperty(const ProcessStartInfo& value) { impl_->startInfo = value; }

intcs Process::getIdProperty() const {
#if defined(SHARP_RUNTIME_PROCESS_POSIX)
    if (!impl_->started) throw System::InvalidOperationException("No process is associated with this object.");
    return static_cast<intcs>(impl_->pid);
#else
    throw System::PlatformNotSupportedException("System::Diagnostics::Process is only supported on POSIX platforms.");
#endif
}

#if defined(SHARP_RUNTIME_PROCESS_POSIX)
namespace {
    // Drains a pipe fd into *out on a background thread until EOF/error, matching this codebase's
    // established pattern (Timer, Socket's async methods) of using std::thread for blocking I/O
    // rather than requiring the caller to poll -- avoids the classic pipe deadlock where the child
    // blocks writing to a full pipe buffer while nobody is reading it.
    void drainPipe(int fd, std::string* out) {
        char buf[4096];
        for (;;) {
            ssize_t n = ::read(fd, buf, sizeof(buf));
            if (n > 0) { out->append(buf, static_cast<size_t>(n)); continue; }
            if (n == 0) break; // EOF
            if (errno == EINTR) continue;
            break;
        }
        ::close(fd);
    }

}

void Process::reapIfNeeded(Impl& impl) {
    if (impl.hasExited || !impl.started || impl.isCurrentProcess) return;
    int status = 0;
    pid_t r = ::waitpid(impl.pid, &status, WNOHANG);
    if (r == impl.pid) {
        impl.hasExited = true;
        impl.exitCode = WIFEXITED(status) ? WEXITSTATUS(status)
                       : WIFSIGNALED(status) ? (128 + WTERMSIG(status))
                       : -1;
        if (impl.stdoutReader.joinable()) impl.stdoutReader.join();
        if (impl.stderrReader.joinable()) impl.stderrReader.join();
    }
}
#endif

bool Process::getHasExitedProperty() const {
#if defined(SHARP_RUNTIME_PROCESS_POSIX)
    if (!impl_->started) throw System::InvalidOperationException("No process is associated with this object.");
    if (impl_->isCurrentProcess) return false;
    reapIfNeeded(*impl_);
    return impl_->hasExited;
#else
    throw System::PlatformNotSupportedException("System::Diagnostics::Process is only supported on POSIX platforms.");
#endif
}

intcs Process::getExitCodeProperty() const {
#if defined(SHARP_RUNTIME_PROCESS_POSIX)
    if (!getHasExitedProperty())
        throw System::InvalidOperationException("Process must exit before requested information can be determined.");
    return impl_->exitCode;
#else
    throw System::PlatformNotSupportedException("System::Diagnostics::Process is only supported on POSIX platforms.");
#endif
}

const std::string& Process::getStandardOutputTextProperty() const {
    if (!impl_->startInfo.getRedirectStandardOutputProperty())
        throw System::InvalidOperationException(
            "StandardOut has not been redirected. Set RedirectStandardOutput on ProcessStartInfo before starting the process.");
    return impl_->stdoutText;
}

const std::string& Process::getStandardErrorTextProperty() const {
    if (!impl_->startInfo.getRedirectStandardErrorProperty())
        throw System::InvalidOperationException(
            "StandardError has not been redirected. Set RedirectStandardError on ProcessStartInfo before starting the process.");
    return impl_->stderrText;
}

bool Process::Start() {
#if defined(SHARP_RUNTIME_PROCESS_POSIX)
    const ProcessStartInfo& si = impl_->startInfo;
    if (si.getFileNameProperty().empty())
        throw System::InvalidOperationException("Cannot start process because a file name has not been provided.");

    // Build argv: argv[0] is conventionally the program name, followed by ArgumentList (the
    // unambiguous path -- no shell/OS quoting), then a naive whitespace split of Arguments for
    // convenience (see ProcessStartInfo's doc-comment for why this is intentionally simplified).
    std::vector<std::string> argvStorage;
    argvStorage.push_back(si.getFileNameProperty());
    for (const auto& a : si.getArgumentListProperty()) argvStorage.push_back(a);
    if (!si.getArgumentsProperty().empty()) {
        std::istringstream iss(si.getArgumentsProperty());
        std::string tok;
        while (iss >> tok) argvStorage.push_back(tok);
    }
    std::vector<char*> argv;
    argv.reserve(argvStorage.size() + 1);
    for (auto& s : argvStorage) argv.push_back(s.data());
    argv.push_back(nullptr);

    int stdoutPipe[2] = {-1, -1};
    int stderrPipe[2] = {-1, -1};
    bool redirOut = si.getRedirectStandardOutputProperty();
    bool redirErr = si.getRedirectStandardErrorProperty();
    if (redirOut && ::pipe(stdoutPipe) != 0)
        throw System::InvalidOperationException(std::string("Failed to create stdout pipe: ") + std::strerror(errno));
    if (redirErr && ::pipe(stderrPipe) != 0) {
        // stdoutPipe was already successfully created above -- close it before throwing, or it
        // leaks both fds (ironic: this failure path fires exactly when fds are scarce, which is
        // one likely reason ::pipe() just failed).
        if (redirOut) { ::close(stdoutPipe[0]); ::close(stdoutPipe[1]); }
        throw System::InvalidOperationException(std::string("Failed to create stderr pipe: ") + std::strerror(errno));
    }

    pid_t pid = ::fork();
    if (pid < 0) {
        if (redirOut) { ::close(stdoutPipe[0]); ::close(stdoutPipe[1]); }
        if (redirErr) { ::close(stderrPipe[0]); ::close(stderrPipe[1]); }
        throw System::InvalidOperationException(std::string("Failed to fork: ") + std::strerror(errno));
    }

    if (pid == 0) {
        // Child: put it in its own process group so Kill(entireProcessTree=true) can target the
        // whole tree via killpg without also signaling the parent's group.
        ::setpgid(0, 0);
        if (redirOut) { ::dup2(stdoutPipe[1], STDOUT_FILENO); ::close(stdoutPipe[0]); ::close(stdoutPipe[1]); }
        if (redirErr) { ::dup2(stderrPipe[1], STDERR_FILENO); ::close(stderrPipe[0]); ::close(stderrPipe[1]); }
        if (!si.getWorkingDirectoryProperty().empty()) {
            if (::chdir(si.getWorkingDirectoryProperty().c_str()) != 0) ::_exit(127);
        }
        ::execvp(argv[0], argv.data());
        ::_exit(127); // execvp only returns on failure
    }

    // Parent
    if (redirOut) {
        ::close(stdoutPipe[1]);
        impl_->stdoutReader = std::thread(drainPipe, stdoutPipe[0], &impl_->stdoutText);
    }
    if (redirErr) {
        ::close(stderrPipe[1]);
        impl_->stderrReader = std::thread(drainPipe, stderrPipe[0], &impl_->stderrText);
    }
    impl_->pid = pid;
    impl_->started = true;
    impl_->hasExited = false;
    return true;
#else
    throw System::PlatformNotSupportedException("System::Diagnostics::Process is only supported on POSIX platforms.");
#endif
}

void Process::Kill() { Kill(false); }

void Process::Kill(bool entireProcessTree) {
    (void)entireProcessTree; // unused on non-POSIX platforms, where this throws below
#if defined(SHARP_RUNTIME_PROCESS_POSIX)
    if (!impl_->started || impl_->isCurrentProcess) return;
    reapIfNeeded(*impl_);
    if (impl_->hasExited) return;
    if (entireProcessTree) ::killpg(impl_->pid, SIGKILL);
    else ::kill(impl_->pid, SIGKILL);
#else
    throw System::PlatformNotSupportedException("System::Diagnostics::Process is only supported on POSIX platforms.");
#endif
}

void Process::WaitForExit() {
#if defined(SHARP_RUNTIME_PROCESS_POSIX)
    if (!impl_->started) throw System::InvalidOperationException("No process is associated with this object.");
    if (impl_->isCurrentProcess) return;
    if (impl_->hasExited) return;
    int status = 0;
    pid_t r = ::waitpid(impl_->pid, &status, 0);
    if (r == impl_->pid) {
        impl_->hasExited = true;
        impl_->exitCode = WIFEXITED(status) ? WEXITSTATUS(status)
                         : WIFSIGNALED(status) ? (128 + WTERMSIG(status))
                         : -1;
    }
    if (impl_->stdoutReader.joinable()) impl_->stdoutReader.join();
    if (impl_->stderrReader.joinable()) impl_->stderrReader.join();
#else
    throw System::PlatformNotSupportedException("System::Diagnostics::Process is only supported on POSIX platforms.");
#endif
}

bool Process::WaitForExit(intcs milliseconds) {
    (void)milliseconds; // unused on non-POSIX platforms, where this throws below
#if defined(SHARP_RUNTIME_PROCESS_POSIX)
    if (!impl_->started) throw System::InvalidOperationException("No process is associated with this object.");
    if (impl_->isCurrentProcess) return false;
    reapIfNeeded(*impl_);
    if (impl_->hasExited) return true;
    auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(milliseconds);
    while (std::chrono::steady_clock::now() < deadline) {
        reapIfNeeded(*impl_);
        if (impl_->hasExited) return true;
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    reapIfNeeded(*impl_);
    return impl_->hasExited;
#else
    throw System::PlatformNotSupportedException("System::Diagnostics::Process is only supported on POSIX platforms.");
#endif
}

Process Process::Start(const ProcessStartInfo& startInfo) {
    Process p;
    p.setStartInfoProperty(startInfo);
    p.Start();
    return p;
}

Process Process::Start(const std::string& fileName) {
    return Start(ProcessStartInfo(fileName));
}

Process Process::Start(const std::string& fileName, const std::string& arguments) {
    return Start(ProcessStartInfo(fileName, arguments));
}

Process Process::GetCurrentProcess() {
#if defined(SHARP_RUNTIME_PROCESS_POSIX)
    Process p;
    p.impl_->pid = ::getpid();
    p.impl_->started = true;
    p.impl_->isCurrentProcess = true;
    return p;
#else
    throw System::PlatformNotSupportedException("System::Diagnostics::Process is only supported on POSIX platforms.");
#endif
}

} // namespace System::Diagnostics

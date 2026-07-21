// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Runtime/InteropServices/PosixSignalRegistration.hpp"
#include "System/ArgumentNullException.hpp"
#include "System/PlatformNotSupportedException.hpp"
#include "System/IO/IOException.hpp"

#if defined(_WIN32) || defined(__EMSCRIPTEN__)
// No POSIX signal handling on Windows (real .NET uses a separate console-control-handler
// mechanism there, out of scope for this pass -- see the platform-not-supported fallback below)
// or under Emscripten (no OS-level signal delivery in a browser sandbox).
#else
#include <algorithm>
#include <atomic>
#include <csignal>
#include <cstdio>
#include <mutex>
#include <thread>
#include <signal.h>
#include <unistd.h>
#include <vector>

namespace System::Runtime::InteropServices {
namespace {

    // Maps the portable PosixSignal enum to this platform's actual SIGxxx macro value, indexed by
    // -static_cast<int>(signal) - 1 (PosixSignal's values run Sighup=-1 .. Sigtstp=-10,
    // contiguous, matching this table's index 0..9). Sigkill(-11) is intentionally excluded --
    // it can never be caught (sigaction(SIGKILL, ...) always fails with EINVAL), so Create()
    // rejects it before reaching this function. Built as a table lookup rather than a switch over
    // the enum's named members so this function reads naturally with the real SIGxxx macros this
    // translation unit needs anyway (see PosixSignal.hpp's doc-comment for why the enum members
    // themselves are spelled Sighup/Sigint/etc. rather than matching those macro names exactly).
    int toNativeSignalNumber(PosixSignal signal) {
        static const int kNativeSignalNumbers[10] = {
            SIGHUP, SIGINT, SIGQUIT, SIGTERM, SIGCHLD, SIGCONT, SIGWINCH, SIGTTIN, SIGTTOU, SIGTSTP
        };
        int index = -static_cast<int>(signal) - 1;
        if (index < 0 || index >= 10) return 0;
        return kNativeSignalNumbers[index];
    }

    struct Entry {
        SharpRuntime::intcs token;
        PosixSignal signal;
        PosixSignalRegistration::Handler handler;
    };

    // Guards both registrations_ (the C++-visible table) and installedSignals_ (which raw signal
    // numbers currently have our sigaction handler installed).
    std::mutex registryMutex_;
    std::vector<Entry> registrations_;
    std::vector<int> installedSignals_;
    SharpRuntime::intcs nextToken_ = 0;

    // Async-signal-safe pending-signal flags, indexed directly by native signal number. sig_atomic_t
    // writes/reads are guaranteed atomic with respect to signal delivery on the same thread by the
    // C standard; combined with the self-pipe write below, this is the standard safe pattern for
    // moving work out of a signal handler (POSIX.1 "Signal Concepts" / the classic self-pipe trick).
    constexpr int kMaxSignalNumber = 64;
    volatile std::sig_atomic_t pending_[kMaxSignalNumber] = {};

    int selfPipe_[2] = {-1, -1};
    std::thread watcherThread_;
    std::atomic<bool> watcherRunning_{false};

    void onNativeSignal(int signo) {
        if (signo >= 0 && signo < kMaxSignalNumber) pending_[signo] = 1;
        char byte = 0;
        // write() to a pipe is async-signal-safe; ignore errors (EAGAIN on a full pipe just means
        // the watcher thread hasn't drained a previous wakeup yet, which is fine -- pending_ already
        // recorded this delivery).
        ssize_t ignored = ::write(selfPipe_[1], &byte, 1);
        (void)ignored;
    }

    void dispatchSignal(int signo) {
        std::vector<Entry> snapshot;
        {
            std::lock_guard<std::mutex> lock(registryMutex_);
            for (const auto& e : registrations_)
                if (toNativeSignalNumber(e.signal) == signo) snapshot.push_back(e);
        }
        if (snapshot.empty()) return;

        // Reverse registration order, matching real .NET's PosixSignalRegistration.Unix.cs.
        PosixSignalContext context(snapshot.back().signal);
        bool canceled = false;
        for (auto it = snapshot.rbegin(); it != snapshot.rend(); ++it) {
            context.setSignalProperty(it->signal);
            it->handler(context);
            if (context.getCancelProperty()) canceled = true;
        }

        if (!canceled) {
            // Run the OS default disposition: temporarily restore SIG_DFL and re-raise, matching
            // real .NET's Interop.Sys.HandleNonCanceledPosixSignal. For most of these signals
            // (SIGTERM/SIGINT/SIGQUIT/SIGHUP) the default disposition terminates the process.
            std::signal(signo, SIG_DFL);
            ::raise(signo);
            // Only reached for signals whose default disposition doesn't terminate (e.g. SIGCHLD,
            // SIGCONT, SIGWINCH, SIGTTIN/SIGTTOU default to ignore/stop-and-continue) -- reinstall
            // our handler so future deliveries are still caught.
            std::signal(signo, onNativeSignal);
        }
    }

    void watcherLoop() {
        while (watcherRunning_.load()) {
            char buf[64];
            ssize_t n = ::read(selfPipe_[0], buf, sizeof(buf));
            if (n <= 0) {
                if (!watcherRunning_.load()) break;
                continue;
            }
            for (int signo = 0; signo < kMaxSignalNumber; ++signo) {
                if (pending_[signo]) {
                    pending_[signo] = 0;
                    dispatchSignal(signo);
                }
            }
        }
    }

    void ensureWatcherStarted() {
        // Caller already holds registryMutex_.
        if (watcherRunning_.load()) return;
        if (::pipe(selfPipe_) != 0) {
            throw System::IO::IOException("Failed to initialize POSIX signal handling (pipe() failed).");
        }
        watcherRunning_.store(true);
        watcherThread_ = std::thread(watcherLoop);
        watcherThread_.detach();
    }

    void installIfNeeded(int signo) {
        // Caller already holds registryMutex_.
        for (int s : installedSignals_) if (s == signo) return;
        struct sigaction sa{};
        sa.sa_handler = onNativeSignal;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESTART;
        if (sigaction(signo, &sa, nullptr) != 0) {
            throw System::IO::IOException("Failed to install handler for the requested POSIX signal.");
        }
        installedSignals_.push_back(signo);
    }

    void uninstallIfUnused(int signo) {
        // Caller already holds registryMutex_.
        for (const auto& e : registrations_)
            if (toNativeSignalNumber(e.signal) == signo) return; // still in use
        std::signal(signo, SIG_DFL);
        installedSignals_.erase(
            std::remove(installedSignals_.begin(), installedSignals_.end(), signo),
            installedSignals_.end());
    }

} // namespace

PosixSignalRegistration::PosixSignalRegistration(PosixSignalRegistration&& other) noexcept
    : signal_(other.signal_), token_(other.token_) {
    other.token_ = -1;
}

PosixSignalRegistration& PosixSignalRegistration::operator=(PosixSignalRegistration&& other) noexcept {
    if (this != &other) {
        Dispose();
        signal_ = other.signal_;
        token_ = other.token_;
        other.token_ = -1;
    }
    return *this;
}

PosixSignalRegistration::~PosixSignalRegistration() { Dispose(); }

PosixSignalRegistration PosixSignalRegistration::Create(PosixSignal signal, Handler handler) {
    if (!handler) throw System::ArgumentNullException("handler");
    if (signal == PosixSignal::Sigkill)
        throw System::PlatformNotSupportedException("SIGKILL cannot be caught or ignored.");

    int signo = toNativeSignalNumber(signal);
    if (signo == 0) throw System::PlatformNotSupportedException();

    std::lock_guard<std::mutex> lock(registryMutex_);
    ensureWatcherStarted();
    installIfNeeded(signo);

    SharpRuntime::intcs token = nextToken_++;
    registrations_.push_back(Entry{token, signal, std::move(handler)});
    return PosixSignalRegistration(signal, token);
}

void PosixSignalRegistration::Dispose() {
    if (token_ < 0) return;
    SharpRuntime::intcs token = token_;
    token_ = -1;

    std::lock_guard<std::mutex> lock(registryMutex_);
    int signo = toNativeSignalNumber(signal_);
    registrations_.erase(
        std::remove_if(registrations_.begin(), registrations_.end(),
                        [token](const Entry& e) { return e.token == token; }),
        registrations_.end());
    uninstallIfUnused(signo);
}

} // namespace System::Runtime::InteropServices

#endif // !_WIN32 && !__EMSCRIPTEN__

#if defined(_WIN32) || defined(__EMSCRIPTEN__)
namespace System::Runtime::InteropServices {

PosixSignalRegistration::PosixSignalRegistration(PosixSignalRegistration&& other) noexcept
    : signal_(other.signal_), token_(other.token_) {
    other.token_ = -1;
}

PosixSignalRegistration& PosixSignalRegistration::operator=(PosixSignalRegistration&& other) noexcept {
    signal_ = other.signal_;
    token_ = other.token_;
    other.token_ = -1;
    return *this;
}

PosixSignalRegistration::~PosixSignalRegistration() = default;

PosixSignalRegistration PosixSignalRegistration::Create(PosixSignal, Handler handler) {
    if (!handler) throw System::ArgumentNullException("handler");
    throw System::PlatformNotSupportedException(
        "POSIX signal handling is not implemented on this platform in this port.");
}

void PosixSignalRegistration::Dispose() { token_ = -1; }

} // namespace System::Runtime::InteropServices
#endif

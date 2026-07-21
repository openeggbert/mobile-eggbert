// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Console.hpp"

#if defined(_WIN32)
#  include <io.h>
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <windows.h>
#elif defined(__EMSCRIPTEN__)
// No terminal to redirect-detect; stdin/stdout/stderr are never real TTYs.
#else
#  include <sys/ioctl.h>
#  include <unistd.h>
#endif

namespace System {

    // Verified against real .NET's Unix ConsolePal.IsOutputRedirectedCore et al.: "redirected"
    // means the stream is *not* wrapping a real character device (TTY) -- callers gate
    // TTY-only behavior (colors, cursor positioning, raw key reads) on `!IsXRedirected`. On
    // Emscripten there is no real TTY at all, so every stream must report as redirected
    // (true); hardcoding false here previously told callers the opposite -- that they had a
    // normal interactive terminal to control -- which is exactly backwards.

    bool Console::getIsInputRedirectedProperty() {
#if defined(_WIN32)
        return !_isatty(_fileno(stdin));
#elif defined(__EMSCRIPTEN__)
        return true;
#else
        return !isatty(fileno(stdin));
#endif
    }

    bool Console::getIsOutputRedirectedProperty() {
#if defined(_WIN32)
        return !_isatty(_fileno(stdout));
#elif defined(__EMSCRIPTEN__)
        return true;
#else
        return !isatty(fileno(stdout));
#endif
    }

    bool Console::getIsErrorRedirectedProperty() {
#if defined(_WIN32)
        return !_isatty(_fileno(stderr));
#elif defined(__EMSCRIPTEN__)
        return true;
#else
        return !isatty(fileno(stderr));
#endif
    }

    // Verified against real .NET's Unix ConsolePal.GetWindowSize: queries the real terminal
    // size (ioctl(TIOCGWINSZ) there; here too) and only falls back to a fixed default when
    // the query fails -- e.g. because stdout is redirected to a file/pipe rather than a TTY,
    // which has no window size to report. Previously this returned the hardcoded fallback
    // unconditionally, which is silently wrong for any real interactive terminal (a common
    // scenario: existing code assuming an 80-column console misformats output on a wider one).

    intcs Console::getWindowWidthProperty() {
#if defined(_WIN32)
        CONSOLE_SCREEN_BUFFER_INFO info;
        if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info))
            return static_cast<intcs>(info.srWindow.Right - info.srWindow.Left + 1);
        return 80;
#elif defined(__EMSCRIPTEN__)
        return 80;
#else
        struct winsize ws{};
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0)
            return static_cast<intcs>(ws.ws_col);
        return 80;
#endif
    }

    intcs Console::getWindowHeightProperty() {
#if defined(_WIN32)
        CONSOLE_SCREEN_BUFFER_INFO info;
        if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info))
            return static_cast<intcs>(info.srWindow.Bottom - info.srWindow.Top + 1);
        return 24;
#elif defined(__EMSCRIPTEN__)
        return 24;
#else
        struct winsize ws{};
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_row > 0)
            return static_cast<intcs>(ws.ws_row);
        return 24;
#endif
    }

} // namespace System

// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <array>
#include <charconv>
#include <cstdio>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/ConsoleColor.hpp"
#include "System/ConsoleKeyInfo.hpp"
#include "System/ConsoleCancelEventArgs.hpp"
#include "System/String.hpp"

namespace System {

    using SharpRuntime::intcs;
    using SharpRuntime::longcs;
    using SharpRuntime::uintcs;
    using SharpRuntime::ulongcs;
    using SharpRuntime::Single;

    /**
     * @brief Represents the standard input, output, and error streams for console applications.
     *
     * C++ counterpart of .NET System.Console.
     * All members are static; instantiation is not allowed.
     * Color and cursor features use ANSI escape sequences (POSIX terminals only).
     */
    class Console {
    public:
        /** @brief Deleted constructor — all members are static. */
        Console() = delete;

        /**
         * @brief The newline string used by the current environment.
         * "\r\n" on Windows, "\n" on POSIX.
         */
        static inline const std::string NewLine =
#ifdef _WIN32
            "\r\n";
#else
            "\n";
#endif

        // -----------------------------------------------------------------------
        // Write
        // -----------------------------------------------------------------------

        /** @brief Writes the specified string to the standard output stream. */
        static void Write(const std::string& value) { std::cout << value; }
        /** @brief Writes the specified C-string to the standard output stream. */
        static void Write(const char* value)         { std::cout << value; }
        /** @brief Writes the specified character to the standard output stream. */
        static void Write(char value)                { std::cout << value; }
        /** @brief Writes the specified integer to the standard output stream. */
        static void Write(intcs value)               { std::cout << value; }
        /** @brief Writes the specified 64-bit integer to the standard output stream. */
        static void Write(longcs value)              { std::cout << value; }
        /** @brief Writes the specified double to the standard output stream. */
        static void Write(double value) {
            std::array<char, 64> buf;
            auto [ptr, ec] = std::to_chars(buf.data(), buf.data() + buf.size(), value);
            if (ec == std::errc{}) std::cout.write(buf.data(), ptr - buf.data());
            else std::cout << value;
        }
        /** @brief Writes the specified float to the standard output stream. */
        static void Write(float value) {
            std::array<char, 32> buf;
            auto [ptr, ec] = std::to_chars(buf.data(), buf.data() + buf.size(), value);
            if (ec == std::errc{}) std::cout.write(buf.data(), ptr - buf.data());
            else std::cout << value;
        }
        /** @brief Writes the specified Boolean to the standard output stream. */
        static void Write(bool value) { std::cout << (value ? "True" : "False"); }
        /** @brief Writes the specified unsigned 32-bit integer to the standard output stream. */
        static void Write(uintcs value)  { std::cout << value; }
        /** @brief Writes the specified unsigned 64-bit integer to the standard output stream. */
        static void Write(ulongcs value) { std::cout << value; }

        /**
         * @brief Writes all characters in a vector to the standard output stream.
         * @param buffer Vector of characters to write.
         */
        static void Write(const std::vector<char>& buffer) {
            std::cout.write(buffer.data(), static_cast<std::streamsize>(buffer.size()));
        }
        /**
         * @brief Writes @p count characters from @p buffer starting at @p index.
         *
         * C++ counterpart of .NET Console.Write(char[], int, int) (via TextWriter.Write).
         * @param buffer Source vector.
         * @param index  Zero-based start index.
         * @param count  Number of characters to write.
         * @throws System::ArgumentOutOfRangeException if @p index or @p count is negative.
         * @throws System::ArgumentException if @p index and @p count do not describe a valid
         *         range within @p buffer.
         */
        static void Write(const std::vector<char>& buffer, intcs index, intcs count) {
            if (index < 0) throw System::ArgumentOutOfRangeException("index", "Non-negative number required.");
            if (count < 0) throw System::ArgumentOutOfRangeException("count", "Non-negative number required.");
            if (static_cast<intcs>(buffer.size()) - index < count)
                throw System::ArgumentException("Offset and length were out of bounds for the array or count is greater than the number of elements from index to the end of the source collection.");
            if (count > 0)
                std::cout.write(buffer.data() + index, static_cast<std::streamsize>(count));
        }

        // -----------------------------------------------------------------------
        // WriteLine
        // -----------------------------------------------------------------------

        /** @brief Writes the current line terminator to the standard output stream. */
        static void WriteLine()                      { std::cout << NewLine; }
        /** @brief Writes the specified string followed by a line terminator. */
        static void WriteLine(const std::string& v)  { std::cout << v << NewLine; }
        /** @brief Writes the specified C-string followed by a line terminator. */
        static void WriteLine(const char* v)         { std::cout << v << NewLine; }
        /** @brief Writes the specified character followed by a line terminator. */
        static void WriteLine(char v)                { std::cout << v << NewLine; }
        /** @brief Writes the specified integer followed by a line terminator. */
        static void WriteLine(intcs v)               { std::cout << v << NewLine; }
        /** @brief Writes the specified 64-bit integer followed by a line terminator. */
        static void WriteLine(longcs v)              { std::cout << v << NewLine; }
        /** @brief Writes the specified double followed by a line terminator. */
        static void WriteLine(double v) { Write(v); std::cout << NewLine; }
        /** @brief Writes the specified float followed by a line terminator. */
        static void WriteLine(float v)  { Write(v); std::cout << NewLine; }
        /** @brief Writes the specified Boolean followed by a line terminator. */
        static void WriteLine(bool v)    { std::cout << (v ? "True" : "False") << NewLine; }
        /** @brief Writes the specified unsigned 32-bit integer followed by a line terminator. */
        static void WriteLine(uintcs v)  { std::cout << v << NewLine; }
        /** @brief Writes the specified unsigned 64-bit integer followed by a line terminator. */
        static void WriteLine(ulongcs v) { std::cout << v << NewLine; }
        /**
         * @brief Writes all characters in a vector followed by a line terminator.
         * @param buffer Vector of characters to write.
         */
        static void WriteLine(const std::vector<char>& buffer) {
            Write(buffer);
            std::cout << NewLine;
        }

        // -----------------------------------------------------------------------
        // Write (format)
        // -----------------------------------------------------------------------

        /** @brief Writes a composite format string with one integer argument. */
        static void Write(const std::string& format, intcs arg0)
            { Write(System::String::Format(format, arg0)); }
        /** @brief Writes a composite format string with one double argument. */
        static void Write(const std::string& format, double arg0)
            { Write(System::String::Format(format, arg0)); }
        /** @brief Writes a composite format string with one string argument. */
        static void Write(const std::string& format, const std::string& arg0)
            { Write(System::String::Format(format, arg0)); }
        /** @brief Writes a composite format string with two integer arguments. */
        static void Write(const std::string& format, intcs arg0, intcs arg1)
            { Write(System::String::Format(format, arg0, arg1)); }
        /** @brief Writes a composite format string with two string arguments. */
        static void Write(const std::string& format, const std::string& arg0, const std::string& arg1)
            { Write(System::String::Format(format, arg0, arg1)); }

        // -----------------------------------------------------------------------
        // WriteLine (format)
        // -----------------------------------------------------------------------

        /** @brief Writes a composite format string with one integer argument, then a line terminator. */
        static void WriteLine(const std::string& format, intcs arg0)
            { WriteLine(System::String::Format(format, arg0)); }
        /** @brief Writes a composite format string with one double argument, then a line terminator. */
        static void WriteLine(const std::string& format, double arg0)
            { WriteLine(System::String::Format(format, arg0)); }
        /** @brief Writes a composite format string with one string argument, then a line terminator. */
        static void WriteLine(const std::string& format, const std::string& arg0)
            { WriteLine(System::String::Format(format, arg0)); }
        /** @brief Writes a composite format string with two integer arguments, then a line terminator. */
        static void WriteLine(const std::string& format, intcs arg0, intcs arg1)
            { WriteLine(System::String::Format(format, arg0, arg1)); }
        /** @brief Writes a composite format string with two string arguments, then a line terminator. */
        static void WriteLine(const std::string& format, const std::string& arg0, const std::string& arg1)
            { WriteLine(System::String::Format(format, arg0, arg1)); }

        // -----------------------------------------------------------------------
        // Error stream
        // -----------------------------------------------------------------------

        /** @brief Writes the specified string to the standard error stream. */
        static void Error_Write(const std::string& v)     { std::cerr << v; }
        /** @brief Writes the specified string followed by a line terminator to the standard error stream. */
        static void Error_WriteLine(const std::string& v) { std::cerr << v << NewLine; }

        // -----------------------------------------------------------------------
        // Input
        // -----------------------------------------------------------------------

        /**
         * @brief Reads the next line of characters from the standard input stream.
         * @return The next line, or an empty string at end-of-file.
         */
        [[nodiscard]] static std::string ReadLine() {
            std::string line;
            std::getline(std::cin, line);
            return line;
        }

        /**
         * @brief Reads the next character from the standard input stream.
         * @return The next character as an int, or -1 at end-of-stream.
         */
        [[nodiscard]] static intcs Read() { return std::cin.get(); }

        /**
         * @brief Reads the next key pressed from the console.
         *
         * Stub — reads a character from stdin and wraps it in a ConsoleKeyInfo.
         * @param intercept If true the key is not echoed (not enforced in this impl).
         * @return A ConsoleKeyInfo describing the pressed key.
         */
        [[nodiscard]] static ConsoleKeyInfo ReadKey(bool intercept = false) {
            (void)intercept;
            int ch = std::cin.get();
            if (ch == EOF) return ConsoleKeyInfo{};
            return ConsoleKeyInfo{
                static_cast<char>(ch),
                ConsoleKey::None,
                false, false, false
            };
        }

        /**
         * @brief Gets a value indicating whether a key press is available in the input stream.
         *
         * Always returns false in this implementation (stdin peeking is platform-specific).
         * @return false (stub).
         */
        [[nodiscard]] static bool getKeyAvailableProperty() { return false; }

        // -----------------------------------------------------------------------
        // Title
        // -----------------------------------------------------------------------

        /**
         * @brief Gets the title of the console window.
         * @return The stored title string (set via setTitleProperty).
         */
        [[nodiscard]] static const std::string& getTitleProperty() { return title_; }

        /**
         * @brief Sets the title of the console window.
         * On POSIX terminals this emits an ANSI/xterm OSC escape sequence.
         * @param title The new title string.
         */
        static void setTitleProperty(const std::string& title) {
            title_ = title;
#ifndef _WIN32
            std::cout << "\033]0;" << title << "\007" << std::flush;
#endif
        }

        // -----------------------------------------------------------------------
        // Redirection detection
        // -----------------------------------------------------------------------

        /**
         * @brief Gets a value indicating whether standard input has been redirected.
         * @return true if stdin is not a terminal.
         */
        [[nodiscard]] static bool getIsInputRedirectedProperty();

        /**
         * @brief Gets a value indicating whether standard output has been redirected.
         * @return true if stdout is not a terminal.
         */
        [[nodiscard]] static bool getIsOutputRedirectedProperty();

        /**
         * @brief Gets a value indicating whether standard error has been redirected.
         * @return true if stderr is not a terminal.
         */
        [[nodiscard]] static bool getIsErrorRedirectedProperty();

        // -----------------------------------------------------------------------
        // Input control
        // -----------------------------------------------------------------------

        /**
         * @brief Gets a value indicating whether CTRL+C is treated as ordinary input.
         * @return The stored value (not enforced in this implementation).
         */
        [[nodiscard]] static bool getTreatControlCAsInputProperty() { return treatCtrlCAsInput_; }
        /** @brief Sets whether CTRL+C should be treated as ordinary input (stored but not enforced). */
        static void setTreatControlCAsInputProperty(bool v) { treatCtrlCAsInput_ = v; }

        /**
         * @brief Stub: registers a handler for the CancelKeyPress event.
         *
         * The handler is stored but not automatically invoked in this implementation.
         * @param handler The ConsoleCancelEventHandler to register.
         */
        static void addCancelKeyPressHandler(ConsoleCancelEventHandler handler) {
            cancelKeyPressHandler_ = std::move(handler);
        }

        // -----------------------------------------------------------------------
        // Color
        // -----------------------------------------------------------------------

        /** @brief Gets the foreground color of the console. */
        [[nodiscard]] static ConsoleColor getForegroundColorProperty() { return fg_; }
        /** @brief Gets the background color of the console. */
        [[nodiscard]] static ConsoleColor getBackgroundColorProperty() { return bg_; }

        /**
         * @brief Sets the foreground color using ANSI escape codes.
         * @param color The ConsoleColor to set as foreground.
         */
        static void setForegroundColorProperty(ConsoleColor color) {
            fg_ = color;
            std::cout << ansiColor(static_cast<int>(color), true);
        }

        /**
         * @brief Sets the background color using ANSI escape codes.
         * @param color The ConsoleColor to set as background.
         */
        static void setBackgroundColorProperty(ConsoleColor color) {
            bg_ = color;
            std::cout << ansiColor(static_cast<int>(color), false);
        }

        /** @brief Resets foreground and background colors to their defaults. */
        static void ResetColor() {
            fg_ = ConsoleColor::White;
            bg_ = ConsoleColor::Black;
            std::cout << "\033[0m";
        }

        // -----------------------------------------------------------------------
        // Cursor / Window
        // -----------------------------------------------------------------------

        /**
         * @brief Sets the cursor position using an ANSI escape sequence (0-based).
         * @param left Column index (0-based).
         * @param top  Row index (0-based).
         */
        static void SetCursorPosition(intcs left, intcs top) {
            cursorLeft_ = left;
            cursorTop_  = top;
            std::printf("\033[%d;%dH", top + 1, left + 1);
        }

        /**
         * @brief Returns the current cursor position as (Left, Top).
         *
         * @note Status: PARTIAL. Unlike real .NET (which queries the terminal's actual
         *   cursor position via a DSR ANSI escape sequence on POSIX), this returns a local
         *   cache that is only updated by SetCursorPosition()/setCursorLeftProperty()/
         *   setCursorTopProperty()/Clear() calls made through this API. Writing text via
         *   Write()/WriteLine() (including text that wraps or contains newlines) moves the
         *   real terminal cursor without updating this cache, so the two can silently
         *   diverge -- do not rely on this for anything that needs the true on-screen
         *   position after writing output.
         * @return std::pair where first=Left, second=Top.
         */
        [[nodiscard]] static std::pair<intcs, intcs> GetCursorPosition() {
            return { cursorLeft_, cursorTop_ };
        }

        /** @brief Clears the console screen using an ANSI escape sequence. */
        static void Clear() { std::cout << "\033[2J\033[1;1H"; cursorLeft_ = 0; cursorTop_ = 0; }

        // -----------------------------------------------------------------------
        // Cursor properties
        // -----------------------------------------------------------------------

        /**
         * @brief Gets the cursor's current column.
         * @note Status: PARTIAL -- see GetCursorPosition()'s doc-comment; this is the same
         *   locally-cached value, not a live terminal query.
         */
        [[nodiscard]] static intcs getCursorLeftProperty() { return cursorLeft_; }
        /** @brief Sets the cursor's column position. */
        static void setCursorLeftProperty(intcs v) { SetCursorPosition(v, cursorTop_); }

        /**
         * @brief Gets the cursor's current row.
         * @note Status: PARTIAL -- see GetCursorPosition()'s doc-comment; this is the same
         *   locally-cached value, not a live terminal query.
         */
        [[nodiscard]] static intcs getCursorTopProperty() { return cursorTop_; }
        /** @brief Sets the cursor's row position. */
        static void setCursorTopProperty(intcs v) { SetCursorPosition(cursorLeft_, v); }

        /**
         * @brief Gets the height of the cursor within a character cell (1–100 percent).
         * @return The stored cursor size (default 25).
         */
        [[nodiscard]] static intcs getCursorSizeProperty() { return cursorSize_; }
        /** @brief Sets the cursor size (stored; not visually applied in this implementation). */
        static void setCursorSizeProperty(intcs v) { cursorSize_ = v; }

        /**
         * @brief Gets a value indicating whether the cursor is visible.
         * @return The stored visibility flag.
         */
        [[nodiscard]] static bool getCursorVisibleProperty() { return cursorVisible_; }
        /**
         * @brief Sets whether the cursor is visible using an ANSI escape sequence.
         * @param v true to show the cursor; false to hide it.
         */
        static void setCursorVisibleProperty(bool v) {
            cursorVisible_ = v;
            std::cout << (v ? "\033[?25h" : "\033[?25l") << std::flush;
        }

        // -----------------------------------------------------------------------
        // Window properties
        // -----------------------------------------------------------------------

        /**
         * @brief Gets the width of the console window in columns.
         * @return The real terminal width when standard output is a TTY (queried via
         *         ioctl(TIOCGWINSZ) on POSIX, GetConsoleScreenBufferInfo on Windows);
         *         80 as a fallback when output is redirected or the query fails.
         */
        [[nodiscard]] static intcs getWindowWidthProperty();

        /**
         * @brief Gets the height of the console window in rows.
         * @return The real terminal height when standard output is a TTY (queried via
         *         ioctl(TIOCGWINSZ) on POSIX, GetConsoleScreenBufferInfo on Windows);
         *         24 as a fallback when output is redirected or the query fails.
         */
        [[nodiscard]] static intcs getWindowHeightProperty();

        /** @brief Gets the leftmost column of the console window area (always 0). */
        [[nodiscard]] static intcs getWindowLeftProperty() { return 0; }
        /** @brief Gets the topmost row of the console window area (always 0). */
        [[nodiscard]] static intcs getWindowTopProperty()  { return 0; }

        /**
         * @brief Gets the largest possible window width on the current display.
         * @return Terminal query result, or 80 as fallback.
         */
        [[nodiscard]] static intcs getLargestWindowWidthProperty()  { return getWindowWidthProperty(); }
        /**
         * @brief Gets the largest possible window height on the current display.
         * @return Terminal query result, or 24 as fallback.
         */
        [[nodiscard]] static intcs getLargestWindowHeightProperty() { return getWindowHeightProperty(); }

        /**
         * @brief Sets the size of the console window using an xterm OSC escape sequence.
         * @param width  New window width in columns.
         * @param height New window height in rows.
         */
        static void SetWindowSize(intcs width, intcs height) {
            std::printf("\033[8;%d;%dt", static_cast<int>(height), static_cast<int>(width));
            std::fflush(stdout);
        }

        /**
         * @brief Sets the position of the console window using an xterm OSC escape sequence.
         * @param left New left position in pixels.
         * @param top  New top position in pixels.
         */
        static void SetWindowPosition(intcs left, intcs top) {
            std::printf("\033[3;%d;%dt", static_cast<int>(top), static_cast<int>(left));
            std::fflush(stdout);
        }

        // -----------------------------------------------------------------------
        // Buffer properties (stubs)
        // -----------------------------------------------------------------------

        /**
         * @brief Gets the width of the console screen buffer in columns.
         * @return Same as window width.
         */
        [[nodiscard]] static intcs getBufferWidthProperty()  { return getWindowWidthProperty(); }
        /**
         * @brief Gets the height of the console screen buffer in rows.
         * @return Same as window height.
         */
        [[nodiscard]] static intcs getBufferHeightProperty() { return getWindowHeightProperty(); }

        /**
         * @brief Sets the screen buffer width (no-op in this implementation).
         * @param v New buffer width.
         */
        static void setBufferWidthProperty(intcs v)  { (void)v; }
        /**
         * @brief Sets the screen buffer height (no-op in this implementation).
         * @param v New buffer height.
         */
        static void setBufferHeightProperty(intcs v) { (void)v; }

        /**
         * @brief Sets the screen buffer size (no-op in this implementation).
         * @param width  New buffer width.
         * @param height New buffer height.
         */
        static void SetBufferSize(intcs width, intcs height) { (void)width; (void)height; }

        /**
         * @brief Copies a rectangular region of the buffer to another location (no-op stub).
         */
        static void MoveBufferArea(intcs sourceLeft, intcs sourceTop,
                                    intcs sourceWidth, intcs sourceHeight,
                                    intcs targetLeft, intcs targetTop) {
            (void)sourceLeft; (void)sourceTop; (void)sourceWidth;
            (void)sourceHeight; (void)targetLeft; (void)targetTop;
        }

        /**
         * @brief Copies a rectangular region of the buffer, filling vacated cells (no-op stub).
         */
        static void MoveBufferArea(intcs sourceLeft, intcs sourceTop,
                                    intcs sourceWidth, intcs sourceHeight,
                                    intcs targetLeft, intcs targetTop,
                                    char sourceChar,
                                    ConsoleColor sourceForeColor,
                                    ConsoleColor sourceBackColor) {
            (void)sourceLeft; (void)sourceTop; (void)sourceWidth;
            (void)sourceHeight; (void)targetLeft; (void)targetTop;
            (void)sourceChar; (void)sourceForeColor; (void)sourceBackColor;
        }

        // -----------------------------------------------------------------------
        // Keyboard state (stubs)
        // -----------------------------------------------------------------------

        /**
         * @brief Gets a value indicating whether the CAPS LOCK key is toggled on.
         * @return false (not queryable portably).
         */
        [[nodiscard]] static bool getCapsLockProperty()   { return false; }
        /**
         * @brief Gets a value indicating whether the NUM LOCK key is toggled on.
         * @return false (not queryable portably).
         */
        [[nodiscard]] static bool getNumberLockProperty() { return false; }

        // -----------------------------------------------------------------------
        // CancelKeyPress event
        // -----------------------------------------------------------------------

        /**
         * @brief Removes the registered CancelKeyPress handler (sets it to null).
         */
        static void removeCancelKeyPressHandler() { cancelKeyPressHandler_ = nullptr; }

        // -----------------------------------------------------------------------
        // Beep
        // -----------------------------------------------------------------------

        /** @brief Produces a simple console beep via the BEL character. */
        static void Beep() { std::cout << '\a' << std::flush; }

        /**
         * @brief Produces a beep of the specified frequency and duration.
         *
         * Frequency and duration are ignored in this implementation — falls back to
         * a simple BEL character, which is the only portable option.
         * @param frequency Tone frequency in hertz (ignored).
         * @param duration  Duration in milliseconds (ignored).
         */
        static void Beep(intcs frequency, intcs duration) {
            (void)frequency; (void)duration;
            std::cout << '\a' << std::flush;
        }

    private:
        static inline ConsoleColor fg_              = ConsoleColor::White;
        static inline ConsoleColor bg_              = ConsoleColor::Black;
        static inline std::string  title_;
        static inline bool         cursorVisible_   = true;
        static inline bool         treatCtrlCAsInput_ = false;
        static inline ConsoleCancelEventHandler cancelKeyPressHandler_;
        static inline intcs cursorLeft_  = 0;
        static inline intcs cursorTop_   = 0;
        static inline intcs cursorSize_  = 25;

        static std::string ansiColor(int c, bool fg) {
            char buf[12];
            if (c < 8)
                std::snprintf(buf, sizeof(buf), "\033[%dm", fg ? 30 + c : 40 + c);
            else
                std::snprintf(buf, sizeof(buf), "\033[%dm", fg ? 90 + (c - 8) : 100 + (c - 8));
            return buf;
        }
    };

} // namespace System

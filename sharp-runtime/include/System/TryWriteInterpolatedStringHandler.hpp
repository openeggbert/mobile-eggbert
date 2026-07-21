// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <cstring>
#include <string>
#include <type_traits>
#include "System/Span.hpp"

namespace System {

    /**
     * @brief Provides a handler for formatting interpolated strings into a character span.
     *
     * C++ counterpart of .NET System.MemoryExtensions.TryWriteInterpolatedStringHandler.
     * In .NET this is a compiler-generated ref struct; here it is a regular class used
     * manually to build formatted output into a fixed-size char buffer.
     *
     * Usage:
     * @code
     *   char buf[64]; std::size_t written = 0;
     *   TryWriteInterpolatedStringHandler h(buf, sizeof(buf));
     *   h.AppendLiteral("x=");
     *   h.AppendFormatted(42);
     *   written = h.getCharsWrittenProperty();  // "x=42" in buf
     * @endcode
     */
    class TryWriteInterpolatedStringHandler {
    public:
        /**
         * @brief Constructs a handler that writes into the given character buffer.
         * @param destination Pointer to the output buffer.
         * @param destLen     Maximum number of characters the buffer can hold (excluding NUL).
         * @param literalLength Hint: total characters expected from literals (used to pre-check).
         * @param shouldAppend On return, true if the buffer is large enough for the literal hint.
         */
        TryWriteInterpolatedStringHandler(char* destination, std::size_t destLen,
                                          std::size_t literalLength, bool& shouldAppend)
            : dest_(destination), destLen_(destLen), pos_(0), success_(true) {
            shouldAppend = success_ = (destLen >= literalLength);
        }

        /**
         * @brief Constructs a handler without a literal-length pre-check.
         * @param destination Pointer to the output buffer.
         * @param destLen     Maximum number of characters the buffer can hold.
         */
        TryWriteInterpolatedStringHandler(char* destination, std::size_t destLen)
            : dest_(destination), destLen_(destLen), pos_(0), success_(true) {}

        // ---------------------------------------------------------------

        /**
         * @brief Appends a string literal into the buffer.
         * @param value The literal to append.
         * @return true if the value fit; false if the buffer is full.
         */
        bool AppendLiteral(const std::string& value) {
            return appendRaw(value.data(), value.size());
        }

        /** @brief C-string overload; see the std::string overload above. */
        bool AppendLiteral(const char* value) {
            return appendRaw(value, std::strlen(value));
        }

        /**
         * @brief Appends a formatted value into the buffer.
         * Converts via std::to_string (numeric types) or falls back to
         * the value's ToString() if it inherits from IFormattable.
         */
        template<typename T>
        bool AppendFormatted(const T& value) {
            return AppendLiteral(formatValue(value));
        }

        /**
         * @brief Appends a formatted value with an explicit format string.
         * Format string is currently ignored (stub — passes through to AppendFormatted<T>).
         */
        template<typename T>
        bool AppendFormatted(const T& value, const std::string& /*format*/) {
            return AppendFormatted(value);
        }

        // ---------------------------------------------------------------

        /** @brief Returns the number of characters written so far. */
        [[nodiscard]] std::size_t getCharsWrittenProperty() const { return pos_; }

        /** @brief Returns true if all append operations succeeded. */
        [[nodiscard]] bool getSuccessProperty() const { return success_; }

        /** @brief Returns a std::string containing the characters written so far. */
        [[nodiscard]] std::string getString() const {
            return success_ ? std::string(dest_, pos_) : std::string{};
        }

    private:
        char*       dest_;
        std::size_t destLen_;
        std::size_t pos_;
        bool        success_;

        bool appendRaw(const char* data, std::size_t len) {
            if (!success_) return false;
            if (pos_ + len > destLen_) { success_ = false; return false; }
            std::memcpy(dest_ + pos_, data, len);
            pos_ += len;
            return true;
        }

        template<typename T>
        static std::string formatValue(const T& v) {
            if constexpr (std::is_same_v<T, std::string>) return v;
            else if constexpr (std::is_same_v<T, const char*>) return std::string(v);
            else if constexpr (std::is_same_v<T, char>) return std::string(1, v);
            else if constexpr (std::is_arithmetic_v<T>) return std::to_string(v);
            else return std::string("[?]");
        }
    };

} // namespace System

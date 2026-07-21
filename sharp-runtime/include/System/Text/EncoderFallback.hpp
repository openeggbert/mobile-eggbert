// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>
#include <string>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Text {

    class EncoderFallbackBuffer;

    /**
     * @brief Provides a basic fallback strategy for handling characters that an Encoder cannot
     * convert to a byte sequence.
     *
     * C++ counterpart of .NET System.Text.EncoderFallback.
     */
    class EncoderFallback {
    public:
        virtual ~EncoderFallback() = default;

        /** @brief Creates a new fallback buffer for use during a single encode operation. */
        [[nodiscard]] virtual std::unique_ptr<EncoderFallbackBuffer> CreateFallbackBuffer() const = 0;

        /** Returns the singleton replacement fallback (substitutes '?'). */
        static std::shared_ptr<EncoderFallback> ReplacementFallback();
    };

    /**
     * @brief Represents a substitute input character sequence used when the original character
     * cannot be encoded.
     *
     * C++ counterpart of .NET System.Text.EncoderFallbackBuffer.
     */
    class EncoderFallbackBuffer {
    protected:
        std::string fallbackString_;
        size_t position_ = 0;

    public:
        virtual ~EncoderFallbackBuffer() = default;

        /** @brief Prepares the fallback buffer to substitute for the given unencodable character. */
        virtual bool Fallback(char charUnknown, SharpRuntime::intcs index) = 0;

        /** @return The next character in the fallback replacement, or '\0' if none remain. */
        virtual char GetNextChar() {
            if (position_ >= fallbackString_.size()) return '\0';
            return fallbackString_[position_++];
        }

        /** @return The number of characters remaining in the fallback buffer. */
        [[nodiscard]] virtual SharpRuntime::intcs getRemainingProperty() const {
            return static_cast<SharpRuntime::intcs>(fallbackString_.size() - position_);
        }
    };

    /** @brief Encoder fallback that substitutes a replacement byte sequence for unencodable characters. */
    class EncoderReplacementFallback : public EncoderFallback {
        std::string replacement_;
    public:
        /** Constructs the fallback with the given replacement string (default "?"). */
        explicit EncoderReplacementFallback(const std::string& replacement = "?") : replacement_(replacement) {}
        /** Gets the default replacement string used by this fallback. */
        [[nodiscard]] const std::string& getDefaultStringProperty() const { return replacement_; }
        [[nodiscard]] std::unique_ptr<EncoderFallbackBuffer> CreateFallbackBuffer() const override;
    };

    /** @brief Fallback buffer that substitutes EncoderReplacementFallback's replacement string. */
    class EncoderReplacementFallbackBuffer : public EncoderFallbackBuffer {
    public:
        /** @brief Initializes the buffer with @p fallback's replacement string. */
        explicit EncoderReplacementFallbackBuffer(const EncoderReplacementFallback& fallback) {
            fallbackString_ = fallback.getDefaultStringProperty();
        }
        /** @brief Prepares the replacement string to be read back; returns true if it is non-empty. */
        bool Fallback(char, SharpRuntime::intcs) override {
            position_ = 0;
            return !fallbackString_.empty();
        }
    };

    inline std::unique_ptr<EncoderFallbackBuffer> EncoderReplacementFallback::CreateFallbackBuffer() const {
        return std::make_unique<EncoderReplacementFallbackBuffer>(*this);
    }

    inline std::shared_ptr<EncoderFallback> EncoderFallback::ReplacementFallback() {
        static auto inst = std::make_shared<EncoderReplacementFallback>("?");
        return inst;
    }

} // namespace System::Text

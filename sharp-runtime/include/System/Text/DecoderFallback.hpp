// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>
#include <string>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Text {

    class DecoderFallbackBuffer;

    /**
     * @brief Provides a basic fallback strategy for handling byte sequences that a Decoder
     * cannot convert to characters.
     *
     * C++ counterpart of .NET System.Text.DecoderFallback.
     */
    class DecoderFallback {
    public:
        virtual ~DecoderFallback() = default;

        /** @brief Creates a new fallback buffer for use during a single decode operation. */
        [[nodiscard]] virtual std::unique_ptr<DecoderFallbackBuffer> CreateFallbackBuffer() const = 0;

        /** Returns the singleton replacement fallback (substitutes '?'). */
        static std::shared_ptr<DecoderFallback> ReplacementFallback();
    };

    /**
     * @brief Represents a substitute input string that is used when the original input byte
     * sequence cannot be decoded.
     *
     * C++ counterpart of .NET System.Text.DecoderFallbackBuffer. Operates on the UTF-8-encoded
     * bytes of the replacement string (this runtime represents .NET `char`/`string` as UTF-8
     * `std::string` throughout, not UTF-16), one byte per GetNextChar() call.
     */
    class DecoderFallbackBuffer {
    protected:
        std::string fallbackString_;
        size_t position_ = 0;

    public:
        virtual ~DecoderFallbackBuffer() = default;

        /** @brief Prepares the fallback buffer to substitute for the given undecodable byte sequence. */
        virtual bool Fallback(const std::vector<SharpRuntime::bytecs>& bytesUnknown, SharpRuntime::intcs index) = 0;

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

    /** @brief Decoder fallback that substitutes a replacement string for undecodable bytes. */
    class DecoderReplacementFallback : public DecoderFallback {
        std::string replacement_;
    public:
        /** Constructs the fallback with the given replacement string (default "?"). */
        explicit DecoderReplacementFallback(const std::string& replacement = "?") : replacement_(replacement) {}
        /** Gets the default replacement string used by this fallback. */
        [[nodiscard]] const std::string& getDefaultStringProperty() const { return replacement_; }
        [[nodiscard]] std::unique_ptr<DecoderFallbackBuffer> CreateFallbackBuffer() const override;
    };

    /** @brief Fallback buffer that substitutes DecoderReplacementFallback's replacement string. */
    class DecoderReplacementFallbackBuffer : public DecoderFallbackBuffer {
    public:
        /** @brief Initializes the buffer with @p fallback's replacement string. */
        explicit DecoderReplacementFallbackBuffer(const DecoderReplacementFallback& fallback) {
            fallbackString_ = fallback.getDefaultStringProperty();
        }
        /** @brief Prepares the replacement string to be read back; returns true if it is non-empty. */
        bool Fallback(const std::vector<SharpRuntime::bytecs>&, SharpRuntime::intcs) override {
            position_ = 0;
            return !fallbackString_.empty();
        }
    };

    inline std::unique_ptr<DecoderFallbackBuffer> DecoderReplacementFallback::CreateFallbackBuffer() const {
        return std::make_unique<DecoderReplacementFallbackBuffer>(*this);
    }

    inline std::shared_ptr<DecoderFallback> DecoderFallback::ReplacementFallback() {
        static auto inst = std::make_shared<DecoderReplacementFallback>("?");
        return inst;
    }

} // namespace System::Text

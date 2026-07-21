// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>
#include <string>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentException.hpp"

namespace System::Text {

    /**
     * @brief Thrown when a character cannot be encoded and the encoder is configured to throw
     * rather than substitute.
     *
     * C++ counterpart of .NET System.Text.EncoderFallbackException.
     *
     * @note Reduced scope: .NET's `CharUnknownHigh`/`CharUnknownLow` (for reporting an
     * unencodable UTF-16 surrogate pair) are collapsed into a single `CharUnknown`, since this
     * runtime represents characters as UTF-8 bytes/`char`, not UTF-16 code units.
     */
    class EncoderFallbackException : public System::ArgumentException {
        char charUnknown_ = '\0';
        SharpRuntime::intcs index_ = 0;

    public:
        /** @brief Initializes a new EncoderFallbackException with a default message. */
        EncoderFallbackException() : System::ArgumentException("Value does not fall within the expected range.") {}
        /** @brief Initializes a new EncoderFallbackException with the specified message. */
        explicit EncoderFallbackException(const std::string& message) : System::ArgumentException(message) {}
        /** @brief Initializes a new EncoderFallbackException with a message and an inner exception. */
        EncoderFallbackException(const std::string& message, std::exception_ptr innerException)
            : System::ArgumentException(message, innerException) {}
        /** @brief Initializes a new EncoderFallbackException with the offending character and its index. */
        EncoderFallbackException(const std::string& message, char charUnknown, SharpRuntime::intcs index)
            : System::ArgumentException(message), charUnknown_(charUnknown), index_(index) {}

        /** @return The character that could not be encoded. */
        [[nodiscard]] char getCharUnknownProperty() const { return charUnknown_; }
        /** @return The index position in the input buffer of the character that caused the exception. */
        [[nodiscard]] SharpRuntime::intcs getIndexProperty() const { return index_; }
    };

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

        /** Returns fallback bytes for a character that cannot be encoded (this runtime's simplified one-shot API). */
        [[nodiscard]] virtual std::vector<SharpRuntime::bytecs> GetFallbackBytes(char unknownChar) const = 0;
        /** Gets the maximum number of bytes produced by one fallback substitution. */
        [[nodiscard]] virtual SharpRuntime::intcs getMaxByteCountProperty() const = 0;

        /** Returns the singleton replacement fallback (substitutes '?'). */
        static std::shared_ptr<EncoderFallback> ReplacementFallback();
        /** Returns the singleton exception fallback (throws on unencodable characters). */
        static std::shared_ptr<EncoderFallback> ExceptionFallback();
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

        /** @brief Moves the internal pointer back one position. @return false if already at the start. */
        virtual bool MovePrevious() {
            if (position_ == 0) return false;
            --position_;
            return true;
        }

        /** @return The number of characters remaining in the fallback buffer. */
        [[nodiscard]] virtual SharpRuntime::intcs getRemainingProperty() const {
            return static_cast<SharpRuntime::intcs>(fallbackString_.size() - position_);
        }

        /** @brief Resets the fallback buffer to its initial (empty) state. */
        virtual void Reset() {
            fallbackString_.clear();
            position_ = 0;
        }
    };

    /** @brief Encoder fallback that substitutes a replacement byte sequence for unencodable characters. */
    class EncoderReplacementFallback : public EncoderFallback {
        std::string replacement_;
    public:
        /** Constructs the fallback with the given replacement string (default "?"). */
        explicit EncoderReplacementFallback(const std::string& replacement = "?") : replacement_(replacement) {}
        /** Returns the replacement bytes for any unencodable character. */
        [[nodiscard]] std::vector<SharpRuntime::bytecs> GetFallbackBytes(char) const override {
            return std::vector<SharpRuntime::bytecs>(replacement_.begin(), replacement_.end());
        }
        /** Gets the byte count of the replacement string. */
        [[nodiscard]] SharpRuntime::intcs getMaxByteCountProperty() const override {
            return static_cast<SharpRuntime::intcs>(replacement_.size());
        }
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

    /** @brief Encoder fallback that throws an EncoderFallbackException for unencodable characters. */
    class EncoderExceptionFallback : public EncoderFallback {
    public:
        /** Always throws EncoderFallbackException. */
        [[nodiscard]] std::vector<SharpRuntime::bytecs> GetFallbackBytes(char unknownChar) const override;
        /** Returns 0 (no bytes produced; an exception is thrown instead). */
        [[nodiscard]] SharpRuntime::intcs getMaxByteCountProperty() const override { return 0; }
        [[nodiscard]] std::unique_ptr<EncoderFallbackBuffer> CreateFallbackBuffer() const override;
    };

    /** @brief Fallback buffer that always throws EncoderFallbackException. */
    class EncoderExceptionFallbackBuffer : public EncoderFallbackBuffer {
    public:
        /** @brief Always throws EncoderFallbackException describing the unencodable character. */
        bool Fallback(char charUnknown, SharpRuntime::intcs index) override;
        /** @brief Never reached (Fallback always throws); returns '\0'. */
        char GetNextChar() override { return '\0'; }
        /** @brief Never reached (Fallback always throws); returns false. */
        bool MovePrevious() override { return false; }
        /** @return 0; no bytes are ever queued since Fallback always throws. */
        [[nodiscard]] SharpRuntime::intcs getRemainingProperty() const override { return 0; }
    };

    inline std::unique_ptr<EncoderFallbackBuffer> EncoderExceptionFallback::CreateFallbackBuffer() const {
        return std::make_unique<EncoderExceptionFallbackBuffer>();
    }

    inline bool EncoderExceptionFallbackBuffer::Fallback(char charUnknown, SharpRuntime::intcs index) {
        throw EncoderFallbackException("Unable to translate character '" + std::string(1, charUnknown) +
                                            "' at index " + std::to_string(index) + " to the specified code page.",
                                        charUnknown, index);
    }

    inline std::vector<SharpRuntime::bytecs> EncoderExceptionFallback::GetFallbackBytes(char unknownChar) const {
        EncoderExceptionFallbackBuffer buffer;
        buffer.Fallback(unknownChar, 0);
        return {}; // unreachable — Fallback() always throws
    }

    inline std::shared_ptr<EncoderFallback> EncoderFallback::ReplacementFallback() {
        static auto inst = std::make_shared<EncoderReplacementFallback>("?");
        return inst;
    }
    inline std::shared_ptr<EncoderFallback> EncoderFallback::ExceptionFallback() {
        static auto inst = std::make_shared<EncoderExceptionFallback>();
        return inst;
    }

} // namespace System::Text

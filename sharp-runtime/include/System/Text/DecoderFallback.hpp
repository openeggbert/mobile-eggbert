// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <cstdio>
#include <memory>
#include <string>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentException.hpp"

namespace System::Text {

    /**
     * @brief Thrown when a byte sequence cannot be decoded and the decoder is configured to
     * throw rather than substitute.
     *
     * C++ counterpart of .NET System.Text.DecoderFallbackException.
     */
    class DecoderFallbackException : public System::ArgumentException {
        std::vector<SharpRuntime::bytecs> bytesUnknown_;
        SharpRuntime::intcs index_ = 0;

    public:
        /** @brief Initializes a new DecoderFallbackException with a default message. */
        DecoderFallbackException() : System::ArgumentException("Value does not fall within the expected range.") {}
        /** @brief Initializes a new DecoderFallbackException with the specified message. */
        explicit DecoderFallbackException(const std::string& message) : System::ArgumentException(message) {}
        /** @brief Initializes a new DecoderFallbackException with a message and an inner exception. */
        DecoderFallbackException(const std::string& message, std::exception_ptr innerException)
            : System::ArgumentException(message, innerException) {}
        /** @brief Initializes a new DecoderFallbackException with the offending byte sequence and its index. */
        DecoderFallbackException(const std::string& message, std::vector<SharpRuntime::bytecs> bytesUnknown,
                                  SharpRuntime::intcs index)
            : System::ArgumentException(message), bytesUnknown_(std::move(bytesUnknown)), index_(index) {}

        /** @return The input byte sequence that caused the exception. */
        [[nodiscard]] const std::vector<SharpRuntime::bytecs>& getBytesUnknownProperty() const { return bytesUnknown_; }
        /** @return The index position in the input buffer of the byte sequence that caused the exception. */
        [[nodiscard]] SharpRuntime::intcs getIndexProperty() const { return index_; }
    };

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

        /** Returns a fallback string for the given undecodable byte sequence (this runtime's simplified one-shot API). */
        [[nodiscard]] virtual std::string GetFallbackString(const SharpRuntime::bytecs* bytesUnknown, SharpRuntime::intcs byteCount) const = 0;
        /** Gets the maximum number of characters produced by one fallback substitution. */
        [[nodiscard]] virtual SharpRuntime::intcs getMaxCharCountProperty() const = 0;

        /** Returns the singleton replacement fallback (substitutes '?'). */
        static std::shared_ptr<DecoderFallback> ReplacementFallback();
        /** Returns the singleton exception fallback (throws on bad bytes). */
        static std::shared_ptr<DecoderFallback> ExceptionFallback();
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

    /** @brief Decoder fallback that substitutes a replacement string for undecodable bytes. */
    class DecoderReplacementFallback : public DecoderFallback {
        std::string replacement_;
    public:
        /** Constructs the fallback with the given replacement string (default "?"). */
        explicit DecoderReplacementFallback(const std::string& replacement = "?") : replacement_(replacement) {}
        /** Returns the replacement string for any undecodable byte sequence. */
        [[nodiscard]] std::string GetFallbackString(const SharpRuntime::bytecs*, SharpRuntime::intcs) const override { return replacement_; }
        /** Gets the length of the replacement string. */
        [[nodiscard]] SharpRuntime::intcs getMaxCharCountProperty() const override { return static_cast<SharpRuntime::intcs>(replacement_.size()); }
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

    /** @brief Decoder fallback that throws a DecoderFallbackException for undecodable bytes. */
    class DecoderExceptionFallback : public DecoderFallback {
    public:
        /** Always throws DecoderFallbackException. */
        [[nodiscard]] std::string GetFallbackString(const SharpRuntime::bytecs* bytesUnknown, SharpRuntime::intcs byteCount) const override;
        /** Returns 0 (no characters produced; an exception is thrown instead). */
        [[nodiscard]] SharpRuntime::intcs getMaxCharCountProperty() const override { return 0; }
        [[nodiscard]] std::unique_ptr<DecoderFallbackBuffer> CreateFallbackBuffer() const override;
    };

    /** @brief Fallback buffer that always throws DecoderFallbackException. */
    class DecoderExceptionFallbackBuffer : public DecoderFallbackBuffer {
    public:
        /** @brief Always throws DecoderFallbackException describing the undecodable bytes. */
        bool Fallback(const std::vector<SharpRuntime::bytecs>& bytesUnknown, SharpRuntime::intcs index) override;
        /** @brief Never reached (Fallback always throws); returns '\0'. */
        char GetNextChar() override { return '\0'; }
        /** @brief Never reached (Fallback always throws); returns false. */
        bool MovePrevious() override { return false; }
        /** @return 0; no characters are ever queued since Fallback always throws. */
        [[nodiscard]] SharpRuntime::intcs getRemainingProperty() const override { return 0; }
    };

    inline std::unique_ptr<DecoderFallbackBuffer> DecoderExceptionFallback::CreateFallbackBuffer() const {
        return std::make_unique<DecoderExceptionFallbackBuffer>();
    }

    inline bool DecoderExceptionFallbackBuffer::Fallback(const std::vector<SharpRuntime::bytecs>& bytesUnknown, SharpRuntime::intcs index) {
        std::string hex;
        for (size_t i = 0; i < bytesUnknown.size() && i < 20; ++i) {
            char buf[8];
            std::snprintf(buf, sizeof(buf), "[%02X]", bytesUnknown[i]);
            hex += buf;
        }
        if (bytesUnknown.size() > 20) hex += " ...";
        throw DecoderFallbackException("Unable to translate bytes " + hex + " at index " + std::to_string(index) +
                                            " from the specified code page to Unicode.",
                                        bytesUnknown, index);
    }

    inline std::string DecoderExceptionFallback::GetFallbackString(const SharpRuntime::bytecs* bytesUnknown, SharpRuntime::intcs byteCount) const {
        std::vector<SharpRuntime::bytecs> bytes(bytesUnknown, bytesUnknown + byteCount);
        DecoderExceptionFallbackBuffer buffer;
        buffer.Fallback(bytes, 0);
        return {}; // unreachable — Fallback() always throws
    }

    inline std::shared_ptr<DecoderFallback> DecoderFallback::ReplacementFallback() {
        static auto inst = std::make_shared<DecoderReplacementFallback>("?");
        return inst;
    }
    inline std::shared_ptr<DecoderFallback> DecoderFallback::ExceptionFallback() {
        static auto inst = std::make_shared<DecoderExceptionFallback>();
        return inst;
    }

} // namespace System::Text

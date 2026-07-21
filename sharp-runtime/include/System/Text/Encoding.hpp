// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <memory>
#include <string>
#include <vector>

#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Text/DecoderFallback.hpp"
#include "System/Text/EncoderFallback.hpp"

namespace System::Text
{
    /**
     * <summary>
     * Represents a character encoding.
     *
     * Abstract base class; use the UTF8() factory method to obtain a concrete
     * instance.
     * </summary>
     */
    class Encoding
    {
        std::shared_ptr<DecoderFallback> decoderFallback_ = DecoderFallback::ReplacementFallback();
        std::shared_ptr<EncoderFallback> encoderFallback_ = EncoderFallback::ReplacementFallback();

    public:
        virtual ~Encoding() = default;

        /** Returns a shared UTF-8 encoding instance. */
        [[nodiscard]] static std::shared_ptr<Encoding> UTF8();

        /**
         * Encodes a string to a byte vector using this encoding.
         * @param str The string to encode.
         * @return Encoded bytes.
         */
        [[nodiscard]] virtual std::vector<SharpRuntime::bytecs> GetBytes(const std::string& str) const;

        /**
         * Decodes a range of bytes to a string using this encoding.
         * @param data  Pointer to the byte buffer.
         * @param index Start index within @p data.
         * @param count Number of bytes to decode.
         * @return Decoded string.
         */
        [[nodiscard]] virtual std::string GetString(
            const SharpRuntime::bytecs* data,
            SharpRuntime::intcs index,
            SharpRuntime::intcs count) const;

        /** Gets the DecoderFallback used when a byte sequence cannot be decoded. */
        [[nodiscard]] std::shared_ptr<DecoderFallback> getDecoderFallbackProperty() const { return decoderFallback_; }
        /** Sets the DecoderFallback used when a byte sequence cannot be decoded. */
        void setDecoderFallbackProperty(std::shared_ptr<DecoderFallback> value) { decoderFallback_ = std::move(value); }

        /** Gets the EncoderFallback used when a character cannot be encoded. */
        [[nodiscard]] std::shared_ptr<EncoderFallback> getEncoderFallbackProperty() const { return encoderFallback_; }
        /** Sets the EncoderFallback used when a character cannot be encoded. */
        void setEncoderFallbackProperty(std::shared_ptr<EncoderFallback> value) { encoderFallback_ = std::move(value); }

    protected:
        Encoding() = default;
    };
}

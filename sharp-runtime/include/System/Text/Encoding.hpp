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
     * Abstract base class; use UTF8(), ASCII(), or Unicode() factory methods
     * to obtain a concrete instance.
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

        /** Returns a shared ASCII encoding instance. */
        [[nodiscard]] static std::shared_ptr<Encoding> ASCII();

        /** Returns a shared UTF-16 LE (Unicode) encoding instance. */
        [[nodiscard]] static std::shared_ptr<Encoding> Unicode();

        /** Returns a shared big-endian UTF-16 encoding instance. */
        [[nodiscard]] static std::shared_ptr<Encoding> BigEndianUnicode();

        /** Returns a shared UTF-32 LE encoding instance. */
        [[nodiscard]] static std::shared_ptr<Encoding> UTF32();

        /** Returns a shared UTF-7 encoding instance. */
        [[nodiscard]] static std::shared_ptr<Encoding> UTF7();

        /** Returns a shared Latin-1 (ISO-8859-1) encoding instance. */
        [[nodiscard]] static std::shared_ptr<Encoding> Latin1();

        /** Returns the default encoding for this runtime (UTF-8, matching .NET Core/5+). */
        [[nodiscard]] static std::shared_ptr<Encoding> Default() { return UTF8(); }

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

        /** Decodes an entire byte vector to a string using this encoding. */
        [[nodiscard]] std::string GetString(const std::vector<SharpRuntime::bytecs>& bytes) const {
            return GetString(bytes.data(), 0, static_cast<SharpRuntime::intcs>(bytes.size()));
        }

        /** Returns the number of bytes that GetBytes() would produce for @p str, without allocating them. */
        [[nodiscard]] virtual SharpRuntime::intcs GetByteCount(const std::string& str) const {
            return static_cast<SharpRuntime::intcs>(GetBytes(str).size());
        }

        /** Returns the number of characters that GetString() would produce for the given byte range. */
        [[nodiscard]] virtual SharpRuntime::intcs GetCharCount(const SharpRuntime::bytecs* data, SharpRuntime::intcs index,
                                                                SharpRuntime::intcs count) const {
            return static_cast<SharpRuntime::intcs>(GetString(data, index, count).size());
        }

        /** Gets the IANA name of this encoding (e.g. "utf-8", "us-ascii"). */
        [[nodiscard]] virtual std::string getEncodingNameProperty() const { return "utf-8"; }

        /** Gets the code page identifier for this encoding (e.g. 65001 for UTF-8). */
        [[nodiscard]] virtual int getCodePageProperty() const { return 65001; }

        /** Gets the human-readable name of this encoding (defaults to the IANA name). */
        [[nodiscard]] virtual std::string getWebNameProperty() const { return getEncodingNameProperty(); }

        /** Returns true if this encoding uses exactly one byte per character (e.g. ASCII, Latin-1). */
        [[nodiscard]] virtual bool getIsSingleByteProperty() const { return false; }

        /** Gets the DecoderFallback used when a byte sequence cannot be decoded. */
        [[nodiscard]] std::shared_ptr<DecoderFallback> getDecoderFallbackProperty() const { return decoderFallback_; }
        /** Sets the DecoderFallback used when a byte sequence cannot be decoded. */
        void setDecoderFallbackProperty(std::shared_ptr<DecoderFallback> value) { decoderFallback_ = std::move(value); }

        /** Gets the EncoderFallback used when a character cannot be encoded. */
        [[nodiscard]] std::shared_ptr<EncoderFallback> getEncoderFallbackProperty() const { return encoderFallback_; }
        /** Sets the EncoderFallback used when a character cannot be encoded. */
        void setEncoderFallbackProperty(std::shared_ptr<EncoderFallback> value) { encoderFallback_ = std::move(value); }

        /** Returns true if @p other has the same code page as this encoding. */
        [[nodiscard]] virtual bool Equals(const Encoding& other) const { return getCodePageProperty() == other.getCodePageProperty(); }
        bool operator==(const Encoding& o) const { return Equals(o); }
        bool operator!=(const Encoding& o) const { return !Equals(o); }

        /** @return A hash code derived from the code page identifier. */
        [[nodiscard]] virtual SharpRuntime::intcs GetHashCode() const { return getCodePageProperty(); }

    protected:
        Encoding() = default;
    };
}

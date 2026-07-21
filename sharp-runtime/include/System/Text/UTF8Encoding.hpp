// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include "System/Text/Encoding.hpp"

namespace System::Text {

    /**
     * @brief Represents a UTF-8 encoding of Unicode characters.
     *
     * Partial C++ counterpart of .NET System.Text.UTF8Encoding.
     *
     * @note Status: Implemented. GetBytes()/GetString() validate that the input is well-formed
     * UTF-8 and route ill-formed byte sequences through getEncoderFallbackProperty()/
     * getDecoderFallbackProperty() (one byte substituted/thrown per invalid position, then
     * resuming) instead of passing bytes through unvalidated. The default fallback (set by the
     * constructor, overridable via setDecoderFallbackProperty()/setEncoderFallbackProperty())
     * replaces with U+FFFD, matching real UTF8Encoding.SetDefaultFallbacks() -- not the generic
     * Encoding base class's "?" default, which is correct only for single-byte code pages.
     */
    class UTF8Encoding : public Encoding {
    public:
        /** Default constructor; sets the U+FFFD replacement fallback .NET uses for UTF-8. */
        UTF8Encoding();
        ~UTF8Encoding() override = default;

        /** Encodes a string to a UTF-8 byte sequence. */
        [[nodiscard]] std::vector<SharpRuntime::bytecs> GetBytes(const std::string& str) const override;
        /** Decodes a UTF-8 byte range to a string. */
        [[nodiscard]] std::string GetString(const SharpRuntime::bytecs* data,
                                            SharpRuntime::intcs index,
                                            SharpRuntime::intcs count) const override;
        /** Returns the encoding name "utf-8". */
        [[nodiscard]] std::string getEncodingNameProperty() const override { return "utf-8"; }
    };

} // namespace System::Text

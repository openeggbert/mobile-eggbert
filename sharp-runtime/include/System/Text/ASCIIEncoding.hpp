// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include "System/Text/Encoding.hpp"

namespace System::Text {

    /**
     * @brief Represents the ASCII character encoding.
     * Characters outside 0–127 are replaced by '?'.
     *
     * Partial C++ counterpart of .NET System.Text.ASCIIEncoding.
     *
     * @note Status: Implemented
     */
    class ASCIIEncoding : public Encoding {
    public:
        /** Default constructor. */
        ASCIIEncoding() = default;
        ~ASCIIEncoding() override = default;

        /** Encodes a string to ASCII bytes; characters above 127 are replaced by '?'. */
        [[nodiscard]] std::vector<SharpRuntime::bytecs> GetBytes(const std::string& str) const override;
        /** Decodes an ASCII byte range to a string. */
        [[nodiscard]] std::string GetString(const SharpRuntime::bytecs* data,
                                            SharpRuntime::intcs index,
                                            SharpRuntime::intcs count) const override;
        /** Returns the encoding name "us-ascii". */
        [[nodiscard]] std::string getEncodingNameProperty() const override { return "us-ascii"; }
        /** Returns the code page 20127 (US-ASCII). */
        [[nodiscard]] int getCodePageProperty() const override { return 20127; }
        /** ASCII always uses exactly one byte per character. */
        [[nodiscard]] bool getIsSingleByteProperty() const override { return true; }
    };

} // namespace System::Text

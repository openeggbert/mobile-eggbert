// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include <vector>
#include "System/Text/Encoding.hpp"

namespace System::Text {

    /**
     * Converts a set of characters into a sequence of bytes using a given Encoding.
     * 
     * Wraps an Encoding instance. Partial C++ counterpart of .NET System.Text.Encoder.
     * 
     * @note Status: Partial — stateless; no support for split surrogate pairs across calls.
     */
    class Encoder {
        std::shared_ptr<Encoding> encoding_;
    public:
        /** @param encoding The encoding used to convert characters to bytes. */
        explicit Encoder(std::shared_ptr<Encoding> encoding) : encoding_(std::move(encoding)) {}

        /**
         * Encodes the string @p s into a byte sequence.
         * @return The encoded bytes.
         */
        [[nodiscard]] std::vector<SharpRuntime::bytecs> GetBytes(const std::string& s) const {
            return encoding_->GetBytes(s);
        }

        /** Returns the number of bytes needed to encode @p s. */
        [[nodiscard]] SharpRuntime::intcs GetByteCount(const std::string& s) const {
            return static_cast<SharpRuntime::intcs>(encoding_->GetBytes(s).size());
        }

        /** Resets encoder state (no-op for stateless encoding). */
        void Reset() {}
    };

} // namespace System::Text

// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include <vector>
#include "System/Text/Encoding.hpp"

namespace System::Text {

    /**
     * @brief Converts a sequence of encoded bytes into a set of characters.
     *
     * Wraps an Encoding instance. Partial C++ counterpart of .NET System.Text.Decoder.
     *
     * @note Status: Partial — stateless; no support for split multi-byte sequences across calls.
     */
    class Decoder {
        std::shared_ptr<Encoding> encoding_;
    public:
        /** Constructs a Decoder wrapping the given Encoding. */
        explicit Decoder(std::shared_ptr<Encoding> encoding) : encoding_(std::move(encoding)) {}

        /** @brief Decodes a byte buffer into a string. */
        [[nodiscard]] std::string GetString(const SharpRuntime::bytecs* bytes,
                                            SharpRuntime::intcs         index,
                                            SharpRuntime::intcs         count) const {
            return encoding_->GetString(bytes, index, count);
        }

        /** Decodes a byte vector to a string, with optional offset and count. */
        [[nodiscard]] std::string GetString(const std::vector<SharpRuntime::bytecs>& bytes,
                                            SharpRuntime::intcs index = 0,
                                            SharpRuntime::intcs count = -1) const {
            SharpRuntime::intcs len = (count < 0) ? static_cast<SharpRuntime::intcs>(bytes.size()) - index : count;
            return encoding_->GetString(bytes.data(), index, len);
        }

        /** @brief Resets decoder state (no-op for stateless encoding). */
        void Reset() {}
    };

} // namespace System::Text

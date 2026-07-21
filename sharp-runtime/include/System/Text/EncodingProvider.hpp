// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>
#include <string>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Text/Encoding.hpp"
#include "System/Text/EncodingInfo.hpp"

namespace System::Text {

    using SharpRuntime::intcs;

    /**
     * @brief Provides the base class for encoding providers that supply encodings unavailable in
     * a base runtime implementation.
     *
     * C++ counterpart of .NET System.Text.EncodingProvider.
     *
     * @note The global registration API (`Encoding.RegisterProvider`) is not reproduced — this
     * runtime's `Encoding` factory methods (`UTF8()`, `ASCII()`, etc.) don't consult a provider
     * registry, since the one real-world provider consumer, `CodePagesEncodingProvider`, is
     * itself out of scope here (see that type's own note).
     */
    class EncodingProvider {
    public:
        virtual ~EncodingProvider() = default;
        /** Returns an encoding for the given code-page identifier, or nullptr if unknown. */
        [[nodiscard]] virtual std::shared_ptr<Encoding> GetEncoding(intcs codepage) = 0;
        /** Returns an encoding for the given encoding name, or nullptr if unknown. */
        [[nodiscard]] virtual std::shared_ptr<Encoding> GetEncoding(const std::string& name) = 0;
        /** Returns the encodings known to this provider (default: none). */
        [[nodiscard]] virtual std::vector<EncodingInfo> GetEncodings() { return {}; }
    };

} // namespace System::Text

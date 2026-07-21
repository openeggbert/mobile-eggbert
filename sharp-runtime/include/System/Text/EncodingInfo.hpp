// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>
#include <string>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Text/Encoding.hpp"

namespace System::Text {

    /** Provides information about a character encoding (code page, name, display name). */
    class EncodingInfo {
        SharpRuntime::intcs codePage_;
        std::string name_;
        std::string displayName_;

    public:
        /** Constructs an EncodingInfo with the given code-page identifier, IANA name, and display name. */
        EncodingInfo(SharpRuntime::intcs codePage, std::string name, std::string displayName)
            : codePage_(codePage), name_(std::move(name)), displayName_(std::move(displayName)) {}

        /** Gets the code-page identifier for this encoding. */
        [[nodiscard]] SharpRuntime::intcs getCodePageProperty() const { return codePage_; }
        /** Gets the IANA name of this encoding. */
        [[nodiscard]] const std::string& getNameProperty() const { return name_; }
        /** Gets the human-readable name of this encoding. */
        [[nodiscard]] const std::string& getDisplayNameProperty() const { return displayName_; }

        /**
         * @brief Returns an Encoding object for this EncodingInfo.
         *
         * Reduced scope: always returns UTF-8 regardless of getCodePageProperty(). Real .NET
         * resolves this via a global code-page table (Encoding.GetEncoding(int)) populated by
         * every registered EncodingProvider; this runtime has no such registry (see
         * EncodingProvider's own class doc-comment) and no EncodingInfo instances are actually
         * constructed anywhere in this codebase today, so there is no live caller depending on
         * per-code-page resolution here. Wiring this up properly would need the same code-page
         * table EncodingProvider already defers.
         */
        [[nodiscard]] std::shared_ptr<Encoding> GetEncoding() const {
            return Encoding::UTF8();
        }
    };

} // namespace System::Text

// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <vector>
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Text/RegularExpressions/Capture.hpp"

namespace System::Text::RegularExpressions {

    /**
     * @brief Represents the set of captures made by a single capturing group.
     *
     * C++ counterpart of .NET System.Text.RegularExpressions.CaptureCollection.
     *
     * @note Since this runtime's Regex is `std::regex`-backed, it has no visibility into
     * intermediate captures made by a quantified group (e.g. `(?:\w+,)+` capturing each `\w+`
     * on every iteration) — only the group's final match is available. This collection therefore
     * always has exactly zero (group didn't participate) or one (group matched) elements, never
     * more, unlike .NET's real regex engine.
     */
    class CaptureCollection {
        std::vector<Capture> captures_;

    public:
        CaptureCollection() = default;
        explicit CaptureCollection(std::vector<Capture> captures) : captures_(std::move(captures)) {}

        /** @return The number of captures. */
        [[nodiscard]] intcs getCountProperty() const { return static_cast<intcs>(captures_.size()); }

        /** @return The capture at @p index. @throws System::ArgumentOutOfRangeException if out of range. */
        [[nodiscard]] const Capture& operator[](intcs index) const {
            if (index < 0 || index >= static_cast<intcs>(captures_.size())) {
                throw System::ArgumentOutOfRangeException("index");
            }
            return captures_[static_cast<size_t>(index)];
        }

        [[nodiscard]] auto begin() const { return captures_.begin(); }
        [[nodiscard]] auto end() const { return captures_.end(); }
    };

} // namespace System::Text::RegularExpressions

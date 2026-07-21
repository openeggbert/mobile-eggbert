// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/Text/RegularExpressions/Capture.hpp"
#include "System/Text/RegularExpressions/CaptureCollection.hpp"

namespace System::Text::RegularExpressions {

    /**
     * @brief Represents the results from a single capturing group.
     *
     * C++ counterpart of .NET System.Text.RegularExpressions.Group.
     */
    class Group : public Capture {
        bool success_ = false;
        std::string name_;
        CaptureCollection captures_;

    public:
        Group() = default;

        Group(std::string value, intcs index, intcs length, bool success, std::string name)
            : Capture(value, success ? index : 0, success ? length : 0), success_(success), name_(std::move(name)) {
            if (success_) {
                captures_ = CaptureCollection({Capture(value_, index_, length_)});
            }
        }

        /** @return true if this group participated in the match. */
        [[nodiscard]] bool getSuccessProperty() const { return success_; }
        /** @return The name of this capturing group (its number, as a string, if unnamed). */
        [[nodiscard]] const std::string& getNameProperty() const { return name_; }
        /** @return All captures made by this group (see CaptureCollection's note on reduced scope). */
        [[nodiscard]] const CaptureCollection& getCapturesProperty() const { return captures_; }

        /** @return An empty, unsuccessful Group. */
        [[nodiscard]] static Group Empty() { return Group("", 0, 0, false, ""); }
    };

} // namespace System::Text::RegularExpressions

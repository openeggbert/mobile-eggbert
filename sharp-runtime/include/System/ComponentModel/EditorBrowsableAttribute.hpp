// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::ComponentModel {

    enum class EditorBrowsableState {
        Always   = 0,
        Never    = 1,
        Advanced = 2,
    };

    class EditorBrowsableAttribute {
        EditorBrowsableState state_;
    public:
        explicit EditorBrowsableAttribute(EditorBrowsableState state = EditorBrowsableState::Always)
            : state_(state) {}

        [[nodiscard]] EditorBrowsableState getStateProperty() const { return state_; }
    };

} // namespace System::ComponentModel

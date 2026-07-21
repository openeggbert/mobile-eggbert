// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Attribute.hpp"

namespace System::Diagnostics {

    /**
     * @brief Specifies how the debugger displays a field or property.
     *
     * C++ counterpart of .NET System.Diagnostics.DebuggerBrowsableState. Note: `Expanded = 1`
     * is intentionally absent -- real .NET's enum comments it out ("Expanded is not supported
     * in this release") and its constructor-validation range check treats it as invalid,
     * which this port's validation below matches.
     */
    enum class DebuggerBrowsableState {
        Never       = 0,
        Collapsed   = 2,
        RootHidden  = 3,
    };

    /**
     * @brief Specifies how the debugger displays a field or property.
     *
     * C++ counterpart of .NET System.Diagnostics.DebuggerBrowsableAttribute.
     */
    class DebuggerBrowsableAttribute : public System::Attribute {
        DebuggerBrowsableState state_;
    public:
        /**
         * @brief Constructs a DebuggerBrowsableAttribute with the given display state.
         *
         * C++ counterpart of .NET DebuggerBrowsableAttribute(DebuggerBrowsableState).
         * @param state The display state.
         * @throws System::ArgumentOutOfRangeException if @p state is outside
         *         [Never, RootHidden] (this also rejects the removed Expanded=1 value).
         */
        explicit DebuggerBrowsableAttribute(DebuggerBrowsableState state) : state_(state) {
            if (state < DebuggerBrowsableState::Never || state > DebuggerBrowsableState::RootHidden)
                throw System::ArgumentOutOfRangeException("state");
        }
        [[nodiscard]] DebuggerBrowsableState getStateProperty() const { return state_; }
    };

} // namespace System::Diagnostics

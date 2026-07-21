// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/EventArgs.hpp"
#include "System/Xml/Linq/XObjectChange.hpp"

namespace System::Xml::Linq {

    /**
     * @brief Provides data for the (unraised, in this port — see XObject) XObject Changing/Changed events.
     *
     * C++ counterpart of .NET System.Xml.Linq.XObjectChangeEventArgs.
     */
    class XObjectChangeEventArgs : public System::EventArgs {
        XObjectChange objectChange_;

    public:
        explicit XObjectChangeEventArgs(XObjectChange objectChange) : objectChange_(objectChange) {}

        /** @return The type of change that occurred. */
        [[nodiscard]] XObjectChange getObjectChangeProperty() const { return objectChange_; }

        static const XObjectChangeEventArgs Add;
        static const XObjectChangeEventArgs Remove;
        static const XObjectChangeEventArgs Name;
        static const XObjectChangeEventArgs Value;
    };

    inline const XObjectChangeEventArgs XObjectChangeEventArgs::Add{XObjectChange::Add};
    inline const XObjectChangeEventArgs XObjectChangeEventArgs::Remove{XObjectChange::Remove};
    inline const XObjectChangeEventArgs XObjectChangeEventArgs::Name{XObjectChange::Name};
    inline const XObjectChangeEventArgs XObjectChangeEventArgs::Value{XObjectChange::Value};

} // namespace System::Xml::Linq

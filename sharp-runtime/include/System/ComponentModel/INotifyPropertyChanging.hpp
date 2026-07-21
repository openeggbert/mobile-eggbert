// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <functional>
#include <string>
#include <vector>

namespace System::ComponentModel {

    /** Event args carrying the name of the property that is about to change. */
    struct PropertyChangingEventArgs {
        std::string PropertyName; ///< The name of the property that is changing.
        /** @param name  Name of the property about to change. */
        explicit PropertyChangingEventArgs(const std::string& name) : PropertyName(name) {}
    };

    /** Delegate type for PropertyChanging event handlers. */
    using PropertyChangingEventHandler = std::function<void(void*, const PropertyChangingEventArgs&)>;

    /**
     * Notifies clients that a property value is about to change.
     * C++ counterpart of .NET System.ComponentModel.INotifyPropertyChanging.
     */
    class INotifyPropertyChanging {
    public:
        std::vector<PropertyChangingEventHandler> PropertyChanging; ///< Subscribers notified before a property changes.
        virtual ~INotifyPropertyChanging() = default;

    protected:
        /** Raises the PropertyChanging event for the property named @p name. */
        void OnPropertyChanging(const std::string& name) {
            PropertyChangingEventArgs args(name);
            for (auto& h : PropertyChanging) h(this, args);
        }
    };

} // namespace System::ComponentModel

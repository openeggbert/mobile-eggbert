// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <functional>
#include <string>
#include <vector>

namespace System::ComponentModel {

    /**
     * @brief Event args carrying the name of the property that changed.
     *
     * Partial C++ counterpart of .NET System.ComponentModel.PropertyChangedEventArgs.
     *
     * @note Status: Implemented
     */
    struct PropertyChangedEventArgs {
        std::string PropertyName;
        explicit PropertyChangedEventArgs(const std::string& name) : PropertyName(name) {}
    };

    using PropertyChangedEventHandler = std::function<void(void*, const PropertyChangedEventArgs&)>;

    /**
     * @brief Notifies clients that a property value has changed.
     *
     * Partial C++ counterpart of .NET System.ComponentModel.INotifyPropertyChanged.
     *
     * @note Status: Implemented
     */
    class INotifyPropertyChanged {
    public:
        virtual ~INotifyPropertyChanged() = default;

        std::vector<PropertyChangedEventHandler> PropertyChanged; ///< Subscribers notified after a property changes.

    protected:
        /** Raises the PropertyChanged event for the property named @p propertyName. */
        void OnPropertyChanged(const std::string& propertyName) {
            PropertyChangedEventArgs args(propertyName);
            for (auto& h : PropertyChanged) h(this, args);
        }
    };

} // namespace System::ComponentModel

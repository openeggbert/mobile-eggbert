// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::ComponentModel
{
    /**
     * @brief Represents a collection of PropertyDescriptor objects.
     *
     * Minimal stub — C++ counterpart of System.ComponentModel.PropertyDescriptorCollection.
     * Provided for API surface compatibility with design-time XNA helpers (e.g. MathTypeConverter).
     */
    class PropertyDescriptorCollection
    {
    public:
        PropertyDescriptorCollection() = default;
        virtual ~PropertyDescriptorCollection() = default;

        /** Returns the number of property descriptors in the collection. */
        [[nodiscard]] virtual int getCountProperty() const { return 0; }
    };
}

// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/Attribute.hpp"

namespace System::ComponentModel {

    /**
     * @brief Specifies a description for a property or event.
     *
     * C++ counterpart of .NET System.ComponentModel.DescriptionAttribute. Stores text only, with
     * no reflection integration (no reflection-driven consumer reads this attribute anywhere in
     * this runtime) -- but the value-holding surface itself (default value, value equality,
     * IsDefaultAttribute) is fully implemented, matching the real .NET type's non-reflection
     * members exactly.
     */
    class DescriptionAttribute : public System::Attribute {
        std::string description_;
    public:
        /** Constructs the attribute with the default description (empty string). */
        DescriptionAttribute() : description_() {}

        /** @param description The description text to store. */
        explicit DescriptionAttribute(const std::string& description) : description_(description) {}

        /** @return The stored description text. */
        [[nodiscard]] virtual const std::string& getDescriptionProperty() const { return description_; }

        /** Pre-built instance holding the default (empty-string) description. */
        static const DescriptionAttribute Default;

        /** @return true if @p other is a DescriptionAttribute with the same description text. */
        bool Equals(const System::Attribute& other) const override {
            const auto* o = dynamic_cast<const DescriptionAttribute*>(&other);
            return o != nullptr && o->description_ == description_;
        }

        /** @return A hash code derived from the stored description text. */
        [[nodiscard]] int GetHashCode() const override {
            return static_cast<int>(std::hash<std::string>{}(description_));
        }

        /** @return true if this attribute's description equals the default (empty string). */
        [[nodiscard]] bool getIsDefaultAttributeProperty() const override { return Equals(Default); }
    };

    inline const DescriptionAttribute DescriptionAttribute::Default{};

} // namespace System::ComponentModel

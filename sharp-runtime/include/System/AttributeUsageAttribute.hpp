// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Attribute.hpp"
#include "System/AttributeTargets.hpp"

namespace System {

/**
 * @brief Specifies the usage of another attribute class.
 *
 * C++ counterpart of .NET System.AttributeUsageAttribute.
 * Applied to a custom attribute class to control which program elements
 * the attribute can be applied to, whether it may be used more than once
 * on the same element, and whether it is inherited by derived classes.
 */
class AttributeUsageAttribute : public Attribute {
    AttributeTargets validOn_;
    bool             allowMultiple_ = false;
    bool             inherited_     = true;

public:
    /**
     * @brief Default AttributeUsageAttribute targeting AttributeTargets::All.
     *
     * C++ counterpart of .NET AttributeUsageAttribute.Default (internal).
     */
    static const AttributeUsageAttribute Default;

    /**
     * @brief Initializes a new instance specifying the valid target elements.
     * @param validOn The set of application elements to which the attribute can be applied.
     */
    explicit AttributeUsageAttribute(AttributeTargets validOn)
        : validOn_(validOn), allowMultiple_(false), inherited_(true) {}

    /**
     * @brief Initializes a new instance with all properties specified.
     *
     * C++ counterpart of .NET's internal AttributeUsageAttribute(AttributeTargets, bool, bool).
     * @param validOn       The set of application elements to which the attribute can be applied.
     * @param allowMultiple Whether more than one instance is allowed on a single element.
     * @param inherited     Whether the attribute is inherited by derived classes.
     */
    AttributeUsageAttribute(AttributeTargets validOn, bool allowMultiple, bool inherited)
        : validOn_(validOn), allowMultiple_(allowMultiple), inherited_(inherited) {}

    // ---- Getters ----

    /**
     * @brief Gets the set of application elements on which the attribute is valid.
     *
     * C++ counterpart of .NET AttributeUsageAttribute.ValidOn.
     */
    [[nodiscard]] AttributeTargets getValidOnProperty()      const { return validOn_; }

    /**
     * @brief Gets a value indicating whether more than one instance of the attribute
     * is allowed on a single element.
     *
     * C++ counterpart of .NET AttributeUsageAttribute.AllowMultiple.
     */
    [[nodiscard]] bool             getAllowMultipleProperty() const { return allowMultiple_; }

    /**
     * @brief Gets a value indicating whether the attribute is inherited by derived classes.
     *
     * C++ counterpart of .NET AttributeUsageAttribute.Inherited.
     */
    [[nodiscard]] bool             getInheritedProperty()    const { return inherited_; }

    // ---- Setters ----

    /**
     * @brief Sets whether more than one instance of the attribute is allowed on a single element.
     *
     * C++ counterpart of the setter of .NET AttributeUsageAttribute.AllowMultiple.
     */
    void setAllowMultipleProperty(bool v) { allowMultiple_ = v; }

    /**
     * @brief Sets whether the attribute is inherited by derived classes.
     *
     * C++ counterpart of the setter of .NET AttributeUsageAttribute.Inherited.
     */
    void setInheritedProperty(bool v)     { inherited_ = v; }
};

} // namespace System

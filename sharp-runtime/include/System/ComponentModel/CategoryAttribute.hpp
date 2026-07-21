// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/Attribute.hpp"

namespace System::ComponentModel {

    /** Specifies the category in which to group a property or event in a property grid. */
    class CategoryAttribute : public System::Attribute {
        std::string category_;
    public:
        /** Constructs the attribute with the default category "Misc". */
        CategoryAttribute() : category_("Misc") {}

        /** Constructs the attribute with the given category name. */
        explicit CategoryAttribute(const std::string& category) : category_(category) {}

        /** @return The category name. */
        [[nodiscard]] const std::string& getCategoryProperty() const { return category_; }
    };

    /** Specifies whether a property or event should be displayed in a property grid. */
    class BrowsableAttribute : public System::Attribute {
    public:
        bool Browsable; ///< True if the member should be shown in property browsers.

        /** @param browsable True to show the member in property browsers. */
        explicit BrowsableAttribute(bool browsable) : Browsable(browsable) {}

        static const BrowsableAttribute Yes; ///< Pre-built "browsable = true" instance.
        static const BrowsableAttribute No;  ///< Pre-built "browsable = false" instance.
    };
    inline const BrowsableAttribute BrowsableAttribute::Yes{true};
    inline const BrowsableAttribute BrowsableAttribute::No{false};

    /** Specifies whether the property it is bound to is read-only. */
    class ReadOnlyAttribute : public System::Attribute {
    public:
        bool IsReadOnly; ///< True if the property is read-only.

        /** @param isReadOnly True to mark the property as read-only. */
        explicit ReadOnlyAttribute(bool isReadOnly) : IsReadOnly(isReadOnly) {}

        static const ReadOnlyAttribute Yes; ///< Pre-built "read-only = true" instance.
        static const ReadOnlyAttribute No;  ///< Pre-built "read-only = false" instance.
    };
    inline const ReadOnlyAttribute ReadOnlyAttribute::Yes{true};
    inline const ReadOnlyAttribute ReadOnlyAttribute::No{false};

    /** Specifies the display name for a property or event. */
    class DisplayNameAttribute : public System::Attribute {
        std::string displayName_;
    public:
        /** Constructs the attribute with an empty display name. */
        DisplayNameAttribute() = default;

        /** Constructs the attribute with the given display name. */
        explicit DisplayNameAttribute(const std::string& displayName) : displayName_(displayName) {}

        /** @return The display name string. */
        [[nodiscard]] const std::string& getDisplayNameProperty() const { return displayName_; }
    };

    /** Specifies that the decorated class is immutable. */
    class ImmutableObjectAttribute : public System::Attribute {
    public:
        bool Immutable; ///< True if the object is immutable.

        /** @param immutable True to mark the object as immutable. */
        explicit ImmutableObjectAttribute(bool immutable) : Immutable(immutable) {}
    };

    /** Specifies whether a property should be localised. */
    class LocalizableAttribute : public System::Attribute {
    public:
        bool IsLocalizable; ///< True if the property value should be localised.

        /** @param localizable True to indicate the property is localizable. */
        explicit LocalizableAttribute(bool localizable) : IsLocalizable(localizable) {}
    };

    /** Specifies whether the list value of this property can be merged into a single object. */
    class MergablePropertyAttribute : public System::Attribute {
    public:
        bool AllowMerge; ///< True if property values can be merged.

        /** @param allowMerge True to allow merging. */
        explicit MergablePropertyAttribute(bool allowMerge) : AllowMerge(allowMerge) {}
    };

    /** Specifies that changing this property should trigger re-querying the parent property. */
    class NotifyParentPropertyAttribute : public System::Attribute {
    public:
        bool NotifyParent; ///< True if the parent property should be notified.

        /** @param notifyParent True to enable parent notification. */
        explicit NotifyParentPropertyAttribute(bool notifyParent) : NotifyParent(notifyParent) {}
    };

    /** Specifies how the property grid refreshes when the decorated property changes. */
    class RefreshPropertiesAttribute : public System::Attribute {
    public:
        /** Controls how the property grid refreshes. */
        enum class Refresh { None = 0, All = 1, Repaint = 2 };

        Refresh RefreshProperties_; ///< The refresh mode.

        /** @param r The desired refresh mode. */
        explicit RefreshPropertiesAttribute(Refresh r) : RefreshProperties_(r) {}
    };

    /** Specifies the type converter to use for a property. */
    class TypeConverterAttribute : public System::Attribute {
        std::string typeName_;
    public:
        /** Constructs the attribute with an empty type name. */
        TypeConverterAttribute() = default;

        /** @param typeName Fully qualified name of the converter type. */
        explicit TypeConverterAttribute(const std::string& typeName) : typeName_(typeName) {}

        /** @return The fully qualified converter type name. */
        [[nodiscard]] const std::string& getConverterTypeNameProperty() const { return typeName_; }
    };

    /** Specifies the designer for a class. */
    class DesignerAttribute : public System::Attribute {
        std::string designerTypeName_;
    public:
        /** @param designerTypeName Fully qualified name of the designer type. */
        explicit DesignerAttribute(const std::string& designerTypeName) : designerTypeName_(designerTypeName) {}

        /** @return The fully qualified designer type name. */
        [[nodiscard]] const std::string& getDesignerTypeNameProperty() const { return designerTypeName_; }
    };

    /** Specifies that the decorated class is usable only in design mode. */
    class DesignOnlyAttribute : public System::Attribute {
    public:
        bool IsDesignOnly; ///< True if the component is design-time only.

        /** @param designOnly True to restrict usage to design time. */
        explicit DesignOnlyAttribute(bool designOnly) : IsDesignOnly(designOnly) {}
    };

} // namespace System::ComponentModel

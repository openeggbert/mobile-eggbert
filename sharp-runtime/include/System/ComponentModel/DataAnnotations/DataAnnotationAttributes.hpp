// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Attribute.hpp"

namespace System::ComponentModel::DataAnnotations {

    using SharpRuntime::intcs;

    /** Base class for validation attributes. */
    class ValidationAttribute : public System::Attribute {
    protected:
        std::string errorMessage_; ///< The validation error message template.
    public:
        /** Constructs the attribute with an empty error message. */
        ValidationAttribute() = default;

        /** @param errorMessage The error message returned on validation failure. */
        explicit ValidationAttribute(const std::string& errorMessage) : errorMessage_(errorMessage) {}

        /** @return The validation error message. */
        [[nodiscard]] const std::string& getErrorMessageProperty() const { return errorMessage_; }

        /** Sets the validation error message. */
        void setErrorMessageProperty(const std::string& v) { errorMessage_ = v; }
    };

    /** Specifies that a data field value is required. */
    class RequiredAttribute : public ValidationAttribute {
    public:
        bool AllowEmptyStrings = false; ///< If true, empty strings are accepted.

        /** Default constructor. */
        RequiredAttribute() = default;

        /** @param errorMessage Custom error message on validation failure. */
        explicit RequiredAttribute(const std::string& errorMessage) : ValidationAttribute(errorMessage) {}
    };

    /** Specifies the numeric range constraints for a data field. */
    class RangeAttribute : public ValidationAttribute {
        double minimum_;
        double maximum_;
    public:
        /**
         * @param min Minimum allowed value.
         * @param max Maximum allowed value.
         */
        RangeAttribute(double min, double max) : minimum_(min), maximum_(max) {}

        /** Integer overload — @param min and @p max are promoted to double. */
        RangeAttribute(intcs min, intcs max) : minimum_(static_cast<double>(min)), maximum_(static_cast<double>(max)) {}

        /** @return The minimum allowed value. */
        [[nodiscard]] double getMinimumProperty() const { return minimum_; }

        /** @return The maximum allowed value. */
        [[nodiscard]] double getMaximumProperty() const { return maximum_; }
    };

    /** Specifies the maximum and minimum string length. */
    class StringLengthAttribute : public ValidationAttribute {
        intcs maximumLength_;
        intcs minimumLength_ = 0;
    public:
        /** @param maximumLength Maximum allowed string length. */
        explicit StringLengthAttribute(intcs maximumLength) : maximumLength_(maximumLength) {}

        /** @return The maximum allowed string length. */
        [[nodiscard]] intcs getMaximumLengthProperty() const { return maximumLength_; }

        /** @return The minimum allowed string length. */
        [[nodiscard]] intcs getMinimumLengthProperty() const { return minimumLength_; }

        /** Sets the minimum allowed string length. */
        void setMinimumLengthProperty(intcs v) { minimumLength_ = v; }
    };

    /** Specifies the maximum length of array or string data. */
    class MaxLengthAttribute : public ValidationAttribute {
        intcs length_;
    public:
        /** @param length Maximum length; -1 means unlimited. */
        explicit MaxLengthAttribute(intcs length = -1) : length_(length) {}

        /** @return The maximum allowed length. */
        [[nodiscard]] intcs getLengthProperty() const { return length_; }
    };

    /** Specifies the minimum length of array or string data. */
    class MinLengthAttribute : public ValidationAttribute {
        intcs length_;
    public:
        /** @param length Minimum required length. */
        explicit MinLengthAttribute(intcs length) : length_(length) {}

        /** @return The minimum required length. */
        [[nodiscard]] intcs getLengthProperty() const { return length_; }
    };

    /** Specifies that a data field value must match the given regular expression. */
    class RegularExpressionAttribute : public ValidationAttribute {
        std::string pattern_;
    public:
        /** @param pattern The regular expression pattern. */
        explicit RegularExpressionAttribute(const std::string& pattern) : pattern_(pattern) {}

        /** @return The regular expression pattern. */
        [[nodiscard]] const std::string& getPatternProperty() const { return pattern_; }
    };

    /** Denotes that a data field value is a well-formed email address. */
    class EmailAddressAttribute : public ValidationAttribute {};
    /** Denotes that a data field value is a phone number. */
    class PhoneAttribute        : public ValidationAttribute {};
    /** Denotes that a data field value is a well-formed URL. */
    class UrlAttribute          : public ValidationAttribute {};
    /** Denotes that a data field value is a credit card number. */
    class CreditCardAttribute   : public ValidationAttribute {};

    /** Provides general-purpose metadata for display in UI scaffolding. */
    class DisplayAttribute : public System::Attribute {
    public:
        std::string Name;               ///< Display name.
        std::string Description;        ///< Description text.
        std::string Prompt;             ///< Watermark / prompt text.
        std::string GroupName;          ///< Group the field belongs to.
        std::string ShortName;          ///< Abbreviated display name.
        intcs Order  = 0;                ///< Display order.
        bool AutoGenerateField  = true; ///< Whether to auto-generate a field for this property.
        bool AutoGenerateFilter = true; ///< Whether to auto-generate a filter for this property.
    };

    /** Denotes one or more properties that uniquely identify an entity. */
    class KeyAttribute          : public System::Attribute {};

    /** Specifies whether a class or data column is excluded from scaffolding. */
    class ScaffoldColumnAttribute : public System::Attribute {
    public:
        bool Scaffold; ///< True if the column should be included in scaffolding.

        /** @param scaffold True to include the column in scaffolding. */
        explicit ScaffoldColumnAttribute(bool scaffold) : Scaffold(scaffold) {}
    };

    /** Specifies the data type to associate with a data field. */
    class DataTypeAttribute : public System::Attribute {
    public:
        std::string DataType; ///< The data type name.

        /** @param dt The data type name string. */
        explicit DataTypeAttribute(const std::string& dt) : DataType(dt) {}
    };

    /** Specifies that a data field value must match the value of another property. */
    class CompareAttribute : public ValidationAttribute {
        std::string otherProperty_;
    public:
        /** @param otherProperty Name of the property to compare against. */
        explicit CompareAttribute(const std::string& otherProperty) : otherProperty_(otherProperty) {}

        /** @return The name of the comparison target property. */
        [[nodiscard]] const std::string& getOtherPropertyProperty() const { return otherProperty_; }
    };

} // namespace System::ComponentModel::DataAnnotations

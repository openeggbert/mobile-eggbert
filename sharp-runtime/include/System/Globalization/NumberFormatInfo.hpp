// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/Globalization/DigitShapes.hpp"

namespace System::Globalization {

    using SharpRuntime::intcs;

/**
 * @brief Provides culture-specific information for formatting and parsing numeric values.
 *
 * C++ counterpart of .NET System.Globalization.NumberFormatInfo.
 * The default constructor initializes invariant (locale-independent) defaults.
 * Only the invariant format is meaningfully implemented; locale-specific formatting
 * is not supported.
 */
class NumberFormatInfo {
    bool isReadOnly_ = false;

    std::string numberDecimalSeparator_ = ".";
    std::string numberGroupSeparator_   = ",";
    intcs numberDecimalDigits_          = 2;
    intcs numberNegativePattern_        = 1;
    std::vector<intcs> numberGroupSizes_ = {3};

    std::string currencyDecimalSeparator_ = ".";
    std::string currencyGroupSeparator_   = ",";
    // U+00A4 (CURRENCY SIGN), matching .NET's real invariant default
    // (NumberFormatInfo.cs: `_currencySymbol = "\x00a4"`) -- not "$", despite what that
    // file's own prose doc-comment table says; the field initializer is ground truth.
    std::string currencySymbol_           = "¤";
    intcs currencyDecimalDigits_          = 2;
    intcs currencyNegativePattern_        = 0;
    intcs currencyPositivePattern_        = 0;
    std::vector<intcs> currencyGroupSizes_ = {3};

    std::string negativeSign_           = "-";
    std::string positiveSign_           = "+";
    std::string naNSymbol_              = "NaN";
    std::string positiveInfinitySymbol_ = "Infinity";
    std::string negativeInfinitySymbol_ = "-Infinity";

    std::string percentSymbol_           = "%";
    std::string perMilleSymbol_          = "\xe2\x80\xb0"; // ‰
    std::string percentDecimalSeparator_ = ".";
    std::string percentGroupSeparator_   = ",";
    intcs percentDecimalDigits_          = 2;
    intcs percentNegativePattern_        = 0;
    intcs percentPositivePattern_        = 0;
    std::vector<intcs> percentGroupSizes_ = {3};

    DigitShapes digitSubstitution_ = DigitShapes::None;
    std::vector<std::string> nativeDigits_ = {"0","1","2","3","4","5","6","7","8","9"};

    /** @brief Throws InvalidOperationException if this instance is read-only. */
    void VerifyWritable() const {
        if (isReadOnly_) throw System::InvalidOperationException("Instance is read-only.");
    }

    /** @brief Throws ArgumentOutOfRangeException if @p value is outside [@p min, @p max]. */
    static void CheckRange(int value, int min, int max) {
        if (value < min || value > max)
            throw System::ArgumentOutOfRangeException("value",
                "Valid values are between " + std::to_string(min) + " and " + std::to_string(max) + ", inclusive.");
    }

    /** @brief Throws ArgumentException if @p value is empty. */
    static void CheckNotEmpty(const std::string& value) {
        if (value.empty())
            throw System::ArgumentException("String cannot be empty.", "value");
    }

    /**
     * @brief Verifies a group-size array: every element must be between 1 and 9,
     * except the last element, which may also be 0.
     *
     * C++ counterpart of .NET NumberFormatInfo.CheckGroupSize.
     */
    static void CheckGroupSize(const std::vector<int>& groupSize) {
        for (size_t i = 0; i < groupSize.size(); ++i) {
            if (groupSize[i] < 1) {
                if (i == groupSize.size() - 1 && groupSize[i] == 0) return;
                throw System::ArgumentException(
                    "Every element in the value array should be between one and nine, except for the last element, which can be zero.", "value");
            }
            if (groupSize[i] > 9)
                throw System::ArgumentException(
                    "Every element in the value array should be between one and nine, except for the last element, which can be zero.", "value");
        }
    }

    /**
     * @brief Verifies a native-digits array: must contain exactly 10 single-codepoint strings.
     *
     * C++ counterpart of .NET NumberFormatInfo.VerifyNativeDigits. .NET validates each entry
     * decodes to exactly one UTF-16 code unit or a valid surrogate pair (i.e. exactly one
     * Unicode scalar value); this port validates the UTF-8 equivalent: each entry must decode
     * to exactly one well-formed codepoint.
     */
    static void CheckNativeDigits(const std::vector<std::string>& nativeDigits) {
        static constexpr const char* kMsg =
            "Each member of the NativeDigits array must be a single text element (one or more "
            "UTF16 code points) and a supplementary character is only allowed if the "
            "corresponding digit in the Latin script, that is a digit in the range 0 through 9, "
            "is also a supplementary character.";
        if (nativeDigits.size() != 10)
            throw System::ArgumentException("The NativeDigits array must contain exactly ten members.", "value");
        for (const auto& digit : nativeDigits) {
            if (digit.empty())
                throw System::ArgumentException(kMsg, "value");
            unsigned char c0 = static_cast<unsigned char>(digit[0]);
            // Verified this is a real gap against the method's own stated intent ("must decode
            // to exactly one well-formed codepoint"): the previous check only compared the
            // total byte length against what the leading byte's value range implies, without
            // checking (a) that the leading byte is actually a valid UTF-8 lead byte -- 0x80-
            // 0xBF are continuation bytes and can never start a sequence -- or (b) that the
            // subsequent bytes are valid continuation bytes (0x80-0xBF each). Confirmed via a
            // standalone repro that both a malformed 2-byte sequence with a bad second byte
            // (e.g. {0xC2, 0x20}) and a bare continuation byte used as if it were a lead byte
            // (e.g. {0x80, 0x80}) both passed the old length-only check.
            size_t expectedLen = c0 < 0x80 ? 1 : c0 < 0xC0 ? 0 /* invalid lead byte */
                                : c0 < 0xE0 ? 2 : c0 < 0xF0 ? 3 : c0 < 0xF8 ? 4 : 0 /* invalid lead byte */;
            if (expectedLen == 0 || digit.size() != expectedLen)
                throw System::ArgumentException(kMsg, "value");
            for (size_t i = 1; i < digit.size(); ++i) {
                unsigned char ci = static_cast<unsigned char>(digit[i]);
                if (ci < 0x80 || ci > 0xBF)
                    throw System::ArgumentException(kMsg, "value");
            }
        }
    }

    /** @brief Throws ArgumentException unless @p value is a valid DigitShapes enum member. */
    static void CheckDigitSubstitution(DigitShapes value) {
        switch (value) {
            case DigitShapes::Context:
            case DigitShapes::None:
            case DigitShapes::NativeNational:
                return;
            default:
                throw System::ArgumentException("The DigitSubstitution property must be set to a valid value of the System.Globalization.DigitShapes enumeration.", "value");
        }
    }

public:
    /**
     * @brief Constructs a mutable NumberFormatInfo with invariant-culture defaults.
     *
     * C++ counterpart of .NET NumberFormatInfo().
     */
    NumberFormatInfo() = default;

    /**
     * @brief Gets a value indicating whether this instance is read-only.
     *
     * C++ counterpart of .NET NumberFormatInfo.IsReadOnly.
     * @return true if this instance is read-only; otherwise false.
     */
    [[nodiscard]] bool getIsReadOnlyProperty() const { return isReadOnly_; }

    /**
     * @brief Returns the culture-independent (invariant) NumberFormatInfo instance.
     *
     * C++ counterpart of .NET NumberFormatInfo.InvariantInfo.
     * @return A const reference to the shared read-only invariant instance.
     */
    [[nodiscard]] static const NumberFormatInfo& getInvariantInfoProperty() {
        static NumberFormatInfo info = makeReadOnly();
        return info;
    }

    /**
     * @brief Returns the NumberFormatInfo for the current thread's culture.
     *
     * C++ counterpart of .NET NumberFormatInfo.CurrentInfo.
     * Stub — returns the invariant info.
     * @return A const reference to the invariant NumberFormatInfo.
     */
    [[nodiscard]] static const NumberFormatInfo& getCurrentInfoProperty() {
        return getInvariantInfoProperty();
    }

    /**
     * @brief Returns a read-only copy of the given NumberFormatInfo.
     *
     * C++ counterpart of .NET NumberFormatInfo.ReadOnly(NumberFormatInfo).
     * @param nfi The source instance.
     * @return A read-only copy.
     */
    static NumberFormatInfo ReadOnly(const NumberFormatInfo& nfi) {
        NumberFormatInfo copy = nfi;
        copy.isReadOnly_ = true;
        return copy;
    }

    /**
     * @brief Returns a mutable copy of this instance.
     *
     * C++ counterpart of .NET NumberFormatInfo.Clone().
     * @return A modifiable copy.
     */
    [[nodiscard]] NumberFormatInfo Clone() const {
        NumberFormatInfo copy = *this;
        copy.isReadOnly_ = false;
        return copy;
    }

    // --- Number format ---
    /** @brief String that separates the integer from the fractional part. C++ counterpart of .NET NumberFormatInfo.NumberDecimalSeparator. @throws System::ArgumentException if @p value is empty. */
    [[nodiscard]] const std::string& getNumberDecimalSeparatorProperty() const { return numberDecimalSeparator_; }
    void setNumberDecimalSeparatorProperty(const std::string& value) { VerifyWritable(); CheckNotEmpty(value); numberDecimalSeparator_ = value; }

    /** @brief String that separates groups of digits left of the decimal point. C++ counterpart of .NET NumberFormatInfo.NumberGroupSeparator. */
    [[nodiscard]] const std::string& getNumberGroupSeparatorProperty() const { return numberGroupSeparator_; }
    void setNumberGroupSeparatorProperty(const std::string& value) { VerifyWritable(); numberGroupSeparator_ = value; }

    /** @brief Number of decimal digits in numeric values. C++ counterpart of .NET NumberFormatInfo.NumberDecimalDigits. @throws System::ArgumentOutOfRangeException if @p value is outside [0, 99]. */
    [[nodiscard]] intcs getNumberDecimalDigitsProperty() const { return numberDecimalDigits_; }
    void setNumberDecimalDigitsProperty(intcs value) { CheckRange(value, 0, 99); VerifyWritable(); numberDecimalDigits_ = value; }

    /** @brief Format pattern for negative numeric values. C++ counterpart of .NET NumberFormatInfo.NumberNegativePattern. @throws System::ArgumentOutOfRangeException if @p value is outside [0, 4]. */
    [[nodiscard]] intcs getNumberNegativePatternProperty() const { return numberNegativePattern_; }
    void setNumberNegativePatternProperty(intcs value) { CheckRange(value, 0, 4); VerifyWritable(); numberNegativePattern_ = value; }

    /** @brief Number of digits in each group left of the decimal point. C++ counterpart of .NET NumberFormatInfo.NumberGroupSizes. Returns a copy, matching .NET. @throws System::ArgumentException if any element is outside [1, 9] (the last element may also be 0). */
    [[nodiscard]] std::vector<intcs> getNumberGroupSizesProperty() const { return numberGroupSizes_; }
    void setNumberGroupSizesProperty(const std::vector<intcs>& value) { VerifyWritable(); CheckGroupSize(value); numberGroupSizes_ = value; }

    // --- Currency format ---
    /** @brief String that separates the integer from the fractional part in currency values. C++ counterpart of .NET NumberFormatInfo.CurrencyDecimalSeparator. @throws System::ArgumentException if @p value is empty. */
    [[nodiscard]] const std::string& getCurrencyDecimalSeparatorProperty() const { return currencyDecimalSeparator_; }
    void setCurrencyDecimalSeparatorProperty(const std::string& value) { VerifyWritable(); CheckNotEmpty(value); currencyDecimalSeparator_ = value; }

    /** @brief String that separates groups of digits left of the decimal in currency values. C++ counterpart of .NET NumberFormatInfo.CurrencyGroupSeparator. */
    [[nodiscard]] const std::string& getCurrencyGroupSeparatorProperty() const { return currencyGroupSeparator_; }
    void setCurrencyGroupSeparatorProperty(const std::string& value) { VerifyWritable(); currencyGroupSeparator_ = value; }

    /** @brief String used as the currency symbol. C++ counterpart of .NET NumberFormatInfo.CurrencySymbol. */
    [[nodiscard]] const std::string& getCurrencySymbolProperty() const { return currencySymbol_; }
    void setCurrencySymbolProperty(const std::string& value) { VerifyWritable(); currencySymbol_ = value; }

    /** @brief Number of decimal digits in currency values. C++ counterpart of .NET NumberFormatInfo.CurrencyDecimalDigits. @throws System::ArgumentOutOfRangeException if @p value is outside [0, 99]. */
    [[nodiscard]] intcs getCurrencyDecimalDigitsProperty() const { return currencyDecimalDigits_; }
    void setCurrencyDecimalDigitsProperty(intcs value) { CheckRange(value, 0, 99); VerifyWritable(); currencyDecimalDigits_ = value; }

    /** @brief Format pattern for negative currency values. C++ counterpart of .NET NumberFormatInfo.CurrencyNegativePattern. @throws System::ArgumentOutOfRangeException if @p value is outside [0, 16]. */
    [[nodiscard]] intcs getCurrencyNegativePatternProperty() const { return currencyNegativePattern_; }
    void setCurrencyNegativePatternProperty(intcs value) { CheckRange(value, 0, 16); VerifyWritable(); currencyNegativePattern_ = value; }

    /** @brief Format pattern for positive currency values. C++ counterpart of .NET NumberFormatInfo.CurrencyPositivePattern. @throws System::ArgumentOutOfRangeException if @p value is outside [0, 3]. */
    [[nodiscard]] intcs getCurrencyPositivePatternProperty() const { return currencyPositivePattern_; }
    void setCurrencyPositivePatternProperty(intcs value) { CheckRange(value, 0, 3); VerifyWritable(); currencyPositivePattern_ = value; }

    /** @brief Number of digits in each group left of the decimal in currency values. C++ counterpart of .NET NumberFormatInfo.CurrencyGroupSizes. Returns a copy, matching .NET. @throws System::ArgumentException if any element is outside [1, 9] (the last element may also be 0). */
    [[nodiscard]] std::vector<intcs> getCurrencyGroupSizesProperty() const { return currencyGroupSizes_; }
    void setCurrencyGroupSizesProperty(const std::vector<intcs>& value) { VerifyWritable(); CheckGroupSize(value); currencyGroupSizes_ = value; }

    // --- Sign and special symbols ---
    /** @brief String that denotes a negative number. C++ counterpart of .NET NumberFormatInfo.NegativeSign. */
    [[nodiscard]] const std::string& getNegativeSignProperty() const { return negativeSign_; }
    void setNegativeSignProperty(const std::string& value) { VerifyWritable(); negativeSign_ = value; }

    /** @brief String that denotes a positive number. C++ counterpart of .NET NumberFormatInfo.PositiveSign. */
    [[nodiscard]] const std::string& getPositiveSignProperty() const { return positiveSign_; }
    void setPositiveSignProperty(const std::string& value) { VerifyWritable(); positiveSign_ = value; }

    /** @brief String that represents the IEEE NaN value. C++ counterpart of .NET NumberFormatInfo.NaNSymbol. */
    [[nodiscard]] const std::string& getNaNSymbolProperty() const { return naNSymbol_; }
    void setNaNSymbolProperty(const std::string& value) { VerifyWritable(); naNSymbol_ = value; }

    /** @brief String that represents positive infinity. C++ counterpart of .NET NumberFormatInfo.PositiveInfinitySymbol. */
    [[nodiscard]] const std::string& getPositiveInfinitySymbolProperty() const { return positiveInfinitySymbol_; }
    void setPositiveInfinitySymbolProperty(const std::string& value) { VerifyWritable(); positiveInfinitySymbol_ = value; }

    /** @brief String that represents negative infinity. C++ counterpart of .NET NumberFormatInfo.NegativeInfinitySymbol. */
    [[nodiscard]] const std::string& getNegativeInfinitySymbolProperty() const { return negativeInfinitySymbol_; }
    void setNegativeInfinitySymbolProperty(const std::string& value) { VerifyWritable(); negativeInfinitySymbol_ = value; }

    // --- Percent format ---
    /** @brief String to use as the percent symbol. C++ counterpart of .NET NumberFormatInfo.PercentSymbol. */
    [[nodiscard]] const std::string& getPercentSymbolProperty() const { return percentSymbol_; }
    void setPercentSymbolProperty(const std::string& value) { VerifyWritable(); percentSymbol_ = value; }

    /** @brief String to use as the per mille symbol (‰). C++ counterpart of .NET NumberFormatInfo.PerMilleSymbol. */
    [[nodiscard]] const std::string& getPerMilleSymbolProperty() const { return perMilleSymbol_; }
    void setPerMilleSymbolProperty(const std::string& value) { VerifyWritable(); perMilleSymbol_ = value; }

    /** @brief String that separates the integer from the fractional part in percent values. C++ counterpart of .NET NumberFormatInfo.PercentDecimalSeparator. @throws System::ArgumentException if @p value is empty. */
    [[nodiscard]] const std::string& getPercentDecimalSeparatorProperty() const { return percentDecimalSeparator_; }
    void setPercentDecimalSeparatorProperty(const std::string& value) { VerifyWritable(); CheckNotEmpty(value); percentDecimalSeparator_ = value; }

    /** @brief String that separates groups of digits left of the decimal in percent values. C++ counterpart of .NET NumberFormatInfo.PercentGroupSeparator. */
    [[nodiscard]] const std::string& getPercentGroupSeparatorProperty() const { return percentGroupSeparator_; }
    void setPercentGroupSeparatorProperty(const std::string& value) { VerifyWritable(); percentGroupSeparator_ = value; }

    /** @brief Number of decimal digits in percent values. C++ counterpart of .NET NumberFormatInfo.PercentDecimalDigits. @throws System::ArgumentOutOfRangeException if @p value is outside [0, 99]. */
    [[nodiscard]] intcs getPercentDecimalDigitsProperty() const { return percentDecimalDigits_; }
    void setPercentDecimalDigitsProperty(intcs value) { CheckRange(value, 0, 99); VerifyWritable(); percentDecimalDigits_ = value; }

    /** @brief Format pattern for negative percent values. C++ counterpart of .NET NumberFormatInfo.PercentNegativePattern. @throws System::ArgumentOutOfRangeException if @p value is outside [0, 11]. */
    [[nodiscard]] intcs getPercentNegativePatternProperty() const { return percentNegativePattern_; }
    void setPercentNegativePatternProperty(intcs value) { CheckRange(value, 0, 11); VerifyWritable(); percentNegativePattern_ = value; }

    /** @brief Format pattern for positive percent values. C++ counterpart of .NET NumberFormatInfo.PercentPositivePattern. @throws System::ArgumentOutOfRangeException if @p value is outside [0, 3]. */
    [[nodiscard]] intcs getPercentPositivePatternProperty() const { return percentPositivePattern_; }
    void setPercentPositivePatternProperty(intcs value) { CheckRange(value, 0, 3); VerifyWritable(); percentPositivePattern_ = value; }

    /** @brief Number of digits in each group left of the decimal in percent values. C++ counterpart of .NET NumberFormatInfo.PercentGroupSizes. Returns a copy, matching .NET. @throws System::ArgumentException if any element is outside [1, 9] (the last element may also be 0). */
    [[nodiscard]] std::vector<intcs> getPercentGroupSizesProperty() const { return percentGroupSizes_; }
    void setPercentGroupSizesProperty(const std::vector<intcs>& value) { VerifyWritable(); CheckGroupSize(value); percentGroupSizes_ = value; }

    // --- Digit substitution ---
    /** @brief Specifies how a GUI displays the shape of a digit. C++ counterpart of .NET NumberFormatInfo.DigitSubstitution. @throws System::ArgumentException if @p value is not a valid DigitShapes member. */
    [[nodiscard]] DigitShapes getDigitSubstitutionProperty() const { return digitSubstitution_; }
    void setDigitSubstitutionProperty(DigitShapes value) { VerifyWritable(); CheckDigitSubstitution(value); digitSubstitution_ = value; }

    /** @brief Native digits equivalent to Western digits 0-9. C++ counterpart of .NET NumberFormatInfo.NativeDigits. Returns a copy, matching .NET. @throws System::ArgumentException unless @p value has exactly 10 single-codepoint entries. */
    [[nodiscard]] std::vector<std::string> getNativeDigitsProperty() const { return nativeDigits_; }
    void setNativeDigitsProperty(const std::vector<std::string>& value) { VerifyWritable(); CheckNativeDigits(value); nativeDigits_ = value; }

private:
    static NumberFormatInfo makeReadOnly() {
        NumberFormatInfo info;
        info.isReadOnly_ = true;
        return info;
    }
};

} // namespace System::Globalization

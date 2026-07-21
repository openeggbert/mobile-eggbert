// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/Uri.hpp"

namespace System {

    /**
     * @brief Converts a string to a Uri and vice versa.
     *
     * Minimal C++ counterpart of .NET System.UriTypeConverter.
     * The .NET class derives from TypeConverter (System.ComponentModel), which
     * is not ported to sharp-runtime.  This class provides the minimum API
     * needed to convert between strings and System::Uri objects.
     */
    class UriTypeConverter {
    public:
        /** @brief Default constructor. */
        UriTypeConverter() = default;

        /** @brief Virtual destructor. */
        virtual ~UriTypeConverter() = default;

        /**
         * @brief Returns true if conversion from a string is supported.
         *
         * C++ counterpart of .NET UriTypeConverter.CanConvertFrom(ITypeDescriptorContext, Type).
         * Always returns true (only string→Uri is supported).
         */
        [[nodiscard]] virtual bool CanConvertFrom() const noexcept { return true; }

        /**
         * @brief Returns true if conversion to a string is supported.
         *
         * C++ counterpart of .NET UriTypeConverter.CanConvertTo(ITypeDescriptorContext, Type).
         * Always returns true (only Uri→string is supported).
         */
        [[nodiscard]] virtual bool CanConvertTo() const noexcept { return true; }

        /**
         * @brief Converts a string to a Uri.
         *
         * C++ counterpart of .NET UriTypeConverter.ConvertFrom(ITypeDescriptorContext, CultureInfo, object).
         * @param text The string representation of a URI.
         * @return A Uri constructed from @p text.
         * @throws System::UriFormatException if @p text is empty or malformed.
         */
        [[nodiscard]] virtual Uri ConvertFrom(const std::string& text) const {
            return Uri(text);
        }

        /**
         * @brief Converts a Uri to its string representation.
         *
         * C++ counterpart of .NET UriTypeConverter.ConvertTo(ITypeDescriptorContext, CultureInfo, object, Type).
         * Real .NET's implementation returns uri.OriginalString, not uri.AbsoluteUri --
         * verified against UriTypeConverter.cs, which explicitly uses OriginalString so the
         * round-trip works for relative Uris too (AbsoluteUri throws for those). This
         * previously called getAbsoluteUriProperty() instead -- fixed to match, though in this
         * port's current implementation the two properties hold the same underlying value (see
         * Uri::getOriginalStringProperty()'s doc-comment), so this is a forward-looking
         * correctness fix rather than one with an observable behavior difference today.
         * @param uri The Uri to convert.
         * @return The original URI string.
         */
        [[nodiscard]] virtual std::string ConvertTo(const Uri& uri) const {
            return uri.getOriginalStringProperty();
        }
    };

} // namespace System

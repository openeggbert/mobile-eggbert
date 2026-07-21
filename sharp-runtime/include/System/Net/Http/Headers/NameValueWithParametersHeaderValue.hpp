// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include <vector>
#include "System/Net/Http/Headers/NameValueHeaderValue.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Net::Http::Headers {

    /**
     * @brief A NameValueHeaderValue with an additional list of ';'-separated parameters
     * (e.g. media-type parameters: "text/plain; charset=utf-8; boundary=xyz").
     *
     * C++ counterpart of .NET System.Net.Http.Headers.NameValueWithParametersHeaderValue.
     *
     * @note Equals/GetHashCode/ToString here are new (non-virtual) member functions that
     * intentionally hide NameValueHeaderValue's — this type is always used concretely, never
     * through a base NameValueHeaderValue reference or pointer, so there is no virtual-dispatch
     * bug risk (see NEXT.md's "hides base overloads" invariant note; that risk applies when
     * callers hold a base reference/pointer and expect the derived override to run, which doesn't
     * happen here).
     * .NET's internal GetNameValueWithParametersLength parsing helper isn't ported.
     */
    class NameValueWithParametersHeaderValue : public NameValueHeaderValue {
        std::vector<NameValueHeaderValue> parameters_;

    public:
        /** Constructs a name-only value with no parameters. */
        explicit NameValueWithParametersHeaderValue(const std::string& name);
        /** Constructs a name/value pair with no parameters. */
        NameValueWithParametersHeaderValue(const std::string& name, const std::string& value);

        /** @return The parameter list (mutable — add entries directly). */
        [[nodiscard]] std::vector<NameValueHeaderValue>& getParametersProperty() { return parameters_; }
        /** @return The parameter list. */
        [[nodiscard]] const std::vector<NameValueHeaderValue>& getParametersProperty() const { return parameters_; }

        /** Equality: base Name/Value must match, and Parameters must contain the same entries (order-independent). */
        [[nodiscard]] bool Equals(const NameValueWithParametersHeaderValue& other) const;
        bool operator==(const NameValueWithParametersHeaderValue& o) const { return Equals(o); }
        bool operator!=(const NameValueWithParametersHeaderValue& o) const { return !Equals(o); }

        /** @return A hash code consistent with Equals(). */
        [[nodiscard]] SharpRuntime::intcs GetHashCode() const;

        /** @return "name=value; param1=value1; param2=value2" (parameters space-separated after each ';'). */
        [[nodiscard]] std::string ToString() const;

        /**
         * @brief Parses "name[=value][; param1=value1; param2=value2...]".
         * @throws System::FormatException if @p input is not valid.
         */
        [[nodiscard]] static NameValueWithParametersHeaderValue Parse(const std::string& input);

        /** Attempts to parse @p input; returns false without throwing on failure. */
        [[nodiscard]] static bool TryParse(const std::string& input, NameValueWithParametersHeaderValue& parsedValue);
    };

} // namespace System::Net::Http::Headers

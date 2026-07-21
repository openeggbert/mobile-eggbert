// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Text/Json/JsonElement.hpp"
#include "System/Collections/Generic/KeyNotFoundException.hpp"
#include "System/FormatException.hpp"
#include "System/IndexOutOfRangeException.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/Text/Json/JsonProperty.hpp"

namespace System::Text::Json {

    namespace {
        const char* kindName(JsonValueKind kind) {
            switch (kind) {
                case JsonValueKind::Undefined: return "Undefined";
                case JsonValueKind::Object: return "Object";
                case JsonValueKind::Array: return "Array";
                case JsonValueKind::String: return "String";
                case JsonValueKind::Number: return "Number";
                case JsonValueKind::True: return "True";
                case JsonValueKind::False: return "False";
                case JsonValueKind::Null: return "Null";
            }
            return "Undefined";
        }
    } // namespace

    const nlohmann::ordered_json& JsonElement::require(JsonValueKind expected, const char* what) const {
        if (getValueKindProperty() != expected) {
            throw System::InvalidOperationException(
                std::string("The requested operation requires an element of type '") + what +
                "', but the target element has type '" + kindName(getValueKindProperty()) + "'.");
        }
        return *node_;
    }

    intcs JsonElement::GetInt32() const {
        intcs value = 0;
        if (!TryGetInt32(value))
            throw System::FormatException("The JSON value could not be converted to Int32.");
        return value;
    }

    longcs JsonElement::GetInt64() const {
        longcs value = 0;
        if (!TryGetInt64(value))
            throw System::FormatException("The JSON value could not be converted to Int64.");
        return value;
    }

    bool JsonElement::GetBoolean() const {
        JsonValueKind kind = getValueKindProperty();
        if (kind == JsonValueKind::True) return true;
        if (kind == JsonValueKind::False) return false;
        throw System::InvalidOperationException(
            std::string("The requested operation requires an element of type 'Boolean', but the target element has type '") +
            kindName(kind) + "'.");
    }

    bool JsonElement::TryGetInt32(intcs& value) const {
        value = 0;
        const auto& n = require(JsonValueKind::Number, "Number");
        // Verified against JsonDocument.cs's TryGetValue(out int): real .NET parses the
        // original number *text* with Utf8Parser.TryParse<int>, which fails outright for a
        // JSON float literal (e.g. "1.0", "2e1") regardless of its numeric value, and never
        // round-trips through a floating-point intermediate. Routing every JSON number through
        // get<double>() first, as this port previously did, is UB when casting an
        // out-of-int32-range (or NaN-adjacent) double to intcs, and silently accepts float
        // literals whose value happens to be integral.
        if (n.is_number_float()) return false;
        if (n.is_number_unsigned()) {
            auto u = n.get<std::uint64_t>();
            if (u > static_cast<std::uint64_t>(SharpRuntime::INTCS_MAX)) return false;
            value = static_cast<intcs>(u);
            return true;
        }
        auto i = n.get<std::int64_t>();
        if (i < static_cast<std::int64_t>(SharpRuntime::INTCS_MIN) || i > static_cast<std::int64_t>(SharpRuntime::INTCS_MAX)) return false;
        value = static_cast<intcs>(i);
        return true;
    }

    bool JsonElement::TryGetInt64(longcs& value) const {
        value = 0;
        const auto& n = require(JsonValueKind::Number, "Number");
        // See TryGetInt32's note: same text-based-parse semantics, and the same UB risk this
        // port previously had casting an out-of-int64-range double to longcs.
        if (n.is_number_float()) return false;
        if (n.is_number_unsigned()) {
            auto u = n.get<std::uint64_t>();
            if (u > static_cast<std::uint64_t>(SharpRuntime::LONGCS_MAX)) return false;
            value = static_cast<longcs>(u);
            return true;
        }
        value = n.get<std::int64_t>();
        return true;
    }

    JsonElement JsonElement::GetProperty(const std::string& name) const {
        JsonElement result;
        if (!TryGetProperty(name, result))
            throw System::Collections::Generic::KeyNotFoundException("The given key '" + name + "' was not present.");
        return result;
    }

    JsonElement JsonElement::operator[](intcs index) const {
        const auto& arr = require(JsonValueKind::Array, "Array");
        if (index < 0 || static_cast<size_t>(index) >= arr.size())
            throw System::IndexOutOfRangeException("Index was outside the bounds of the array.");
        return JsonElement(std::shared_ptr<const nlohmann::ordered_json>(node_, &arr[static_cast<size_t>(index)]));
    }

    std::vector<JsonProperty> JsonElement::EnumerateObject() const {
        const auto& obj = require(JsonValueKind::Object, "Object");
        std::vector<JsonProperty> result;
        result.reserve(obj.size());
        for (auto it = obj.begin(); it != obj.end(); ++it) {
            JsonElement value(std::shared_ptr<const nlohmann::ordered_json>(node_, &(*it)));
            result.push_back(JsonProperty(it.key(), std::move(value)));
        }
        return result;
    }

} // namespace System::Text::Json

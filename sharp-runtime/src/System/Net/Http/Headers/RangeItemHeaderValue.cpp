// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Net/Http/Headers/RangeItemHeaderValue.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/HashCode.hpp"
#include <stdexcept>

namespace System::Net::Http::Headers {

    RangeItemHeaderValue::RangeItemHeaderValue(std::optional<SharpRuntime::longcs> from, std::optional<SharpRuntime::longcs> to) {
        if (!from.has_value() && !to.has_value()) {
            throw System::ArgumentException("At least one of 'from' or 'to' must be specified.");
        }
        if (from.has_value() && *from < 0) throw System::ArgumentOutOfRangeException("from");
        if (to.has_value() && *to < 0) throw System::ArgumentOutOfRangeException("to");
        if (from.has_value() && to.has_value() && *from > *to) throw System::ArgumentOutOfRangeException("from");

        from_ = from;
        to_ = to;
    }

    std::string RangeItemHeaderValue::ToString() const {
        std::string result;
        if (from_.has_value()) result += std::to_string(*from_);
        result += "-";
        if (to_.has_value()) result += std::to_string(*to_);
        return result;
    }

    SharpRuntime::intcs RangeItemHeaderValue::GetHashCode() const {
        return System::HashCode::Combine(from_.value_or(-1), to_.value_or(-1));
    }

} // namespace System::Net::Http::Headers

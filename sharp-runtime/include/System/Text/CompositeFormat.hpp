// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <string>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/FormatException.hpp"

namespace System::Text {

    /** Represents a parsed composite format string (e.g. "Hello, {0}!") that avoids re-parsing on each use. */
    class CompositeFormat {
        std::string format_;
        SharpRuntime::intcs minArgCount_ = 0;

        // Real .NET's CompositeFormat.Parse validates the format string and throws FormatException
        // for an invalid format item -- unterminated "{"/stray "}", or an empty/non-numeric index.
        // An earlier version of this parser silently accepted any malformed input (e.g. "Hello {"
        // or a stray "}") and just returned MinimumArgumentCount 0, masking bugs in caller-supplied
        // format strings instead of surfacing them like real .NET does.
        static SharpRuntime::intcs countPlaceholders(const std::string& fmt) {
            SharpRuntime::intcs maxIdx = -1;
            for (std::size_t i = 0; i < fmt.size(); ) {
                char c = fmt[i];
                if (c == '{') {
                    if (i + 1 < fmt.size() && fmt[i + 1] == '{') { i += 2; continue; }
                    std::size_t j = i + 1;
                    while (j < fmt.size() && fmt[j] != '}' && fmt[j] != ',' && fmt[j] != ':') ++j;
                    if (j >= fmt.size())
                        throw System::FormatException("Input string was not in a correct format.");
                    std::string idxStr = fmt.substr(i + 1, j - i - 1);
                    if (idxStr.empty() ||
                        !std::all_of(idxStr.begin(), idxStr.end(), [](unsigned char ch) { return std::isdigit(ch) != 0; }))
                        throw System::FormatException("Input string was not in a correct format.");
                    SharpRuntime::intcs idx = std::stoi(idxStr);
                    if (idx > maxIdx) maxIdx = idx;
                    while (j < fmt.size() && fmt[j] != '}') ++j;
                    if (j >= fmt.size())
                        throw System::FormatException("Input string was not in a correct format.");
                    i = j + 1;
                } else if (c == '}') {
                    if (i + 1 < fmt.size() && fmt[i + 1] == '}') { i += 2; continue; }
                    throw System::FormatException("Input string was not in a correct format.");
                } else {
                    ++i;
                }
            }
            return maxIdx + 1;
        }

    public:
        /**
         * @brief Parses a composite format string and returns a CompositeFormat instance.
         * @throws System::FormatException if @p format contains an invalid format item
         *         (unterminated placeholder, stray '}', or a non-numeric/empty argument index).
         */
        static CompositeFormat Parse(const std::string& format) {
            CompositeFormat cf;
            cf.format_ = format;
            cf.minArgCount_ = countPlaceholders(format);
            return cf;
        }

        /** Gets the original format string. */
        [[nodiscard]] const std::string& getFormatProperty() const { return format_; }
        /** Gets the minimum number of arguments required to satisfy the format string. */
        [[nodiscard]] SharpRuntime::intcs getMinimumArgumentCountProperty() const { return minArgCount_; }
    };

} // namespace System::Text

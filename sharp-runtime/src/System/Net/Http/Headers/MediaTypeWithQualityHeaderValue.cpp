// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Net/Http/Headers/MediaTypeWithQualityHeaderValue.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/FormatException.hpp"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <stdexcept>

namespace System::Net::Http::Headers {

    namespace {
        bool equalsIgnoreCase(const std::string& a, const std::string& b) {
            if (a.size() != b.size()) return false;
            return std::equal(a.begin(), a.end(), b.begin(), [](unsigned char x, unsigned char y) {
                return std::tolower(x) == std::tolower(y);
            });
        }

        std::string formatQuality(double quality) {
            char buf[16];
            std::snprintf(buf, sizeof(buf), "%.3f", quality);
            std::string s(buf);
            size_t lastNonZero = s.find_last_not_of('0');
            if (lastNonZero != std::string::npos && s[lastNonZero] == '.') lastNonZero++;
            return s.substr(0, lastNonZero + 1);
        }

        // Matches real .NET's HttpRuleParser.GetNumberLength(input, start, allowDecimal: true)
        // grammar: digits with at most one '.', no leading dot (".5" is invalid, "0.5" required),
        // no sign, no exponent. A trailing dot with no digits after ("1.") is valid. std::stod
        // alone is far more permissive (accepts a leading dot, a sign, and scientific notation
        // like "1e0"), which would let TryParse silently accept quality values real .NET rejects.
        bool isValidQualityNumber(const std::string& s) {
            if (s.empty() || s[0] == '.') return false;
            bool haveDot = false, haveDigit = false;
            for (char c : s) {
                if (c >= '0' && c <= '9') haveDigit = true;
                else if (c == '.' && !haveDot) haveDot = true;
                else return false;
            }
            return haveDigit;
        }

        bool tryParseQuality(const std::string& s, double& out) {
            if (!isValidQualityNumber(s)) return false;
            try {
                size_t pos = 0;
                out = std::stod(s, &pos);
                return pos == s.size() && out >= 0.0 && out <= 1.0;
            } catch (...) {
                return false;
            }
        }
    }

    MediaTypeWithQualityHeaderValue::MediaTypeWithQualityHeaderValue(const std::string& mediaType)
        : MediaTypeHeaderValue(mediaType) {}

    MediaTypeWithQualityHeaderValue::MediaTypeWithQualityHeaderValue(const std::string& mediaType, double quality)
        : MediaTypeHeaderValue(mediaType) {
        setQualityProperty(quality);
    }

    std::optional<double> MediaTypeWithQualityHeaderValue::getQualityProperty() const {
        for (const auto& p : getParametersProperty()) {
            if (equalsIgnoreCase(p.getNameProperty(), "q")) {
                double q;
                if (tryParseQuality(p.getValueProperty(), q)) return q;
                return std::nullopt;
            }
        }
        return std::nullopt;
    }

    void MediaTypeWithQualityHeaderValue::setQualityProperty(std::optional<double> value) {
        auto& params = getParametersProperty();
        for (auto it = params.begin(); it != params.end(); ++it) {
            if (equalsIgnoreCase(it->getNameProperty(), "q")) {
                params.erase(it);
                break;
            }
        }
        if (value.has_value()) {
            if (*value < 0.0) throw System::ArgumentOutOfRangeException("quality", "quality must not be negative.");
            if (*value > 1.0) throw System::ArgumentOutOfRangeException("quality", "quality must not be greater than 1.0.");
            params.push_back(NameValueHeaderValue("q", formatQuality(*value)));
        }
    }

    bool MediaTypeWithQualityHeaderValue::TryParse(const std::string& input, MediaTypeWithQualityHeaderValue& parsedValue) {
        MediaTypeHeaderValue base_("x/x");
        if (!MediaTypeHeaderValue::TryParse(input, base_)) return false;

        for (const auto& p : base_.getParametersProperty()) {
            if (equalsIgnoreCase(p.getNameProperty(), "q")) {
                double q;
                if (!tryParseQuality(p.getValueProperty(), q)) return false;
            }
        }

        MediaTypeWithQualityHeaderValue result(base_.getMediaTypeProperty());
        result.getParametersProperty() = base_.getParametersProperty();
        parsedValue = result;
        return true;
    }

    MediaTypeWithQualityHeaderValue MediaTypeWithQualityHeaderValue::Parse(const std::string& input) {
        MediaTypeWithQualityHeaderValue result("x/x");
        if (!TryParse(input, result)) {
            throw System::FormatException("The media type header value is not valid: " + input);
        }
        return result;
    }

} // namespace System::Net::Http::Headers

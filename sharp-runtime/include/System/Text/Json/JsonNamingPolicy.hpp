// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <cctype>
#include <memory>
#include <optional>
#include <string>

namespace System::Text::Json {

    /**
     * @brief Determines the naming policy used to convert a string-based name to another format,
     * such as a camel-casing format.
     *
     * C++ counterpart of .NET System.Text.Json.JsonNamingPolicy.
     *
     * @note Word-boundary detection uses `<cctype>` (ASCII upper/lower/digit classification)
     * rather than .NET's full Unicode-category classification — matches for the common
     * ASCII-identifier case (the overwhelmingly common case for JSON property names in ported
     * game code) but won't split words on non-ASCII letters the way .NET's Unicode-aware version
     * would.
     */
    class JsonNamingPolicy {
    public:
        virtual ~JsonNamingPolicy() = default;

        /** @brief Converts @p name according to the policy. */
        [[nodiscard]] virtual std::string ConvertName(const std::string& name) const = 0;

        /** @return The naming policy for camelCasing. */
        static const std::shared_ptr<JsonNamingPolicy>& CamelCase();
        /** @return The naming policy for lower snake_casing. */
        static const std::shared_ptr<JsonNamingPolicy>& SnakeCaseLower();
        /** @return The naming policy for upper SNAKE_CASING. */
        static const std::shared_ptr<JsonNamingPolicy>& SnakeCaseUpper();
        /** @return The naming policy for lower kebab-casing. */
        static const std::shared_ptr<JsonNamingPolicy>& KebabCaseLower();
        /** @return The naming policy for upper KEBAB-CASING. */
        static const std::shared_ptr<JsonNamingPolicy>& KebabCaseUpper();
        /** @return The naming policy for PascalCasing. */
        static const std::shared_ptr<JsonNamingPolicy>& PascalCase();

    protected:
        enum class WordCasing { LowerCase, UpperCase, PascalCase };

        // Mirrors .NET's internal JsonSeparatorNamingPolicy.ConvertNameCore: walks the input
        // classifying each char as an uppercase letter, lowercase-letter-or-digit, a space, or
        // other, inserting `separator` at word boundaries and re-casing each letter per `wordCasing`.
        static std::string ConvertNameCore(std::optional<char> separator, WordCasing wordCasing,
                                            const std::string& name) {
            enum class SeparatorState { NotStarted, UppercaseLetter, LowercaseLetterOrDigit, SpaceSeparator };

            std::string result;
            result.reserve(name.size() + name.size() / 5);
            SeparatorState state = SeparatorState::NotStarted;

            for (size_t i = 0; i < name.size(); ++i) {
                unsigned char c = static_cast<unsigned char>(name[i]);

                if (std::isupper(c)) {
                    bool isWordBoundary = false;
                    switch (state) {
                        case SeparatorState::NotStarted:
                            isWordBoundary = true;
                            break;
                        case SeparatorState::LowercaseLetterOrDigit:
                        case SeparatorState::SpaceSeparator:
                            isWordBoundary = true;
                            if (separator.has_value()) result += *separator;
                            break;
                        case SeparatorState::UppercaseLetter:
                            if (i + 1 < name.size() && std::islower(static_cast<unsigned char>(name[i + 1]))) {
                                isWordBoundary = true;
                                if (separator.has_value()) result += *separator;
                            }
                            break;
                    }

                    char out = static_cast<char>(c);
                    switch (wordCasing) {
                        case WordCasing::LowerCase:
                            out = static_cast<char>(std::tolower(c));
                            break;
                        case WordCasing::PascalCase:
                            out = isWordBoundary ? static_cast<char>(c) : static_cast<char>(std::tolower(c));
                            break;
                        case WordCasing::UpperCase:
                            break;
                    }
                    result += out;
                    state = SeparatorState::UppercaseLetter;
                } else if (std::islower(c) || std::isdigit(c)) {
                    bool isWordStart = state == SeparatorState::SpaceSeparator || state == SeparatorState::NotStarted;
                    if (state == SeparatorState::SpaceSeparator && separator.has_value()) result += *separator;

                    char out = static_cast<char>(c);
                    if (std::islower(c)) {
                        switch (wordCasing) {
                            case WordCasing::UpperCase:
                                out = static_cast<char>(std::toupper(c));
                                break;
                            case WordCasing::PascalCase:
                                out = isWordStart ? static_cast<char>(std::toupper(c)) : static_cast<char>(c);
                                break;
                            case WordCasing::LowerCase:
                                break;
                        }
                    }
                    result += out;
                    state = SeparatorState::LowercaseLetterOrDigit;
                } else if (c == ' ') {
                    if (state != SeparatorState::NotStarted) state = SeparatorState::SpaceSeparator;
                } else {
                    result += static_cast<char>(c);
                    state = SeparatorState::NotStarted;
                }
            }
            return result;
        }
    };

    namespace detail {

        // Mirrors .NET's internal JsonCamelCaseNamingPolicy: unlike the other policies below, camel
        // case only lowercases a leading run of uppercase letters and otherwise passes the rest of
        // the string through unchanged (so e.g. "XMLReader" -> "xmlReader", not "xMLReader").
        class JsonCamelCaseNamingPolicy final : public JsonNamingPolicy {
        public:
            [[nodiscard]] std::string ConvertName(const std::string& name) const override {
                if (name.empty() || !std::isupper(static_cast<unsigned char>(name[0]))) return name;
                std::string chars = name;
                for (size_t i = 0; i < chars.size(); ++i) {
                    if (i == 1 && !std::isupper(static_cast<unsigned char>(chars[i]))) break;
                    bool hasNext = i + 1 < chars.size();
                    if (i > 0 && hasNext && !std::isupper(static_cast<unsigned char>(chars[i + 1]))) {
                        if (chars[i + 1] == ' ') chars[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(chars[i])));
                        break;
                    }
                    chars[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(chars[i])));
                }
                return chars;
            }
        };

        class JsonSeparatorNamingPolicy : public JsonNamingPolicy {
            std::optional<char> separator_;
            WordCasing wordCasing_;

        public:
            JsonSeparatorNamingPolicy(std::optional<char> separator, WordCasing wordCasing)
                : separator_(separator), wordCasing_(wordCasing) {}

            [[nodiscard]] std::string ConvertName(const std::string& name) const override {
                return ConvertNameCore(separator_, wordCasing_, name);
            }
        };

    } // namespace detail

    inline const std::shared_ptr<JsonNamingPolicy>& JsonNamingPolicy::CamelCase() {
        static const std::shared_ptr<JsonNamingPolicy> instance = std::make_shared<detail::JsonCamelCaseNamingPolicy>();
        return instance;
    }
    inline const std::shared_ptr<JsonNamingPolicy>& JsonNamingPolicy::SnakeCaseLower() {
        static const std::shared_ptr<JsonNamingPolicy> instance =
            std::make_shared<detail::JsonSeparatorNamingPolicy>('_', WordCasing::LowerCase);
        return instance;
    }
    inline const std::shared_ptr<JsonNamingPolicy>& JsonNamingPolicy::SnakeCaseUpper() {
        static const std::shared_ptr<JsonNamingPolicy> instance =
            std::make_shared<detail::JsonSeparatorNamingPolicy>('_', WordCasing::UpperCase);
        return instance;
    }
    inline const std::shared_ptr<JsonNamingPolicy>& JsonNamingPolicy::KebabCaseLower() {
        static const std::shared_ptr<JsonNamingPolicy> instance =
            std::make_shared<detail::JsonSeparatorNamingPolicy>('-', WordCasing::LowerCase);
        return instance;
    }
    inline const std::shared_ptr<JsonNamingPolicy>& JsonNamingPolicy::KebabCaseUpper() {
        static const std::shared_ptr<JsonNamingPolicy> instance =
            std::make_shared<detail::JsonSeparatorNamingPolicy>('-', WordCasing::UpperCase);
        return instance;
    }
    inline const std::shared_ptr<JsonNamingPolicy>& JsonNamingPolicy::PascalCase() {
        static const std::shared_ptr<JsonNamingPolicy> instance =
            std::make_shared<detail::JsonSeparatorNamingPolicy>(std::nullopt, WordCasing::PascalCase);
        return instance;
    }

} // namespace System::Text::Json

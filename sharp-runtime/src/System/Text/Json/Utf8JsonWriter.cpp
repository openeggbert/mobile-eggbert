// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Text/Json/Utf8JsonWriter.hpp"
#include <cmath>
#include <cstdio>
#include "System/ArgumentException.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/Text/Json/JsonException.hpp"
#include "nlohmann/json.hpp"

namespace System::Text::Json {

namespace {

    // Decodes one UTF-8 sequence starting at s[i], validating continuation bytes and
    // rejecting overlong encodings -- the same conformance logic already applied elsewhere
    // in this codebase (Rune::TryGetRuneAt/UnicodeEncoding/UTF32Encoding/ASCIIEncoding/
    // IdnMapping's decode loops). An ill-formed sequence decodes to U+FFFD (length 1).
    void decodeUtf8(const std::string& s, size_t i, uint32_t& codePoint, size_t& length) {
        auto isContinuation = [](unsigned char b) { return (b & 0xC0) == 0x80; };
        unsigned char c0 = static_cast<unsigned char>(s[i]);
        uint32_t cp; size_t len;
        if (c0 < 0x80) {
            cp = c0; len = 1;
        } else if ((c0 & 0xE0) == 0xC0 && i + 1 < s.size() &&
                   isContinuation(static_cast<unsigned char>(s[i + 1]))) {
            cp = (static_cast<uint32_t>(c0 & 0x1F) << 6) | (static_cast<unsigned char>(s[i + 1]) & 0x3F);
            len = 2;
            if (cp < 0x80) { codePoint = 0xFFFD; length = 1; return; }
        } else if ((c0 & 0xF0) == 0xE0 && i + 2 < s.size() &&
                   isContinuation(static_cast<unsigned char>(s[i + 1])) &&
                   isContinuation(static_cast<unsigned char>(s[i + 2]))) {
            cp = (static_cast<uint32_t>(c0 & 0x0F) << 12) | ((static_cast<unsigned char>(s[i + 1]) & 0x3F) << 6) |
                 (static_cast<unsigned char>(s[i + 2]) & 0x3F);
            len = 3;
            if (cp < 0x800) { codePoint = 0xFFFD; length = 1; return; }
        } else if ((c0 & 0xF8) == 0xF0 && i + 3 < s.size() &&
                   isContinuation(static_cast<unsigned char>(s[i + 1])) &&
                   isContinuation(static_cast<unsigned char>(s[i + 2])) &&
                   isContinuation(static_cast<unsigned char>(s[i + 3]))) {
            cp = (static_cast<uint32_t>(c0 & 0x07) << 18) | ((static_cast<unsigned char>(s[i + 1]) & 0x3F) << 12) |
                 ((static_cast<unsigned char>(s[i + 2]) & 0x3F) << 6) | (static_cast<unsigned char>(s[i + 3]) & 0x3F);
            len = 4;
            if (cp < 0x10000) { codePoint = 0xFFFD; length = 1; return; }
        } else {
            codePoint = 0xFFFD;
            length = 1;
            return;
        }
        if (cp > 0x10FFFF || (cp >= 0xD800 && cp <= 0xDFFF)) {
            codePoint = 0xFFFD;
            length = 1;
            return;
        }
        codePoint = cp;
        length = len;
    }

} // namespace

    void Utf8JsonWriter::writeIndentIfNeeded() {
        if (!options_.Indented) return;
        buffer_ += options_.NewLine;
        buffer_.append(stack_.size() * static_cast<size_t>(options_.IndentSize), options_.IndentCharacter);
    }

    void Utf8JsonWriter::beforeWritingElement() {
        if (!stack_.empty()) {
            Frame& top = stack_.back();
            if (top.hasWrittenItem) buffer_ += ',';
            writeIndentIfNeeded();
            top.hasWrittenItem = true;
        }
    }

    // Called after a value (scalar or nested container) has been fully written: clears the
    // enclosing object's "awaiting a property value" state, or marks the root scalar as written.
    void Utf8JsonWriter::markValueWritten() {
        if (stack_.empty()) {
            rootValueWritten_ = true;
        } else if (stack_.back().isObject) {
            stack_.back().awaitingPropertyValue = false;
        }
    }

    // Verified against DefaultJavaScriptEncoder.cs/AllowedBmpCodePointsBitmap.cs: the
    // default encoder's allow-list is UnicodeRanges.BasicLatin (U+0000-U+007F) with
    // undefined/control characters (including DEL, U+007F) and HTML-sensitive characters
    // (< > & ' " +) removed, plus explicit extra escapes for \ and ` -- so effectively
    // every codepoint outside printable ASCII minus that punctuation set is escaped, and
    // (per MaxOutputCharactersPerInputCharacter's doc comment) every codepoint >= U+0080
    // unconditionally, as \uXXXX (or a \uXXXX\uYYYY surrogate pair for astral code points
    // U+10000+). The previous implementation only escaped control chars, ", \, and
    // <>&' -- non-ASCII characters (names, i18n text, emoji) passed through completely
    // unescaped, and +/`/DEL were missed even within the ASCII range.
    void Utf8JsonWriter::appendEscapedString(const std::string& value) {
        buffer_ += '"';
        size_t i = 0;
        while (i < value.size()) {
            unsigned char c0 = static_cast<unsigned char>(value[i]);
            if (c0 < 0x80) {
                char c = static_cast<char>(c0);
                switch (c) {
                    case '"': buffer_ += "\\\""; break;
                    case '\\': buffer_ += "\\\\"; break;
                    case '\b': buffer_ += "\\b"; break;
                    case '\f': buffer_ += "\\f"; break;
                    case '\n': buffer_ += "\\n"; break;
                    case '\r': buffer_ += "\\r"; break;
                    case '\t': buffer_ += "\\t"; break;
                    case '<': case '>': case '&': case '\'': case '+': case '`': {
                        char hex[8];
                        std::snprintf(hex, sizeof(hex), "\\u%04x", c0);
                        buffer_ += hex;
                        break;
                    }
                    default:
                        if (c0 < 0x20 || c0 == 0x7F) {
                            char hex[8];
                            std::snprintf(hex, sizeof(hex), "\\u%04x", c0);
                            buffer_ += hex;
                        } else {
                            buffer_ += c;
                        }
                        break;
                }
                ++i;
            } else {
                uint32_t cp; size_t len;
                decodeUtf8(value, i, cp, len);
                i += len;
                if (cp <= 0xFFFF) {
                    char hex[8];
                    std::snprintf(hex, sizeof(hex), "\\u%04x", cp);
                    buffer_ += hex;
                } else {
                    uint32_t v = cp - 0x10000;
                    uint32_t hi = 0xD800 + (v >> 10);
                    uint32_t lo = 0xDC00 + (v & 0x3FF);
                    char hex[16];
                    std::snprintf(hex, sizeof(hex), "\\u%04x\\u%04x", hi, lo);
                    buffer_ += hex;
                }
            }
        }
        buffer_ += '"';
    }

    void Utf8JsonWriter::validateCanWriteContainerStart() const {
        if (options_.SkipValidation) return;
        bool ok = stack_.empty() ? !rootValueWritten_
                                  : (!stack_.back().isObject || stack_.back().awaitingPropertyValue);
        if (!ok) throw System::InvalidOperationException("Cannot write the start of an array or object here.");
        if (options_.MaxDepth > 0 && static_cast<intcs>(stack_.size()) >= options_.MaxDepth)
            throw System::InvalidOperationException("The depth of the JSON exceeds the maximum configured depth.");
    }

    void Utf8JsonWriter::validateCanWritePropertyName() const {
        if (options_.SkipValidation) return;
        if (stack_.empty() || !stack_.back().isObject || stack_.back().awaitingPropertyValue)
            throw System::InvalidOperationException("Cannot write a property name here.");
    }

    void Utf8JsonWriter::validateCanWriteValue() const {
        if (options_.SkipValidation) return;
        bool ok = stack_.empty() ? !rootValueWritten_
                                  : (!stack_.back().isObject || stack_.back().awaitingPropertyValue);
        if (!ok) throw System::InvalidOperationException("Cannot write a value here.");
    }

    void Utf8JsonWriter::validateCanWriteEnd(bool isObject) const {
        if (options_.SkipValidation) return;
        if (stack_.empty() || stack_.back().isObject != isObject || stack_.back().awaitingPropertyValue)
            throw System::InvalidOperationException(
                std::string("Cannot write the end of an ") + (isObject ? "object" : "array") + " here.");
    }

    void Utf8JsonWriter::WriteStartObject() {
        validateCanWriteContainerStart();
        beforeWritingElement();
        buffer_ += '{';
        markValueWritten();
        stack_.push_back(Frame{true, false, false});
    }

    void Utf8JsonWriter::WriteStartObject(const std::string& propertyName) {
        WritePropertyName(propertyName);
        if (!options_.SkipValidation && options_.MaxDepth > 0 && static_cast<intcs>(stack_.size()) >= options_.MaxDepth)
            throw System::InvalidOperationException("The depth of the JSON exceeds the maximum configured depth.");
        buffer_ += '{';
        markValueWritten();
        stack_.push_back(Frame{true, false, false});
    }

    void Utf8JsonWriter::WriteEndObject() {
        validateCanWriteEnd(true);
        bool hadItems = stack_.back().hasWrittenItem;
        stack_.pop_back();
        if (hadItems) writeIndentIfNeeded();
        buffer_ += '}';
    }

    void Utf8JsonWriter::WriteStartArray() {
        validateCanWriteContainerStart();
        beforeWritingElement();
        buffer_ += '[';
        markValueWritten();
        stack_.push_back(Frame{false, false, false});
    }

    void Utf8JsonWriter::WriteStartArray(const std::string& propertyName) {
        WritePropertyName(propertyName);
        if (!options_.SkipValidation && options_.MaxDepth > 0 && static_cast<intcs>(stack_.size()) >= options_.MaxDepth)
            throw System::InvalidOperationException("The depth of the JSON exceeds the maximum configured depth.");
        buffer_ += '[';
        markValueWritten();
        stack_.push_back(Frame{false, false, false});
    }

    void Utf8JsonWriter::WriteEndArray() {
        validateCanWriteEnd(false);
        bool hadItems = stack_.back().hasWrittenItem;
        stack_.pop_back();
        if (hadItems) writeIndentIfNeeded();
        buffer_ += ']';
    }

    void Utf8JsonWriter::WritePropertyName(const std::string& propertyName) {
        validateCanWritePropertyName();
        beforeWritingElement();
        appendEscapedString(propertyName);
        buffer_ += options_.Indented ? ": " : ":";
        stack_.back().awaitingPropertyValue = true;
    }

    void Utf8JsonWriter::WriteString(const std::string& propertyName, const std::string& value) {
        WritePropertyName(propertyName);
        WriteStringValue(value);
    }

    void Utf8JsonWriter::WriteStringValue(const std::string& value) {
        validateCanWriteValue();
        if (stack_.empty() || !stack_.back().isObject) beforeWritingElement();
        appendEscapedString(value);
        markValueWritten();
    }

    void Utf8JsonWriter::WriteNumber(const std::string& propertyName, intcs value) {
        WritePropertyName(propertyName);
        WriteNumberValue(value);
    }
    void Utf8JsonWriter::WriteNumber(const std::string& propertyName, longcs value) {
        WritePropertyName(propertyName);
        WriteNumberValue(value);
    }
    void Utf8JsonWriter::WriteNumber(const std::string& propertyName, double value) {
        // Verified against Utf8JsonWriter.WriteProperties.Double.cs's WriteNumber: real .NET
        // calls ValidateDouble(value) before writing anything (property name included), so a
        // NaN/Infinity value fails atomically rather than leaving a dangling property name
        // written with no value to follow it.
        if (!std::isfinite(value)) {
            throw System::ArgumentException(
                ".NET number values such as positive and negative infinity cannot be written as valid JSON.", "value");
        }
        WritePropertyName(propertyName);
        WriteNumberValue(value);
    }

    void Utf8JsonWriter::WriteNumberValue(intcs value) { WriteNumberValue(static_cast<longcs>(value)); }

    void Utf8JsonWriter::WriteNumberValue(longcs value) {
        validateCanWriteValue();
        if (stack_.empty() || !stack_.back().isObject) beforeWritingElement();
        buffer_ += std::to_string(value);
        markValueWritten();
    }

    // Verified against JsonWriterHelper.cs's ValidateDouble: real .NET throws ArgumentException
    // ("special number values... cannot be written as valid JSON") for NaN/+-Infinity, since
    // JSON has no representation for them. nlohmann::json(value).dump() silently serializes any
    // of the three as the JSON literal "null" instead of throwing -- a genuinely different,
    // silently-wrong value with no error.
    void Utf8JsonWriter::WriteNumberValue(double value) {
        if (!std::isfinite(value)) {
            throw System::ArgumentException(
                ".NET number values such as positive and negative infinity cannot be written as valid JSON.", "value");
        }
        validateCanWriteValue();
        if (stack_.empty() || !stack_.back().isObject) beforeWritingElement();
        buffer_ += nlohmann::json(value).dump();
        markValueWritten();
    }

    void Utf8JsonWriter::WriteBoolean(const std::string& propertyName, bool value) {
        WritePropertyName(propertyName);
        WriteBooleanValue(value);
    }

    void Utf8JsonWriter::WriteBooleanValue(bool value) {
        validateCanWriteValue();
        if (stack_.empty() || !stack_.back().isObject) beforeWritingElement();
        buffer_ += value ? "true" : "false";
        markValueWritten();
    }

    void Utf8JsonWriter::WriteNull(const std::string& propertyName) {
        WritePropertyName(propertyName);
        WriteNullValue();
    }

    void Utf8JsonWriter::WriteNullValue() {
        validateCanWriteValue();
        if (stack_.empty() || !stack_.back().isObject) beforeWritingElement();
        buffer_ += "null";
        markValueWritten();
    }

    void Utf8JsonWriter::WriteRawValue(const std::string& json, bool skipInputValidation) {
        if (!skipInputValidation && !options_.SkipValidation) {
            try {
                auto parsed = nlohmann::json::parse(json);
                (void)parsed;
            } catch (const nlohmann::json::parse_error& e) {
                throw JsonException(std::string("Invalid JSON passed to WriteRawValue: ") + e.what());
            }
        }
        validateCanWriteValue();
        if (stack_.empty() || !stack_.back().isObject) beforeWritingElement();
        buffer_ += json;
        markValueWritten();
    }

} // namespace System::Text::Json

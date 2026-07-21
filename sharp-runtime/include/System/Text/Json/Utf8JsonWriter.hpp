// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Text/Json/JsonWriterOptions.hpp"

namespace System::Text::Json {

    using SharpRuntime::intcs;
    using SharpRuntime::longcs;

    /**
     * @brief Provides a high-performance API for forward-only, non-cached writing of UTF-8 encoded
     * JSON text.
     *
     * C++ counterpart of .NET System.Text.Json.Utf8JsonWriter.
     *
     * @note Real .NET writes into a caller-supplied `IBufferWriter<byte>`/`Stream`; this port
     * instead owns an internal `std::string` buffer directly (there's no `IBufferWriter`
     * equivalent in this runtime) — retrieve the written text via GetString()/getBytesWrittenProperty()
     * instead of flushing to an external sink. Only `std::string`-based overloads are provided
     * (no separate `ReadOnlySpan<byte>`/`JsonEncodedText` overloads — those exist in .NET purely
     * for zero-copy performance, not for a different observable behavior). String escaping covers
     * control characters, `\`, `"`, and (matching .NET's default safe `JavaScriptEncoder`) the
     * HTML-sensitive `<`, `>`, `&`, `'` characters — not .NET's full default-encoder Unicode range
     * table (see JsonWriterOptions's doc comment on `Encoder` being out of scope).
     */
    class Utf8JsonWriter {
        struct Frame {
            bool isObject;
            bool hasWrittenItem = false;
            // Only meaningful when isObject is true: true between WritePropertyName() and the
            // value that satisfies it. Tracked per-frame (not as a single writer-wide flag) since
            // descending into a nested container-as-value pushes a new frame on top; the state of
            // an outer object awaiting a property value must survive untouched while its child
            // frame is on top, and only clear once the child fully closes (or a scalar/container
            // value referencing it is written) — a single shared flag would leak across depths.
            bool awaitingPropertyValue = false;
        };

        std::string buffer_;
        JsonWriterOptions options_;
        std::vector<Frame> stack_;
        bool rootValueWritten_ = false;

        void validateOptions() const { options_.Validate(); }
        void beforeWritingElement();
        void markValueWritten();
        void writeIndentIfNeeded();
        void appendEscapedString(const std::string& value);
        void validateCanWriteContainerStart() const;
        void validateCanWritePropertyName() const;
        void validateCanWriteValue() const;
        void validateCanWriteEnd(bool isObject) const;

    public:
        /** @brief Constructs a writer with the given options (an empty internal buffer to start). */
        explicit Utf8JsonWriter(JsonWriterOptions options = {}) : options_(std::move(options)) {
            validateOptions();
            if (options_.MaxDepth == 0)
                options_.MaxDepth = JsonWriterOptions::DefaultMaxDepth;
        }

        /** @return The options this writer was constructed with. */
        [[nodiscard]] const JsonWriterOptions& getOptionsProperty() const { return options_; }
        /** @return The current nesting depth (0 at the root). */
        [[nodiscard]] intcs getCurrentDepthProperty() const { return static_cast<intcs>(stack_.size()); }
        /** @return The number of bytes written to the internal buffer so far. */
        [[nodiscard]] longcs getBytesWrittenProperty() const { return static_cast<longcs>(buffer_.size()); }

        /** @brief Clears the writer's internal buffer and all state so it can be reused. */
        void Reset() {
            buffer_.clear();
            stack_.clear();
            rootValueWritten_ = false;
        }

        /** @brief No-op — the internal buffer already holds everything written (see class doc comment). */
        void Flush() {}

        /** @return The JSON text written so far. */
        [[nodiscard]] const std::string& GetString() const { return buffer_; }

        void WriteStartObject();
        void WriteStartObject(const std::string& propertyName);
        void WriteEndObject();
        void WriteStartArray();
        void WriteStartArray(const std::string& propertyName);
        void WriteEndArray();

        void WritePropertyName(const std::string& propertyName);

        void WriteString(const std::string& propertyName, const std::string& value);
        void WriteStringValue(const std::string& value);

        void WriteNumber(const std::string& propertyName, intcs value);
        void WriteNumber(const std::string& propertyName, longcs value);
        void WriteNumber(const std::string& propertyName, double value);
        void WriteNumberValue(intcs value);
        void WriteNumberValue(longcs value);
        void WriteNumberValue(double value);

        void WriteBoolean(const std::string& propertyName, bool value);
        void WriteBooleanValue(bool value);

        void WriteNull(const std::string& propertyName);
        void WriteNullValue();

        /** @brief Writes already-formatted JSON text verbatim as the next value. @throws System::InvalidOperationException unless @p skipInputValidation is true or @p json is well-formed JSON. */
        void WriteRawValue(const std::string& json, bool skipInputValidation = false);
    };

} // namespace System::Text::Json

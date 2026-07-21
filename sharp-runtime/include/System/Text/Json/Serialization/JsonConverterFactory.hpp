// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>

namespace System::Text::Json::Serialization {

    /**
     * @brief Supports converting several types by matching against a type name rather than a
     * single, statically-known type.
     *
     * C++ counterpart of .NET System.Text.Json.Serialization.JsonConverterFactory.
     *
     * @note Real .NET dispatches on a runtime `System.Type` (`CanConvert(Type)`/
     * `CreateConverter(Type, options)`); this runtime has no reflection `Type` system (see
     * CLAUDE.md's parity philosophy), so this port dispatches on a caller-supplied type name
     * string instead. `CreateConverter`'s return type can't be expressed uniformly here (real
     * .NET returns a non-generic `JsonConverter` — a `JsonConverter<T>` for whatever `T`
     * `CanConvert` matched — which this port's templated `JsonConverter<T>` can't represent without
     * reflection either); concrete factories should expose their own typed creation method
     * alongside `CanConvert`.
     */
    class JsonConverterFactory {
    public:
        virtual ~JsonConverterFactory() = default;

        /** @return true if this factory can create a converter for the type named @p typeName. */
        [[nodiscard]] virtual bool CanConvert(const std::string& typeName) const = 0;
    };

} // namespace System::Text::Json::Serialization

// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Text::Json {

    /**
     * @brief Defines the various JSON tokens that make up a JSON text.
     *
     * C++ counterpart of .NET System.Text.Json.JsonTokenType.
     */
    enum class JsonTokenType : SharpRuntime::bytecs {
        /** @brief No value has been read yet (as distinct from Null). */
        None = 0,
        /** @brief The start of a JSON object ('{'). */
        StartObject = 1,
        /** @brief The end of a JSON object ('}'). */
        EndObject = 2,
        /** @brief The start of a JSON array ('['). */
        StartArray = 3,
        /** @brief The end of a JSON array (']'). */
        EndArray = 4,
        /** @brief A JSON property name. */
        PropertyName = 5,
        /** @brief A comment string. */
        Comment = 6,
        /** @brief A JSON string. */
        String = 7,
        /** @brief A JSON number. */
        Number = 8,
        /** @brief The JSON literal true. */
        True = 9,
        /** @brief The JSON literal false. */
        False = 10,
        /** @brief The JSON literal null. */
        Null = 11,
    };

} // namespace System::Text::Json

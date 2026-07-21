// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Text::Json::Serialization {

    /**
     * @brief Specifies that a type should have its OnDeserializing() method called before
     * deserialization occurs.
     *
     * C++ counterpart of .NET System.Text.Json.Serialization.IJsonOnDeserializing.
     *
     * @note Not automatically invoked — `JsonSerializer::Deserialize<T>` in this runtime dispatches
     * to `nlohmann::json`'s ADL `from_json` (see JsonSerializer's doc comment on the reflection/
     * source-gen-based member walking this stands in for), which has no hook to call this
     * interface's method for an arbitrary `T`. Reproduced for API/data compatibility with ported
     * code that implements the interface; callers wanting the behavior must invoke it themselves.
     */
    class IJsonOnDeserializing {
    public:
        virtual ~IJsonOnDeserializing() = default;
        /** @brief Called before deserialization. */
        virtual void OnDeserializing() = 0;
    };

    /**
     * @brief Specifies that a type should have its OnDeserialized() method called after
     * deserialization occurs.
     *
     * C++ counterpart of .NET System.Text.Json.Serialization.IJsonOnDeserialized.
     * @note See IJsonOnDeserializing's doc comment — not automatically invoked.
     */
    class IJsonOnDeserialized {
    public:
        virtual ~IJsonOnDeserialized() = default;
        /** @brief Called after deserialization. */
        virtual void OnDeserialized() = 0;
    };

    /**
     * @brief Specifies that a type should have its OnSerializing() method called before
     * serialization occurs.
     *
     * C++ counterpart of .NET System.Text.Json.Serialization.IJsonOnSerializing.
     * @note See IJsonOnDeserializing's doc comment — not automatically invoked.
     */
    class IJsonOnSerializing {
    public:
        virtual ~IJsonOnSerializing() = default;
        /** @brief Called before serialization. */
        virtual void OnSerializing() = 0;
    };

    /**
     * @brief Specifies that a type should have its OnSerialized() method called after
     * serialization occurs.
     *
     * C++ counterpart of .NET System.Text.Json.Serialization.IJsonOnSerialized.
     * @note See IJsonOnDeserializing's doc comment — not automatically invoked.
     */
    class IJsonOnSerialized {
    public:
        virtual ~IJsonOnSerialized() = default;
        /** @brief Called after serialization. */
        virtual void OnSerialized() = 0;
    };

} // namespace System::Text::Json::Serialization

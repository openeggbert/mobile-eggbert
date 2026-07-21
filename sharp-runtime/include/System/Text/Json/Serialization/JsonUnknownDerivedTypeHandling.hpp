// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Text::Json::Serialization {

    /**
     * @brief Defines how objects of a derived runtime type that has not been explicitly declared
     * for polymorphic serialization should be handled.
     *
     * C++ counterpart of .NET System.Text.Json.Serialization.JsonUnknownDerivedTypeHandling.
     */
    enum class JsonUnknownDerivedTypeHandling {
        /** @brief An object of undeclared runtime type will fail polymorphic serialization. */
        FailSerialization = 0,
        /** @brief An object of undeclared runtime type falls back to the base type's contract. */
        FallBackToBaseType = 1,
        /** @brief An object of undeclared runtime type falls back to the nearest declared ancestor type's contract. */
        FallBackToNearestAncestor = 2,
    };

} // namespace System::Text::Json::Serialization

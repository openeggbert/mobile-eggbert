// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/InvalidOperationException.hpp"

namespace System::Text::Json::Serialization {

    /**
     * @brief Defines how a JsonSerializer preserves references across a serialization/deserialization
     * call.
     *
     * C++ counterpart of .NET System.Text.Json.Serialization.ReferenceResolver.
     *
     * @note `object` maps to `const void*` — pointer identity is this port's substitute for .NET's
     * `object.ReferenceEquals`. Not wired into JsonSerializer's `Serialize`/`Deserialize` (see
     * JsonSerializer's doc comment: those dispatch through nlohmann's ADL customization points,
     * which have no hook for $id/$ref metadata injection); usable directly by hand-written
     * converters that need reference-cycle tracking.
     */
    class ReferenceResolver {
    public:
        virtual ~ReferenceResolver() = default;

        /** @brief Adds an (id, value) pair to the bookkeeping of the resolver. */
        virtual void AddReference(const std::string& referenceId, const void* value) = 0;

        /** @return The reference id for @p value, adding a new one if not already tracked. @param alreadyExists Set to true if @p value was already tracked. */
        [[nodiscard]] virtual std::string GetReference(const void* value, bool& alreadyExists) = 0;

        /** @return The value previously associated with @p referenceId. @throws System::InvalidOperationException if not found. */
        [[nodiscard]] virtual const void* ResolveReference(const std::string& referenceId) = 0;
    };

} // namespace System::Text::Json::Serialization

// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Xml {

    /**
     * @brief Specifies how entities are handled by legacy readers (XmlTextReader/XmlValidatingReader).
     *
     * C++ counterpart of .NET System.Xml.EntityHandling.
     */
    enum class EntityHandling {
        /** Expand all entities in place; no EntityReference nodes are returned. */
        ExpandEntities = 1,
        /** Expand character entities only; general entities are returned as EntityReference nodes. */
        ExpandCharEntities = 2,
    };

} // namespace System::Xml

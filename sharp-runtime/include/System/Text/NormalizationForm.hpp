// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Text {

    /** Specifies the Unicode normalization form for string normalization. */
    enum class NormalizationForm {
        /** Canonical decomposition followed by canonical composition. */
        FormC  = 1,
        /** Canonical decomposition. */
        FormD  = 2,
        /** Compatibility decomposition followed by canonical composition. */
        FormKC = 5,
        /** Compatibility decomposition. */
        FormKD = 6,
    };

} // namespace System::Text

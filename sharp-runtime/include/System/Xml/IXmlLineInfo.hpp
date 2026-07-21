// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Xml {

    /**
     * @brief Provides line and position information, typically for error reporting during XML parsing.
     *
     * C++ counterpart of .NET System.Xml.IXmlLineInfo.
     */
    class IXmlLineInfo {
    public:
        virtual ~IXmlLineInfo() = default;

        /** @return @c true if line/position information is available. */
        [[nodiscard]] virtual bool HasLineInfo() const = 0;

        /** @return The current line number, or 0 if unavailable. */
        [[nodiscard]] virtual SharpRuntime::intcs getLineNumberProperty() const = 0;

        /** @return The current line position (column), or 0 if unavailable. */
        [[nodiscard]] virtual SharpRuntime::intcs getLinePositionProperty() const = 0;
    };

} // namespace System::Xml

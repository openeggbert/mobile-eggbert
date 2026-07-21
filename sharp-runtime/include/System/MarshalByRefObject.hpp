// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System {

    /**
     * @brief Enables access to objects across application domain boundaries in applications
     * that support remoting.
     *
     * C++ counterpart of .NET System.MarshalByRefObject.
     * In this port the class serves as a base-class marker; full remoting is not implemented.
     */
    class MarshalByRefObject {
    public:
        /** @brief Virtual destructor to allow safe polymorphic deletion. */
        virtual ~MarshalByRefObject() = default;
    };

} // namespace System

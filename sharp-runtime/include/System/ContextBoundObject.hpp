// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/MarshalByRefObject.hpp"

namespace System {

    /**
     * @brief Defines the base class for all context-bound classes.
     *
     * C++ counterpart of .NET System.ContextBoundObject.
     * Objects of classes derived from ContextBoundObject can only be used within
     * the context in which they were created. In this port the class is a
     * lightweight marker base; full .NET context / remoting semantics are not
     * implemented.
     *
     * @note This class cannot be instantiated directly — only subclasses can be.
     */
    class ContextBoundObject : public MarshalByRefObject {
    protected:
        /**
         * @brief Initializes a new instance of the ContextBoundObject class.
         *
         * C++ counterpart of .NET ContextBoundObject() (protected constructor).
         */
        ContextBoundObject() = default;
    };

} // namespace System

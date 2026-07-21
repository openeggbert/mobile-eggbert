// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System {

    /**
     * @brief Represents the base class for classes that contain event data.
     *
     * C++ counterpart of .NET System.EventArgs.
     * Can be used directly for events that carry no additional data.
     */
    class EventArgs {
    public:
        /**
         * @brief A shared empty instance of EventArgs, for events that carry no data.
         *
         * C++ counterpart of .NET EventArgs.Empty.
         */
        static const EventArgs Empty;

        /** @brief Initializes a new instance of the EventArgs class. */
        EventArgs() = default;

        /** @brief Virtual destructor so derived event-data classes can be safely destroyed through an EventArgs pointer/reference. */
        virtual ~EventArgs() = default;
    };

}
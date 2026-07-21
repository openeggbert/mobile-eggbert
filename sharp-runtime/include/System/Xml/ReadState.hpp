// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Xml {

    /**
     * @brief Specifies the state of an XmlReader.
     *
     * C++ counterpart of .NET System.Xml.ReadState.
     */
    enum class ReadState {
        /** Read() has not yet been called. */
        Initial = 0,
        /** Reading is in progress. */
        Interactive = 1,
        /** An error occurred that prevents the reader from continuing. */
        Error = 2,
        /** The end of the input has been reached successfully. */
        EndOfFile = 3,
        /** Close() has been called. */
        Closed = 4,
    };

} // namespace System::Xml

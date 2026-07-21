// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Net::Http {

    /**
     * @brief Indicates whether HttpClient operations should return as soon as a response
     * is available, or after reading the entire response including content.
     *
     * C++ counterpart of .NET System.Net.Http.HttpCompletionOption.
     */
    enum class HttpCompletionOption {
        /** The operation should complete after reading the entire response, including content. */
        ResponseContentRead = 0,
        /** The operation should complete as soon as a response is available and headers are read; content is not read yet. */
        ResponseHeadersRead,
    };

} // namespace System::Net::Http

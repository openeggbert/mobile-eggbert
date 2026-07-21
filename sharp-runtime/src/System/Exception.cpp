// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Created by robertvokac on 6/5/25.
//

#include "System/Exception.hpp"

namespace System {

    Exception::Exception(const char* msg)
        : message_(msg ? msg : "") {
    }

    Exception::Exception(const std::string& msg)
        : message_(msg) {
    }

    Exception::Exception(const std::string& msg, std::exception_ptr inner)
        : message_(msg), innerException_(std::move(inner)) {
    }

    const std::string& Exception::getMessageProperty() const {
        return message_;
    }

    void Exception::setHResultProperty(SharpRuntime::intcs value) {
        hResult_ = value;
    }

    const char* Exception::what() const noexcept {
        return message_.c_str();
    }

} // namespace System
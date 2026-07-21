// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Created by robertvokac on 6/5/25.
//

#include "System/Exception.hpp"

namespace System {

    Exception::Exception()
        : message_("") {
    }

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

    std::exception_ptr Exception::getInnerExceptionProperty() const {
        return innerException_;
    }

    const std::string& Exception::getStackTraceProperty() const {
        static const std::string empty;
        return empty;
    }

    std::map<std::string, std::string>& Exception::getDataProperty() {
        return data_;
    }

    const std::map<std::string, std::string>& Exception::getDataProperty() const {
        return data_;
    }

    const std::string& Exception::getHelpLinkProperty() const {
        return helpLink_;
    }

    void Exception::setHelpLinkProperty(const std::string& value) {
        helpLink_ = value;
    }

    const std::string& Exception::getSourceProperty() const {
        return source_;
    }

    void Exception::setSourceProperty(const std::string& value) {
        source_ = value;
    }

    SharpRuntime::intcs Exception::getHResultProperty() const {
        return hResult_;
    }

    void Exception::setHResultProperty(SharpRuntime::intcs value) {
        hResult_ = value;
    }

    const char* Exception::what() const noexcept {
        return message_.c_str();
    }

} // namespace System
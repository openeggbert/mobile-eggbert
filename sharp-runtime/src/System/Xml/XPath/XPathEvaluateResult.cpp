// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Xml/XPath/XPathEvaluateResult.hpp"

#include "System/Xml/XPath/XPathNodeIterator.hpp"

namespace System::Xml::XPath {

    XPathEvaluateResult::~XPathEvaluateResult() { delete nodeIterator_; }

    XPathEvaluateResult::XPathEvaluateResult(XPathEvaluateResult&& other) noexcept
        : kind_(other.kind_), number_(other.number_), string_(std::move(other.string_)),
          boolean_(other.boolean_), nodeIterator_(other.nodeIterator_) {
        other.nodeIterator_ = nullptr;
    }

    XPathEvaluateResult& XPathEvaluateResult::operator=(XPathEvaluateResult&& other) noexcept {
        if (this != &other) {
            delete nodeIterator_;
            kind_ = other.kind_;
            number_ = other.number_;
            string_ = std::move(other.string_);
            boolean_ = other.boolean_;
            nodeIterator_ = other.nodeIterator_;
            other.nodeIterator_ = nullptr;
        }
        return *this;
    }

    XPathEvaluateResult XPathEvaluateResult::OfNumber(double value) {
        XPathEvaluateResult r;
        r.kind_ = XPathResultType::Number;
        r.number_ = value;
        return r;
    }

    XPathEvaluateResult XPathEvaluateResult::OfString(std::string value) {
        XPathEvaluateResult r;
        r.kind_ = XPathResultType::String;
        r.string_ = std::move(value);
        return r;
    }

    XPathEvaluateResult XPathEvaluateResult::OfBoolean(bool value) {
        XPathEvaluateResult r;
        r.kind_ = XPathResultType::Boolean;
        r.boolean_ = value;
        return r;
    }

    XPathEvaluateResult XPathEvaluateResult::OfNodeSet(XPathNodeIterator* iterator) {
        XPathEvaluateResult r;
        r.kind_ = XPathResultType::NodeSet;
        r.nodeIterator_ = iterator;
        return r;
    }

    XPathNodeIterator* XPathEvaluateResult::releaseNodeIterator() {
        XPathNodeIterator* it = nodeIterator_;
        nodeIterator_ = nullptr;
        return it;
    }

} // namespace System::Xml::XPath

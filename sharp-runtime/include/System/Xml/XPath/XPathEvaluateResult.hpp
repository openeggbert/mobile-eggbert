// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <string>

#include "System/Xml/XPath/XPathResultType.hpp"

namespace System::Xml::XPath {

    class XPathNodeIterator;

    /**
     * @brief The result of XPathNavigator::Evaluate().
     *
     * C++ approximation of .NET's `object Evaluate(...)`, which returns a boxed double, string,
     * bool, or XPathNodeIterator depending on the expression's XPathResultType. Since C++ has no
     * built-in boxed-object type, this is a small tagged union instead: check
     * getResultTypeProperty() (one of Number/String/Boolean/NodeSet), then read the matching
     * accessor. For NodeSet, ownership of the returned XPathNodeIterator* transfers to the
     * caller (delete it, or extract it via releaseNodeIterator() and store it elsewhere).
     */
    class XPathEvaluateResult {
        XPathResultType kind_;
        double number_ = 0.0;
        std::string string_;
        bool boolean_ = false;
        XPathNodeIterator* nodeIterator_ = nullptr;

        XPathEvaluateResult() : kind_(XPathResultType::Error) {}

    public:
        ~XPathEvaluateResult();
        XPathEvaluateResult(const XPathEvaluateResult&) = delete;
        XPathEvaluateResult& operator=(const XPathEvaluateResult&) = delete;
        XPathEvaluateResult(XPathEvaluateResult&& other) noexcept;
        XPathEvaluateResult& operator=(XPathEvaluateResult&& other) noexcept;

        [[nodiscard]] static XPathEvaluateResult OfNumber(double value);
        [[nodiscard]] static XPathEvaluateResult OfString(std::string value);
        [[nodiscard]] static XPathEvaluateResult OfBoolean(bool value);
        /** @brief Takes ownership of @p iterator. */
        [[nodiscard]] static XPathEvaluateResult OfNodeSet(XPathNodeIterator* iterator);

        /** @return Which accessor is populated: Number, String, Boolean, or NodeSet. */
        [[nodiscard]] XPathResultType getResultTypeProperty() const { return kind_; }

        /** @return The numeric value. Only meaningful if getResultTypeProperty() == Number. */
        [[nodiscard]] double getNumberProperty() const { return number_; }
        /** @return The string value. Only meaningful if getResultTypeProperty() == String. */
        [[nodiscard]] const std::string& getStringProperty() const { return string_; }
        /** @return The boolean value. Only meaningful if getResultTypeProperty() == Boolean. */
        [[nodiscard]] bool getBooleanProperty() const { return boolean_; }
        /** @return The node-set iterator. Only meaningful if getResultTypeProperty() == NodeSet. Owned by this result; do not delete directly (or call releaseNodeIterator() first). */
        [[nodiscard]] XPathNodeIterator* getNodeIteratorProperty() const { return nodeIterator_; }
        /** @brief Releases ownership of the node-set iterator to the caller, who must delete it. @return The iterator (nullptr if this result is not a NodeSet, or ownership was already released). */
        [[nodiscard]] XPathNodeIterator* releaseNodeIterator();
    };

} // namespace System::Xml::XPath

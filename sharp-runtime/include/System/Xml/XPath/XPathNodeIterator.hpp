// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Xml::XPath {

    class XPathNavigator;

    /**
     * @brief A forward-only cursor over a sequence of nodes produced by
     * XPathNavigator::Select()/SelectChildren()/etc.
     *
     * C++ counterpart of .NET System.Xml.XPath.XPathNodeIterator. Usage mirrors .NET:
     * @code
     * XPathNodeIterator* it = navigator->Select("book");
     * while (it->MoveNext()) {
     *     XPathNavigator* current = it->getCurrentProperty();
     *     ...
     * }
     * delete it;
     * @endcode
     * There is no IEnumerable/LINQ surface (per project convention); iterate with MoveNext().
     */
    class XPathNodeIterator {
    public:
        virtual ~XPathNodeIterator() = default;

        /** @return An independent copy of this iterator, positioned at the same point. Caller takes ownership. */
        [[nodiscard]] virtual XPathNodeIterator* Clone() const = 0;

        /** @brief Advances to the next node. @return false if there are no more nodes. */
        virtual bool MoveNext() = 0;

        /** @return The navigator positioned on the current node, or nullptr if MoveNext() has not been called yet (or returned false). Owned by the iterator; do not delete. */
        [[nodiscard]] virtual XPathNavigator* getCurrentProperty() const = 0;

        /** @return The 1-based position of the current node, or 0 if MoveNext() has not been called yet. */
        [[nodiscard]] virtual SharpRuntime::intcs getCurrentPositionProperty() const = 0;

        /** @return The total number of nodes in the selected set. Default implementation clones this iterator and exhausts the clone; override for a cheaper answer when the count is already known. */
        [[nodiscard]] virtual SharpRuntime::intcs getCountProperty() const {
            XPathNodeIterator* clone = Clone();
            while (clone->MoveNext()) {}
            SharpRuntime::intcs result = clone->getCurrentPositionProperty();
            delete clone;
            return result;
        }
    };

} // namespace System::Xml::XPath

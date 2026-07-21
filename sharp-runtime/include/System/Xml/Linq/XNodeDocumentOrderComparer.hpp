// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Xml/Linq/XNode.hpp"

namespace System::Xml::Linq {

    /**
     * @brief Compares nodes for their relative document order. Also usable directly as a
     * strict-weak-ordering predicate (via operator()) for std::sort/std::ranges::sort.
     *
     * C++ counterpart of .NET System.Xml.Linq.XNodeDocumentOrderComparer.
     */
    class XNodeDocumentOrderComparer {
    public:
        /** @return 0 if @p x and @p y are equal; -1 if @p x is before @p y; 1 if @p x is after @p y. */
        [[nodiscard]] SharpRuntime::intcs Compare(const XNode* x, const XNode* y) const {
            return XNode::CompareDocumentOrder(x, y);
        }
        /** @return Same as Compare(const XNode*, const XNode*), for shared_ptr-held nodes. */
        [[nodiscard]] SharpRuntime::intcs Compare(const std::shared_ptr<XNode>& x, const std::shared_ptr<XNode>& y) const {
            return Compare(x.get(), y.get());
        }

        /** @brief Strict-weak-ordering predicate: true if @p x is before @p y in document order. */
        [[nodiscard]] bool operator()(const XNode* x, const XNode* y) const { return Compare(x, y) < 0; }
        /** @brief Strict-weak-ordering predicate, for shared_ptr-held nodes. */
        [[nodiscard]] bool operator()(const std::shared_ptr<XNode>& x, const std::shared_ptr<XNode>& y) const { return Compare(x, y) < 0; }
    };

} // namespace System::Xml::Linq

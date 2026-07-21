// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <cstddef>
#include <memory>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Xml/Linq/XNode.hpp"

namespace System::Xml::Linq {

    /**
     * @brief Compares nodes for value (structural) equality rather than identity. Also usable
     * directly as a std::unordered_set/std::unordered_map equality+hash functor (via operator()).
     *
     * C++ counterpart of .NET System.Xml.Linq.XNodeEqualityComparer.
     *
     * @note A null node equals only another null node. Nodes of different NodeTypes are never
     * equal. See XNode::DeepEquals for the exact per-type comparison rules.
     */
    class XNodeEqualityComparer {
    public:
        /** @return true if @p x and @p y are deeply (structurally) equal. */
        [[nodiscard]] bool Equals(const XNode* x, const XNode* y) const { return XNode::DeepEquals(x, y); }
        /** @return Same as Equals(const XNode*, const XNode*), for shared_ptr-held nodes. */
        [[nodiscard]] bool Equals(const std::shared_ptr<XNode>& x, const std::shared_ptr<XNode>& y) const { return Equals(x.get(), y.get()); }

        /** @return A value-based hash code for @p obj's content (0 if @p obj is nullptr). */
        [[nodiscard]] SharpRuntime::intcs GetHashCode(const XNode* obj) const { return obj != nullptr ? obj->GetDeepHashCode() : 0; }
        /** @return Same as GetHashCode(const XNode*), for a shared_ptr-held node. */
        [[nodiscard]] SharpRuntime::intcs GetHashCode(const std::shared_ptr<XNode>& obj) const { return GetHashCode(obj.get()); }

        /** @brief Equality functor form, for std::unordered_set/std::unordered_map. */
        [[nodiscard]] bool operator()(const std::shared_ptr<XNode>& x, const std::shared_ptr<XNode>& y) const { return Equals(x, y); }
        /** @brief Hash functor form, for std::unordered_set/std::unordered_map. */
        [[nodiscard]] std::size_t operator()(const std::shared_ptr<XNode>& x) const { return static_cast<std::size_t>(GetHashCode(x)); }
    };

} // namespace System::Xml::Linq

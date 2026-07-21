// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <algorithm>
#include <concepts>
#include <memory>
#include <ranges>
#include <vector>
#include "System/Xml/Linq/XAttribute.hpp"
#include "System/Xml/Linq/XContainer.hpp"
#include "System/Xml/Linq/XElement.hpp"
#include "System/Xml/Linq/XNode.hpp"
#include "System/Xml/Linq/XNodeDocumentOrderComparer.hpp"

namespace System::Xml::Linq::Extensions {

    /**
     * @brief Free-function reimplementations of .NET System.Xml.Linq.Extensions — the
     * LINQ-to-XML "query a whole sequence of elements/nodes at once" helpers.
     *
     * C++ has no LINQ (per CLAUDE.md); these are plain std::ranges-friendly template functions
     * over any range whose value_type is a `std::shared_ptr` to XContainer/XElement/XNode/
     * XAttribute (or a type implicitly convertible to one, e.g. a range of shared_ptr<XElement>
     * satisfies the XContainer-constrained overloads). Each returns a materialized
     * std::vector (matching this codebase's existing query-method convention, e.g.
     * XContainer::Elements()) rather than a lazy view.
     *
     * @note Scoped to what maps cleanly onto this port's node model: Elements/Attributes/Nodes/
     * DescendantNodes/Descendants/Ancestors/AncestorsAndSelf/Remove/InDocumentOrder. Real .NET's
     * DescendantsAndSelf/DescendantNodesAndSelf (only meaningful for XElement, not XDocument)
     * are intentionally not duplicated here — call XContainer::Descendants()/DescendantNodes()
     * directly plus include the source item itself if needed.
     */

    /** @return All child elements of every container in @p source, in source order (each container's own elements in document order). */
    template <std::ranges::input_range R>
    [[nodiscard]] std::vector<std::shared_ptr<XElement>> Elements(const R& source)
        requires std::convertible_to<std::ranges::range_value_t<R>, std::shared_ptr<XContainer>>
    {
        std::vector<std::shared_ptr<XElement>> result;
        for (const auto& c : source) {
            if (!c) continue;
            auto es = c->Elements();
            result.insert(result.end(), es.begin(), es.end());
        }
        return result;
    }

    /** @return As Elements(source), filtered to elements named @p name. */
    template <std::ranges::input_range R>
    [[nodiscard]] std::vector<std::shared_ptr<XElement>> Elements(const R& source, const XName& name)
        requires std::convertible_to<std::ranges::range_value_t<R>, std::shared_ptr<XContainer>>
    {
        std::vector<std::shared_ptr<XElement>> result;
        for (const std::shared_ptr<XContainer>& c : source) {
            if (!c) continue;
            auto es = c->Elements(name);
            result.insert(result.end(), es.begin(), es.end());
        }
        return result;
    }

    /** @return All attributes of every element in @p source, in source order. */
    template <std::ranges::input_range R>
    [[nodiscard]] std::vector<std::shared_ptr<XAttribute>> Attributes(const R& source)
        requires std::convertible_to<std::ranges::range_value_t<R>, std::shared_ptr<XElement>>
    {
        std::vector<std::shared_ptr<XAttribute>> result;
        for (const std::shared_ptr<XElement>& e : source) {
            if (!e) continue;
            auto as = e->Attributes();
            result.insert(result.end(), as.begin(), as.end());
        }
        return result;
    }

    /** @return As Attributes(source), filtered to attributes named @p name. */
    template <std::ranges::input_range R>
    [[nodiscard]] std::vector<std::shared_ptr<XAttribute>> Attributes(const R& source, const XName& name)
        requires std::convertible_to<std::ranges::range_value_t<R>, std::shared_ptr<XElement>>
    {
        std::vector<std::shared_ptr<XAttribute>> result;
        for (const std::shared_ptr<XElement>& e : source) {
            if (!e) continue;
            auto a = e->Attribute(name);
            if (a) result.push_back(a);
        }
        return result;
    }

    /** @return All child nodes of every container in @p source, in source order. */
    template <std::ranges::input_range R>
    [[nodiscard]] std::vector<std::shared_ptr<XNode>> Nodes(const R& source)
        requires std::convertible_to<std::ranges::range_value_t<R>, std::shared_ptr<XContainer>>
    {
        std::vector<std::shared_ptr<XNode>> result;
        for (const std::shared_ptr<XContainer>& c : source) {
            if (!c) continue;
            auto ns = c->Nodes();
            result.insert(result.end(), ns.begin(), ns.end());
        }
        return result;
    }

    /** @return All descendant nodes of every container in @p source. */
    template <std::ranges::input_range R>
    [[nodiscard]] std::vector<std::shared_ptr<XNode>> DescendantNodes(const R& source)
        requires std::convertible_to<std::ranges::range_value_t<R>, std::shared_ptr<XContainer>>
    {
        std::vector<std::shared_ptr<XNode>> result;
        for (const std::shared_ptr<XContainer>& c : source) {
            if (!c) continue;
            auto ns = c->DescendantNodes();
            result.insert(result.end(), ns.begin(), ns.end());
        }
        return result;
    }

    /** @return All descendant elements of every container in @p source. */
    template <std::ranges::input_range R>
    [[nodiscard]] std::vector<std::shared_ptr<XElement>> Descendants(const R& source)
        requires std::convertible_to<std::ranges::range_value_t<R>, std::shared_ptr<XContainer>>
    {
        std::vector<std::shared_ptr<XElement>> result;
        for (const std::shared_ptr<XContainer>& c : source) {
            if (!c) continue;
            auto ds = c->Descendants();
            result.insert(result.end(), ds.begin(), ds.end());
        }
        return result;
    }

    /** @return As Descendants(source), filtered to elements named @p name. */
    template <std::ranges::input_range R>
    [[nodiscard]] std::vector<std::shared_ptr<XElement>> Descendants(const R& source, const XName& name)
        requires std::convertible_to<std::ranges::range_value_t<R>, std::shared_ptr<XContainer>>
    {
        std::vector<std::shared_ptr<XElement>> result;
        for (const auto& c : source) {
            if (!c) continue;
            auto ds = c->Descendants(name);
            result.insert(result.end(), ds.begin(), ds.end());
        }
        return result;
    }

    /**
     * @return The ancestor elements (parent, grandparent, ... up to the root) of every node in
     * @p source. Does not include the nodes themselves. Returned as raw XElement* (non-owning),
     * matching XObject::getParentProperty()'s existing raw-pointer convention — the tree already
     * owns these elements.
     */
    template <std::ranges::input_range R>
    [[nodiscard]] std::vector<XElement*> Ancestors(const R& source)
        requires std::convertible_to<std::ranges::range_value_t<R>, std::shared_ptr<XNode>>
    {
        std::vector<XElement*> result;
        for (const std::shared_ptr<XNode>& n : source) {
            if (!n) continue;
            for (XElement* e = n->getParentProperty(); e != nullptr; e = e->getParentProperty()) {
                result.push_back(e);
            }
        }
        return result;
    }

    /** @return As Ancestors(source), filtered to ancestor elements named @p name. */
    template <std::ranges::input_range R>
    [[nodiscard]] std::vector<XElement*> Ancestors(const R& source, const XName& name)
        requires std::convertible_to<std::ranges::range_value_t<R>, std::shared_ptr<XNode>>
    {
        std::vector<XElement*> result;
        for (const std::shared_ptr<XNode>& n : source) {
            if (!n) continue;
            for (XElement* e = n->getParentProperty(); e != nullptr; e = e->getParentProperty()) {
                if (e->getNameProperty() == name) result.push_back(e);
            }
        }
        return result;
    }

    /**
     * @return As Ancestors(source), but each source element is itself included first (before its
     * own ancestors), matching real .NET's `AncestorsAndSelf`. Only defined for a source of
     * XElement (not any XNode), matching real .NET's overload constraint — "self" only makes
     * sense when the source item is itself an element.
     */
    template <std::ranges::input_range R>
    [[nodiscard]] std::vector<XElement*> AncestorsAndSelf(const R& source)
        requires std::convertible_to<std::ranges::range_value_t<R>, std::shared_ptr<XElement>>
    {
        std::vector<XElement*> result;
        for (const std::shared_ptr<XElement>& el : source) {
            if (!el) continue;
            for (XElement* e = el.get(); e != nullptr; e = e->getParentProperty()) {
                result.push_back(e);
            }
        }
        return result;
    }

    /** @return As AncestorsAndSelf(source), filtered to elements named @p name. */
    template <std::ranges::input_range R>
    [[nodiscard]] std::vector<XElement*> AncestorsAndSelf(const R& source, const XName& name)
        requires std::convertible_to<std::ranges::range_value_t<R>, std::shared_ptr<XElement>>
    {
        std::vector<XElement*> result;
        for (const std::shared_ptr<XElement>& el : source) {
            if (!el) continue;
            for (XElement* e = el.get(); e != nullptr; e = e->getParentProperty()) {
                if (e->getNameProperty() == name) result.push_back(e);
            }
        }
        return result;
    }

    /** @brief Removes each attribute in @p source (snapshot semantics: the source is materialized before any removal). */
    template <std::ranges::input_range R>
    void Remove(const R& source)
        requires std::convertible_to<std::ranges::range_value_t<R>, std::shared_ptr<XAttribute>>
    {
        std::vector<std::shared_ptr<XAttribute>> snapshot(std::ranges::begin(source), std::ranges::end(source));
        for (auto& a : snapshot) {
            if (a) a->Remove();
        }
    }

    /**
     * @brief Removes each node in @p source (snapshot semantics: the source is materialized
     * before any removal). Overloaded on the range's value type against the XAttribute overload
     * above (mutually exclusive constraints — XAttribute does not derive from XNode — so there's
     * no ambiguity), matching .NET's `Extensions.Remove` overload set.
     */
    template <std::ranges::input_range R>
    void Remove(const R& source)
        requires std::convertible_to<std::ranges::range_value_t<R>, std::shared_ptr<XNode>>
    {
        std::vector<std::shared_ptr<XNode>> snapshot(std::ranges::begin(source), std::ranges::end(source));
        for (auto& n : snapshot) {
            if (n) n->Remove();
        }
    }

    /** @return The nodes of @p source sorted into document order. @throws System::InvalidOperationException if any two nodes do not share a common ancestor. */
    template <std::ranges::input_range R>
    [[nodiscard]] std::vector<std::shared_ptr<XNode>> InDocumentOrder(const R& source)
        requires std::convertible_to<std::ranges::range_value_t<R>, std::shared_ptr<XNode>>
    {
        std::vector<std::shared_ptr<XNode>> result(std::ranges::begin(source), std::ranges::end(source));
        std::ranges::sort(result, XNodeDocumentOrderComparer{});
        return result;
    }

} // namespace System::Xml::Linq::Extensions

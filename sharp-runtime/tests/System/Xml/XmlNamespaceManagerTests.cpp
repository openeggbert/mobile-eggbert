// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/ArgumentException.hpp"
#include "System/Xml/NameTable.hpp"
#include "System/Xml/XmlNamespaceManager.hpp"

using namespace System::Xml;

namespace {
    std::shared_ptr<XmlNameTable> NewNameTable() {
        return std::make_shared<NameTable>();
    }
}

TEST(XmlNamespaceManagerTests, DefaultNamespace_InitiallyEmpty) {
    XmlNamespaceManager mgr(NewNameTable());
    EXPECT_EQ(mgr.getDefaultNamespaceProperty(), "");
}

TEST(XmlNamespaceManagerTests, PredefinedXmlPrefix_ResolvesToFixedNamespace) {
    XmlNamespaceManager mgr(NewNameTable());
    auto ns = mgr.LookupNamespace("xml");
    ASSERT_TRUE(ns.has_value());
    EXPECT_EQ(*ns, "http://www.w3.org/XML/1998/namespace");
}

TEST(XmlNamespaceManagerTests, AddNamespace_ThenLookupNamespace_Resolves) {
    XmlNamespaceManager mgr(NewNameTable());
    mgr.AddNamespace("foo", "urn:foo");
    auto ns = mgr.LookupNamespace("foo");
    ASSERT_TRUE(ns.has_value());
    EXPECT_EQ(*ns, "urn:foo");
}

TEST(XmlNamespaceManagerTests, AddNamespace_XmlnsPrefix_Throws) {
    XmlNamespaceManager mgr(NewNameTable());
    EXPECT_THROW(mgr.AddNamespace("xmlns", "urn:foo"), System::ArgumentException);
}

TEST(XmlNamespaceManagerTests, AddNamespace_XmlPrefixWithWrongUri_Throws) {
    XmlNamespaceManager mgr(NewNameTable());
    EXPECT_THROW(mgr.AddNamespace("xml", "urn:bogus"), System::ArgumentException);
}

TEST(XmlNamespaceManagerTests, AddNamespace_XmlPrefixWithCorrectUri_Allowed) {
    XmlNamespaceManager mgr(NewNameTable());
    EXPECT_NO_THROW(mgr.AddNamespace("xml", "http://www.w3.org/XML/1998/namespace"));
}

TEST(XmlNamespaceManagerTests, LookupPrefix_FindsMatchingUri) {
    XmlNamespaceManager mgr(NewNameTable());
    mgr.AddNamespace("foo", "urn:foo");
    auto prefix = mgr.LookupPrefix("urn:foo");
    ASSERT_TRUE(prefix.has_value());
    EXPECT_EQ(*prefix, "foo");
}

TEST(XmlNamespaceManagerTests, LookupNamespace_UnknownPrefix_ReturnsNullopt) {
    XmlNamespaceManager mgr(NewNameTable());
    EXPECT_FALSE(mgr.LookupNamespace("nope").has_value());
}

TEST(XmlNamespaceManagerTests, PushScope_ThenAddNamespace_ShadowsOuterOnly_WithinScope) {
    XmlNamespaceManager mgr(NewNameTable());
    mgr.AddNamespace("foo", "urn:foo-outer");
    mgr.PushScope();
    mgr.AddNamespace("foo", "urn:foo-inner");
    EXPECT_EQ(*mgr.LookupNamespace("foo"), "urn:foo-inner");
    mgr.PopScope();
    EXPECT_EQ(*mgr.LookupNamespace("foo"), "urn:foo-outer");
}

TEST(XmlNamespaceManagerTests, PopScope_AtRootScope_ReturnsFalse) {
    XmlNamespaceManager mgr(NewNameTable());
    EXPECT_FALSE(mgr.PopScope());
}

TEST(XmlNamespaceManagerTests, HasNamespace_ChecksCurrentScopeOnly) {
    XmlNamespaceManager mgr(NewNameTable());
    mgr.AddNamespace("foo", "urn:foo");
    EXPECT_TRUE(mgr.HasNamespace("foo"));
    mgr.PushScope();
    EXPECT_FALSE(mgr.HasNamespace("foo"));
}

TEST(XmlNamespaceManagerTests, RemoveNamespace_MatchingPrefixAndUri_Removes) {
    XmlNamespaceManager mgr(NewNameTable());
    mgr.AddNamespace("foo", "urn:foo");
    mgr.RemoveNamespace("foo", "urn:foo");
    EXPECT_FALSE(mgr.LookupNamespace("foo").has_value());
}

TEST(XmlNamespaceManagerTests, GetNamespacesInScope_All_IncludesXmlButNotXmlnsOrEmpty) {
    XmlNamespaceManager mgr(NewNameTable());
    mgr.AddNamespace("foo", "urn:foo");
    auto scope = mgr.GetNamespacesInScope(XmlNamespaceScope::All);
    EXPECT_EQ(scope.count("xml"), 1u);
    EXPECT_EQ(scope.count("xmlns"), 0u);
    EXPECT_EQ(scope.count(""), 0u);
    EXPECT_EQ(scope.at("foo"), "urn:foo");
}

TEST(XmlNamespaceManagerTests, GetNamespacesInScope_ExcludeXml_OmitsXml) {
    XmlNamespaceManager mgr(NewNameTable());
    auto scope = mgr.GetNamespacesInScope(XmlNamespaceScope::ExcludeXml);
    EXPECT_EQ(scope.count("xml"), 0u);
}

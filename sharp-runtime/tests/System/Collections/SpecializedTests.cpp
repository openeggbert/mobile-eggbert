// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include <any>
#include <string>
#include <vector>
#include "System/ArgumentException.hpp"
#include "System/Collections/Specialized/NameValueCollection.hpp"
#include "System/Collections/Specialized/StringCollection.hpp"
#include "System/Collections/Specialized/BitVector32.hpp"
#include "System/Collections/Specialized/HybridDictionary.hpp"
#include "System/Collections/Specialized/ListDictionary.hpp"
#include "System/Collections/Specialized/StringDictionary.hpp"
#include "System/Collections/Specialized/OrderedDictionary.hpp"

using System::Collections::Specialized::NameValueCollection;
using System::Collections::Specialized::StringCollection;
using System::Collections::Specialized::BitVector32;
using System::Collections::Specialized::HybridDictionary;
using System::Collections::Specialized::ListDictionary;
using System::Collections::Specialized::StringDictionary;
using System::Collections::Specialized::OrderedDictionary;

// ===========================================================================
// NameValueCollection
// ===========================================================================

TEST(NameValueCollectionTests, DefaultCtor_IsEmpty) {
    NameValueCollection nvc;
    EXPECT_EQ(nvc.getCountProperty(), 0);
    EXPECT_FALSE(nvc.HasKeys());
}

TEST(NameValueCollectionTests, Add_IncreasesCount) {
    NameValueCollection nvc;
    nvc.Add("key", "value");
    EXPECT_EQ(nvc.getCountProperty(), 1);
    EXPECT_TRUE(nvc.HasKeys());
}

TEST(NameValueCollectionTests, Get_ReturnsFirstValue) {
    NameValueCollection nvc;
    nvc.Add("name", "Alice");
    EXPECT_EQ(nvc.Get("name"), "Alice");
}

TEST(NameValueCollectionTests, Get_MissingKey_ReturnsEmpty) {
    NameValueCollection nvc;
    EXPECT_EQ(nvc.Get("missing"), "");
}

TEST(NameValueCollectionTests, Add_MultipleValues_SameKey) {
    NameValueCollection nvc;
    nvc.Add("tag", "a");
    nvc.Add("tag", "b");
    auto vals = nvc.GetValues("tag");
    EXPECT_EQ(vals.size(), 2u);
    EXPECT_EQ(vals[0], "a");
    EXPECT_EQ(vals[1], "b");
}

TEST(NameValueCollectionTests, Set_OverwritesAllValues) {
    NameValueCollection nvc;
    nvc.Add("k", "old1");
    nvc.Add("k", "old2");
    nvc.Set("k", "new");
    EXPECT_EQ(nvc.Get("k"), "new");
    EXPECT_EQ(nvc.GetValues("k").size(), 1u);
}

TEST(NameValueCollectionTests, Remove_RemovesKey) {
    NameValueCollection nvc;
    nvc.Add("a", "1");
    nvc.Add("b", "2");
    nvc.Remove("a");
    EXPECT_EQ(nvc.getCountProperty(), 1);
    EXPECT_EQ(nvc.Get("a"), "");
}

TEST(NameValueCollectionTests, Clear_EmptiesCollection) {
    NameValueCollection nvc;
    nvc.Add("x", "y");
    nvc.Clear();
    EXPECT_EQ(nvc.getCountProperty(), 0);
}

TEST(NameValueCollectionTests, GetKey_ReturnsKeyByIndex) {
    NameValueCollection nvc;
    nvc.Add("first", "v");
    EXPECT_EQ(nvc.GetKey(0), "first");
}

TEST(NameValueCollectionTests, AllKeys_ReturnsList) {
    NameValueCollection nvc;
    nvc.Add("p", "1");
    nvc.Add("q", "2");
    EXPECT_EQ(nvc.AllKeys().size(), 2u);
}

TEST(NameValueCollectionTests, OperatorBracket_ByName) {
    NameValueCollection nvc;
    nvc.Add("city", "Prague");
    EXPECT_EQ(nvc["city"], "Prague");
}

TEST(NameValueCollectionTests, Get_MultipleValues_CommaJoined) {
    NameValueCollection nvc;
    nvc.Add("tag", "a");
    nvc.Add("tag", "b");
    nvc.Add("tag", "c");
    EXPECT_EQ(nvc.Get("tag"), "a,b,c");
}

TEST(NameValueCollectionTests, Get_ByIndex_ReturnsCommaJoined) {
    NameValueCollection nvc;
    nvc.Add("colors", "red");
    nvc.Add("colors", "blue");
    EXPECT_EQ(nvc.Get(0), "red,blue");
}

TEST(NameValueCollectionTests, GetValues_ByIndex) {
    NameValueCollection nvc;
    nvc.Add("x", "1");
    nvc.Add("x", "2");
    auto vals = nvc.GetValues(0);
    ASSERT_EQ(vals.size(), 2u);
    EXPECT_EQ(vals[0], "1");
    EXPECT_EQ(vals[1], "2");
}

TEST(NameValueCollectionTests, OperatorBracket_ByIndex_CommaJoined) {
    NameValueCollection nvc;
    nvc.Add("k", "p");
    nvc.Add("k", "q");
    EXPECT_EQ(nvc[0], "p,q");
}

TEST(NameValueCollectionTests, Add_OtherCollection) {
    NameValueCollection a, b;
    a.Add("foo", "1");
    b.Add("bar", "2");
    b.Add("foo", "extra");
    a.Add(b);
    EXPECT_EQ(a.getCountProperty(), 2);
    EXPECT_EQ(a.Get("foo"), "1,extra");
    EXPECT_EQ(a.Get("bar"), "2");
}

// ===========================================================================
// StringCollection
// ===========================================================================

TEST(StringCollectionTests, DefaultCtor_IsEmpty) {
    StringCollection sc;
    EXPECT_EQ(sc.getCountProperty(), 0);
}

TEST(StringCollectionTests, Add_IncreasesCount) {
    StringCollection sc;
    sc.Add("hello");
    sc.Add("world");
    EXPECT_EQ(sc.getCountProperty(), 2);
}

TEST(StringCollectionTests, Contains_True_False) {
    StringCollection sc;
    sc.Add("foo");
    EXPECT_TRUE(sc.Contains("foo"));
    EXPECT_FALSE(sc.Contains("bar"));
}

TEST(StringCollectionTests, IndexOf_Found_NotFound) {
    StringCollection sc;
    sc.Add("a");
    sc.Add("b");
    EXPECT_EQ(sc.IndexOf("b"), 1);
    EXPECT_EQ(sc.IndexOf("z"), -1);
}

TEST(StringCollectionTests, OperatorBracket_Access) {
    StringCollection sc;
    sc.Add("x");
    sc.Add("y");
    EXPECT_EQ(sc[0], "x");
    EXPECT_EQ(sc[1], "y");
}

TEST(StringCollectionTests, Remove_RemovesFirst) {
    StringCollection sc;
    sc.Add("a");
    sc.Add("b");
    sc.Remove("a");
    EXPECT_EQ(sc.getCountProperty(), 1);
    EXPECT_EQ(sc[0], "b");
}

TEST(StringCollectionTests, RemoveAt_ByIndex) {
    StringCollection sc;
    sc.Add("one");
    sc.Add("two");
    sc.Add("three");
    sc.RemoveAt(1);
    EXPECT_EQ(sc.getCountProperty(), 2);
    EXPECT_EQ(sc[1], "three");
}

TEST(StringCollectionTests, Clear_EmptiesCollection) {
    StringCollection sc;
    sc.Add("item");
    sc.Clear();
    EXPECT_EQ(sc.getCountProperty(), 0);
}

TEST(StringCollectionTests, AddRange_AddsMultiple) {
    StringCollection sc;
    sc.AddRange({"alpha", "beta", "gamma"});
    EXPECT_EQ(sc.getCountProperty(), 3);
}

TEST(StringCollectionTests, Insert_AtIndex) {
    StringCollection sc;
    sc.Add("a");
    sc.Add("c");
    sc.Insert(1, "b");
    EXPECT_EQ(sc[1], "b");
    EXPECT_EQ(sc.getCountProperty(), 3);
}

// ===========================================================================
// BitVector32
// ===========================================================================

TEST(BitVector32Tests, DefaultCtor_DataIsZero) {
    BitVector32 bv;
    EXPECT_EQ(bv.getDataProperty(), 0u);
}

TEST(BitVector32Tests, IntCtor_StoresData) {
    BitVector32 bv(0b1010);
    EXPECT_EQ(bv.getDataProperty(), 0b1010u);
}

TEST(BitVector32Tests, SetBit_True) {
    BitVector32 bv;
    bv.set(1, true);
    EXPECT_TRUE(bv[1]);
}

TEST(BitVector32Tests, SetBit_False) {
    BitVector32 bv(0xFF);
    bv.set(1, false);
    EXPECT_FALSE(bv[1]);
}

TEST(BitVector32Tests, CreateMask_First) {
    EXPECT_EQ(BitVector32::CreateMask(), 1);
}

TEST(BitVector32Tests, CreateMask_Subsequent) {
    int m1 = BitVector32::CreateMask();
    int m2 = BitVector32::CreateMask(m1);
    EXPECT_EQ(m2, 2);
    int m3 = BitVector32::CreateMask(m2);
    EXPECT_EQ(m3, 4);
}

TEST(BitVector32Tests, Section_SetAndGet) {
    BitVector32 bv;
    auto s = BitVector32::CreateSection(7);
    bv.set(s, 5);
    EXPECT_EQ(bv[s], 5);
}

TEST(BitVector32Tests, ToString_ContainsBitVector32) {
    BitVector32 bv;
    EXPECT_NE(bv.ToString().find("BitVector32"), std::string::npos);
}

TEST(BitVector32Tests, Equality_SameData) {
    BitVector32 a(42), b(42);
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
}

// ===========================================================================
// HybridDictionary
// ===========================================================================

TEST(HybridDictionaryTests, DefaultCtor_IsEmpty) {
    HybridDictionary hd;
    EXPECT_EQ(hd.getCountProperty(), 0);
}

TEST(HybridDictionaryTests, Add_Contains) {
    HybridDictionary hd;
    hd.Add("key", std::any{42});
    EXPECT_TRUE(hd.Contains("key"));
    EXPECT_EQ(hd.getCountProperty(), 1);
}

TEST(HybridDictionaryTests, Remove_RemovesEntry) {
    HybridDictionary hd;
    hd.Add("x", std::any{1});
    hd.Remove("x");
    EXPECT_FALSE(hd.Contains("x"));
}

TEST(HybridDictionaryTests, Clear_EmptiesMap) {
    HybridDictionary hd;
    hd.Add("a", std::any{1});
    hd.Clear();
    EXPECT_EQ(hd.getCountProperty(), 0);
}

TEST(HybridDictionaryTests, OperatorBracket_ReturnsValue) {
    HybridDictionary hd;
    hd.Add("n", std::any{100});
    auto v = hd["n"];
    EXPECT_EQ(std::any_cast<int>(v), 100);
}

// ===========================================================================
// ListDictionary
// ===========================================================================

TEST(ListDictionaryTests, DefaultCtor_IsEmpty) {
    ListDictionary ld;
    EXPECT_EQ(ld.getCountProperty(), 0);
    EXPECT_TRUE(ld.getIsEmptyProperty());
}

TEST(ListDictionaryTests, Add_Contains) {
    ListDictionary ld;
    ld.Add("k", std::any{7});
    EXPECT_TRUE(ld.Contains("k"));
    EXPECT_EQ(ld.getCountProperty(), 1);
}

TEST(ListDictionaryTests, Add_DuplicateKey_Throws) {
    ListDictionary ld;
    ld.Add("k", std::any{1});
    EXPECT_THROW(ld.Add("k", std::any{2}), System::ArgumentException);
}

TEST(ListDictionaryTests, Remove_RemovesEntry) {
    ListDictionary ld;
    ld.Add("x", std::any{1});
    ld.Remove("x");
    EXPECT_FALSE(ld.Contains("x"));
}

TEST(ListDictionaryTests, Clear_EmptiesList) {
    ListDictionary ld;
    ld.Add("a", std::any{1});
    ld.Clear();
    EXPECT_EQ(ld.getCountProperty(), 0);
}

TEST(ListDictionaryTests, OperatorBracket_ReadValue) {
    ListDictionary ld;
    ld.Add("m", std::any{55});
    auto v = ld["m"];
    EXPECT_EQ(std::any_cast<int>(v), 55);
}

// ===========================================================================
// StringDictionary
// ===========================================================================

TEST(StringDictionaryTests, DefaultCtor_IsEmpty) {
    StringDictionary sd;
    EXPECT_EQ(sd.getCountProperty(), 0);
}

TEST(StringDictionaryTests, Add_ContainsKey) {
    StringDictionary sd;
    sd.Add("Name", "Alice");
    EXPECT_TRUE(sd.ContainsKey("name"));
    EXPECT_TRUE(sd.ContainsKey("NAME"));
}

TEST(StringDictionaryTests, GetValue_CaseInsensitive) {
    StringDictionary sd;
    sd.Add("City", "Prague");
    EXPECT_EQ(sd.GetValue("CITY"), "Prague");
    EXPECT_EQ(sd.GetValue("city"), "Prague");
}

TEST(StringDictionaryTests, ContainsValue) {
    StringDictionary sd;
    sd.Add("key", "hello");
    EXPECT_TRUE(sd.ContainsValue("hello"));
    EXPECT_FALSE(sd.ContainsValue("world"));
}

TEST(StringDictionaryTests, Remove_RemovesKey) {
    StringDictionary sd;
    sd.Add("k", "v");
    sd.Remove("K");
    EXPECT_FALSE(sd.ContainsKey("k"));
}

TEST(StringDictionaryTests, Clear_EmptiesMap) {
    StringDictionary sd;
    sd.Add("x", "y");
    sd.Clear();
    EXPECT_EQ(sd.getCountProperty(), 0);
}

// ===========================================================================
// OrderedDictionary
// ===========================================================================

TEST(OrderedDictionaryTests, DefaultCtor_IsEmpty) {
    OrderedDictionary od;
    EXPECT_EQ(od.getCountProperty(), 0);
}

TEST(OrderedDictionaryTests, Add_IncreasesCount) {
    OrderedDictionary od;
    od.Add("a", "1");
    od.Add("b", "2");
    EXPECT_EQ(od.getCountProperty(), 2);
}

TEST(OrderedDictionaryTests, Add_DuplicateKey_Throws) {
    OrderedDictionary od;
    od.Add("k", "v");
    EXPECT_THROW(od.Add("k", "v2"), System::ArgumentException);
}

TEST(OrderedDictionaryTests, Contains_True_False) {
    OrderedDictionary od;
    od.Add("x", "1");
    EXPECT_TRUE(od.Contains("x"));
    EXPECT_FALSE(od.Contains("y"));
}

TEST(OrderedDictionaryTests, OperatorBracket_ReadByKey) {
    OrderedDictionary od;
    od.Add("city", "Brno");
    EXPECT_EQ(od["city"], "Brno");
}

TEST(OrderedDictionaryTests, GetByIndex_PreservesInsertionOrder) {
    OrderedDictionary od;
    od.Add("first", "1");
    od.Add("second", "2");
    od.Add("third", "3");
    EXPECT_EQ(od.GetByIndex(0), "1");
    EXPECT_EQ(od.GetByIndex(1), "2");
    EXPECT_EQ(od.GetByIndex(2), "3");
}

TEST(OrderedDictionaryTests, GetKey_ReturnsKeyByIndex) {
    OrderedDictionary od;
    od.Add("alpha", "v");
    EXPECT_EQ(od.GetKey(0), "alpha");
}

TEST(OrderedDictionaryTests, Remove_RemovesEntry) {
    OrderedDictionary od;
    od.Add("a", "1");
    od.Add("b", "2");
    od.Remove("a");
    EXPECT_EQ(od.getCountProperty(), 1);
    EXPECT_FALSE(od.Contains("a"));
}

TEST(OrderedDictionaryTests, RemoveAt_RemovesByIndex) {
    OrderedDictionary od;
    od.Add("x", "1");
    od.Add("y", "2");
    od.RemoveAt(0);
    EXPECT_EQ(od.getCountProperty(), 1);
    EXPECT_FALSE(od.Contains("x"));
    EXPECT_TRUE(od.Contains("y"));
}

TEST(OrderedDictionaryTests, Clear_EmptiesDictionary) {
    OrderedDictionary od;
    od.Add("a", "1");
    od.Clear();
    EXPECT_EQ(od.getCountProperty(), 0);
}

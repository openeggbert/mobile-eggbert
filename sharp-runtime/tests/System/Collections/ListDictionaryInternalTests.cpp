// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Collections/ListDictionaryInternal.hpp"
#include "System/ArgumentException.hpp"
#include "System/InvalidOperationException.hpp"

using namespace System::Collections;

TEST(ListDictionaryInternalTest, DefaultEmpty) {
    ListDictionaryInternal d;
    EXPECT_EQ(d.getCountProperty(), 0);
}

TEST(ListDictionaryInternalTest, AddAndCount) {
    ListDictionaryInternal d;
    int k1 = 1, v1 = 10;
    d.Add(&k1, &v1);
    EXPECT_EQ(d.getCountProperty(), 1);
}

TEST(ListDictionaryInternalTest, AddDuplicateThrows) {
    ListDictionaryInternal d;
    int k = 1, v = 2;
    d.Add(&k, &v);
    EXPECT_THROW(d.Add(&k, &v), System::ArgumentException);
}

TEST(ListDictionaryInternalTest, GetItemFound) {
    ListDictionaryInternal d;
    int k = 42, v = 99;
    d.Add(&k, &v);
    EXPECT_EQ(d.getItem(&k), &v);
}

TEST(ListDictionaryInternalTest, GetItemNotFound) {
    ListDictionaryInternal d;
    int k = 1;
    EXPECT_EQ(d.getItem(&k), nullptr);
}

TEST(ListDictionaryInternalTest, SetItemUpdatesExisting) {
    ListDictionaryInternal d;
    int k = 1, v1 = 10, v2 = 20;
    d.Add(&k, &v1);
    d.setItem(&k, &v2);
    EXPECT_EQ(d.getItem(&k), &v2);
    EXPECT_EQ(d.getCountProperty(), 1);
}

TEST(ListDictionaryInternalTest, SetItemAddsNew) {
    ListDictionaryInternal d;
    int k = 7, v = 77;
    d.setItem(&k, &v);
    EXPECT_EQ(d.getItem(&k), &v);
    EXPECT_EQ(d.getCountProperty(), 1);
}

TEST(ListDictionaryInternalTest, Contains) {
    ListDictionaryInternal d;
    int k1 = 1, k2 = 2, v = 0;
    d.Add(&k1, &v);
    EXPECT_TRUE(d.Contains(&k1));
    EXPECT_FALSE(d.Contains(&k2));
}

TEST(ListDictionaryInternalTest, Remove) {
    ListDictionaryInternal d;
    int k = 5, v = 50;
    d.Add(&k, &v);
    d.Remove(&k);
    EXPECT_EQ(d.getCountProperty(), 0);
    EXPECT_FALSE(d.Contains(&k));
}

TEST(ListDictionaryInternalTest, Clear) {
    ListDictionaryInternal d;
    int k1 = 1, k2 = 2, v = 0;
    d.Add(&k1, &v);
    d.Add(&k2, &v);
    d.Clear();
    EXPECT_EQ(d.getCountProperty(), 0);
}

TEST(ListDictionaryInternalTest, GetEnumerator_IteratesAllEntries) {
    ListDictionaryInternal d;
    int k1 = 1, k2 = 2, v1 = 10, v2 = 20;
    d.Add(&k1, &v1);
    d.Add(&k2, &v2);

    IDictionaryEnumerator* e = d.GetEnumerator();
    int count = 0;
    while (e->MoveNext()) {
        ++count;
        EXPECT_NE(e->getKeyProperty(), nullptr);
    }
    EXPECT_EQ(count, 2);
    delete e;
}

TEST(ListDictionaryInternalTest, GetEnumerator_CurrentBeforeMoveNext_Throws) {
    ListDictionaryInternal d;
    int k = 1, v = 2;
    d.Add(&k, &v);
    IDictionaryEnumerator* e = d.GetEnumerator();
    EXPECT_THROW(e->getKeyProperty(), System::InvalidOperationException);
    delete e;
}

TEST(ListDictionaryInternalTest, Keys_ReflectsCountAndContents) {
    ListDictionaryInternal d;
    int k1 = 1, k2 = 2, v = 0;
    d.Add(&k1, &v);
    d.Add(&k2, &v);

    ICollection* keys = d.getKeysProperty();
    EXPECT_EQ(keys->getCountProperty(), 2);

    IEnumerator* e = keys->GetEnumerator();
    int count = 0;
    while (e->MoveNext()) {
        void* k = e->getCurrentProperty();
        EXPECT_TRUE(k == &k1 || k == &k2);
        ++count;
    }
    EXPECT_EQ(count, 2);
    delete e;
    delete keys;
}

TEST(ListDictionaryInternalTest, Values_ReflectsContents) {
    ListDictionaryInternal d;
    int k = 1, v = 99;
    d.Add(&k, &v);

    ICollection* values = d.getValuesProperty();
    EXPECT_EQ(values->getCountProperty(), 1);

    IEnumerator* e = values->GetEnumerator();
    ASSERT_TRUE(e->MoveNext());
    EXPECT_EQ(e->getCurrentProperty(), &v);
    delete e;
    delete values;
}

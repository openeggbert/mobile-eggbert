// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/UnitySerializationHolder.hpp"
#include "System/DBNull.hpp"

using System::UnitySerializationHolder;

TEST(UnitySerializationHolderTest, NullUnityConstant) {
    EXPECT_EQ(UnitySerializationHolder::NullUnity, 0x0002);
}

TEST(UnitySerializationHolderTest, CtorStoresUnityType) {
    UnitySerializationHolder h(UnitySerializationHolder::NullUnity);
    EXPECT_EQ(h.getUnityTypeProperty(), UnitySerializationHolder::NullUnity);
}

TEST(UnitySerializationHolderTest, CtorStoresData) {
    UnitySerializationHolder h(UnitySerializationHolder::NullUnity, "info");
    EXPECT_EQ(h.getDataProperty(), "info");
}

TEST(UnitySerializationHolderTest, GetRealObjectForNullUnity) {
    UnitySerializationHolder h(UnitySerializationHolder::NullUnity);
    EXPECT_NO_THROW({ auto& obj = h.GetRealObject(); (void)obj; });
}

TEST(UnitySerializationHolderTest, GetRealObjectReturnsDBNull) {
    UnitySerializationHolder h(UnitySerializationHolder::NullUnity);
    auto& obj = h.GetRealObject();
    EXPECT_EQ(&obj, &System::DBNull::Value());
}

TEST(UnitySerializationHolderTest, GetRealObjectInvalidTypeThrows) {
    UnitySerializationHolder h(99);
    EXPECT_THROW(h.GetRealObject(), System::ArgumentException);
}

TEST(UnitySerializationHolderTest, GetObjectDataThrows) {
    UnitySerializationHolder h(UnitySerializationHolder::NullUnity);
    EXPECT_THROW(h.GetObjectData(), System::NotSupportedException);
}

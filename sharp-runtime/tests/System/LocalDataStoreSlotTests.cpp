// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/LocalDataStoreSlot.hpp"

using System::LocalDataStoreSlot;

TEST(LocalDataStoreSlotTests, DefaultCtor_HasNoData) {
    LocalDataStoreSlot slot;
    EXPECT_FALSE(slot.hasData());
}

TEST(LocalDataStoreSlotTests, SetData_HasData) {
    LocalDataStoreSlot slot;
    slot.setData(42);
    EXPECT_TRUE(slot.hasData());
}

TEST(LocalDataStoreSlotTests, SetData_GetData_CorrectValue) {
    LocalDataStoreSlot slot;
    slot.setData(std::string("hello"));
    EXPECT_EQ(std::any_cast<std::string>(slot.getData()), "hello");
}

TEST(LocalDataStoreSlotTests, SetData_Int_CorrectValue) {
    LocalDataStoreSlot slot;
    slot.setData(99);
    EXPECT_EQ(std::any_cast<int>(slot.getData()), 99);
}

TEST(LocalDataStoreSlotTests, Clear_RemovesData) {
    LocalDataStoreSlot slot;
    slot.setData(1);
    slot.clear();
    EXPECT_FALSE(slot.hasData());
}

TEST(LocalDataStoreSlotTests, Overwrite_NewValueStored) {
    LocalDataStoreSlot slot;
    slot.setData(1);
    slot.setData(std::string("new"));
    EXPECT_EQ(std::any_cast<std::string>(slot.getData()), "new");
}

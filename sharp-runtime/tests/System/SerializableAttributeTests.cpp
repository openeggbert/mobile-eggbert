// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/SerializableAttribute.hpp"

using System::SerializableAttribute;

TEST(SerializableAttributeTest, DefaultCtor) {
    SerializableAttribute a;
    (void)a;
}

TEST(SerializableAttributeTest, IsAttribute) {
    SerializableAttribute a;
    EXPECT_NO_THROW({ System::Attribute& ref = a; (void)ref; });
}

TEST(SerializableAttributeTest, TwoInstancesAreIndependent) {
    SerializableAttribute a;
    SerializableAttribute b;
    EXPECT_NE(&a, &b);
}

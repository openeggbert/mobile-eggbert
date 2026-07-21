// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/UriCreationOptions.hpp"

using System::UriCreationOptions;

TEST(UriCreationOptionsTest, DefaultCtor) {
    UriCreationOptions opts;
    EXPECT_FALSE(opts.DangerousDisablePathAndQueryCanonicalization);
}

TEST(UriCreationOptionsTest, SetFlag) {
    UriCreationOptions opts;
    opts.DangerousDisablePathAndQueryCanonicalization = true;
    EXPECT_TRUE(opts.DangerousDisablePathAndQueryCanonicalization);
}

TEST(UriCreationOptionsTest, TwoInstancesIndependent) {
    UriCreationOptions a;
    UriCreationOptions b;
    b.DangerousDisablePathAndQueryCanonicalization = true;
    EXPECT_FALSE(a.DangerousDisablePathAndQueryCanonicalization);
    EXPECT_TRUE(b.DangerousDisablePathAndQueryCanonicalization);
}

// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/DataMisalignedException.hpp"

using System::DataMisalignedException;
using System::SystemException;

TEST(DataMisalignedExceptionTests2, DefaultCtor_HasMessage) {
    DataMisalignedException ex;
    EXPECT_FALSE(std::string(ex.what()).empty());
}

TEST(DataMisalignedExceptionTests2, MessageCtor_StoresMessage) {
    DataMisalignedException ex("bad alignment");
    EXPECT_EQ(std::string(ex.what()), "bad alignment");
}

TEST(DataMisalignedExceptionTests2, IsA_SystemException) {
    EXPECT_THROW(throw DataMisalignedException(), SystemException);
}

TEST(DataMisalignedExceptionTests2, InnerExceptionCtor_StoresMessage) {
    auto inner = std::make_exception_ptr(std::runtime_error("root"));
    DataMisalignedException ex("misaligned", inner);
    EXPECT_EQ(std::string(ex.what()), "misaligned");
}

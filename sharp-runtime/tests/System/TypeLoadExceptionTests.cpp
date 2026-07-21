// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include <stdexcept>
#include <string>
#include "System/TypeLoadException.hpp"
#include "System/SystemException.hpp"

// ---------------------------------------------------------------------------
// Constructors
// ---------------------------------------------------------------------------

TEST(TypeLoadExceptionTests, DefaultCtor_MessageNotEmpty) {
    System::TypeLoadException ex;
    EXPECT_FALSE(std::string(ex.what()).empty());
}

TEST(TypeLoadExceptionTests, DefaultCtor_TypeName_Empty) {
    System::TypeLoadException ex;
    EXPECT_EQ(ex.getTypeNameProperty(), "");
}

TEST(TypeLoadExceptionTests, CharPtrCtor_StoresMessage) {
    System::TypeLoadException ex("bad type");
    EXPECT_NE(std::string(ex.what()).find("bad type"), std::string::npos);
}

TEST(TypeLoadExceptionTests, StringCtor_StoresMessage) {
    System::TypeLoadException ex(std::string("load failed"));
    EXPECT_NE(std::string(ex.what()).find("load failed"), std::string::npos);
}

TEST(TypeLoadExceptionTests, InnerExceptionCtor_ContainsBoth) {
    auto inner = std::make_exception_ptr(std::runtime_error("root cause"));
    System::TypeLoadException ex("outer msg", inner);
    std::string w = ex.what();
    EXPECT_NE(w.find("outer msg"), std::string::npos);
}

TEST(TypeLoadExceptionTests, InnerExceptionCtor_TypeName_Empty) {
    auto inner = std::make_exception_ptr(std::runtime_error("x"));
    System::TypeLoadException ex("msg", inner);
    EXPECT_EQ(ex.getTypeNameProperty(), "");
}

TEST(TypeLoadExceptionTests, TypeNameCtor_StoresTypeName) {
    System::TypeLoadException ex("Cannot load type", "System.Foo.Bar");
    EXPECT_EQ(ex.getTypeNameProperty(), "System.Foo.Bar");
}

TEST(TypeLoadExceptionTests, TypeNameCtor_StoresMessage) {
    System::TypeLoadException ex("Cannot load type", "System.Foo.Bar");
    EXPECT_NE(std::string(ex.what()).find("Cannot load type"), std::string::npos);
}

// ---------------------------------------------------------------------------
// Inheritance
// ---------------------------------------------------------------------------

TEST(TypeLoadExceptionTests, IsA_SystemException) {
    EXPECT_THROW(throw System::TypeLoadException(), System::SystemException);
}

TEST(TypeLoadExceptionTests, IsA_StdException) {
    EXPECT_THROW(throw System::TypeLoadException(), std::exception);
}

// ---------------------------------------------------------------------------
// TypeName property
// ---------------------------------------------------------------------------

TEST(TypeLoadExceptionTests, TypeName_Default_IsEmpty) {
    System::TypeLoadException ex;
    EXPECT_TRUE(ex.getTypeNameProperty().empty());
}

TEST(TypeLoadExceptionTests, TypeName_Set_RoundTrip) {
    System::TypeLoadException ex("msg", "My.Namespace.MyClass");
    EXPECT_EQ(ex.getTypeNameProperty(), "My.Namespace.MyClass");
}

TEST(TypeLoadExceptionTests, TypeName_EmptyString_Roundtrip) {
    System::TypeLoadException ex("msg", std::string(""));
    EXPECT_EQ(ex.getTypeNameProperty(), "");
}

TEST(TypeLoadExceptionTests, HResult_IsCorTypeLoad) {
    System::TypeLoadException ex;
    EXPECT_EQ(ex.getHResultProperty(), static_cast<SharpRuntime::intcs>(0x80131522));
}

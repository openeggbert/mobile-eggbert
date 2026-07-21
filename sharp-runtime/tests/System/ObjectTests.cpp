// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include <memory>
#include <climits>
#include <typeinfo>
#include "System/Object.hpp"
#include "System/Type.hpp"

// ---------------------------------------------------------------------------
// Minimal concrete subclasses for testing
// ---------------------------------------------------------------------------

namespace {
    struct SimpleObj : public System::Object {
        [[nodiscard]] const std::string& GetTypeName() const override {
            static const std::string n = "SimpleObj";
            return n;
        }
    };

    struct NamedObj : public System::Object {
        std::string name;
        explicit NamedObj(std::string n) : name(std::move(n)) {}
        [[nodiscard]] const std::string& GetTypeName() const override {
            static const std::string t = "NamedObj";
            return t;
        }
        [[nodiscard]] std::string ToString() const override { return name; }
        bool Equals(const System::Object* obj) const override {
            const auto* other = dynamic_cast<const NamedObj*>(obj);
            return other && name == other->name;
        }
        [[nodiscard]] int GetHashCode() const override {
            return static_cast<int>(std::hash<std::string>{}(name) & 0x7fffffff);
        }
    };
} // anonymous namespace

// ---------------------------------------------------------------------------
// GetTypeName
// ---------------------------------------------------------------------------

TEST(ObjectTests, GetTypeName_ReturnsCorrectName) {
    SimpleObj o;
    EXPECT_EQ(o.GetTypeName(), "SimpleObj");
}

TEST(ObjectTests, GetTypeName_MatchesToString) {
    SimpleObj o;
    EXPECT_EQ(o.GetTypeName(), o.ToString());
}

// ---------------------------------------------------------------------------
// ToString
// ---------------------------------------------------------------------------

TEST(ObjectTests, ToString_Default_ReturnsTypeName) {
    SimpleObj o;
    EXPECT_EQ(o.ToString(), "SimpleObj");
}

TEST(ObjectTests, ToString_Override_ReturnsCustomString) {
    NamedObj o("Alice");
    EXPECT_EQ(o.ToString(), "Alice");
}

// ---------------------------------------------------------------------------
// Equals (instance)
// ---------------------------------------------------------------------------

TEST(ObjectTests, Equals_SamePointer_True) {
    SimpleObj o;
    EXPECT_TRUE(o.Equals(&o));
}

TEST(ObjectTests, Equals_DifferentPointers_DefaultFalse) {
    SimpleObj a, b;
    EXPECT_FALSE(a.Equals(&b));
}

TEST(ObjectTests, Equals_Nullptr_False) {
    SimpleObj o;
    EXPECT_FALSE(o.Equals(nullptr));
}

TEST(ObjectTests, Equals_Override_ValueEquality_True) {
    NamedObj a("Bob"), b("Bob");
    EXPECT_TRUE(a.Equals(&b));
}

TEST(ObjectTests, Equals_Override_ValueEquality_False) {
    NamedObj a("Bob"), b("Carol");
    EXPECT_FALSE(a.Equals(&b));
}

// ---------------------------------------------------------------------------
// Equals (static)
// ---------------------------------------------------------------------------

TEST(ObjectTests, StaticEquals_BothNull_True) {
    EXPECT_TRUE(System::Object::Equals(nullptr, nullptr));
}

TEST(ObjectTests, StaticEquals_LeftNull_False) {
    SimpleObj o;
    EXPECT_FALSE(System::Object::Equals(nullptr, &o));
}

TEST(ObjectTests, StaticEquals_RightNull_False) {
    SimpleObj o;
    EXPECT_FALSE(System::Object::Equals(&o, nullptr));
}

TEST(ObjectTests, StaticEquals_SamePointer_True) {
    SimpleObj o;
    EXPECT_TRUE(System::Object::Equals(&o, &o));
}

TEST(ObjectTests, StaticEquals_DifferentDefault_False) {
    SimpleObj a, b;
    EXPECT_FALSE(System::Object::Equals(&a, &b));
}

TEST(ObjectTests, StaticEquals_Override_ValueEquality) {
    NamedObj a("Eve"), b("Eve");
    EXPECT_TRUE(System::Object::Equals(&a, &b));
}

// ---------------------------------------------------------------------------
// ReferenceEquals (static)
// ---------------------------------------------------------------------------

TEST(ObjectTests, ReferenceEquals_BothNull_True) {
    EXPECT_TRUE(System::Object::ReferenceEquals(nullptr, nullptr));
}

TEST(ObjectTests, ReferenceEquals_NullAndObject_False) {
    SimpleObj o;
    EXPECT_FALSE(System::Object::ReferenceEquals(nullptr, &o));
    EXPECT_FALSE(System::Object::ReferenceEquals(&o, nullptr));
}

TEST(ObjectTests, ReferenceEquals_SamePointer_True) {
    SimpleObj o;
    EXPECT_TRUE(System::Object::ReferenceEquals(&o, &o));
}

TEST(ObjectTests, ReferenceEquals_DifferentPointers_False) {
    SimpleObj a, b;
    EXPECT_FALSE(System::Object::ReferenceEquals(&a, &b));
}

TEST(ObjectTests, ReferenceEquals_EqualByValue_StillFalse) {
    NamedObj a("same"), b("same");
    EXPECT_FALSE(System::Object::ReferenceEquals(&a, &b));
}

// ---------------------------------------------------------------------------
// GetHashCode
// ---------------------------------------------------------------------------

TEST(ObjectTests, GetHashCode_IsNonNegativeValue) {
    SimpleObj o;
    EXPECT_GE(o.GetHashCode(), 0);
}

TEST(ObjectTests, GetHashCode_WithinIntRange) {
    SimpleObj o;
    EXPECT_LE(o.GetHashCode(), INT_MAX);
}

TEST(ObjectTests, GetHashCode_Consistent) {
    SimpleObj o;
    EXPECT_EQ(o.GetHashCode(), o.GetHashCode());
}

TEST(ObjectTests, GetHashCode_Override_BasedOnValue) {
    NamedObj a("hash-me"), b("hash-me");
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}

TEST(ObjectTests, GetHashCode_Override_DifferentValues_MayDiffer) {
    NamedObj a("alpha"), b("beta");
    EXPECT_NE(a.GetHashCode(), b.GetHashCode());
}

// ---------------------------------------------------------------------------
// Polymorphic usage
// ---------------------------------------------------------------------------

TEST(ObjectTests, HeapAlloc_ViaBasePtr) {
    std::unique_ptr<System::Object> p = std::make_unique<SimpleObj>();
    EXPECT_NE(p, nullptr);
    EXPECT_EQ(p->ToString(), "SimpleObj");
}

TEST(ObjectTests, PolyDispatch_Equals_ViaBasePtr) {
    NamedObj a("X"), b("X");
    System::Object* pa = &a;
    System::Object* pb = &b;
    EXPECT_TRUE(pa->Equals(pb));
}

// ---------------------------------------------------------------------------
// GetType
// ---------------------------------------------------------------------------

TEST(ObjectTests, GetType_NonNullTypeInfo) {
    SimpleObj o;
    EXPECT_NE(o.GetType().getTypeInfo(), nullptr);
}

TEST(ObjectTests, GetType_DynamicType_NotBaseObject) {
    SimpleObj o;
    System::Type t = o.GetType();
    System::Type objectType = System::Type::From<System::Object>();
    EXPECT_NE(t, objectType);
}

TEST(ObjectTests, GetType_MatchesTypeId) {
    SimpleObj o;
    System::Type t = o.GetType();
    System::Type expected = System::Type::From<SimpleObj>();
    EXPECT_EQ(t, expected);
}

TEST(ObjectTests, GetType_ViaPoly_ReturnsDerivedType) {
    SimpleObj o;
    System::Object* p = &o;
    // Even through a base pointer, GetType returns the dynamic (derived) type
    EXPECT_EQ(p->GetType(), System::Type::From<SimpleObj>());
}

TEST(ObjectTests, GetType_TwoInstancesSameClass_Equal) {
    SimpleObj a, b;
    EXPECT_EQ(a.GetType(), b.GetType());
}

TEST(ObjectTests, GetType_DifferentClasses_NotEqual) {
    SimpleObj a;
    NamedObj b("x");
    EXPECT_NE(a.GetType(), b.GetType());
}

TEST(ObjectTests, GetType_ToString_NonEmpty) {
    SimpleObj o;
    EXPECT_FALSE(o.GetType().ToString().empty());
}

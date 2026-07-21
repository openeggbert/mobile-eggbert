// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/STAThreadAttribute.hpp"
#include "System/MTAThreadAttribute.hpp"
#include "System/Attribute.hpp"

using System::STAThreadAttribute;
using System::MTAThreadAttribute;

// ---------------------------------------------------------------------------
// STAThreadAttribute
// ---------------------------------------------------------------------------

TEST(STAThreadAttributeTests, DefaultCtor_DoesNotThrow) {
    EXPECT_NO_THROW(STAThreadAttribute());
}

TEST(STAThreadAttributeTests, IsA_Attribute) {
    STAThreadAttribute attr;
    EXPECT_NE(dynamic_cast<System::Attribute*>(&attr), nullptr);
}

TEST(STAThreadAttributeTests, TwoInstances_BothConstruct) {
    STAThreadAttribute a;
    STAThreadAttribute b;
    EXPECT_NE(dynamic_cast<System::Attribute*>(&a), nullptr);
    EXPECT_NE(dynamic_cast<System::Attribute*>(&b), nullptr);
}

TEST(STAThreadAttributeTests, CaughtAsAttribute) {
    bool caught = false;
    try { throw STAThreadAttribute(); }
    catch (const System::Attribute&) { caught = true; }
    EXPECT_TRUE(caught);
}

// ---------------------------------------------------------------------------
// MTAThreadAttribute
// ---------------------------------------------------------------------------

TEST(MTAThreadAttributeTests, DefaultCtor_DoesNotThrow) {
    EXPECT_NO_THROW(MTAThreadAttribute());
}

TEST(MTAThreadAttributeTests, IsA_Attribute) {
    MTAThreadAttribute attr;
    EXPECT_NE(dynamic_cast<System::Attribute*>(&attr), nullptr);
}

TEST(MTAThreadAttributeTests, TwoInstances_BothConstruct) {
    MTAThreadAttribute a;
    MTAThreadAttribute b;
    EXPECT_NE(dynamic_cast<System::Attribute*>(&a), nullptr);
    EXPECT_NE(dynamic_cast<System::Attribute*>(&b), nullptr);
}

TEST(MTAThreadAttributeTests, CaughtAsAttribute) {
    bool caught = false;
    try { throw MTAThreadAttribute(); }
    catch (const System::Attribute&) { caught = true; }
    EXPECT_TRUE(caught);
}

// ---------------------------------------------------------------------------
// Distinct types
// ---------------------------------------------------------------------------

TEST(ThreadAttributeTests, STA_And_MTA_AreDistinctTypes) {
    // Both derive from Attribute but are unrelated to each other.
    // A pointer to STAThreadAttribute must not satisfy a STAThreadAttribute*
    // catch when the thrown object is MTAThreadAttribute, and vice versa.
    bool staCaught = false;
    try { throw MTAThreadAttribute(); }
    catch (const STAThreadAttribute&) { staCaught = true; }
    catch (...) {}
    EXPECT_FALSE(staCaught);

    bool mtaCaught = false;
    try { throw STAThreadAttribute(); }
    catch (const MTAThreadAttribute&) { mtaCaught = true; }
    catch (...) {}
    EXPECT_FALSE(mtaCaught);
}

// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Tests for System::Diagnostics::CodeAnalysis attribute types.
#include <gtest/gtest.h>
#include <any>
#include <string>
#include "System/Diagnostics/CodeAnalysis/CodeAnalysisAttributes.hpp"

using namespace System::Diagnostics::CodeAnalysis;

// ===========================================================================
// Marker-only null-analysis attributes
// ===========================================================================

TEST(CodeAnalysisMarkerTests, NotNullAttribute_DefaultCtor) {
    EXPECT_NO_THROW(NotNullAttribute{});
}

TEST(CodeAnalysisMarkerTests, MaybeNullAttribute_DefaultCtor) {
    EXPECT_NO_THROW(MaybeNullAttribute{});
}

TEST(CodeAnalysisMarkerTests, AllowNullAttribute_DefaultCtor) {
    EXPECT_NO_THROW(AllowNullAttribute{});
}

TEST(CodeAnalysisMarkerTests, DisallowNullAttribute_DefaultCtor) {
    EXPECT_NO_THROW(DisallowNullAttribute{});
}

TEST(CodeAnalysisMarkerTests, DoesNotReturnAttribute_DefaultCtor) {
    EXPECT_NO_THROW(DoesNotReturnAttribute{});
}

// ===========================================================================
// NotNullIfNotNullAttribute
// ===========================================================================

TEST(NotNullIfNotNullAttributeTests, ParameterName_Stored) {
    NotNullIfNotNullAttribute attr("other");
    EXPECT_EQ(attr.getParameterNameProperty(), "other");
}

// ===========================================================================
// MaybeNullWhenAttribute / NotNullWhenAttribute
// ===========================================================================

TEST(MaybeNullWhenAttributeTests, ReturnValue_True) {
    MaybeNullWhenAttribute attr(true);
    EXPECT_TRUE(attr.getReturnValueProperty());
}

TEST(MaybeNullWhenAttributeTests, ReturnValue_False) {
    MaybeNullWhenAttribute attr(false);
    EXPECT_FALSE(attr.getReturnValueProperty());
}

TEST(NotNullWhenAttributeTests, ReturnValue_True) {
    NotNullWhenAttribute attr(true);
    EXPECT_TRUE(attr.getReturnValueProperty());
}

TEST(NotNullWhenAttributeTests, ReturnValue_False) {
    NotNullWhenAttribute attr(false);
    EXPECT_FALSE(attr.getReturnValueProperty());
}

// ===========================================================================
// DoesNotReturnIfAttribute
// ===========================================================================

TEST(DoesNotReturnIfAttributeTests, ParameterValue_True) {
    DoesNotReturnIfAttribute attr(true);
    EXPECT_TRUE(attr.getParameterValueProperty());
}

TEST(DoesNotReturnIfAttributeTests, ParameterValue_False) {
    DoesNotReturnIfAttribute attr(false);
    EXPECT_FALSE(attr.getParameterValueProperty());
}

// ===========================================================================
// MemberNotNullAttribute
// ===========================================================================

TEST(MemberNotNullAttributeTests, SingleMember_Stored) {
    MemberNotNullAttribute attr("_field");
    ASSERT_EQ(attr.getMembersProperty().size(), 1u);
    EXPECT_EQ(attr.getMembersProperty()[0], "_field");
}

TEST(MemberNotNullAttributeTests, MultipleMembers_Stored) {
    MemberNotNullAttribute attr(std::vector<std::string>{"_a", "_b", "_c"});
    ASSERT_EQ(attr.getMembersProperty().size(), 3u);
    EXPECT_EQ(attr.getMembersProperty()[0], "_a");
    EXPECT_EQ(attr.getMembersProperty()[1], "_b");
    EXPECT_EQ(attr.getMembersProperty()[2], "_c");
}

// ===========================================================================
// MemberNotNullWhenAttribute
// ===========================================================================

TEST(MemberNotNullWhenAttributeTests, ReturnValueAndSingleMember_Stored) {
    MemberNotNullWhenAttribute attr(true, "_field");
    EXPECT_TRUE(attr.getReturnValueProperty());
    ASSERT_EQ(attr.getMembersProperty().size(), 1u);
    EXPECT_EQ(attr.getMembersProperty()[0], "_field");
}

TEST(MemberNotNullWhenAttributeTests, ReturnValueAndMultipleMembers_Stored) {
    MemberNotNullWhenAttribute attr(false, std::vector<std::string>{"_a", "_b"});
    EXPECT_FALSE(attr.getReturnValueProperty());
    ASSERT_EQ(attr.getMembersProperty().size(), 2u);
    EXPECT_EQ(attr.getMembersProperty()[0], "_a");
    EXPECT_EQ(attr.getMembersProperty()[1], "_b");
}

// ===========================================================================
// RequiresUnreferencedCodeAttribute
// ===========================================================================

TEST(RequiresUnreferencedCodeAttributeTests, Message_Stored) {
    RequiresUnreferencedCodeAttribute attr("Reflection used");
    EXPECT_EQ(attr.getMessageProperty(), "Reflection used");
}

TEST(RequiresUnreferencedCodeAttributeTests, Url_Optional_DefaultEmpty) {
    RequiresUnreferencedCodeAttribute attr("msg");
    EXPECT_TRUE(attr.getUrlProperty().empty());
}

TEST(RequiresUnreferencedCodeAttributeTests, Url_Stored) {
    RequiresUnreferencedCodeAttribute attr("msg", "https://example.com");
    EXPECT_EQ(attr.getUrlProperty(), "https://example.com");
}

TEST(RequiresUnreferencedCodeAttributeTests, ExcludeStatics_DefaultFalse_Settable) {
    RequiresUnreferencedCodeAttribute attr("msg");
    EXPECT_FALSE(attr.getExcludeStaticsProperty());
    attr.setExcludeStaticsProperty(true);
    EXPECT_TRUE(attr.getExcludeStaticsProperty());
}

// ===========================================================================
// RequiresDynamicCodeAttribute
// ===========================================================================

TEST(RequiresDynamicCodeAttributeTests, Message_Stored) {
    RequiresDynamicCodeAttribute attr("Dynamic emit used");
    EXPECT_EQ(attr.getMessageProperty(), "Dynamic emit used");
}

TEST(RequiresDynamicCodeAttributeTests, Url_Stored) {
    RequiresDynamicCodeAttribute attr("msg", "https://docs.example.com");
    EXPECT_EQ(attr.getUrlProperty(), "https://docs.example.com");
}

TEST(RequiresDynamicCodeAttributeTests, ExcludeStatics_DefaultFalse_Settable) {
    RequiresDynamicCodeAttribute attr("msg");
    EXPECT_FALSE(attr.getExcludeStaticsProperty());
    attr.setExcludeStaticsProperty(true);
    EXPECT_TRUE(attr.getExcludeStaticsProperty());
}

// ===========================================================================
// ExcludeFromCodeCoverageAttribute
// ===========================================================================

TEST(ExcludeFromCodeCoverageAttributeTests, DefaultCtor_JustificationEmpty) {
    ExcludeFromCodeCoverageAttribute attr;
    EXPECT_TRUE(attr.getJustificationProperty().empty());
}

TEST(ExcludeFromCodeCoverageAttributeTests, Justification_Stored) {
    ExcludeFromCodeCoverageAttribute attr("Stub method");
    EXPECT_EQ(attr.getJustificationProperty(), "Stub method");
}

TEST(ExcludeFromCodeCoverageAttributeTests, SetJustification_Stored) {
    ExcludeFromCodeCoverageAttribute attr;
    attr.setJustificationProperty("Generated code");
    EXPECT_EQ(attr.getJustificationProperty(), "Generated code");
}

// ===========================================================================
// SuppressMessageAttribute
// ===========================================================================

TEST(SuppressMessageAttributeTests, CategoryAndCheckId_Stored) {
    SuppressMessageAttribute attr("Performance", "CA1815");
    EXPECT_EQ(attr.getCategoryProperty(), "Performance");
    EXPECT_EQ(attr.getCheckIdProperty(), "CA1815");
}

TEST(SuppressMessageAttributeTests, SetJustification_Stored) {
    SuppressMessageAttribute attr("C", "ID");
    attr.setJustificationProperty("intentional");
    EXPECT_EQ(attr.getJustificationProperty(), "intentional");
}

// ===========================================================================
// StringSyntaxAttribute
// ===========================================================================

TEST(StringSyntaxAttributeTests, Syntax_Stored) {
    StringSyntaxAttribute attr("Regex");
    EXPECT_EQ(attr.getSyntaxProperty(), "Regex");
}

TEST(StringSyntaxAttributeTests, StaticConstants_Correct) {
    EXPECT_EQ(std::string(StringSyntaxAttribute::CompositeFormat), "CompositeFormat");
    EXPECT_EQ(std::string(StringSyntaxAttribute::Regex), "Regex");
    EXPECT_EQ(std::string(StringSyntaxAttribute::Json), "Json");
    EXPECT_EQ(std::string(StringSyntaxAttribute::Uri), "Uri");
    EXPECT_EQ(std::string(StringSyntaxAttribute::CSharp), "C#");
    EXPECT_EQ(std::string(StringSyntaxAttribute::FSharp), "F#");
    EXPECT_EQ(std::string(StringSyntaxAttribute::VisualBasic), "Visual Basic");
}

TEST(StringSyntaxAttributeTests, Arguments_DefaultEmpty) {
    StringSyntaxAttribute attr("Regex");
    EXPECT_TRUE(attr.getArgumentsProperty().empty());
}

TEST(StringSyntaxAttributeTests, Arguments_Stored) {
    StringSyntaxAttribute attr("CompositeFormat", std::vector<std::any>{1, std::string("x")});
    ASSERT_EQ(attr.getArgumentsProperty().size(), 2u);
    EXPECT_EQ(std::any_cast<int>(attr.getArgumentsProperty()[0]), 1);
    EXPECT_EQ(std::any_cast<std::string>(attr.getArgumentsProperty()[1]), "x");
}

// ===========================================================================
// ConstantExpectedAttribute
// ===========================================================================

TEST(ConstantExpectedAttributeTests, DefaultCtor_MinMaxEmpty) {
    ConstantExpectedAttribute attr;
    EXPECT_FALSE(attr.getMinProperty().has_value());
    EXPECT_FALSE(attr.getMaxProperty().has_value());
}

TEST(ConstantExpectedAttributeTests, SetMinMax_Stored) {
    ConstantExpectedAttribute attr;
    attr.setMinProperty(1);
    attr.setMaxProperty(10);
    EXPECT_EQ(std::any_cast<int>(attr.getMinProperty()), 1);
    EXPECT_EQ(std::any_cast<int>(attr.getMaxProperty()), 10);
}

// ===========================================================================
// ExperimentalAttribute
// ===========================================================================

TEST(ExperimentalAttributeTests, DiagnosticId_Stored) {
    ExperimentalAttribute attr("SYSLIB5001");
    EXPECT_EQ(attr.getDiagnosticIdProperty(), "SYSLIB5001");
    EXPECT_TRUE(attr.getMessageProperty().empty());
    EXPECT_TRUE(attr.getUrlFormatProperty().empty());
}

TEST(ExperimentalAttributeTests, SetMessageAndUrlFormat_Stored) {
    ExperimentalAttribute attr("SYSLIB5001");
    attr.setMessageProperty("Preview feature");
    attr.setUrlFormatProperty("https://example.com/{0}");
    EXPECT_EQ(attr.getMessageProperty(), "Preview feature");
    EXPECT_EQ(attr.getUrlFormatProperty(), "https://example.com/{0}");
}

// ===========================================================================
// RequiresAssemblyFilesAttribute
// ===========================================================================

TEST(RequiresAssemblyFilesAttributeTests, DefaultCtor_MessageEmpty) {
    RequiresAssemblyFilesAttribute attr;
    EXPECT_TRUE(attr.getMessageProperty().empty());
}

TEST(RequiresAssemblyFilesAttributeTests, Message_Stored) {
    RequiresAssemblyFilesAttribute attr("Needs file on disk");
    EXPECT_EQ(attr.getMessageProperty(), "Needs file on disk");
}

TEST(RequiresAssemblyFilesAttributeTests, SetUrl_Stored) {
    RequiresAssemblyFilesAttribute attr("msg");
    attr.setUrlProperty("https://example.com");
    EXPECT_EQ(attr.getUrlProperty(), "https://example.com");
}

// ===========================================================================
// Marker-only attributes: RequiresUnsafeAttribute, SetsRequiredMembersAttribute, UnscopedRefAttribute
// ===========================================================================

TEST(CodeAnalysisMarkerTests, RequiresUnsafeAttribute_DefaultCtor) {
    EXPECT_NO_THROW(RequiresUnsafeAttribute{});
}

TEST(CodeAnalysisMarkerTests, SetsRequiredMembersAttribute_DefaultCtor) {
    EXPECT_NO_THROW(SetsRequiredMembersAttribute{});
}

TEST(CodeAnalysisMarkerTests, UnscopedRefAttribute_DefaultCtor) {
    EXPECT_NO_THROW(UnscopedRefAttribute{});
}

// ===========================================================================
// UnconditionalSuppressMessageAttribute
// ===========================================================================

TEST(UnconditionalSuppressMessageAttributeTests, CategoryAndCheckId_Stored) {
    UnconditionalSuppressMessageAttribute attr("Performance", "CA1815");
    EXPECT_EQ(attr.getCategoryProperty(), "Performance");
    EXPECT_EQ(attr.getCheckIdProperty(), "CA1815");
}

TEST(UnconditionalSuppressMessageAttributeTests, SetJustificationScopeTarget_Stored) {
    UnconditionalSuppressMessageAttribute attr("C", "ID");
    attr.setJustificationProperty("intentional");
    attr.setScopeProperty("member");
    attr.setTargetProperty("System.IO.Stream.ctor():System.Void");
    EXPECT_EQ(attr.getJustificationProperty(), "intentional");
    EXPECT_EQ(attr.getScopeProperty(), "member");
    EXPECT_EQ(attr.getTargetProperty(), "System.IO.Stream.ctor():System.Void");
}

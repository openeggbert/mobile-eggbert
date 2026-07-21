// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Covers the small standalone System.Xml.Linq support types: LoadOptions, ReaderOptions,
// SaveOptions, XObjectChange, XObjectChangeEventArgs, XNamespace. The XObject/XNode/XContainer
// inheritance hierarchy (XCData, XComment, XContainer, XDocumentType, XProcessingInstruction,
// XStreamingElement, XText, XNodeDocumentOrderComparer, XNodeEqualityComparer, and the
// Extensions free functions) is covered in XLinqNodeTests.cpp.
#include <gtest/gtest.h>
#include "System/Xml/Linq/LoadOptions.hpp"
#include "System/Xml/Linq/ReaderOptions.hpp"
#include "System/Xml/Linq/SaveOptions.hpp"
#include "System/Xml/Linq/XName.hpp"
#include "System/Xml/Linq/XNamespace.hpp"
#include "System/Xml/Linq/XObjectChange.hpp"
#include "System/Xml/Linq/XObjectChangeEventArgs.hpp"

using System::Xml::Linq::LoadOptions;
using System::Xml::Linq::ReaderOptions;
using System::Xml::Linq::SaveOptions;
using System::Xml::Linq::XName;
using System::Xml::Linq::XNamespace;
using System::Xml::Linq::XObjectChange;
using System::Xml::Linq::XObjectChangeEventArgs;

// ===========================================================================
// LoadOptions
// ===========================================================================

TEST(LoadOptionsTests, ValuesMatchDotNet) {
    EXPECT_EQ(static_cast<int>(LoadOptions::None), 0);
    EXPECT_EQ(static_cast<int>(LoadOptions::PreserveWhitespace), 1);
    EXPECT_EQ(static_cast<int>(LoadOptions::SetBaseUri), 2);
    EXPECT_EQ(static_cast<int>(LoadOptions::SetLineInfo), 4);
}

TEST(LoadOptionsTests, BitwiseOr_CombinesFlags) {
    auto combined = LoadOptions::PreserveWhitespace | LoadOptions::SetLineInfo;
    EXPECT_EQ(static_cast<int>(combined), 5);
}

TEST(LoadOptionsTests, BitwiseAnd_ExtractsFlag) {
    auto combined = LoadOptions::PreserveWhitespace | LoadOptions::SetLineInfo;
    EXPECT_EQ(combined & LoadOptions::SetLineInfo, LoadOptions::SetLineInfo);
    EXPECT_EQ(combined & LoadOptions::SetBaseUri, LoadOptions::None);
}

// ===========================================================================
// ReaderOptions
// ===========================================================================

TEST(ReaderOptionsTests, ValuesMatchDotNet) {
    EXPECT_EQ(static_cast<int>(ReaderOptions::None), 0);
    EXPECT_EQ(static_cast<int>(ReaderOptions::OmitDuplicateNamespaces), 1);
}

// ===========================================================================
// SaveOptions
// ===========================================================================

TEST(SaveOptionsTests, ValuesMatchDotNet) {
    EXPECT_EQ(static_cast<int>(SaveOptions::None), 0);
    EXPECT_EQ(static_cast<int>(SaveOptions::DisableFormatting), 1);
    EXPECT_EQ(static_cast<int>(SaveOptions::OmitDuplicateNamespaces), 2);
}

TEST(SaveOptionsTests, BitwiseOr_CombinesFlags) {
    auto combined = SaveOptions::DisableFormatting | SaveOptions::OmitDuplicateNamespaces;
    EXPECT_EQ(static_cast<int>(combined), 3);
}

// ===========================================================================
// XObjectChange / XObjectChangeEventArgs
// ===========================================================================

TEST(XObjectChangeTests, ValuesAreDistinct) {
    EXPECT_NE(XObjectChange::Add, XObjectChange::Remove);
    EXPECT_NE(XObjectChange::Name, XObjectChange::Value);
}

TEST(XObjectChangeEventArgsTests, StaticInstances_CarryExpectedChangeKind) {
    EXPECT_EQ(XObjectChangeEventArgs::Add.getObjectChangeProperty(), XObjectChange::Add);
    EXPECT_EQ(XObjectChangeEventArgs::Remove.getObjectChangeProperty(), XObjectChange::Remove);
    EXPECT_EQ(XObjectChangeEventArgs::Name.getObjectChangeProperty(), XObjectChange::Name);
    EXPECT_EQ(XObjectChangeEventArgs::Value.getObjectChangeProperty(), XObjectChange::Value);
}

TEST(XObjectChangeEventArgsTests, Constructor_StoresChangeKind) {
    XObjectChangeEventArgs args(XObjectChange::Remove);
    EXPECT_EQ(args.getObjectChangeProperty(), XObjectChange::Remove);
}

// ===========================================================================
// XNamespace
// ===========================================================================

TEST(XNamespaceTests, DefaultConstructor_IsEmpty) {
    XNamespace ns;
    EXPECT_EQ(ns.getNamespaceNameProperty(), "");
}

TEST(XNamespaceTests, Get_StoresUri) {
    XNamespace ns = XNamespace::Get("http://example.com/ns");
    EXPECT_EQ(ns.getNamespaceNameProperty(), "http://example.com/ns");
    EXPECT_EQ(ns.ToString(), "http://example.com/ns");
}

TEST(XNamespaceTests, Equality_ComparesByUri) {
    EXPECT_EQ(XNamespace::Get("a"), XNamespace::Get("a"));
    EXPECT_NE(XNamespace::Get("a"), XNamespace::Get("b"));
}

TEST(XNamespaceTests, GetName_BuildsQualifiedXName) {
    XNamespace ns = XNamespace::Get("http://example.com/ns");
    XName name = ns.GetName("local");
    EXPECT_EQ(name.getNamespaceNameProperty(), "http://example.com/ns");
    EXPECT_EQ(name.getLocalNameProperty(), "local");
}

TEST(XNamespaceTests, PlusOperator_BuildsQualifiedXName) {
    XNamespace ns = XNamespace::Get("http://example.com/ns");
    XName name = ns + "local";
    EXPECT_EQ(name.ToString(), "{http://example.com/ns}local");
}

TEST(XNamespaceTests, PredefinedNamespaces) {
    EXPECT_EQ(XNamespace::None.getNamespaceNameProperty(), "");
    EXPECT_EQ(XNamespace::Xml.getNamespaceNameProperty(), "http://www.w3.org/XML/1998/namespace");
    EXPECT_EQ(XNamespace::Xmlns.getNamespaceNameProperty(), "http://www.w3.org/2000/xmlns/");
}

TEST(XNameTests, GetNamespaceProperty_ReturnsMatchingXNamespace) {
    XName name = XName::Get("local", "http://example.com/ns");
    XNamespace ns = name.getNamespaceProperty();
    EXPECT_EQ(ns.getNamespaceNameProperty(), "http://example.com/ns");
}

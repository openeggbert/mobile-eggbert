// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/IO/Path.hpp"
#include "System/ArgumentException.hpp"

using System::IO::Path;

// ---------------------------------------------------------------------------
// Combine — empty-argument handling must not add a spurious separator
// ---------------------------------------------------------------------------

TEST(PathTests, Combine_SecondEmpty_ReturnsFirstUnchanged) {
    EXPECT_EQ(Path::Combine("a/b", ""), "a/b");
}

TEST(PathTests, Combine_FirstEmpty_ReturnsSecond) {
    EXPECT_EQ(Path::Combine("", "b"), "b");
}

TEST(PathTests, Combine_SecondRooted_ReturnsSecond) {
    EXPECT_EQ(Path::Combine("/a", "/b"), "/b");
}

TEST(PathTests, Combine_Normal_JoinsWithSeparator) {
    EXPECT_EQ(Path::Combine("a", "b"), "a/b");
}

TEST(PathTests, Combine_TrailingSeparatorOnFirst_NoDoubleSeparator) {
    EXPECT_EQ(Path::Combine("a/", "b"), "a/b");
}

TEST(PathTests, Combine3_LastEmpty_ReturnsFirstTwoCombined) {
    EXPECT_EQ(Path::Combine("a", "b", ""), "a/b");
}

TEST(PathTests, Combine3_MiddleRooted_ReturnsFromMiddle) {
    EXPECT_EQ(Path::Combine("a", "/b", "c"), "/b/c");
}

// ---------------------------------------------------------------------------
// GetDirectoryName
// ---------------------------------------------------------------------------

TEST(PathTests, GetDirectoryName_Root_ReturnsEmpty) {
    EXPECT_EQ(Path::GetDirectoryName("/"), "");
}

TEST(PathTests, GetDirectoryName_TopLevelFile_ReturnsRoot) {
    EXPECT_EQ(Path::GetDirectoryName("/a"), "/");
}

TEST(PathTests, GetDirectoryName_Nested_ReturnsParent) {
    EXPECT_EQ(Path::GetDirectoryName("/a/b"), "/a");
}

TEST(PathTests, GetDirectoryName_Relative_ReturnsParent) {
    EXPECT_EQ(Path::GetDirectoryName("a/b"), "a");
}

TEST(PathTests, GetDirectoryName_NoSeparator_ReturnsEmpty) {
    EXPECT_EQ(Path::GetDirectoryName("a"), "");
}

TEST(PathTests, GetDirectoryName_EmptyPath_ReturnsEmpty) {
    EXPECT_EQ(Path::GetDirectoryName(""), "");
}

// ---------------------------------------------------------------------------
// GetExtension / HasExtension — leading-dot (dotfile) and trailing-dot edge
// cases, where .NET's algorithm differs from std::filesystem's.
// ---------------------------------------------------------------------------

TEST(PathTests, GetExtension_Normal_ReturnsExtensionWithDot) {
    EXPECT_EQ(Path::GetExtension("file.txt"), ".txt");
}

TEST(PathTests, GetExtension_DotfileWithNoOtherDot_TreatsWholeNameAsExtension) {
    // .NET quirk: a lone leading '.' is not special-cased, so the whole
    // name is returned as the "extension" (verified against Path.cs GetExtension).
    EXPECT_EQ(Path::GetExtension(".bashrc"), ".bashrc");
}

TEST(PathTests, GetExtension_TrailingDot_ReturnsEmpty) {
    EXPECT_EQ(Path::GetExtension("file."), "");
}

TEST(PathTests, GetExtension_NoDot_ReturnsEmpty) {
    EXPECT_EQ(Path::GetExtension("file"), "");
}

TEST(PathTests, HasExtension_DotfileWithNoOtherDot_ReturnsTrue) {
    EXPECT_TRUE(Path::HasExtension(".bashrc"));
}

TEST(PathTests, HasExtension_TrailingDot_ReturnsFalse) {
    EXPECT_FALSE(Path::HasExtension("file."));
}

TEST(PathTests, HasExtension_Normal_ReturnsTrue) {
    EXPECT_TRUE(Path::HasExtension("file.txt"));
}

TEST(PathTests, GetFileNameWithoutExtension_Dotfile_ReturnsEmpty) {
    EXPECT_EQ(Path::GetFileNameWithoutExtension(".bashrc"), "");
}

TEST(PathTests, GetFileNameWithoutExtension_Normal_StripsExtension) {
    EXPECT_EQ(Path::GetFileNameWithoutExtension("/a/file.txt"), "file");
}

TEST(PathTests, ChangeExtension_Dotfile_ReplacesWholeNameAsExtension) {
    EXPECT_EQ(Path::ChangeExtension(".bashrc", ".txt"), ".txt");
}

TEST(PathTests, ChangeExtension_Normal_ReplacesExtension) {
    EXPECT_EQ(Path::ChangeExtension("file.txt", ".md"), "file.md");
}

// ---------------------------------------------------------------------------
// GetFullPath — argument validation and "." / ".." normalization
// ---------------------------------------------------------------------------

TEST(PathTests, GetFullPath_Empty_Throws) {
    EXPECT_THROW(Path::GetFullPath(""), System::ArgumentException);
}

TEST(PathTests, GetFullPath_NulCharacter_Throws) {
    std::string withNul = std::string("a") + '\0' + "b";
    EXPECT_THROW(Path::GetFullPath(withNul), System::ArgumentException);
}

TEST(PathTests, GetFullPath_ResolvesDotDotSegments) {
    EXPECT_EQ(Path::GetFullPath("/a/b/../c"), "/a/c");
}

TEST(PathTests, GetFullPath_ResolvesDotSegments) {
    EXPECT_EQ(Path::GetFullPath("/a/./b"), "/a/b");
}

TEST(PathTests, GetFullPath_CollapsesDoubleSeparators) {
    EXPECT_EQ(Path::GetFullPath("/a//b"), "/a/b");
}

TEST(PathTests, GetFullPath_AlreadyAbsoluteNoDots_Unchanged) {
    EXPECT_EQ(Path::GetFullPath("/a/b/c"), "/a/b/c");
}

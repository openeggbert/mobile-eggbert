// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include <fstream>
#include "System/NotSupportedException.hpp"
#include "System/Xml/XmlException.hpp"
#include "System/Xml/XmlSecureResolver.hpp"
#include "System/Xml/XmlUrlResolver.hpp"

using namespace System::Xml;

TEST(XmlUrlResolverTests, GetEntity_LocalFile_ReturnsContents) {
    std::string path = "/tmp/sharp_rt_xmlurlresolver_test.txt";
    {
        std::ofstream out(path);
        out << "hello resolver";
    }
    XmlUrlResolver resolver;
    System::Uri uri(std::string("file://") + path);
    std::any result = resolver.GetEntity(uri, "", std::nullopt);
    EXPECT_EQ(std::any_cast<std::string>(result), "hello resolver");
    std::remove(path.c_str());
}

TEST(XmlUrlResolverTests, GetEntity_UnsupportedScheme_Throws) {
    XmlUrlResolver resolver;
    System::Uri uri("http://example.com/foo.xml");
    EXPECT_THROW(resolver.GetEntity(uri, "", std::nullopt), XmlException);
}

TEST(XmlUrlResolverTests, GetEntity_MissingFile_Throws) {
    XmlUrlResolver resolver;
    System::Uri uri(std::string("file:///tmp/sharp_rt_does_not_exist_xyz.txt"));
    EXPECT_THROW(resolver.GetEntity(uri, "", std::nullopt), XmlException);
}

TEST(XmlSecureResolverTests, GetEntity_AlwaysThrows) {
    XmlUrlResolver inner;
    XmlSecureResolver secure(inner, "");
    System::Uri uri("http://example.com/foo.xml");
    EXPECT_THROW(secure.GetEntity(uri, "", std::nullopt), XmlException);
}

TEST(XmlUrlResolverTests, ResolveUri_NoBaseUri_ParsesRelativeUri) {
    XmlUrlResolver resolver;
    System::Uri result = resolver.ResolveUri(std::nullopt, "http://example.com/foo.xml");
    EXPECT_EQ(result.getAbsoluteUriProperty(), "http://example.com/foo.xml");
}

TEST(XmlUrlResolverTests, ResolveUri_AbsoluteBaseUri_CombinesWithRelative) {
    XmlUrlResolver resolver;
    System::Uri base("http://example.com/dir/");
    System::Uri result = resolver.ResolveUri(base, "foo.xml");
    EXPECT_EQ(result.getAbsoluteUriProperty(), "http://example.com/dir/foo.xml");
}

TEST(XmlUrlResolverTests, ResolveUri_EmptyRelative_ReturnsBaseUri) {
    XmlUrlResolver resolver;
    System::Uri base("http://example.com/dir/");
    System::Uri result = resolver.ResolveUri(base, "");
    EXPECT_EQ(result.getAbsoluteUriProperty(), base.getAbsoluteUriProperty());
}

TEST(XmlUrlResolverTests, ResolveUri_RelativeBaseUri_ThrowsNotSupportedException) {
    XmlUrlResolver resolver;
    System::Uri relativeBase("foo/bar", System::UriKind::Relative);
    EXPECT_THROW(resolver.ResolveUri(relativeBase, "baz.xml"), System::NotSupportedException);
}

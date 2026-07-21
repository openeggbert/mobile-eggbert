// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/PlatformNotSupportedException.hpp"
#include "System/Security/Cryptography/AuthenticationTagMismatchException.hpp"
#include "System/Security/Cryptography/CryptographicException.hpp"
#include "System/Security/Cryptography/CryptographicUnexpectedOperationException.hpp"
#include "System/Security/Cryptography/HashAlgorithmName.hpp"
#include "System/Security/Cryptography/KeySizes.hpp"
#include "System/Security/Cryptography/MD5CryptoServiceProvider.hpp"
#include "System/Security/Cryptography/Oid.hpp"
#include "System/Security/Cryptography/OidCollection.hpp"
#include "System/Security/Cryptography/SHA1Managed.hpp"
#include "System/Security/Cryptography/SHA256Managed.hpp"
#include "System/Security/Cryptography/SHA384Managed.hpp"
#include "System/Security/Cryptography/SHA512Managed.hpp"

using namespace System::Security::Cryptography;

// --- Exceptions ------------------------------------------------------------------------------

TEST(CryptographicExceptionTests, DefaultMessage) {
    CryptographicException ex;
    EXPECT_STREQ(ex.what(), "Error occurred during a cryptographic operation.");
}

TEST(CryptographicExceptionTests, CustomMessage) {
    CryptographicException ex("Invalid key size");
    EXPECT_STREQ(ex.what(), "Invalid key size");
}

TEST(CryptographicExceptionTests, HResultConstructor) {
    CryptographicException ex(static_cast<SharpRuntime::intcs>(0x80090005));
    EXPECT_EQ(ex.getHResultProperty(), static_cast<SharpRuntime::intcs>(0x80090005));
}

TEST(CryptographicExceptionTests, IsThrowableAsSystemException) {
    EXPECT_THROW(
        { try { throw CryptographicException(); }
          catch (const System::SystemException&) { throw; } },
        CryptographicException);
}

TEST(CryptographicUnexpectedOperationExceptionTests, IsCryptographicException) {
    EXPECT_THROW(
        { try { throw CryptographicUnexpectedOperationException("bad state"); }
          catch (const CryptographicException&) { throw; } },
        CryptographicUnexpectedOperationException);
}

TEST(AuthenticationTagMismatchExceptionTests, DefaultMessage) {
    AuthenticationTagMismatchException ex;
    EXPECT_STREQ(ex.what(), "The computed authentication tag did not match the input authentication tag.");
}

TEST(AuthenticationTagMismatchExceptionTests, IsCryptographicException) {
    EXPECT_THROW(
        { try { throw AuthenticationTagMismatchException(); }
          catch (const CryptographicException&) { throw; } },
        AuthenticationTagMismatchException);
}

// --- KeySizes ----------------------------------------------------------------------------------

TEST(KeySizesTests, BasicAccessors) {
    KeySizes sizes(128, 256, 64);
    EXPECT_EQ(sizes.getMinSizeProperty(), 128);
    EXPECT_EQ(sizes.getMaxSizeProperty(), 256);
    EXPECT_EQ(sizes.getSkipSizeProperty(), 64);
}

// --- HashAlgorithmName ---------------------------------------------------------------------

TEST(HashAlgorithmNameTests, WellKnownValues) {
    EXPECT_EQ(*HashAlgorithmName::SHA256.getNameProperty(), "SHA256");
    EXPECT_EQ(HashAlgorithmName::SHA256.ToString(), "SHA256");
}

TEST(HashAlgorithmNameTests, Equality) {
    HashAlgorithmName a(std::string("SHA256"));
    EXPECT_EQ(a, HashAlgorithmName::SHA256);
    EXPECT_NE(a, HashAlgorithmName::SHA1);
}

TEST(HashAlgorithmNameTests, FromOid_KnownValue) {
    auto name = HashAlgorithmName::FromOid("2.16.840.1.101.3.4.2.1");
    EXPECT_EQ(name, HashAlgorithmName::SHA256);
}

TEST(HashAlgorithmNameTests, FromOid_UnknownValue_Throws) {
    EXPECT_THROW(HashAlgorithmName::FromOid("9.9.9.9"), CryptographicException);
}

TEST(HashAlgorithmNameTests, TryFromOid_UnknownValue_ReturnsFalse) {
    HashAlgorithmName value;
    EXPECT_FALSE(HashAlgorithmName::TryFromOid("9.9.9.9", value));
}

// --- Oid / OidCollection -----------------------------------------------------------------------

TEST(OidTests, ConstructWithValueAndFriendlyName) {
    Oid oid(std::string("1.2.840.113549.2.5"), std::string("md5"));
    EXPECT_EQ(*oid.getValueProperty(), "1.2.840.113549.2.5");
    EXPECT_EQ(*oid.getFriendlyNameProperty(), "md5");
}

TEST(OidTests, SetValueOnce_SameValue_Succeeds) {
    Oid oid;
    oid.setValueProperty(std::string("1.2.3"));
    EXPECT_NO_THROW(oid.setValueProperty(std::string("1.2.3")));
}

TEST(OidTests, SetValueOnce_DifferentValue_Throws) {
    Oid oid;
    oid.setValueProperty(std::string("1.2.3"));
    EXPECT_THROW(oid.setValueProperty(std::string("4.5.6")), System::PlatformNotSupportedException);
}

TEST(OidCollectionTests, AddAndIndex) {
    OidCollection collection;
    Oid oid(std::string("1.2.3"), std::string("test"));
    SharpRuntime::intcs index = collection.Add(oid);
    EXPECT_EQ(index, 0);
    EXPECT_EQ(collection.getCountProperty(), 1);
    EXPECT_EQ(*collection[0].getValueProperty(), "1.2.3");
}

TEST(OidCollectionTests, StringIndexer_FindsByValue) {
    OidCollection collection;
    collection.Add(Oid(std::string("1.2.3"), std::string("test")));
    const Oid* found = collection[std::string("1.2.3")];
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(*found->getFriendlyNameProperty(), "test");
}

TEST(OidCollectionTests, StringIndexer_NotFound_ReturnsNull) {
    OidCollection collection;
    EXPECT_EQ(collection[std::string("9.9.9")], nullptr);
}

TEST(OidCollectionTests, IndexOutOfRange_Throws) {
    OidCollection collection;
    EXPECT_THROW(collection[0], System::ArgumentOutOfRangeException);
}

// CopyTo was previously missing entirely from this port (OidCollection.cs's CopyTo(Oid[], int)).
TEST(OidCollectionTests, CopyTo_CopiesElementsAtIndex) {
    OidCollection collection;
    collection.Add(Oid(std::string("1.2.3"), std::string("a")));
    collection.Add(Oid(std::string("4.5.6"), std::string("b")));
    std::vector<Oid> dest(3, Oid(std::string("0"), std::string("placeholder")));
    collection.CopyTo(dest, 1);
    EXPECT_EQ(*dest[1].getValueProperty(), "1.2.3");
    EXPECT_EQ(*dest[2].getValueProperty(), "4.5.6");
}

// .NET's own check is deliberately `index >= array.Length` (not `>`), matching
// OidCollection.cs exactly -- so even index 0 on an empty destination throws.
TEST(OidCollectionTests, CopyTo_IndexAtOrPastArrayLength_ThrowsArgumentOutOfRange) {
    OidCollection collection;
    std::vector<Oid> emptyDest;
    EXPECT_THROW(collection.CopyTo(emptyDest, 0), System::ArgumentOutOfRangeException);
}

TEST(OidCollectionTests, CopyTo_DestinationTooSmall_ThrowsArgumentException) {
    OidCollection collection;
    collection.Add(Oid(std::string("1.2.3"), std::string("a")));
    collection.Add(Oid(std::string("4.5.6"), std::string("b")));
    std::vector<Oid> dest(1, Oid(std::string("0"), std::string("placeholder")));
    EXPECT_THROW(collection.CopyTo(dest, 0), System::ArgumentException);
}

// --- Legacy wrapper classes ------------------------------------------------------------------

TEST(LegacyHashWrapperTests, MD5CryptoServiceProvider_MatchesMD5) {
    MD5CryptoServiceProvider provider;
    auto digest = provider.ComputeHash(std::vector<SharpRuntime::bytecs>{});
    EXPECT_EQ(digest.size(), 16u);
}

TEST(LegacyHashWrapperTests, SHA1Managed_ProducesCorrectSize) {
    SHA1Managed sha1;
    EXPECT_EQ(sha1.ComputeHash(std::vector<SharpRuntime::bytecs>{}).size(), 20u);
}

TEST(LegacyHashWrapperTests, SHA256Managed_ProducesCorrectSize) {
    SHA256Managed sha256;
    EXPECT_EQ(sha256.ComputeHash(std::vector<SharpRuntime::bytecs>{}).size(), 32u);
}

TEST(LegacyHashWrapperTests, SHA384Managed_ProducesCorrectSize) {
    SHA384Managed sha384;
    EXPECT_EQ(sha384.ComputeHash(std::vector<SharpRuntime::bytecs>{}).size(), 48u);
}

TEST(LegacyHashWrapperTests, SHA512Managed_ProducesCorrectSize) {
    SHA512Managed sha512;
    EXPECT_EQ(sha512.ComputeHash(std::vector<SharpRuntime::bytecs>{}).size(), 64u);
}

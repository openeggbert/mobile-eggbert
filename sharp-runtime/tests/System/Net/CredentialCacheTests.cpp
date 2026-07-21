// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Net/CredentialCache.hpp"
#include "System/Net/NetworkCredential.hpp"
#include "System/ArgumentException.hpp"
#include "System/Uri.hpp"

using System::Net::CredentialCache;
using System::Net::NetworkCredential;
using System::Uri;

// ===========================================================================
// NetworkCredential
// ===========================================================================

TEST(NetworkCredentialTests, DefaultConstructor_EmptyFields) {
    NetworkCredential cred;
    EXPECT_EQ(cred.getUserNameProperty(), "");
    EXPECT_EQ(cred.getPasswordProperty(), "");
    EXPECT_EQ(cred.getDomainProperty(), "");
}

TEST(NetworkCredentialTests, TwoArgConstructor_SetsUserNameAndPassword) {
    NetworkCredential cred("alice", "secret");
    EXPECT_EQ(cred.getUserNameProperty(), "alice");
    EXPECT_EQ(cred.getPasswordProperty(), "secret");
    EXPECT_EQ(cred.getDomainProperty(), "");
}

TEST(NetworkCredentialTests, ThreeArgConstructor_SetsAllFields) {
    NetworkCredential cred("alice", "secret", "CORP");
    EXPECT_EQ(cred.getUserNameProperty(), "alice");
    EXPECT_EQ(cred.getPasswordProperty(), "secret");
    EXPECT_EQ(cred.getDomainProperty(), "CORP");
}

TEST(NetworkCredentialTests, Setters_UpdateFields) {
    NetworkCredential cred;
    cred.setUserNameProperty("bob");
    cred.setPasswordProperty("hunter2");
    cred.setDomainProperty("CORP");
    EXPECT_EQ(cred.getUserNameProperty(), "bob");
    EXPECT_EQ(cred.getPasswordProperty(), "hunter2");
    EXPECT_EQ(cred.getDomainProperty(), "CORP");
}

TEST(NetworkCredentialTests, GetCredential_ByUri_ReturnsSelf) {
    auto cred = std::make_shared<NetworkCredential>("alice", "secret");
    Uri uri("http://example.com/");
    auto result = cred->GetCredential(uri, "Basic");
    EXPECT_EQ(result, cred);
}

TEST(NetworkCredentialTests, GetCredential_ByHost_ReturnsSelf) {
    auto cred = std::make_shared<NetworkCredential>("alice", "secret");
    auto result = cred->GetCredential("example.com", 80, "Basic");
    EXPECT_EQ(result, cred);
}

// ===========================================================================
// CredentialCache
// ===========================================================================

TEST(CredentialCacheTests, GetCredential_ByUri_NoMatch_ReturnsNull) {
    CredentialCache cache;
    Uri uri("http://example.com/");
    EXPECT_EQ(cache.GetCredential(uri, "Basic"), nullptr);
}

TEST(CredentialCacheTests, Add_ThenGetCredential_ByUri_ExactMatch) {
    CredentialCache cache;
    Uri uri("http://example.com/");
    auto cred = std::make_shared<NetworkCredential>("alice", "secret");
    cache.Add(uri, "Basic", cred);

    auto result = cache.GetCredential(uri, "Basic");
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->getUserNameProperty(), "alice");
}

TEST(CredentialCacheTests, Add_ThenGetCredential_ByUri_PrefixMatch) {
    CredentialCache cache;
    Uri prefix("http://example.com/dir/");
    auto cred = std::make_shared<NetworkCredential>("alice", "secret");
    cache.Add(prefix, "Basic", cred);

    Uri deeper("http://example.com/dir/sub/page.html");
    auto result = cache.GetCredential(deeper, "Basic");
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->getUserNameProperty(), "alice");
}

TEST(CredentialCacheTests, Add_ThenGetCredential_ByUri_MostSpecificPrefixWins) {
    CredentialCache cache;
    Uri shallow("http://example.com/");
    Uri deep("http://example.com/dir/");
    cache.Add(shallow, "Basic", std::make_shared<NetworkCredential>("shallow-user", "p"));
    cache.Add(deep, "Basic", std::make_shared<NetworkCredential>("deep-user", "p"));

    Uri target("http://example.com/dir/page.html");
    auto result = cache.GetCredential(target, "Basic");
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->getUserNameProperty(), "deep-user");
}

TEST(CredentialCacheTests, Add_DuplicateKey_Throws) {
    CredentialCache cache;
    Uri uri("http://example.com/");
    cache.Add(uri, "Basic", std::make_shared<NetworkCredential>("alice", "secret"));
    EXPECT_THROW(cache.Add(uri, "Basic", std::make_shared<NetworkCredential>("bob", "pw")), System::ArgumentException);
}

TEST(CredentialCacheTests, Remove_ByUri_RemovesEntry) {
    CredentialCache cache;
    Uri uri("http://example.com/");
    cache.Add(uri, "Basic", std::make_shared<NetworkCredential>("alice", "secret"));
    cache.Remove(uri, "Basic");
    EXPECT_EQ(cache.GetCredential(uri, "Basic"), nullptr);
}

TEST(CredentialCacheTests, Add_ThenGetCredential_ByHost) {
    CredentialCache cache;
    auto cred = std::make_shared<NetworkCredential>("alice", "secret");
    cache.Add("example.com", 80, "Basic", cred);

    auto result = cache.GetCredential("example.com", 80, "Basic");
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->getUserNameProperty(), "alice");
}

TEST(CredentialCacheTests, GetCredential_ByHost_NoMatch_ReturnsNull) {
    CredentialCache cache;
    EXPECT_EQ(cache.GetCredential("example.com", 80, "Basic"), nullptr);
}

TEST(CredentialCacheTests, Add_DuplicateHostKey_Throws) {
    CredentialCache cache;
    cache.Add("example.com", 80, "Basic", std::make_shared<NetworkCredential>("alice", "secret"));
    EXPECT_THROW(cache.Add("example.com", 80, "Basic", std::make_shared<NetworkCredential>("bob", "pw")), System::ArgumentException);
}

TEST(CredentialCacheTests, Remove_ByHost_RemovesEntry) {
    CredentialCache cache;
    cache.Add("example.com", 80, "Basic", std::make_shared<NetworkCredential>("alice", "secret"));
    cache.Remove("example.com", 80, "Basic");
    EXPECT_EQ(cache.GetCredential("example.com", 80, "Basic"), nullptr);
}

TEST(CredentialCacheTests, DefaultCredentials_NotNull) {
    EXPECT_NE(CredentialCache::getDefaultCredentialsProperty(), nullptr);
    EXPECT_NE(CredentialCache::getDefaultNetworkCredentialsProperty(), nullptr);
}

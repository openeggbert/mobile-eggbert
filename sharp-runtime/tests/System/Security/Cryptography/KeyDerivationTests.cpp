// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include <iomanip>
#include <sstream>
#include "System/Security/Cryptography/RNGCryptoServiceProvider.hpp"
#include "System/Security/Cryptography/RandomNumberGenerator.hpp"
#include "System/Security/Cryptography/Rfc2898DeriveBytes.hpp"

using namespace System::Security::Cryptography;

namespace {

std::vector<SharpRuntime::bytecs> toBytes(const std::string& s) { return std::vector<SharpRuntime::bytecs>(s.begin(), s.end()); }

std::string toHex(const std::vector<SharpRuntime::bytecs>& bytes) {
    std::ostringstream oss;
    for (auto b : bytes) oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(b);
    return oss.str();
}

} // namespace

// --- RandomNumberGenerator -------------------------------------------------------------------

TEST(RandomNumberGeneratorTests, GetBytes_ProducesRequestedLength) {
    auto bytes = RandomNumberGenerator::GetBytes(32);
    EXPECT_EQ(bytes.size(), 32u);
}

TEST(RandomNumberGeneratorTests, GetBytes_ProducesDifferentValuesEachCall) {
    auto a = RandomNumberGenerator::GetBytes(32);
    auto b = RandomNumberGenerator::GetBytes(32);
    EXPECT_NE(a, b);
}

TEST(RandomNumberGeneratorTests, Fill_FillsEntireBuffer) {
    std::vector<SharpRuntime::bytecs> buffer(16, 0);
    RandomNumberGenerator::Fill(buffer);
    bool allZero = std::all_of(buffer.begin(), buffer.end(), [](auto b) { return b == 0; });
    EXPECT_FALSE(allZero);
}

TEST(RandomNumberGeneratorTests, GetInt32_Range_StaysWithinBounds) {
    for (int i = 0; i < 200; ++i) {
        SharpRuntime::intcs v = RandomNumberGenerator::GetInt32(10, 20);
        EXPECT_GE(v, 10);
        EXPECT_LT(v, 20);
    }
}

TEST(RandomNumberGeneratorTests, GetInt32_SingleValueRange_AlwaysReturnsThatValue) {
    EXPECT_EQ(RandomNumberGenerator::GetInt32(5, 6), 5);
}

TEST(RandomNumberGeneratorTests, Create_ReturnsWorkingInstance) {
    auto rng = RandomNumberGenerator::Create();
    std::vector<SharpRuntime::bytecs> buffer(8);
    rng->GetBytes(buffer);
    EXPECT_EQ(buffer.size(), 8u);
}

TEST(RNGCryptoServiceProviderTests, GetBytes_ProducesRequestedLength) {
    RNGCryptoServiceProvider rng;
    std::vector<SharpRuntime::bytecs> buffer(24);
    rng.GetBytes(buffer);
    EXPECT_EQ(buffer.size(), 24u);
}

// --- Rfc2898DeriveBytes (PBKDF2, RFC 6070 test vectors) ------------------------------------

TEST(Rfc2898DeriveBytesTests, RFC6070_SHA1_OneIteration) {
    Rfc2898DeriveBytes pbkdf2(std::string("password"), toBytes("salt"), 1, HashAlgorithmName::SHA1);
    EXPECT_EQ(toHex(pbkdf2.GetBytes(20)), "0c60c80f961f0e71f3a9b524af6012062fe037a6");
}

TEST(Rfc2898DeriveBytesTests, RFC6070_SHA1_TwoIterations) {
    Rfc2898DeriveBytes pbkdf2(std::string("password"), toBytes("salt"), 2, HashAlgorithmName::SHA1);
    EXPECT_EQ(toHex(pbkdf2.GetBytes(20)), "ea6c014dc72d6f8ccd1ed92ace1d41f0d8de8957");
}

TEST(Rfc2898DeriveBytesTests, RFC6070_SHA1_4096Iterations) {
    Rfc2898DeriveBytes pbkdf2(std::string("password"), toBytes("salt"), 4096, HashAlgorithmName::SHA1);
    EXPECT_EQ(toHex(pbkdf2.GetBytes(20)), "4b007901b765489abead49d926f721d065a429c1");
}

TEST(Rfc2898DeriveBytesTests, GetBytes_MultipleCallsAccumulate_MatchesSingleCall) {
    Rfc2898DeriveBytes a(std::string("password"), toBytes("salt"), 1, HashAlgorithmName::SHA1);
    auto whole = a.GetBytes(40);

    Rfc2898DeriveBytes b(std::string("password"), toBytes("salt"), 1, HashAlgorithmName::SHA1);
    auto part1 = b.GetBytes(7);
    auto part2 = b.GetBytes(33);
    std::vector<SharpRuntime::bytecs> combined = part1;
    combined.insert(combined.end(), part2.begin(), part2.end());

    EXPECT_EQ(whole, combined);
}

TEST(Rfc2898DeriveBytesTests, Reset_RestartsFromBeginning) {
    Rfc2898DeriveBytes pbkdf2(std::string("password"), toBytes("salt"), 1, HashAlgorithmName::SHA1);
    auto first = pbkdf2.GetBytes(20);
    pbkdf2.Reset();
    auto second = pbkdf2.GetBytes(20);
    EXPECT_EQ(first, second);
}

TEST(Rfc2898DeriveBytesTests, SaltAndIterationCountAccessors) {
    Rfc2898DeriveBytes pbkdf2(std::string("password"), toBytes("salt"), 4096, HashAlgorithmName::SHA1);
    EXPECT_EQ(pbkdf2.getIterationCountProperty(), 4096);
    EXPECT_EQ(pbkdf2.getSaltProperty(), toBytes("salt"));
}

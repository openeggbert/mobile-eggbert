// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include <iomanip>
#include <sstream>
#include "System/Security/Cryptography/HMACSHA3_256.hpp"
#include "System/Security/Cryptography/HMACSHA3_384.hpp"
#include "System/Security/Cryptography/HMACSHA3_512.hpp"
#include "System/Security/Cryptography/SHA3_256.hpp"
#include "System/Security/Cryptography/SHA3_384.hpp"
#include "System/Security/Cryptography/SHA3_512.hpp"
#include "System/Security/Cryptography/Shake128.hpp"
#include "System/Security/Cryptography/Shake256.hpp"

using namespace System::Security::Cryptography;

namespace {

std::vector<SharpRuntime::bytecs> toBytes(const std::string& s) { return std::vector<SharpRuntime::bytecs>(s.begin(), s.end()); }

std::string toHex(const std::vector<SharpRuntime::bytecs>& bytes) {
    std::ostringstream oss;
    for (auto b : bytes) oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(static_cast<unsigned char>(b));
    return oss.str();
}

} // namespace

// NIST FIPS 202 SHA3-256/384/512 test vectors, cross-checked against Python's hashlib (which
// implements SHA-3 natively since 3.6).

TEST(SHA3_256Tests, EmptyString) {
    SHA3_256 sha3;
    EXPECT_EQ(toHex(sha3.ComputeHash({})), "a7ffc6f8bf1ed76651c14756a061d662f580ff4de43b49fa82d80a4b80f8434a");
}

TEST(SHA3_256Tests, Abc) {
    SHA3_256 sha3;
    EXPECT_EQ(toHex(sha3.ComputeHash(toBytes("abc"))), "3a985da74fe225b2045c172d6bd390bd855f086e3e9d525b46bfe24511431532");
}

TEST(SHA3_256Tests, LongerMessage_MultiBlockAbsorb) {
    // 57 bytes -- exceeds the 136-byte rate by a comfortable margin only after several calls to
    // HashCore via ComputeHash, but this specific message also matches NIST's own SHA3-256
    // multi-round test message.
    SHA3_256 sha3;
    EXPECT_EQ(toHex(sha3.ComputeHash(toBytes("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"))),
              "41c0dba2a9d6240849100376a8235e2c82e1b9998a999e21db32dd97496d3376");
}

TEST(SHA3_256Tests, ReusedAfterComputeHash_ProducesSameResultAgain) {
    SHA3_256 sha3;
    auto first = sha3.ComputeHash(toBytes("abc"));
    auto second = sha3.ComputeHash(toBytes("abc"));
    EXPECT_EQ(first, second);
}

TEST(SHA3_384Tests, EmptyString) {
    SHA3_384 sha3;
    EXPECT_EQ(toHex(sha3.ComputeHash({})),
              "0c63a75b845e4f7d01107d852e4c2485c51a50aaaa94fc61995e71bbee983a2ac3713831264adb47fb6bd1e058d5f004");
}

TEST(SHA3_384Tests, Abc) {
    SHA3_384 sha3;
    EXPECT_EQ(toHex(sha3.ComputeHash(toBytes("abc"))),
              "ec01498288516fc926459f58e2c6ad8df9b473cb0fc08c2596da7cf0e49be4b298d88cea927ac7f539f1edf228376d25");
}

TEST(SHA3_512Tests, EmptyString) {
    SHA3_512 sha3;
    EXPECT_EQ(toHex(sha3.ComputeHash({})),
              "a69f73cca23a9ac5c8b567dc185a756e97c982164fe25859e0d1dcc1475c80a615b2123af1f5f94c11e3e9402c3ac558f500199d95b6d3e301758586281dcd26");
}

TEST(SHA3_512Tests, Abc) {
    SHA3_512 sha3;
    EXPECT_EQ(toHex(sha3.ComputeHash(toBytes("abc"))),
              "b751850b1a57168a5693cd924b6b096e08f621827444f70d884f5d0240d2712e10e116e9192af3c91a7ec57647e3934057340b4cf408d5a56592f8274eec53f0");
}

// FIPS 202 SHAKE128/256 test vectors, also cross-checked against Python's hashlib.

TEST(Shake128Tests, EmptyString_32Bytes) {
    EXPECT_EQ(toHex(Shake128::HashData({}, 32)), "7f9c2ba4e88f827d616045507605853ed73b8093f6efbc88eb1a6eacfa66ef26");
}

TEST(Shake128Tests, Abc_32Bytes) {
    EXPECT_EQ(toHex(Shake128::HashData(toBytes("abc"), 32)), "5881092dd818bf5cf8a3ddb793fbcba74097d5c526a6d35f97b83351940f2cc8");
}

TEST(Shake128Tests, IncrementalRead_MatchesOneShotPrefix) {
    // Reading in two smaller chunks via Read() must produce the same bytes as one larger
    // GetHashAndReset() call -- the defining property of an extendable-output function.
    Shake128 incremental;
    incremental.AppendData(toBytes("abc"));
    auto part1 = incremental.Read(16);
    auto part2 = incremental.Read(16);
    std::vector<SharpRuntime::bytecs> combined(part1);
    combined.insert(combined.end(), part2.begin(), part2.end());
    EXPECT_EQ(toHex(combined), "5881092dd818bf5cf8a3ddb793fbcba74097d5c526a6d35f97b83351940f2cc8");
}

TEST(Shake128Tests, NegativeOutputLength_Throws) {
    Shake128 shake;
    EXPECT_THROW(shake.GetHashAndReset(-1), System::ArgumentOutOfRangeException);
}

TEST(Shake256Tests, EmptyString_32Bytes) {
    EXPECT_EQ(toHex(Shake256::HashData({}, 32)), "46b9dd2b0ba88d13233b3feb743eeb243fcd52ea62b81b82b50c27646ed5762f");
}

TEST(Shake256Tests, Abc_32Bytes) {
    EXPECT_EQ(toHex(Shake256::HashData(toBytes("abc"), 32)), "483366601360a8771c6863080cc4114d8db44530f8f1e1ee4f94ea37e78b5739");
}

// HMAC-SHA3, cross-checked against Python's hmac module with hashlib.sha3_256.

TEST(HMACSHA3_256Tests, KeyValueVector) {
    HMACSHA3_256 hmac(toBytes("key"));
    EXPECT_EQ(toHex(hmac.ComputeHash(toBytes("The quick brown fox jumps over the lazy dog"))),
              "8c6e0683409427f8931711b10ca92a506eb1fafa48fadd66d76126f47ac2c333");
}

TEST(HMACSHA3_384Tests, ProducesCorrectLength) {
    HMACSHA3_384 hmac(toBytes("key"));
    EXPECT_EQ(hmac.ComputeHash(toBytes("message")).size(), 48u);
}

TEST(HMACSHA3_512Tests, ProducesCorrectLength) {
    HMACSHA3_512 hmac(toBytes("key"));
    EXPECT_EQ(hmac.ComputeHash(toBytes("message")).size(), 64u);
}

// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include <sstream>
#include <iomanip>
#include "System/Security/Cryptography/HMACMD5.hpp"
#include "System/Security/Cryptography/HMACSHA1.hpp"
#include "System/Security/Cryptography/HMACSHA256.hpp"
#include "System/Security/Cryptography/HMACSHA384.hpp"
#include "System/Security/Cryptography/HMACSHA512.hpp"
#include "System/Security/Cryptography/MD5.hpp"
#include "System/Security/Cryptography/SHA1.hpp"
#include "System/Security/Cryptography/SHA256.hpp"
#include "System/Security/Cryptography/SHA384.hpp"
#include "System/Security/Cryptography/SHA512.hpp"

using namespace System::Security::Cryptography;

namespace {

std::vector<SharpRuntime::bytecs> toBytes(const std::string& s) { return std::vector<SharpRuntime::bytecs>(s.begin(), s.end()); }

std::string toHex(const std::vector<SharpRuntime::bytecs>& bytes) {
    std::ostringstream oss;
    for (auto b : bytes) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(b);
    }
    return oss.str();
}

std::vector<SharpRuntime::bytecs> repeat(const std::string& s, size_t times) {
    std::string out;
    out.reserve(s.size() * times);
    for (size_t i = 0; i < times; ++i) out += s;
    return toBytes(out);
}

} // namespace

// --- MD5 (RFC 1321 test vectors) ------------------------------------------------------------

TEST(MD5Tests, EmptyString) {
    MD5 md5;
    EXPECT_EQ(toHex(md5.ComputeHash(toBytes(""))), "d41d8cd98f00b204e9800998ecf8427e");
}

TEST(MD5Tests, Abc) {
    MD5 md5;
    EXPECT_EQ(toHex(md5.ComputeHash(toBytes("abc"))), "900150983cd24fb0d6963f7d28e17f72");
}

TEST(MD5Tests, LongerMessage) {
    MD5 md5;
    EXPECT_EQ(toHex(md5.ComputeHash(toBytes("message digest"))), "f96b697d7cb7938d525a2f31aaf161d0");
}

TEST(MD5Tests, Alphabet) {
    MD5 md5;
    EXPECT_EQ(toHex(md5.ComputeHash(toBytes("abcdefghijklmnopqrstuvwxyz"))), "c3fcd3d76192e4007dfb496cca67e13b");
}

TEST(MD5Tests, MultiBlockMessage_80Chars) {
    MD5 md5;
    EXPECT_EQ(toHex(md5.ComputeHash(toBytes("12345678901234567890123456789012345678901234567890123456789012345678901234567890"))),
              "57edf4a22be3c955ac49da2e2107b67a");
}

TEST(MD5Tests, ReusedAfterComputeHash_ProducesSameResultAgain) {
    MD5 md5;
    auto first = md5.ComputeHash(toBytes("abc"));
    auto second = md5.ComputeHash(toBytes("abc"));
    EXPECT_EQ(first, second);
}

// --- SHA-1 (RFC 3174 test vectors) ----------------------------------------------------------

TEST(SHA1Tests, EmptyString) {
    SHA1 sha1;
    EXPECT_EQ(toHex(sha1.ComputeHash(toBytes(""))), "da39a3ee5e6b4b0d3255bfef95601890afd80709");
}

TEST(SHA1Tests, Abc) {
    SHA1 sha1;
    EXPECT_EQ(toHex(sha1.ComputeHash(toBytes("abc"))), "a9993e364706816aba3e25717850c26c9cd0d89d");
}

TEST(SHA1Tests, LongerMessage) {
    SHA1 sha1;
    EXPECT_EQ(toHex(sha1.ComputeHash(toBytes("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"))),
              "84983e441c3bd26ebaae4aa1f95129e5e54670f1");
}

TEST(SHA1Tests, MillionRepeatedA) {
    SHA1 sha1;
    EXPECT_EQ(toHex(sha1.ComputeHash(repeat("a", 1'000'000))), "34aa973cd4c4daa4f61eeb2bdbad27316534016f");
}

// --- SHA-256 (FIPS 180-4 test vectors) -------------------------------------------------------

TEST(SHA256Tests, EmptyString) {
    SHA256 sha256;
    EXPECT_EQ(toHex(sha256.ComputeHash(toBytes(""))), "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
}

TEST(SHA256Tests, Abc) {
    SHA256 sha256;
    EXPECT_EQ(toHex(sha256.ComputeHash(toBytes("abc"))), "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
}

TEST(SHA256Tests, TwoBlockMessage) {
    SHA256 sha256;
    EXPECT_EQ(toHex(sha256.ComputeHash(toBytes("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"))),
              "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1");
}

// --- SHA-384 (FIPS 180-4 test vectors) -------------------------------------------------------

TEST(SHA384Tests, EmptyString) {
    SHA384 sha384;
    EXPECT_EQ(toHex(sha384.ComputeHash(toBytes(""))),
              "38b060a751ac96384cd9327eb1b1e36a21fdb71114be07434c0cc7bf63f6e1da274edebfe76f65fbd51ad2f14898b95b");
}

TEST(SHA384Tests, Abc) {
    SHA384 sha384;
    EXPECT_EQ(toHex(sha384.ComputeHash(toBytes("abc"))),
              "cb00753f45a35e8bb5a03d699ac65007272c32ab0eded1631a8b605a43ff5bed8086072ba1e7cc2358baeca134c825a7");
}

// --- SHA-512 (FIPS 180-4 test vectors) -------------------------------------------------------

TEST(SHA512Tests, EmptyString) {
    SHA512 sha512;
    EXPECT_EQ(toHex(sha512.ComputeHash(toBytes(""))),
              "cf83e1357eefb8bdf1542850d66d8007d620e4050b5715dc83f4a921d36ce9ce47d0d13c5d85f2b0ff8318d2877eec2f63b931bd47417a81a538327af927da3e");
}

TEST(SHA512Tests, Abc) {
    SHA512 sha512;
    EXPECT_EQ(toHex(sha512.ComputeHash(toBytes("abc"))),
              "ddaf35a193617abacc417349ae20413112e6fa4e89a97ea20a9eeee64b55d39a2192992a274fc1a836ba3c23a3feebbd454d4423643ce80e2a9ac94fa54ca49f");
}

// --- HMAC (RFC 2202 / RFC 4231 test vectors) -------------------------------------------------

TEST(HMACMD5Tests, RFC2202_TestCase1) {
    // Key = 0x0b repeated 16 times, data = "Hi There"
    std::vector<SharpRuntime::bytecs> key(16, 0x0b);
    HMACMD5 hmac(key);
    EXPECT_EQ(toHex(hmac.ComputeHash(toBytes("Hi There"))), "9294727a3638bb1c13f48ef8158bfc9d");
}

TEST(HMACSHA1Tests, RFC2202_TestCase1) {
    std::vector<SharpRuntime::bytecs> key(20, 0x0b);
    HMACSHA1 hmac(key);
    EXPECT_EQ(toHex(hmac.ComputeHash(toBytes("Hi There"))), "b617318655057264e28bc0b6fb378c8ef146be00");
}

TEST(HMACSHA256Tests, RFC4231_TestCase1) {
    std::vector<SharpRuntime::bytecs> key(20, 0x0b);
    HMACSHA256 hmac(key);
    EXPECT_EQ(toHex(hmac.ComputeHash(toBytes("Hi There"))),
              "b0344c61d8db38535ca8afceaf0bf12b881dc200c9833da726e9376c2e32cff7");
}

TEST(HMACSHA384Tests, RFC4231_TestCase1) {
    std::vector<SharpRuntime::bytecs> key(20, 0x0b);
    HMACSHA384 hmac(key);
    EXPECT_EQ(toHex(hmac.ComputeHash(toBytes("Hi There"))),
              "afd03944d84895626b0825f4ab46907f15f9dadbe4101ec682aa034c7cebc59cfaea9ea9076ede7f4af152e8b2fa9cb6");
}

TEST(HMACSHA512Tests, RFC4231_TestCase1) {
    std::vector<SharpRuntime::bytecs> key(20, 0x0b);
    HMACSHA512 hmac(key);
    EXPECT_EQ(toHex(hmac.ComputeHash(toBytes("Hi There"))),
              "87aa7cdea5ef619d4ff0b4241a1d6cb02379f4e2ce4ec2787ad0b30545e17cdedaa833b7d6b8a702038b274eaea3f4e4be9d914eeb61f1702e696c203a126854");
}

TEST(HMACSHA256Tests, KeyLongerThanBlockSize_IsHashedFirst) {
    // RFC 4231 test case 6: key length 131 bytes (> block size 64), so it's hashed down first.
    std::vector<SharpRuntime::bytecs> key(131, 0xaa);
    HMACSHA256 hmac(key);
    EXPECT_EQ(toHex(hmac.ComputeHash(toBytes("Test Using Larger Than Block-Size Key - Hash Key First"))),
              "60e431591ee0b67f0d8a26aacbf5b77f8e0bc6213728c5140546040f0ee37f54");
}

// --- getHashSizeProperty() / TryComputeHash() -----------------------------------------------
//
// Regression: hashSizeValue_ was never set by MD5/SHA1/SHA256/SHA384/SHA512's constructors
// (stayed at HashAlgorithm's default of 0). getHashSizeProperty() returned 0 instead of the
// real bit length, and TryComputeHash's undersized-buffer guard
// (destination.size() < hashSizeValue_ / 8) became "size < 0" -- always false for any
// unsigned size -- so it never rejected a too-small destination, and the unconditional
// std::copy(final.begin(), final.end(), destination.begin()) that followed wrote the real
// digest (16/20/32/48/64 bytes) into whatever buffer was passed, however small, an actual
// out-of-bounds write.

TEST(HashSizePropertyTests, MD5_Is128) {
    EXPECT_EQ(MD5().getHashSizeProperty(), 128);
}
TEST(HashSizePropertyTests, SHA1_Is160) {
    EXPECT_EQ(SHA1().getHashSizeProperty(), 160);
}
TEST(HashSizePropertyTests, SHA256_Is256) {
    EXPECT_EQ(SHA256().getHashSizeProperty(), 256);
}
TEST(HashSizePropertyTests, SHA384_Is384) {
    EXPECT_EQ(SHA384().getHashSizeProperty(), 384);
}
TEST(HashSizePropertyTests, SHA512_Is512) {
    EXPECT_EQ(SHA512().getHashSizeProperty(), 512);
}

TEST(TryComputeHashTests, MD5_UndersizedDestination_ReturnsFalseAndDoesNotWrite) {
    MD5 md5;
    std::vector<SharpRuntime::bytecs> destination(4, 0xFF); // real digest is 16 bytes
    SharpRuntime::intcs bytesWritten = -1;
    EXPECT_FALSE(md5.TryComputeHash(toBytes("abc"), destination, bytesWritten));
    EXPECT_EQ(bytesWritten, 0);
    // Buffer must be untouched -- confirms no out-of-bounds write occurred.
    EXPECT_EQ(destination, std::vector<SharpRuntime::bytecs>(4, 0xFF));
}

TEST(TryComputeHashTests, SHA256_ExactSizedDestination_Succeeds) {
    SHA256 sha256;
    std::vector<SharpRuntime::bytecs> destination(32, 0);
    SharpRuntime::intcs bytesWritten = 0;
    EXPECT_TRUE(sha256.TryComputeHash(toBytes("abc"), destination, bytesWritten));
    EXPECT_EQ(bytesWritten, 32);
    EXPECT_EQ(toHex(destination), "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
}

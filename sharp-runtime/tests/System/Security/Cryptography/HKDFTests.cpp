// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include <iomanip>
#include <sstream>
#include "System/Security/Cryptography/HKDF.hpp"

using namespace System::Security::Cryptography;

namespace {

std::vector<SharpRuntime::bytecs> fromRepeatedByte(uint8_t byte, size_t count) {
    return std::vector<SharpRuntime::bytecs>(count, static_cast<SharpRuntime::bytecs>(byte));
}

std::vector<SharpRuntime::bytecs> fromRange(uint8_t start, uint8_t end) {
    std::vector<SharpRuntime::bytecs> out;
    for (int b = start; b <= end; ++b) out.push_back(static_cast<SharpRuntime::bytecs>(b));
    return out;
}

std::string toHex(const std::vector<SharpRuntime::bytecs>& bytes) {
    std::ostringstream oss;
    for (auto b : bytes) oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(static_cast<unsigned char>(b));
    return oss.str();
}

} // namespace

// RFC 5869 Appendix A test vectors, cross-checked against a from-scratch Python
// hmac/hashlib-based reference implementation of HKDF-Extract/Expand.

TEST(HKDFTests, RFC5869_TestCase1_Basic_SHA256) {
    auto ikm = fromRepeatedByte(0x0b, 22);
    auto salt = fromRange(0x00, 0x0c);
    auto info = fromRange(0xf0, 0xf9);

    auto prk = HKDF::Extract(HashAlgorithmName::SHA256, ikm, salt);
    EXPECT_EQ(toHex(prk), "077709362c2e32df0ddc3f0dc47bba6390b6c73bb50f9c3122ec844ad7c2b3e5");

    auto okm = HKDF::Expand(HashAlgorithmName::SHA256, prk, 42, info);
    EXPECT_EQ(toHex(okm), "3cb25f25faacd57a90434f64d0362f2a2d2d0a90cf1a5a4c5db02d56ecc4c5bf34007208d5b887185865");

    // DeriveKey must produce the same OKM in one call.
    EXPECT_EQ(toHex(HKDF::DeriveKey(HashAlgorithmName::SHA256, ikm, 42, salt, info)), toHex(okm));
}

TEST(HKDFTests, RFC5869_TestCase3_ZeroLengthSaltAndInfo_SHA256) {
    auto ikm = fromRepeatedByte(0x0b, 22);

    auto prk = HKDF::Extract(HashAlgorithmName::SHA256, ikm);
    EXPECT_EQ(toHex(prk), "19ef24a32c717b167f33a91d6f648bdf96596776afdb6377ac434c1c293ccb04");

    auto okm = HKDF::Expand(HashAlgorithmName::SHA256, prk, 42);
    EXPECT_EQ(toHex(okm), "8da4e775a563c18f715f802a063c5a31b8a11f5c5ee1879ec3454e5f3c738d2d9d201395faa4b61a96c8");
}

TEST(HKDFTests, Expand_NegativeLength_Throws) {
    std::vector<SharpRuntime::bytecs> prk(32, 0);
    EXPECT_THROW(HKDF::Expand(HashAlgorithmName::SHA256, prk, -1), System::ArgumentOutOfRangeException);
}

TEST(HKDFTests, Expand_LengthExceedsMaximum_Throws) {
    std::vector<SharpRuntime::bytecs> prk(32, 0);
    EXPECT_THROW(HKDF::Expand(HashAlgorithmName::SHA256, prk, 255 * 32 + 1), System::ArgumentOutOfRangeException);
}

TEST(HKDFTests, DeriveKey_ProducesRequestedLength) {
    auto ikm = fromRepeatedByte(0x42, 16);
    auto okm = HKDF::DeriveKey(HashAlgorithmName::SHA256, ikm, 100);
    EXPECT_EQ(okm.size(), 100u);
}

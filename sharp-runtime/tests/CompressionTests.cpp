// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include <gtest/gtest.h>
#include "System/IO/Compression/GZipStream.hpp"
#include "System/IO/Compression/DeflateStream.hpp"
#include "System/IO/InvalidDataException.hpp"
#include "System/IO/MemoryStream.hpp"
#include "System/NotSupportedException.hpp"
#include <string>
#include <vector>
#include <cstdint>

using namespace System::IO;
using namespace System::IO::Compression;
using SharpRuntime::bytecs;
using SharpRuntime::intcs;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static std::vector<bytecs> toBytes(const std::string& s) {
    return std::vector<bytecs>(s.begin(), s.end());
}

static std::vector<bytecs> gzipCompress(const std::vector<bytecs>& input) {
    MemoryStream compressed;
    {
        GZipStream gz(&compressed, CompressionMode::Compress, /*leaveOpen=*/true);
        gz.Write(input.data(), 0, static_cast<intcs>(input.size()));
        gz.Close();
    }
    return compressed.ToArray();
}

static std::vector<bytecs> gzipDecompress(const std::vector<bytecs>& compressed,
                                           intcs outputSize) {
    MemoryStream src(compressed.data(), static_cast<intcs>(compressed.size()));
    GZipStream gz(&src, CompressionMode::Decompress, /*leaveOpen=*/true);
    std::vector<bytecs> out(static_cast<size_t>(outputSize));
    intcs n = gz.Read(out.data(), 0, outputSize);
    out.resize(static_cast<size_t>(n));
    return out;
}

static std::vector<bytecs> deflateCompress(const std::vector<bytecs>& input) {
    MemoryStream compressed;
    {
        DeflateStream ds(&compressed, CompressionMode::Compress, /*leaveOpen=*/true);
        ds.Write(input.data(), 0, static_cast<intcs>(input.size()));
        ds.Close();
    }
    return compressed.ToArray();
}

static std::vector<bytecs> deflateDecompress(const std::vector<bytecs>& compressed,
                                              intcs outputSize) {
    MemoryStream src(compressed.data(), static_cast<intcs>(compressed.size()));
    DeflateStream ds(&src, CompressionMode::Decompress, /*leaveOpen=*/true);
    std::vector<bytecs> out(static_cast<size_t>(outputSize));
    intcs n = ds.Read(out.data(), 0, outputSize);
    out.resize(static_cast<size_t>(n));
    return out;
}

// ---------------------------------------------------------------------------
// GZipStream tests
// ---------------------------------------------------------------------------

TEST(GZipStream, CompressProducesGZipMagicBytes) {
    auto input = toBytes("hello");
    auto compressed = gzipCompress(input);
    ASSERT_GE(static_cast<intcs>(compressed.size()), 2);
    EXPECT_EQ(compressed[0], static_cast<bytecs>(0x1f));
    EXPECT_EQ(compressed[1], static_cast<bytecs>(0x8b));
}

TEST(GZipStream, CompressProducesNonEmptyOutput) {
    auto input = toBytes("hello world");
    auto compressed = gzipCompress(input);
    EXPECT_GT(compressed.size(), 0u);
}

TEST(GZipStream, RoundTripShortString) {
    auto input = toBytes("Hello, GZip!");
    auto compressed   = gzipCompress(input);
    auto decompressed = gzipDecompress(compressed, static_cast<intcs>(input.size()));
    EXPECT_EQ(decompressed, input);
}

TEST(GZipStream, RoundTripLongRepetitiveData) {
    // 200 KB of a repeating pattern — should compress very well
    std::vector<bytecs> input(200 * 1024);
    for (size_t i = 0; i < input.size(); ++i)
        input[i] = static_cast<bytecs>(i % 251);
    auto compressed   = gzipCompress(input);
    EXPECT_LT(compressed.size(), input.size()); // should actually compress
    auto decompressed = gzipDecompress(compressed, static_cast<intcs>(input.size()));
    EXPECT_EQ(decompressed, input);
}

TEST(GZipStream, RoundTripBinaryZeroes) {
    std::vector<bytecs> input(1024, 0);
    auto compressed   = gzipCompress(input);
    auto decompressed = gzipDecompress(compressed, static_cast<intcs>(input.size()));
    EXPECT_EQ(decompressed, input);
}

TEST(GZipStream, RoundTripSingleByte) {
    std::vector<bytecs> input = {42};
    auto compressed   = gzipCompress(input);
    auto decompressed = gzipDecompress(compressed, 1);
    EXPECT_EQ(decompressed, input);
}

TEST(GZipStream, CompressedIsSmallerThanUncompressibleForRepetitive) {
    std::string repeated(10000, 'A');
    auto input = toBytes(repeated);
    auto compressed = gzipCompress(input);
    EXPECT_LT(compressed.size(), input.size());
}

TEST(GZipStream, CanReadIsTrueForDecompress) {
    MemoryStream ms;
    GZipStream gz(&ms, CompressionMode::Decompress, true);
    EXPECT_TRUE(gz.getCanReadProperty());
    EXPECT_FALSE(gz.getCanWriteProperty());
}

TEST(GZipStream, CanWriteIsTrueForCompress) {
    MemoryStream ms;
    GZipStream gz(&ms, CompressionMode::Compress, true);
    EXPECT_FALSE(gz.getCanReadProperty());
    EXPECT_TRUE(gz.getCanWriteProperty());
}

TEST(GZipStream, GetLengthThrows) {
    MemoryStream ms;
    GZipStream gz(&ms, CompressionMode::Compress, true);
    EXPECT_THROW((void)gz.getLengthProperty(), System::NotSupportedException);
}

TEST(GZipStream, RoundTripMultipleWrites) {
    // Write input in three chunks, then decompress as one block
    auto input = toBytes("abcdefghijklmnopqrstuvwxyz0123456789");
    MemoryStream compressed;
    {
        GZipStream gz(&compressed, CompressionMode::Compress, true);
        intcs third = static_cast<intcs>(input.size()) / 3;
        gz.Write(input.data(), 0,         third);
        gz.Write(input.data(), third,     third);
        gz.Write(input.data(), third * 2, static_cast<intcs>(input.size()) - third * 2);
        gz.Close();
    }
    const auto& compBytes = compressed.ToArray();
    MemoryStream src(compBytes.data(), static_cast<intcs>(compBytes.size()));
    GZipStream gz2(&src, CompressionMode::Decompress, true);
    std::vector<bytecs> out(input.size());
    intcs n = gz2.Read(out.data(), 0, static_cast<intcs>(out.size()));
    out.resize(static_cast<size_t>(n));
    EXPECT_EQ(out, input);
}

// ---------------------------------------------------------------------------
// DeflateStream tests
// ---------------------------------------------------------------------------

TEST(DeflateStream, CompressProducesNonEmptyOutput) {
    auto input = toBytes("hello world");
    auto compressed = deflateCompress(input);
    EXPECT_GT(compressed.size(), 0u);
}

TEST(DeflateStream, RoundTripShortString) {
    auto input = toBytes("Hello, Deflate!");
    auto compressed   = deflateCompress(input);
    auto decompressed = deflateDecompress(compressed, static_cast<intcs>(input.size()));
    EXPECT_EQ(decompressed, input);
}

TEST(DeflateStream, RoundTripLongRepetitiveData) {
    std::vector<bytecs> input(200 * 1024);
    for (size_t i = 0; i < input.size(); ++i)
        input[i] = static_cast<bytecs>(i % 251);
    auto compressed   = deflateCompress(input);
    EXPECT_LT(compressed.size(), input.size());
    auto decompressed = deflateDecompress(compressed, static_cast<intcs>(input.size()));
    EXPECT_EQ(decompressed, input);
}

TEST(DeflateStream, RoundTripBinaryZeroes) {
    std::vector<bytecs> input(1024, 0);
    auto compressed   = deflateCompress(input);
    auto decompressed = deflateDecompress(compressed, static_cast<intcs>(input.size()));
    EXPECT_EQ(decompressed, input);
}

TEST(DeflateStream, RoundTripSingleByte) {
    std::vector<bytecs> input = {99};
    auto compressed   = deflateCompress(input);
    auto decompressed = deflateDecompress(compressed, 1);
    EXPECT_EQ(decompressed, input);
}

TEST(DeflateStream, NoGZipMagicHeader) {
    auto input = toBytes("test");
    auto compressed = deflateCompress(input);
    // Raw deflate must NOT start with gzip magic 0x1f 0x8b
    if (compressed.size() >= 2) {
        bool isGzip = (compressed[0] == 0x1f && compressed[1] == static_cast<bytecs>(0x8b));
        EXPECT_FALSE(isGzip);
    }
}

TEST(DeflateStream, CanReadIsTrueForDecompress) {
    MemoryStream ms;
    DeflateStream ds(&ms, CompressionMode::Decompress, true);
    EXPECT_TRUE(ds.getCanReadProperty());
    EXPECT_FALSE(ds.getCanWriteProperty());
}

TEST(DeflateStream, CanWriteIsTrueForCompress) {
    MemoryStream ms;
    DeflateStream ds(&ms, CompressionMode::Compress, true);
    EXPECT_FALSE(ds.getCanReadProperty());
    EXPECT_TRUE(ds.getCanWriteProperty());
}

TEST(DeflateStream, GetLengthThrows) {
    MemoryStream ms;
    DeflateStream ds(&ms, CompressionMode::Compress, true);
    EXPECT_THROW((void)ds.getLengthProperty(), System::NotSupportedException);
}

TEST(DeflateStream, DeflateAndGZipOutputDiffer) {
    auto input = toBytes("same input different format");
    auto gzipped  = gzipCompress(input);
    auto deflated = deflateCompress(input);
    // GZip has a header so bytes differ from raw deflate
    EXPECT_NE(gzipped, deflated);
}

TEST(DeflateStream, GZipCannotDecompressRawDeflate) {
    auto input    = toBytes("mismatch test");
    auto deflated = deflateCompress(input);
    MemoryStream src(deflated.data(), static_cast<intcs>(deflated.size()));
    GZipStream gz(&src, CompressionMode::Decompress, true);
    std::vector<bytecs> out(1024);
    EXPECT_THROW(gz.Read(out.data(), 0, static_cast<intcs>(out.size())), System::IO::InvalidDataException);
}

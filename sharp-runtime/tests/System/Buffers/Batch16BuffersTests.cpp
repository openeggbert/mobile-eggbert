// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Decimal.hpp"
#include "System/String.hpp"
#include "System/Buffers/ArrayPool.hpp"
#include "System/Buffers/MemoryHandle.hpp"
#include "System/Buffers/IPinnable.hpp"
#include "System/Buffers/MemoryManager.hpp"
#include "System/NotSupportedException.hpp"
#include "System/Buffers/SearchValues.hpp"
#include "System/Buffers/SequenceReaderExtensions.hpp"

// ===========================================================================
// Decimal — FromOACurrency / ToOACurrency
// ===========================================================================

TEST(DecimalOACurrencyTests, FromOACurrency_Zero) {
    System::Decimal d = System::Decimal::FromOACurrency(0LL);
    EXPECT_EQ(d, System::Decimal::Zero);
}

TEST(DecimalOACurrencyTests, FromOACurrency_10000_IsOne) {
    System::Decimal d = System::Decimal::FromOACurrency(10000LL);
    EXPECT_EQ(d, System::Decimal(1));
}

TEST(DecimalOACurrencyTests, FromOACurrency_15000_IsOnePointFive) {
    System::Decimal d = System::Decimal::FromOACurrency(15000LL);
    System::Decimal expected = System::Decimal::Parse("1.5");
    EXPECT_EQ(d, expected);
}

TEST(DecimalOACurrencyTests, FromOACurrency_Negative) {
    System::Decimal d = System::Decimal::FromOACurrency(-10000LL);
    EXPECT_EQ(d, System::Decimal(-1));
}

TEST(DecimalOACurrencyTests, ToOACurrency_Zero) {
    EXPECT_EQ(System::Decimal::Zero.ToOACurrency(), 0LL);
}

TEST(DecimalOACurrencyTests, ToOACurrency_One_Is10000) {
    System::Decimal one(1);
    EXPECT_EQ(one.ToOACurrency(), 10000LL);
}

TEST(DecimalOACurrencyTests, ToOACurrency_Static_One_Is10000) {
    EXPECT_EQ(System::Decimal::ToOACurrency(System::Decimal(1)), 10000LL);
}

TEST(DecimalOACurrencyTests, RoundTrip) {
    long long cy = 12345LL;
    System::Decimal d = System::Decimal::FromOACurrency(cy);
    EXPECT_EQ(d.ToOACurrency(), cy);
}

// ===========================================================================
// String — Intern / IsInterned
// ===========================================================================

TEST(StringInternTests, Intern_ReturnsInputUnchanged) {
    std::string s = "hello";
    EXPECT_EQ(System::String::Intern(s), s);
}

TEST(StringInternTests, IsInterned_ReturnsInputUnchanged) {
    std::string s = "world";
    EXPECT_EQ(System::String::IsInterned(s), s);
}

TEST(StringInternTests, Intern_EmptyString) {
    EXPECT_EQ(System::String::Intern(""), "");
}

// ===========================================================================
// ArrayPool — Create factory
// ===========================================================================

TEST(ArrayPoolCreateTests, Create_NoArgs_CanRentAndReturn) {
    auto pool = System::Buffers::ArrayPool<int>::Create();
    auto v = pool->Rent(10);
    EXPECT_GE(static_cast<int>(v.size()), 10);
    pool->Return(v);
    SUCCEED();
}

TEST(ArrayPoolCreateTests, Create_WithArgs_CanRent) {
    auto pool = System::Buffers::ArrayPool<double>::Create(1024, 50);
    auto v = pool->Rent(5);
    EXPECT_GE(static_cast<int>(v.size()), 5);
    pool->Return(v);
    SUCCEED();
}

TEST(ArrayPoolCreateTests, Create_ClearOnReturn) {
    auto pool = System::Buffers::ArrayPool<int>::Create();
    auto v = pool->Rent(4);
    v[0] = 99;
    pool->Return(v, true);  // clearArray = true
    // After clearing, the vector should have been zeroed
    // (check indirectly — just verify no crash)
    SUCCEED();
}

// ===========================================================================
// MemoryHandle
// ===========================================================================

TEST(MemoryHandleTests, DefaultCtor_PointerNull) {
    System::Buffers::MemoryHandle mh;
    EXPECT_EQ(mh.getPointerProperty(), nullptr);
}

TEST(MemoryHandleTests, Ctor_StoresPointer) {
    int x = 42;
    System::Buffers::MemoryHandle mh(static_cast<void*>(&x));
    EXPECT_EQ(mh.getPointerProperty(), static_cast<void*>(&x));
}

TEST(MemoryHandleTests, Dispose_ClearsPointer) {
    int x = 42;
    System::Buffers::MemoryHandle mh(static_cast<void*>(&x));
    mh.Dispose();
    EXPECT_EQ(mh.getPointerProperty(), nullptr);
}

// ===========================================================================
// IPinnable (via a concrete stub implementing it)
// ===========================================================================

namespace {
struct StubPinnable : System::Buffers::IPinnable {
    bool unpinCalled = false;
    System::Buffers::MemoryHandle Pin(int /*elementIndex*/) override {
        return System::Buffers::MemoryHandle(reinterpret_cast<void*>(0x1234), this);
    }
    void Unpin() override { unpinCalled = true; }
};
} // anonymous namespace

TEST(IPinnableTests, Pin_ReturnsHandle) {
    StubPinnable pinnable;
    auto mh = pinnable.Pin(0);
    EXPECT_NE(mh.getPointerProperty(), nullptr);
}

TEST(IPinnableTests, DisposeHandle_CallsUnpin) {
    StubPinnable pinnable;
    {
        auto mh = pinnable.Pin(0);
        mh.Dispose();
    }
    EXPECT_TRUE(pinnable.unpinCalled);
}

// ===========================================================================
// MemoryManager (via a concrete stub)
// ===========================================================================

namespace {
struct StubMemoryManager : System::Buffers::MemoryManager<int> {
    std::vector<int> data_{10, 20, 30};

    System::Span<int> GetSpan() override {
        return System::Span<int>(data_.data(), static_cast<int>(data_.size()));
    }
    System::Buffers::MemoryHandle Pin(int /*elementIndex*/) override {
        return System::Buffers::MemoryHandle(data_.data());
    }
    void Unpin() override {}
};
} // anonymous namespace

TEST(MemoryManagerTests, GetSpan_ReturnsNonEmpty) {
    StubMemoryManager mgr;
    auto span = mgr.GetSpan();
    EXPECT_EQ(span.getLengthProperty(), 3);
}

TEST(MemoryManagerTests, Pin_ReturnsValidHandle) {
    StubMemoryManager mgr;
    auto mh = mgr.Pin(0);
    EXPECT_NE(mh.getPointerProperty(), nullptr);
}

TEST(MemoryManagerTests, Dispose_NoThrow) {
    StubMemoryManager mgr;
    EXPECT_NO_THROW(mgr.Dispose());
}

TEST(MemoryManagerTests, GetMemoryProperty_Throws) {
    // This port's Memory<T> doesn't support manager-backed storage, so the
    // default .Memory implementation throws NotSupportedException (matching
    // this codebase's established exception hierarchy) rather than a bare
    // std::runtime_error.
    StubMemoryManager mgr;
    EXPECT_THROW(mgr.getMemoryProperty(), System::NotSupportedException);
}

// ===========================================================================
// SearchValues
// ===========================================================================

TEST(SearchValuesTests, Contains_ValuePresent) {
    System::Buffers::SearchValues<int> sv{1, 2, 3, 4, 5};
    EXPECT_TRUE(sv.Contains(3));
}

TEST(SearchValuesTests, Contains_ValueAbsent) {
    System::Buffers::SearchValues<int> sv{1, 2, 3};
    EXPECT_FALSE(sv.Contains(99));
}

TEST(SearchValuesTests, Contains_Byte) {
    System::Buffers::SearchValues<uint8_t> sv{0xAB, 0xCD};
    EXPECT_TRUE(sv.Contains(0xAB));
    EXPECT_FALSE(sv.Contains(0x00));
}

TEST(SearchValuesTests, Contains_Char) {
    System::Buffers::SearchValues<char> sv{'a', 'e', 'i', 'o', 'u'};
    EXPECT_TRUE(sv.Contains('e'));
    EXPECT_FALSE(sv.Contains('b'));
}

TEST(SearchValuesTests, GetValues_NonEmpty) {
    System::Buffers::SearchValues<int> sv{10, 20, 30};
    auto vals = sv.GetValues();
    EXPECT_EQ(vals.size(), 3u);
}

TEST(SearchValuesFactoryTests, Create_Byte_Contains) {
    auto sv = System::Buffers::SearchValuesFactory::Create({uint8_t{1}, uint8_t{2}, uint8_t{3}});
    EXPECT_TRUE(sv.Contains(2));
    EXPECT_FALSE(sv.Contains(4));
}

TEST(SearchValuesFactoryTests, Create_Char_Contains) {
    auto sv = System::Buffers::SearchValuesFactory::Create({'x', 'y', 'z'});
    EXPECT_TRUE(sv.Contains('y'));
    EXPECT_FALSE(sv.Contains('a'));
}

TEST(SearchValuesFactoryTests, Create_Vector_Contains) {
    std::vector<int> vals{100, 200, 300};
    auto sv = System::Buffers::SearchValuesFactory::Create(vals);
    EXPECT_TRUE(sv.Contains(200));
    EXPECT_FALSE(sv.Contains(999));
}

// ===========================================================================
// SequenceReaderExtensions — binary reading
// ===========================================================================

TEST(SequenceReaderExtensionsTests, TryReadLittleEndian_Int16) {
    std::vector<uint8_t> data{0x01, 0x00};  // little-endian 1
    System::Buffers::ReadOnlySequence<uint8_t> seq(data);
    System::Buffers::SequenceReader<uint8_t> reader(seq);
    int16_t value = 0;
    bool ok = System::Buffers::SequenceReaderExtensions::TryReadLittleEndian(reader, value);
    EXPECT_TRUE(ok);
    EXPECT_EQ(value, 1);
}

TEST(SequenceReaderExtensionsTests, TryReadBigEndian_Int16) {
    std::vector<uint8_t> data{0x00, 0x01};  // big-endian 1
    System::Buffers::ReadOnlySequence<uint8_t> seq(data);
    System::Buffers::SequenceReader<uint8_t> reader(seq);
    int16_t value = 0;
    bool ok = System::Buffers::SequenceReaderExtensions::TryReadBigEndian(reader, value);
    EXPECT_TRUE(ok);
    EXPECT_EQ(value, 1);
}

TEST(SequenceReaderExtensionsTests, TryReadLittleEndian_Int32) {
    std::vector<uint8_t> data{0x05, 0x00, 0x00, 0x00};  // little-endian 5
    System::Buffers::ReadOnlySequence<uint8_t> seq(data);
    System::Buffers::SequenceReader<uint8_t> reader(seq);
    int32_t value = 0;
    bool ok = System::Buffers::SequenceReaderExtensions::TryReadLittleEndian(reader, value);
    EXPECT_TRUE(ok);
    EXPECT_EQ(value, 5);
}

TEST(SequenceReaderExtensionsTests, TryReadBigEndian_Int32) {
    std::vector<uint8_t> data{0x00, 0x00, 0x00, 0x05};  // big-endian 5
    System::Buffers::ReadOnlySequence<uint8_t> seq(data);
    System::Buffers::SequenceReader<uint8_t> reader(seq);
    int32_t value = 0;
    bool ok = System::Buffers::SequenceReaderExtensions::TryReadBigEndian(reader, value);
    EXPECT_TRUE(ok);
    EXPECT_EQ(value, 5);
}

TEST(SequenceReaderExtensionsTests, TryRead_NotEnoughData_ReturnsFalse) {
    std::vector<uint8_t> data{0x01};  // only 1 byte, need 2 for int16
    System::Buffers::ReadOnlySequence<uint8_t> seq(data);
    System::Buffers::SequenceReader<uint8_t> reader(seq);
    int16_t value = -1;
    bool ok = System::Buffers::SequenceReaderExtensions::TryReadLittleEndian(reader, value);
    EXPECT_FALSE(ok);
    EXPECT_EQ(value, 0);
}

TEST(SequenceReaderExtensionsTests, TryReadLittleEndian_Int64) {
    std::vector<uint8_t> data{0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};  // LE 7
    System::Buffers::ReadOnlySequence<uint8_t> seq(data);
    System::Buffers::SequenceReader<uint8_t> reader(seq);
    int64_t value = 0;
    bool ok = System::Buffers::SequenceReaderExtensions::TryReadLittleEndian(reader, value);
    EXPECT_TRUE(ok);
    EXPECT_EQ(value, 7LL);
}

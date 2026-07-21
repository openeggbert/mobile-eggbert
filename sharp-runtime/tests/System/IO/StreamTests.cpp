// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include <cstdint>
#include <string>
#include <vector>

#include "System/IO/MemoryStream.hpp"
#include "System/IO/SeekOrigin.hpp"
#include "System/IO/StringReader.hpp"
#include "System/IO/StringWriter.hpp"
#include "System/IO/UnmanagedMemoryStream.hpp"
#include "System/IO/UnmanagedMemoryAccessor.hpp"
#include "System/IO/FileAccess.hpp"
#include "System/IO/IOException.hpp"
#include "System/ArgumentNullException.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/NotSupportedException.hpp"
#include "System/ObjectDisposedException.hpp"

using System::IO::MemoryStream;
using System::IO::SeekOrigin;
using System::IO::StringReader;
using System::IO::StringWriter;
using System::IO::UnmanagedMemoryStream;
using System::IO::UnmanagedMemoryAccessor;
using System::IO::FileAccess;
using System::IO::intcs;

// ---------------------------------------------------------------------------
// MemoryStream — writable (default ctor)
// ---------------------------------------------------------------------------

TEST(MemoryStreamTests, DefaultCtorIsEmpty) {
    MemoryStream ms;
    EXPECT_EQ(ms.getLengthProperty(), 0);
    EXPECT_TRUE(ms.getCanWriteProperty());
}

TEST(MemoryStreamTests, WriteByte) {
    MemoryStream ms;
    ms.WriteByte(0x41); // 'A'
    EXPECT_EQ(ms.getLengthProperty(), 1);
    EXPECT_EQ(ms.ToArray()[0], 0x41u);
}

TEST(MemoryStreamTests, WriteMultipleBytes) {
    MemoryStream ms;
    ms.WriteByte(1); ms.WriteByte(2); ms.WriteByte(3);
    EXPECT_EQ(ms.getLengthProperty(), 3);
    EXPECT_EQ(ms.ToArray()[2], 3u);
}

TEST(MemoryStreamTests, WriteBuffer) {
    MemoryStream ms;
    uint8_t data[] = {10, 20, 30, 40};
    ms.Write(data, 0, 4);
    EXPECT_EQ(ms.getLengthProperty(), 4);
    const auto& arr = ms.ToArray();
    EXPECT_EQ(arr[0], 10u);
    EXPECT_EQ(arr[3], 40u);
}

TEST(MemoryStreamTests, WriteBufferWithOffset) {
    MemoryStream ms;
    uint8_t data[] = {0, 0, 99, 100, 101};
    ms.Write(data, 2, 3); // skip first 2 bytes
    EXPECT_EQ(ms.getLengthProperty(), 3);
    EXPECT_EQ(ms.ToArray()[0], 99u);
    EXPECT_EQ(ms.ToArray()[2], 101u);
}

// Post-stabilization audit ticket 1714: Write() previously only checked
// `buffer == nullptr || count <= 0` and silently returned for everything else -- a negative
// offset reached std::copy(buffer + offset, ...) unchecked, an out-of-bounds read confirmed via
// a standalone ASan repro (stack-buffer-underflow), not just a silent no-op.
TEST(MemoryStreamTests, WriteNullBufferThrowsArgumentNullException) {
    MemoryStream ms;
    EXPECT_THROW(ms.Write(nullptr, 0, 10), System::ArgumentNullException);
}

TEST(MemoryStreamTests, WriteNegativeOffsetThrowsArgumentOutOfRangeException) {
    MemoryStream ms;
    uint8_t data[] = {1, 2, 3, 4};
    EXPECT_THROW(ms.Write(data, -1, 2), System::ArgumentOutOfRangeException);
    // Stream must be left unmodified -- no partial/corrupted write occurred.
    EXPECT_EQ(ms.getLengthProperty(), 0);
}

TEST(MemoryStreamTests, WriteNegativeCountThrowsArgumentOutOfRangeException) {
    MemoryStream ms;
    uint8_t data[] = {1, 2, 3, 4};
    EXPECT_THROW(ms.Write(data, 0, -1), System::ArgumentOutOfRangeException);
    EXPECT_EQ(ms.getLengthProperty(), 0);
}

TEST(MemoryStreamTests, WriteZeroCountIsNoOpNotThrow) {
    MemoryStream ms;
    uint8_t data[] = {1, 2, 3, 4};
    EXPECT_NO_THROW(ms.Write(data, 0, 0));
    EXPECT_EQ(ms.getLengthProperty(), 0);
}

TEST(MemoryStreamTests, WriteValidArgumentsStillWorksAfterValidationAdded) {
    MemoryStream ms;
    uint8_t data[] = {5, 6, 7, 8};
    ms.Write(data, 1, 2);
    EXPECT_EQ(ms.getLengthProperty(), 2);
    EXPECT_EQ(ms.ToArray()[0], 6u);
    EXPECT_EQ(ms.ToArray()[1], 7u);
}

TEST(MemoryStreamTests, GetBufferReturnsLiveReference) {
    MemoryStream ms;
    ms.WriteByte(7);
    const auto& buf1 = ms.GetBuffer();
    const auto& buf2 = ms.GetBuffer();
    EXPECT_EQ(&buf1, &buf2);
    EXPECT_EQ(buf1.size(), 1u);
    EXPECT_EQ(buf1[0], 7u);
}

TEST(MemoryStreamTests, ToArrayReturnsIndependentCopy) {
    MemoryStream ms;
    ms.WriteByte(42);
    auto snapshot = ms.ToArray();
    EXPECT_EQ(snapshot.size(), 1u);
    ms.WriteByte(43);
    // snapshot must be unaffected by writes that happen after it was taken.
    EXPECT_EQ(snapshot.size(), 1u);
    EXPECT_EQ(snapshot[0], 42u);
}

TEST(MemoryStreamTests, CanWriteIsTrue) {
    MemoryStream ms;
    EXPECT_TRUE(ms.getCanWriteProperty());
}

// ---------------------------------------------------------------------------
// MemoryStream — read-only (from buffer ctor)
// ---------------------------------------------------------------------------

TEST(MemoryStreamTests, ReadOnlyCtorFromBuffer) {
    uint8_t src[] = {1, 2, 3, 4, 5};
    MemoryStream ms(src, 5);
    EXPECT_EQ(ms.getLengthProperty(), 5);
    EXPECT_FALSE(ms.getCanWriteProperty());
}

TEST(MemoryStreamTests, ReadFromBuffer) {
    uint8_t src[] = {10, 20, 30};
    MemoryStream ms(src, 3);
    uint8_t dst[3] = {};
    int n = ms.Read(dst, 0, 3);
    EXPECT_EQ(n, 3);
    EXPECT_EQ(dst[0], 10u);
    EXPECT_EQ(dst[1], 20u);
    EXPECT_EQ(dst[2], 30u);
}

TEST(MemoryStreamTests, ReadReturnsZeroAtEnd) {
    uint8_t src[] = {1};
    MemoryStream ms(src, 1);
    uint8_t dst[4] = {};
    ms.Read(dst, 0, 1); // consume the one byte
    int n = ms.Read(dst, 0, 4);
    EXPECT_EQ(n, 0);
}

TEST(MemoryStreamTests, ReadLessThanRequested) {
    // only 2 bytes available, ask for 10
    uint8_t src[] = {5, 6};
    MemoryStream ms(src, 2);
    uint8_t dst[10] = {};
    int n = ms.Read(dst, 0, 10);
    EXPECT_EQ(n, 2);
    EXPECT_EQ(dst[0], 5u);
    EXPECT_EQ(dst[1], 6u);
}

// ---------------------------------------------------------------------------
// MemoryStream — write then read roundtrip
// ---------------------------------------------------------------------------

TEST(MemoryStreamTests, WriteReadRoundtrip) {
    MemoryStream writer;
    uint8_t payload[] = {0xDE, 0xAD, 0xBE, 0xEF};
    writer.Write(payload, 0, 4);

    // Construct a read-only stream from the written data
    const auto& buf = writer.ToArray();
    MemoryStream reader(buf.data(), static_cast<int>(buf.size()));
    uint8_t readback[4] = {};
    reader.Read(readback, 0, 4);

    EXPECT_EQ(readback[0], 0xDEu);
    EXPECT_EQ(readback[3], 0xEFu);
}

// ---------------------------------------------------------------------------
// MemoryStream — Position
// ---------------------------------------------------------------------------

TEST(MemoryStreamTests, PositionStartsAtZero) {
    MemoryStream ms;
    EXPECT_EQ(ms.getPositionProperty(), 0);
}

TEST(MemoryStreamTests, PositionAdvancesOnWrite) {
    MemoryStream ms;
    ms.WriteByte(1); ms.WriteByte(2);
    EXPECT_EQ(ms.getPositionProperty(), 2);
}

TEST(MemoryStreamTests, PositionAdvancesOnRead) {
    uint8_t src[] = {1, 2, 3};
    MemoryStream ms(src, 3);
    uint8_t dst[2] = {};
    ms.Read(dst, 0, 2);
    EXPECT_EQ(ms.getPositionProperty(), 2);
}

TEST(MemoryStreamTests, SetPositionSeeksForRewrite) {
    MemoryStream ms;
    uint8_t payload[] = {0xAA, 0xBB, 0xCC};
    ms.Write(payload, 0, 3);
    ms.setPositionProperty(1);
    ms.WriteByte(0xFF);
    EXPECT_EQ(ms.getLengthProperty(), 3);
    EXPECT_EQ(ms.ToArray()[1], 0xFFu);
}

TEST(MemoryStreamTests, SetPositionNegativeThrows) {
    MemoryStream ms;
    EXPECT_THROW(ms.setPositionProperty(-1), System::ArgumentOutOfRangeException);
}

TEST(MemoryStreamTests, CanSeekIsTrue) {
    MemoryStream ms;
    EXPECT_TRUE(ms.getCanSeekProperty());
}

TEST(MemoryStreamTests, SeekFromBeginMatchesSetPosition) {
    MemoryStream ms;
    uint8_t payload[] = {1, 2, 3, 4, 5};
    ms.Write(payload, 0, 5);
    auto newPos = ms.Seek(2, SeekOrigin::Begin);
    EXPECT_EQ(newPos, 2);
    EXPECT_EQ(ms.getPositionProperty(), 2);
}

TEST(MemoryStreamTests, SeekFromCurrentAdvancesRelatively) {
    MemoryStream ms;
    uint8_t payload[] = {1, 2, 3, 4, 5};
    ms.Write(payload, 0, 5);
    ms.setPositionProperty(1);
    auto newPos = ms.Seek(2, SeekOrigin::Current);
    EXPECT_EQ(newPos, 3);
}

TEST(MemoryStreamTests, SeekFromEndComputesFromLength) {
    MemoryStream ms;
    uint8_t payload[] = {1, 2, 3, 4, 5}; // length 5
    ms.Write(payload, 0, 5);
    auto newPos = ms.Seek(-2, SeekOrigin::End);
    EXPECT_EQ(newPos, 3);
}

TEST(MemoryStreamTests, SeekBeforeBeginning_ThrowsIOException) {
    // Regression for a 2026-07-13 exception-type mismatch fix: Stream::Seek() previously routed
    // through setPositionProperty() for validation, which throws ArgumentOutOfRangeException --
    // but real .NET's MemoryStream.Seek (SeekCore) throws IOException specifically for a
    // resulting position before the start of the stream, a different validation rule from the
    // Position setter's own (still-correct) ArgumentOutOfRangeException for a direct assignment.
    MemoryStream ms;
    uint8_t payload[] = {1, 2, 3, 4, 5};
    ms.Write(payload, 0, 5);
    ms.setPositionProperty(0);
    EXPECT_THROW(ms.Seek(-5, SeekOrigin::Current), System::IO::IOException);
    EXPECT_THROW(ms.Seek(-1, SeekOrigin::Begin), System::IO::IOException);
}

TEST(MemoryStreamTests, DirectNegativePositionAssignment_StillThrowsArgumentOutOfRangeException) {
    // The direct Position-setter path is unaffected by the Seek() fix above -- it's a genuinely
    // different real-.NET validation rule (verified against MemoryStream.cs's Position setter,
    // which calls ArgumentOutOfRangeException.ThrowIfNegative unconditionally).
    MemoryStream ms;
    EXPECT_THROW(ms.setPositionProperty(-1), System::ArgumentOutOfRangeException);
}

TEST(MemoryStreamTests, SetLengthTruncates) {
    MemoryStream ms;
    uint8_t payload[] = {1, 2, 3, 4, 5};
    ms.Write(payload, 0, 5);
    ms.SetLength(2);
    EXPECT_EQ(ms.getLengthProperty(), 2);
}

TEST(MemoryStreamTests, SetLengthExtendsWithZeros) {
    MemoryStream ms;
    ms.WriteByte(1);
    ms.SetLength(3);
    EXPECT_EQ(ms.getLengthProperty(), 3);
    EXPECT_EQ(ms.ToArray()[1], 0u);
    EXPECT_EQ(ms.ToArray()[2], 0u);
}

TEST(MemoryStreamTests, SetLengthNegativeThrows) {
    MemoryStream ms;
    EXPECT_THROW(ms.SetLength(-1), System::ArgumentOutOfRangeException);
}

TEST(MemoryStreamTests, SetLengthOnReadOnlyThrowsNotSupportedException) {
    uint8_t src[] = {1, 2, 3};
    MemoryStream ms(src, 3);
    EXPECT_THROW(ms.SetLength(1), System::NotSupportedException);
}

TEST(MemoryStreamTests, WriteOnReadOnlyThrowsNotSupportedException) {
    // Regression: Write() previously silently no-op'd instead of throwing when !CanWrite,
    // matching SetLength()'s existing correct behavior (above) rather than being the one
    // internally-inconsistent silent-drop path.
    uint8_t src[] = {1, 2, 3};
    MemoryStream ms(src, 3);
    ASSERT_FALSE(ms.getCanWriteProperty());
    uint8_t buf[] = {9, 9};
    EXPECT_THROW(ms.Write(buf, 0, 2), System::NotSupportedException);
}

TEST(MemoryStreamTests, WriteByteOnReadOnlyThrowsNotSupportedException) {
    uint8_t src[] = {1, 2, 3};
    MemoryStream ms(src, 3);
    EXPECT_THROW(ms.WriteByte(9), System::NotSupportedException);
}

// Regression test (ticket 1487): Position can legally be set arbitrarily far past the end
// (matching real .NET), so position_+count in Write() could genuinely signed-overflow --
// confirmed real UB via a standalone UBSan repro on the identical pattern in Span<T>::Slice
// (ticket 265) -- and worse, silently bypass the resize check, corrupting memory. Matches real
// .NET's own MemoryStream.Write, which explicitly throws IOException for this exact case.
TEST(MemoryStreamTests, Write_PositionPlusCountOverflows_ThrowsIOException) {
    MemoryStream ms;
    ms.setPositionProperty(2147483647);
    uint8_t buf[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    EXPECT_THROW(ms.Write(buf, 0, 10), System::IO::IOException);
}

TEST(MemoryStreamTests, ReadByteReturnsBytesThenMinusOne) {
    uint8_t src[] = {0x41, 0x42};
    MemoryStream ms(src, 2);
    EXPECT_EQ(ms.ReadByte(), 0x41);
    EXPECT_EQ(ms.ReadByte(), 0x42);
    EXPECT_EQ(ms.ReadByte(), -1);
}

// Regression tests for a wave-3 audit finding: Read() returned 0 (indistinguishable from
// "stream at EOF") for a null buffer or negative offset/count instead of throwing. Verified
// against MemoryStream.cs's Read()/ValidateBufferArguments, matching FileStream::Read's
// existing validation in this codebase.
TEST(MemoryStreamTests, Read_NullBuffer_ThrowsArgumentNullException) {
    uint8_t src[] = {1, 2, 3};
    MemoryStream ms(src, 3);
    EXPECT_THROW(ms.Read(nullptr, 0, 1), System::ArgumentNullException);
}

TEST(MemoryStreamTests, Read_NegativeOffset_ThrowsArgumentOutOfRangeException) {
    uint8_t src[] = {1, 2, 3};
    MemoryStream ms(src, 3);
    uint8_t buf[4];
    EXPECT_THROW(ms.Read(buf, -1, 1), System::ArgumentOutOfRangeException);
}

TEST(MemoryStreamTests, Read_NegativeCount_ThrowsArgumentOutOfRangeException) {
    uint8_t src[] = {1, 2, 3};
    MemoryStream ms(src, 3);
    uint8_t buf[4];
    EXPECT_THROW(ms.Read(buf, 0, -1), System::ArgumentOutOfRangeException);
}

// Regression test for a wave-3 audit finding: Close() cleared the underlying buffer,
// contradicting its own doc comment ("no-op for MemoryStream") and real .NET's
// MemoryStream.Dispose(bool), which explicitly leaves the buffer and position untouched
// ("Don't set buffer to null - allow TryGetBuffer, GetBuffer & ToArray to work"). Ticket 1724
// (post-stabilization-audit) later added disposed-state tracking: Length/Position (and
// Read/Write/Seek) now correctly throw ObjectDisposedException once closed, matching real
// .NET's _isOpen guard -- so this test now checks the buffer survives Close() via ToArray()
// only, which (like GetBuffer()) deliberately stays usable post-dispose in real .NET.
TEST(MemoryStreamTests, Close_DoesNotClearBufferOrPosition) {
    MemoryStream ms;
    uint8_t data[] = {1, 2, 3, 4};
    ms.Write(data, 0, 4);
    ASSERT_EQ(ms.getLengthProperty(), 4);
    ASSERT_EQ(ms.getPositionProperty(), 4);

    ms.Close();

    auto arr = ms.ToArray();
    ASSERT_EQ(arr.size(), 4u);
    EXPECT_EQ(arr[0], 1);
    EXPECT_EQ(arr[3], 4);
}

TEST(MemoryStreamTests, Close_ThenReadWriteSeekLengthPosition_ThrowsObjectDisposedException) {
    MemoryStream ms;
    uint8_t data[] = {1, 2, 3, 4};
    ms.Write(data, 0, 4);
    ms.Close();

    uint8_t buf[4];
    EXPECT_THROW(ms.Read(buf, 0, 4), System::ObjectDisposedException);
    EXPECT_THROW(ms.Write(data, 0, 4), System::ObjectDisposedException);
    EXPECT_THROW(ms.WriteByte(5), System::ObjectDisposedException);
    EXPECT_THROW(ms.getLengthProperty(), System::ObjectDisposedException);
    EXPECT_THROW(ms.getPositionProperty(), System::ObjectDisposedException);
    EXPECT_THROW(ms.setPositionProperty(0), System::ObjectDisposedException);
    EXPECT_THROW(ms.SetLength(10), System::ObjectDisposedException);
    EXPECT_FALSE(ms.getCanSeekProperty());
}

TEST(MemoryStreamTests, Close_ThenGetBufferAndToArray_StillWork) {
    MemoryStream ms;
    uint8_t data[] = {1, 2, 3, 4};
    ms.Write(data, 0, 4);
    ms.Close();

    // Matches real .NET: GetBuffer()/ToArray() deliberately remain usable after Dispose()/Close().
    const auto& buffer = ms.GetBuffer();
    EXPECT_EQ(buffer.size(), 4u);
    auto arr = ms.ToArray();
    EXPECT_EQ(arr.size(), 4u);
}

// ---------------------------------------------------------------------------
// Stream — default Position behavior for non-seekable streams
// ---------------------------------------------------------------------------

namespace {
    class NonSeekableTestStream final : public System::IO::Stream {
    public:
        System::IO::intcs Read(System::IO::bytecs*, System::IO::intcs, System::IO::intcs) override { return 0; }
        void Close() override {}
        [[nodiscard]] System::IO::intcs getLengthProperty() const override { return 0; }
    };
}

TEST(StreamTests, DefaultGetPositionThrowsNotSupported) {
    NonSeekableTestStream s;
    EXPECT_THROW(s.getPositionProperty(), System::NotSupportedException);
}

TEST(StreamTests, DefaultSetPositionThrowsNotSupported) {
    NonSeekableTestStream s;
    EXPECT_THROW(s.setPositionProperty(0), System::NotSupportedException);
}

TEST(StreamTests, DefaultCanSeekIsFalse) {
    NonSeekableTestStream s;
    EXPECT_FALSE(s.getCanSeekProperty());
}

TEST(StreamTests, DefaultSeekThrowsNotSupported) {
    NonSeekableTestStream s;
    EXPECT_THROW(s.Seek(0, System::IO::SeekOrigin::Begin), System::NotSupportedException);
}

TEST(StreamTests, DefaultSetLengthThrowsNotSupported) {
    NonSeekableTestStream s;
    EXPECT_THROW(s.SetLength(0), System::NotSupportedException);
}

TEST(StreamTests, DefaultReadByteReturnsMinusOneAtEnd) {
    NonSeekableTestStream s;
    EXPECT_EQ(s.ReadByte(), -1);
}

// ---------------------------------------------------------------------------
// StringReader
// ---------------------------------------------------------------------------

TEST(StringReaderTests, PeekReturnsFirstChar) {
    StringReader sr("hello");
    EXPECT_EQ(sr.Peek(), int('h'));
}

TEST(StringReaderTests, PeekDoesNotAdvance) {
    StringReader sr("ab");
    EXPECT_EQ(sr.Peek(), int('a'));
    EXPECT_EQ(sr.Peek(), int('a')); // still 'a'
}

TEST(StringReaderTests, ReadAdvances) {
    StringReader sr("ab");
    EXPECT_EQ(sr.Read(), int('a'));
    EXPECT_EQ(sr.Read(), int('b'));
}

TEST(StringReaderTests, ReadReturnsMinusOneAtEnd) {
    StringReader sr("x");
    sr.Read(); // consume 'x'
    EXPECT_EQ(sr.Read(),  -1);
    EXPECT_EQ(sr.Peek(),  -1);
}

TEST(StringReaderTests, ReadEmptyString) {
    StringReader sr("");
    EXPECT_EQ(sr.Read(),  -1);
    EXPECT_EQ(sr.Peek(),  -1);
}

TEST(StringReaderTests, ReadLine) {
    StringReader sr("line1\nline2\nline3");
    EXPECT_EQ(sr.ReadLine(), "line1");
    EXPECT_EQ(sr.ReadLine(), "line2");
    EXPECT_EQ(sr.ReadLine(), "line3");
}

TEST(StringReaderTests, ReadLineStripsCarriageReturn) {
    StringReader sr("line1\r\nline2");
    EXPECT_EQ(sr.ReadLine(), "line1");
    EXPECT_EQ(sr.ReadLine(), "line2");
}

// Regression test for a wave-3 audit finding: ReadLine() only stopped scanning at '\n', so a
// lone '\r' (classic Mac line ending, not followed by '\n') was treated as ordinary line
// content instead of a line terminator -- merging "line1\rline2\nline3" into two lines
// ("line1\rline2", "line3") instead of three. Verified against StringReader.cs's ReadLine(),
// which treats '\r' and '\n' as interchangeable terminators.
TEST(StringReaderTests, ReadLine_LoneCarriageReturn_TerminatesLine) {
    StringReader sr("line1\rline2\nline3");
    EXPECT_EQ(sr.ReadLine(), "line1");
    EXPECT_EQ(sr.ReadLine(), "line2");
    EXPECT_EQ(sr.ReadLine(), "line3");
}

TEST(StringReaderTests, ReadLine_TrailingLoneCarriageReturn_TerminatesFinalLine) {
    StringReader sr("line1\rline2");
    EXPECT_EQ(sr.ReadLine(), "line1");
    EXPECT_EQ(sr.ReadLine(), "line2");
}

TEST(StringReaderTests, ReadLineAtEndReturnsEmpty) {
    StringReader sr("only");
    sr.ReadLine(); // consume "only"
    EXPECT_EQ(sr.ReadLine(), "");
}

TEST(StringReaderTests, ReadToEnd) {
    StringReader sr("hello world");
    EXPECT_EQ(sr.ReadToEnd(), "hello world");
}

TEST(StringReaderTests, ReadToEndAfterPartialRead) {
    StringReader sr("abcdef");
    sr.Read(); sr.Read(); // consume 'a', 'b'
    EXPECT_EQ(sr.ReadToEnd(), "cdef");
}

TEST(StringReaderTests, ReadToEndAtEndIsEmpty) {
    StringReader sr("x");
    sr.ReadToEnd();
    EXPECT_EQ(sr.ReadToEnd(), "");
}

// ---------------------------------------------------------------------------
// StringWriter
// ---------------------------------------------------------------------------

TEST(StringWriterTests, DefaultCtorEmpty) {
    StringWriter sw;
    EXPECT_EQ(sw.ToString(), "");
}

TEST(StringWriterTests, WriteAppends) {
    StringWriter sw;
    sw.Write(std::string("hello"));
    EXPECT_EQ(sw.ToString(), "hello");
}

TEST(StringWriterTests, WriteMultipleTimes) {
    StringWriter sw;
    sw.Write(std::string("foo"));
    sw.Write(std::string("bar"));
    EXPECT_EQ(sw.ToString(), "foobar");
}

TEST(StringWriterTests, GetStringBuilderAliasToString) {
    StringWriter sw;
    sw.Write(std::string("test"));
    EXPECT_EQ(sw.GetStringBuilder(), sw.ToString());
}

TEST(StringWriterTests, ToStringIdempotent) {
    StringWriter sw;
    sw.Write(std::string("data"));
    EXPECT_EQ(sw.ToString(), sw.ToString());
}

// ---------------------------------------------------------------------------
// UnmanagedMemoryStream
// ---------------------------------------------------------------------------

TEST(UnmanagedMemoryStreamTests, ReadOnlyCtor_ReadsBytes) {
    uint8_t data[] = {1, 2, 3, 4, 5};
    UnmanagedMemoryStream ums(data, 5);
    EXPECT_EQ(ums.getLengthProperty(), 5);
    EXPECT_TRUE(ums.getCanReadProperty());
    EXPECT_FALSE(ums.getCanWriteProperty());
    uint8_t buf[5] = {};
    intcs n = ums.Read(buf, 0, 5);
    EXPECT_EQ(n, 5);
    EXPECT_EQ(buf[4], 5u);
}

// Regression tests for a wave-3 audit finding: Read()/Write()/SetLength() after Close()
// threw NotSupportedException instead of ObjectDisposedException. Verified against
// UnmanagedMemoryStream.cs's EnsureNotClosed()/EnsureReadable()/EnsureWriteable(), which check
// isOpen FIRST (ObjectDisposedException) before CanRead/CanWrite (NotSupportedException) --
// this port's getCanReadProperty()/getCanWriteProperty() folded "is closed" into the same
// boolean as "was never readable/writable", losing the distinction.
TEST(UnmanagedMemoryStreamTests, Read_AfterClose_ThrowsObjectDisposedException) {
    uint8_t data[] = {1, 2, 3};
    UnmanagedMemoryStream ums(data, 3);
    ums.Close();
    uint8_t buf[3] = {};
    EXPECT_THROW(ums.Read(buf, 0, 3), System::ObjectDisposedException);
}

TEST(UnmanagedMemoryStreamTests, Write_AfterClose_ThrowsObjectDisposedException) {
    uint8_t data[8] = {};
    UnmanagedMemoryStream ums(data, 0, 8, FileAccess::ReadWrite);
    ums.Close();
    uint8_t payload[] = {1};
    EXPECT_THROW(ums.Write(payload, 0, 1), System::ObjectDisposedException);
}

TEST(UnmanagedMemoryStreamTests, SetLength_AfterClose_ThrowsObjectDisposedException) {
    uint8_t data[8] = {};
    UnmanagedMemoryStream ums(data, 0, 8, FileAccess::ReadWrite);
    ums.Close();
    EXPECT_THROW(ums.SetLength(4), System::ObjectDisposedException);
}

TEST(UnmanagedMemoryStreamTests, ReadReturnsZeroAtEnd) {
    uint8_t data[] = {1, 2};
    UnmanagedMemoryStream ums(data, 2);
    uint8_t buf[2] = {};
    ums.Read(buf, 0, 2);
    EXPECT_EQ(ums.Read(buf, 0, 2), 0);
}

TEST(UnmanagedMemoryStreamTests, WriteWithinCapacity_UpdatesLength) {
    uint8_t data[8] = {};
    UnmanagedMemoryStream ums(data, 0, 8, FileAccess::ReadWrite);
    uint8_t payload[] = {9, 9, 9};
    ums.Write(payload, 0, 3);
    EXPECT_EQ(ums.getLengthProperty(), 3);
    EXPECT_EQ(data[2], 9u);
}

TEST(UnmanagedMemoryStreamTests, WriteBeyondCapacity_Throws) {
    uint8_t data[2] = {};
    UnmanagedMemoryStream ums(data, 0, 2, FileAccess::ReadWrite);
    uint8_t payload[] = {1, 2, 3};
    EXPECT_THROW(ums.Write(payload, 0, 3), System::NotSupportedException);
}

// Regression test (ticket 1487): Position can legally be set arbitrarily far past the end
// (matching real .NET), so position_+count in Write() could genuinely signed-overflow --
// confirmed real UB via a standalone UBSan repro on the identical pattern in Span<T>::Slice
// (ticket 265) -- and worse, silently bypass the capacity check, corrupting memory. Matches
// real .NET's own UnmanagedMemoryStream.WriteCore, which computes the sum in `long` to avoid
// exactly this and throws NotSupportedException once genuinely beyond capacity.
TEST(UnmanagedMemoryStreamTests, Write_PositionPlusCountOverflows_ThrowsInsteadOfBypassingCheck) {
    uint8_t data[2] = {};
    UnmanagedMemoryStream ums(data, 0, 2, FileAccess::ReadWrite);
    ums.setPositionProperty(2147483647);
    uint8_t payload[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    EXPECT_THROW(ums.Write(payload, 0, 10), System::NotSupportedException);
}

TEST(UnmanagedMemoryStreamTests, Write_NullBuffer_ThrowsArgumentNullException) {
    uint8_t data[2] = {};
    UnmanagedMemoryStream ums(data, 0, 2, FileAccess::ReadWrite);
    EXPECT_THROW(ums.Write(nullptr, 0, 1), System::ArgumentNullException);
}

TEST(UnmanagedMemoryStreamTests, Write_NegativeOffset_ThrowsArgumentOutOfRangeException) {
    uint8_t data[2] = {};
    UnmanagedMemoryStream ums(data, 0, 2, FileAccess::ReadWrite);
    uint8_t payload[] = {1};
    EXPECT_THROW(ums.Write(payload, -1, 1), System::ArgumentOutOfRangeException);
}

TEST(UnmanagedMemoryStreamTests, Read_NullBuffer_ThrowsArgumentNullException) {
    uint8_t data[] = {1, 2};
    UnmanagedMemoryStream ums(data, 2);
    EXPECT_THROW(ums.Read(nullptr, 0, 1), System::ArgumentNullException);
}

TEST(UnmanagedMemoryStreamTests, Read_NegativeCount_ThrowsArgumentOutOfRangeException) {
    uint8_t data[] = {1, 2};
    UnmanagedMemoryStream ums(data, 2);
    uint8_t out[2];
    EXPECT_THROW(ums.Read(out, 0, -1), System::ArgumentOutOfRangeException);
}

TEST(UnmanagedMemoryStreamTests, WriteWhenReadOnly_ThrowsNotSupportedException) {
    uint8_t data[] = {1, 2};
    UnmanagedMemoryStream ums(data, 2);
    uint8_t payload[] = {9};
    EXPECT_THROW(ums.Write(payload, 0, 1), System::NotSupportedException);
}

TEST(UnmanagedMemoryStreamTests, CanSeek_TrueWhileOpen) {
    uint8_t data[] = {1, 2};
    UnmanagedMemoryStream ums(data, 2);
    EXPECT_TRUE(ums.getCanSeekProperty());
}

TEST(UnmanagedMemoryStreamTests, SetPosition_SeeksForRead) {
    uint8_t data[] = {10, 20, 30};
    UnmanagedMemoryStream ums(data, 3);
    ums.setPositionProperty(1);
    uint8_t buf[2] = {};
    ums.Read(buf, 0, 2);
    EXPECT_EQ(buf[0], 20u);
}

TEST(UnmanagedMemoryStreamTests, SetLength_BeyondCapacity_Throws) {
    uint8_t data[4] = {};
    UnmanagedMemoryStream ums(data, 0, 4, FileAccess::ReadWrite);
    EXPECT_THROW(ums.SetLength(5), System::IO::IOException);
}

TEST(UnmanagedMemoryStreamTests, NullPointer_ThrowsArgumentNullException) {
    EXPECT_THROW(UnmanagedMemoryStream(nullptr, 0), System::ArgumentNullException);
}

TEST(UnmanagedMemoryStreamTests, LengthGreaterThanCapacity_Throws) {
    uint8_t data[4] = {};
    EXPECT_THROW(UnmanagedMemoryStream(data, 4, 2, FileAccess::Read), System::ArgumentOutOfRangeException);
}

// ---------------------------------------------------------------------------
// UnmanagedMemoryAccessor
// ---------------------------------------------------------------------------

TEST(UnmanagedMemoryAccessorTests, ReadWriteByte_Roundtrip) {
    uint8_t data[4] = {};
    UnmanagedMemoryAccessor acc(data, 4, FileAccess::ReadWrite);
    acc.Write(0, static_cast<uint8_t>(0xAB));
    EXPECT_EQ(acc.ReadByte(0), 0xABu);
}

TEST(UnmanagedMemoryAccessorTests, ReadWriteInt32_Roundtrip) {
    uint8_t data[4] = {};
    UnmanagedMemoryAccessor acc(data, 4, FileAccess::ReadWrite);
    acc.Write(0, static_cast<int32_t>(-12345));
    EXPECT_EQ(acc.ReadInt32(0), -12345);
}

TEST(UnmanagedMemoryAccessorTests, ReadWriteDouble_Roundtrip) {
    uint8_t data[8] = {};
    UnmanagedMemoryAccessor acc(data, 8, FileAccess::ReadWrite);
    acc.Write(0, 3.14159);
    EXPECT_DOUBLE_EQ(acc.ReadDouble(0), 3.14159);
}

TEST(UnmanagedMemoryAccessorTests, ReadWriteBoolean_Roundtrip) {
    uint8_t data[1] = {};
    UnmanagedMemoryAccessor acc(data, 1, FileAccess::ReadWrite);
    acc.Write(0, true);
    EXPECT_TRUE(acc.ReadBoolean(0));
}

TEST(UnmanagedMemoryAccessorTests, CapacityProperty) {
    uint8_t data[16] = {};
    UnmanagedMemoryAccessor acc(data, 16);
    EXPECT_EQ(acc.getCapacityProperty(), 16);
}

TEST(UnmanagedMemoryAccessorTests, TwoArgCtor_DefaultsToReadOnly) {
    uint8_t data[4] = {};
    UnmanagedMemoryAccessor acc(data, 4);
    EXPECT_TRUE(acc.getCanReadProperty());
    EXPECT_FALSE(acc.getCanWriteProperty());
    EXPECT_THROW(acc.Write(0, static_cast<uint8_t>(1)), System::NotSupportedException);
}

TEST(UnmanagedMemoryAccessorTests, ReadOnly_WriteThrowsNotSupportedException) {
    uint8_t data[4] = {};
    UnmanagedMemoryAccessor acc(data, 4, FileAccess::Read);
    EXPECT_TRUE(acc.getCanReadProperty());
    EXPECT_FALSE(acc.getCanWriteProperty());
    EXPECT_THROW(acc.Write(0, static_cast<uint8_t>(1)), System::NotSupportedException);
}

TEST(UnmanagedMemoryAccessorTests, PositionBeyondCapacity_ThrowsArgumentOutOfRangeException) {
    uint8_t data[4] = {};
    UnmanagedMemoryAccessor acc(data, 4);
    EXPECT_THROW(acc.ReadByte(4), System::ArgumentOutOfRangeException); // position >= capacity
}

TEST(UnmanagedMemoryAccessorTests, PositionWithinCapacityButNotEnoughBytes_ThrowsArgumentException) {
    uint8_t data[4] = {};
    UnmanagedMemoryAccessor acc(data, 4);
    // needs 4 bytes at position 1, but position(1) < capacity(4), so this is
    // "not enough remaining bytes" (ArgumentException), not "out of range" (ArgumentOutOfRangeException).
    EXPECT_THROW(acc.ReadInt32(1), System::ArgumentException);
}

TEST(UnmanagedMemoryAccessorTests, AfterDispose_ThrowsObjectDisposedException) {
    uint8_t data[4] = {};
    UnmanagedMemoryAccessor acc(data, 4);
    acc.Dispose();
    EXPECT_THROW(acc.ReadByte(0), System::ObjectDisposedException);
}

TEST(UnmanagedMemoryAccessorTests, NullBuffer_ThrowsArgumentNullException) {
    EXPECT_THROW(UnmanagedMemoryAccessor(nullptr, 4), System::ArgumentNullException);
}

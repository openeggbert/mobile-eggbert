// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Functional tests for System::IO: Path, File, FileInfo, Directory, DirectoryInfo,
// BinaryReader/Writer, StreamReader/Writer, BufferedStream, FileStream, IsolatedStorageFile.
#include <gtest/gtest.h>
#include <cstdint>
#include <limits>
#include <string>
#include <vector>
#include "System/ArgumentException.hpp"
#include "System/ArgumentNullException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/ObjectDisposedException.hpp"
#include "System/NotSupportedException.hpp"
#include "System/IO/DirectoryNotFoundException.hpp"
#include "System/IO/EndOfStreamException.hpp"
#include "System/IO/IOException.hpp"
#include "System/IO/Path.hpp"
#include "System/IO/File.hpp"
#include "System/IO/FileInfo.hpp"
#include "System/IO/FileSystemInfo.hpp"
#include "System/DateTime.hpp"
#include "System/TimeZoneInfo.hpp"
#include "System/IO/Directory.hpp"
#include "System/IO/DirectoryInfo.hpp"
#include "System/IO/BinaryReader.hpp"
#include "System/IO/BinaryWriter.hpp"
#include "System/IO/StreamReader.hpp"
#include "System/IO/StreamWriter.hpp"
#include "System/IO/BufferedStream.hpp"
#include "System/IO/FileStream.hpp"
#include "System/IO/MemoryStream.hpp"
#include "System/IO/FileMode.hpp"
#include "System/IO/FileAccess.hpp"
#include "System/IO/SeekOrigin.hpp"
#include "System/IO/RandomAccess.hpp"
#include "System/IO/FileNotFoundException.hpp"
#include "System/IO/IsolatedStorage/IsolatedStorageException.hpp"
#include "System/IO/IsolatedStorage/IsolatedStorageFile.hpp"
#include "System/IO/IsolatedStorage/IsolatedStorageFileStream.hpp"

using System::IO::Path;
using System::IO::File;
using System::IO::FileInfo;
using System::IO::FileSystemInfo;
using System::IO::Directory;
using System::IO::DirectoryInfo;
using System::IO::BinaryReader;
using System::IO::BinaryWriter;
using System::IO::StreamReader;
using System::IO::StreamWriter;
using System::IO::BufferedStream;
using System::IO::FileStream;
using System::IO::MemoryStream;
using System::IO::FileMode;
using System::IO::FileAccess;
using System::IO::SeekOrigin;
using System::IO::intcs;
using System::IO::IsolatedStorage::IsolatedStorageFile;

static std::string tf(const char* name) {
    return std::string("/tmp/sharp_rt_io_") + name;
}

// ===========================================================================
// Path
// ===========================================================================

TEST(PathTests, DirectorySeparatorChar_IsForwardSlash) {
    EXPECT_EQ(Path::DirectorySeparatorChar, '/');
}

TEST(PathTests, Combine_TwoParts) {
    EXPECT_EQ(Path::Combine("/foo", "bar"), "/foo/bar");
}

TEST(PathTests, Combine_TwoParts_TrailingSlash) {
    EXPECT_EQ(Path::Combine("/foo/", "bar"), "/foo/bar");
}

TEST(PathTests, Combine_ThreeParts) {
    std::string r = Path::Combine("/a", "b", "c");
    EXPECT_NE(r.find("a"), std::string::npos);
    EXPECT_NE(r.find("b"), std::string::npos);
    EXPECT_NE(r.find("c"), std::string::npos);
}

TEST(PathTests, GetFileName_WithDirectory) {
    EXPECT_EQ(Path::GetFileName("/foo/bar.txt"), "bar.txt");
}

TEST(PathTests, GetFileName_NoDirectory) {
    EXPECT_EQ(Path::GetFileName("file.txt"), "file.txt");
}

TEST(PathTests, GetFileNameWithoutExtension) {
    EXPECT_EQ(Path::GetFileNameWithoutExtension("/foo/bar.txt"), "bar");
}

TEST(PathTests, GetExtension_WithDot) {
    EXPECT_EQ(Path::GetExtension("file.txt"), ".txt");
}

TEST(PathTests, GetExtension_NoDot) {
    EXPECT_EQ(Path::GetExtension("readme"), "");
}

TEST(PathTests, GetDirectoryName_AbsolutePath) {
    EXPECT_EQ(Path::GetDirectoryName("/foo/bar.txt"), "/foo");
}

TEST(PathTests, HasExtension_True) {
    EXPECT_TRUE(Path::HasExtension("file.txt"));
}

TEST(PathTests, HasExtension_False) {
    EXPECT_FALSE(Path::HasExtension("readme"));
}

TEST(PathTests, IsPathRooted_AbsolutePath) {
    EXPECT_TRUE(Path::IsPathRooted("/foo/bar"));
}

TEST(PathTests, IsPathRooted_RelativePath) {
    EXPECT_FALSE(Path::IsPathRooted("relative/path"));
}

TEST(PathTests, ChangeExtension_ReplacesExt) {
    std::string r = Path::ChangeExtension("file.txt", ".md");
    EXPECT_NE(r.find(".md"), std::string::npos);
    EXPECT_EQ(r.find(".txt"), std::string::npos);
}

// ===========================================================================
// File
// ===========================================================================

TEST(FileTests, Exists_ExistingFile_True) {
    std::string p = tf("exists_true.txt");
    File::WriteAllText(p, "x");
    EXPECT_TRUE(File::Exists(p));
    File::Delete(p);
}

TEST(FileTests, Exists_NonExistentFile_False) {
    EXPECT_FALSE(File::Exists(tf("nonexistent_xyz_abc.txt")));
}

TEST(FileTests, WriteAllText_ReadAllText_Roundtrip) {
    std::string p = tf("rw_text.txt");
    File::WriteAllText(p, "hello world");
    EXPECT_EQ(File::ReadAllText(p), "hello world");
    File::Delete(p);
}

TEST(FileTests, WriteAllText_Overwrites) {
    std::string p = tf("overwrite.txt");
    File::WriteAllText(p, "first");
    File::WriteAllText(p, "second");
    EXPECT_EQ(File::ReadAllText(p), "second");
    File::Delete(p);
}

TEST(FileTests, WriteAllLines_ReadAllLines_Roundtrip) {
    std::string p = tf("alllines.txt");
    std::vector<std::string> lines = {"line one", "line two", "line three"};
    File::WriteAllLines(p, lines);
    auto read = File::ReadAllLines(p);
    ASSERT_EQ(read.size(), 3u);
    EXPECT_EQ(read[0], "line one");
    EXPECT_EQ(read[2], "line three");
    File::Delete(p);
}

TEST(FileTests, WriteAllBytes_ReadAllBytes_Roundtrip) {
    std::string p = tf("allbytes.bin");
    std::vector<uint8_t> bytes = {10, 20, 30, 40, 50};
    File::WriteAllBytes(p, bytes);
    auto read = File::ReadAllBytes(p);
    ASSERT_EQ(read.size(), 5u);
    EXPECT_EQ(read[0], 10u);
    EXPECT_EQ(read[4], 50u);
    File::Delete(p);
}

TEST(FileTests, AppendAllText_Appends) {
    std::string p = tf("append.txt");
    File::WriteAllText(p, "hello ");
    File::AppendAllText(p, "world");
    EXPECT_EQ(File::ReadAllText(p), "hello world");
    File::Delete(p);
}

TEST(FileTests, Delete_RemovesFile) {
    std::string p = tf("delete_me.txt");
    File::WriteAllText(p, "temp");
    File::Delete(p);
    EXPECT_FALSE(File::Exists(p));
}

// Regression test for a wave-3 audit finding: File::Delete used std::filesystem::remove(),
// which is documented to also remove an empty directory (calling the equivalent of rmdir).
// Real .NET's File.Delete calls unlink() directly, which can never remove a directory --
// verified against FileSystem.Unix.cs's DeleteFile. Previously this port would silently
// delete an empty directory passed to File::Delete instead of throwing.
TEST(FileTests, Delete_OnDirectory_ThrowsIOException) {
    std::string dir = tf("delete_me_dir");
    Directory::CreateDirectory(dir);
    EXPECT_THROW(File::Delete(dir), System::IO::IOException);
    EXPECT_TRUE(Directory::Exists(dir));
    Directory::Delete(dir);
}

TEST(FileTests, Copy_CreatesDestination) {
    std::string src = tf("copy_src.txt");
    std::string dst = tf("copy_dst.txt");
    File::WriteAllText(src, "copy content");
    File::Copy(src, dst, true);
    EXPECT_TRUE(File::Exists(dst));
    EXPECT_EQ(File::ReadAllText(dst), "copy content");
    File::Delete(src);
    File::Delete(dst);
}

TEST(FileTests, Move_RenamesFile) {
    std::string src = tf("move_src.txt");
    std::string dst = tf("move_dst.txt");
    File::WriteAllText(src, "move content");
    File::Move(src, dst);
    EXPECT_FALSE(File::Exists(src));
    EXPECT_TRUE(File::Exists(dst));
    EXPECT_EQ(File::ReadAllText(dst), "move content");
    File::Delete(dst);
}

TEST(FileTests, ReadAllText_ThrowsForNonExistentFile) {
    EXPECT_THROW((void)File::ReadAllText(tf("missing_xyz_abc.txt")),
                 System::IO::FileNotFoundException);
}

TEST(FileTests, Exists_ExistingDirectory_False) {
    // File.Exists must return false for a directory, matching .NET.
    EXPECT_FALSE(File::Exists("/tmp"));
}

TEST(FileTests, Delete_NonExistentFile_DoesNotThrow) {
    // .NET: deleting a nonexistent file is not an error.
    EXPECT_NO_THROW(File::Delete(tf("no_such_file_for_delete_xyz.txt")));
}

TEST(FileTests, Copy_NonExistentSource_ThrowsFileNotFoundException) {
    EXPECT_THROW(File::Copy(tf("no_such_src_xyz.txt"), tf("copy_dst_xyz.txt")),
                 System::IO::FileNotFoundException);
}

TEST(FileTests, Move_NonExistentSource_ThrowsFileNotFoundException) {
    EXPECT_THROW(File::Move(tf("no_such_move_src_xyz.txt"), tf("move_dst_xyz.txt")),
                 System::IO::FileNotFoundException);
}

TEST(FileTests, Move_ExistingDestination_ThrowsIOException_DoesNotOverwrite) {
    // Regression: Move previously used std::filesystem::rename() unconditionally, which
    // silently replaces an existing destination file on POSIX instead of matching real .NET's
    // non-overwrite Move contract.
    std::string src = tf("move_noclobber_src.txt");
    std::string dst = tf("move_noclobber_dst.txt");
    File::WriteAllText(src, "source content");
    File::WriteAllText(dst, "original destination content");
    EXPECT_THROW(File::Move(src, dst), System::IO::IOException);
    EXPECT_TRUE(File::Exists(src));
    EXPECT_EQ(File::ReadAllText(dst), "original destination content");
    File::Delete(src);
    File::Delete(dst);
}

TEST(FileTests, Copy_EmptySourcePath_ThrowsArgumentException) {
    EXPECT_THROW(File::Copy("", tf("dst_xyz.txt")), System::ArgumentException);
}

// ===========================================================================
// FileInfo
// ===========================================================================

TEST(FileInfoTests, Constructor_NoThrow) {
    EXPECT_NO_THROW(FileInfo{tf("fi_nofile.txt")});
}

TEST(FileInfoTests, getNameProperty_ReturnsFilename) {
    FileInfo fi("/tmp/sharp_rt_io_name_test.txt");
    EXPECT_EQ(fi.getNameProperty(), "sharp_rt_io_name_test.txt");
}

TEST(FileInfoTests, getExtensionProperty_ReturnsExt) {
    FileInfo fi("/foo/bar.txt");
    EXPECT_EQ(fi.getExtensionProperty(), ".txt");
}

TEST(FileInfoTests, getExistsProperty_ExistingFile_True) {
    std::string p = tf("fi_exists.txt");
    File::WriteAllText(p, "hi");
    FileInfo fi(p);
    EXPECT_TRUE(fi.getExistsProperty());
    File::Delete(p);
}

TEST(FileInfoTests, getExistsProperty_NonExistent_False) {
    FileInfo fi(tf("fi_no_such_xyz.txt"));
    EXPECT_FALSE(fi.getExistsProperty());
}

TEST(FileInfoTests, getLengthProperty_MatchesContent) {
    std::string p = tf("fi_length.txt");
    File::WriteAllText(p, "hello");
    FileInfo fi(p);
    EXPECT_EQ(fi.getLengthProperty(), 5LL);
    File::Delete(p);
}

TEST(FileInfoTests, getLengthProperty_NonExistentFile_ReturnsZero) {
    // Matches .NET's real Unix FileInfo.Length (FileStatus.Unix.cs's GetLength: "return
    // EntryExists ? _fileCache.Size : 0") — a missing file returns 0, it does not throw.
    FileInfo fi(tf("fi_length_nonexistent_xyz123.txt"));
    EXPECT_EQ(fi.getLengthProperty(), 0LL);
}

TEST(FileInfoTests, getLengthProperty_Directory_ThrowsFileNotFoundException) {
    std::string dir = tf("fi_length_dir");
    Directory::CreateDirectory(dir);
    FileInfo fi(dir);
    EXPECT_THROW(fi.getLengthProperty(), System::IO::FileNotFoundException);
    Directory::Delete(dir);
}

TEST(FileInfoTests, getFullNameProperty_IsAbsolute) {
    FileInfo fi(tf("fi_fullname.txt"));
    std::string full = fi.getFullNameProperty();
    EXPECT_EQ(full[0], '/');
}

TEST(FileInfoTests, getIsReadOnlyProperty_NonExistent_ReturnsFalse) {
    FileInfo fi(tf("fi_no_such_readonly_xyz.txt"));
    EXPECT_FALSE(fi.getIsReadOnlyProperty());
}

TEST(FileInfoTests, Delete_NonExistentFile_DoesNotThrow) {
    FileInfo fi(tf("fi_no_such_delete_xyz.txt"));
    EXPECT_NO_THROW(fi.Delete());
}

TEST(FileInfoTests, CopyTo_NonExistentSource_ThrowsFileNotFoundException) {
    FileInfo fi(tf("fi_no_such_copy_src_xyz.txt"));
    EXPECT_THROW(fi.CopyTo(tf("fi_copy_dst_xyz.txt")), System::IO::FileNotFoundException);
}

TEST(FileInfoTests, CopyTo_EmptyDest_ThrowsArgumentException) {
    std::string p = tf("fi_copy_src_for_empty_dest.txt");
    File::WriteAllText(p, "x");
    FileInfo fi(p);
    EXPECT_THROW(fi.CopyTo(""), System::ArgumentException);
    File::Delete(p);
}

TEST(FileInfoTests, MoveTo_NonExistentSource_ThrowsFileNotFoundException) {
    FileInfo fi(tf("fi_no_such_move_src_xyz.txt"));
    EXPECT_THROW(fi.MoveTo(tf("fi_move_dst_xyz.txt")), System::IO::FileNotFoundException);
}

TEST(FileInfoTests, MoveTo_ExistingDestination_ThrowsIOException_DoesNotOverwrite) {
    std::string src = tf("fi_move_noclobber_src.txt");
    std::string dst = tf("fi_move_noclobber_dst.txt");
    File::WriteAllText(src, "source content");
    File::WriteAllText(dst, "original destination content");
    FileInfo fi(src);
    EXPECT_THROW(fi.MoveTo(dst), System::IO::IOException);
    EXPECT_TRUE(File::Exists(src));
    EXPECT_EQ(File::ReadAllText(dst), "original destination content");
    File::Delete(src);
    File::Delete(dst);
}

TEST(FileInfoTests, CopyTo_Succeeds_DestinationExists) {
    std::string src = tf("fi_copyto_src.txt");
    std::string dst = tf("fi_copyto_dst.txt");
    File::WriteAllText(src, "copy me");
    FileInfo fi(src);
    fi.CopyTo(dst, true);
    EXPECT_TRUE(File::Exists(dst));
    File::Delete(src);
    File::Delete(dst);
}

// ===========================================================================
// FileSystemInfo (base of FileInfo/DirectoryInfo)
// ===========================================================================

TEST(FileSystemInfoTests, FileInfo_IsPolymorphicallyAFileSystemInfo) {
    std::string path = tf("fsi_poly_file.txt");
    File::WriteAllText(path, "x");
    FileInfo fi(path);
    FileSystemInfo* base = &fi;
    EXPECT_TRUE(base->getExistsProperty());
    EXPECT_EQ(base->getNameProperty(), "sharp_rt_io_fsi_poly_file.txt");
    base->Delete();
    EXPECT_FALSE(File::Exists(path));
}

TEST(FileSystemInfoTests, DirectoryInfo_IsPolymorphicallyAFileSystemInfo) {
    std::string path = tf("fsi_poly_dir");
    DirectoryInfo di(path);
    di.Create();
    FileSystemInfo* base = &di;
    EXPECT_TRUE(base->getExistsProperty());
    base->Delete();
    EXPECT_FALSE(di.getExistsProperty());
}

TEST(FileSystemInfoTests, ToString_ReturnsOriginalPath_NotFullName) {
    FileInfo fi("relative_name.txt");
    EXPECT_EQ(fi.ToString(), "relative_name.txt");
    EXPECT_NE(fi.ToString(), fi.getFullNameProperty());
}

TEST(FileSystemInfoTests, LastWriteTime_RoundTrips) {
    std::string path = tf("fsi_lastwrite.txt");
    File::WriteAllText(path, "content");
    FileInfo fi(path);
    System::DateTime original = fi.getLastWriteTimeUtcProperty();
    System::DateTime target(original.getTicksProperty() - System::DateTime::TicksPerDay);
    fi.setLastWriteTimeUtcProperty(target);
    EXPECT_EQ(fi.getLastWriteTimeUtcProperty().getTicksProperty() / System::DateTime::TicksPerSecond,
              target.getTicksProperty() / System::DateTime::TicksPerSecond);
    File::Delete(path);
}

// Regression test for a wave-3 audit finding: CreationTime/LastAccessTime/LastWriteTime (the
// "local" properties) returned the UTC value verbatim with no timezone conversion at all.
// Verified against FileSystemInfo.cs, where each is `Xxx => XxxUtc.ToLocalTime()`. This
// asserts the applied offset exactly matches TimeZoneInfo::Local()'s base UTC offset (rather
// than asserting local != utc, which would be a no-op in a UTC-configured CI environment) --
// it fails under the old bug whenever the local zone's offset is non-zero, and also catches a
// wrong-sign conversion.
TEST(FileSystemInfoTests, CreationTime_AppliesLocalUtcOffset) {
    std::string path = tf("fsi_localtime.txt");
    File::WriteAllText(path, "content");
    FileInfo fi(path);
    System::DateTime utc = fi.getCreationTimeUtcProperty();
    System::DateTime local = fi.getCreationTimeProperty();
    SharpRuntime::longcs expectedOffsetTicks = System::TimeZoneInfo::Local().getBaseUtcOffsetProperty().getTicksProperty();
    EXPECT_EQ(local.getTicksProperty() - utc.getTicksProperty(), expectedOffsetTicks);
    File::Delete(path);
}

TEST(FileSystemInfoTests, SetLastWriteTime_ConvertsFromLocalToUtc) {
    std::string path = tf("fsi_setlocaltime.txt");
    File::WriteAllText(path, "content");
    FileInfo fi(path);
    System::DateTime localTarget(System::DateTime(2020, 6, 15, 12, 0, 0));
    fi.setLastWriteTimeProperty(localTarget);
    System::DateTime expectedUtc = System::TimeZoneInfo::ConvertTimeToUtc(localTarget, System::TimeZoneInfo::Local());
    EXPECT_EQ(fi.getLastWriteTimeUtcProperty().getTicksProperty() / System::DateTime::TicksPerSecond,
              expectedUtc.getTicksProperty() / System::DateTime::TicksPerSecond);
    File::Delete(path);
}

TEST(FileSystemInfoTests, CreationAndAccessTime_DoNotThrow_ForExistingFile) {
    std::string path = tf("fsi_times.txt");
    File::WriteAllText(path, "content");
    FileInfo fi(path);
    EXPECT_NO_THROW({
        auto creation = fi.getCreationTimeUtcProperty();
        auto access = fi.getLastAccessTimeUtcProperty();
        (void)creation;
        (void)access;
    });
    File::Delete(path);
}

// ===========================================================================
// Directory + DirectoryInfo
// ===========================================================================

TEST(DirectoryTests, Exists_ExistingDir_True) {
    EXPECT_TRUE(Directory::Exists("/tmp"));
}

TEST(DirectoryTests, Exists_NonExistentDir_False) {
    EXPECT_FALSE(Directory::Exists(tf("no_such_dir_xyz_abc")));
}

TEST(DirectoryTests, CreateDirectory_Delete_Roundtrip) {
    std::string d = tf("mkdir_test");
    Directory::CreateDirectory(d);
    EXPECT_TRUE(Directory::Exists(d));
    Directory::Delete(d);
    EXPECT_FALSE(Directory::Exists(d));
}

TEST(DirectoryTests, GetCurrentDirectory_NotEmpty) {
    EXPECT_FALSE(Directory::GetCurrentDirectory().empty());
}

TEST(DirectoryTests, Delete_NonExistentDir_ThrowsDirectoryNotFoundException) {
    EXPECT_THROW(Directory::Delete(tf("no_such_dir_for_delete_xyz")),
                 System::IO::DirectoryNotFoundException);
}

TEST(DirectoryTests, GetFiles_NonExistentDir_ThrowsDirectoryNotFoundException) {
    EXPECT_THROW((void)Directory::GetFiles(tf("no_such_dir_for_getfiles_xyz")),
                 System::IO::DirectoryNotFoundException);
}

// Regression test for a wave-3 audit finding: GetFiles(path, "*.*") converted the pattern
// literally to a regex requiring a literal '.' in the filename, silently excluding every
// extensionless file. Verified against FileSystemName.cs's TranslateWin32Expression, which
// special-cases "*.*" (legacy DOS 8.3 compatibility) to match every file, with or without an
// extension.
TEST(DirectoryTests, GetFiles_StarDotStarPattern_IncludesExtensionlessFiles) {
    std::string dir = tf("getfiles_stardotstar_dir");
    Directory::CreateDirectory(dir);
    File::WriteAllText(dir + "/README", "no extension");
    File::WriteAllText(dir + "/notes.txt", "has extension");

    auto files = Directory::GetFiles(dir, "*.*");
    EXPECT_EQ(files.size(), 2u);

    Directory::Delete(dir, true);
}

TEST(DirectoryTests, GetDirectories_NonExistentDir_ThrowsDirectoryNotFoundException) {
    EXPECT_THROW((void)Directory::GetDirectories(tf("no_such_dir_for_getdirs_xyz")),
                 System::IO::DirectoryNotFoundException);
}

TEST(DirectoryTests, CreateDirectory_EmptyPath_ThrowsArgumentException) {
    EXPECT_THROW(Directory::CreateDirectory(""), System::ArgumentException);
}

TEST(DirectoryTests, Move_NonExistentSource_ThrowsDirectoryNotFoundException) {
    EXPECT_THROW(Directory::Move(tf("no_such_src_xyz"), tf("move_dst_xyz")),
                 System::IO::DirectoryNotFoundException);
}

TEST(DirectoryTests, Move_ExistingDestination_ThrowsIOException_DoesNotOverwrite) {
    std::string src = tf("dir_move_noclobber_src");
    std::string dst = tf("dir_move_noclobber_dst");
    Directory::CreateDirectory(src);
    Directory::CreateDirectory(dst);
    EXPECT_THROW(Directory::Move(src, dst), System::IO::IOException);
    EXPECT_TRUE(Directory::Exists(src));
    EXPECT_TRUE(Directory::Exists(dst));
    Directory::Delete(src);
    Directory::Delete(dst);
}

TEST(DirectoryTests, Exists_ExistingFileNotDirectory_False) {
    std::string p = tf("exists_file_not_dir.txt");
    File::WriteAllText(p, "x");
    EXPECT_FALSE(Directory::Exists(p));
    File::Delete(p);
}

TEST(DirectoryInfoTests, Constructor_NoThrow) {
    EXPECT_NO_THROW(DirectoryInfo{"/tmp"});
}

TEST(DirectoryInfoTests, getExistsProperty_Tmp_True) {
    DirectoryInfo di("/tmp");
    EXPECT_TRUE(di.getExistsProperty());
}

TEST(DirectoryInfoTests, getExistsProperty_NonExistent_False) {
    DirectoryInfo di(tf("di_no_such_xyz"));
    EXPECT_FALSE(di.getExistsProperty());
}

TEST(DirectoryInfoTests, Create_Delete_Roundtrip) {
    std::string d = tf("di_mkdir");
    DirectoryInfo di(d);
    di.Create();
    EXPECT_TRUE(di.getExistsProperty());
    di.Delete();
    EXPECT_FALSE(di.getExistsProperty());
}

TEST(DirectoryInfoTests, Delete_NonExistent_ThrowsDirectoryNotFoundException) {
    DirectoryInfo di(tf("di_no_such_delete_xyz"));
    EXPECT_THROW(di.Delete(), System::IO::DirectoryNotFoundException);
}

TEST(DirectoryInfoTests, MoveTo_NonExistent_ThrowsDirectoryNotFoundException) {
    DirectoryInfo di(tf("di_no_such_move_xyz"));
    EXPECT_THROW(di.MoveTo(tf("di_move_dst_xyz")), System::IO::DirectoryNotFoundException);
}

TEST(DirectoryInfoTests, MoveTo_ExistingDestination_ThrowsIOException_DoesNotOverwrite) {
    std::string src = tf("di_move_noclobber_src");
    std::string dst = tf("di_move_noclobber_dst");
    Directory::CreateDirectory(src);
    Directory::CreateDirectory(dst);
    DirectoryInfo di(src);
    EXPECT_THROW(di.MoveTo(dst), System::IO::IOException);
    EXPECT_TRUE(Directory::Exists(src));
    EXPECT_TRUE(Directory::Exists(dst));
    Directory::Delete(src);
    Directory::Delete(dst);
}

TEST(DirectoryInfoTests, GetFiles_NonExistent_ThrowsDirectoryNotFoundException) {
    DirectoryInfo di(tf("di_no_such_getfiles_xyz"));
    EXPECT_THROW((void)di.GetFiles(), System::IO::DirectoryNotFoundException);
}

// ===========================================================================
// BinaryWriter + BinaryReader
// ===========================================================================

namespace {
    // Test double tracking Flush()/Close() calls, since MemoryStream's Flush() is an
    // unobservable no-op -- used to verify BinaryWriter's leaveOpen handling actually calls
    // Flush() on the underlying stream rather than silently doing nothing.
    class FlushTrackingStream : public System::IO::Stream {
    public:
        bool flushCalled = false;
        bool closeCalled = false;
        intcs Read(SharpRuntime::bytecs*, intcs, intcs) override { return 0; }
        void Write(const SharpRuntime::bytecs*, intcs, intcs) override {}
        void Close() override { closeCalled = true; }
        void Flush() override { flushCalled = true; }
        [[nodiscard]] intcs getLengthProperty() const override { return 0; }
        [[nodiscard]] bool getCanWriteProperty() const override { return true; }
    };
}

// Regression tests for a wave-3 audit finding: BinaryWriter::Close()/~BinaryWriter() with
// leaveOpen=true did nothing at all to the underlying stream, instead of flushing it. Verified
// against BinaryWriter.cs's Dispose(bool): "if (_leaveOpen) OutStream.Flush(); else
// OutStream.Close();" -- leaveOpen=true means "flush what's pending, but don't take ownership
// of closing it," not "do nothing."
TEST(BinaryReaderWriterTests, BinaryWriter_Close_LeaveOpenTrue_FlushesUnderlyingStream) {
    FlushTrackingStream fs;
    BinaryWriter bw(&fs, true);
    bw.Close();
    EXPECT_TRUE(fs.flushCalled);
    EXPECT_FALSE(fs.closeCalled);
}

TEST(BinaryReaderWriterTests, BinaryWriter_Destructor_LeaveOpenTrue_FlushesUnderlyingStream) {
    FlushTrackingStream fs;
    {
        BinaryWriter bw(&fs, true);
        (void)bw;
    }
    EXPECT_TRUE(fs.flushCalled);
    EXPECT_FALSE(fs.closeCalled);
}

TEST(BinaryReaderWriterTests, BinaryWriter_Close_LeaveOpenFalse_ClosesUnderlyingStream) {
    FlushTrackingStream fs;
    BinaryWriter bw(&fs, false);
    bw.Close();
    EXPECT_TRUE(fs.closeCalled);
}

TEST(BinaryReaderWriterTests, WriteRead_Int32_Roundtrip) {
    MemoryStream ms;
    BinaryWriter bw(&ms, true);
    bw.Write((int32_t)123456);
    bw.Flush();
    auto buf = ms.ToArray();
    MemoryStream ms2(buf.data(), (int32_t)buf.size());
    BinaryReader br(&ms2, true);
    EXPECT_EQ(br.ReadInt32(), 123456);
}

TEST(BinaryReaderWriterTests, WriteRead_NegativeInt32_Roundtrip) {
    MemoryStream ms;
    BinaryWriter bw(&ms, true);
    bw.Write((int32_t)-42);
    bw.Flush();
    auto buf = ms.ToArray();
    MemoryStream ms2(buf.data(), (int32_t)buf.size());
    BinaryReader br(&ms2, true);
    EXPECT_EQ(br.ReadInt32(), -42);
}

TEST(BinaryReaderWriterTests, WriteRead_Int16_Roundtrip) {
    MemoryStream ms;
    BinaryWriter bw(&ms, true);
    bw.Write((int16_t)1000);
    bw.Flush();
    auto buf = ms.ToArray();
    MemoryStream ms2(buf.data(), (int32_t)buf.size());
    BinaryReader br(&ms2, true);
    EXPECT_EQ(br.ReadInt16(), 1000);
}

TEST(BinaryReaderWriterTests, WriteRead_UInt16_Roundtrip) {
    MemoryStream ms;
    BinaryWriter bw(&ms, true);
    bw.Write((uint16_t)65000u);
    bw.Flush();
    auto buf = ms.ToArray();
    MemoryStream ms2(buf.data(), (int32_t)buf.size());
    BinaryReader br(&ms2, true);
    EXPECT_EQ(br.ReadUInt16(), 65000u);
}

TEST(BinaryReaderWriterTests, WriteRead_Int64_Roundtrip) {
    MemoryStream ms;
    BinaryWriter bw(&ms, true);
    bw.Write((int64_t)9876543210LL);
    bw.Flush();
    auto buf = ms.ToArray();
    MemoryStream ms2(buf.data(), (int32_t)buf.size());
    BinaryReader br(&ms2, true);
    EXPECT_EQ(br.ReadInt64(), 9876543210LL);
}

TEST(BinaryReaderWriterTests, WriteRead_Byte_Roundtrip) {
    MemoryStream ms;
    BinaryWriter bw(&ms, true);
    bw.Write((uint8_t)0xAB);
    bw.Flush();
    auto buf = ms.ToArray();
    MemoryStream ms2(buf.data(), (int32_t)buf.size());
    BinaryReader br(&ms2, true);
    EXPECT_EQ(br.ReadByte(), 0xABu);
}

TEST(BinaryReaderWriterTests, WriteRead_Boolean_True_Roundtrip) {
    MemoryStream ms;
    BinaryWriter bw(&ms, true);
    bw.Write(true);
    bw.Flush();
    auto buf = ms.ToArray();
    MemoryStream ms2(buf.data(), (int32_t)buf.size());
    BinaryReader br(&ms2, true);
    EXPECT_TRUE(br.ReadBoolean());
}

TEST(BinaryReaderWriterTests, WriteRead_Boolean_False_Roundtrip) {
    MemoryStream ms;
    BinaryWriter bw(&ms, true);
    bw.Write(false);
    bw.Flush();
    auto buf = ms.ToArray();
    MemoryStream ms2(buf.data(), (int32_t)buf.size());
    BinaryReader br(&ms2, true);
    EXPECT_FALSE(br.ReadBoolean());
}

TEST(BinaryReaderWriterTests, WriteRead_Single_Roundtrip) {
    MemoryStream ms;
    BinaryWriter bw(&ms, true);
    bw.Write(3.14f);
    bw.Flush();
    auto buf = ms.ToArray();
    MemoryStream ms2(buf.data(), (int32_t)buf.size());
    BinaryReader br(&ms2, true);
    EXPECT_FLOAT_EQ(br.ReadSingle(), 3.14f);
}

TEST(BinaryReaderWriterTests, WriteRead_Double_Roundtrip) {
    MemoryStream ms;
    BinaryWriter bw(&ms, true);
    bw.Write(2.718281828);
    bw.Flush();
    auto buf = ms.ToArray();
    MemoryStream ms2(buf.data(), (int32_t)buf.size());
    BinaryReader br(&ms2, true);
    EXPECT_DOUBLE_EQ(br.ReadDouble(), 2.718281828);
}

// Fixes an asymmetry where Write(Single)/Write(double) previously did a raw host-native-order
// memcpy straight to the wire buffer, while ReadSingle()/ReadDouble() always explicitly
// reconstruct little-endian - byte-identical to the fix on every little-endian host this test
// actually runs on (x86/x64/ARM), so this asserts the exact expected IEEE-754 little-endian byte
// sequence directly (not host-endianness-dependent) to lock in the portable, intended behavior
// rather than relying on the roundtrip test above, which cannot distinguish the two
// implementations on a little-endian host.
TEST(BinaryReaderWriterTests, WriteSingle_ProducesExactLittleEndianByteSequence) {
    MemoryStream ms;
    BinaryWriter bw(&ms, true);
    bw.Write(1.0f); // IEEE-754 single bit pattern 0x3F800000
    bw.Flush();
    auto buf = ms.ToArray();
    ASSERT_EQ(buf.size(), 4u);
    EXPECT_EQ(buf[0], 0x00);
    EXPECT_EQ(buf[1], 0x00);
    EXPECT_EQ(buf[2], 0x80);
    EXPECT_EQ(buf[3], 0x3F);
}

TEST(BinaryReaderWriterTests, WriteDouble_ProducesExactLittleEndianByteSequence) {
    MemoryStream ms;
    BinaryWriter bw(&ms, true);
    bw.Write(1.0); // IEEE-754 double bit pattern 0x3FF0000000000000
    bw.Flush();
    auto buf = ms.ToArray();
    ASSERT_EQ(buf.size(), 8u);
    EXPECT_EQ(buf[0], 0x00);
    EXPECT_EQ(buf[1], 0x00);
    EXPECT_EQ(buf[2], 0x00);
    EXPECT_EQ(buf[3], 0x00);
    EXPECT_EQ(buf[4], 0x00);
    EXPECT_EQ(buf[5], 0x00);
    EXPECT_EQ(buf[6], 0xF0);
    EXPECT_EQ(buf[7], 0x3F);
}

TEST(BinaryReaderWriterTests, WriteRead_String_Roundtrip) {
    MemoryStream ms;
    BinaryWriter bw(&ms, true);
    bw.Write(std::string("hello sharp"));
    bw.Flush();
    auto buf = ms.ToArray();
    MemoryStream ms2(buf.data(), (int32_t)buf.size());
    BinaryReader br(&ms2, true);
    EXPECT_EQ(br.ReadString(), "hello sharp");
}

TEST(BinaryReaderWriterTests, BinaryWriter_BaseStreamProperty) {
    MemoryStream ms;
    BinaryWriter bw(&ms, true);
    EXPECT_EQ(bw.getBaseStreamProperty(), &ms);
}

TEST(BinaryReaderWriterTests, Ctor_NullStream_ThrowsArgumentNullException) {
    EXPECT_THROW(BinaryReader br(nullptr), System::ArgumentNullException);
}

TEST(BinaryReaderWriterTests, ReadInt32_PastEndOfStream_ThrowsEndOfStreamException) {
    MemoryStream ms; // empty stream
    BinaryReader br(&ms, true);
    EXPECT_THROW(br.ReadInt32(), System::IO::EndOfStreamException);
}

TEST(BinaryReaderWriterTests, ReadByte_AfterClose_ThrowsObjectDisposedException) {
    MemoryStream ms;
    BinaryWriter bw(&ms, true);
    bw.Write((int32_t)123);
    bw.Flush();
    auto buf = ms.ToArray();
    MemoryStream ms2(buf.data(), (int32_t)buf.size());
    BinaryReader br(&ms2, true);
    br.Close();
    EXPECT_THROW(br.ReadByte(), System::ObjectDisposedException);
}

TEST(BinaryReaderWriterTests, ReadBytes_ReturnsRequestedCount) {
    MemoryStream ms;
    BinaryWriter bw(&ms, true);
    bw.Write((uint8_t)1); bw.Write((uint8_t)2); bw.Write((uint8_t)3);
    bw.Flush();
    auto buf = ms.ToArray();
    MemoryStream ms2(buf.data(), (int32_t)buf.size());
    BinaryReader br(&ms2, true);
    auto bytes = br.ReadBytes(3);
    ASSERT_EQ(bytes.size(), 3u);
    EXPECT_EQ(bytes[0], 1);
    EXPECT_EQ(bytes[1], 2);
    EXPECT_EQ(bytes[2], 3);
}

TEST(BinaryReaderWriterTests, ReadBytes_PastEndOfStream_TrimsInsteadOfThrowing) {
    MemoryStream ms;
    BinaryWriter bw(&ms, true);
    bw.Write((uint8_t)1);
    bw.Flush();
    auto buf = ms.ToArray();
    MemoryStream ms2(buf.data(), (int32_t)buf.size());
    BinaryReader br(&ms2, true);
    auto bytes = br.ReadBytes(10);
    EXPECT_EQ(bytes.size(), 1u);
}

TEST(BinaryReaderWriterTests, ReadBytes_NegativeCount_ThrowsArgumentOutOfRange) {
    MemoryStream ms;
    BinaryReader br(&ms, true);
    EXPECT_THROW(br.ReadBytes(-1), System::ArgumentOutOfRangeException);
}

TEST(BinaryReaderWriterTests, Read7BitEncodedInt_RoundTripsWithReadString) {
    MemoryStream ms;
    BinaryWriter bw(&ms, true);
    bw.Write(std::string("hello"));
    bw.Flush();
    auto buf = ms.ToArray();
    MemoryStream ms2(buf.data(), (int32_t)buf.size());
    BinaryReader br(&ms2, true);
    EXPECT_EQ(br.Read7BitEncodedInt(), 5);
}

TEST(BinaryReaderWriterTests, BinaryWriter_Ctor_NullStream_ThrowsArgumentNullException) {
    EXPECT_THROW(BinaryWriter bw(nullptr), System::ArgumentNullException);
}

TEST(BinaryReaderWriterTests, BinaryWriter_Write_AfterClose_ThrowsObjectDisposedException) {
    MemoryStream ms;
    BinaryWriter bw(&ms, true);
    bw.Close();
    EXPECT_THROW(bw.Write((int32_t)1), System::ObjectDisposedException);
}

TEST(BinaryReaderWriterTests, BinaryWriter_Close_RespectsLeaveOpen) {
    MemoryStream ms;
    {
        BinaryWriter bw(&ms, true); // leaveOpen = true
        bw.Write((int32_t)42);
        bw.Close();
    }
    // Stream must still be usable after the writer closed, since leaveOpen was true.
    EXPECT_NO_THROW(ms.ToArray());
}

TEST(BinaryReaderWriterTests, Write7BitEncodedInt_RoundTrip) {
    MemoryStream ms;
    BinaryWriter bw(&ms, true);
    bw.Write7BitEncodedInt(300); // requires 2 encoded bytes
    bw.Flush();
    auto buf = ms.ToArray();
    MemoryStream ms2(buf.data(), (int32_t)buf.size());
    BinaryReader br(&ms2, true);
    EXPECT_EQ(br.Read7BitEncodedInt(), 300);
}

// ===========================================================================
// StreamWriter + StreamReader
// ===========================================================================

TEST(StreamWriterReaderTests, WriteString_ReadToEnd_Roundtrip) {
    MemoryStream ms;
    StreamWriter sw(&ms, true);
    sw.Write(std::string("sharp runtime"));
    sw.Flush();
    auto buf = ms.ToArray();
    MemoryStream ms2(buf.data(), (int32_t)buf.size());
    StreamReader sr(&ms2);
    EXPECT_EQ(sr.ReadToEnd(), "sharp runtime");
}

TEST(StreamWriterReaderTests, WriteLine_AppendsNewline) {
    MemoryStream ms;
    StreamWriter sw(&ms, true);
    sw.WriteLine("test");
    sw.Flush();
    auto buf = ms.ToArray();
    MemoryStream ms2(buf.data(), (int32_t)buf.size());
    StreamReader sr(&ms2);
    std::string result = sr.ReadToEnd();
    EXPECT_NE(result.find("test"), std::string::npos);
    EXPECT_NE(result.find('\n'), std::string::npos);
}

TEST(StreamWriterReaderTests, WriteInt_Roundtrip) {
    MemoryStream ms;
    StreamWriter sw(&ms, true);
    sw.Write((int32_t)42);
    sw.Flush();
    auto buf = ms.ToArray();
    MemoryStream ms2(buf.data(), (int32_t)buf.size());
    StreamReader sr(&ms2);
    EXPECT_EQ(sr.ReadToEnd(), "42");
}

TEST(StreamWriterReaderTests, WriteBool_True) {
    MemoryStream ms;
    StreamWriter sw(&ms, true);
    sw.Write(true);
    sw.Flush();
    auto buf = ms.ToArray();
    MemoryStream ms2(buf.data(), (int32_t)buf.size());
    StreamReader sr(&ms2);
    EXPECT_FALSE(sr.ReadToEnd().empty());
}

TEST(StreamWriterReaderTests, WriteToFile_ReadBackWithFile) {
    std::string p = tf("sw_file.txt");
    {
        StreamWriter sw(p);
        sw.Write(std::string("from file writer"));
        sw.Close();
    }
    EXPECT_EQ(File::ReadAllText(p), "from file writer");
    File::Delete(p);
}

TEST(StreamWriterReaderTests, StreamWriter_BaseStreamProperty) {
    MemoryStream ms;
    StreamWriter sw(&ms, true);
    EXPECT_EQ(sw.getBaseStreamProperty(), &ms);
}

// Regression: Close() previously closed the underlying stream unconditionally, ignoring
// leaveOpen (only the destructor honored it). These use FlushTrackingStream (defined above,
// near the BinaryWriter tests) rather than MemoryStream: MemoryStream::Close() is itself a
// no-op against real .NET (see MemoryStreamTests.Close_DoesNotClearBufferOrPosition), so its
// buffer being cleared or not is no longer a signal of whether Close() was actually called on
// it.
TEST(StreamWriterReaderTests, StreamWriter_Close_LeaveOpenTrue_DoesNotCloseUnderlyingStream) {
    FlushTrackingStream fs;
    StreamWriter sw(&fs, true);
    sw.Write(std::string("data"));
    sw.Close();
    EXPECT_FALSE(fs.closeCalled);
}

TEST(StreamWriterReaderTests, StreamWriter_Close_LeaveOpenFalse_ClosesUnderlyingStream) {
    FlushTrackingStream fs;
    StreamWriter sw(&fs, false);
    sw.Write(std::string("data"));
    sw.Close();
    EXPECT_TRUE(fs.closeCalled);
}

TEST(StreamWriterReaderTests, StreamReader_Close_LeaveOpenTrue_DoesNotCloseUnderlyingStream) {
    FlushTrackingStream fs;
    StreamReader sr(&fs, true);
    sr.Close();
    EXPECT_FALSE(fs.closeCalled);
}

TEST(StreamWriterReaderTests, StreamReader_Close_LeaveOpenFalse_ClosesUnderlyingStream) {
    FlushTrackingStream fs;
    StreamReader sr(&fs, false);
    sr.Close();
    EXPECT_TRUE(fs.closeCalled);
}

TEST(StreamWriterReaderTests, WriteStringLiteral_DoesNotResolveToBoolOverload) {
    // Regression: TextWriter previously had no Write(const char*) overload, so a string
    // literal bound to Write(bool) via a preferred standard pointer-to-bool conversion
    // instead of the user-defined conversion to std::string, silently writing "True".
    MemoryStream ms;
    StreamWriter sw(&ms, true);
    sw.Write("hello");
    sw.Flush();
    auto buf = ms.ToArray();
    MemoryStream ms2(buf.data(), static_cast<int32_t>(buf.size()));
    StreamReader sr(&ms2);
    EXPECT_EQ(sr.ReadToEnd(), "hello");
}

TEST(StreamWriterReaderTests, StreamReader_PeekDoesNotAdvance) {
    MemoryStream ms;
    StreamWriter sw(&ms, true);
    sw.Write("ab");
    sw.Flush();
    auto buf = ms.ToArray();
    MemoryStream ms2(buf.data(), static_cast<int32_t>(buf.size()));
    StreamReader sr(&ms2);
    EXPECT_EQ(sr.Peek(), int('a'));
    EXPECT_EQ(sr.Peek(), int('a'));
    EXPECT_EQ(sr.Read(), int('a'));
    EXPECT_EQ(sr.Read(), int('b'));
    EXPECT_EQ(sr.Read(), -1);
}

TEST(StreamWriterReaderTests, StreamReader_ReadLine) {
    MemoryStream ms;
    StreamWriter sw(&ms, true);
    sw.Write("line1\nline2\nline3");
    sw.Flush();
    auto buf = ms.ToArray();
    MemoryStream ms2(buf.data(), static_cast<int32_t>(buf.size()));
    StreamReader sr(&ms2);
    EXPECT_EQ(sr.ReadLine(), "line1");
    EXPECT_EQ(sr.ReadLine(), "line2");
    EXPECT_EQ(sr.ReadLine(), "line3");
}

// Regression test for a wave-3 audit finding: ReadLine() only stopped scanning at '\n', so a
// lone '\r' (classic Mac line ending, not followed by '\n') was treated as ordinary line
// content instead of a line terminator. Verified against StreamReader.cs's ReadLine(), which
// treats '\r' and '\n' as interchangeable terminators.
TEST(StreamWriterReaderTests, StreamReader_ReadLine_LoneCarriageReturn_TerminatesLine) {
    MemoryStream ms;
    StreamWriter sw(&ms, true);
    sw.Write("line1\rline2\nline3");
    sw.Flush();
    auto buf = ms.ToArray();
    MemoryStream ms2(buf.data(), static_cast<int32_t>(buf.size()));
    StreamReader sr(&ms2);
    EXPECT_EQ(sr.ReadLine(), "line1");
    EXPECT_EQ(sr.ReadLine(), "line2");
    EXPECT_EQ(sr.ReadLine(), "line3");
}

TEST(StreamWriterReaderTests, StreamReader_PathCtor_ReadsFile) {
    std::string p = tf("sr_file.txt");
    File::WriteAllText(p, "from file reader");
    StreamReader sr(p);
    EXPECT_EQ(sr.ReadToEnd(), "from file reader");
    sr.Close();
    File::Delete(p);
}

// ===========================================================================
// BufferedStream
// ===========================================================================

TEST(BufferedStreamTests, Constructor_NoThrow) {
    MemoryStream ms;
    EXPECT_NO_THROW(BufferedStream{&ms});
}

TEST(BufferedStreamTests, DelegatesWrite_AndRead) {
    MemoryStream ms;
    BufferedStream bs(&ms);
    uint8_t writeData[] = {1, 2, 3, 4};
    bs.Write(writeData, 0, 4);
    bs.Flush();
    EXPECT_EQ(ms.getLengthProperty(), 4);
}

TEST(BufferedStreamTests, DelegatesGetCanWriteProperty) {
    MemoryStream ms;
    BufferedStream bs(&ms);
    EXPECT_EQ(bs.getCanWriteProperty(), ms.getCanWriteProperty());
}

TEST(BufferedStreamTests, DelegatesGetLengthProperty) {
    MemoryStream ms;
    uint8_t data[] = {10, 20};
    ms.Write(data, 0, 2);
    BufferedStream bs(&ms);
    EXPECT_EQ(bs.getLengthProperty(), 2);
}

TEST(BufferedStreamTests, Constructor_NullStream_ThrowsArgumentNullException) {
    EXPECT_THROW(BufferedStream bs(nullptr), System::ArgumentNullException);
}

TEST(BufferedStreamTests, DelegatesPosition) {
    MemoryStream ms;
    uint8_t data[] = {1, 2, 3};
    ms.Write(data, 0, 3);
    BufferedStream bs(&ms);
    bs.setPositionProperty(1);
    EXPECT_EQ(bs.getPositionProperty(), 1);
    EXPECT_EQ(ms.getPositionProperty(), 1);
}

// --- ticket 1477: real internal buffering ---

TEST(BufferedStreamTests, SmallWrite_NotImmediatelyPropagatedToInnerStream) {
    MemoryStream ms;
    BufferedStream bs(&ms);
    uint8_t writeData[] = {1, 2, 3, 4};
    bs.Write(writeData, 0, 4);
    // A write far smaller than the buffer should be held internally, not yet reach the inner stream.
    EXPECT_EQ(ms.getLengthProperty(), 0);
    bs.Flush();
    EXPECT_EQ(ms.getLengthProperty(), 4);
}

TEST(BufferedStreamTests, MultipleSmallWrites_BatchedIntoOneUnderlyingWrite) {
    MemoryStream ms;
    BufferedStream bs(&ms, 64);
    for (int i = 0; i < 10; ++i) {
        uint8_t b = static_cast<uint8_t>(i);
        bs.Write(&b, 0, 1);
    }
    EXPECT_EQ(ms.getLengthProperty(), 0);
    bs.Flush();
    EXPECT_EQ(ms.getLengthProperty(), 10);
    ms.setPositionProperty(0);
    for (int i = 0; i < 10; ++i) {
        uint8_t out = 0;
        EXPECT_EQ(ms.Read(&out, 0, 1), 1);
        EXPECT_EQ(out, static_cast<uint8_t>(i));
    }
}

TEST(BufferedStreamTests, WriteLargerThanBufferSize_BypassesBufferDirectly) {
    MemoryStream ms;
    BufferedStream bs(&ms, 8);
    std::vector<uint8_t> data(100, 0x42);
    bs.Write(data.data(), 0, static_cast<intcs>(data.size()));
    // Empty buffer + a chunk >= bufferSize goes straight through, per real .NET's bypass threshold.
    EXPECT_EQ(ms.getLengthProperty(), 100);
}

TEST(BufferedStreamTests, ReadThroughBuffer_RoundTrip) {
    MemoryStream ms;
    uint8_t data[] = {10, 20, 30, 40, 50};
    ms.Write(data, 0, 5);
    ms.setPositionProperty(0);
    BufferedStream bs(&ms);
    uint8_t out[5] = {};
    EXPECT_EQ(bs.Read(out, 0, 5), 5);
    EXPECT_EQ(std::vector<uint8_t>(out, out + 5), std::vector<uint8_t>(data, data + 5));
}

TEST(BufferedStreamTests, SmallRead_ServedFromInternalBufferOnSubsequentCalls) {
    MemoryStream ms;
    uint8_t data[] = {1, 2, 3, 4, 5, 6};
    ms.Write(data, 0, 6);
    ms.setPositionProperty(0);
    BufferedStream bs(&ms, 64);
    uint8_t a = 0, b = 0;
    EXPECT_EQ(bs.Read(&a, 0, 1), 1);
    EXPECT_EQ(a, 1);
    // The first Read should have pulled the whole (short) stream into the internal buffer;
    // the inner stream's position should already be at EOF even though only 1 byte was consumed.
    EXPECT_EQ(ms.getPositionProperty(), 6);
    EXPECT_EQ(bs.Read(&b, 0, 1), 1);
    EXPECT_EQ(b, 2);
}

TEST(BufferedStreamTests, Close_FlushesPendingBufferedWrites) {
    MemoryStream ms;
    {
        BufferedStream bs(&ms);
        uint8_t writeData[] = {9, 9, 9};
        bs.Write(writeData, 0, 3);
        bs.Close();
    }
    EXPECT_EQ(ms.getLengthProperty(), 3);
}

TEST(BufferedStreamTests, Destructor_FlushesPendingBufferedWrites) {
    MemoryStream ms;
    {
        BufferedStream bs(&ms);
        uint8_t writeData[] = {7, 7};
        bs.Write(writeData, 0, 2);
    }
    EXPECT_EQ(ms.getLengthProperty(), 2);
}

TEST(BufferedStreamTests, BufferSizeConstructor_UsesGivenSize) {
    MemoryStream ms;
    BufferedStream bs(&ms, 128);
    EXPECT_EQ(bs.getBufferSizeProperty(), 128);
}

TEST(BufferedStreamTests, Constructor_NonPositiveBufferSize_ThrowsArgumentOutOfRangeException) {
    MemoryStream ms1, ms2;
    EXPECT_THROW(BufferedStream(&ms1, 0), System::ArgumentOutOfRangeException);
    EXPECT_THROW(BufferedStream(&ms2, -1), System::ArgumentOutOfRangeException);
}

TEST(BufferedStreamTests, WriteThenRead_FlushesPendingWriteBufferFirst) {
    MemoryStream ms;
    BufferedStream bs(&ms);
    uint8_t writeData[] = {1, 2, 3};
    bs.Write(writeData, 0, 3);
    bs.setPositionProperty(0);
    uint8_t out[3] = {};
    EXPECT_EQ(bs.Read(out, 0, 3), 3);
    EXPECT_EQ(std::vector<uint8_t>(out, out + 3), std::vector<uint8_t>(writeData, writeData + 3));
}

TEST(BufferedStreamTests, UnderlyingStreamProperty_ReturnsWrappedStream) {
    MemoryStream ms;
    BufferedStream bs(&ms);
    EXPECT_EQ(bs.getUnderlyingStreamProperty(), &ms);
}

// ===========================================================================
// FileStream
// ===========================================================================

TEST(FileStreamTests, CreateAndOpen_WriteReadRoundtrip) {
    std::string p = tf("fstream_rw.bin");
    {
        FileStream fs(p, FileMode::Create, FileAccess::Write);
        uint8_t data[] = {10, 20, 30, 40};
        fs.Write(data, 0, 4);
        fs.Close();
    }
    {
        FileStream fs(p, FileMode::Open, FileAccess::Read);
        EXPECT_EQ(fs.getLengthProperty(), 4);
        uint8_t buf[4] = {};
        fs.Read(buf, 0, 4);
        EXPECT_EQ(buf[0], 10u);
        EXPECT_EQ(buf[3], 40u);
        fs.Close();
    }
    File::Delete(p);
}

TEST(FileStreamTests, CreateMode_CanWrite) {
    std::string p = tf("fstream_write.bin");
    FileStream fs(p, FileMode::Create, FileAccess::Write);
    EXPECT_TRUE(fs.getCanWriteProperty());
    fs.Close();
    File::Delete(p);
}

TEST(FileStreamTests, OpenMode_CanRead) {
    std::string p = tf("fstream_read.bin");
    File::WriteAllText(p, "abc");
    FileStream fs(p, FileMode::Open, FileAccess::Read);
    EXPECT_TRUE(fs.getCanReadProperty());
    fs.Close();
    File::Delete(p);
}

TEST(FileStreamTests, IsOpen_AfterConstruct_True) {
    std::string p = tf("fstream_open.bin");
    FileStream fs(p, FileMode::Create);
    EXPECT_TRUE(fs.IsOpen());
    fs.Close();
    File::Delete(p);
}

TEST(FileStreamTests, getLengthProperty_AfterWrite) {
    std::string p = tf("fstream_len.bin");
    {
        FileStream fs(p, FileMode::Create, FileAccess::Write);
        uint8_t data[] = {1, 2, 3, 4, 5};
        fs.Write(data, 0, 5);
        fs.Close();
    }
    FileStream fs(p, FileMode::Open);
    EXPECT_EQ(fs.getLengthProperty(), 5);
    fs.Close();
    File::Delete(p);
}

TEST(FileStreamTests, getLengthProperty_OnSameOpenInstance_AfterWrite_ReflectsExtension) {
    // Regression: previously length_ was only cached at construction (0 for FileMode::Create)
    // and only ever updated by SetLength() -- Write() never touched it, so querying Length on
    // the SAME still-open FileStream right after a create-then-write returned the stale,
    // construction-time value (0) instead of the file's actual current size.
    std::string p = tf("fstream_len_live.bin");
    FileStream fs(p, FileMode::Create, FileAccess::Write);
    EXPECT_EQ(fs.getLengthProperty(), 0);
    uint8_t data[] = {1, 2, 3, 4, 5};
    fs.Write(data, 0, 5);
    EXPECT_EQ(fs.getLengthProperty(), 5);
    fs.Write(data, 0, 3);
    EXPECT_EQ(fs.getLengthProperty(), 8);
    fs.Close();
    File::Delete(p);
}

TEST(FileStreamTests, WriteByte_Flush_NoThrow) {
    std::string p = tf("fstream_wb.bin");
    FileStream fs(p, FileMode::Create, FileAccess::Write);
    EXPECT_NO_THROW(fs.WriteByte(0xFF));
    EXPECT_NO_THROW(fs.Flush());
    fs.Close();
    File::Delete(p);
}

TEST(FileStreamTests, CreateMode_WriteOnly_CanReadIsFalse) {
    // Regression: getCanReadProperty() previously ignored the actual opened access
    // and returned true here even though the file wasn't opened with std::ios::in.
    std::string p = tf("fstream_create_writeonly.bin");
    FileStream fs(p, FileMode::Create, FileAccess::Write);
    EXPECT_FALSE(fs.getCanReadProperty());
    EXPECT_TRUE(fs.getCanWriteProperty());
    fs.Close();
    File::Delete(p);
}

TEST(FileStreamTests, CreateNew_ExistingFile_ThrowsIOException) {
    std::string p = tf("fstream_createnew_exists.bin");
    File::WriteAllText(p, "x");
    EXPECT_THROW(FileStream(p, FileMode::CreateNew, FileAccess::ReadWrite), System::IO::IOException);
    File::Delete(p);
}

TEST(FileStreamTests, CreateNew_NonExistentFile_Succeeds) {
    std::string p = tf("fstream_createnew_new.bin");
    File::Delete(p); // ensure absent
    EXPECT_NO_THROW({
        FileStream fs(p, FileMode::CreateNew, FileAccess::ReadWrite);
        fs.Close();
    });
    EXPECT_TRUE(File::Exists(p));
    File::Delete(p);
}

TEST(FileStreamTests, Open_NonExistentFile_ThrowsFileNotFoundException) {
    EXPECT_THROW(FileStream(tf("fstream_open_missing.bin"), FileMode::Open, FileAccess::Read),
                 System::IO::FileNotFoundException);
}

TEST(FileStreamTests, Truncate_NonExistentFile_ThrowsFileNotFoundException) {
    EXPECT_THROW(FileStream(tf("fstream_truncate_missing.bin"), FileMode::Truncate, FileAccess::ReadWrite),
                 System::IO::FileNotFoundException);
}

// Regression tests for a wave-3 audit finding: opening/creating a file whose *parent*
// directory doesn't exist threw a generic IOException (or, for FileMode::Open,
// FileNotFoundException) instead of DirectoryNotFoundException. Verified against
// Interop.IOErrors.cs's GetExceptionForIoErrno: "For Windows compatibility, throw
// DirectoryNotFoundException instead of FileNotFoundException when the parent folder does not
// exist."
TEST(FileStreamTests, Open_MissingParentDirectory_ThrowsDirectoryNotFoundException) {
    EXPECT_THROW(FileStream(tf("no_such_parent_dir_xyz/file.bin"), FileMode::Open, FileAccess::Read),
                 System::IO::DirectoryNotFoundException);
}

TEST(FileStreamTests, Create_MissingParentDirectory_ThrowsDirectoryNotFoundException) {
    EXPECT_THROW(FileStream(tf("no_such_parent_dir_xyz/file.bin"), FileMode::Create, FileAccess::ReadWrite),
                 System::IO::DirectoryNotFoundException);
}

TEST(FileStreamTests, OpenOrCreate_NonExistentFile_CreatesEmpty) {
    std::string p = tf("fstream_openorcreate_new.bin");
    File::Delete(p);
    FileStream fs(p, FileMode::OpenOrCreate, FileAccess::ReadWrite);
    EXPECT_EQ(fs.getLengthProperty(), 0);
    fs.Close();
    File::Delete(p);
}

TEST(FileStreamTests, OpenOrCreate_ExistingFile_PreservesContent) {
    std::string p = tf("fstream_openorcreate_existing.bin");
    File::WriteAllText(p, "hello");
    FileStream fs(p, FileMode::OpenOrCreate, FileAccess::ReadWrite);
    EXPECT_EQ(fs.getLengthProperty(), 5);
    fs.Close();
    File::Delete(p);
}

TEST(FileStreamTests, Append_WithReadAccess_ThrowsArgumentException) {
    std::string p = tf("fstream_append_read.bin");
    EXPECT_THROW(FileStream(p, FileMode::Append, FileAccess::ReadWrite), System::ArgumentException);
}

TEST(FileStreamTests, Create_WithReadOnlyAccess_ThrowsArgumentException) {
    std::string p = tf("fstream_create_readonly.bin");
    EXPECT_THROW(FileStream(p, FileMode::Create, FileAccess::Read), System::ArgumentException);
}

TEST(FileStreamTests, CanSeek_TrueWhileOpen) {
    std::string p = tf("fstream_canseek.bin");
    FileStream fs(p, FileMode::Create, FileAccess::ReadWrite);
    EXPECT_TRUE(fs.getCanSeekProperty());
    fs.Close();
    File::Delete(p);
}

TEST(FileStreamTests, Position_ReflectsReadProgress) {
    std::string p = tf("fstream_position_read.bin");
    File::WriteAllText(p, "abcde");
    FileStream fs(p, FileMode::Open, FileAccess::Read);
    EXPECT_EQ(fs.getPositionProperty(), 0);
    uint8_t buf[3] = {};
    fs.Read(buf, 0, 3);
    EXPECT_EQ(fs.getPositionProperty(), 3);
    fs.Close();
    File::Delete(p);
}

TEST(FileStreamTests, SetPosition_SeeksForRead) {
    std::string p = tf("fstream_setposition.bin");
    File::WriteAllText(p, "abcde");
    FileStream fs(p, FileMode::Open, FileAccess::Read);
    fs.setPositionProperty(2);
    uint8_t buf[3] = {};
    intcs n = fs.Read(buf, 0, 3);
    EXPECT_EQ(n, 3);
    EXPECT_EQ(buf[0], 'c');
    fs.Close();
    File::Delete(p);
}

TEST(FileStreamTests, Seek_FromBegin_MatchesSetPosition) {
    std::string p = tf("fstream_seek_begin.bin");
    File::WriteAllText(p, "abcde");
    FileStream fs(p, FileMode::Open, FileAccess::Read);
    intcs newPos = fs.Seek(2, SeekOrigin::Begin);
    EXPECT_EQ(newPos, 2);
    EXPECT_EQ(fs.getPositionProperty(), 2);
    fs.Close();
    File::Delete(p);
}

TEST(FileStreamTests, Seek_FromCurrent_AdvancesRelatively) {
    std::string p = tf("fstream_seek_current.bin");
    File::WriteAllText(p, "abcde");
    FileStream fs(p, FileMode::Open, FileAccess::Read);
    fs.setPositionProperty(1);
    intcs newPos = fs.Seek(2, SeekOrigin::Current);
    EXPECT_EQ(newPos, 3);
    fs.Close();
    File::Delete(p);
}

TEST(FileStreamTests, Seek_FromEnd_ComputesFromLength) {
    std::string p = tf("fstream_seek_end.bin");
    File::WriteAllText(p, "abcde"); // length 5
    FileStream fs(p, FileMode::Open, FileAccess::Read);
    intcs newPos = fs.Seek(-2, SeekOrigin::End);
    EXPECT_EQ(newPos, 3);
    fs.Close();
    File::Delete(p);
}

TEST(FileStreamTests, SetLength_Truncates) {
    std::string p = tf("fstream_setlength_truncate.bin");
    File::WriteAllText(p, "abcdefgh");
    FileStream fs(p, FileMode::Open, FileAccess::ReadWrite);
    fs.SetLength(3);
    EXPECT_EQ(fs.getLengthProperty(), 3);
    fs.Close();
    EXPECT_EQ(File::ReadAllText(p), "abc");
    File::Delete(p);
}

TEST(FileStreamTests, SetLength_Extends) {
    std::string p = tf("fstream_setlength_extend.bin");
    File::WriteAllText(p, "ab");
    FileStream fs(p, FileMode::Open, FileAccess::ReadWrite);
    fs.SetLength(5);
    EXPECT_EQ(fs.getLengthProperty(), 5);
    fs.Close();
    File::Delete(p);
}

TEST(FileStreamTests, SetLength_WhenReadOnly_ThrowsNotSupportedException) {
    std::string p = tf("fstream_setlength_readonly.bin");
    File::WriteAllText(p, "abc");
    FileStream fs(p, FileMode::Open, FileAccess::Read);
    EXPECT_THROW(fs.SetLength(1), System::NotSupportedException);
    fs.Close();
    File::Delete(p);
}

TEST(FileStreamTests, ReadByte_ReturnsBytesThenMinusOne) {
    std::string p = tf("fstream_readbyte.bin");
    File::WriteAllText(p, "A");
    FileStream fs(p, FileMode::Open, FileAccess::Read);
    EXPECT_EQ(fs.ReadByte(), static_cast<intcs>('A'));
    EXPECT_EQ(fs.ReadByte(), -1);
    fs.Close();
    File::Delete(p);
}

TEST(FileStreamTests, Read_AfterClose_ThrowsObjectDisposedException) {
    std::string p = tf("fstream_read_after_close.bin");
    File::WriteAllText(p, "abc");
    FileStream fs(p, FileMode::Open, FileAccess::Read);
    fs.Close();
    uint8_t buf[3] = {};
    EXPECT_THROW(fs.Read(buf, 0, 3), System::ObjectDisposedException);
    File::Delete(p);
}

TEST(FileStreamTests, Write_AfterClose_ThrowsObjectDisposedException) {
    std::string p = tf("fstream_write_after_close.bin");
    FileStream fs(p, FileMode::Create, FileAccess::Write);
    fs.Close();
    uint8_t buf[3] = {1, 2, 3};
    EXPECT_THROW(fs.Write(buf, 0, 3), System::ObjectDisposedException);
    File::Delete(p);
}

TEST(FileStreamTests, Read_NullBuffer_ThrowsArgumentNullException) {
    std::string p = tf("fstream_read_null.bin");
    File::WriteAllText(p, "abc");
    FileStream fs(p, FileMode::Open, FileAccess::Read);
    EXPECT_THROW(fs.Read(nullptr, 0, 3), System::ArgumentNullException);
    fs.Close();
    File::Delete(p);
}

TEST(FileStreamTests, Read_NegativeOffset_ThrowsArgumentOutOfRangeException) {
    std::string p = tf("fstream_read_negoffset.bin");
    File::WriteAllText(p, "abc");
    FileStream fs(p, FileMode::Open, FileAccess::Read);
    uint8_t buf[3] = {};
    EXPECT_THROW(fs.Read(buf, -1, 3), System::ArgumentOutOfRangeException);
    fs.Close();
    File::Delete(p);
}

TEST(FileStreamTests, Read_NegativeCount_ThrowsArgumentOutOfRangeException) {
    std::string p = tf("fstream_read_negcount.bin");
    File::WriteAllText(p, "abc");
    FileStream fs(p, FileMode::Open, FileAccess::Read);
    uint8_t buf[3] = {};
    EXPECT_THROW(fs.Read(buf, 0, -1), System::ArgumentOutOfRangeException);
    fs.Close();
    File::Delete(p);
}

TEST(FileStreamTests, SetLength_AfterClose_ThrowsObjectDisposedException_DoesNotResizeFile) {
    // Regression: SetLength() previously checked only canWrite_ (never reset by Close()),
    // so calling it after Close() silently resized the file on disk despite the stream
    // claiming to be closed.
    std::string p = tf("fstream_setlength_after_close.bin");
    File::WriteAllText(p, "abcdefgh");
    FileStream fs(p, FileMode::Open, FileAccess::ReadWrite);
    fs.Close();
    EXPECT_THROW(fs.SetLength(3), System::ObjectDisposedException);
    EXPECT_EQ(File::ReadAllText(p), "abcdefgh");
    File::Delete(p);
}

// ===========================================================================
// IsolatedStorageFile
// ===========================================================================

TEST(IsolatedStorageFileTests, GetUserStoreForApplication_NoThrow) {
    auto store = IsolatedStorageFile::GetUserStoreForApplication();
    EXPECT_FALSE(store.getRootDirectoryProperty().string().empty());
}

TEST(IsolatedStorageFileTests, FileExists_False_ForNonExistent) {
    auto store = IsolatedStorageFile::GetUserStoreForApplication();
    EXPECT_FALSE(store.FileExists("sharp_rt_nonexistent_xyzxyz_12345.dat"));
}

TEST(IsolatedStorageFileTests, OpenFile_WriteAndDelete_Roundtrip) {
    auto store = IsolatedStorageFile::GetUserStoreForApplication();
    const std::string fname = "sharp_rt_iso_test_file.dat";
    {
        auto stream = store.OpenFile(fname, FileMode::Create);
        uint8_t data[] = {0xDE, 0xAD, 0xBE, 0xEF};
        stream.Write(data, 0, 4);
        stream.Close();
    }
    EXPECT_TRUE(store.FileExists(fname));
    store.DeleteFile(fname);
    EXPECT_FALSE(store.FileExists(fname));
}

TEST(IsolatedStorageFileTests, OpenFile_OpenMode_SupportsReadAndWrite) {
    // Regression: IsolatedStorageFileStream previously opened FileMode::Open as read-only
    // (std::ios::in only), unlike .NET's IsolatedStorageFileStream(path, mode) which defaults
    // access to ReadWrite for every mode except Append.
    auto store = IsolatedStorageFile::GetUserStoreForApplication();
    const std::string fname = "sharp_rt_iso_openmode_rw.dat";
    { auto s = store.CreateFile(fname); s.Close(); }

    auto stream = store.OpenFile(fname, FileMode::Open);
    EXPECT_TRUE(stream.getCanReadProperty());
    EXPECT_TRUE(stream.getCanWriteProperty());
    stream.Close();
    store.DeleteFile(fname);
}

TEST(IsolatedStorageFileTests, OpenFile_SupportsSeek) {
    // Regression: IsolatedStorageFileStream previously duplicated a thin std::fstream wrapper
    // with no Position/Seek support at all; it now derives from the real FileStream.
    auto store = IsolatedStorageFile::GetUserStoreForApplication();
    const std::string fname = "sharp_rt_iso_seek.dat";
    auto stream = store.CreateFile(fname);
    uint8_t data[] = {1, 2, 3, 4, 5};
    stream.Write(data, 0, 5);

    stream.setPositionProperty(1);
    uint8_t buf[3] = {};
    intcs n = stream.Read(buf, 0, 3);
    EXPECT_EQ(n, 3);
    EXPECT_EQ(buf[0], 2u);

    stream.Close();
    store.DeleteFile(fname);
}

TEST(IsolatedStorageFileTests, CreateFile_Creates) {
    auto store = IsolatedStorageFile::GetUserStoreForApplication();
    const std::string fname = "sharp_rt_iso_create.dat";
    { auto s = store.CreateFile(fname); s.Close(); }
    EXPECT_TRUE(store.FileExists(fname));
    store.DeleteFile(fname);
}

TEST(IsolatedStorageFileTests, CopyFile_CopiesContent) {
    auto store = IsolatedStorageFile::GetUserStoreForApplication();
    const std::string src = "sharp_rt_iso_src.dat";
    const std::string dst = "sharp_rt_iso_dst.dat";
    { auto s = store.CreateFile(src); s.Close(); }
    store.CopyFile(src, dst);
    EXPECT_TRUE(store.FileExists(dst));
    store.DeleteFile(src);
    store.DeleteFile(dst);
}

TEST(IsolatedStorageFileTests, MoveFile_MovesFile) {
    auto store = IsolatedStorageFile::GetUserStoreForApplication();
    const std::string src = "sharp_rt_iso_mv_src.dat";
    const std::string dst = "sharp_rt_iso_mv_dst.dat";
    { auto s = store.CreateFile(src); s.Close(); }
    store.MoveFile(src, dst);
    EXPECT_FALSE(store.FileExists(src));
    EXPECT_TRUE(store.FileExists(dst));
    store.DeleteFile(dst);
}

TEST(IsolatedStorageFileTests, GetFileNames_ReturnsCreatedFile) {
    auto store = IsolatedStorageFile::GetUserStoreForApplication();
    const std::string fname = "sharp_rt_iso_list.dat";
    { auto s = store.CreateFile(fname); s.Close(); }
    auto names = store.GetFileNames("sharp_rt_iso_list*");
    EXPECT_FALSE(names.empty());
    store.DeleteFile(fname);
}

TEST(IsolatedStorageFileTests, DirectoryExists_FalseForNonExistent) {
    auto store = IsolatedStorageFile::GetUserStoreForApplication();
    EXPECT_FALSE(store.DirectoryExists("sharp_rt_iso_no_such_dir_xyz"));
}

TEST(IsolatedStorageFileTests, CreateDirectory_DirectoryExists_Delete) {
    auto store = IsolatedStorageFile::GetUserStoreForApplication();
    const std::string dir = "sharp_rt_iso_testdir";
    store.CreateDirectory(dir);
    EXPECT_TRUE(store.DirectoryExists(dir));
    store.DeleteDirectory(dir);
    EXPECT_FALSE(store.DirectoryExists(dir));
}

TEST(IsolatedStorageFileTests, MoveDirectory_MovesDir) {
    auto store = IsolatedStorageFile::GetUserStoreForApplication();
    const std::string src = "sharp_rt_iso_dir_src";
    const std::string dst = "sharp_rt_iso_dir_dst";
    store.CreateDirectory(src);
    store.MoveDirectory(src, dst);
    EXPECT_FALSE(store.DirectoryExists(src));
    EXPECT_TRUE(store.DirectoryExists(dst));
    store.DeleteDirectory(dst);
}

TEST(IsolatedStorageFileTests, GetDirectoryNames_ReturnsCreatedDir) {
    auto store = IsolatedStorageFile::GetUserStoreForApplication();
    const std::string dir = "sharp_rt_iso_listed_dir";
    store.CreateDirectory(dir);
    auto names = store.GetDirectoryNames("sharp_rt_iso_listed*");
    EXPECT_FALSE(names.empty());
    store.DeleteDirectory(dir);
}

TEST(IsolatedStorageFileTests, AvailableFreeSpace_Positive) {
    auto store = IsolatedStorageFile::GetUserStoreForApplication();
    EXPECT_GT(store.getAvailableFreeSpaceProperty(), 0);
}

TEST(IsolatedStorageFileTests, UsedSize_AfterWrite_Positive) {
    auto store = IsolatedStorageFile::GetUserStoreForApplication();
    const std::string fname = "sharp_rt_iso_used.dat";
    {
        auto s = store.CreateFile(fname);
        uint8_t buf[64] = {};
        s.Write(buf, 0, 64);
        s.Close();
    }
    EXPECT_GE(store.getUsedSizeProperty(), 64);
    store.DeleteFile(fname);
}

TEST(IsolatedStorageFileTests, Dispose_DoesNotThrow) {
    auto store = IsolatedStorageFile::GetUserStoreForApplication();
    EXPECT_NO_THROW(store.Dispose());
}

TEST(IsolatedStorageFileTests, AfterDispose_OperationsThrowObjectDisposedException) {
    auto store = IsolatedStorageFile::GetUserStoreForApplication();
    store.Dispose();
    EXPECT_THROW(store.FileExists("x"), System::ObjectDisposedException);
    EXPECT_THROW(store.DirectoryExists("x"), System::ObjectDisposedException);
    EXPECT_THROW(store.CreateDirectory("x"), System::ObjectDisposedException);
    EXPECT_THROW(store.DeleteFile("x"), System::ObjectDisposedException);
    EXPECT_THROW(store.GetFileNames(), System::ObjectDisposedException);
    EXPECT_THROW(store.GetDirectoryNames(), System::ObjectDisposedException);
}

TEST(IsolatedStorageFileTests, DeleteDirectory_NonEmpty_ThrowsInsteadOfRecursivelyDeleting) {
    // Regression: DeleteDirectory previously used remove_all (recursive), silently deleting an
    // entire subtree instead of matching real .NET's Directory.Delete(path, recursive: false)
    // "must be empty" contract.
    auto store = IsolatedStorageFile::GetUserStoreForApplication();
    const std::string dir = "sharp_rt_iso_nonempty_dir";
    const std::string nested = dir + "/nested.dat";
    store.CreateDirectory(dir);
    { auto s = store.CreateFile(nested); s.Close(); }

    EXPECT_THROW(store.DeleteDirectory(dir), System::IO::IsolatedStorage::IsolatedStorageException);
    EXPECT_TRUE(store.DirectoryExists(dir));
    EXPECT_TRUE(store.FileExists(nested));

    store.DeleteFile(nested);
    store.DeleteDirectory(dir);
}

TEST(IsolatedStorageFileTests, CopyFile_EmptyPath_ThrowsArgumentException) {
    auto store = IsolatedStorageFile::GetUserStoreForApplication();
    EXPECT_THROW(store.CopyFile("", "dst"), System::ArgumentException);
    EXPECT_THROW(store.CopyFile("src", ""), System::ArgumentException);
}

TEST(IsolatedStorageFileTests, MoveFile_EmptyPath_ThrowsArgumentException) {
    auto store = IsolatedStorageFile::GetUserStoreForApplication();
    EXPECT_THROW(store.MoveFile("", "dst"), System::ArgumentException);
    EXPECT_THROW(store.MoveFile("src", ""), System::ArgumentException);
}

TEST(IsolatedStorageFileTests, MoveDirectory_EmptyPath_ThrowsArgumentException) {
    auto store = IsolatedStorageFile::GetUserStoreForApplication();
    EXPECT_THROW(store.MoveDirectory("", "dst"), System::ArgumentException);
    EXPECT_THROW(store.MoveDirectory("src", ""), System::ArgumentException);
}

TEST(IsolatedStorageFileTests, Quota_IsLongMax) {
    auto store = IsolatedStorageFile::GetUserStoreForApplication();
    EXPECT_EQ(store.getQuotaProperty(), std::numeric_limits<SharpRuntime::longcs>::max());
}

TEST(IsolatedStorageFileTests, GetUserStoreForApplication_HasApplicationAndUserScope) {
    auto store = IsolatedStorageFile::GetUserStoreForApplication();
    EXPECT_EQ(store.getScopeProperty(),
              System::IO::IsolatedStorage::IsolatedStorageScope::Application |
              System::IO::IsolatedStorage::IsolatedStorageScope::User);
}

TEST(IsolatedStorageFileTests, GetUserStoreForAssembly_HasAssemblyAndUserScope) {
    auto store = IsolatedStorageFile::GetUserStoreForAssembly();
    EXPECT_EQ(store.getScopeProperty(),
              System::IO::IsolatedStorage::IsolatedStorageScope::Assembly |
              System::IO::IsolatedStorage::IsolatedStorageScope::User);
}

TEST(IsolatedStorageFileTests, IsAnIsolatedStorageBase_ViaVirtualDispatch) {
    // Regression: IsolatedStorageFile previously did not derive from IsolatedStorage at all.
    // Live disk free space can legitimately change between two calls under concurrent disk
    // activity, so this checks the virtual dispatch resolves to a sane value, not bit-for-bit
    // equality across two separate calls.
    IsolatedStorageFile store = IsolatedStorageFile::GetUserStoreForApplication();
    System::IO::IsolatedStorage::IsolatedStorage& base = store;
    EXPECT_GE(base.getAvailableFreeSpaceProperty(), 0);
    EXPECT_FALSE(base.IncreaseQuotaTo(1024));
}

// ===========================================================================
// RandomAccess
// ===========================================================================

#ifndef _WIN32
#include <fcntl.h>
#include <unistd.h>

TEST(RandomAccessTests, WriteThenRead_AtOffset_Roundtrip) {
    std::string p = tf("randomaccess_rw.bin");
    File::Delete(p);
    int fd = ::open(p.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0644);
    ASSERT_GE(fd, 0);

    uint8_t data[] = {10, 20, 30, 40};
    System::IO::RandomAccess::Write(fd, data, 4, 0);

    uint8_t readback[4] = {};
    intcs n = System::IO::RandomAccess::Read(fd, readback, 4, 0);
    EXPECT_EQ(n, 4);
    EXPECT_EQ(readback[0], 10u);
    EXPECT_EQ(readback[3], 40u);

    ::close(fd);
    File::Delete(p);
}

// Regression test for a wave-3 audit finding: RandomAccess::Write issued a single pwrite()
// call and silently discarded any bytes it didn't cover on a "short write" (pwrite() can
// legitimately write fewer bytes than requested), with no error and no way for the
// void-returning caller to detect the loss. Verified against RandomAccess.Unix.cs's
// WriteAtOffset, which loops until the whole buffer is written. A single pwrite() call is
// most likely to write fewer bytes than requested for a large buffer (pwrite() has no
// mandated minimum below which it must always complete in one call), so this write is
// intentionally large enough to exercise that path in practice, verifying every byte lands
// correctly rather than just asserting the total count.
TEST(RandomAccessTests, Write_LargeBuffer_AllBytesWrittenCorrectly) {
    std::string p = tf("randomaccess_large.bin");
    File::Delete(p);
    int fd = ::open(p.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0644);
    ASSERT_GE(fd, 0);

    constexpr size_t size = 8 * 1024 * 1024;
    std::vector<uint8_t> data(size);
    for (size_t i = 0; i < size; ++i) data[i] = static_cast<uint8_t>(i);
    System::IO::RandomAccess::Write(fd, data.data(), static_cast<intcs>(size), 0);

    EXPECT_EQ(System::IO::RandomAccess::GetLength(fd), static_cast<int64_t>(size));

    std::vector<uint8_t> readback(size);
    intcs totalRead = 0;
    while (static_cast<size_t>(totalRead) < size) {
        intcs n = System::IO::RandomAccess::Read(fd, readback.data() + totalRead,
                                                   static_cast<intcs>(size) - totalRead, totalRead);
        ASSERT_GT(n, 0);
        totalRead += n;
    }
    EXPECT_EQ(readback, data);

    ::close(fd);
    File::Delete(p);
}

TEST(RandomAccessTests, GetLength_ReflectsFileSize) {
    std::string p = tf("randomaccess_len.bin");
    File::WriteAllText(p, "abcde");
    int fd = ::open(p.c_str(), O_RDONLY);
    ASSERT_GE(fd, 0);

    EXPECT_EQ(System::IO::RandomAccess::GetLength(fd), 5);

    ::close(fd);
    File::Delete(p);
}

TEST(RandomAccessTests, SetLength_TruncatesFile) {
    std::string p = tf("randomaccess_setlen.bin");
    File::WriteAllText(p, "abcdefgh");
    int fd = ::open(p.c_str(), O_RDWR);
    ASSERT_GE(fd, 0);

    System::IO::RandomAccess::SetLength(fd, 3);
    EXPECT_EQ(System::IO::RandomAccess::GetLength(fd), 3);

    ::close(fd);
    File::Delete(p);
}

TEST(RandomAccessTests, Read_AtNonZeroOffset) {
    std::string p = tf("randomaccess_offset.bin");
    File::WriteAllText(p, "abcdef");
    int fd = ::open(p.c_str(), O_RDONLY);
    ASSERT_GE(fd, 0);

    uint8_t buf[3] = {};
    intcs n = System::IO::RandomAccess::Read(fd, buf, 3, 2);
    EXPECT_EQ(n, 3);
    EXPECT_EQ(buf[0], 'c');
    EXPECT_EQ(buf[1], 'd');
    EXPECT_EQ(buf[2], 'e');

    ::close(fd);
    File::Delete(p);
}

TEST(RandomAccessTests, FlushToDisk_NoThrow) {
    std::string p = tf("randomaccess_flush.bin");
    File::WriteAllText(p, "x");
    int fd = ::open(p.c_str(), O_RDWR);
    ASSERT_GE(fd, 0);

    EXPECT_NO_THROW(System::IO::RandomAccess::FlushToDisk(fd));

    ::close(fd);
    File::Delete(p);
}
#endif

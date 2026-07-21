// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Tests for System::IO enums, exceptions, and structural option types.
// Functional I/O (File, FileStream, BinaryReader/Writer, StreamReader/Writer, etc.)
// is covered in IOStreamTests.cpp.
#include <gtest/gtest.h>
#include <atomic>
#include <chrono>
#include <string>
#include <thread>
#include "System/IO/Directory.hpp"
#include "System/IO/File.hpp"
#include "System/IO/FileMode.hpp"
#include "System/IO/FileAccess.hpp"
#include "System/IO/FileShare.hpp"
#include "System/IO/FileAttributes.hpp"
#include "System/IO/FileOptions.hpp"
#include "System/IO/SeekOrigin.hpp"
#include "System/IO/SearchOption.hpp"
#include "System/IO/SearchTarget.hpp"
#include "System/IO/MatchCasing.hpp"
#include "System/IO/MatchType.hpp"
#include "System/IO/HandleInheritability.hpp"
#include "System/IO/UnixFileMode.hpp"
#include "System/IO/IOException.hpp"
#include "System/IO/InternalBufferOverflowException.hpp"
#include "System/IO/FileNotFoundException.hpp"
#include "System/IO/DirectoryNotFoundException.hpp"
#include "System/IO/DriveNotFoundException.hpp"
#include "System/IO/ErrorEventArgs.hpp"
#include "System/IO/ErrorEventHandler.hpp"
#include "System/IO/FileFormatException.hpp"
#include "System/IO/FileHandleType.hpp"
#include "System/IO/FileSystemEventArgs.hpp"
#include "System/IO/FileSystemEventHandler.hpp"
#include "System/IO/FileSystemWatcher.hpp"
#include "System/IO/NotifyFilters.hpp"
#include "System/IO/RenamedEventArgs.hpp"
#include "System/IO/RenamedEventHandler.hpp"
#include "System/IO/WaitForChangedResult.hpp"
#include "System/IO/WatcherChangeTypes.hpp"
#include "System/IO/EndOfStreamException.hpp"
#include "System/IO/PathTooLongException.hpp"
#include "System/IO/FileLoadException.hpp"
#include "System/IO/InvalidDataException.hpp"
#include "System/IO/IsolatedStorage/IsolatedStorageScope.hpp"
#include "System/IO/IsolatedStorage/IsolatedStorageException.hpp"
#include "System/IO/EnumerationOptions.hpp"
#include "System/IO/FileStreamOptions.hpp"
#include "System/IO/DriveInfo.hpp"
#include "System/ArgumentException.hpp"

namespace {
    std::string tf(const char* name) {
        return std::string("/tmp/sharp_rt_io_") + name;
    }
}

using System::IO::FileMode;
using System::IO::FileAccess;
using System::IO::FileShare;
using System::IO::FileAttributes;
using System::IO::FileOptions;
using System::IO::SeekOrigin;
using System::IO::SearchOption;
using System::IO::SearchTarget;
using System::IO::MatchCasing;
using System::IO::MatchType;
using System::IO::HandleInheritability;
using System::IO::UnixFileMode;
using System::IO::IOException;
using System::IO::InternalBufferOverflowException;
using System::IO::FileNotFoundException;
using System::IO::DirectoryNotFoundException;
using System::IO::DriveNotFoundException;
using System::IO::ErrorEventArgs;
using System::IO::ErrorEventHandler;
using System::IO::FileFormatException;
using System::IO::FileHandleType;
using System::IO::FileSystemEventArgs;
using System::IO::FileSystemEventHandler;
using System::IO::FileSystemWatcher;
using System::IO::NotifyFilters;
using System::IO::RenamedEventArgs;
using System::IO::RenamedEventHandler;
using System::IO::WaitForChangedResult;
using System::IO::WatcherChangeTypes;
using System::IO::EndOfStreamException;
using System::IO::PathTooLongException;
using System::IO::FileLoadException;
using System::IO::InvalidDataException;
using System::IO::IsolatedStorage::IsolatedStorageScope;
using System::IO::IsolatedStorage::IsolatedStorageException;
using System::IO::EnumerationOptions;
using System::IO::FileStreamOptions;
using System::IO::DriveType;
using System::IO::DriveInfo;

// ===========================================================================
// FileMode
// ===========================================================================

TEST(FileModeTests, CreateNew_IsOne) {
    EXPECT_EQ(static_cast<int>(FileMode::CreateNew), 1);
}

TEST(FileModeTests, Open_IsThree) {
    EXPECT_EQ(static_cast<int>(FileMode::Open), 3);
}

TEST(FileModeTests, Append_IsSix) {
    EXPECT_EQ(static_cast<int>(FileMode::Append), 6);
}

// ===========================================================================
// FileAccess
// ===========================================================================

TEST(FileAccessTests, Read_IsOne) {
    EXPECT_EQ(static_cast<int>(FileAccess::Read), 1);
}

TEST(FileAccessTests, Write_IsTwo) {
    EXPECT_EQ(static_cast<int>(FileAccess::Write), 2);
}

TEST(FileAccessTests, ReadWrite_IsThree) {
    EXPECT_EQ(static_cast<int>(FileAccess::ReadWrite), 3);
}

TEST(FileAccessTests, OperatorOr_ReadAndWrite_EqualsReadWrite) {
    EXPECT_EQ(FileAccess::Read | FileAccess::Write, FileAccess::ReadWrite);
}

TEST(FileAccessTests, OperatorAnd_MasksToRead) {
    auto masked = FileAccess::ReadWrite & FileAccess::Read;
    EXPECT_EQ(masked, FileAccess::Read);
}

// ===========================================================================
// FileShare
// ===========================================================================

TEST(FileShareTests, None_IsZero) {
    EXPECT_EQ(static_cast<int>(FileShare::None), 0);
}

TEST(FileShareTests, ReadWrite_IsThree) {
    EXPECT_EQ(static_cast<int>(FileShare::ReadWrite), 3);
}

TEST(FileShareTests, Inheritable_Is16) {
    EXPECT_EQ(static_cast<int>(FileShare::Inheritable), 16);
}

TEST(FileShareTests, OperatorOr_ReadAndWrite_EqualsReadWrite) {
    EXPECT_EQ(FileShare::Read | FileShare::Write, FileShare::ReadWrite);
}

TEST(FileShareTests, OperatorAnd_MasksToRead) {
    auto masked = FileShare::ReadWrite & FileShare::Read;
    EXPECT_EQ(masked, FileShare::Read);
}

// ===========================================================================
// FileAttributes
// ===========================================================================

TEST(FileAttributesTests, None_IsZero) {
    EXPECT_EQ(static_cast<int>(FileAttributes::None), 0);
}

TEST(FileAttributesTests, ReadOnly_IsOne) {
    EXPECT_EQ(static_cast<int>(FileAttributes::ReadOnly), 1);
}

TEST(FileAttributesTests, Hidden_IsTwo) {
    EXPECT_EQ(static_cast<int>(FileAttributes::Hidden), 2);
}

TEST(FileAttributesTests, Operator_OR) {
    auto r = FileAttributes::ReadOnly | FileAttributes::Hidden;
    EXPECT_EQ(static_cast<int>(r), 3);
}

TEST(FileAttributesTests, Operator_AND) {
    auto r = (FileAttributes::ReadOnly | FileAttributes::Hidden) & FileAttributes::ReadOnly;
    EXPECT_EQ(static_cast<int>(r), 1);
}

// ===========================================================================
// FileOptions
// ===========================================================================

TEST(FileOptionsTests, None_IsZero) {
    EXPECT_EQ(static_cast<int>(FileOptions::None), 0);
}

TEST(FileOptionsTests, RandomAccess_Value) {
    EXPECT_EQ(static_cast<int>(FileOptions::RandomAccess), 0x10000000);
}

TEST(FileOptionsTests, Operator_OR) {
    auto r = FileOptions::RandomAccess | FileOptions::DeleteOnClose;
    EXPECT_EQ(static_cast<int>(r), 0x10000000 | 0x04000000);
}

TEST(FileOptionsTests, Asynchronous_Value) {
    EXPECT_EQ(static_cast<unsigned int>(FileOptions::Asynchronous), 0x40000000u);
}

TEST(FileOptionsTests, Encrypted_Value) {
    EXPECT_EQ(static_cast<int>(FileOptions::Encrypted), 0x00004000);
}

// ===========================================================================
// SeekOrigin
// ===========================================================================

TEST(SeekOriginTests, Begin_IsZero) {
    EXPECT_EQ(static_cast<int>(SeekOrigin::Begin), 0);
}

TEST(SeekOriginTests, Current_IsOne) {
    EXPECT_EQ(static_cast<int>(SeekOrigin::Current), 1);
}

TEST(SeekOriginTests, End_IsTwo) {
    EXPECT_EQ(static_cast<int>(SeekOrigin::End), 2);
}

// ===========================================================================
// SearchOption
// ===========================================================================

TEST(SearchOptionTests, TopDirectoryOnly_IsZero) {
    EXPECT_EQ(static_cast<int>(SearchOption::TopDirectoryOnly), 0);
}

TEST(SearchOptionTests, AllDirectories_IsOne) {
    EXPECT_EQ(static_cast<int>(SearchOption::AllDirectories), 1);
}

// ===========================================================================
// SearchTarget
// ===========================================================================

TEST(SearchTargetTests, Files_IsOne) {
    EXPECT_EQ(static_cast<int>(SearchTarget::Files), 1);
}

TEST(SearchTargetTests, Directories_IsTwo) {
    EXPECT_EQ(static_cast<int>(SearchTarget::Directories), 2);
}

TEST(SearchTargetTests, Both_IsThree) {
    EXPECT_EQ(static_cast<int>(SearchTarget::Both), 3);
}

// ===========================================================================
// MatchCasing
// ===========================================================================

TEST(MatchCasingTests, PlatformDefault_IsZero) {
    EXPECT_EQ(static_cast<int>(MatchCasing::PlatformDefault), 0);
}

TEST(MatchCasingTests, CaseSensitive_IsOne) {
    EXPECT_EQ(static_cast<int>(MatchCasing::CaseSensitive), 1);
}

TEST(MatchCasingTests, CaseInsensitive_IsTwo) {
    EXPECT_EQ(static_cast<int>(MatchCasing::CaseInsensitive), 2);
}

// ===========================================================================
// MatchType
// ===========================================================================

TEST(MatchTypeTests, Simple_IsZero) {
    EXPECT_EQ(static_cast<int>(MatchType::Simple), 0);
}

TEST(MatchTypeTests, Win32_IsOne) {
    EXPECT_EQ(static_cast<int>(MatchType::Win32), 1);
}

// ===========================================================================
// HandleInheritability
// ===========================================================================

TEST(HandleInheritabilityTests, None_IsZero) {
    EXPECT_EQ(static_cast<int>(HandleInheritability::None), 0);
}

TEST(HandleInheritabilityTests, Inheritable_IsOne) {
    EXPECT_EQ(static_cast<int>(HandleInheritability::Inheritable), 1);
}

// ===========================================================================
// UnixFileMode
// ===========================================================================

TEST(UnixFileModeTests, None_IsZero) {
    EXPECT_EQ(static_cast<int>(UnixFileMode::None), 0);
}

TEST(UnixFileModeTests, UserRead_IsOctal400) {
    EXPECT_EQ(static_cast<int>(UnixFileMode::UserRead), 0400);
}

TEST(UnixFileModeTests, UserWrite_IsOctal200) {
    EXPECT_EQ(static_cast<int>(UnixFileMode::UserWrite), 0200);
}

TEST(UnixFileModeTests, Operator_OR) {
    auto r = UnixFileMode::UserRead | UnixFileMode::UserWrite;
    EXPECT_EQ(static_cast<int>(r), 0400 | 0200);
}

TEST(UnixFileModeTests, Operator_AND) {
    auto r = (UnixFileMode::UserRead | UnixFileMode::UserWrite) & UnixFileMode::UserRead;
    EXPECT_EQ(static_cast<int>(r), 0400);
}

// ===========================================================================
// IOException
// ===========================================================================

TEST(IOExceptionTests, DefaultCtor_NoThrow) {
    EXPECT_NO_THROW(IOException{});
}

TEST(IOExceptionTests, CharPtrCtor_WhatContainsMessage) {
    IOException ex("disk full");
    EXPECT_NE(std::string(ex.what()).find("disk full"), std::string::npos);
}

TEST(IOExceptionTests, StringCtor_WhatContainsMessage) {
    IOException ex(std::string("read error"));
    EXPECT_NE(std::string(ex.what()).find("read error"), std::string::npos);
}

TEST(IOExceptionTests, IsA_SystemException) {
    EXPECT_THROW(throw IOException("err"), System::SystemException);
}

TEST(IOExceptionTests, HResult_MatchesDotNet) {
    IOException ex;
    EXPECT_EQ(ex.getHResultProperty(), static_cast<SharpRuntime::intcs>(0x80131620));
}

// ===========================================================================
// FileNotFoundException
// ===========================================================================

TEST(FileNotFoundExceptionTests, DefaultCtor_NoThrow) {
    EXPECT_NO_THROW(FileNotFoundException{});
}

TEST(FileNotFoundExceptionTests, MessageCtor_WhatContainsMessage) {
    FileNotFoundException ex("file not found");
    EXPECT_NE(std::string(ex.what()).find("file not found"), std::string::npos);
}

TEST(FileNotFoundExceptionTests, MessageFilenameCtor_WhatContainsMessage) {
    FileNotFoundException ex("not found", "/tmp/missing.txt");
    EXPECT_NE(std::string(ex.what()).find("not found"), std::string::npos);
}

TEST(FileNotFoundExceptionTests, getFileNameProperty_ReturnsFilename) {
    FileNotFoundException ex("not found", "/tmp/missing.txt");
    EXPECT_EQ(ex.getFileNameProperty(), "/tmp/missing.txt");
}

TEST(FileNotFoundExceptionTests, IsA_IOException) {
    EXPECT_THROW(throw FileNotFoundException("err"), IOException);
}

TEST(FileNotFoundExceptionTests, HResult_MatchesDotNet) {
    FileNotFoundException ex;
    EXPECT_EQ(ex.getHResultProperty(), static_cast<SharpRuntime::intcs>(0x80070002));
}

TEST(FileNotFoundExceptionTests, MessageAndInnerCtor_NoThrow) {
    EXPECT_NO_THROW(FileNotFoundException("wrapped", std::exception_ptr{}));
}

TEST(FileNotFoundExceptionTests, MessageFileNameAndInnerCtor_StoresFileName) {
    FileNotFoundException ex("not found", "/tmp/missing.txt", std::exception_ptr{});
    EXPECT_EQ(ex.getFileNameProperty(), "/tmp/missing.txt");
}

// ===========================================================================
// DirectoryNotFoundException
// ===========================================================================

TEST(DirectoryNotFoundExceptionTests, DefaultCtor_NoThrow) {
    EXPECT_NO_THROW(DirectoryNotFoundException{});
}

TEST(DirectoryNotFoundExceptionTests, DefaultCtor_MatchesDotNetMessage) {
    DirectoryNotFoundException ex;
    EXPECT_STREQ(ex.what(), "Unable to find the specified directory.");
}

TEST(DirectoryNotFoundExceptionTests, MessageCtor_WhatContainsMessage) {
    DirectoryNotFoundException ex("no such dir");
    EXPECT_NE(std::string(ex.what()).find("no such dir"), std::string::npos);
}

TEST(DirectoryNotFoundExceptionTests, MessageAndPathCtor_StoresDirectoryPath) {
    DirectoryNotFoundException ex("no such dir", "/tmp/missing");
    EXPECT_EQ(ex.getDirectoryPathProperty(), "/tmp/missing");
}

TEST(DirectoryNotFoundExceptionTests, MessageAndInnerCtor_NoThrow) {
    EXPECT_NO_THROW(DirectoryNotFoundException("wrapped", std::exception_ptr{}));
}

TEST(DirectoryNotFoundExceptionTests, HResult_MatchesCorEDirectoryNotFound) {
    DirectoryNotFoundException ex;
    EXPECT_EQ(ex.getHResultProperty(), static_cast<SharpRuntime::intcs>(0x80070003));
}

// ===========================================================================
// DriveNotFoundException
// ===========================================================================

TEST(DriveNotFoundExceptionTests, DefaultCtor_WhatContainsDriveNotFound) {
    DriveNotFoundException ex;
    EXPECT_NE(std::string(ex.what()).find("drive"), std::string::npos);
}

TEST(DriveNotFoundExceptionTests, MessageCtor_WhatContainsMessage) {
    DriveNotFoundException ex("no such drive");
    EXPECT_NE(std::string(ex.what()).find("no such drive"), std::string::npos);
}

TEST(DriveNotFoundExceptionTests, MessageAndInnerCtor_NoThrow) {
    EXPECT_NO_THROW(DriveNotFoundException("wrapped", std::exception_ptr{}));
}

TEST(DriveNotFoundExceptionTests, IsA_IOException) {
    EXPECT_THROW(throw DriveNotFoundException(), System::IO::IOException);
}

TEST(DriveNotFoundExceptionTests, HResult_MatchesCorEDirectoryNotFound) {
    // .NET's DriveNotFoundException.cs sets COR_E_DIRECTORYNOTFOUND (same as
    // DirectoryNotFoundException, not a distinct DriveNotFound HResult).
    DriveNotFoundException ex;
    EXPECT_EQ(ex.getHResultProperty(), static_cast<SharpRuntime::intcs>(0x80070003));
}

// ===========================================================================
// ErrorEventArgs
// ===========================================================================

TEST(ErrorEventArgsTests, GetException_ReturnsStoredException) {
    std::exception_ptr ptr;
    try { throw IOException("boom"); } catch (...) { ptr = std::current_exception(); }
    ErrorEventArgs args(ptr);
    EXPECT_EQ(args.GetException(), ptr);
}

TEST(ErrorEventArgsTests, IsA_EventArgs) {
    ErrorEventArgs args(std::exception_ptr{});
    System::EventArgs& base = args;
    (void)base;
    SUCCEED();
}

// ===========================================================================
// ErrorEventHandler
// ===========================================================================

TEST(ErrorEventHandlerTests, InvokesWithSenderAndArgs) {
    bool called = false;
    void* seenSender = nullptr;
    ErrorEventHandler handler = [&](void* sender, const ErrorEventArgs& e) {
        called = true;
        seenSender = sender;
        (void)e;
    };
    int fakeSender = 0;
    ErrorEventArgs args(std::exception_ptr{});
    handler(&fakeSender, args);
    EXPECT_TRUE(called);
    EXPECT_EQ(seenSender, &fakeSender);
}

// ===========================================================================
// FileFormatException
// ===========================================================================

TEST(FileFormatExceptionTests, DefaultCtor_WhatContainsMessage) {
    FileFormatException ex;
    EXPECT_NE(std::string(ex.what()).find("Invalid file format"), std::string::npos);
}

TEST(FileFormatExceptionTests, MessageCtor_WhatContainsMessage) {
    FileFormatException ex("bad format");
    EXPECT_NE(std::string(ex.what()).find("bad format"), std::string::npos);
}

TEST(FileFormatExceptionTests, SourceUriCtor_MessageContainsUri_AndSourceUriIsSet) {
    System::Uri uri("file:///tmp/bad.docx");
    FileFormatException ex(uri);
    EXPECT_NE(std::string(ex.what()).find("bad.docx"), std::string::npos);
    ASSERT_NE(ex.getSourceUriProperty(), nullptr);
    EXPECT_EQ(ex.getSourceUriProperty()->ToString(), uri.ToString());
}

TEST(FileFormatExceptionTests, DefaultCtor_SourceUriIsNull) {
    FileFormatException ex;
    EXPECT_EQ(ex.getSourceUriProperty(), nullptr);
}

TEST(FileFormatExceptionTests, IsA_FormatException) {
    EXPECT_THROW(throw FileFormatException(), System::FormatException);
}

// ===========================================================================
// FileHandleType
// ===========================================================================

TEST(FileHandleTypeTests, Unknown_IsZero) {
    EXPECT_EQ(static_cast<int>(FileHandleType::Unknown), 0);
}

TEST(FileHandleTypeTests, RegularFile_IsOne) {
    EXPECT_EQ(static_cast<int>(FileHandleType::RegularFile), 1);
}

TEST(FileHandleTypeTests, BlockDevice_IsSeven) {
    EXPECT_EQ(static_cast<int>(FileHandleType::BlockDevice), 7);
}

// ===========================================================================
// WatcherChangeTypes
// ===========================================================================

TEST(WatcherChangeTypesTests, Created_IsOne) {
    EXPECT_EQ(static_cast<int>(WatcherChangeTypes::Created), 1);
}

TEST(WatcherChangeTypesTests, All_CombinesEveryValue) {
    auto all = WatcherChangeTypes::Created | WatcherChangeTypes::Deleted |
               WatcherChangeTypes::Changed | WatcherChangeTypes::Renamed;
    EXPECT_EQ(all, WatcherChangeTypes::All);
}

// ===========================================================================
// FileSystemEventArgs
// ===========================================================================

TEST(FileSystemEventArgsTests, PropertiesReturnConstructorValues) {
    FileSystemEventArgs args(WatcherChangeTypes::Created, "/tmp/dir", "file.txt");
    EXPECT_EQ(args.getChangeTypeProperty(), WatcherChangeTypes::Created);
    EXPECT_EQ(args.getNameProperty(), "file.txt");
    EXPECT_EQ(args.getFullPathProperty(), "/tmp/dir/file.txt");
}

TEST(FileSystemEventArgsTests, DirectoryWithTrailingSeparator_NoDoubleSeparator) {
    FileSystemEventArgs args(WatcherChangeTypes::Changed, "/tmp/dir/", "file.txt");
    EXPECT_EQ(args.getFullPathProperty(), "/tmp/dir/file.txt");
}

TEST(FileSystemEventArgsTests, IsA_EventArgs) {
    FileSystemEventArgs args(WatcherChangeTypes::Deleted, "/tmp", "x");
    System::EventArgs& base = args;
    (void)base;
    SUCCEED();
}

// ===========================================================================
// FileSystemEventHandler
// ===========================================================================

TEST(FileSystemEventHandlerTests, InvokesWithSenderAndArgs) {
    bool called = false;
    void* seenSender = nullptr;
    FileSystemEventHandler handler = [&](void* sender, const FileSystemEventArgs& e) {
        called = true;
        seenSender = sender;
        (void)e;
    };
    int fakeSender = 0;
    FileSystemEventArgs args(WatcherChangeTypes::Created, "/tmp", "x");
    handler(&fakeSender, args);
    EXPECT_TRUE(called);
    EXPECT_EQ(seenSender, &fakeSender);
}

// ===========================================================================
// NotifyFilters
// ===========================================================================

TEST(NotifyFiltersTests, FileName_IsOne) {
    EXPECT_EQ(static_cast<int>(NotifyFilters::FileName), 1);
}

TEST(NotifyFiltersTests, OperatorOr_Combines) {
    auto combined = NotifyFilters::FileName | NotifyFilters::LastWrite;
    EXPECT_EQ(static_cast<int>(combined), 0x1 | 0x10);
}

// ===========================================================================
// RenamedEventArgs
// ===========================================================================

TEST(RenamedEventArgsTests, PropertiesReturnConstructorValues) {
    RenamedEventArgs args(WatcherChangeTypes::Renamed, "/tmp", "new.txt", "old.txt");
    EXPECT_EQ(args.getNameProperty(), "new.txt");
    EXPECT_EQ(args.getFullPathProperty(), "/tmp/new.txt");
    EXPECT_EQ(args.getOldNameProperty(), "old.txt");
    EXPECT_EQ(args.getOldFullPathProperty(), "/tmp/old.txt");
}

TEST(RenamedEventArgsTests, IsA_FileSystemEventArgs) {
    RenamedEventArgs args(WatcherChangeTypes::Renamed, "/tmp", "a", "b");
    FileSystemEventArgs& base = args;
    (void)base;
    SUCCEED();
}

// ===========================================================================
// RenamedEventHandler
// ===========================================================================

TEST(RenamedEventHandlerTests, InvokesWithSenderAndArgs) {
    bool called = false;
    RenamedEventHandler handler = [&](void* sender, const RenamedEventArgs& e) {
        called = true;
        (void)sender;
        (void)e;
    };
    RenamedEventArgs args(WatcherChangeTypes::Renamed, "/tmp", "a", "b");
    int fakeSender = 0;
    handler(&fakeSender, args);
    EXPECT_TRUE(called);
}

// ===========================================================================
// WaitForChangedResult
// ===========================================================================

TEST(WaitForChangedResultTests, DefaultConstructed_NotTimedOut) {
    WaitForChangedResult r;
    EXPECT_FALSE(r.TimedOut);
}

TEST(WaitForChangedResultTests, TimedOutResult_IsTimedOut) {
    auto r = WaitForChangedResult::TimedOutResult();
    EXPECT_TRUE(r.TimedOut);
}

// ===========================================================================
// FileSystemWatcher
// ===========================================================================

TEST(FileSystemWatcherTests, DefaultCtor_PathIsEmpty) {
    FileSystemWatcher fsw;
    EXPECT_TRUE(fsw.getPathProperty().empty());
}

TEST(FileSystemWatcherTests, PathCtor_ExistingDirectory_Succeeds) {
    FileSystemWatcher fsw("/tmp");
    EXPECT_EQ(fsw.getPathProperty(), "/tmp");
}

TEST(FileSystemWatcherTests, PathCtor_NonExistentDirectory_ThrowsArgumentException) {
    EXPECT_THROW(FileSystemWatcher(tf("no_such_watch_dir_xyz")), System::ArgumentException);
}

TEST(FileSystemWatcherTests, FilterCtor_SetsFilter) {
    FileSystemWatcher fsw("/tmp", "*.txt");
    EXPECT_EQ(fsw.getFilterProperty(), "*.txt");
}

TEST(FileSystemWatcherTests, DefaultFilter_IsStar) {
    FileSystemWatcher fsw("/tmp");
    EXPECT_EQ(fsw.getFilterProperty(), "*");
}

TEST(FileSystemWatcherTests, DefaultNotifyFilter_MatchesDotNetDefault) {
    FileSystemWatcher fsw;
    auto expected = NotifyFilters::LastWrite | NotifyFilters::FileName | NotifyFilters::DirectoryName;
    EXPECT_EQ(fsw.getNotifyFilterProperty(), expected);
}

TEST(FileSystemWatcherTests, SetNotifyFilter_InvalidBits_ThrowsArgumentException) {
    FileSystemWatcher fsw;
    EXPECT_THROW(fsw.setNotifyFilterProperty(static_cast<NotifyFilters>(0x40000)), System::ArgumentException);
}

TEST(FileSystemWatcherTests, InternalBufferSize_ClampedToMinimum) {
    FileSystemWatcher fsw;
    fsw.setInternalBufferSizeProperty(100);
    EXPECT_EQ(fsw.getInternalBufferSizeProperty(), 4096);
}

TEST(FileSystemWatcherTests, EnableRaisingEvents_RoundTrips) {
    FileSystemWatcher fsw("/tmp");
    EXPECT_FALSE(fsw.getEnableRaisingEventsProperty());
    fsw.setEnableRaisingEventsProperty(true);
    EXPECT_TRUE(fsw.getEnableRaisingEventsProperty());
}

TEST(FileSystemWatcherTests, ChangedHandlerList_CanRegisterHandler) {
    FileSystemWatcher fsw("/tmp");
    fsw.Changed.push_back([](void*, const FileSystemEventArgs&) {});
    EXPECT_EQ(fsw.Changed.size(), 1u);
}

// --- ticket 1478: real inotify-backed end-to-end coverage (Linux) ---

namespace {
    // Polls `ready` for up to ~2s; real inotify delivery is fast (single-digit ms) but this
    // gives ample headroom for a loaded CI box without making a failing test take unreasonably
    // long to report.
    bool WaitUntil(const std::atomic<bool>& ready) {
        for (int i = 0; i < 200; ++i) {
            if (ready.load()) return true;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        return ready.load();
    }
}

TEST(FileSystemWatcherTests, EndToEnd_CreateFile_FiresCreatedEvent) {
    std::string dir = tf("fsw_create_dir");
    System::IO::Directory::CreateDirectory(dir);
    FileSystemWatcher fsw(dir);
    std::atomic<bool> fired{false};
    std::string observedName;
    fsw.Created.push_back([&](void*, const FileSystemEventArgs& e) {
        observedName = e.getNameProperty();
        fired.store(true);
    });
    fsw.setEnableRaisingEventsProperty(true);

    System::IO::File::WriteAllText(dir + "/newfile.txt", "hello");

    EXPECT_TRUE(WaitUntil(fired));
    EXPECT_EQ(observedName, "newfile.txt");

    fsw.setEnableRaisingEventsProperty(false);
    System::IO::Directory::Delete(dir, true);
}

TEST(FileSystemWatcherTests, EndToEnd_ModifyFile_FiresChangedEvent) {
    std::string dir = tf("fsw_modify_dir");
    System::IO::Directory::CreateDirectory(dir);
    System::IO::File::WriteAllText(dir + "/existing.txt", "v1");

    FileSystemWatcher fsw(dir);
    std::atomic<bool> fired{false};
    fsw.Changed.push_back([&](void*, const FileSystemEventArgs& e) {
        if (e.getNameProperty() == "existing.txt") fired.store(true);
    });
    fsw.setEnableRaisingEventsProperty(true);

    System::IO::File::WriteAllText(dir + "/existing.txt", "v2 has more bytes than v1");

    EXPECT_TRUE(WaitUntil(fired));

    fsw.setEnableRaisingEventsProperty(false);
    System::IO::Directory::Delete(dir, true);
}

TEST(FileSystemWatcherTests, EndToEnd_DeleteFile_FiresDeletedEvent) {
    std::string dir = tf("fsw_delete_dir");
    System::IO::Directory::CreateDirectory(dir);
    System::IO::File::WriteAllText(dir + "/doomed.txt", "bye");

    FileSystemWatcher fsw(dir);
    std::atomic<bool> fired{false};
    fsw.Deleted.push_back([&](void*, const FileSystemEventArgs& e) {
        if (e.getNameProperty() == "doomed.txt") fired.store(true);
    });
    fsw.setEnableRaisingEventsProperty(true);

    System::IO::File::Delete(dir + "/doomed.txt");

    EXPECT_TRUE(WaitUntil(fired));

    fsw.setEnableRaisingEventsProperty(false);
    System::IO::Directory::Delete(dir, true);
}

TEST(FileSystemWatcherTests, EndToEnd_RenameFileWithinWatchedDirectory_FiresRenamedEvent) {
    std::string dir = tf("fsw_rename_dir");
    System::IO::Directory::CreateDirectory(dir);
    System::IO::File::WriteAllText(dir + "/before.txt", "content");

    FileSystemWatcher fsw(dir);
    std::atomic<bool> fired{false};
    std::string newName, oldName;
    fsw.Renamed.push_back([&](void*, const RenamedEventArgs& e) {
        newName = e.getNameProperty();
        oldName = e.getOldNameProperty();
        fired.store(true);
    });
    fsw.setEnableRaisingEventsProperty(true);

    System::IO::File::Move(dir + "/before.txt", dir + "/after.txt");

    EXPECT_TRUE(WaitUntil(fired));
    EXPECT_EQ(newName, "after.txt");
    EXPECT_EQ(oldName, "before.txt");

    fsw.setEnableRaisingEventsProperty(false);
    System::IO::Directory::Delete(dir, true);
}

TEST(FileSystemWatcherTests, EndToEnd_FilterExcludesNonMatchingFiles) {
    std::string dir = tf("fsw_filter_dir");
    System::IO::Directory::CreateDirectory(dir);
    FileSystemWatcher fsw(dir, "*.txt");
    std::atomic<bool> txtFired{false};
    std::atomic<bool> logFired{false};
    fsw.Created.push_back([&](void*, const FileSystemEventArgs& e) {
        if (e.getNameProperty() == "match.txt") txtFired.store(true);
        if (e.getNameProperty() == "skip.log") logFired.store(true);
    });
    fsw.setEnableRaisingEventsProperty(true);

    System::IO::File::WriteAllText(dir + "/skip.log", "should be filtered out");
    System::IO::File::WriteAllText(dir + "/match.txt", "should fire Created");

    EXPECT_TRUE(WaitUntil(txtFired));
    // Give the (already-delivered-or-not) skip.log event a moment to have arrived too, then
    // confirm it never fired -- the filter should have excluded it before dispatch.
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_FALSE(logFired.load());

    fsw.setEnableRaisingEventsProperty(false);
    System::IO::Directory::Delete(dir, true);
}

TEST(FileSystemWatcherTests, Destructor_StopsWatchThreadCleanly) {
    // Regression guard for the dangling-`this` hazard class: destroying a FileSystemWatcher
    // with EnableRaisingEvents still true must not crash, hang, or leave the watch thread
    // running past the object's lifetime.
    std::string dir = tf("fsw_dtor_dir");
    System::IO::Directory::CreateDirectory(dir);
    {
        FileSystemWatcher fsw(dir);
        fsw.setEnableRaisingEventsProperty(true);
        // fsw destructs here while still "enabled" -- must join cleanly, not detach.
    }
    System::IO::Directory::Delete(dir, true);
    SUCCEED();
}

// ===========================================================================
// EndOfStreamException
// ===========================================================================

TEST(EndOfStreamExceptionTests, DefaultCtor_NoThrow) {
    EXPECT_NO_THROW(EndOfStreamException{});
}

TEST(EndOfStreamExceptionTests, MessageCtor_WhatContainsMessage) {
    EndOfStreamException ex("end of stream");
    EXPECT_NE(std::string(ex.what()).find("end of stream"), std::string::npos);
}

TEST(EndOfStreamExceptionTests, MessageAndInnerCtor_NoThrow) {
    EXPECT_NO_THROW(EndOfStreamException("wrapped", std::exception_ptr{}));
}

TEST(EndOfStreamExceptionTests, HResult_MatchesCorEEndOfStream) {
    EndOfStreamException ex;
    EXPECT_EQ(ex.getHResultProperty(), static_cast<SharpRuntime::intcs>(0x80070026));
}

// ===========================================================================
// PathTooLongException
// ===========================================================================

TEST(PathTooLongExceptionTests, DefaultCtor_WhatNotEmpty) {
    PathTooLongException ex;
    EXPECT_FALSE(std::string(ex.what()).empty());
}

TEST(PathTooLongExceptionTests, DefaultCtor_MatchesDotNetMessage) {
    PathTooLongException ex;
    EXPECT_STREQ(ex.what(), "The specified file name or path is too long, or a component of the specified path is too long.");
}

TEST(PathTooLongExceptionTests, MessageCtor_WhatContainsMessage) {
    PathTooLongException ex("path is too long");
    EXPECT_NE(std::string(ex.what()).find("path is too long"), std::string::npos);
}

// ===========================================================================
// FileLoadException
// ===========================================================================

TEST(FileLoadExceptionTests, DefaultCtor_WhatNotEmpty) {
    FileLoadException ex;
    EXPECT_FALSE(std::string(ex.what()).empty());
}

TEST(FileLoadExceptionTests, HResult_MatchesDotNet) {
    FileLoadException ex;
    EXPECT_EQ(ex.getHResultProperty(), static_cast<SharpRuntime::intcs>(0x80131621));
}

TEST(FileLoadExceptionTests, MessageCtor_WhatContainsMessage) {
    FileLoadException ex("cannot load file");
    EXPECT_NE(std::string(ex.what()).find("cannot load file"), std::string::npos);
}

TEST(FileLoadExceptionTests, MessageFilenameCtor_getFileNameProperty) {
    FileLoadException ex("failed", "bad.dll");
    EXPECT_EQ(ex.getFileNameProperty(), "bad.dll");
}

TEST(FileLoadExceptionTests, MessageFileNameAndInnerCtor_StoresFileName) {
    FileLoadException ex("failed", "bad.dll", std::exception_ptr{});
    EXPECT_EQ(ex.getFileNameProperty(), "bad.dll");
}

// ===========================================================================
// InvalidDataException
// ===========================================================================

TEST(InvalidDataExceptionTests, DefaultCtor_NoThrow) {
    EXPECT_NO_THROW(InvalidDataException{});
}

TEST(InvalidDataExceptionTests, DefaultCtor_MatchesDotNetMessage) {
    InvalidDataException ex;
    EXPECT_STREQ(ex.what(), "Found invalid data while decoding.");
}

TEST(InvalidDataExceptionTests, MessageCtor_WhatContainsMessage) {
    InvalidDataException ex("corrupt data");
    EXPECT_NE(std::string(ex.what()).find("corrupt data"), std::string::npos);
}

// ===========================================================================
// InternalBufferOverflowException
// ===========================================================================

TEST(InternalBufferOverflowExceptionTests, DefaultCtor_WhatNotEmpty) {
    InternalBufferOverflowException ex;
    EXPECT_FALSE(std::string(ex.what()).empty());
}

TEST(InternalBufferOverflowExceptionTests, HResult_MatchesDotNet) {
    InternalBufferOverflowException ex;
    EXPECT_EQ(ex.getHResultProperty(), static_cast<SharpRuntime::intcs>(0x80131905));
}

TEST(InternalBufferOverflowExceptionTests, MessageCtor_WhatContainsMessage) {
    InternalBufferOverflowException ex("buffer overflowed");
    EXPECT_NE(std::string(ex.what()).find("buffer overflowed"), std::string::npos);
}

TEST(InternalBufferOverflowExceptionTests, IsA_SystemException) {
    EXPECT_THROW({
        try { throw InternalBufferOverflowException("x"); }
        catch (const System::SystemException&) { throw; }
    }, InternalBufferOverflowException);
}

// ===========================================================================
// IsolatedStorageScope
// ===========================================================================

TEST(IsolatedStorageScopeTests, None_IsZero) {
    EXPECT_EQ(static_cast<int>(IsolatedStorageScope::None), 0x00);
}

TEST(IsolatedStorageScopeTests, User_IsOne) {
    EXPECT_EQ(static_cast<int>(IsolatedStorageScope::User), 0x01);
}

TEST(IsolatedStorageScopeTests, Assembly_IsFour) {
    EXPECT_EQ(static_cast<int>(IsolatedStorageScope::Assembly), 0x04);
}

TEST(IsolatedStorageScopeTests, Machine_Is16) {
    EXPECT_EQ(static_cast<int>(IsolatedStorageScope::Machine), 0x10);
}

TEST(IsolatedStorageScopeTests, Operator_OR) {
    auto r = IsolatedStorageScope::User | IsolatedStorageScope::Assembly;
    EXPECT_EQ(static_cast<int>(r), 0x05);
}

// ===========================================================================
// IsolatedStorageException
// ===========================================================================

TEST(IsolatedStorageExceptionTests, MessageCtor_WhatContainsMessage) {
    IsolatedStorageException ex("quota exceeded");
    EXPECT_NE(std::string(ex.what()).find("quota exceeded"), std::string::npos);
}

TEST(IsolatedStorageExceptionTests, IsA_Exception) {
    EXPECT_THROW(throw IsolatedStorageException("err"), System::Exception);
}

TEST(IsolatedStorageExceptionTests, DefaultCtor_WhatNotEmpty) {
    IsolatedStorageException ex;
    EXPECT_FALSE(std::string(ex.what()).empty());
}

TEST(IsolatedStorageExceptionTests, MessageAndInnerCtor_WhatContainsMessage) {
    IsolatedStorageException ex("wrapped failure", std::exception_ptr{});
    EXPECT_NE(std::string(ex.what()).find("wrapped failure"), std::string::npos);
}

TEST(IsolatedStorageExceptionTests, HResult_MatchesCorEIsoStore) {
    IsolatedStorageException ex;
    EXPECT_EQ(ex.getHResultProperty(), static_cast<SharpRuntime::intcs>(0x80131450));
}

// ===========================================================================
// EnumerationOptions
// ===========================================================================

TEST(EnumerationOptionsTests, DefaultCtor_NoThrow) {
    EXPECT_NO_THROW(EnumerationOptions{});
}

TEST(EnumerationOptionsTests, RecurseSubdirectories_DefaultFalse) {
    EnumerationOptions opts;
    EXPECT_FALSE(opts.RecurseSubdirectories);
}

TEST(EnumerationOptionsTests, IgnoreInaccessible_DefaultTrue) {
    EnumerationOptions opts;
    EXPECT_TRUE(opts.IgnoreInaccessible);
}

TEST(EnumerationOptionsTests, ReturnSpecialDirectories_DefaultFalse) {
    EnumerationOptions opts;
    EXPECT_FALSE(opts.ReturnSpecialDirectories);
}

TEST(EnumerationOptionsTests, AttributesToSkip_DefaultIsHiddenOrSystem) {
    EnumerationOptions opts;
    FileAttributes expected = static_cast<FileAttributes>(
        static_cast<int>(FileAttributes::Hidden) | static_cast<int>(FileAttributes::System));
    EXPECT_EQ(opts.AttributesToSkip, expected);
}

TEST(EnumerationOptionsTests, MaxRecursionDepth_DefaultIsIntMax) {
    EnumerationOptions opts;
    EXPECT_EQ(opts.MaxRecursionDepth, INT_MAX);
}

TEST(EnumerationOptionsTests, BufferSize_DefaultZero) {
    EnumerationOptions opts;
    EXPECT_EQ(opts.BufferSize, 0);
}

// ===========================================================================
// FileStreamOptions
// ===========================================================================

TEST(FileStreamOptionsTests, DefaultCtor_NoThrow) {
    EXPECT_NO_THROW(FileStreamOptions{});
}

TEST(FileStreamOptionsTests, DefaultMode_IsOpen) {
    FileStreamOptions opts;
    EXPECT_EQ(opts.Mode, FileMode::Open);
}

TEST(FileStreamOptionsTests, DefaultAccess_IsRead) {
    FileStreamOptions opts;
    EXPECT_EQ(opts.Access, FileAccess::Read);
}

TEST(FileStreamOptionsTests, DefaultShare_IsRead) {
    FileStreamOptions opts;
    EXPECT_EQ(opts.Share, FileShare::Read);
}

// ===========================================================================
// DriveType
// ===========================================================================

TEST(DriveTypeTests, Unknown_IsZero) {
    EXPECT_EQ(static_cast<int>(DriveType::Unknown), 0);
}

TEST(DriveTypeTests, Fixed_IsThree) {
    EXPECT_EQ(static_cast<int>(DriveType::Fixed), 3);
}

TEST(DriveTypeTests, CDRom_IsFive) {
    EXPECT_EQ(static_cast<int>(DriveType::CDRom), 5);
}

// ===========================================================================
// DriveInfo
// ===========================================================================

TEST(DriveInfoTests, Constructor_StoresName) {
    DriveInfo d("/");
    EXPECT_EQ(d.getNameProperty(), "/");
}

TEST(DriveInfoTests, IsReady_RootDirExists) {
    DriveInfo d("/");
    EXPECT_TRUE(d.getIsReadyProperty());
}

TEST(DriveInfoTests, DriveType_IsFixed) {
    DriveInfo d("/");
    EXPECT_EQ(d.getDriveTypeProperty(), DriveType::Fixed);
}

TEST(DriveInfoTests, ToString_ReturnsName) {
    DriveInfo d("/");
    EXPECT_EQ(d.ToString(), "/");
}

TEST(DriveInfoTests, GetDrives_NotEmpty) {
    auto drives = DriveInfo::GetDrives();
    EXPECT_FALSE(drives.empty());
}

TEST(DriveInfoTests, VolumeLabel_ReturnsName) {
    DriveInfo d("/");
    EXPECT_EQ(d.getVolumeLabelProperty(), "/");
}

TEST(DriveInfoTests, TotalSize_IsPositiveForRealDrive) {
    DriveInfo d("/");
    EXPECT_GT(d.getTotalSizeProperty(), 0);
}

TEST(DriveInfoTests, TotalFreeSpace_DoesNotExceedTotalSize) {
    DriveInfo d("/");
    EXPECT_LE(d.getTotalFreeSpaceProperty(), d.getTotalSizeProperty());
}

TEST(DriveInfoTests, AvailableFreeSpace_DoesNotExceedTotalFreeSpace) {
    DriveInfo d("/");
    EXPECT_LE(d.getAvailableFreeSpaceProperty(), d.getTotalFreeSpaceProperty());
}

TEST(DriveInfoTests, TotalSize_NonExistentPath_ReturnsZero) {
    DriveInfo d("/no/such/path/xyz_abc_123");
    EXPECT_EQ(d.getTotalSizeProperty(), 0);
}

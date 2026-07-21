// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>
#include <string>
#include <vector>
#include "System/IO/Stream.hpp"
#include "System/IO/Compression/CompressionLevel.hpp"

namespace System::IO::Compression {

    /** @brief Specifies values for interacting with zip archive entries. */
    enum class ZipArchiveMode { Read = 0, Create = 1, Update = 2 };

    struct ZipArchiveEntryState; ///< Opaque miniz state; defined in ZipArchive.cpp.
    struct ZipArchiveState;      ///< Opaque miniz state; defined in ZipArchive.cpp.

    /**
     * @brief Represents a single entry (file) inside a zip archive.
     *
     * In read mode, @c Open() returns a @c MemoryStream with the decompressed
     * entry contents.  In create/update mode, @c Open() returns a writable
     * stream; data written to it is stored when the owning @c ZipArchive is
     * disposed.
     *
     * Partial C++ counterpart of .NET System.IO.Compression.ZipArchiveEntry.
     *
     * @note Status: IMPLEMENTED — backed by vendored miniz.
     */
    class ZipArchiveEntry {
        std::shared_ptr<ZipArchiveEntryState> state_;

    public:
        /** @brief Default-constructs an invalid (empty) entry. */
        ZipArchiveEntry() = default;

        /** @brief Internal constructor used by ZipArchive; prefer the archive's factory methods. */
        explicit ZipArchiveEntry(std::shared_ptr<ZipArchiveEntryState> s);

        ~ZipArchiveEntry() = default;

        /** @brief Returns @c true if this entry was successfully obtained from an archive. */
        [[nodiscard]] bool IsValid() const;

        /** @brief Returns the file name of the entry (without directory path). */
        [[nodiscard]] std::string getNameProperty() const;

        /** @brief Returns the full relative path of the entry within the archive. */
        [[nodiscard]] std::string getFullNameProperty() const;

        /** @brief Returns the uncompressed size in bytes. */
        [[nodiscard]] long long getLengthProperty() const;

        /**
         * @brief Opens the entry for reading or writing.
         *
         * In read mode the caller receives a heap-allocated @c MemoryStream
         * containing the decompressed data; the caller owns the pointer.
         *
         * In create/update mode the caller receives a writable stream; data
         * written to it is committed when the owning @c ZipArchive is disposed.
         * The caller owns the pointer.
         *
         * @return Heap-allocated @c Stream; caller is responsible for deletion.
         */
        System::IO::Stream* Open();

        /**
         * @brief Removes this entry from the archive (update mode only).
         *
         * @throws System::NotSupportedException if the archive is not in update mode.
         */
        void Delete();
    };

    /**
     * @brief Represents a zip archive that can be opened for reading, creating, or updating.
     *
     * Backed by vendored miniz.  The stream-based constructor reads the entire
     * stream into memory and uses miniz's in-memory reader.
     *
     * Partial C++ counterpart of .NET System.IO.Compression.ZipArchive.
     *
     * @note Status: IMPLEMENTED — backed by vendored miniz.
     *   Read mode is fully supported; Create mode supports adding entries via
     *   @c CreateEntry() followed by writing to the stream returned from
     *   @c ZipArchiveEntry::Open(), committed on @c Dispose().
     */
    class ZipArchive {
        std::shared_ptr<ZipArchiveState> state_;

    public:
        /**
         * @brief Opens or creates a zip archive backed by @p stream.
         *
         * In Read/Update mode the stream contents are copied to an internal buffer at
         * construction time. In Create/Update mode, @p stream is retained (non-owning -- the
         * caller must keep it alive for this ZipArchive's lifetime) and the finalized archive
         * is written back to it on Dispose(). If @p stream supports seeking
         * (getCanSeekProperty()), the write-back correctly repositions to the start and
         * truncates to the new size first; a non-seekable stream can only be appended to, which
         * is only correct for a fresh Create-mode stream with nothing already written to it.
         *
         * @param stream     Source or destination stream. Not owned/closed by this ZipArchive.
         * @param mode       Read, Create, or Update.
         * @throws System::IO::InvalidDataException or System::IO::IOException on initialisation failure.
         */
        ZipArchive(System::IO::Stream* stream, ZipArchiveMode mode = ZipArchiveMode::Read);

        /**
         * @brief Opens or creates a zip archive at @p archivePath.
         *
         * @param archivePath  File-system path.
         * @param mode         Read, Create, or Update.
         * @throws System::IO::InvalidDataException or System::IO::IOException on initialisation failure.
         */
        ZipArchive(const std::string& archivePath, ZipArchiveMode mode = ZipArchiveMode::Read);

        ~ZipArchive();

        /**
         * @brief Returns all entries in the archive.
         *
         * Only valid in Read or Update mode.
         */
        [[nodiscard]] std::vector<ZipArchiveEntry> getEntriesProperty() const;

        /**
         * @brief Returns the entry with the given full name, or an invalid
         *        @c ZipArchiveEntry if not found.
         *
         * @param entryName  Full path of the entry within the archive.
         */
        ZipArchiveEntry GetEntry(const std::string& entryName) const;

        /**
         * @brief Creates a new entry for writing.
         *
         * Only valid in Create or Update mode.
         *
         * @param entryName  Path to use for the entry within the archive.
         * @param compressionLevel Compression level to use when the entry is flushed on Dispose().
         * @return A @c ZipArchiveEntry whose @c Open() stream accepts the entry data.
         */
        ZipArchiveEntry CreateEntry(const std::string& entryName,
                                    CompressionLevel compressionLevel = CompressionLevel::Optimal);

        /**
         * @brief Finalises and closes the archive.
         *
         * In Create/Update mode this writes all pending entries to the backing
         * file or stream before closing.  Safe to call multiple times.
         */
        void Dispose();
    };

} // namespace System::IO::Compression

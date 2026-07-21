// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/IO/Compression/ZipArchive.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/IO/IOException.hpp"
#include "System/IO/InvalidDataException.hpp"
#include "System/IO/MemoryStream.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/NotSupportedException.hpp"
#include <miniz/miniz.h>
#include <cstring>
#include <algorithm>
#include <unordered_set>

namespace System::IO::Compression {

// ---------------------------------------------------------------------------
// Internal write-back stream (create/update mode)
// ---------------------------------------------------------------------------

class ZipEntryWriteStream final : public System::IO::Stream {
    std::shared_ptr<std::vector<SharpRuntime::bytecs>> buf_;
public:
    explicit ZipEntryWriteStream(std::shared_ptr<std::vector<SharpRuntime::bytecs>> b)
        : buf_(std::move(b)) {}

    SharpRuntime::intcs Read(SharpRuntime::bytecs*, SharpRuntime::intcs, SharpRuntime::intcs) override { return 0; }
    void Write(const SharpRuntime::bytecs* data, SharpRuntime::intcs offset, SharpRuntime::intcs count) override {
        // Matches this codebase's established Stream::Write convention (see
        // MemoryStream::Write): count<=0 is a graceful no-op, but a negative offset was
        // previously used completely unchecked in `data + offset`, computing a pointer before
        // the buffer start -- confirmed as a genuine ASan stack-buffer-overflow via a
        // standalone repro before this fix.
        if (count <= 0) return;
        if (offset < 0)
            throw System::ArgumentOutOfRangeException("offset", "Non-negative number required.");
        buf_->insert(buf_->end(), data + offset, data + offset + count);
    }
    [[nodiscard]] SharpRuntime::intcs getLengthProperty() const override {
        return static_cast<SharpRuntime::intcs>(buf_->size());
    }
    [[nodiscard]] bool getCanReadProperty()  const override { return false; }
    [[nodiscard]] bool getCanWriteProperty() const override { return true;  }
    void Flush() override {}
    void Close() override {}
};

// ---------------------------------------------------------------------------
// Opaque state structs
// ---------------------------------------------------------------------------

struct PendingEntry {
    std::string                                         name;
    std::shared_ptr<std::vector<SharpRuntime::bytecs>> data;
    int                                                  miniLevel = MZ_DEFAULT_COMPRESSION;
};

namespace {
    int ToMinizLevel(CompressionLevel level) {
        switch (level) {
            case CompressionLevel::NoCompression: return MZ_NO_COMPRESSION;
            case CompressionLevel::Fastest:       return MZ_BEST_SPEED;
            case CompressionLevel::SmallestSize:  return MZ_BEST_COMPRESSION;
            case CompressionLevel::Optimal:
            default:                              return MZ_DEFAULT_COMPRESSION;
        }
    }
}

struct ZipArchiveState {
    mz_zip_archive          zip{};
    bool                    readerOpen   = false;
    ZipArchiveMode          mode         = ZipArchiveMode::Read;
    std::string             filePath;
    std::vector<SharpRuntime::bytecs> memBuf;      // backing buffer for stream-based read
    std::vector<PendingEntry>         pending;     // entries queued for write
    std::unordered_set<std::string>   deletedEntries; // full names marked via ZipArchiveEntry::Delete()
    bool                    disposed     = false;
    // Non-owning: set only when constructed via ZipArchive(Stream*, mode) in Create/Update mode.
    // The finalized memBuf is written back to *stream on Dispose() -- see that method. The
    // caller retains ownership and must keep the stream alive for the ZipArchive's lifetime,
    // matching the documented "Source or destination stream" parameter contract. Previously
    // this pointer was never even stored: Create/Update against a stream silently produced an
    // archive only in memBuf, which was then discarded on destruction -- the caller-supplied
    // stream was never touched at all, a confirmed real bug (audit finding A-01, 2026-07-14).
    System::IO::Stream*     stream       = nullptr;
};

struct ZipArchiveEntryState {
    std::shared_ptr<ZipArchiveState>                   archive;
    mz_uint                                            index    = 0;
    std::string                                        fullName;
    std::string                                        name;
    long long                                          length   = 0;
    // Create/update mode:
    bool                                               isWrite  = false;
    std::shared_ptr<std::vector<SharpRuntime::bytecs>> writeBuf;
};

// ---------------------------------------------------------------------------
// ZipArchiveEntry
// ---------------------------------------------------------------------------

ZipArchiveEntry::ZipArchiveEntry(std::shared_ptr<ZipArchiveEntryState> s)
    : state_(std::move(s)) {}

bool ZipArchiveEntry::IsValid() const { return state_ != nullptr; }

std::string ZipArchiveEntry::getNameProperty() const {
    return state_ ? state_->name : std::string{};
}
std::string ZipArchiveEntry::getFullNameProperty() const {
    return state_ ? state_->fullName : std::string{};
}
long long ZipArchiveEntry::getLengthProperty() const {
    return state_ ? state_->length : 0LL;
}

System::IO::Stream* ZipArchiveEntry::Open() {
    if (!state_) throw System::InvalidOperationException("ZipArchiveEntry::Open: invalid entry");

    if (state_->isWrite) {
        // Write-mode: return a stream that writes into our pending buffer
        return new ZipEntryWriteStream(state_->writeBuf);
    }

    // Read-mode: extract via miniz
    auto& arc = *state_->archive;
    if (!arc.readerOpen)
        throw System::InvalidOperationException("ZipArchiveEntry::Open: archive not open for reading");

    size_t outSize = 0;
    void* raw = mz_zip_reader_extract_to_heap(&arc.zip, state_->index, &outSize, 0);
    if (!raw)
        throw System::IO::InvalidDataException("ZipArchiveEntry::Open: extract failed for " + state_->fullName);

    auto* ms = new System::IO::MemoryStream(
        reinterpret_cast<SharpRuntime::bytecs*>(raw),
        static_cast<SharpRuntime::intcs>(outSize));
    mz_free(raw);
    return ms;
}

void ZipArchiveEntry::Delete() {
    if (!state_) return;
    if (state_->archive->mode != ZipArchiveMode::Update)
        throw System::NotSupportedException("ZipArchiveEntry::Delete: archive must be in Update mode");
    // Record the full name for exclusion when the archive is next flushed (Dispose()).
    // Covers both a pre-existing entry (excluded from the pass-through copy of the reader's
    // contents) and a still-pending, not-yet-written CreateEntry() result (excluded from
    // the pending list) uniformly, since flushWriter() filters both against this set.
    state_->archive->deletedEntries.insert(state_->fullName);
    state_ = nullptr;
}

// ---------------------------------------------------------------------------
// ZipArchive helpers
// ---------------------------------------------------------------------------

static std::string basename(const std::string& fullName) {
    auto pos = fullName.rfind('/');
    if (pos == std::string::npos) pos = fullName.rfind('\\');
    return (pos == std::string::npos) ? fullName : fullName.substr(pos + 1);
}

static void openReader(ZipArchiveState& st) {
    mz_bool ok;
    if (!st.filePath.empty()) {
        ok = mz_zip_reader_init_file(&st.zip, st.filePath.c_str(), 0);
    } else {
        ok = mz_zip_reader_init_mem(&st.zip, st.memBuf.data(), st.memBuf.size(), 0);
    }
    if (!ok)
        throw System::IO::InvalidDataException("ZipArchive: failed to open zip for reading: " +
                                 (st.filePath.empty() ? "(memory)" : st.filePath));
    st.readerOpen = true;
}

namespace {
    struct ExtractedEntry {
        std::string name;
        std::vector<SharpRuntime::bytecs> data;
    };
}

// Verified against ZipArchive.cs: real .NET keeps a single unified _entries list containing
// both entries read from disk and newly-created ones, so disposing an Update-mode archive
// always preserves every entry that wasn't explicitly Delete()'d. This port's miniz-backed
// design uses a separate reader (existing entries) and pending list (new entries) instead of
// one unified list, so flushWriter() must explicitly extract and carry forward every
// pre-existing, non-deleted entry -- previously it wrote only st.pending, silently discarding
// every pre-existing entry whenever the archive was disposed with at least one CreateEntry()
// call (real, silent, irreversible data loss on a standard "update a zip" workflow).
// Passing a null buffer pointer for a zero-length entry (e.g. an empty vector's .data(), which
// libstdc++ returns as nullptr) eventually reaches miniz's own mz_zip_file_write_func, which
// calls fwrite(pBuf, 1, 0, file) -- fwrite's first parameter is declared nonnull, so this is
// technically undefined behavior by the letter of the standard even though it's universally
// harmless in practice (0 bytes are never actually read through a null pointer). Confirmed via
// UndefinedBehaviorSanitizer (2026-07-14). vendor/miniz is third-party, unmodified-from-upstream
// source (see CLAUDE.md) -- not the place to fix this; passing a non-null, never-dereferenced
// pointer for an empty buffer here avoids the UB entirely at the call site instead.
static const SharpRuntime::bytecs* safeDataPtr(const SharpRuntime::bytecs* data, size_t size) {
    static const SharpRuntime::bytecs dummy = 0;
    return size == 0 ? &dummy : data;
}

static void flushWriter(ZipArchiveState& st) {
    const bool hasChanges = !st.pending.empty() || !st.deletedEntries.empty();
    if (st.mode == ZipArchiveMode::Update && !hasChanges) return;
    if (st.mode == ZipArchiveMode::Create && st.pending.empty()) return;

    // Extract every pre-existing, non-deleted entry into memory before opening a writer --
    // the writer may truncate/recreate the same backing file or buffer the reader is
    // currently attached to, so the reader must be fully drained (and closed) first.
    std::vector<ExtractedEntry> existing;
    if (st.mode == ZipArchiveMode::Update && st.readerOpen) {
        mz_uint count = mz_zip_reader_get_num_files(&st.zip);
        existing.reserve(count);
        for (mz_uint i = 0; i < count; ++i) {
            mz_zip_archive_file_stat stat{};
            if (!mz_zip_reader_file_stat(&st.zip, i, &stat)) continue;
            if (stat.m_is_directory) continue;
            std::string name = stat.m_filename;
            if (st.deletedEntries.count(name)) continue;

            size_t outSize = 0;
            void* raw = mz_zip_reader_extract_to_heap(&st.zip, i, &outSize, 0);
            if (!raw) continue; // best-effort: skip an unreadable entry rather than aborting the flush
            ExtractedEntry ee;
            ee.name = std::move(name);
            ee.data.assign(reinterpret_cast<SharpRuntime::bytecs*>(raw),
                           reinterpret_cast<SharpRuntime::bytecs*>(raw) + outSize);
            mz_free(raw);
            existing.push_back(std::move(ee));
        }
        mz_zip_reader_end(&st.zip);
        st.readerOpen = false;
    }

    if (!st.filePath.empty()) {
        mz_zip_archive writer{};
        if (!mz_zip_writer_init_file(&writer, st.filePath.c_str(), 0))
            throw System::IO::IOException("ZipArchive: failed to init writer for " + st.filePath);
        // mz_zip_writer_add_mem/finalize_archive return mz_bool and were previously never
        // checked, so a write failure (duplicate name, allocation failure, disk full) silently
        // left a truncated/incomplete zip file on disk with no exception raised at all --
        // matches the already-established convention just above (init_file's own check) of
        // treating a miniz failure as IOException, just extended to the calls that were missed.
        // mz_zip_writer_end must still run on the throw path to release miniz's internal state.
        try {
            for (auto& e : existing) {
                if (!mz_zip_writer_add_mem(&writer, e.name.c_str(), safeDataPtr(e.data.data(), e.data.size()), e.data.size(),
                                           static_cast<mz_uint>(MZ_DEFAULT_COMPRESSION)))
                    throw System::IO::IOException("ZipArchive: failed to write entry '" + e.name + "'");
            }
            for (auto& e : st.pending) {
                if (st.deletedEntries.count(e.name)) continue;
                if (!mz_zip_writer_add_mem(&writer, e.name.c_str(),
                                           safeDataPtr(e.data->data(), e.data->size()), e.data->size(),
                                           static_cast<mz_uint>(e.miniLevel)))
                    throw System::IO::IOException("ZipArchive: failed to write entry '" + e.name + "'");
            }
            if (!mz_zip_writer_finalize_archive(&writer))
                throw System::IO::IOException("ZipArchive: failed to finalize " + st.filePath);
        } catch (...) {
            mz_zip_writer_end(&writer);
            throw;
        }
        mz_zip_writer_end(&writer);
    } else {
        // Memory-based write — store result back in memBuf
        mz_zip_archive writer{};
        if (!mz_zip_writer_init_heap(&writer, 0, 65536))
            throw System::IO::IOException("ZipArchive: failed to init heap writer");
        void* buf = nullptr; size_t sz = 0;
        try {
            for (auto& e : existing) {
                if (!mz_zip_writer_add_mem(&writer, e.name.c_str(), safeDataPtr(e.data.data(), e.data.size()), e.data.size(),
                                           static_cast<mz_uint>(MZ_DEFAULT_COMPRESSION)))
                    throw System::IO::IOException("ZipArchive: failed to write entry '" + e.name + "'");
            }
            for (auto& e : st.pending) {
                if (st.deletedEntries.count(e.name)) continue;
                if (!mz_zip_writer_add_mem(&writer, e.name.c_str(),
                                           safeDataPtr(e.data->data(), e.data->size()), e.data->size(),
                                           static_cast<mz_uint>(e.miniLevel)))
                    throw System::IO::IOException("ZipArchive: failed to write entry '" + e.name + "'");
            }
            if (!mz_zip_writer_finalize_heap_archive(&writer, &buf, &sz))
                throw System::IO::IOException("ZipArchive: failed to finalize heap archive");
        } catch (...) {
            mz_zip_writer_end(&writer);
            throw;
        }
        mz_zip_writer_end(&writer);
        st.memBuf.assign(reinterpret_cast<SharpRuntime::bytecs*>(buf),
                         reinterpret_cast<SharpRuntime::bytecs*>(buf) + sz);
        mz_free(buf);
    }
    st.pending.clear();
    st.deletedEntries.clear();
}

// ---------------------------------------------------------------------------
// ZipArchive constructors / destructor
// ---------------------------------------------------------------------------

ZipArchive::ZipArchive(System::IO::Stream* stream, ZipArchiveMode mode)
    : state_(std::make_shared<ZipArchiveState>())
{
    state_->mode = mode;
    if (mode == ZipArchiveMode::Read || mode == ZipArchiveMode::Update) {
        // Read full stream into memory buffer
        SharpRuntime::bytecs tmp[65536];
        SharpRuntime::intcs n;
        while ((n = stream->Read(tmp, 0, 65536)) > 0)
            state_->memBuf.insert(state_->memBuf.end(), tmp, tmp + n);
        // Found alongside the A-01 fix (2026-07-14): openReader() was only called for Read
        // mode here, unlike the file-path constructor below, which correctly opens it for
        // BOTH Read and Update. Without it, readerOpen stayed false for a stream-backed
        // Update-mode archive, so flushWriter()'s "carry forward every pre-existing entry"
        // step (gated on readerOpen) never ran -- every existing entry was silently dropped
        // on Dispose() whenever Update mode was used via a Stream*, confirmed via a
        // standalone regression test that round-tripped a stream through Create then Update.
        if (mode == ZipArchiveMode::Read || mode == ZipArchiveMode::Update)
            openReader(*state_);
    }
    // Create/Update mode: retain the stream so Dispose() can write the finalized archive back
    // to it -- see ZipArchiveState::stream's own doc-comment (audit finding A-01, 2026-07-14).
    // Not retained for Read mode: read-only, nothing is ever written back.
    if (mode == ZipArchiveMode::Create || mode == ZipArchiveMode::Update) {
        state_->stream = stream;
    }
}

ZipArchive::ZipArchive(const std::string& archivePath, ZipArchiveMode mode)
    : state_(std::make_shared<ZipArchiveState>())
{
    state_->mode     = mode;
    state_->filePath = archivePath;
    if (mode == ZipArchiveMode::Read || mode == ZipArchiveMode::Update)
        openReader(*state_);
}

// Best-effort, non-throwing (audit finding A-02, 2026-07-14) -- see DeflateStream::~DeflateStream's
// identical doc-comment for the full rationale and confirmed std::terminate repro. Dispose() can
// throw both from the pre-existing flushWriter() miniz-failure path and from this port's own A-01
// stream-write-back code (stream->Write()/SetLength() on a failing stream).
ZipArchive::~ZipArchive() { try { Dispose(); } catch (...) {} }

// ---------------------------------------------------------------------------
// ZipArchive operations
// ---------------------------------------------------------------------------

std::vector<ZipArchiveEntry> ZipArchive::getEntriesProperty() const {
    if (!state_ || !state_->readerOpen) return {};
    mz_uint count = mz_zip_reader_get_num_files(&state_->zip);
    std::vector<ZipArchiveEntry> result;
    result.reserve(count);
    for (mz_uint i = 0; i < count; ++i) {
        mz_zip_archive_file_stat stat{};
        if (!mz_zip_reader_file_stat(&state_->zip, i, &stat)) continue;
        if (stat.m_is_directory) continue;
        auto es       = std::make_shared<ZipArchiveEntryState>();
        es->archive   = state_;
        es->index     = i;
        es->fullName  = stat.m_filename;
        es->name      = basename(stat.m_filename);
        es->length    = static_cast<long long>(stat.m_uncomp_size);
        result.emplace_back(std::move(es));
    }
    return result;
}

ZipArchiveEntry ZipArchive::GetEntry(const std::string& entryName) const {
    if (!state_ || !state_->readerOpen) return ZipArchiveEntry{};
    mz_uint count = mz_zip_reader_get_num_files(&state_->zip);
    for (mz_uint i = 0; i < count; ++i) {
        mz_zip_archive_file_stat stat{};
        if (!mz_zip_reader_file_stat(&state_->zip, i, &stat)) continue;
        if (entryName == stat.m_filename) {
            auto es      = std::make_shared<ZipArchiveEntryState>();
            es->archive  = state_;
            es->index    = i;
            es->fullName = stat.m_filename;
            es->name     = basename(stat.m_filename);
            es->length   = static_cast<long long>(stat.m_uncomp_size);
            return ZipArchiveEntry(std::move(es));
        }
    }
    return ZipArchiveEntry{};
}

ZipArchiveEntry ZipArchive::CreateEntry(const std::string& entryName, CompressionLevel compressionLevel) {
    if (!state_)
        throw System::InvalidOperationException("ZipArchive::CreateEntry: archive not open");
    if (state_->mode == ZipArchiveMode::Read)
        throw System::NotSupportedException("ZipArchive::CreateEntry: archive is read-only");

    auto buf = std::make_shared<std::vector<SharpRuntime::bytecs>>();
    state_->pending.push_back({entryName, buf, ToMinizLevel(compressionLevel)});

    auto es       = std::make_shared<ZipArchiveEntryState>();
    es->archive   = state_;
    es->fullName  = entryName;
    es->name      = basename(entryName);
    es->isWrite   = true;
    es->writeBuf  = buf;
    return ZipArchiveEntry(std::move(es));
}

void ZipArchive::Dispose() {
    if (!state_ || state_->disposed) return;
    state_->disposed = true;

    if (state_->mode == ZipArchiveMode::Create || state_->mode == ZipArchiveMode::Update)
        flushWriter(*state_);

    if (state_->readerOpen) {
        mz_zip_reader_end(&state_->zip);
        state_->readerOpen = false;
    }

    // Write the finalized archive back to the caller-supplied stream (audit finding A-01,
    // 2026-07-14): flushWriter() above only ever populates memBuf for a stream-backed archive
    // (filePath is empty in that case); this port has no live streaming writer the way real
    // .NET's ZipArchive does, so the simplest correct fix matching this port's existing
    // buffer-then-flush design is to push the finalized buffer to the stream here, once,
    // unconditionally -- memBuf always holds the current, complete archive content at this
    // point, whether flushWriter just rebuilt it or (Update mode, no changes made) it's still
    // the original content read at construction. Requires seek support to correctly overwrite
    // from the start and truncate to the new size (matching real .NET's own Seek(0)+
    // SetLength+write sequence); a non-seekable stream skips the reposition/truncate and just
    // appends, which is the best this port's simpler buffered design can do without adopting
    // real .NET's full live-streaming architecture.
    if (state_->stream != nullptr &&
        (state_->mode == ZipArchiveMode::Create || state_->mode == ZipArchiveMode::Update)) {
        System::IO::Stream* stream = state_->stream;
        if (stream->getCanSeekProperty()) {
            stream->setPositionProperty(0);
            stream->SetLength(static_cast<SharpRuntime::intcs>(state_->memBuf.size()));
        }
        if (!state_->memBuf.empty()) {
            stream->Write(state_->memBuf.data(), 0, static_cast<SharpRuntime::intcs>(state_->memBuf.size()));
        }
        stream->Flush();
    }
}

} // namespace System::IO::Compression

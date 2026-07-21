// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Buffers/OperationStatus.hpp"

namespace System::Text::Unicode {

    using SharpRuntime::bytecs;
    using SharpRuntime::intcs;

    /**
     * @brief Provides static methods that convert data between UTF-8 and UTF-16 encodings, and
     * methods that validate UTF-8 sequences.
     *
     * C++ counterpart of .NET System.Text.Unicode.Utf8.
     *
     * @note `FromUtf16`/`ToUtf16` process the whole source buffer in a single call rather than
     * reproducing .NET's internal chunked-transcoding pointer arithmetic (`Utf8Utility`'s
     * resumable per-call state) — the observable single-call contract (return status, `*Read`/
     * `*Written` counts, replacement behaviour) matches .NET exactly for both `isFinalBlock`
     * values; only true multi-call incremental resumption across separate buffers isn't
     * reproduced (not needed since callers here always hold the full buffer already).
     */
    class Utf8 {
    public:
        Utf8() = delete;

        /**
         * @brief Finds the index of the first invalid UTF-8 subsequence.
         * @param value The UTF-8 input bytes to examine.
         * @return The index of the first invalid UTF-8 subsequence, or -1 if the entire input is valid.
         */
        static intcs IndexOfInvalidSubsequence(const std::vector<bytecs>& value) {
            size_t i = 0;
            while (i < value.size()) {
                auto b0 = static_cast<unsigned char>(value[i]);
                if (b0 < 0x80) { ++i; continue; }

                size_t len;
                char32_t cp;
                char32_t minCp;
                if ((b0 & 0xE0) == 0xC0)      { len = 2; cp = b0 & 0x1Fu; minCp = 0x80; }
                else if ((b0 & 0xF0) == 0xE0) { len = 3; cp = b0 & 0x0Fu; minCp = 0x800; }
                else if ((b0 & 0xF8) == 0xF0) { len = 4; cp = b0 & 0x07u; minCp = 0x10000; }
                else return static_cast<intcs>(i); // stray continuation byte or 0xF8-0xFF

                if (i + len > value.size()) return static_cast<intcs>(i); // truncated sequence

                bool ok = true;
                for (size_t k = 1; k < len; ++k) {
                    auto bk = static_cast<unsigned char>(value[i + k]);
                    if ((bk & 0xC0) != 0x80) { ok = false; break; }
                    cp = (cp << 6) | (bk & 0x3Fu);
                }
                if (!ok) return static_cast<intcs>(i);
                if (cp < minCp) return static_cast<intcs>(i);              // overlong encoding
                if (cp > 0x10FFFF) return static_cast<intcs>(i);           // outside Unicode range
                if (cp >= 0xD800 && cp <= 0xDFFF) return static_cast<intcs>(i); // surrogate code point

                i += len;
            }
            return -1;
        }

        /**
         * @brief Validates that the value is well-formed UTF-8.
         * @param value The UTF-8 input bytes to validate.
         * @return true if @p value is well-formed UTF-8, false otherwise.
         */
        static bool IsValid(const std::vector<bytecs>& value) { return IndexOfInvalidSubsequence(value) < 0; }

        /**
         * @brief Transcodes the UTF-16 @p source buffer to @p destination as UTF-8.
         * @param source The UTF-16 input text.
         * @param destination The buffer to receive the UTF-8 output; its `size()` is the capacity.
         * @param charsRead Set to the number of UTF-16 code units consumed from @p source.
         * @param bytesWritten Set to the number of UTF-8 bytes written to @p destination.
         * @param replaceInvalidSequences If true, ill-formed input is replaced with U+FFFD instead
         * of causing `InvalidData`.
         * @param isFinalBlock If false, a source that ends with an incomplete high surrogate is
         * reported as `NeedMoreData` instead of being treated as ill-formed.
         * @return `Done`, `DestinationTooSmall`, `NeedMoreData`, or `InvalidData`.
         */
        static System::Buffers::OperationStatus FromUtf16(const std::u16string& source,
                                                            std::vector<bytecs>& destination, intcs& charsRead,
                                                            intcs& bytesWritten, bool replaceInvalidSequences = true,
                                                            bool isFinalBlock = true) {
            size_t si = 0, di = 0;
            while (si < source.size()) {
                char16_t c = source[si];
                char32_t cp;
                size_t consumed;
                bool valid = true;

                if (c >= 0xD800 && c <= 0xDBFF) { // high surrogate
                    if (si + 1 < source.size()) {
                        char16_t c2 = source[si + 1];
                        if (c2 >= 0xDC00 && c2 <= 0xDFFF) {
                            cp = 0x10000 + ((static_cast<char32_t>(c) - 0xD800) << 10) +
                                 (static_cast<char32_t>(c2) - 0xDC00);
                            consumed = 2;
                        } else {
                            valid = false;
                            consumed = 1;
                        }
                    } else if (!isFinalBlock) {
                        charsRead = static_cast<intcs>(si);
                        bytesWritten = static_cast<intcs>(di);
                        return System::Buffers::OperationStatus::NeedMoreData;
                    } else {
                        valid = false;
                        consumed = 1;
                    }
                } else if (c >= 0xDC00 && c <= 0xDFFF) { // unpaired low surrogate
                    valid = false;
                    consumed = 1;
                    cp = 0;
                } else {
                    cp = c;
                    consumed = 1;
                }

                if (!valid) {
                    if (!replaceInvalidSequences) {
                        charsRead = static_cast<intcs>(si);
                        bytesWritten = static_cast<intcs>(di);
                        return System::Buffers::OperationStatus::InvalidData;
                    }
                    cp = 0xFFFD;
                }

                size_t needed = (cp < 0x80) ? 1 : (cp < 0x800) ? 2 : (cp < 0x10000) ? 3 : 4;
                if (di + needed > destination.size()) {
                    charsRead = static_cast<intcs>(si);
                    bytesWritten = static_cast<intcs>(di);
                    return System::Buffers::OperationStatus::DestinationTooSmall;
                }

                switch (needed) {
                    case 1:
                        destination[di++] = static_cast<bytecs>(cp);
                        break;
                    case 2:
                        destination[di++] = static_cast<bytecs>(0xC0 | (cp >> 6));
                        destination[di++] = static_cast<bytecs>(0x80 | (cp & 0x3F));
                        break;
                    case 3:
                        destination[di++] = static_cast<bytecs>(0xE0 | (cp >> 12));
                        destination[di++] = static_cast<bytecs>(0x80 | ((cp >> 6) & 0x3F));
                        destination[di++] = static_cast<bytecs>(0x80 | (cp & 0x3F));
                        break;
                    default:
                        destination[di++] = static_cast<bytecs>(0xF0 | (cp >> 18));
                        destination[di++] = static_cast<bytecs>(0x80 | ((cp >> 12) & 0x3F));
                        destination[di++] = static_cast<bytecs>(0x80 | ((cp >> 6) & 0x3F));
                        destination[di++] = static_cast<bytecs>(0x80 | (cp & 0x3F));
                        break;
                }
                si += consumed;
            }
            charsRead = static_cast<intcs>(si);
            bytesWritten = static_cast<intcs>(di);
            return System::Buffers::OperationStatus::Done;
        }

        /**
         * @brief Transcodes the UTF-8 @p source buffer to @p destination as UTF-16.
         * @param source The UTF-8 input bytes.
         * @param destination The buffer to receive the UTF-16 output; its `size()` is the capacity.
         * @param bytesRead Set to the number of UTF-8 bytes consumed from @p source.
         * @param charsWritten Set to the number of UTF-16 code units written to @p destination.
         * @param replaceInvalidSequences If true, ill-formed input is replaced with U+FFFD instead
         * of causing `InvalidData`.
         * @param isFinalBlock If false, a source that ends mid-sequence is reported as
         * `NeedMoreData` instead of being treated as ill-formed.
         * @return `Done`, `DestinationTooSmall`, `NeedMoreData`, or `InvalidData`.
         */
        static System::Buffers::OperationStatus ToUtf16(const std::vector<bytecs>& source,
                                                          std::u16string& destination, intcs& bytesRead,
                                                          intcs& charsWritten, bool replaceInvalidSequences = true,
                                                          bool isFinalBlock = true) {
            size_t si = 0, di = 0;
            while (si < source.size()) {
                auto b0 = static_cast<unsigned char>(source[si]);
                size_t len;
                char32_t cp;
                char32_t minCp;
                bool valid = true;

                if (b0 < 0x80) {
                    len = 1;
                    cp = b0;
                    minCp = 0;
                } else if ((b0 & 0xE0) == 0xC0) {
                    len = 2;
                    cp = b0 & 0x1Fu;
                    minCp = 0x80;
                } else if ((b0 & 0xF0) == 0xE0) {
                    len = 3;
                    cp = b0 & 0x0Fu;
                    minCp = 0x800;
                } else if ((b0 & 0xF8) == 0xF0) {
                    len = 4;
                    cp = b0 & 0x07u;
                    minCp = 0x10000;
                } else {
                    valid = false;
                    len = 1;
                    cp = 0xFFFD;
                    minCp = 0;
                }

                if (valid && len > 1) {
                    // Scan continuation bytes up to whichever comes first: an invalid
                    // continuation byte, or the end of the available buffer -- this determines
                    // the "maximal subpart" of the ill-formed sequence per the Unicode
                    // Standard's replacement recommendation (Ch. 3.9), which real .NET follows
                    // via Rune.DecodeFromUtf8 (Utf8.ToUtf16 delegates to it for exactly this
                    // purpose after writing a replacement char). The previous version had a
                    // separate upfront "not enough bytes for the promised length" branch that
                    // consumed EVERY remaining byte as one replacement, even when some of
                    // those bytes weren't even valid continuation bytes and should have been
                    // preserved/reprocessed as their own characters -- confirmed via a
                    // standalone repro: {0xF0, 'A', 'B'} (a 4-byte lead byte followed by two
                    // ordinary ASCII bytes, no continuation bytes at all) previously produced
                    // only U+FFFD, silently swallowing 'A' and 'B', instead of U+FFFD + "AB".
                    size_t k = 1;
                    for (; k < len; ++k) {
                        if (si + k >= source.size()) {
                            // Ran out of buffer before completing the promised sequence length.
                            if (!isFinalBlock) {
                                bytesRead = static_cast<intcs>(si);
                                charsWritten = static_cast<intcs>(di);
                                return System::Buffers::OperationStatus::NeedMoreData;
                            }
                            valid = false;
                            break;
                        }
                        auto bk = static_cast<unsigned char>(source[si + k]);
                        if ((bk & 0xC0) != 0x80) {
                            valid = false;
                            break;
                        }
                        cp = (cp << 6) | (bk & 0x3Fu);
                    }
                    if (!valid) {
                        len = k; // maximal subpart: only the lead byte + validated continuation bytes so far
                    } else if (cp < minCp || cp > 0x10FFFF || (cp >= 0xD800 && cp <= 0xDFFF)) {
                        valid = false;
                        // len stays at the full promised length: overlong/surrogate/out-of-range
                        // is only detectable once the whole sequence has been read, matching
                        // the pre-existing (already-correct) behavior for this case.
                    }
                }

                if (!valid) {
                    if (!replaceInvalidSequences) {
                        bytesRead = static_cast<intcs>(si);
                        charsWritten = static_cast<intcs>(di);
                        return System::Buffers::OperationStatus::InvalidData;
                    }
                    cp = 0xFFFD;
                    if (len == 0) len = 1;
                }

                size_t needed = (cp < 0x10000) ? 1 : 2;
                if (di + needed > destination.size()) {
                    bytesRead = static_cast<intcs>(si);
                    charsWritten = static_cast<intcs>(di);
                    return System::Buffers::OperationStatus::DestinationTooSmall;
                }

                if (needed == 1) {
                    destination[di++] = static_cast<char16_t>(cp);
                } else {
                    char32_t v = cp - 0x10000;
                    destination[di++] = static_cast<char16_t>(0xD800 + (v >> 10));
                    destination[di++] = static_cast<char16_t>(0xDC00 + (v & 0x3FF));
                }
                si += len;
            }
            bytesRead = static_cast<intcs>(si);
            charsWritten = static_cast<intcs>(di);
            return System::Buffers::OperationStatus::Done;
        }
    };

} // namespace System::Text::Unicode

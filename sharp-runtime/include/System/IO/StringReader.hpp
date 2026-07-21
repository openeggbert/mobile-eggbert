// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/IO/TextReader.hpp"

namespace System::IO {

    /**
     * @brief Implements a TextReader that reads from a string.
     *
     * Partial C++ counterpart of .NET System.IO.StringReader.
     *
     * @note Status: Implemented
     */
    class StringReader : public TextReader {
        std::string s_;
        intcs pos_ = 0;
    public:
        /** Constructs a StringReader over the given string. */
        explicit StringReader(const std::string& s) : s_(s) {}

        /** Returns the next character without advancing the position, or -1 at end. */
        [[nodiscard]] intcs Peek() override {
            if (pos_ >= static_cast<intcs>(s_.size())) return -1;
            return static_cast<unsigned char>(s_[pos_]);
        }

        /** Reads and returns the next character, or -1 at end. */
        intcs Read() override {
            if (pos_ >= static_cast<intcs>(s_.size())) return -1;
            return static_cast<unsigned char>(s_[pos_++]);
        }

        /**
         * @brief Reads the next line, stripping the line terminator.
         * @note Verified against StringReader.cs's ReadLine(): real .NET treats '\r' and '\n'
         * as interchangeable line terminators -- a lone '\r' (classic Mac line ending) ends
         * the line on its own, not just as part of "\r\n". If '\r' is immediately followed by
         * '\n', both are consumed as one terminator (CRLF); a '\r' not followed by '\n' still
         * terminates the line by itself. This previously only stopped scanning at '\n', so a
         * lone '\r' was treated as ordinary line content -- silently merging what should be
         * two separate lines into one, with the '\r' left embedded in the middle of the
         * result.
         */
        [[nodiscard]] std::string ReadLine() override {
            if (pos_ >= static_cast<intcs>(s_.size())) return "";
            auto start = pos_;
            while (pos_ < static_cast<intcs>(s_.size()) && s_[pos_] != '\n' && s_[pos_] != '\r') ++pos_;
            std::string line = s_.substr(start, pos_ - start);
            if (pos_ < static_cast<intcs>(s_.size())) {
                bool isCrLf = s_[pos_] == '\r' && pos_ + 1 < static_cast<intcs>(s_.size()) && s_[pos_ + 1] == '\n';
                pos_ += isCrLf ? 2 : 1;
            }
            return line;
        }

        /** Reads all remaining text from the current position. */
        [[nodiscard]] std::string ReadToEnd() override {
            if (pos_ >= static_cast<intcs>(s_.size())) return "";
            std::string rest = s_.substr(pos_);
            pos_ = static_cast<intcs>(s_.size());
            return rest;
        }
    };

} // namespace System::IO

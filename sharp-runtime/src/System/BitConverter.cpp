// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/BitConverter.hpp"
#include <sstream>
#include <iomanip>
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"

namespace System {

    std::string BitConverter::ToString(const bytecs* value, intcs startIndex, intcs length) {
        // value is a raw pointer with no length information, so this overload can only
        // validate what doesn't require knowing the buffer's size -- see the vector overload
        // below for the full array-length-aware bounds checking. Confirmed as a real,
        // reachable out-of-bounds read (not just missing validation): before this fix, the
        // vector overload below computed `length = value.size() - startIndex` with no
        // validation of startIndex, so ToString(vec, -1) computed an inflated length and this
        // function then read value[startIndex + i] = value[-1] on the first iteration --
        // genuine undefined behavior, one element before the buffer's start.
        if (startIndex < 0)
            throw System::ArgumentOutOfRangeException("startIndex");
        if (length < 0)
            throw System::ArgumentOutOfRangeException("length");
        std::ostringstream oss;
        oss.imbue(std::locale::classic());
        for (intcs i = 0; i < length; ++i) {
            if (i > 0) oss << '-';
            oss << std::uppercase << std::hex << std::setw(2) << std::setfill('0')
                << static_cast<unsigned>(static_cast<unsigned char>(value[startIndex + i]));
        }
        return oss.str();
    }

    std::string BitConverter::ToString(const std::vector<bytecs>& value, intcs startIndex, intcs length) {
        // Verified against BitConverter.cs's ToString(byte[], int, int) exactly, including its
        // quirky-but-real boundary case: startIndex == size (for a non-empty array) THROWS in
        // real .NET, it does not simply produce an empty string, even though length would come
        // out to 0 for that combination via the two-arg overload's delegation below.
        intcs size = static_cast<intcs>(value.size());
        if (startIndex < 0 || (startIndex >= size && startIndex > 0))
            throw System::ArgumentOutOfRangeException("startIndex");
        if (length < 0)
            throw System::ArgumentOutOfRangeException("length");
        if (startIndex > size - length)
            throw System::ArgumentException(
                "Destination array was not long enough. Check the destination index, length, and the array's lower bounds.",
                "value");
        return ToString(value.data(), startIndex, length);
    }

    std::string BitConverter::ToString(const std::vector<bytecs>& value, intcs startIndex) {
        return ToString(value, startIndex, static_cast<intcs>(value.size()) - startIndex);
    }

    std::string BitConverter::ToString(const std::vector<bytecs>& value) {
        return ToString(value, 0, static_cast<intcs>(value.size()));
    }

} // namespace System

// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Text/StringBuilder.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentNullException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"

#include <algorithm>
#include <array>
#include <charconv>
#include <sstream>
#include <utility>

namespace System::Text
{
    StringBuilder::StringBuilder()
        : buffer()
    {
    }

    StringBuilder::StringBuilder(const std::string& value)
        : buffer(value)
    {
    }

    void StringBuilder::Clear()
    {
        buffer.clear();
    }

    StringBuilder& StringBuilder::Append(const std::string& value)
    {
        buffer += value;
        return *this;
    }

    StringBuilder& StringBuilder::Append(const char* value)
    {
        if (value != nullptr)
        {
            buffer += value;
        }
        return *this;
    }

    StringBuilder& StringBuilder::Append(char value)
    {
        buffer += value;
        return *this;
    }

    StringBuilder& StringBuilder::Append(char value, intcs repeatCount)
    {
        // Matches StringBuilder.cs's Append(char, int): ArgumentOutOfRangeException.
        // ThrowIfNegative(repeatCount). Without this, a negative repeatCount wraps to a huge
        // size_t via the unsigned cast below, and std::string::append throws a raw
        // std::length_error instead of the System:: exception this API is documented to throw.
        if (repeatCount < 0)
            throw System::ArgumentOutOfRangeException("repeatCount", "Non-negative number required.");
        buffer.append(static_cast<size_t>(repeatCount), value);
        return *this;
    }

    StringBuilder& StringBuilder::Append(intcs value)
    {
        buffer += std::to_string(value);
        return *this;
    }

    StringBuilder& StringBuilder::Append(double value)
    {
        std::array<char, 64> buf;
        auto [ptr, ec] = std::to_chars(buf.data(), buf.data() + buf.size(), value);
        buffer += (ec == std::errc{}) ? std::string(buf.data(), ptr) : std::to_string(value);
        return *this;
    }

    StringBuilder& StringBuilder::Append(float value)
    {
        std::array<char, 32> buf;
        auto [ptr, ec] = std::to_chars(buf.data(), buf.data() + buf.size(), value);
        buffer += (ec == std::errc{}) ? std::string(buf.data(), ptr) : std::to_string(value);
        return *this;
    }

    StringBuilder& StringBuilder::Append(bool value)
    {
        buffer += (value ? "True" : "False");
        return *this;
    }

    StringBuilder& StringBuilder::AppendLine()
    {
        buffer += '\n';
        return *this;
    }

    StringBuilder& StringBuilder::AppendLine(const std::string& value)
    {
        buffer += value;
        buffer += '\n';
        return *this;
    }

    std::string StringBuilder::ToString() const
    {
        return buffer;
    }

    intcs StringBuilder::getLengthProperty() const
    {
        return static_cast<intcs>(buffer.size());
    }

    void StringBuilder::setLengthProperty(intcs value)
    {
        // Matches StringBuilder.cs's Length setter: ArgumentOutOfRangeException.
        // ThrowIfNegative(value). Without this, a negative value wraps to a huge size_t and
        // std::string::resize throws a raw std::length_error/std::bad_alloc instead of the
        // System:: exception this API is documented to throw.
        if (value < 0)
            throw System::ArgumentOutOfRangeException("value", "Non-negative number required.");
        buffer.resize(static_cast<size_t>(value), '\0');
    }

    bool StringBuilder::Empty() const
    {
        return buffer.empty();
    }

    void StringBuilder::CopyTo(intcs sourceIndex, char* destination, intcs destinationLength,
                                intcs destinationIndex, intcs count) const
    {
        if (destination == nullptr)
            throw System::ArgumentNullException("destination");
        if (count < 0)
            throw System::ArgumentOutOfRangeException("count", "Non-negative number required.");
        if (destinationIndex < 0)
            throw System::ArgumentOutOfRangeException("destinationIndex", "Non-negative number required.");
        intcs length = static_cast<intcs>(buffer.size());
        if (sourceIndex < 0 || sourceIndex > length)
            throw System::ArgumentOutOfRangeException("sourceIndex", "Index was out of range. Must be non-negative and less than or equal to the length of the StringBuilder.");
        if (sourceIndex > length - count)
            throw System::ArgumentException("sourceIndex was greater than the length of the source StringBuilder minus count.");
        if (destinationIndex > destinationLength - count)
            throw System::ArgumentException("Offset and length were out of bounds for the destination array, or count is greater than the number of elements from destinationIndex to the end of the destination array.");
        std::copy(buffer.begin() + sourceIndex, buffer.begin() + sourceIndex + count,
                  destination + destinationIndex);
    }

    StringBuilder& StringBuilder::Append(SharpRuntime::longcs value)
    {
        buffer += std::to_string(value);
        return *this;
    }

    StringBuilder& StringBuilder::Insert(intcs index, const std::string& value)
    {
        // Matches StringBuilder.cs's Insert(int, string): index must be in [0, Length]
        // (checked via the unsigned-comparison idiom `(uint)index > (uint)Length`, which also
        // catches a negative index). Without this, an out-of-range index wraps/overflows into
        // std::string::insert, which throws a raw std::out_of_range instead of the System::
        // exception this API is documented to throw.
        if (static_cast<SharpRuntime::uintcs>(index) > static_cast<SharpRuntime::uintcs>(buffer.size()))
            throw System::ArgumentOutOfRangeException("index", "Index must be within the bounds of the StringBuilder.");
        buffer.insert(static_cast<size_t>(index), value);
        return *this;
    }

    StringBuilder& StringBuilder::Remove(intcs startIndex, intcs count)
    {
        // Matches StringBuilder.cs's Remove(int, int): both must be non-negative, and
        // count must not exceed Length - startIndex. Without these, a negative startIndex/count
        // wraps into std::string::erase (raw std::out_of_range instead of System::
        // ArgumentOutOfRangeException), and an over-large (but non-wrapping) count was
        // previously silently clamped by std::string::erase instead of throwing -- real .NET
        // rejects it outright rather than removing fewer characters than requested.
        if (startIndex < 0)
            throw System::ArgumentOutOfRangeException("startIndex", "Non-negative number required.");
        if (count < 0)
            throw System::ArgumentOutOfRangeException("count", "Non-negative number required.");
        if (count > static_cast<intcs>(buffer.size()) - startIndex)
            throw System::ArgumentOutOfRangeException("count", "Index and count must refer to a location within the string.");
        buffer.erase(static_cast<size_t>(startIndex), static_cast<size_t>(count));
        return *this;
    }

    StringBuilder& StringBuilder::Replace(const std::string& oldValue, const std::string& newValue)
    {
        // .NET throws for a null/empty oldValue (ArgumentException.ThrowIfNullOrEmpty,
        // StringBuilder.cs) rather than looping. Without this check, buffer.find("", pos)
        // always succeeds at pos: if newValue is also empty, pos never advances and this
        // loops forever; if newValue is non-empty, each iteration inserts newValue.size()
        // characters and advances pos by exactly that much, so buffer.size() - pos never
        // shrinks and the loop still never terminates (runs until OOM).
        if (oldValue.empty())
            throw System::ArgumentException("The value cannot be an empty string.", "oldValue");
        size_t pos = 0;
        while ((pos = buffer.find(oldValue, pos)) != std::string::npos) {
            buffer.replace(pos, oldValue.size(), newValue);
            pos += newValue.size();
        }
        return *this;
    }
}
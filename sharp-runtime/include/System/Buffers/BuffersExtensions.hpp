// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <algorithm>
#include <optional>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Buffers/ReadOnlySequence.hpp"
#include "System/Buffers/IBufferWriter.hpp"
#include "System/Span.hpp"

namespace System::Buffers {

    using SharpRuntime::intcs;

/**
 * @brief Extension methods for ReadOnlySequence&lt;T&gt; and IBufferWriter&lt;T&gt;.
 *
 * C++ counterpart of .NET System.Buffers.BuffersExtensions.
 * These are free functions (C++ has no extension methods).
 */
namespace BuffersExtensions {

/**
 * @brief Returns the position of the first occurrence of @p value in @p source.
 *
 * C++ counterpart of .NET BuffersExtensions.PositionOf&lt;T&gt;(ReadOnlySequence, T).
 * @param source The sequence to search.
 * @param value  The value to search for.
 * @return The SequencePosition of the first match, or std::nullopt if not found.
 */
template<typename T>
[[nodiscard]] std::optional<System::SequencePosition> PositionOf(
    const ReadOnlySequence<T>& source, const T& value)
{
    long long len = source.getLengthProperty();
    for (long long i = 0; i < len; ++i) {
        auto slice = source.Slice(source.getStartProperty(),
                                  source.GetPosition(i + 1));
        auto arr = slice.ToArray();
        if (!arr.empty() && arr.back() == value) {
            return source.GetPosition(i);
        }
    }
    return std::nullopt;
}

/**
 * @brief Copies the entire @p source sequence into @p destination.
 *
 * C++ counterpart of .NET BuffersExtensions.CopyTo(ReadOnlySequence, Span).
 * @param source      The sequence to copy from.
 * @param destination The span to copy into.
 */
template<typename T>
void CopyTo(const ReadOnlySequence<T>& source, System::Span<T> destination) {
    source.CopyTo(destination);
}

/**
 * @brief Writes the contents of @p value to @p writer.
 *
 * C++ counterpart of .NET BuffersExtensions.Write&lt;T&gt;(IBufferWriter&lt;T&gt;, ReadOnlySpan&lt;T&gt;).
 * Copies via as many GetSpan()/Advance() round-trips as the writer's buffer requires.
 * @param writer The IBufferWriter to write into.
 * @param value  The data to copy.
 */
template<typename T>
void Write(IBufferWriter<T>& writer, System::ReadOnlySpan<T> value) {
    System::Span<T> destination = writer.GetSpan();
    if (value.getLengthProperty() <= destination.getLengthProperty()) {
        value.CopyTo(destination);
        writer.Advance(value.getLengthProperty());
        return;
    }
    System::ReadOnlySpan<T> input = value;
    while (true) {
        intcs writeSize = std::min(destination.getLengthProperty(), input.getLengthProperty());
        input.Slice(0, writeSize).CopyTo(destination);
        writer.Advance(writeSize);
        input = input.Slice(writeSize);
        if (input.getLengthProperty() == 0) break;
        destination = writer.GetSpan(input.getLengthProperty());
    }
}

/**
 * @brief Copies all data from @p source into @p writer.
 *
 * C++ convenience overload (no direct .NET equivalent — .NET's Write only takes
 * a ReadOnlySpan&lt;T&gt;) for writing an entire ReadOnlySequence&lt;T&gt; at once.
 * @param writer The IBufferWriter to write into.
 * @param source The sequence to copy.
 */
template<typename T>
void Write(IBufferWriter<T>& writer, const ReadOnlySequence<T>& source) {
    auto data = source.ToArray();
    if (data.empty()) return;
    Write(writer, System::ReadOnlySpan<T>(data.data(), static_cast<intcs>(data.size())));
}

/**
 * @brief Returns all elements of @p source as a std::vector.
 *
 * C++ counterpart of .NET ReadOnlySequence&lt;T&gt;.ToArray() as an extension.
 */
template<typename T>
[[nodiscard]] std::vector<T> ToArray(const ReadOnlySequence<T>& source) {
    return source.ToArray();
}

} // namespace BuffersExtensions
} // namespace System::Buffers

// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <array>
#include <cstdint>
#include <fstream>
#include <functional>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/ReadOnlyMemory.hpp"
#include "System/IO/Stream.hpp"
#include "System/IO/MemoryStream.hpp"
#include "System/IO/File.hpp"
#include "System/IO/FileNotFoundException.hpp"
#include "System/IO/IOException.hpp"

namespace System {

    using SharpRuntime::intcs;

    /**
     * @brief A lightweight abstraction for a payload of bytes that supports
     * converting between string, bytes, streams, and files.
     *
     * C++ counterpart of .NET System.BinaryData.
     * Owns its byte data via an internal std::vector<uint8_t>.
     * JSON serialization/deserialization methods (FromObjectAsJson, ToObjectFromJson)
     * are omitted as they require runtime type metadata that is out of scope for the
     * game-dev port target.
     *
     * Intentional C++ deviations:
     *  - Equals/operator== compare by content (bytes + media type). .NET BinaryData.Equals
     *    uses reference equality and is marked [EditorBrowsable(Never)].
     *  - GetHashCode is content-based (consistent with content-based Equals).
     */
    class BinaryData {
        std::vector<uint8_t> bytes_;
        std::string          mediaType_;

    public:
        // -----------------------------------------------------------------------
        // Static singleton
        // -----------------------------------------------------------------------

        /** @brief Returns an empty BinaryData instance. */
        [[nodiscard]] static const BinaryData& Empty() {
            static BinaryData instance{std::vector<uint8_t>{}};
            return instance;
        }

        // -----------------------------------------------------------------------
        // Constructors
        // -----------------------------------------------------------------------

        /**
         * @brief Creates a BinaryData instance by copying the provided byte vector.
         * @param data The data to copy.
         */
        explicit BinaryData(const std::vector<uint8_t>& data)
            : bytes_(data) {}

        /**
         * @brief Creates a BinaryData instance by copying the provided byte vector
         * and sets the media type.
         * @param data      The data to copy.
         * @param mediaType MIME type string (e.g. "application/octet-stream").
         */
        BinaryData(const std::vector<uint8_t>& data, std::string mediaType)
            : bytes_(data), mediaType_(std::move(mediaType)) {}

        /**
         * @brief Creates a BinaryData instance by copying the bytes from the provided ReadOnlyMemory.
         * @param data Byte data to copy.
         */
        explicit BinaryData(ReadOnlyMemory<uint8_t> data)
            : bytes_(data.getPointer(), data.getPointer() + data.getLengthProperty()) {}

        /**
         * @brief Creates a BinaryData instance by copying the bytes from the provided ReadOnlyMemory
         * and sets the media type.
         * @param data      Byte data to copy.
         * @param mediaType MIME type string.
         */
        BinaryData(ReadOnlyMemory<uint8_t> data, std::string mediaType)
            : bytes_(data.getPointer(), data.getPointer() + data.getLengthProperty()),
              mediaType_(std::move(mediaType)) {}

        /**
         * @brief Creates a BinaryData instance from a string using UTF-8 encoding.
         * @param data The string data.
         */
        explicit BinaryData(const std::string& data)
            : bytes_(reinterpret_cast<const uint8_t*>(data.data()),
                     reinterpret_cast<const uint8_t*>(data.data()) + data.size()) {}

        /**
         * @brief Creates a BinaryData instance from a string using UTF-8 encoding
         * and sets the media type.
         * @param data      The string data.
         * @param mediaType MIME type string.
         */
        BinaryData(const std::string& data, std::string mediaType)
            : bytes_(reinterpret_cast<const uint8_t*>(data.data()),
                     reinterpret_cast<const uint8_t*>(data.data()) + data.size()),
              mediaType_(std::move(mediaType)) {}

        // -----------------------------------------------------------------------
        // Properties
        // -----------------------------------------------------------------------

        /**
         * @brief Gets the number of bytes in this data.
         * @return Byte count.
         */
        [[nodiscard]] intcs getLengthProperty() const noexcept {
            return static_cast<intcs>(bytes_.size());
        }

        /**
         * @brief Gets a value indicating whether this data is empty.
         * @return true if the length is zero.
         */
        [[nodiscard]] bool getIsEmptyProperty() const noexcept {
            return bytes_.empty();
        }

        /**
         * @brief Gets the MIME type of this data (empty string if not set).
         * @return MIME type string.
         */
        [[nodiscard]] const std::string& getMediaTypeProperty() const noexcept {
            return mediaType_;
        }

        // -----------------------------------------------------------------------
        // Static factory methods
        // -----------------------------------------------------------------------

        /**
         * @brief Creates a BinaryData instance by wrapping the provided ReadOnlyMemory.
         * @param data Byte data to copy.
         * @return New BinaryData instance.
         */
        [[nodiscard]] static BinaryData FromBytes(ReadOnlyMemory<uint8_t> data) {
            return BinaryData(data);
        }

        /**
         * @brief Creates a BinaryData instance by wrapping the provided ReadOnlyMemory
         * and sets the media type.
         * @param data      Byte data to copy.
         * @param mediaType MIME type string.
         * @return New BinaryData instance.
         */
        [[nodiscard]] static BinaryData FromBytes(ReadOnlyMemory<uint8_t> data,
                                                   std::string mediaType) {
            return BinaryData(data, std::move(mediaType));
        }

        /**
         * @brief Creates a BinaryData instance by copying the provided byte vector.
         * @param data The array to copy.
         * @return New BinaryData instance.
         */
        [[nodiscard]] static BinaryData FromBytes(const std::vector<uint8_t>& data) {
            return BinaryData(data);
        }

        /**
         * @brief Creates a BinaryData instance by copying the provided byte vector
         * and sets the media type.
         * @param data      The array to copy.
         * @param mediaType MIME type string.
         * @return New BinaryData instance.
         */
        [[nodiscard]] static BinaryData FromBytes(const std::vector<uint8_t>& data,
                                                   std::string mediaType) {
            return BinaryData(data, std::move(mediaType));
        }

        /**
         * @brief Creates a BinaryData instance from a string using UTF-8 encoding.
         * @param data The string data.
         * @return New BinaryData instance.
         */
        [[nodiscard]] static BinaryData FromString(const std::string& data) {
            return BinaryData(data);
        }

        /**
         * @brief Creates a BinaryData instance from a string using UTF-8 encoding
         * and sets the media type.
         * @param data      The string data.
         * @param mediaType MIME type string.
         * @return New BinaryData instance.
         */
        [[nodiscard]] static BinaryData FromString(const std::string& data,
                                                    std::string mediaType) {
            return BinaryData(data, std::move(mediaType));
        }

        /**
         * @brief Creates a BinaryData instance from the specified file.
         * @param path Path to the file.
         * @return New BinaryData instance containing all bytes from the file.
         * @throws System::IO::FileNotFoundException if the file does not exist.
         * @throws System::IO::IOException if the file exists but cannot be opened.
         */
        [[nodiscard]] static BinaryData FromFile(const std::string& path) {
            return FromFile(path, "");
        }

        /**
         * @brief Creates a BinaryData instance from the specified file and sets the media type.
         * @param path      Path to the file.
         * @param mediaType MIME type string.
         * @return New BinaryData instance containing all bytes from the file.
         * @throws System::IO::FileNotFoundException if the file does not exist.
         * @throws System::IO::IOException if the file exists but cannot be opened.
         */
        [[nodiscard]] static BinaryData FromFile(const std::string& path,
                                                  std::string mediaType) {
            if (!System::IO::File::Exists(path))
                throw System::IO::FileNotFoundException("Unable to find the specified file.", path);
            std::ifstream file(path, std::ios::binary);
            if (!file) throw System::IO::IOException("Failed to open file: " + path);
            std::vector<uint8_t> bytes(
                (std::istreambuf_iterator<char>(file)),
                std::istreambuf_iterator<char>());
            return BinaryData(bytes, std::move(mediaType));
        }

        /**
         * @brief Creates a BinaryData instance by reading all remaining bytes from the stream.
         * The stream is not disposed by this method.
         * @param stream The stream to read from.
         * @return New BinaryData instance containing all bytes read.
         */
        [[nodiscard]] static BinaryData FromStream(System::IO::Stream& stream) {
            return FromStream(stream, "");
        }

        /**
         * @brief Creates a BinaryData instance by reading all remaining bytes from the stream
         * and sets the media type.
         * The stream is not disposed by this method.
         * @param stream    The stream to read from.
         * @param mediaType MIME type string.
         * @return New BinaryData instance containing all bytes read.
         */
        [[nodiscard]] static BinaryData FromStream(System::IO::Stream& stream,
                                                    std::string mediaType) {
            std::vector<uint8_t> bytes;
            std::array<uint8_t, 4096> buf{};
            SharpRuntime::intcs n;
            while ((n = stream.Read(buf.data(), 0,
                                    static_cast<SharpRuntime::intcs>(buf.size()))) > 0) {
                bytes.insert(bytes.end(), buf.begin(), buf.begin() + n);
            }
            return BinaryData(bytes, std::move(mediaType));
        }

        // -----------------------------------------------------------------------
        // Conversion methods
        // -----------------------------------------------------------------------

        /**
         * @brief Converts the value of this instance to a string using UTF-8 decoding.
         * @return A string decoded from the bytes of this instance.
         */
        [[nodiscard]] std::string ToString() const {
            return std::string(reinterpret_cast<const char*>(bytes_.data()), bytes_.size());
        }

        /**
         * @brief Converts the BinaryData to a byte vector (copy).
         * @return A vector containing a copy of the underlying bytes.
         */
        [[nodiscard]] std::vector<uint8_t> ToArray() const {
            return bytes_;
        }

        /**
         * @brief Returns a read-only view over the underlying bytes.
         *
         * The returned ReadOnlyMemory is valid only for the lifetime of this BinaryData instance.
         * @return ReadOnlyMemory&lt;uint8_t&gt; over the backing store.
         */
        [[nodiscard]] ReadOnlyMemory<uint8_t> ToMemory() const noexcept {
            return ReadOnlyMemory<uint8_t>(bytes_.data(), static_cast<intcs>(bytes_.size()));
        }

        /**
         * @brief Converts the BinaryData to a read-only MemoryStream over the underlying bytes.
         *
         * C++ counterpart of .NET BinaryData.ToStream().
         * @return A MemoryStream wrapping a copy of the underlying bytes.
         */
        [[nodiscard]] System::IO::MemoryStream ToStream() const {
            return System::IO::MemoryStream(bytes_.data(),
                                             static_cast<SharpRuntime::intcs>(bytes_.size()));
        }

        /**
         * @brief Returns a new BinaryData that wraps the same bytes with a different media type.
         * @param mediaType New MIME type string.
         * @return New BinaryData with the updated media type.
         */
        [[nodiscard]] BinaryData WithMediaType(std::string mediaType) const {
            return BinaryData(bytes_, std::move(mediaType));
        }

        /**
         * @brief Provides direct access to the underlying bytes.
         *
         * Not present on real .NET's BinaryData (which has no indexer) -- a C++-side
         * convenience addition. Bounds-checked to match this project's other array-like
         * accessors (e.g. Span<T>::operator[], mirroring real .NET's own Span<T>.this[int]):
         * a negative index cast to std::size_t wraps to a huge value, so an unchecked
         * `bytes_[index]` is a real out-of-bounds read (confirmed via ASan heap-buffer-overflow)
         * rather than a caught exception.
         * @param index Zero-based byte index.
         * @return The byte at the specified index.
         * @throws System::ArgumentOutOfRangeException if @p index is negative or ≥ Length.
         */
        [[nodiscard]] uint8_t operator[](intcs index) const {
            if (static_cast<SharpRuntime::uintcs>(index) >= static_cast<SharpRuntime::uintcs>(bytes_.size()))
                throw System::ArgumentOutOfRangeException("index");
            return bytes_[static_cast<std::size_t>(index)];
        }

        /**
         * @brief Defines an implicit conversion from BinaryData to ReadOnlyMemory&lt;uint8_t&gt;.
         *
         * The returned view is valid only for the lifetime of this BinaryData instance.
         * C++ counterpart of .NET implicit operator ReadOnlyMemory&lt;byte&gt;.
         */
        operator ReadOnlyMemory<uint8_t>() const noexcept {
            return ReadOnlyMemory<uint8_t>(bytes_.data(), static_cast<intcs>(bytes_.size()));
        }

        /**
         * @brief Defines an implicit conversion from BinaryData to ReadOnlySpan&lt;uint8_t&gt;.
         *
         * The returned view is valid only for the lifetime of this BinaryData instance.
         * C++ counterpart of .NET implicit operator ReadOnlySpan&lt;byte&gt;.
         */
        operator ReadOnlySpan<uint8_t>() const noexcept {
            return ReadOnlySpan<uint8_t>(bytes_.data(), static_cast<intcs>(bytes_.size()));
        }

        /**
         * @brief Determines whether this instance is equal to another BinaryData by value.
         *
         * Intentional C++ deviation: .NET BinaryData.Equals uses reference equality
         * (ReferenceEquals) and is marked [EditorBrowsable(Never)]. In C++ value semantics
         * are natural, so this compares bytes and media type for content equality.
         * @param other The BinaryData to compare with.
         * @return true if both instances contain the same bytes and media type.
         */
        [[nodiscard]] bool Equals(const BinaryData& other) const {
            if (getLengthProperty() != other.getLengthProperty()) return false;
            if (mediaType_ != other.mediaType_) return false;
            return bytes_ == other.bytes_;
        }

        /** @brief Returns true if both instances have the same bytes and media type. */
        bool operator==(const BinaryData& other) const { return Equals(other); }

        /** @brief Returns true if the instances differ. */
        bool operator!=(const BinaryData& other) const { return !Equals(other); }

        /**
         * @brief Returns a content-based hash code for this instance.
         *
         * Intentional C++ deviation: .NET BinaryData.GetHashCode() uses identity hash
         * (base.GetHashCode(), marked [EditorBrowsable(Never)]). In C++ a content-based
         * hash is more useful and consistent with the content-based Equals above.
         */
        [[nodiscard]] intcs GetHashCode() const noexcept {
            std::size_t h = std::hash<intcs>{}(getLengthProperty());
            h ^= std::hash<std::string>{}(mediaType_) + 0x9e3779b9 + (h << 6) + (h >> 2);
            if (!bytes_.empty()) {
                h ^= std::hash<uint8_t>{}(bytes_[0]) + 0x9e3779b9 + (h << 6) + (h >> 2);
            }
            return static_cast<intcs>(h & 0x7fffffff);
        }
    };

} // namespace System

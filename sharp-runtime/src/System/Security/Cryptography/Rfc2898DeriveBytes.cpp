// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Security/Cryptography/Rfc2898DeriveBytes.hpp"
#include <algorithm>
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Security/Cryptography/CryptographicException.hpp"
#include "System/Security/Cryptography/HMACSHA1.hpp"
#include "System/Security/Cryptography/HMACSHA256.hpp"
#include "System/Security/Cryptography/HMACSHA384.hpp"
#include "System/Security/Cryptography/HMACSHA512.hpp"

namespace System::Security::Cryptography {

namespace {

    intcs blockSizeForHash(const HashAlgorithmName& name) {
        const auto& n = name.getNameProperty();
        if (n == "SHA1") return 20;
        if (n == "SHA256") return 32;
        if (n == "SHA384") return 48;
        if (n == "SHA512") return 64;
        throw CryptographicException("'" + n.value_or("") + "' is not a known hash algorithm.");
    }

} // namespace

std::unique_ptr<HMAC> Rfc2898DeriveBytes::createHmac() const {
    const auto& n = hashAlgorithm_.getNameProperty();
    if (n == "SHA1") return std::make_unique<HMACSHA1>(password_);
    if (n == "SHA256") return std::make_unique<HMACSHA256>(password_);
    if (n == "SHA384") return std::make_unique<HMACSHA384>(password_);
    if (n == "SHA512") return std::make_unique<HMACSHA512>(password_);
    throw CryptographicException("'" + n.value_or("") + "' is not a known hash algorithm.");
}

Rfc2898DeriveBytes::Rfc2898DeriveBytes(std::vector<bytecs> password, std::vector<bytecs> salt, intcs iterations,
                                        HashAlgorithmName hashAlgorithm)
    : password_(std::move(password)), iterations_(static_cast<uint32_t>(iterations)),
      hashAlgorithm_(std::move(hashAlgorithm)) {
    System::ArgumentOutOfRangeException::ThrowIfNegativeOrZero(iterations, "iterations");
    blockSize_ = blockSizeForHash(hashAlgorithm_);
    salt_ = std::move(salt);
    salt_.resize(salt_.size() + 4, bytecs{0});
    initialize();
}

Rfc2898DeriveBytes::Rfc2898DeriveBytes(const std::string& password, std::vector<bytecs> salt, intcs iterations,
                                        HashAlgorithmName hashAlgorithm)
    : Rfc2898DeriveBytes(std::vector<bytecs>(password.begin(), password.end()), std::move(salt), iterations,
                         std::move(hashAlgorithm)) {}

void Rfc2898DeriveBytes::setIterationCountProperty(intcs value) {
    System::ArgumentOutOfRangeException::ThrowIfNegativeOrZero(value, "value");
    iterations_ = static_cast<uint32_t>(value);
    initialize();
}

std::vector<bytecs> Rfc2898DeriveBytes::getSaltProperty() const {
    return std::vector<bytecs>(salt_.begin(), salt_.end() - 4);
}

void Rfc2898DeriveBytes::setSaltProperty(std::vector<bytecs> value) {
    value.resize(value.size() + 4, bytecs{0});
    salt_ = std::move(value);
    initialize();
}

void Rfc2898DeriveBytes::initialize() {
    buffer_.assign(static_cast<size_t>(blockSize_), bytecs{0});
    block_ = 0;
    startIndex_ = endIndex_ = 0;
}

void Rfc2898DeriveBytes::func() {
    if (block_ == 0xFFFFFFFFu) {
        throw CryptographicException("The total number of bytes extracted cannot exceed UInt32.MaxValue * hash length.");
    }

    uint32_t blockIndex = block_ + 1;
    salt_[salt_.size() - 4] = static_cast<bytecs>((blockIndex >> 24) & 0xFF);
    salt_[salt_.size() - 3] = static_cast<bytecs>((blockIndex >> 16) & 0xFF);
    salt_[salt_.size() - 2] = static_cast<bytecs>((blockIndex >> 8) & 0xFF);
    salt_[salt_.size() - 1] = static_cast<bytecs>(blockIndex & 0xFF);

    auto hmac = createHmac();
    std::vector<bytecs> u = hmac->ComputeHash(salt_);
    buffer_ = u;

    for (uint32_t i = 2; i <= iterations_; ++i) {
        u = hmac->ComputeHash(u);
        for (size_t j = 0; j < buffer_.size(); ++j) {
            buffer_[j] = static_cast<bytecs>(buffer_[j] ^ u[j]);
        }
    }

    block_++;
}

std::vector<bytecs> Rfc2898DeriveBytes::GetBytes(intcs cb) {
    System::ArgumentOutOfRangeException::ThrowIfNegativeOrZero(cb, "cb");

    std::vector<bytecs> result(static_cast<size_t>(cb));
    size_t offset = 0;
    size_t size = endIndex_ - startIndex_;

    if (size > 0) {
        if (static_cast<size_t>(cb) >= size) {
            std::copy(buffer_.begin() + startIndex_, buffer_.begin() + endIndex_, result.begin());
            startIndex_ = endIndex_ = 0;
            offset += size;
        } else {
            std::copy(buffer_.begin() + startIndex_, buffer_.begin() + startIndex_ + static_cast<size_t>(cb),
                      result.begin());
            startIndex_ += static_cast<size_t>(cb);
            return result;
        }
    }

    while (offset < static_cast<size_t>(cb)) {
        func();
        size_t remainder = static_cast<size_t>(cb) - offset;
        if (remainder >= static_cast<size_t>(blockSize_)) {
            std::copy(buffer_.begin(), buffer_.end(), result.begin() + offset);
            offset += static_cast<size_t>(blockSize_);
        } else {
            std::copy(buffer_.begin(), buffer_.begin() + remainder, result.begin() + offset);
            startIndex_ = remainder;
            endIndex_ = buffer_.size();
            return result;
        }
    }
    return result;
}

} // namespace System::Security::Cryptography

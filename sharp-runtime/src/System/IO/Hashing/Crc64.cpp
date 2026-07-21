// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/IO/Hashing/Crc64.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentNullException.hpp"

namespace System::IO::Hashing {

    Crc64::Crc64()
        : NonCryptographicHashAlgorithm(Size), parameterSet_(Crc64ParameterSet::getCrc64Property())
    {
        crc_ = parameterSet_->getInitialValueProperty();
    }

    Crc64::Crc64(std::shared_ptr<Crc64ParameterSet> parameterSet)
        : NonCryptographicHashAlgorithm(Size), parameterSet_(std::move(parameterSet))
    {
        if (!parameterSet_) throw System::ArgumentNullException("parameterSet");
        crc_ = parameterSet_->getInitialValueProperty();
    }

    Crc64::Crc64(ulongcs crc, std::shared_ptr<Crc64ParameterSet> parameterSet)
        : NonCryptographicHashAlgorithm(Size), crc_(crc), parameterSet_(std::move(parameterSet)) {}

    void Crc64::Append(const bytecs* source, intcs length) {
        crc_ = parameterSet_->Update(crc_, source, length);
    }

    void Crc64::Reset() {
        crc_ = parameterSet_->getInitialValueProperty();
    }

    void Crc64::GetCurrentHashCore(bytecs* destination) const {
        parameterSet_->WriteCrcToSpan(parameterSet_->Finalize(crc_), destination);
    }

    void Crc64::GetHashAndResetCore(bytecs* destination) {
        parameterSet_->WriteCrcToSpan(parameterSet_->Finalize(crc_), destination);
        crc_ = parameterSet_->getInitialValueProperty();
    }

    ulongcs Crc64::GetCurrentHashAsUInt64() const {
        return parameterSet_->Finalize(crc_);
    }

    std::vector<bytecs> Crc64::Hash(const bytecs* source, intcs length) {
        return Hash(Crc64ParameterSet::getCrc64Property(), source, length);
    }

    std::vector<bytecs> Crc64::Hash(const std::shared_ptr<Crc64ParameterSet>& parameterSet,
                                    const bytecs* source, intcs length) {
        std::vector<bytecs> ret(static_cast<size_t>(Size));
        const ulongcs hash = HashToUInt64(parameterSet, source, length);
        parameterSet->WriteCrcToSpan(hash, ret.data());
        return ret;
    }

    intcs Crc64::Hash(const bytecs* source, intcs length, bytecs* destination, intcs destinationLength) {
        return Hash(Crc64ParameterSet::getCrc64Property(), source, length, destination, destinationLength);
    }

    intcs Crc64::Hash(const std::shared_ptr<Crc64ParameterSet>& parameterSet,
                      const bytecs* source, intcs length, bytecs* destination, intcs destinationLength) {
        if (destinationLength < Size) {
            throw System::ArgumentException("Destination is too short.", "destination");
        }
        const ulongcs hash = HashToUInt64(parameterSet, source, length);
        parameterSet->WriteCrcToSpan(hash, destination);
        return Size;
    }

    bool Crc64::TryHash(const bytecs* source, intcs length, bytecs* destination, intcs destinationLength, intcs& bytesWritten) {
        return TryHash(Crc64ParameterSet::getCrc64Property(), source, length, destination, destinationLength, bytesWritten);
    }

    bool Crc64::TryHash(const std::shared_ptr<Crc64ParameterSet>& parameterSet,
                        const bytecs* source, intcs length, bytecs* destination, intcs destinationLength, intcs& bytesWritten) {
        if (destinationLength < Size) {
            bytesWritten = 0;
            return false;
        }
        const ulongcs hash = HashToUInt64(parameterSet, source, length);
        parameterSet->WriteCrcToSpan(hash, destination);
        bytesWritten = Size;
        return true;
    }

    ulongcs Crc64::HashToUInt64(const bytecs* source, intcs length) {
        return HashToUInt64(Crc64ParameterSet::getCrc64Property(), source, length);
    }

    ulongcs Crc64::HashToUInt64(const std::shared_ptr<Crc64ParameterSet>& parameterSet, const bytecs* source, intcs length) {
        if (!parameterSet) throw System::ArgumentNullException("parameterSet");
        const ulongcs crc = parameterSet->Update(parameterSet->getInitialValueProperty(), source, length);
        return parameterSet->Finalize(crc);
    }

} // namespace System::IO::Hashing

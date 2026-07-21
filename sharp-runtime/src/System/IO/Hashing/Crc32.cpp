// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/IO/Hashing/Crc32.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentNullException.hpp"

namespace System::IO::Hashing {

    Crc32::Crc32()
        : NonCryptographicHashAlgorithm(Size), parameterSet_(Crc32ParameterSet::getCrc32Property())
    {
        crc_ = parameterSet_->getInitialValueProperty();
    }

    Crc32::Crc32(std::shared_ptr<Crc32ParameterSet> parameterSet)
        : NonCryptographicHashAlgorithm(Size), parameterSet_(std::move(parameterSet))
    {
        if (!parameterSet_) throw System::ArgumentNullException("parameterSet");
        crc_ = parameterSet_->getInitialValueProperty();
    }

    Crc32::Crc32(uintcs crc, std::shared_ptr<Crc32ParameterSet> parameterSet)
        : NonCryptographicHashAlgorithm(Size), crc_(crc), parameterSet_(std::move(parameterSet)) {}

    void Crc32::Append(const bytecs* source, intcs length) {
        crc_ = parameterSet_->Update(crc_, source, length);
    }

    void Crc32::Reset() {
        crc_ = parameterSet_->getInitialValueProperty();
    }

    void Crc32::GetCurrentHashCore(bytecs* destination) const {
        parameterSet_->WriteCrcToSpan(parameterSet_->Finalize(crc_), destination);
    }

    void Crc32::GetHashAndResetCore(bytecs* destination) {
        parameterSet_->WriteCrcToSpan(parameterSet_->Finalize(crc_), destination);
        crc_ = parameterSet_->getInitialValueProperty();
    }

    uintcs Crc32::GetCurrentHashAsUInt32() const {
        return parameterSet_->Finalize(crc_);
    }

    std::vector<bytecs> Crc32::Hash(const bytecs* source, intcs length) {
        return Hash(Crc32ParameterSet::getCrc32Property(), source, length);
    }

    std::vector<bytecs> Crc32::Hash(const std::shared_ptr<Crc32ParameterSet>& parameterSet,
                                    const bytecs* source, intcs length) {
        std::vector<bytecs> ret(static_cast<size_t>(Size));
        const uintcs hash = HashToUInt32(parameterSet, source, length);
        parameterSet->WriteCrcToSpan(hash, ret.data());
        return ret;
    }

    intcs Crc32::Hash(const bytecs* source, intcs length, bytecs* destination, intcs destinationLength) {
        return Hash(Crc32ParameterSet::getCrc32Property(), source, length, destination, destinationLength);
    }

    intcs Crc32::Hash(const std::shared_ptr<Crc32ParameterSet>& parameterSet,
                      const bytecs* source, intcs length, bytecs* destination, intcs destinationLength) {
        if (destinationLength < Size) {
            throw System::ArgumentException("Destination is too short.", "destination");
        }
        const uintcs hash = HashToUInt32(parameterSet, source, length);
        parameterSet->WriteCrcToSpan(hash, destination);
        return Size;
    }

    bool Crc32::TryHash(const bytecs* source, intcs length, bytecs* destination, intcs destinationLength, intcs& bytesWritten) {
        return TryHash(Crc32ParameterSet::getCrc32Property(), source, length, destination, destinationLength, bytesWritten);
    }

    bool Crc32::TryHash(const std::shared_ptr<Crc32ParameterSet>& parameterSet,
                        const bytecs* source, intcs length, bytecs* destination, intcs destinationLength, intcs& bytesWritten) {
        if (destinationLength < Size) {
            bytesWritten = 0;
            return false;
        }
        const uintcs hash = HashToUInt32(parameterSet, source, length);
        parameterSet->WriteCrcToSpan(hash, destination);
        bytesWritten = Size;
        return true;
    }

    uintcs Crc32::HashToUInt32(const bytecs* source, intcs length) {
        const auto& parameterSet = Crc32ParameterSet::getCrc32Property();
        return ~parameterSet->Update(parameterSet->getInitialValueProperty(), source, length);
    }

    uintcs Crc32::HashToUInt32(const std::shared_ptr<Crc32ParameterSet>& parameterSet, const bytecs* source, intcs length) {
        if (!parameterSet) throw System::ArgumentNullException("parameterSet");
        const uintcs crc = parameterSet->Update(parameterSet->getInitialValueProperty(), source, length);
        return parameterSet->Finalize(crc);
    }

} // namespace System::IO::Hashing

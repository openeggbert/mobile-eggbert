// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Security/Cryptography/RandomNumberGenerator.hpp"
#include "System/PlatformNotSupportedException.hpp"
#include "System/Security/Cryptography/CryptographicException.hpp"

#if defined(_WIN32)
#include <windows.h>
#include <bcrypt.h>
#elif defined(__EMSCRIPTEN__)
// No secure random source wired up under Emscripten yet.
#else
#include <sys/random.h>
#include <cerrno>
#include <cstring>
#endif

namespace System::Security::Cryptography {

namespace {

    class OsRandomNumberGenerator : public RandomNumberGenerator {
    public:
        void GetBytes(std::vector<bytecs>& data) override {
            if (data.empty()) return;
#if defined(_WIN32)
            NTSTATUS status = BCryptGenRandom(nullptr, data.data(), static_cast<ULONG>(data.size()),
                                               BCRYPT_USE_SYSTEM_PREFERRED_RNG);
            if (status < 0) {
                throw CryptographicException("BCryptGenRandom failed.");
            }
#elif defined(__EMSCRIPTEN__)
            // No secure random source wired up under Emscripten yet — throw a clear exception
            // rather than silently produce weak randomness (e.g. from an unseeded PRNG).
            (void)data;
            throw System::PlatformNotSupportedException(
                "RandomNumberGenerator is not implemented on Emscripten in this runtime.");
#else
            size_t total = 0;
            while (total < data.size()) {
                ssize_t n = ::getrandom(data.data() + total, data.size() - total, 0);
                if (n < 0) {
                    if (errno == EINTR) continue;
                    throw CryptographicException(std::string("getrandom() failed: ") + std::strerror(errno));
                }
                total += static_cast<size_t>(n);
            }
#endif
        }
    };

} // namespace

std::shared_ptr<RandomNumberGenerator> RandomNumberGenerator::Create() {
    return std::make_shared<OsRandomNumberGenerator>();
}

void RandomNumberGenerator::Fill(std::vector<bytecs>& data) {
    static const std::shared_ptr<RandomNumberGenerator> singleton = Create();
    singleton->GetBytes(data);
}

} // namespace System::Security::Cryptography

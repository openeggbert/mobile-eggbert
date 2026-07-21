// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Runtime/InteropServices/RuntimeInformation.hpp"

#if defined(_WIN32)
#include <windows.h>
#elif defined(__EMSCRIPTEN__)
// No uname() under Emscripten.
#else
#include <sys/utsname.h>
#endif

namespace System::Runtime::InteropServices {

Architecture RuntimeInformation::getProcessArchitectureProperty() {
#if defined(__x86_64__) || defined(_M_X64)
    return Architecture::X64;
#elif defined(__i386__) || defined(_M_IX86)
    return Architecture::X86;
#elif defined(__aarch64__) || defined(_M_ARM64)
    return Architecture::Arm64;
#elif defined(__arm__) || defined(_M_ARM)
    return Architecture::Arm;
#elif defined(__wasm__)
    return Architecture::Wasm;
#elif defined(__loongarch64)
    return Architecture::LoongArch64;
#elif defined(__powerpc64__)
    return Architecture::Ppc64le;
#elif defined(__riscv) && __riscv_xlen == 64
    return Architecture::RiscV64;
#else
    return Architecture::X64;
#endif
}

Architecture RuntimeInformation::getOSArchitectureProperty() {
    // Real .NET's OSArchitecture queries the actual OS/kernel architecture (Interop.Sys.
    // GetOSArchitecture(), backed by uname()) rather than the running process' own architecture,
    // falling back to ProcessArchitecture only if that query is unavailable -- this matters for a
    // 32-bit process running under a 64-bit kernel (e.g. via WOW64/multilib), where the two
    // legitimately differ. An earlier version of this port always aliased OSArchitecture to
    // ProcessArchitecture, silently losing that distinction.
#if defined(_WIN32) || defined(__EMSCRIPTEN__)
    return getProcessArchitectureProperty();
#else
    struct utsname info{};
    if (::uname(&info) == 0) {
        std::string machine(info.machine);
        if (machine == "x86_64") return Architecture::X64;
        if (machine == "i386" || machine == "i486" || machine == "i586" || machine == "i686") return Architecture::X86;
        if (machine == "aarch64" || machine == "arm64") return Architecture::Arm64;
        if (machine == "armv6l") return Architecture::Armv6;
        if (machine.substr(0, 3) == "arm") return Architecture::Arm;
        if (machine == "riscv64") return Architecture::RiscV64;
        if (machine == "loongarch64") return Architecture::LoongArch64;
        if (machine == "ppc64le") return Architecture::Ppc64le;
        if (machine == "s390x") return Architecture::S390x;
    }
    return getProcessArchitectureProperty();
#endif
}

bool RuntimeInformation::IsOSPlatform(const OSPlatform& osPlatform) {
#if defined(_WIN32)
    return osPlatform == OSPlatform::Windows;
#elif defined(__APPLE__)
    return osPlatform == OSPlatform::OSX;
#elif defined(__FreeBSD__)
    return osPlatform == OSPlatform::FreeBSD;
#elif defined(__linux__)
    return osPlatform == OSPlatform::Linux;
#else
    (void)osPlatform;
    return false;
#endif
}

std::string RuntimeInformation::getOSDescriptionProperty() {
#if defined(_WIN32)
    return "Microsoft Windows";
#elif defined(__EMSCRIPTEN__)
    return "Emscripten";
#else
    struct utsname info{};
    if (::uname(&info) == 0) {
        return std::string(info.sysname) + " " + info.release;
    }
    return "Unknown";
#endif
}

} // namespace System::Runtime::InteropServices

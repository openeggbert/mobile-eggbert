// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Net/Sockets/NetworkStream.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/IO/IOException.hpp"
#include "System/NotSupportedException.hpp"
#include "System/Net/Sockets/SocketException.hpp"
#include "System/Net/Sockets/detail/ErrnoTranslation.hpp"
#include <exception>

#if defined(_WIN32)
#  include <winsock2.h>
#  if defined(_MSC_VER)
#    pragma comment(lib, "ws2_32.lib")
#  endif
namespace {
    using SockFd = SOCKET;
    inline SockFd toSk(int fd)    { return static_cast<SockFd>(fd); }
    inline bool   validFd(int fd) { return toSk(fd) != INVALID_SOCKET; }
    inline void   closeSk(int fd) { ::closesocket(toSk(fd)); }
    inline int lastErrorCode() { return WSAGetLastError(); }
    inline std::string netErr() {
        char buf[32]; snprintf(buf, sizeof(buf), "WSA error %d", WSAGetLastError()); return buf;
    }
    // Winsock error codes already share SocketError's own numbering (see SocketError.hpp's
    // doc comment) -- no translation needed here.
    inline System::Net::Sockets::SocketError toSocketError(int code) {
        return static_cast<System::Net::Sockets::SocketError>(code);
    }
}
#elif defined(__EMSCRIPTEN__)
#  include "System/PlatformNotSupportedException.hpp"
namespace {
    inline bool validFd([[maybe_unused]] int fd) { return false; }
    inline void closeSk([[maybe_unused]] int fd) {}
}
#else
#  include <sys/socket.h>
#  include <unistd.h>
#  include <cerrno>
#  include <cstring>
namespace {
    using SockFd = int;
    inline SockFd toSk(int fd)    { return fd; }
    inline bool   validFd(int fd) { return fd >= 0; }
    inline void   closeSk(int fd) { ::close(fd); }
    inline int lastErrorCode()    { return errno; }
    inline std::string netErr()   { return std::strerror(errno); }
    // Verified against SocketErrorPal.Unix.cs's GetSocketErrorForNativeError: real .NET
    // translates POSIX errno into the WSA-numbered SocketError space before constructing a
    // SocketException. This port previously cast the raw POSIX errno straight into
    // SocketException's "errorCode" parameter unchanged, so SocketErrorCode never matched any
    // real SocketError value.
    inline System::Net::Sockets::SocketError toSocketError(int code) {
        return SharpRuntimeDetail::Net::Sockets::TranslateErrno(code);
    }
}
#endif

namespace System::Net::Sockets {

NetworkStream::NetworkStream(int fd) : fd_(fd) {}

NetworkStream::~NetworkStream() { Close(); }

intcs NetworkStream::Read(bytecs buffer[], intcs offset, intcs count) {
    // Unlike this codebase's other Stream implementations (e.g. FileStream::Read), this method
    // previously had no argument validation at all -- a negative `count` wrapped to a huge
    // size_t once cast for the recv() call below, letting the kernel write past the end of a
    // small destination buffer (confirmed via an ASan repro: stack-buffer-overflow). Matches
    // real .NET's Stream.ValidateBufferArguments behavior.
    if (offset < 0)
        throw System::ArgumentOutOfRangeException("offset", "Non-negative number required.");
    if (count < 0)
        throw System::ArgumentOutOfRangeException("count", "Non-negative number required.");
#if defined(__EMSCRIPTEN__)
    (void)buffer; (void)offset; (void)count;
    throw System::PlatformNotSupportedException("NetworkStream is not supported on Emscripten.");
#else
    if (count == 0 || !validFd(fd_)) return 0;
    auto n = ::recv(toSk(fd_), reinterpret_cast<char*>(buffer + offset),
                    static_cast<size_t>(count), 0);
    if (n < 0) {
        auto code = lastErrorCode();
        auto err = netErr();
        auto inner = std::make_exception_ptr(SocketException(toSocketError(code), err));
        throw System::IO::IOException("Unable to read data from the transport connection: " + err + ".", inner);
    }
    return static_cast<intcs>(n);
#endif
}

void NetworkStream::Write(const bytecs buffer[], intcs offset, intcs count) {
    if (offset < 0)
        throw System::ArgumentOutOfRangeException("offset", "Non-negative number required.");
    if (count < 0)
        throw System::ArgumentOutOfRangeException("count", "Non-negative number required.");
#if defined(__EMSCRIPTEN__)
    (void)buffer; (void)offset; (void)count;
    throw System::PlatformNotSupportedException("NetworkStream is not supported on Emscripten.");
#else
    if (count == 0 || !validFd(fd_)) return;
    auto n = ::send(toSk(fd_), reinterpret_cast<const char*>(buffer + offset),
                    static_cast<size_t>(count), 0);
    if (n < 0) {
        auto code = lastErrorCode();
        auto err = netErr();
        auto inner = std::make_exception_ptr(SocketException(toSocketError(code), err));
        throw System::IO::IOException("Unable to write data to the transport connection: " + err + ".", inner);
    }
#endif
}

void NetworkStream::Close() {
    if (validFd(fd_)) {
        closeSk(fd_);
        fd_ = -1;
    }
}

intcs NetworkStream::getLengthProperty() const {
    throw System::NotSupportedException("NetworkStream does not support seeking.");
}

} // namespace System::Net::Sockets

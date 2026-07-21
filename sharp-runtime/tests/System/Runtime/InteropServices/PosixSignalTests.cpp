// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include <atomic>
#include <chrono>
#include <csignal>
#include <mutex>
#include <thread>
#include <vector>
#include <unistd.h>
#include "System/ArgumentNullException.hpp"
#include "System/PlatformNotSupportedException.hpp"
#include "System/Runtime/InteropServices/PosixSignalRegistration.hpp"

using namespace System::Runtime::InteropServices;

namespace {
    bool waitFor(std::atomic<int>& counter, int target, int timeoutMs) {
        auto start = std::chrono::steady_clock::now();
        while (counter.load() < target) {
            if (std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - start).count() > timeoutMs) return false;
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        return true;
    }
}

TEST(PosixSignalTests, Create_NullHandler_Throws) {
    EXPECT_THROW(PosixSignalRegistration::Create(PosixSignal::Sigwinch, nullptr), System::ArgumentNullException);
}

TEST(PosixSignalTests, Create_Sigkill_Throws) {
    EXPECT_THROW(PosixSignalRegistration::Create(PosixSignal::Sigkill, [](PosixSignalContext&) {}),
                 System::PlatformNotSupportedException);
}

TEST(PosixSignalTests, HandlerFires_OnMatchingSignal) {
    std::atomic<int> fired{0};
    auto reg = PosixSignalRegistration::Create(PosixSignal::Sigwinch, [&](PosixSignalContext& ctx) {
        EXPECT_EQ(ctx.getSignalProperty(), PosixSignal::Sigwinch);
        fired++;
    });
    kill(getpid(), SIGWINCH);
    EXPECT_TRUE(waitFor(fired, 1, 2000));
}

TEST(PosixSignalTests, MultipleHandlers_FireInReverseRegistrationOrder) {
    std::atomic<int> fired{0};
    std::vector<int> order;
    std::mutex orderMutex;

    // fired++ must be the LAST action in each handler with nothing implicit executing after it
    // (e.g. a lock_guard destructor) -- otherwise the main thread's waitFor(fired, ...) below
    // observes "handler done" while the watcher thread is still finishing trailing cleanup (the
    // mutex unlock), a genuine ThreadSanitizer-flagged data race (2026-07-14) on this test's
    // stack-local orderMutex/order once a later test reuses that stack memory. Scoping the
    // lock_guard so it unlocks BEFORE fired++ makes fired's atomic increment correctly
    // happens-after everything the handler does.
    auto reg1 = PosixSignalRegistration::Create(PosixSignal::Sigwinch, [&](PosixSignalContext&) {
        {
            std::lock_guard<std::mutex> l(orderMutex);
            order.push_back(1);
        }
        fired++;
    });
    auto reg2 = PosixSignalRegistration::Create(PosixSignal::Sigwinch, [&](PosixSignalContext&) {
        {
            std::lock_guard<std::mutex> l(orderMutex);
            order.push_back(2);
        }
        fired++;
    });

    kill(getpid(), SIGWINCH);
    ASSERT_TRUE(waitFor(fired, 2, 2000));
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    ASSERT_EQ(order.size(), 2u);
    EXPECT_EQ(order[0], 2);
    EXPECT_EQ(order[1], 1);
}

TEST(PosixSignalTests, Dispose_UnregistersHandler) {
    std::atomic<int> fired{0};
    auto reg = PosixSignalRegistration::Create(PosixSignal::Sigwinch, [&](PosixSignalContext&) { fired++; });
    reg.Dispose();
    kill(getpid(), SIGWINCH);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_EQ(fired.load(), 0);
}

TEST(PosixSignalTests, Destructor_UnregistersHandler) {
    std::atomic<int> fired{0};
    {
        auto reg = PosixSignalRegistration::Create(PosixSignal::Sigwinch, [&](PosixSignalContext&) { fired++; });
    }
    kill(getpid(), SIGWINCH);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_EQ(fired.load(), 0);
}

// Registers SIGTERM and cancels its default disposition -- if Cancel() didn't actually suppress
// the OS default action (process termination), this test process would be killed before it could
// report a result, which is itself a meaningful (if blunt) failure signal.
TEST(PosixSignalTests, Cancel_SuppressesDefaultDisposition) {
    std::atomic<int> fired{0};
    auto reg = PosixSignalRegistration::Create(PosixSignal::Sigterm, [&](PosixSignalContext& ctx) {
        fired++;
        ctx.setCancelProperty(true);
    });
    kill(getpid(), SIGTERM);
    ASSERT_TRUE(waitFor(fired, 1, 2000));
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    SUCCEED(); // still alive
}

TEST(PosixSignalContextTests, Constructor_SetsSignalProperty) {
    PosixSignalContext ctx(PosixSignal::Sighup);
    EXPECT_EQ(ctx.getSignalProperty(), PosixSignal::Sighup);
    EXPECT_FALSE(ctx.getCancelProperty());
}

TEST(PosixSignalContextTests, SetCancelProperty_RoundTrips) {
    PosixSignalContext ctx(PosixSignal::Sigint);
    ctx.setCancelProperty(true);
    EXPECT_TRUE(ctx.getCancelProperty());
}

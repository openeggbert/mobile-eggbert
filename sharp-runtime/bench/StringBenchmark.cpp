// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Minimal std::chrono-based timing harness for System::String's hot-path operations, added as
// part of the 2026-07-13 performance-audit pilot (NEXT.md). Deliberately dependency-free (no
// vendored benchmarking library) per that task's own scope decision -- this is a standalone tool
// with its own main(), not part of the GoogleTest suite, and is not built by default (see the
// SHARP_RUNTIME_BUILD_BENCHMARKS CMake option). Run manually (use a Release config -- this
// project's default CMake build applies no optimization flags to the SHARP_RUNTIME library
// itself, only to this benchmark's own translation unit, so an unoptimized library build makes
// cross-run/cross-change comparisons meaningless):
//   cmake -S . -B build-bench -DSHARP_RUNTIME_BUILD_BENCHMARKS=ON -DCMAKE_BUILD_TYPE=Release
//   cmake --build build-bench --target SharpRuntimeBench --parallel 4
//   ./build-bench/SharpRuntimeBench
#include <chrono>
#include <cstdio>
#include <string>
#include <vector>

#include "System/String.hpp"

namespace {

using Clock = std::chrono::steady_clock;

template <typename Fn>
double TimeMs(int iterations, Fn&& fn) {
    // volatile accumulator so the compiler can't prove the loop's result is unused and elide it.
    volatile std::size_t sink = 0;
    auto start = Clock::now();
    for (int i = 0; i < iterations; ++i) {
        sink += fn();
    }
    auto end = Clock::now();
    (void)sink;
    return std::chrono::duration<double, std::milli>(end - start).count();
}

void ReportMs(const char* label, double ms, int iterations) {
    std::printf("%-40s %8.2f ms  (%7.1f ns/call)\n", label, ms, ms * 1e6 / iterations);
}

void BenchSplitChar() {
    const std::string csvLine =
        "player_1,42,gold,1234,inventory_slot_7,active,quest_003,dialogue_state_12,x_pos_45.5,y_pos_12.3";
    constexpr int kIterations = 200000;

    double ms = TimeMs(kIterations, [&]() {
        return System::String::Split(csvLine, ',').size();
    });
    ReportMs("String::Split(value, char)", ms, kIterations);
}

void BenchJoin() {
    std::vector<std::string> values;
    for (int i = 0; i < 50; ++i) {
        values.push_back("item_" + std::to_string(i) + "_data");
    }
    constexpr int kIterations = 200000;

    double ms = TimeMs(kIterations, [&]() {
        return System::String::Join(",", values).size();
    });
    ReportMs("String::Join(separator, 50 items)", ms, kIterations);
}

void BenchConcatVector() {
    std::vector<std::string> values;
    for (int i = 0; i < 50; ++i) {
        values.push_back("item_" + std::to_string(i) + "_data");
    }
    constexpr int kIterations = 200000;

    double ms = TimeMs(kIterations, [&]() {
        return System::String::Concat(values).size();
    });
    ReportMs("String::Concat(50-item vector)", ms, kIterations);
}

} // namespace

int main() {
    std::printf("sharp-runtime String hot-path micro-benchmarks\n");
    std::printf("------------------------------------------------------------\n");
    BenchSplitChar();
    BenchJoin();
    BenchConcatVector();
    return 0;
}

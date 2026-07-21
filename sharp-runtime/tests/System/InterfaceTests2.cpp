// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
// Tests for: IFormatProvider, IFormattable, IObservable, IObserver,
//            IParsable, IProgress, IServiceProvider, ISpanFormattable
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <typeinfo>
#include <vector>
#include "System/IFormatProvider.hpp"
#include "System/IFormattable.hpp"
#include "System/IObservable.hpp"
#include "System/IObserver.hpp"
#include "System/IParsable.hpp"
#include "System/IProgress.hpp"
#include "System/IServiceProvider.hpp"
#include "System/ISpanFormattable.hpp"

// ---------------------------------------------------------------------------
// IFormatProvider
// ---------------------------------------------------------------------------
struct NullFormatProvider : public System::IFormatProvider {
    void* GetFormat(const std::type_info&) const override { return nullptr; }
};

TEST(IFormatProviderTests2, GetFormat_ReturnsNull) {
    NullFormatProvider p;
    EXPECT_EQ(p.GetFormat(typeid(int)), nullptr);
}

// ---------------------------------------------------------------------------
// IFormattable
// ---------------------------------------------------------------------------
struct FormattableVal : public System::IFormattable {
    int value;
    explicit FormattableVal(int v) : value(v) {}
    std::string ToString(const std::string& /*fmt*/) const override {
        return std::to_string(value);
    }
};

TEST(IFormattableTests2, ToString_ReturnsString) {
    FormattableVal fv(42);
    EXPECT_EQ(fv.ToString("D"), "42");
}

TEST(IFormattableTests2, ToString_WithProvider_DefaultsToOneArgOverload) {
    FormattableVal fv(42);
    const System::IFormattable& base = fv;
    EXPECT_EQ(base.ToString("D", nullptr), "42");
}

// ---------------------------------------------------------------------------
// IObservable / IObserver
// ---------------------------------------------------------------------------
struct IntObserver2 : public System::IObserver<int> {
    std::vector<int> received;
    bool completed = false;
    bool errored = false;
    void OnNext(const int& v) override { received.push_back(v); }
    void OnCompleted() override { completed = true; }
    void OnError(const std::exception&) override { errored = true; }
};

struct IntObservable2 : public System::IObservable<int> {
    std::vector<std::shared_ptr<System::IObserver<int>>> observers;
    std::shared_ptr<System::IDisposable> Subscribe(
            std::shared_ptr<System::IObserver<int>> obs) override {
        observers.push_back(obs);
        return nullptr;
    }
    void Emit(int v) { for (auto& o : observers) o->OnNext(v); }
    void Complete() { for (auto& o : observers) o->OnCompleted(); }
};

TEST(IObservableTests2, Subscribe_AndReceiveValues) {
    IntObservable2 src;
    auto obs = std::make_shared<IntObserver2>();
    src.Subscribe(obs);
    src.Emit(10); src.Emit(20);
    ASSERT_EQ(obs->received.size(), 2u);
    EXPECT_EQ(obs->received[0], 10);
    EXPECT_EQ(obs->received[1], 20);
}

TEST(IObserverTests2, OnCompleted_SetsFlag) {
    IntObservable2 src;
    auto obs = std::make_shared<IntObserver2>();
    src.Subscribe(obs);
    src.Complete();
    EXPECT_TRUE(obs->completed);
}

// ---------------------------------------------------------------------------
// IParsable
// ---------------------------------------------------------------------------
struct ParseableInt : public System::IParsable<ParseableInt> {
    int value = 0;
    ParseableInt Parse(const std::string& s, const System::IFormatProvider*) override {
        ParseableInt r; r.value = std::stoi(s); return r;
    }
    bool TryParse(const std::string& s, const System::IFormatProvider*, ParseableInt& result) override {
        try { result = Parse(s, nullptr); return true; }
        catch (...) { return false; }
    }
};

TEST(IParsableTests2, Parse_IntFromString) {
    ParseableInt p;
    auto r = p.Parse("99", nullptr);
    EXPECT_EQ(r.value, 99);
}

TEST(IParsableTests2, TryParse_InvalidInput_ReturnsFalse) {
    ParseableInt p;
    ParseableInt r;
    EXPECT_FALSE(p.TryParse("xyz", nullptr, r));
}

// ---------------------------------------------------------------------------
// IProgress
// ---------------------------------------------------------------------------
struct ProgressCapture2 : public System::IProgress<int> {
    std::vector<int> reports;
    void Report(const int& v) override { reports.push_back(v); }
};

TEST(IProgressTests2, Report_CapturesValues) {
    ProgressCapture2 pc;
    pc.Report(10); pc.Report(50); pc.Report(100);
    ASSERT_EQ(pc.reports.size(), 3u);
    EXPECT_EQ(pc.reports[2], 100);
}

// ---------------------------------------------------------------------------
// IServiceProvider
// ---------------------------------------------------------------------------
struct SimpleServiceProvider2 : public System::IServiceProvider {
    void* GetService(const std::type_info&) const override { return nullptr; }
};

TEST(IServiceProviderTests2, GetService_ReturnsNull) {
    SimpleServiceProvider2 sp;
    EXPECT_EQ(sp.GetService(typeid(int)), nullptr);
}

// ---------------------------------------------------------------------------
// ISpanFormattable
// ---------------------------------------------------------------------------
struct SpanFormattableInt2 : public System::ISpanFormattable {
    int value;
    explicit SpanFormattableInt2(int v) : value(v) {}
    std::string ToString(const std::string& /*fmt*/) const override {
        return std::to_string(value);
    }
    bool TryFormat(char* dest, std::size_t destLen,
                   std::size_t& charsWritten,
                   const std::string& /*fmt*/) const override {
        std::string s = std::to_string(value);
        if (s.size() > destLen) { charsWritten = 0; return false; }
        std::copy(s.begin(), s.end(), dest);
        charsWritten = s.size();
        return true;
    }
};

TEST(ISpanFormattableTests2, TryFormat_WritesValue) {
    SpanFormattableInt2 sfi(77);
    char buf[32];
    std::size_t written = 0;
    EXPECT_TRUE(sfi.TryFormat(buf, sizeof(buf), written, ""));
    EXPECT_EQ(std::string(buf, written), "77");
}

TEST(ISpanFormattableTests2, TryFormat_WithProvider_IgnoresProviderAndDelegates) {
    // Default TryFormat(..., provider) implementation forwards to the 4-arg overload
    // without requiring implementers to override it (matching the IFormattable::ToString fix).
    // Invoked through the base interface, since the derived class's 4-arg override
    // otherwise hides the base's 5-arg overload from unqualified name lookup.
    SpanFormattableInt2 sfi(123);
    const System::ISpanFormattable& base = sfi;
    char buf[32];
    std::size_t written = 0;
    EXPECT_TRUE(base.TryFormat(buf, sizeof(buf), written, "", nullptr));
    EXPECT_EQ(std::string(buf, written), "123");
}

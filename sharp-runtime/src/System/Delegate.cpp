// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Delegate.hpp"
#include "System/NotImplementedException.hpp"
#include <functional>
#include <optional>

namespace System {

namespace {
    using PlainFnPtr = void(*)();

    // std::function<void()>::target<PlainFnPtr>() returns a pointer to the INTERNAL STORAGE
    // SLOT holding the wrapped callable, not the wrapped callable's own value -- that slot's
    // address is always different per std::function instance, even when two instances wrap the
    // identical free function (confirmed via a standalone repro). Dereferencing it yields the
    // actual function pointer value, which two distinct std::function objects DO compare equal
    // on when they wrap the same function. Returns nullopt if invoke_ doesn't hold exactly a
    // plain function pointer (e.g. it holds a lambda/closure, which can't be compared this way).
    std::optional<PlainFnPtr> PlainFunctionTarget(const Delegate::ErasedInvoke& invoke) {
        if (const PlainFnPtr* slot = invoke.target<PlainFnPtr>())
            return *slot;
        return std::nullopt;
    }
}

bool Delegate::Equals(const Delegate& other) const {
    // Fast path: same object
    if (this == &other) return true;
    // Compare invocation lists by pointer
    const auto& la = invocationList_;
    const auto& lb = other.invocationList_;
    if (la.empty() && lb.empty()) {
        // Verified against real .NET's documented Delegate.Equals contract: two single-cast
        // delegates are equal when they refer to the same method on the same target (a
        // value-equality comparison), not merely when they're the same object instance. A
        // std::function generically cannot be compared this way for arbitrary callables -- no
        // operator== exists for lambdas/closures with captured state, a fundamental, already-
        // documented C++-vs-C# delegate limitation (see CLAUDE.md) -- but the one mechanically
        // comparable case, both delegates wrapping the identical plain function pointer, is
        // handled here (previously this branch was identity-only unconditionally, which was
        // also inconsistent with GetHashCode() below already assuming function-pointer-based
        // equality was meaningful, even though its own implementation of that assumption was
        // itself broken -- see GetHashCode()'s comment).
        auto ta = PlainFunctionTarget(invoke_);
        auto tb = PlainFunctionTarget(other.invoke_);
        return ta.has_value() && tb.has_value() && *ta == *tb;
    }
    if (la.size() != lb.size()) return false;
    for (std::size_t i = 0; i < la.size(); ++i)
        if (la[i].get() != lb[i].get()) return false;
    return true;
}

std::size_t Delegate::GetHashCode() const noexcept {
    // Empty delegate (no target, no list)
    if (!invoke_ && invocationList_.empty()) return 0;
    // Multicast: fold pointer hashes
    if (!invocationList_.empty()) {
        std::size_t h = 0;
        std::hash<Delegate*> hasher;
        for (const auto& d : invocationList_) {
            std::size_t p = hasher(d.get());
            h ^= p + 0x9e3779b9u + (h << 6) + (h >> 2);
        }
        return h;
    }
    // Single-target: hash the actual wrapped function pointer value when invoke_ holds one, so
    // that two Delegate objects wrapping the same free function hash equal (required: they are
    // now Equals() per the fix above). The previous code hashed invoke_.target<>()'s return
    // value directly -- the internal storage slot's address, not the function pointer's value
    // -- which never actually produced a consistent hash across distinct std::function
    // instances wrapping the same function (confirmed via a standalone repro). See
    // PlainFunctionTarget()'s comment above for the full explanation.
    if (auto target = PlainFunctionTarget(invoke_))
        return std::hash<const void*>{}(reinterpret_cast<const void*>(*target));
    return std::hash<const Delegate*>{}(this);
}

void Delegate::Invoke() const {
    if (!invocationList_.empty()) {
        for (const auto& d : invocationList_) d->Invoke();
    } else if (invoke_) {
        invoke_();
    }
}

bool Delegate::getHasSingleTargetProperty() const {
    return invocationList_.size() <= 1;
}

std::vector<std::shared_ptr<Delegate>> Delegate::GetInvocationList() const {
    if (!invocationList_.empty()) return invocationList_;
    return { const_cast<Delegate*>(this)->shared_from_this() };
}

std::shared_ptr<Delegate> Delegate::Clone() const {
    return std::make_shared<Delegate>(*this);
}

std::any Delegate::DynamicInvoke(const std::vector<std::any>&) {
    throw NotImplementedException("DynamicInvoke is not supported in sharp-runtime");
}

std::shared_ptr<Delegate> Delegate::Combine(
        std::shared_ptr<Delegate> a, std::shared_ptr<Delegate> b) {
    if (!a) return b;
    if (!b) return a;

    std::vector<std::shared_ptr<Delegate>> combined;

    const auto& la = a->invocationList_;
    if (la.empty()) combined.push_back(a);
    else combined.insert(combined.end(), la.begin(), la.end());

    const auto& lb = b->invocationList_;
    if (lb.empty()) combined.push_back(b);
    else combined.insert(combined.end(), lb.begin(), lb.end());

    return std::shared_ptr<Delegate>(new Delegate(MulticastTag{}, std::move(combined)));
}

std::shared_ptr<Delegate> Delegate::Combine(
        const std::vector<std::shared_ptr<Delegate>>& delegates) {
    std::shared_ptr<Delegate> result;
    for (const auto& d : delegates) result = Combine(result, d);
    return result;
}

std::shared_ptr<Delegate> Delegate::Remove(
        std::shared_ptr<Delegate> source, std::shared_ptr<Delegate> value) {
    if (!source) return nullptr;
    if (!value)  return source;

    const auto& sl = source->invocationList_;
    if (sl.empty()) {
        // Single-target delegate
        if (source.get() == value.get() || source->Equals(*value)) return nullptr;
        return source;
    }

    auto list = sl;
    for (SharpRuntime::intcs i = static_cast<SharpRuntime::intcs>(list.size()) - 1; i >= 0; --i) {
        if (list[i].get() == value.get() || list[i]->Equals(*value)) {
            list.erase(list.begin() + i);
            if (list.empty()) return nullptr;
            if (list.size() == 1) return list[0];
            return std::shared_ptr<Delegate>(new Delegate(MulticastTag{}, std::move(list)));
        }
    }
    return source; // Not found — return unchanged
}

std::shared_ptr<Delegate> Delegate::RemoveAll(
        std::shared_ptr<Delegate> source, std::shared_ptr<Delegate> value) {
    std::shared_ptr<Delegate> result = source;
    while (result) {
        auto next = Remove(result, value);
        if (next.get() == result.get()) break; // Nothing removed
        result = next;
    }
    return result;
}

} // namespace System

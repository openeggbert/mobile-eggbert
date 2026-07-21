// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Xml/NameTable.hpp"

namespace System::Xml {

    std::optional<std::string> NameTable::Get(const std::string& array) const {
        auto it = entries_.find(array);
        if (it == entries_.end()) return std::nullopt;
        return *it;
    }

    std::optional<std::string> NameTable::Get(const char* array, SharpRuntime::intcs offset,
                                              SharpRuntime::intcs length) const {
        return Get(std::string(array + offset, static_cast<size_t>(length)));
    }

    std::string NameTable::Add(const std::string& array) {
        return *entries_.insert(array).first;
    }

    std::string NameTable::Add(const char* array, SharpRuntime::intcs offset, SharpRuntime::intcs length) {
        return Add(std::string(array + offset, static_cast<size_t>(length)));
    }

} // namespace System::Xml

// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Xml/XmlQualifiedName.hpp"

#include <functional>

namespace System::Xml {

    const XmlQualifiedName XmlQualifiedName::Empty{};

    XmlQualifiedName::XmlQualifiedName() = default;

    XmlQualifiedName::XmlQualifiedName(const std::string& name) : name_(name) {}

    XmlQualifiedName::XmlQualifiedName(const std::string& name, const std::string& ns)
        : name_(name), ns_(ns) {}

    SharpRuntime::intcs XmlQualifiedName::GetHashCode() const {
        // .NET only hashes Name, not Namespace, "for perf reasons" (see XmlQualifiedName.cs).
        return static_cast<SharpRuntime::intcs>(std::hash<std::string>{}(name_));
    }

    std::string XmlQualifiedName::ToString() const {
        return ToString(name_, ns_);
    }

    std::string XmlQualifiedName::ToString(const std::string& name, const std::string& ns) {
        return ns.empty() ? name : ns + ":" + name;
    }

    bool XmlQualifiedName::Equals(const XmlQualifiedName& other) const {
        return name_ == other.name_ && ns_ == other.ns_;
    }

} // namespace System::Xml

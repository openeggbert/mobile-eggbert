// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Object.hpp"

#include <typeinfo>
#include <functional>
#include <cstdint>

namespace System
{
    Object::Object() = default;

    Object::~Object() = default;

    std::string Object::ToString() const
    {
        return GetTypeName();
    }

    bool Object::Equals(const Object* obj) const
    {
        return this == obj;
    }

    bool Object::Equals(const Object* objA, const Object* objB)
    {
        if (objA == objB)
            return true;
        if (objA == nullptr || objB == nullptr)
            return false;
        return objA->Equals(objB);
    }

    bool Object::ReferenceEquals(const Object* objA, const Object* objB)
    {
        return objA == objB;
    }

    int Object::GetHashCode() const
    {
        const std::size_t hash = std::hash<const void*>{}(this);
        return static_cast<int>(hash & 0x7fffffff);
    }
}

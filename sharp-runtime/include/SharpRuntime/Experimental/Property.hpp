// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#pragma once

/**
 * @brief Experimental property wrapper.
 * @note Status: Complete, but deliberately unused. The getter/setter delegation below is fully
 * implemented (not a stub) -- this type is deprioritized in favor of SharpRuntime::Prop.hpp,
 * which expands to ordinary members and methods without this type's per-instance
 * std::function-based indirection. Kept for reference/experimentation only.
 */
#include <functional>
#include <iostream>
#include <stdexcept>
#include "System/NotSupportedException.hpp"

#define DEF_PROP_AUTO(type, name, init) \
    private: type name##VVVV = init; \
    public: SharpRuntime::Property<type> name;

#define IMPL_PROP_AUTO(type, name)\
name( [this]() { return name##VVVV; } , [this](type v) {name##VVVV = v; })

#define IMPL_PROP_AUTO_READONLY(type, name)\
name( [this]() { return name##VVVV; })

#define DEF_PROP_CUSTOM(type, name) \
SharpRuntime::Property<type> name;

#define IMPL_PROP_CUSTOM(type, name, customGetter, customSetter)\
name( [this]() customGetter, [this](type v) customSetter)

#define IMPL_PROP_CUSTOM_READONLY(type, name, customGetter)\
name( [this]() customGetter )

namespace SharpRuntime::Experimental {
    // Template for Property
    /// Experimental property wrapper that delegates get/set to std::function objects.
    /// Prefer SharpRuntime::Prop.hpp in production code; this class adds per-instance overhead.
    template <typename T>
    class Property {
    public:
        /// @param customGetter  Lambda invoked on every read.
        /// @param customSetter  Lambda invoked on every write (nullptr = read-only).
        Property(std::function<T()> customGetter, std::function<void(const T&)> customSetter = nullptr)
            : getter(customGetter), setter(customSetter) {}

        /// Writes @p value via the custom setter. Throws System::NotSupportedException if no setter was provided.
        T& operator=(const T& value) {
            if (setter) {
                setter(value);
            } else {
                throw System::NotSupportedException("Setter not implemented.");
            }
            return cachedValue;
        }

        /// Reads the current value via the custom getter.
        operator T() const {
            return getter();
        }
        /// @return The current value via the custom getter.
        T get() const {
            return getter();
        }
        /// Writes @p value via the custom setter. Throws System::NotSupportedException if no setter was provided.
        void set(T value) {
            if (setter) {
                setter(value);
            } else {
                throw System::NotSupportedException("Setter not implemented.");
            }
        }

    private:
        std::function<T()> getter;
        std::function<void(const T&)> setter;
        T cachedValue; // For cases when setter stores its own value
    };

}


//// Example usage
//class MyClass {
//public:
//    MyClass()
//        : ReadWriteValue(
//            // Getter
//            [this]() { return value; },
//            // Setter
//            [this](const int& v) { value = v; }),
//          ReadOnlyValue(
//            // Getter
//            [this]() { return readOnlyValue; }
//          )
//    {}
//
//    // Property: Read-Write
//    Property<int> ReadWriteValue;
//
//    // Property: Read-Only
//    Property<int> ReadOnlyValue;
//
//private:
//    int value = 0;
//    int readOnlyValue = 42; // Example of a read-only value
//};
//
//int main() {
//    MyClass obj;
//
//    // Read-Write Property
//    obj.ReadWriteValue = 100; // Sets the value using the custom setter
//    std::cout << "Read-Write Value: " << obj.ReadWriteValue << "\n";
//
//    // Read-Only Property
//    std::cout << "Read-Only Value: " << obj.ReadOnlyValue << "\n";
//
//    // Uncommenting the following line will throw an exception because it's read-only
//    // obj.ReadOnlyValue = 200;
//
//    return 0;
//}

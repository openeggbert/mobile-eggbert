// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#pragma once

/**
 * @brief Experimental read-only property wrapper.
 * @note Status: Complete, but deliberately unused. The getter-only delegation below (with the
 * setter deleted to enforce read-only) is fully implemented (not a stub) -- this type is
 * deprioritized in favor of SharpRuntime::Prop.hpp, which expands to ordinary members and
 * methods without this type's per-instance std::function-based indirection. Kept for
 * reference/experimentation only.
 */

#include <functional>
#include "Property.hpp"

namespace SharpRuntime::Experimental {
    // Derived class: ReadOnlyProperty
    template <typename T>
    class ReadOnlyProperty : public Property<T> {
    public:
        ReadOnlyProperty(std::function<T()> customGetter)
            : Property<T>(customGetter, nullptr) {}

        // Delete setter to enforce read-only
        T& operator=(const T& value) = delete;
    };

}




///// Example usage
//class Owner {
//public:
//    Owner()
//        : Width([this]() { return rightX - leftX; }),
//          Height([this]() { return bottomY - topY; }) {}
//
//    // Properties
//    ReadOnlyProperty<int> Width;
//    ReadOnlyProperty<int> Height;
//
//private:
//    int leftX = 10;
//    int rightX = 50;
//    int topY = 20;
//    int bottomY = 80;
//};
//
//int main() {
//    Owner rect;
//
//    // Access the read-only properties
//    std::cout << "Width: " << rect.Width << "\n";  // Output: 40
//    std::cout << "Height: " << rect.Height << "\n"; // Output: 60
//
//    // Uncommenting the following lines will result in a compilation error due to deleted setter
//    // rect.Width = 100;
//    // rect.Height = 200;
//
//    return 0;
//}

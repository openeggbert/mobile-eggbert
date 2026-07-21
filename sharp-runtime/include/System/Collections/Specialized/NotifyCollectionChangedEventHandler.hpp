// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <functional>
#include "System/Collections/Specialized/NotifyCollectionChangedEventArgs.hpp"

namespace System::Collections::Specialized {

/**
 * @brief The delegate type used for handlers of the CollectionChanged event.
 *
 * C++ counterpart of .NET System.Collections.Specialized.NotifyCollectionChangedEventHandler.
 * .NET's delegate signature is (object? sender, NotifyCollectionChangedEventArgs e); the sender
 * is passed as an untyped void* here since sharp-runtime has no common object base to mirror it.
 *
 * @tparam T The type of elements in the collection raising the event.
 */
template<typename T>
using NotifyCollectionChangedEventHandler = std::function<void(void*, const NotifyCollectionChangedEventArgs<T>&)>;

} // namespace System::Collections::Specialized

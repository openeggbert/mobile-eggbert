// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Collections::Specialized {

/**
 * @brief Describes the action that caused a CollectionChanged event.
 *
 * C++ counterpart of .NET System.Collections.Specialized.NotifyCollectionChangedAction.
 */
enum class NotifyCollectionChangedAction {
    Add     = 0, ///< One or more items were added to the collection.
    Remove  = 1, ///< One or more items were removed from the collection.
    Replace = 2, ///< One or more items were replaced in the collection.
    Move    = 3, ///< One or more items were moved within the collection.
    Reset   = 4  ///< The contents of the collection changed dramatically.
};

} // namespace System::Collections::Specialized

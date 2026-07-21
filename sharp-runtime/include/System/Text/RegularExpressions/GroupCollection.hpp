// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <unordered_map>
#include <vector>
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Text/RegularExpressions/Group.hpp"

namespace System::Text::RegularExpressions {

    /**
     * @brief Represents the set of captured groups in a single match.
     *
     * C++ counterpart of .NET System.Text.RegularExpressions.GroupCollection.
     */
    class GroupCollection {
        std::vector<Group> groups_;
        std::unordered_map<std::string, intcs> nameToIndex_;

    public:
        GroupCollection() = default;
        GroupCollection(std::vector<Group> groups, std::unordered_map<std::string, intcs> nameToIndex)
            : groups_(std::move(groups)), nameToIndex_(std::move(nameToIndex)) {}

        /** @return The number of groups (including group 0, the whole match). */
        [[nodiscard]] intcs getCountProperty() const { return static_cast<intcs>(groups_.size()); }

        /** @return The group at @p index. @throws System::ArgumentOutOfRangeException if out of range. */
        [[nodiscard]] const Group& operator[](intcs index) const {
            if (index < 0 || index >= static_cast<intcs>(groups_.size())) {
                throw System::ArgumentOutOfRangeException("index");
            }
            return groups_[static_cast<size_t>(index)];
        }

        /** @return The named group @p name, or an unsuccessful empty Group if no such named group exists. */
        [[nodiscard]] const Group& operator[](const std::string& name) const {
            auto it = nameToIndex_.find(name);
            if (it == nameToIndex_.end()) {
                static const Group empty = Group::Empty();
                return empty;
            }
            return (*this)[it->second];
        }

        [[nodiscard]] auto begin() const { return groups_.begin(); }
        [[nodiscard]] auto end() const { return groups_.end(); }
    };

} // namespace System::Text::RegularExpressions

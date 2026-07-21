// SPDX-License-Identifier: MS-PL
#include "Microsoft/Xna/Framework/Input/Touch/TouchCollection.hpp"
#include "Microsoft/Xna/Framework/Input/Touch/TouchPanel.hpp"

#include <stdexcept>

namespace Microsoft::Xna::Framework::Input::Touch
{
    TouchCollection::TouchCollection() = default;

    TouchCollection::TouchCollection(const std::vector<TouchLocation>& touches)
        : touches_(touches)
    {
    }

    TouchCollection::TouchCollection(std::vector<TouchLocation>&& touches)
        : touches_(std::move(touches))
    {
    }

    int  TouchCollection::getCountProperty()       const { return static_cast<int>(touches_.size()); }
    bool TouchCollection::getIsConnectedProperty() const { return TouchPanel::getTouchDeviceExistsProperty(); }

    TouchLocation& TouchCollection::operator[](std::size_t index)
    {
        if (index >= touches_.size())
            throw std::out_of_range("TouchCollection index out of range");
        return touches_[index];
    }

    const TouchLocation& TouchCollection::operator[](std::size_t index) const
    {
        if (index >= touches_.size())
            throw std::out_of_range("TouchCollection index out of range");
        return touches_[index];
    }

    std::vector<TouchLocation>::iterator TouchCollection::begin() { return touches_.begin(); }
    std::vector<TouchLocation>::iterator TouchCollection::end()   { return touches_.end(); }

    std::vector<TouchLocation>::const_iterator TouchCollection::begin() const { return touches_.begin(); }
    std::vector<TouchLocation>::const_iterator TouchCollection::end()   const { return touches_.end(); }
}

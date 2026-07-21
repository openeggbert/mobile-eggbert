// SPDX-License-Identifier: MS-PL
#include "CNA/Input/Joysticks.hpp"

namespace CNA::Input
{
    System::MulticastAction<std::uint32_t> Joysticks::ConnectedEXT;
    System::MulticastAction<std::uint32_t> Joysticks::DisconnectedEXT;
}

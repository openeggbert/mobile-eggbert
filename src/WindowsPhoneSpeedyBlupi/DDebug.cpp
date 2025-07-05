#include "WindowsPhoneSpeedyBlupi/DDebug.h"

#include <iostream>

#define DEBUGGING_ENABLED false

namespace WindowsPhoneSpeedyBlupi {
    IDATA(bool, DetailedDebugging, DDebug)

    DDebug::DDebug(bool detailed_debugging)
        : DetailedDebugging_(detailed_debugging) {
    }

    void DDebug::WriteLine(const std::string &msg) {
        if constexpr (DEBUGGING_ENABLED) {
            std::cout << msg;
        }
    }
}

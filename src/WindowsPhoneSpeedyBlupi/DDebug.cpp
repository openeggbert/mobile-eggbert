#include "../../include/WindowsPhoneSpeedyBlupi/DDebug.h"

#include "NeoSdk/Property.h"

namespace WindowsPhoneSpeedyBlupi {
    bool DDebug::detailedDebugging = false;
    void DDebug::WriteLine(std::string &msg) {
        if (detailedDebugging) {
            std::cout << msg;
        }
    }

}

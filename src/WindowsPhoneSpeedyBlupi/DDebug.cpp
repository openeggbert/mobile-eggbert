#include "../../include/WindowsPhoneSpeedyBlupi/DDebug.h"

#include "NeoSdk/Property.h"
#define DEBUGGING_ENABLED false
namespace WindowsPhoneSpeedyBlupi {

    void DDebug::WriteLine(const std::string& msg) {
        if (DEBUGGING_ENABLED) {
            std::cout << msg;
        }
    }

}

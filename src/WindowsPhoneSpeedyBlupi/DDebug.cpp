#include "../../include/WindowsPhoneSpeedyBlupi/DDebug.h"

#include "NeoSdk/Property.h"
#define DEFINE_DEBUGGING false
namespace WindowsPhoneSpeedyBlupi {

    void DDebug::WriteLine(std::string &msg) {
        if (DEFINE_DEBUGGING) {
            std::cout << msg;
        }
    }

}

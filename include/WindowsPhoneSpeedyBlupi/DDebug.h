
#include "NeoSdk/Property.h"

namespace WindowsPhoneSpeedyBlupi
{
    //static class
    class DDebug {
    private: static bool detailedDebugging;
        NeoSdk::Property<bool> DetailedDebugging{ [this]() { return detailedDebugging; } , [this](bool value) {detailedDebugging = value; }};

    public: static void WriteLine(const std::string& msg);
    };
}

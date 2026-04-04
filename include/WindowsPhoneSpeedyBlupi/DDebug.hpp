#ifndef DDEBUG_H
#define DDEBUG_H
#include <string>

#include "CNA/Prop.hpp"

namespace WindowsPhoneSpeedyBlupi {
    //static class
    class DDebug {
        DDATA(bool, DetailedDebugging)

    public:
        explicit DDebug(bool detailed_debugging);

        static void WriteLine(const std::string &msg);
    };
}
#endif // DDEBUG_H

//
// Created by robertvokac on 5/28/25.
//

#ifndef HELPER_H
#define HELPER_H
#include <regex>
#include <string>
#define ToString(a) std::to_string(a)
#define STRING_VECTOR(items) std::vector<string>{items}

namespace WindowsPhoneSpeedyBlupi {
    class Helper {
    public:
        static std::string formatString(const std::string &format, const std::vector<std::string> &args);

        static std::vector<std::string> split(const std::string &text, char delimiter);
    };
}


#endif //HELPER_H

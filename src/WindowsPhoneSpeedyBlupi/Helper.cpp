//
// Created by robertvokac on 5/28/25.
//

#include "WindowsPhoneSpeedyBlupi/Helper.hpp"
namespace WindowsPhoneSpeedyBlupi {
    std::string Helper::formatString(const std::string& format, const std::vector<std::string>& args) {
        std::string result = format;
        for (size_t i = 0; i < args.size(); ++i) {
            std::string placeholder = "\\{" + std::to_string(i) + "\\}";
            result = std::regex_replace(result, std::regex(placeholder), args[i]);
        }
        return result;
    }

}

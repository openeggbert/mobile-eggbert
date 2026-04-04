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
    std::vector<std::string> Helper::split(const std::string& text, char delimiter) {
        std::vector<std::string> result;
        std::stringstream ss(text);
        std::string item;

        while (std::getline(ss, item, delimiter)) {
            result.push_back(item);
        }

        return result;
    }

}

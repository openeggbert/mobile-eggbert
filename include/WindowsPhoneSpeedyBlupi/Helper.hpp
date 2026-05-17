//
// Created by robertvokac on 5/28/25.
//

#ifndef HELPER_H
#define HELPER_H
#include <regex>
#include <string>
#define TO_STRING(a) std::to_string(a)
#define STRING_VECTOR(items) std::vector<string>{items}

namespace WindowsPhoneSpeedyBlupi
{
    /**
     * @brief Static utility class providing string formatting helpers.
     *
     * Helper supplements the standard library with convenience methods used
     * throughout the port, primarily for producing human-readable strings from
     * numeric arguments. It is not part of the original game logic.
     */
    class Helper
    {
    public:
        /**
         * @brief Formats a string by substituting {0}, {1}, ... placeholders with args.
         *
         * Mimics the behaviour of C# String.Format for simple indexed placeholders.
         * Out-of-range placeholder indices are left as-is in the output.
         *
         * @param format Template string with {N} placeholders.
         * @param args   Vector of replacement strings in index order.
         * @return Formatted string with all recognised placeholders replaced.
         */
        static std::string formatString(const std::string& format, const std::vector<std::string>& args);
    };
}


#endif //HELPER_H

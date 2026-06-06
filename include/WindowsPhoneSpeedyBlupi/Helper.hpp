/**
 * @file Helper.hpp
 * @brief Declarations for the Helper static utility class and related macros.
 * @details Provides a lightweight C#-style string formatter and convenience
 *          macros used throughout the port.  This file is not part of the
 *          original game logic; it was added during the C++ porting effort.
 */

//
// Created by robertvokac on 5/28/25.
//

#ifndef HELPER_H
#define HELPER_H
#include <regex>
#include <string>

/**
 * @def TO_STRING(a)
 * @brief Converts a numeric value to its std::string representation.
 * @details Thin wrapper around std::to_string() for brevity in call sites.
 * @param a Value to convert.
 */
#define TO_STRING(a) std::to_string(a)

/**
 * @def STRING_VECTOR(items)
 * @brief Creates a std::vector<string> from a brace-enclosed initialiser list.
 * @details Intended for single-expression use at call sites of Helper::formatString().
 * @param items Comma-separated string literals or std::string values.
 */
#define STRING_VECTOR(items) std::vector<string>{items}

namespace WindowsPhoneSpeedyBlupi
{
    /**
     * @class Helper
     * @brief Static utility class providing string formatting helpers.
     *
     * @details Helper supplements the standard library with convenience methods
     *          used throughout the port, primarily for producing human-readable
     *          strings from mixed arguments.  It mimics a small subset of the
     *          C# @c String.Format API.  This class is not part of the original
     *          game logic; it was introduced during the C++ porting effort.
     */
    class Helper
    {
    public:
        /**
         * @brief Formats a string by substituting @c {N} placeholders with @p args.
         *
         * @details Iterates over @p args in index order.  For each element at index
         *          @c i, all occurrences of the literal token @c {i} in @p format
         *          are replaced with @c args[i].  If the format string contains a
         *          placeholder whose index is greater than or equal to
         *          @c args.size(), that placeholder is left unchanged in the output
         *          (no exception is thrown and no truncation occurs).
         *
         * @param[in] format Template string containing @c {0}, @c {1}, … tokens.
         * @param[in] args   Ordered replacement strings; index @c i replaces @c {i}.
         * @return Formatted string with all recognised placeholders substituted.
         *
         * @note Out-of-range placeholder indices are silently preserved as-is.
         */
        static std::string formatString(const std::string& format, const std::vector<std::string>& args);
    };
}


#endif //HELPER_H

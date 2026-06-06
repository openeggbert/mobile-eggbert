/**
 * @file Helper.cpp
 * @brief Implementation of Helper::formatString().
 * @details See Helper.hpp for the full API contract.
 *
 * ### Edge-case behaviour for out-of-range placeholder indices
 * The implementation iterates only over indices 0 … args.size()-1.
 * Any placeholder @c {N} in @p format whose index @c N is greater than or
 * equal to @c args.size() is never visited and is therefore left verbatim in
 * the returned string.  No exception is thrown and no empty string is
 * substituted — the original token is preserved unchanged.
 *
 * ### Multiple occurrences of the same placeholder
 * Each placeholder token @c {N} is replaced in a single pass using
 * @c std::string::find in a loop.  All occurrences are replaced.  The search
 * position advances past each replacement to avoid re-scanning already-
 * substituted text, so self-referential placeholders (e.g. a replacement that
 * itself contains @c {N}) are not processed recursively.
 */

//
// Created by robertvokac on 5/28/25.
//

#include "WindowsPhoneSpeedyBlupi/Helper.hpp"
namespace WindowsPhoneSpeedyBlupi {
    std::string Helper::formatString(
        const std::string& format,
        const std::vector<std::string>& args
    ) {
        std::string result = format;

        for (size_t i = 0; i < args.size(); ++i) {
            std::string placeholder = "{" + std::to_string(i) + "}";

            size_t pos = 0;
            while ((pos = result.find(placeholder, pos)) != std::string::npos) {
                result.replace(pos, placeholder.length(), args[i]);
                pos += args[i].length();
            }
        }

        return result;
    }

}

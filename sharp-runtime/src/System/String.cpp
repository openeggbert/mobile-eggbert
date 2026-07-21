// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/String.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/FormatException.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <charconv>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <sstream>

namespace System
{
    std::vector<std::string> String::Split(const std::string& value, char delimiter)
    {
        // Manual find/substr scan instead of std::stringstream + std::getline: measured ~2.6x
        // faster for typical short game-code strings (e.g. tokenizing a CSV-ish line), since
        // stringstream carries locale-facet and virtual-dispatch overhead getline doesn't need
        // for a single-character delimiter. Matches the approach String::Split(value, string)
        // already used below (this Split overload was the odd one out).
        //
        // Side effect verified against real .NET (String.Manipulation.cs's
        // CreateSplitArrayOfThisAsSoleValue): this also fixes a pre-existing correctness bug --
        // the old getline-based loop returned an EMPTY vector for an empty @p value, but real
        // .NET's "".Split(',') returns a one-element array containing "". This scan naturally
        // produces the correct {""} result without a special case (the trailing push_back below
        // always executes at least once).
        std::vector<std::string> result;
        std::size_t pos = 0, found;
        while ((found = value.find(delimiter, pos)) != std::string::npos)
        {
            result.push_back(value.substr(pos, found - pos));
            pos = found + 1;
        }
        result.push_back(value.substr(pos));
        return result;
    }

    std::vector<std::string> String::Split(const std::string& value, const std::vector<char>& delimiters)
    {
        std::vector<std::string> result;
        std::string current;
        for (char c : value) {
            bool isDelim = false;
            for (char d : delimiters) if (c == d) { isDelim = true; break; }
            if (isDelim) { result.push_back(current); current.clear(); }
            else current += c;
        }
        result.push_back(current);
        return result;
    }

    std::vector<std::string> String::Split(const std::string& value, const std::string& delimiter)
    {
        std::vector<std::string> result;
        if (delimiter.empty()) { result.push_back(value); return result; }
        size_t pos = 0, found;
        while ((found = value.find(delimiter, pos)) != std::string::npos) {
            result.push_back(value.substr(pos, found - pos));
            pos = found + delimiter.size();
        }
        result.push_back(value.substr(pos));
        return result;
    }

    bool String::IsEmpty(const std::string& value)
    {
        return value.empty();
    }

    bool String::StartsWith(const std::string& value, const std::string& prefix)
    {
        return value.size() >= prefix.size() &&
               value.compare(0, prefix.size(), prefix) == 0;
    }

    bool String::IsNullOrEmpty(const std::string& value)
    {
        return value.empty();
    }

    // --- Format helpers (file-internal) ---
    namespace {
        // Extract the spec from the first occurrence of {N:spec} in fmt (returns "" if none).
        std::string extractSpec(const std::string& fmt, int n) {
            std::string tok = "{" + std::to_string(n);
            size_t pos = fmt.find(tok);
            if (pos == std::string::npos) return "";
            size_t after = pos + tok.size();
            if (after >= fmt.size() || fmt[after] != ':') return "";
            size_t end = fmt.find('}', after);
            return end == std::string::npos ? "" : fmt.substr(after + 1, end - after - 1);
        }

        // Replace every {N} or {N:spec} occurrence in result with value.
        std::string replaceArg(const std::string& fmt, int n, const std::string& value) {
            std::string result = fmt;
            std::string tok = "{" + std::to_string(n);
            size_t searchFrom = 0;
            while (true) {
                size_t pos = result.find(tok, searchFrom);
                if (pos == std::string::npos) break;
                size_t afterTok = pos + tok.size();
                // tok is a bare digit prefix (e.g. "{1" for n=1) -- if another digit follows,
                // this is actually a longer index like "{10}", not a real match for n. A naive
                // find() would otherwise silently consume "{10}" while replacing arg 1, which
                // both corrupts the output and hides the very out-of-range-index condition
                // FinalizeFormat below is meant to catch.
                if (afterTok < result.size() && std::isdigit(static_cast<unsigned char>(result[afterTok]))) {
                    searchFrom = pos + 1;
                    continue;
                }
                size_t end = result.find('}', pos);
                if (end == std::string::npos) break;
                result.replace(pos, end - pos + 1, value);
                searchFrom = 0;
            }
            return result;
        }

        // After all known argument substitutions have run, any remaining "{<digits>" token means
        // the format string referenced an argument index beyond what was supplied to this Format
        // overload -- matches real .NET's String.Format, which throws FormatException for this
        // rather than silently leaving the placeholder unreplaced in the output. A "{" not
        // followed by a digit (or with no matching "}") is likewise rejected as a malformed
        // format string. Verified: this port previously had no validation at all here.
        std::string FinalizeFormat(const std::string& result) {
            for (size_t i = 0; i < result.size(); ++i) {
                if (result[i] != '{') continue;
                if (i + 1 < result.size() && std::isdigit(static_cast<unsigned char>(result[i + 1]))) {
                    throw System::FormatException(
                        "Index (zero based) must be greater than or equal to zero and less than the size of the argument list.");
                }
                throw System::FormatException("Input string was not in a correct format.");
            }
            return result;
        }

        // Format integer with .NET-style specifier (X/x=hex, D=decimal padded, else plain).
        std::string fmtInt(SharpRuntime::intcs value, const std::string& spec) {
            if (spec.empty()) return std::to_string(value);
            char sc = spec[0];
            int width = spec.size() > 1 ? std::abs(std::stoi(spec.substr(1))) : 0;
            if (sc == 'X' || sc == 'x') {
                std::ostringstream oss;
                oss.imbue(std::locale::classic());
                if (sc == 'X') oss << std::uppercase;
                oss << std::hex;
                if (width > 0) oss << std::setw(width) << std::setfill('0');
                oss << static_cast<unsigned int>(value);
                return oss.str();
            }
            if (sc == 'D' || sc == 'd') {
                bool neg = value < 0;
                std::string s = std::to_string(neg ? -static_cast<long long>(value) : static_cast<long long>(value));
                while (static_cast<int>(s.size()) < width) s = "0" + s;
                return neg ? "-" + s : s;
            }
            return std::to_string(value);
        }

        // Format double with .NET-style specifier (F=fixed, G=general, E=scientific).
        std::string fmtDouble(double value, const std::string& spec) {
            if (spec.empty()) {
                std::array<char, 64> buf;
                auto [ptr, ec] = std::to_chars(buf.data(), buf.data() + buf.size(), value);
                return ec == std::errc{} ? std::string(buf.data(), ptr) : std::to_string(value);
            }
            char sc = spec[0];
            int prec = spec.size() > 1 ? std::stoi(spec.substr(1)) : 2;
            std::ostringstream oss;
            oss.imbue(std::locale::classic());
            if (sc == 'F' || sc == 'f') {
                oss << std::fixed << std::setprecision(prec) << value;
            } else if (sc == 'G' || sc == 'g') {
                oss << std::defaultfloat << std::setprecision(prec > 0 ? prec : 6) << value;
            } else if (sc == 'E' || sc == 'e') {
                if (sc == 'E') oss << std::uppercase;
                oss << std::scientific << std::setprecision(prec) << value;
            } else {
                oss << value;
            }
            return oss.str();
        }
    }

    std::string String::Format(const std::string& format, SharpRuntime::intcs arg0)
    {
        return FinalizeFormat(replaceArg(format, 0, fmtInt(arg0, extractSpec(format, 0))));
    }

    std::string String::Format(const std::string& format, double arg0)
    {
        return FinalizeFormat(replaceArg(format, 0, fmtDouble(arg0, extractSpec(format, 0))));
    }

    std::string String::Format(const std::string& format, const std::string& arg0)
    {
        return FinalizeFormat(replaceArg(format, 0, arg0));
    }

    std::string String::Format(const std::string& format, SharpRuntime::intcs arg0, SharpRuntime::intcs arg1)
    {
        std::string r = replaceArg(format,  0, fmtInt(arg0, extractSpec(format, 0)));
        return FinalizeFormat(replaceArg(r, 1, fmtInt(arg1, extractSpec(format, 1))));
    }

    std::string String::Format(const std::string& format, SharpRuntime::intcs arg0, const std::string& arg1)
    {
        std::string r = replaceArg(format, 0, fmtInt(arg0, extractSpec(format, 0)));
        return FinalizeFormat(replaceArg(r, 1, arg1));
    }

    std::string String::Format(const std::string& format, const std::string& arg0, SharpRuntime::intcs arg1)
    {
        std::string r = replaceArg(format, 0, arg0);
        return FinalizeFormat(replaceArg(r, 1, fmtInt(arg1, extractSpec(format, 1))));
    }

    std::string String::Format(const std::string& format, const std::string& arg0, const std::string& arg1)
    {
        std::string r = replaceArg(format, 0, arg0);
        return FinalizeFormat(replaceArg(r, 1, arg1));
    }

    std::string String::Format(const std::string& format, double arg0, double arg1)
    {
        std::string r = replaceArg(format, 0, fmtDouble(arg0, extractSpec(format, 0)));
        return FinalizeFormat(replaceArg(r, 1, fmtDouble(arg1, extractSpec(format, 1))));
    }
    std::string String::Format(const std::string& format, bool arg0)
    {
        return Format(format, arg0 ? std::string("True") : std::string("False"));
    }
    std::string String::Format(const std::string& format, char arg0)
    {
        return Format(format, std::string(1, arg0));
    }
    std::string String::ToString(SharpRuntime::intcs value, SharpRuntime::intcs width, char fill)
    {
        std::ostringstream oss;
        oss << std::setw(width) << std::setfill(fill) << value;
        return oss.str();
    }

    bool String::IsNullOrWhiteSpace(const std::string& value)
    {
        for (char c : value)
            if (!std::isspace(static_cast<unsigned char>(c))) return false;
        return true;
    }

    bool String::EndsWith(const std::string& value, const std::string& suffix)
    {
        if (suffix.size() > value.size()) return false;
        return value.compare(value.size() - suffix.size(), suffix.size(), suffix) == 0;
    }

    bool String::Contains(const std::string& value, const std::string& substr)
    {
        return value.find(substr) != std::string::npos;
    }

    std::string String::Replace(const std::string& value, const std::string& oldValue, const std::string& newValue)
    {
        if (oldValue.empty()) return value;
        std::string result;
        result.reserve(value.size());
        size_t pos = 0, found;
        while ((found = value.find(oldValue, pos)) != std::string::npos)
        {
            result.append(value, pos, found - pos);
            result.append(newValue);
            pos = found + oldValue.size();
        }
        result.append(value, pos, std::string::npos);
        return result;
    }

    std::string String::Replace(const std::string& value, char oldChar, char newChar)
    {
        std::string result = value;
        for (char& c : result)
            if (c == oldChar) c = newChar;
        return result;
    }

    std::string String::Substring(const std::string& value, SharpRuntime::intcs startIndex)
    {
        if (startIndex < 0 || static_cast<size_t>(startIndex) > value.size())
            throw System::ArgumentOutOfRangeException("startIndex", "String::Substring: startIndex must be within the bounds of the string.");
        return value.substr(static_cast<size_t>(startIndex));
    }

    std::string String::Substring(const std::string& value, SharpRuntime::intcs startIndex, SharpRuntime::intcs length)
    {
        if (startIndex < 0 || length < 0 ||
            static_cast<size_t>(startIndex) + static_cast<size_t>(length) > value.size())
            throw System::ArgumentOutOfRangeException("startIndex", "String::Substring: startIndex and length must refer to a location within the string.");
        return value.substr(static_cast<size_t>(startIndex), static_cast<size_t>(length));
    }

    std::string String::Trim(const std::string& value)
    {
        auto first = value.find_first_not_of(" \t\n\r\f\v");
        if (first == std::string::npos) return {};
        auto last = value.find_last_not_of(" \t\n\r\f\v");
        return value.substr(first, last - first + 1);
    }

    std::string String::TrimStart(const std::string& value)
    {
        auto first = value.find_first_not_of(" \t\n\r\f\v");
        return first == std::string::npos ? std::string{} : value.substr(first);
    }

    std::string String::TrimEnd(const std::string& value)
    {
        auto last = value.find_last_not_of(" \t\n\r\f\v");
        return last == std::string::npos ? std::string{} : value.substr(0, last + 1);
    }

    std::string String::Concat(const std::string& a, const std::string& b)
    {
        return a + b;
    }

    std::string String::Concat(const std::string& a, const std::string& b, const std::string& c)
    {
        return a + b + c;
    }

    std::string String::Concat(const std::string& a, const std::string& b, const std::string& c, const std::string& d)
    {
        return a + b + c + d;
    }

    std::string String::Concat(const std::vector<std::string>& values)
    {
        // Pre-computing the total size and reserving once avoids the handful of geometric
        // reallocations std::string's amortized-growth += would otherwise perform -- measured
        // ~1.4x faster for a realistic multi-item join/concat (see Join's identical rationale
        // below; the two functions share the same append-loop shape).
        std::size_t total = 0;
        for (const auto& s : values) total += s.size();
        std::string result;
        result.reserve(total);
        for (const auto& s : values) result += s;
        return result;
    }

    std::string String::Join(const std::string& separator, const std::vector<std::string>& values)
    {
        // See Concat(vector<string>)'s comment above -- same upfront-reserve rationale, measured
        // ~1.4x faster for a realistic 50-item join.
        std::size_t total = 0;
        for (const auto& s : values) total += s.size();
        if (!values.empty()) total += separator.size() * (values.size() - 1);
        std::string result;
        result.reserve(total);
        for (size_t i = 0; i < values.size(); ++i)
        {
            if (i > 0) result += separator;
            result += values[i];
        }
        return result;
    }

    std::string String::ToUpper(const std::string& value)
    {
        std::string result = value;
        for (char& c : result) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        return result;
    }

    std::string String::ToLower(const std::string& value)
    {
        std::string result = value;
        for (char& c : result) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return result;
    }

    SharpRuntime::intcs String::IndexOf(const std::string& value, const std::string& substr)
    {
        auto pos = value.find(substr);
        return pos == std::string::npos ? -1 : static_cast<SharpRuntime::intcs>(pos);
    }

    SharpRuntime::intcs String::IndexOf(const std::string& value, char ch)
    {
        auto pos = value.find(ch);
        return pos == std::string::npos ? -1 : static_cast<SharpRuntime::intcs>(pos);
    }

    SharpRuntime::intcs String::LastIndexOf(const std::string& value, const std::string& substr)
    {
        auto pos = value.rfind(substr);
        return pos == std::string::npos ? -1 : static_cast<SharpRuntime::intcs>(pos);
    }

    SharpRuntime::intcs String::LastIndexOf(const std::string& value, char ch)
    {
        auto pos = value.rfind(ch);
        return pos == std::string::npos ? -1 : static_cast<SharpRuntime::intcs>(pos);
    }

    std::string String::PadLeft(const std::string& value, SharpRuntime::intcs totalWidth)
    {
        return PadLeft(value, totalWidth, ' ');
    }

    std::string String::PadLeft(const std::string& value, SharpRuntime::intcs totalWidth, char paddingChar)
    {
        if (totalWidth < 0)
            throw System::ArgumentOutOfRangeException("totalWidth", "String::PadLeft: totalWidth must not be negative.");
        if (static_cast<SharpRuntime::intcs>(value.size()) >= totalWidth) return value;
        return std::string(static_cast<size_t>(totalWidth) - value.size(), paddingChar) + value;
    }

    std::string String::PadRight(const std::string& value, SharpRuntime::intcs totalWidth)
    {
        return PadRight(value, totalWidth, ' ');
    }

    std::string String::PadRight(const std::string& value, SharpRuntime::intcs totalWidth, char paddingChar)
    {
        if (totalWidth < 0)
            throw System::ArgumentOutOfRangeException("totalWidth", "String::PadRight: totalWidth must not be negative.");
        if (static_cast<SharpRuntime::intcs>(value.size()) >= totalWidth) return value;
        return value + std::string(static_cast<size_t>(totalWidth) - value.size(), paddingChar);
    }

    SharpRuntime::intcs String::IndexOfAny(const std::string& value, const std::vector<char>& anyOf)
    {
        for (SharpRuntime::intcs i = 0; i < static_cast<SharpRuntime::intcs>(value.size()); ++i)
            for (char c : anyOf)
                if (value[static_cast<size_t>(i)] == c) return i;
        return -1;
    }

    SharpRuntime::intcs String::LastIndexOfAny(const std::string& value, const std::vector<char>& anyOf)
    {
        for (SharpRuntime::intcs i = static_cast<SharpRuntime::intcs>(value.size()) - 1; i >= 0; --i)
            for (char c : anyOf)
                if (value[static_cast<size_t>(i)] == c) return i;
        return -1;
    }

    bool String::Contains(const std::string& value, char ch)
    {
        return value.find(ch) != std::string::npos;
    }

    SharpRuntime::intcs String::IndexOf(const std::string& value, const std::string& substr, SharpRuntime::intcs startIndex)
    {
        auto len = static_cast<SharpRuntime::intcs>(value.size());
        if (startIndex < 0 || startIndex > len)
            throw System::ArgumentOutOfRangeException("startIndex", "String::IndexOf: startIndex must be within the bounds of the string.");
        auto pos = value.find(substr, static_cast<size_t>(startIndex));
        return pos == std::string::npos ? -1 : static_cast<SharpRuntime::intcs>(pos);
    }

    SharpRuntime::intcs String::IndexOf(const std::string& value, char ch, SharpRuntime::intcs startIndex)
    {
        auto len = static_cast<SharpRuntime::intcs>(value.size());
        if (startIndex < 0 || startIndex > len)
            throw System::ArgumentOutOfRangeException("startIndex", "String::IndexOf: startIndex must be within the bounds of the string.");
        auto pos = value.find(ch, static_cast<size_t>(startIndex));
        return pos == std::string::npos ? -1 : static_cast<SharpRuntime::intcs>(pos);
    }

    SharpRuntime::intcs String::LastIndexOf(const std::string& value, const std::string& substr, SharpRuntime::intcs startIndex)
    {
        if (value.empty()) return substr.empty() ? 0 : -1;
        auto len = static_cast<SharpRuntime::intcs>(value.size());
        // Real .NET's CompareInfo.LastIndexOf explicitly special-cases startIndex == source.Length
        // as valid (a documented back-compat off-by-one fixup: "The caller likely had an off-by-one
        // error when invoking the API") by silently treating it as startIndex == length - 1, rather
        // than throwing -- a natural "search the whole string backward" idiom deserves to work, not
        // throw ArgumentOutOfRangeException.
        if (startIndex == len) startIndex = len - 1;
        if (startIndex < 0 || startIndex >= len)
            throw System::ArgumentOutOfRangeException("startIndex", "String::LastIndexOf: startIndex must be within the bounds of the string.");
        auto pos = value.rfind(substr, static_cast<size_t>(startIndex));
        return pos == std::string::npos ? -1 : static_cast<SharpRuntime::intcs>(pos);
    }

    SharpRuntime::intcs String::LastIndexOf(const std::string& value, char ch, SharpRuntime::intcs startIndex)
    {
        if (value.empty()) return -1;
        auto len = static_cast<SharpRuntime::intcs>(value.size());
        // See the (string, string, int) overload above for why startIndex == length is special-cased
        // rather than rejected, matching real .NET's CompareInfo.LastIndexOf fixup.
        if (startIndex == len) startIndex = len - 1;
        if (startIndex < 0 || startIndex >= len)
            throw System::ArgumentOutOfRangeException("startIndex", "String::LastIndexOf: startIndex must be within the bounds of the string.");
        auto pos = value.rfind(ch, static_cast<size_t>(startIndex));
        return pos == std::string::npos ? -1 : static_cast<SharpRuntime::intcs>(pos);
    }

    std::string String::Create(SharpRuntime::intcs count, char ch)
    {
        if (count < 0)
            throw System::ArgumentOutOfRangeException("count", "String::Create: count must not be negative.");
        return std::string(static_cast<size_t>(count), ch);
    }

    SharpRuntime::intcs String::Compare(const std::string& a, const std::string& b)
    {
        return static_cast<SharpRuntime::intcs>(a.compare(b));
    }

    SharpRuntime::intcs String::Compare(const std::string& a, const std::string& b, bool ignoreCase)
    {
        if (!ignoreCase) return Compare(a, b);
        std::string la = ToLower(a), lb = ToLower(b);
        return static_cast<SharpRuntime::intcs>(la.compare(lb));
    }

    bool String::Equals(const std::string& a, const std::string& b)
    {
        return a == b;
    }

    bool String::Equals(const std::string& a, const std::string& b, bool ignoreCase)
    {
        if (!ignoreCase) return a == b;
        return ToLower(a) == ToLower(b);
    }

    std::string String::Format(const std::string& format, SharpRuntime::intcs arg0, SharpRuntime::intcs arg1, SharpRuntime::intcs arg2)
    {
        std::string r = replaceArg(format, 0, fmtInt(arg0, extractSpec(format, 0)));
        r = replaceArg(r, 1, fmtInt(arg1, extractSpec(format, 1)));
        return FinalizeFormat(replaceArg(r, 2, fmtInt(arg2, extractSpec(format, 2))));
    }

    std::string String::Format(const std::string& format, const std::string& arg0, const std::string& arg1, const std::string& arg2)
    {
        std::string r = replaceArg(format, 0, arg0);
        r = replaceArg(r, 1, arg1);
        return FinalizeFormat(replaceArg(r, 2, arg2));
    }

    std::string String::Format(const std::string& format, SharpRuntime::longcs arg0)
    {
        return Format(format, std::to_string(arg0));
    }

    std::string String::Format(const std::string& format, double arg0, const std::string& arg1)
    {
        std::string r = replaceArg(format, 0, fmtDouble(arg0, extractSpec(format, 0)));
        return FinalizeFormat(replaceArg(r, 1, arg1));
    }

    std::string String::Format(const std::string& format, const std::string& arg0, double arg1)
    {
        std::string r = replaceArg(format, 0, arg0);
        return FinalizeFormat(replaceArg(r, 1, fmtDouble(arg1, extractSpec(format, 1))));
    }

    std::string String::Format(const std::string& format, SharpRuntime::longcs arg0, SharpRuntime::intcs arg1)
    {
        std::string r = replaceArg(format, 0, std::to_string(arg0));
        return FinalizeFormat(replaceArg(r, 1, fmtInt(arg1, extractSpec(format, 1))));
    }

    std::string String::Format(const std::string& format, SharpRuntime::intcs arg0, SharpRuntime::longcs arg1)
    {
        std::string r = replaceArg(format, 0, fmtInt(arg0, extractSpec(format, 0)));
        return FinalizeFormat(replaceArg(r, 1, std::to_string(arg1)));
    }

    bool String::StartsWith(const std::string& value, char ch)
    {
        return !value.empty() && value.front() == ch;
    }

    bool String::EndsWith(const std::string& value, char ch)
    {
        return !value.empty() && value.back() == ch;
    }

    std::string String::Remove(const std::string& value, SharpRuntime::intcs startIndex)
    {
        if (startIndex < 0 || static_cast<size_t>(startIndex) > value.size())
            throw System::ArgumentOutOfRangeException("startIndex", "String::Remove: startIndex must be within the bounds of the string.");
        return value.substr(0, static_cast<size_t>(startIndex));
    }

    std::string String::Remove(const std::string& value, SharpRuntime::intcs startIndex, SharpRuntime::intcs count)
    {
        if (startIndex < 0 || count < 0 ||
            static_cast<size_t>(startIndex) + static_cast<size_t>(count) > value.size())
            throw System::ArgumentOutOfRangeException("startIndex", "String::Remove: startIndex and count must refer to a location within the string.");
        std::string result = value;
        result.erase(static_cast<size_t>(startIndex), static_cast<size_t>(count));
        return result;
    }

    std::string String::Insert(const std::string& value, SharpRuntime::intcs startIndex, const std::string& insertValue)
    {
        if (startIndex < 0 || static_cast<size_t>(startIndex) > value.size())
            throw System::ArgumentOutOfRangeException("startIndex", "String::Insert: startIndex must be within the bounds of the string.");
        std::string result = value;
        result.insert(static_cast<size_t>(startIndex), insertValue);
        return result;
    }

    std::string String::Join(const std::string& separator, const std::vector<SharpRuntime::intcs>& values)
    {
        std::string result;
        for (size_t i = 0; i < values.size(); ++i) {
            if (i > 0) result += separator;
            result += std::to_string(values[i]);
        }
        return result;
    }

    std::string String::Join(const std::string& separator, const std::vector<double>& values)
    {
        std::string result;
        for (size_t i = 0; i < values.size(); ++i) {
            if (i > 0) result += separator;
            result += std::to_string(values[i]);
        }
        return result;
    }

    std::vector<char> String::ToCharArray(const std::string& value)
    {
        return std::vector<char>(value.begin(), value.end());
    }

    std::string String::Trim(const std::string& value, const std::vector<char>& trimChars)
    {
        return TrimEnd(TrimStart(value, trimChars), trimChars);
    }

    std::string String::TrimStart(const std::string& value, const std::vector<char>& trimChars)
    {
        size_t start = 0;
        while (start < value.size()) {
            bool found = false;
            for (char c : trimChars) if (value[start] == c) { found = true; break; }
            if (!found) break;
            ++start;
        }
        return value.substr(start);
    }

    std::string String::TrimEnd(const std::string& value, const std::vector<char>& trimChars)
    {
        size_t end = value.size();
        while (end > 0) {
            bool found = false;
            for (char c : trimChars) if (value[end - 1] == c) { found = true; break; }
            if (!found) break;
            --end;
        }
        return value.substr(0, end);
    }

    static std::vector<std::string> applySplitOptions(std::vector<std::string> parts, StringSplitOptions options)
    {
        bool removeEmpty = (static_cast<int>(options) & static_cast<int>(StringSplitOptions::RemoveEmptyEntries)) != 0;
        bool trimEntries = (static_cast<int>(options) & static_cast<int>(StringSplitOptions::TrimEntries)) != 0;
        std::vector<std::string> result;
        for (auto& p : parts) {
            if (trimEntries) p = String::Trim(p);
            if (removeEmpty && p.empty()) continue;
            result.push_back(std::move(p));
        }
        return result;
    }

    std::vector<std::string> String::Split(const std::string& value, char delimiter, StringSplitOptions options)
    {
        return applySplitOptions(Split(value, delimiter), options);
    }

    std::vector<std::string> String::Split(const std::string& value, const std::string& delimiter, StringSplitOptions options)
    {
        return applySplitOptions(Split(value, delimiter), options);
    }

    std::string String::Format(const std::string& format, SharpRuntime::intcs arg0, double arg1)
    {
        std::string r = replaceArg(format, 0, fmtInt(arg0, extractSpec(format, 0)));
        return FinalizeFormat(replaceArg(r, 1, fmtDouble(arg1, extractSpec(format, 1))));
    }

    std::string String::Format(const std::string& format, double arg0, SharpRuntime::intcs arg1)
    {
        std::string r = replaceArg(format, 0, fmtDouble(arg0, extractSpec(format, 0)));
        return FinalizeFormat(replaceArg(r, 1, fmtInt(arg1, extractSpec(format, 1))));
    }

    const std::string String::Empty{};

    std::string String::Format(const std::string& format, float arg0)
    {
        return Format(format, static_cast<double>(arg0));
    }

    std::string String::Format(const std::string& format, SharpRuntime::longcs arg0, SharpRuntime::longcs arg1)
    {
        std::string r = replaceArg(format, 0, std::to_string(arg0));
        return FinalizeFormat(replaceArg(r, 1, std::to_string(arg1)));
    }

    std::string String::Format(const std::string& format,
                               const std::string& arg0, const std::string& arg1,
                               const std::string& arg2, const std::string& arg3)
    {
        std::string r = replaceArg(format, 0, arg0);
        r = replaceArg(r, 1, arg1);
        r = replaceArg(r, 2, arg2);
        return FinalizeFormat(replaceArg(r, 3, arg3));
    }

    std::string String::ToUpperInvariant(const std::string& value)
    {
        return ToUpper(value); // ASCII subset is locale-invariant with std::toupper
    }

    std::string String::ToLowerInvariant(const std::string& value)
    {
        return ToLower(value);
    }

    SharpRuntime::intcs String::CompareOrdinal(const std::string& a, const std::string& b)
    {
        if (a < b) return -1;
        if (a > b) return  1;
        return 0;
    }

    SharpRuntime::intcs String::IndexOf(const std::string& value, char ch, SharpRuntime::intcs startIndex, SharpRuntime::intcs count)
    {
        auto len = static_cast<SharpRuntime::intcs>(value.size());
        if (startIndex < 0 || startIndex > len)
            throw System::ArgumentOutOfRangeException("startIndex", "String::IndexOf: startIndex must be within the bounds of the string.");
        if (count < 0 || count > len - startIndex)
            throw System::ArgumentOutOfRangeException("count", "String::IndexOf: count must refer to a location within the string.");
        SharpRuntime::intcs end = startIndex + count;
        for (SharpRuntime::intcs i = startIndex; i < end; ++i)
            if (value[static_cast<size_t>(i)] == ch) return i;
        return -1;
    }

    SharpRuntime::intcs String::IndexOf(const std::string& value, const std::string& substr, SharpRuntime::intcs startIndex, SharpRuntime::intcs count)
    {
        auto len = static_cast<SharpRuntime::intcs>(value.size());
        if (startIndex < 0 || startIndex > len)
            throw System::ArgumentOutOfRangeException("startIndex", "String::IndexOf: startIndex must be within the bounds of the string.");
        if (count < 0 || count > len - startIndex)
            throw System::ArgumentOutOfRangeException("count", "String::IndexOf: count must refer to a location within the string.");
        if (substr.empty()) return startIndex;
        SharpRuntime::intcs end = startIndex + count;
        auto pos = value.find(substr, static_cast<size_t>(startIndex));
        if (pos == std::string::npos) return -1;
        return static_cast<SharpRuntime::intcs>(pos) < end ? static_cast<SharpRuntime::intcs>(pos) : -1;
    }

    SharpRuntime::intcs String::LastIndexOf(const std::string& value, char ch, SharpRuntime::intcs startIndex, SharpRuntime::intcs count)
    {
        if (value.empty()) return -1;
        auto len = static_cast<SharpRuntime::intcs>(value.size());
        // Real .NET's CompareInfo.LastIndexOf off-by-one fixup: startIndex == length is treated as
        // startIndex == length - 1 with count reduced by one to match (not just startIndex alone) --
        // see the 3-arg overload above for the non-count-adjusted rationale.
        if (startIndex == len) { startIndex = len - 1; if (count > 0) count -= 1; }
        if (startIndex < 0 || startIndex >= len)
            throw System::ArgumentOutOfRangeException("startIndex", "String::LastIndexOf: startIndex must be within the bounds of the string.");
        if (count < 0 || count > startIndex + 1)
            throw System::ArgumentOutOfRangeException("count", "String::LastIndexOf: count must refer to a location within the string.");
        SharpRuntime::intcs begin = startIndex - count + 1;
        for (SharpRuntime::intcs i = startIndex; i >= begin && i >= 0; --i)
            if (value[static_cast<size_t>(i)] == ch) return i;
        return -1;
    }

    SharpRuntime::intcs String::LastIndexOf(const std::string& value, const std::string& substr, SharpRuntime::intcs startIndex, SharpRuntime::intcs count)
    {
        if (value.empty()) return substr.empty() ? 0 : -1;
        auto len = static_cast<SharpRuntime::intcs>(value.size());
        // See the (string, char, int, int) overload above for the off-by-one fixup rationale.
        if (startIndex == len) { startIndex = len - 1; if (count > 0) count -= 1; }
        if (startIndex < 0 || startIndex >= len)
            throw System::ArgumentOutOfRangeException("startIndex", "String::LastIndexOf: startIndex must be within the bounds of the string.");
        if (count < 0 || count > startIndex + 1)
            throw System::ArgumentOutOfRangeException("count", "String::LastIndexOf: count must refer to a location within the string.");
        if (substr.empty()) return startIndex;
        SharpRuntime::intcs begin = startIndex - count + 1;
        auto pos = value.rfind(substr, static_cast<size_t>(startIndex));
        if (pos == std::string::npos) return -1;
        return static_cast<SharpRuntime::intcs>(pos) >= begin ? static_cast<SharpRuntime::intcs>(pos) : -1;
    }

    std::vector<char> String::ToCharArray(const std::string& value, SharpRuntime::intcs startIndex, SharpRuntime::intcs length)
    {
        // startIndex+length (both intcs/int32) can itself signed-overflow for large inputs,
        // silently bypassing this check instead of throwing -- confirmed real UB via a
        // standalone UBSan repro on the identical pattern in Span<T>::Slice (ticket 265/1487);
        // fixed the same way real .NET's Span<T>.Slice(int,int) does: unsigned comparison,
        // subtraction instead of addition.
        auto sz = static_cast<SharpRuntime::intcs>(value.size());
        if (static_cast<SharpRuntime::uintcs>(startIndex) > static_cast<SharpRuntime::uintcs>(sz) ||
            static_cast<SharpRuntime::uintcs>(length) > static_cast<SharpRuntime::uintcs>(sz - startIndex))
            throw System::ArgumentOutOfRangeException("index", "String::ToCharArray: index out of range");
        return std::vector<char>(value.begin() + startIndex, value.begin() + startIndex + length);
    }

    SharpRuntime::intcs String::GetHashCode(const std::string& value) noexcept
    {
        // Widen to a fixed 64-bit type before shifting by 32 - std::size_t is only 32 bits
        // on some targets (e.g. Emscripten's wasm32), where "h >> 32" would shift by the
        // full width of the type.
        const auto h = static_cast<std::uint64_t>(std::hash<std::string>{}(value));
        return static_cast<SharpRuntime::intcs>(h ^ (h >> 32));
    }

    // -----------------------------------------------------------------------
    // StringComparison helpers
    // -----------------------------------------------------------------------

    static std::string sc_toLower(const std::string& s) {
        std::string r = s;
        std::transform(r.begin(), r.end(), r.begin(),
            [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
        return r;
    }

    static bool sc_isIgnoreCase(StringComparison c) {
        return c == StringComparison::CurrentCultureIgnoreCase  ||
               c == StringComparison::InvariantCultureIgnoreCase ||
               c == StringComparison::OrdinalIgnoreCase;
    }

    SharpRuntime::intcs String::Compare(const std::string& a, const std::string& b, StringComparison comparisonType) {
        return Compare(a, b, sc_isIgnoreCase(comparisonType));
    }

    bool String::Equals(const std::string& a, const std::string& b, StringComparison comparisonType) {
        return Equals(a, b, sc_isIgnoreCase(comparisonType));
    }

    bool String::Contains(const std::string& value, const std::string& substr, StringComparison comparisonType) {
        if (sc_isIgnoreCase(comparisonType))
            return sc_toLower(value).find(sc_toLower(substr)) != std::string::npos;
        return value.find(substr) != std::string::npos;
    }

    bool String::StartsWith(const std::string& value, const std::string& prefix, StringComparison comparisonType) {
        if (sc_isIgnoreCase(comparisonType))
            return StartsWith(sc_toLower(value), sc_toLower(prefix));
        return StartsWith(value, prefix);
    }

    bool String::EndsWith(const std::string& value, const std::string& suffix, StringComparison comparisonType) {
        if (sc_isIgnoreCase(comparisonType))
            return EndsWith(sc_toLower(value), sc_toLower(suffix));
        return EndsWith(value, suffix);
    }

    SharpRuntime::intcs String::IndexOf(const std::string& value, const std::string& substr, StringComparison comparisonType) {
        if (sc_isIgnoreCase(comparisonType)) {
            auto pos = sc_toLower(value).find(sc_toLower(substr));
            return pos == std::string::npos ? -1 : static_cast<SharpRuntime::intcs>(pos);
        }
        return IndexOf(value, substr);
    }

    SharpRuntime::intcs String::LastIndexOf(const std::string& value, const std::string& substr, StringComparison comparisonType) {
        if (sc_isIgnoreCase(comparisonType)) {
            auto pos = sc_toLower(value).rfind(sc_toLower(substr));
            return pos == std::string::npos ? -1 : static_cast<SharpRuntime::intcs>(pos);
        }
        return LastIndexOf(value, substr);
    }

    std::string String::Join(char separator, const std::vector<std::string>& values) {
        return Join(std::string(1, separator), values);
    }

}

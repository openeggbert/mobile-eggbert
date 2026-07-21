// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/IO/Path.hpp"
#include "System/IO/IOException.hpp"
#include "System/ArgumentException.hpp"

#include <filesystem>
#ifdef _WIN32
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <windows.h>
#  undef GetTempPath      // windows.h macro collides with our method name
#  undef GetTempFileName  // windows.h macro collides with our method name
#else
#  include <unistd.h>
#endif

namespace System::IO {

    namespace {
        inline bool isDirSep(char c) {
            return c == Path::DirectorySeparatorChar || c == Path::AltDirectorySeparatorChar;
        }

        // Length of the root of `path` (1 if rooted, 0 otherwise). Sharp-runtime paths are
        // POSIX-style even on Windows builds — drive letters/UNC prefixes are not modeled.
        std::size_t getRootLength(const std::string& path) {
            return (!path.empty() && isDirSep(path[0])) ? 1 : 0;
        }

        // Port of .NET's PathInternal.RemoveRelativeSegments (PathInternal.cs): collapses
        // "//", "/./" and "/../" segments in-place so GetFullPath's normalization matches
        // .NET's (std::filesystem::absolute() does NOT do this — it only resolves against cwd).
        std::string removeRelativeSegments(const std::string& path, std::size_t rootLength) {
            std::string sb;
            sb.reserve(path.size());

            std::size_t skip = rootLength;
            if (skip > 0 && isDirSep(path[skip - 1]))
                --skip;

            if (skip > 0)
                sb.append(path, 0, skip);

            for (std::size_t i = skip; i < path.size(); ++i) {
                char c = path[i];

                if (isDirSep(c) && i + 1 < path.size()) {
                    if (isDirSep(path[i + 1]))
                        continue;

                    if ((i + 2 == path.size() || isDirSep(path[i + 2])) && path[i + 1] == '.') {
                        ++i;
                        continue;
                    }

                    if (i + 2 < path.size() &&
                        (i + 3 == path.size() || isDirSep(path[i + 3])) &&
                        path[i + 1] == '.' && path[i + 2] == '.') {
                        std::size_t s = sb.size();
                        bool found = false;
                        while (s > skip) {
                            --s;
                            if (isDirSep(sb[s])) {
                                sb.resize((i + 3 >= path.size() && s == skip) ? s + 1 : s);
                                found = true;
                                break;
                            }
                        }
                        if (!found)
                            sb.resize(skip);
                        i += 2;
                        continue;
                    }
                }

                sb.push_back(c);
            }

            if (skip != rootLength && sb.size() < rootLength)
                sb.push_back(path[rootLength - 1]);

            return sb;
        }
    }

    std::string Path::Combine(const std::string& path1, const std::string& path2) {
        if (path1.empty()) return path2;
        if (path2.empty()) return path1;
        if (IsPathRooted(path2)) return path2;
        return (std::filesystem::path(path1) / path2).string();
    }

    std::string Path::Combine(const std::string& path1, const std::string& path2,
                               const std::string& path3) {
        if (path1.empty()) return Combine(path2, path3);
        if (path2.empty()) return Combine(path1, path3);
        if (path3.empty()) return Combine(path1, path2);
        if (IsPathRooted(path3)) return path3;
        if (IsPathRooted(path2)) return Combine(path2, path3);
        return (std::filesystem::path(path1) / path2 / path3).string();
    }

    std::string Path::GetFileName(const std::string& path) {
        return std::filesystem::path(path).filename().string();
    }

    std::string Path::GetFileNameWithoutExtension(const std::string& path) {
        std::string fileName = GetFileName(path);
        auto lastPeriod = fileName.find_last_of('.');
        return (lastPeriod == std::string::npos) ? fileName : fileName.substr(0, lastPeriod);
    }

    std::string Path::GetExtension(const std::string& path) {
        for (std::size_t i = path.size(); i > 0; ) {
            --i;
            char ch = path[i];
            if (ch == '.')
                return (i != path.size() - 1) ? path.substr(i) : std::string();
            if (isDirSep(ch))
                break;
        }
        return std::string();
    }

    std::string Path::GetDirectoryName(const std::string& path) {
        if (path.empty())
            return "";

        std::size_t rootLength = getRootLength(path);
        std::size_t end = path.size();
        if (end <= rootLength)
            return "";

        while (end > rootLength) {
            --end;
            if (isDirSep(path[end]))
                break;
        }
        while (end > rootLength && isDirSep(path[end - 1]))
            --end;

        return path.substr(0, end);
    }

    std::string Path::GetFullPath(const std::string& path) {
        if (path.empty())
            throw System::ArgumentException("The value cannot be an empty string.", "path");
        if (path.find('\0') != std::string::npos)
            throw System::ArgumentException("Null character in path.", "path");

        std::string full = path;
        if (!IsPathRooted(full)) {
            std::error_code ec;
            auto cwd = std::filesystem::current_path(ec);
            if (ec) throw IOException("Failed to get full path: " + ec.message());
            full = Combine(cwd.string(), full);
        }

        std::string collapsed = removeRelativeSegments(full, getRootLength(full));
        return collapsed.empty() ? std::string(1, DirectorySeparatorChar) : collapsed;
    }

    std::string Path::GetTempPath() {
        return std::filesystem::temp_directory_path().string() +
               std::string(1, DirectorySeparatorChar);
    }

    std::string Path::GetTempFileName() {
        auto tmp = std::filesystem::temp_directory_path() / "tmp_XXXXXX";
        std::string tmpl = tmp.string();
#ifdef _WIN32
        char buf[MAX_PATH];
        if (GetTempFileNameA(std::filesystem::temp_directory_path().string().c_str(),
                             "tmp", 0, buf) == 0)
            throw IOException("Failed to create temp file.");
        return std::string(buf);
#else
        std::vector<char> buf(tmpl.begin(), tmpl.end());
        buf.push_back('\0');
        int fd = mkstemp(buf.data());
        if (fd == -1) throw IOException("Failed to create temp file.");
        close(fd);
        return std::string(buf.data());
#endif
    }

    std::string Path::ChangeExtension(const std::string& path, const std::string& extension) {
        if (path.empty())
            return "";

        std::size_t subLength = path.size();
        for (std::size_t i = path.size(); i > 0; ) {
            --i;
            char ch = path[i];
            if (ch == '.') { subLength = i; break; }
            if (isDirSep(ch)) break;
        }

        std::string subpath = path.substr(0, subLength);
        if (extension.empty())
            return subpath;
        return (extension[0] == '.') ? subpath + extension : subpath + "." + extension;
    }

    bool Path::HasExtension(const std::string& path) {
        for (std::size_t i = path.size(); i > 0; ) {
            --i;
            char ch = path[i];
            if (ch == '.')
                return i != path.size() - 1;
            if (isDirSep(ch))
                break;
        }
        return false;
    }

    bool Path::IsPathRooted(const std::string& path) {
        return std::filesystem::path(path).is_absolute();
    }

} // namespace System::IO

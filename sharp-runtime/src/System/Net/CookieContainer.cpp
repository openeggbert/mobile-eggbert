// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Net/CookieContainer.hpp"
#include <algorithm>
#include <cctype>
#include <sstream>

namespace System::Net {

std::string CookieContainer::toLowerAscii(std::string s) {
    for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

// RFC 6265 5.1.3 domain-match: the cookie's domain-value is either identical to the request
// host, or the request host is a subdomain of it (host ends with "." + domain).
bool CookieContainer::domainMatches(const std::string& cookieDomain, const std::string& host) {
    if (cookieDomain.empty()) return true; // host-only cookie stored without an explicit domain
    std::string domain = toLowerAscii(cookieDomain);
    std::string h      = toLowerAscii(host);
    if (!domain.empty() && domain[0] == '.') domain = domain.substr(1);
    if (h == domain) return true;
    if (h.size() > domain.size() &&
        h.compare(h.size() - domain.size(), domain.size(), domain) == 0 &&
        h[h.size() - domain.size() - 1] == '.')
        return true;
    return false;
}

// RFC 6265 5.1.4 path-match: identical, a proper "/"-terminated prefix, or the cookie path is "/".
bool CookieContainer::pathMatches(const std::string& cookiePath, const std::string& requestPath) {
    std::string cp = cookiePath.empty() ? "/" : cookiePath;
    if (requestPath == cp) return true;
    if (requestPath.size() > cp.size() && requestPath.compare(0, cp.size(), cp) == 0) {
        if (cp.back() == '/') return true;
        if (requestPath[cp.size()] == '/') return true;
    }
    return false;
}

void CookieContainer::Add(const System::Uri& uri, const Cookie& cookie) {
    Cookie stored = cookie;
    if (stored.getDomainImplicitProperty()) stored.setDomainProperty(uri.getHostProperty());
    if (stored.getPathImplicitProperty())   stored.setPathProperty(uri.getAbsolutePathProperty());

    // Replace an existing cookie with the same Name/Domain/Path (matches real .NET's
    // "adding a cookie with the same identity overwrites/refreshes it" semantics).
    for (auto& existing : cookies_) {
        if (existing.getNameProperty() == stored.getNameProperty() &&
            existing.getDomainProperty() == stored.getDomainProperty() &&
            existing.getPathProperty() == stored.getPathProperty()) {
            existing = stored;
            return;
        }
    }
    cookies_.push_back(stored);
}

void CookieContainer::Add(const System::Uri& uri, const CookieCollection& cookies) {
    for (const auto& c : cookies) Add(uri, c);
}

namespace {
    std::string trim(const std::string& s) {
        size_t start = s.find_first_not_of(" \t");
        if (start == std::string::npos) return "";
        size_t end = s.find_last_not_of(" \t");
        return s.substr(start, end - start + 1);
    }
}

void CookieContainer::SetCookies(const System::Uri& uri, const std::string& cookieHeader) {
    // Split on ';' into "attribute[=value]" segments. The first segment is always the
    // required Name=Value pair; subsequent segments are optional cookie attributes.
    std::vector<std::string> parts;
    {
        std::stringstream ss(cookieHeader);
        std::string part;
        while (std::getline(ss, part, ';')) parts.push_back(trim(part));
    }
    if (parts.empty() || parts[0].empty())
        throw CookieException("SetCookies: empty Set-Cookie header value.");

    size_t eq = parts[0].find('=');
    if (eq == std::string::npos)
        throw CookieException("SetCookies: missing '=' in cookie Name=Value pair: '" + parts[0] + "'.");

    Cookie cookie(trim(parts[0].substr(0, eq)), trim(parts[0].substr(eq + 1)));

    for (size_t i = 1; i < parts.size(); ++i) {
        const std::string& attr = parts[i];
        if (attr.empty()) continue;
        size_t aeq = attr.find('=');
        std::string attrName  = toLowerAscii(trim(aeq == std::string::npos ? attr : attr.substr(0, aeq)));
        std::string attrValue = aeq == std::string::npos ? "" : trim(attr.substr(aeq + 1));

        if (attrName == "domain") {
            cookie.setDomainProperty(attrValue);
        } else if (attrName == "path") {
            cookie.setPathProperty(attrValue);
        } else if (attrName == "secure") {
            cookie.setSecureProperty(true);
        } else if (attrName == "httponly") {
            cookie.setHttpOnlyProperty(true);
        } else if (attrName == "expires") {
            DateTime parsed;
            if (DateTime::TryParse(attrValue, parsed)) cookie.setExpiresProperty(parsed);
        } else if (attrName == "max-age") {
            try {
                int seconds = std::stoi(attrValue);
                cookie.setExpiresProperty(
                    seconds <= 0 ? DateTime(1601, 1, 1) : DateTime::getNowProperty().AddSeconds(seconds));
            } catch (...) {
                // Malformed Max-Age: ignore the attribute, matching real .NET's lenient parser.
            }
        }
        // Version/Comment/CommentUri/Port/SameSite attributes are accepted but not modeled.
    }

    Add(uri, cookie);
}

std::string CookieContainer::GetCookieHeader(const System::Uri& uri) const {
    CookieCollection matching = GetCookies(uri);
    std::string result;
    for (intcs i = 0; i < matching.getCountProperty(); ++i) {
        if (!result.empty()) result += "; ";
        result += matching[i].ToString();
    }
    return result;
}

CookieCollection CookieContainer::GetCookies(const System::Uri& uri) const {
    CookieCollection result;
    bool isSecureRequest = toLowerAscii(uri.getSchemeProperty()) == "https";
    std::string path = uri.getAbsolutePathProperty();
    if (path.empty()) path = "/";

    for (const auto& c : cookies_) {
        if (c.getExpiredProperty()) continue;
        if (c.getSecureProperty() && !isSecureRequest) continue;
        if (!domainMatches(c.getDomainProperty(), uri.getHostProperty())) continue;
        if (!pathMatches(c.getPathProperty(), path)) continue;
        result.Add(c);
    }
    return result;
}

} // namespace System::Net

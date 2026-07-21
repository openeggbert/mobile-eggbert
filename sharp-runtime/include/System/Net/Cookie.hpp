// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <algorithm>
#include <cctype>
#include <string>
#include "System/DateTime.hpp"
#include "System/Net/CookieException.hpp"

namespace System::Net {

    /**
     * @brief Provides a set of properties and methods used to manage cookies.
     *
     * C++ counterpart of .NET System.Net.Cookie. This is a pragmatic subset covering the
     * fields games commonly need for session-based web APIs (leaderboards, auth, analytics):
     * Name/Value/Domain/Path/Expires/Secure/HttpOnly. Version-2 cookie attributes (Port,
     * Comment, CommentUri, Discard) and RFC 2965 quoting rules are intentionally not
     * implemented -- real-world cookies overwhelmingly use the simple Version-0 "Netscape"
     * shape this class targets.
     */
    class Cookie {
    public:
        Cookie() = default;

        /** @brief Initializes a new instance with the given name and value. */
        Cookie(const std::string& name, const std::string& value) {
            setNameProperty(name);
            value_ = value;
        }

        /** @brief Initializes a new instance with the given name, value, and path. */
        Cookie(const std::string& name, const std::string& value, const std::string& path)
            : Cookie(name, value) {
            path_ = path;
        }

        /** @brief Initializes a new instance with the given name, value, path, and domain. */
        Cookie(const std::string& name, const std::string& value, const std::string& path,
               const std::string& domain)
            : Cookie(name, value, path) {
            domain_ = domain;
        }

        /**
         * @brief Gets or sets the name of the cookie.
         * @throws CookieException if the name is empty, starts with '$', or contains a
         *         character invalid in a cookie name (whitespace, control chars, '=' ';' ',').
         */
        [[nodiscard]] const std::string& getNameProperty() const { return name_; }
        void setNameProperty(const std::string& v) {
            if (v.empty() || v[0] == '$')
                throw CookieException("Cookie name must not be empty or start with '$'.");
            for (unsigned char c : v) {
                if (c <= 0x20 || c == 0x7F || c == '=' || c == ';' || c == ',')
                    throw CookieException("Cookie name contains invalid characters: '" + v + "'.");
            }
            name_ = v;
        }

        /** @brief Gets or sets the value of the cookie. */
        [[nodiscard]] const std::string& getValueProperty() const { return value_; }
        void setValueProperty(const std::string& v) { value_ = v; }

        /**
         * @brief Gets or sets the URI(s) for which the cookie is valid, restricted to a
         * particular domain. A leading '.' matches the domain and all its subdomains.
         */
        [[nodiscard]] const std::string& getDomainProperty() const { return domain_; }
        void setDomainProperty(const std::string& v) { domain_ = v; domainImplicit_ = false; }

        /** @brief Gets or sets the URI(s) that access the cookie is restricted to. */
        [[nodiscard]] std::string getPathProperty() const { return path_.empty() ? kRootPath : path_; }
        void setPathProperty(const std::string& v) { path_ = v; pathImplicit_ = false; }

        /**
         * @brief Gets or sets the expiration date and time for the cookie.
         * DateTime::MinValue (the default) means the cookie is a session cookie -- it is
         * discarded when the session ends and never reported as expired.
         */
        [[nodiscard]] DateTime getExpiresProperty() const { return expires_; }
        void setExpiresProperty(DateTime v) { expires_ = v; }

        /** @brief Gets or sets whether the cookie should only be transmitted over HTTPS. */
        [[nodiscard]] bool getSecureProperty() const { return secure_; }
        void setSecureProperty(bool v) { secure_ = v; }

        /**
         * @brief Gets or sets whether a page script or other active content can access this
         * cookie (mirrors the Set-Cookie HttpOnly attribute; this class does not enforce it --
         * that is a browser/script-engine concern this runtime does not have).
         */
        [[nodiscard]] bool getHttpOnlyProperty() const { return httpOnly_; }
        void setHttpOnlyProperty(bool v) { httpOnly_ = v; }

        /** @brief Returns true if the cookie's Expires time has passed (session cookies never expire here). */
        [[nodiscard]] bool getExpiredProperty() const {
            return expires_ != DateTime::MinValue && expires_ <= DateTime::getNowProperty();
        }

        /** @brief Marks the cookie for discard by the client (used by SetCookies to expire a cookie immediately). */
        void setExpiredProperty(bool v) {
            if (v) expires_ = DateTime(1601, 1, 1);
        }

        [[nodiscard]] bool getDomainImplicitProperty() const { return domainImplicit_; }
        [[nodiscard]] bool getPathImplicitProperty() const { return pathImplicit_; }

        /**
         * @brief Returns the string representation of this cookie as sent in a Cookie request
         * header: "Name=Value". C++ counterpart of .NET Cookie.ToString() for a plain
         * (Version 0) cookie -- the common case this class targets.
         */
        [[nodiscard]] std::string ToString() const {
            if (name_.empty()) return "";
            return name_ + "=" + value_;
        }

        [[nodiscard]] bool Equals(const Cookie& other) const {
            return strEqualsIgnoreCase(name_, other.name_) &&
                   value_ == other.value_ &&
                   strEqualsIgnoreCase(getPathProperty(), other.getPathProperty()) &&
                   strEqualsIgnoreCase(domain_, other.domain_);
        }
        bool operator==(const Cookie& other) const { return Equals(other); }
        bool operator!=(const Cookie& other) const { return !Equals(other); }

    private:
        static constexpr const char* kRootPath = "/";

        static bool strEqualsIgnoreCase(const std::string& a, const std::string& b) {
            if (a.size() != b.size()) return false;
            for (size_t i = 0; i < a.size(); ++i)
                if (std::tolower(static_cast<unsigned char>(a[i])) != std::tolower(static_cast<unsigned char>(b[i])))
                    return false;
            return true;
        }

        std::string name_;
        std::string value_;
        std::string domain_;
        std::string path_;
        DateTime    expires_ = DateTime::MinValue;
        bool        secure_ = false;
        bool        httpOnly_ = false;
        bool        domainImplicit_ = true;
        bool        pathImplicit_ = true;
    };

} // namespace System::Net

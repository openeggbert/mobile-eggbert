// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <optional>
#include <string>
#include <vector>
#include "System/Net/Http/Headers/NameValueHeaderValue.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Net::Http::Headers {

    /**
     * @brief Represents the value of the Cache-Control header.
     *
     * C++ counterpart of .NET System.Net.Http.Headers.CacheControlHeaderValue.
     *
     * @note MaxAge/SharedMaxAge/MaxStaleLimit/MinFresh are seconds as `std::optional<intcs>`
     * rather than .NET's `TimeSpan?` — this runtime has no `TimeSpan`, and the header wire format
     * only ever carries whole seconds anyway (.NET itself converts via `TimeSpan.TotalSeconds`
     * truncated to `int` when formatting).
     * @note .NET's internal GetCacheControlLength parsing helper is not ported; Parse/TryParse
     * here implement the same practical algorithm directly (split on top-level ',' respecting
     * quoted-string values, dispatch each "name[=value]" pair by known directive name, unknown
     * names become Extensions). ICloneable is not ported (no reflection-based virtual-clone story,
     * per NEXT.md).
     */
    class CacheControlHeaderValue {
        bool noCache_ = false;
        bool noStore_ = false;
        bool maxStale_ = false;
        bool noTransform_ = false;
        bool onlyIfCached_ = false;
        bool public_ = false;
        bool private_ = false;
        bool mustRevalidate_ = false;
        bool proxyRevalidate_ = false;

        std::optional<SharpRuntime::intcs> maxAge_;
        std::optional<SharpRuntime::intcs> sharedMaxAge_;
        std::optional<SharpRuntime::intcs> maxStaleLimit_;
        std::optional<SharpRuntime::intcs> minFresh_;

        std::vector<std::string> noCacheHeaders_;
        std::vector<std::string> privateHeaders_;
        std::vector<NameValueHeaderValue> extensions_;

    public:
        CacheControlHeaderValue() = default;

        /** @return true if the "no-cache" directive is present. */
        [[nodiscard]] bool getNoCacheProperty() const { return noCache_; }
        void setNoCacheProperty(bool value) { noCache_ = value; }
        /** @return The list of header names restricted by "no-cache" (e.g. `no-cache="Set-Cookie"`), mutable. */
        [[nodiscard]] std::vector<std::string>& getNoCacheHeadersProperty() { return noCacheHeaders_; }
        [[nodiscard]] const std::vector<std::string>& getNoCacheHeadersProperty() const { return noCacheHeaders_; }

        [[nodiscard]] bool getNoStoreProperty() const { return noStore_; }
        void setNoStoreProperty(bool value) { noStore_ = value; }

        /** @return max-age in seconds, or empty if not set. */
        [[nodiscard]] std::optional<SharpRuntime::intcs> getMaxAgeProperty() const { return maxAge_; }
        void setMaxAgeProperty(std::optional<SharpRuntime::intcs> value) { maxAge_ = value; }

        /** @return s-maxage in seconds, or empty if not set. */
        [[nodiscard]] std::optional<SharpRuntime::intcs> getSharedMaxAgeProperty() const { return sharedMaxAge_; }
        void setSharedMaxAgeProperty(std::optional<SharpRuntime::intcs> value) { sharedMaxAge_ = value; }

        [[nodiscard]] bool getMaxStaleProperty() const { return maxStale_; }
        void setMaxStaleProperty(bool value) { maxStale_ = value; }
        /** @return max-stale's optional seconds limit. */
        [[nodiscard]] std::optional<SharpRuntime::intcs> getMaxStaleLimitProperty() const { return maxStaleLimit_; }
        void setMaxStaleLimitProperty(std::optional<SharpRuntime::intcs> value) { maxStaleLimit_ = value; }

        /** @return min-fresh in seconds, or empty if not set. */
        [[nodiscard]] std::optional<SharpRuntime::intcs> getMinFreshProperty() const { return minFresh_; }
        void setMinFreshProperty(std::optional<SharpRuntime::intcs> value) { minFresh_ = value; }

        [[nodiscard]] bool getNoTransformProperty() const { return noTransform_; }
        void setNoTransformProperty(bool value) { noTransform_ = value; }

        [[nodiscard]] bool getOnlyIfCachedProperty() const { return onlyIfCached_; }
        void setOnlyIfCachedProperty(bool value) { onlyIfCached_ = value; }

        [[nodiscard]] bool getPublicProperty() const { return public_; }
        void setPublicProperty(bool value) { public_ = value; }

        [[nodiscard]] bool getPrivateProperty() const { return private_; }
        void setPrivateProperty(bool value) { private_ = value; }
        /** @return The list of header names restricted by "private", mutable. */
        [[nodiscard]] std::vector<std::string>& getPrivateHeadersProperty() { return privateHeaders_; }
        [[nodiscard]] const std::vector<std::string>& getPrivateHeadersProperty() const { return privateHeaders_; }

        [[nodiscard]] bool getMustRevalidateProperty() const { return mustRevalidate_; }
        void setMustRevalidateProperty(bool value) { mustRevalidate_ = value; }

        [[nodiscard]] bool getProxyRevalidateProperty() const { return proxyRevalidate_; }
        void setProxyRevalidateProperty(bool value) { proxyRevalidate_ = value; }

        /** @return Any unrecognized "name[=value]" directives, mutable. */
        [[nodiscard]] std::vector<NameValueHeaderValue>& getExtensionsProperty() { return extensions_; }
        [[nodiscard]] const std::vector<NameValueHeaderValue>& getExtensionsProperty() const { return extensions_; }

        /** @return The comma-separated Cache-Control directive list. */
        [[nodiscard]] std::string ToString() const;

        /** Equality: every field and collection must match (token lists compared case-insensitively, order-independent). */
        [[nodiscard]] bool Equals(const CacheControlHeaderValue& other) const;
        bool operator==(const CacheControlHeaderValue& o) const { return Equals(o); }
        bool operator!=(const CacheControlHeaderValue& o) const { return !Equals(o); }

        /** @return A hash code consistent with Equals(). */
        [[nodiscard]] SharpRuntime::intcs GetHashCode() const;

        /**
         * @brief Parses a Cache-Control header value.
         * @throws System::FormatException if @p input is not valid.
         */
        [[nodiscard]] static CacheControlHeaderValue Parse(const std::string& input);

        /** Attempts to parse @p input; returns false without throwing on failure. */
        [[nodiscard]] static bool TryParse(const std::string& input, CacheControlHeaderValue& parsedValue);
    };

} // namespace System::Net::Http::Headers

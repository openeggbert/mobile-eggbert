// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <cstdint>
#include "System/Attribute.hpp"

namespace System::Security {

    /** Identifies the set of security transparency rules applied to an assembly. */
    enum class SecurityRuleSet : uint8_t {
        None    = 0, ///< No transparency rules.
        Level1  = 1, ///< .NET Framework 2.0 transparency rules.
        Level2  = 2, ///< .NET Framework 4.0 transparency rules.
    };

    /** Controls how a partially trusted assembly is made visible to other assemblies. */
    enum class PartialTrustVisibilityLevel {
        VisibleToAllHosts   = 0, ///< Visible to all hosts (default).
        NotVisibleByDefault = 1, ///< Not visible to partial-trust hosts by default.
    };

    /**
     * Allows partially trusted callers to call an assembly that would otherwise
     * require full trust.
     */
    class AllowPartiallyTrustedCallersAttribute : public System::Attribute {
        PartialTrustVisibilityLevel visibilityLevel_ = PartialTrustVisibilityLevel::VisibleToAllHosts;
    public:
        /** Default constructor — visibility is VisibleToAllHosts. */
        AllowPartiallyTrustedCallersAttribute() = default;

        /** @return The partial-trust visibility level. */
        [[nodiscard]] PartialTrustVisibilityLevel getPartialTrustVisibilityLevelProperty() const { return visibilityLevel_; }

        /** Sets the partial-trust visibility level. */
        void setPartialTrustVisibilityLevelProperty(PartialTrustVisibilityLevel v) { visibilityLevel_ = v; }
    };

    /** Marks a type or member as security-critical (cannot be accessed from transparent code). */
    class SecurityCriticalAttribute : public System::Attribute {};

    /** Marks a type or member as security-safe-critical (accessible from transparent code). */
    class SecuritySafeCriticalAttribute : public System::Attribute {};

    /** Marks an assembly as fully security-transparent (no critical code). */
    class SecurityTransparentAttribute : public System::Attribute {};

    /** Marks a method as a dynamic security method (never inlined by the JIT). */
    class DynamicSecurityMethodAttribute : public System::Attribute {};

    /** Suppresses the unmanaged-code security check for the decorated P/Invoke method. */
    class SuppressUnmanagedCodeSecurityAttribute : public System::Attribute {};

    /** Marks an assembly as containing unverifiable code. */
    class UnverifiableCodeAttribute : public System::Attribute {};

    /** Specifies the security transparency rule set applied to an assembly. */
    class SecurityRulesAttribute : public System::Attribute {
        SecurityRuleSet ruleSet_;
    public:
        /** @param ruleSet The transparency rule set to apply. */
        explicit SecurityRulesAttribute(SecurityRuleSet ruleSet) : ruleSet_(ruleSet) {}

        /** @return The transparency rule set. */
        [[nodiscard]] SecurityRuleSet getRuleSetProperty() const { return ruleSet_; }

        bool skipVerificationInFullTrust_ = false; ///< Skip IL verification in full-trust scenarios.
    };

} // namespace System::Security

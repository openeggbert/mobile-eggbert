// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/Attribute.hpp"

namespace System::Diagnostics {

    /**
     * @brief Specifies the visualizer for a type shown in the debugger.
     * 
     * C++ counterpart of .NET System.Diagnostics.DebuggerVisualizerAttribute.
     */
    class DebuggerVisualizerAttribute : public System::Attribute {
        std::string visualizerTypeName_;
        std::string visualizerObjectSourceTypeName_;
        std::string description_;
        std::string target_;

    public:
        /**
         * @brief Constructs the attribute with a visualizer type name.
         * @param visualizerTypeName Fully-qualified name of the visualizer type.
         */
        explicit DebuggerVisualizerAttribute(const std::string& visualizerTypeName)
            : visualizerTypeName_(visualizerTypeName) {}

        /**
         * @brief Constructs the attribute with a visualizer and its object-source type.
         * @param visualizerTypeName             Fully-qualified name of the visualizer type.
         * @param visualizerObjectSourceTypeName Fully-qualified name of the object-source type.
         */
        DebuggerVisualizerAttribute(const std::string& visualizerTypeName,
                                    const std::string& visualizerObjectSourceTypeName)
            : visualizerTypeName_(visualizerTypeName),
              visualizerObjectSourceTypeName_(visualizerObjectSourceTypeName) {}

        /** @return Fully-qualified name of the visualizer type. */
        [[nodiscard]] const std::string& getVisualizerTypeNameProperty()             const { return visualizerTypeName_; }
        /** @return Fully-qualified name of the visualizer object-source type; empty if not set. */
        [[nodiscard]] const std::string& getVisualizerObjectSourceTypeNameProperty() const { return visualizerObjectSourceTypeName_; }
        /** @return Human-readable description of the visualizer; empty if not set. */
        [[nodiscard]] const std::string& getDescriptionProperty()                    const { return description_; }
        /** @return Target type name this visualizer applies to; empty if not set. */
        [[nodiscard]] const std::string& getTargetProperty()                         const { return target_; }

        /** @brief Sets the human-readable description of the visualizer. */
        void setDescriptionProperty(const std::string& v) { description_ = v; }
        /** @brief Sets the target type name this visualizer applies to. */
        void setTargetProperty(const std::string& v)      { target_ = v; }
    };

} // namespace System::Diagnostics

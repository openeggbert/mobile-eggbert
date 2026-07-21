// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Diagnostics {

    using SharpRuntime::intcs;

    /**
     * @brief Provides information about a single frame in a call stack.
     *
     * Partial C++ counterpart of .NET System.Diagnostics.StackFrame.
     */
    class StackFrame {
        std::string fileName_;
        intcs fileLineNumber_ = 0;
        intcs fileColumnNumber_ = 0;
        std::string methodName_;
        intcs nativeOffset_ = -1; // OFFSET_UNKNOWN (verified against StackFrame.cs's InitMembers: both offsets default to OFFSET_UNKNOWN, not 0)
        intcs ilOffset_ = -1;

    public:
        static constexpr intcs OFFSET_UNKNOWN = -1; ///< Sentinel for an unknown offset.

        /** @brief Constructs an empty StackFrame with no location information. */
        StackFrame() = default;

        /**
         * @brief Constructs a StackFrame with source location information.
         * @param fileName   Source file name.
         * @param lineNumber Line number within the source file.
         * @param columnNumber Column number within the line (0 if unknown).
         */
        explicit StackFrame(const std::string& fileName, intcs lineNumber, intcs columnNumber = 0)
            : fileName_(fileName), fileLineNumber_(lineNumber), fileColumnNumber_(columnNumber) {}

        /** @return The source file name for this frame, or empty if unknown. */
        [[nodiscard]] const std::string& getFileNameProperty()        const { return fileName_; }
        /** @return The line number in the source file, or 0 if unknown. */
        [[nodiscard]] intcs              getFileLineNumberProperty()  const { return fileLineNumber_; }
        /** @return The column number in the source file, or 0 if unknown. */
        [[nodiscard]] intcs              getFileColumnNumberProperty()const { return fileColumnNumber_; }
        /** @return The name of the method for this frame, or empty if unknown. */
        [[nodiscard]] const std::string& getMethodNameProperty()      const { return methodName_; }
        /** @return The native code offset within the method, or OFFSET_UNKNOWN. */
        [[nodiscard]] intcs              getNativeOffsetProperty()    const { return nativeOffset_; }
        /** @return The IL offset within the method, or OFFSET_UNKNOWN. */
        [[nodiscard]] intcs              getILOffsetProperty()        const { return ilOffset_; }

        /** @return Human-readable representation of this frame. */
        [[nodiscard]] std::string ToString() const {
            if (fileName_.empty()) return "<unknown>";
            return fileName_ + ":line " + std::to_string(fileLineNumber_);
        }
    };

} // namespace System::Diagnostics

// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/ArgumentException.hpp"
#include "System/DBNull.hpp"
#include "System/NotSupportedException.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System {

    using SharpRuntime::intcs;

    /**
     * @brief Holds the Null singleton, guaranteeing only one instance exists.
     *
     * C++ counterpart of .NET System.UnitySerializationHolder (sealed).
     *
     * In .NET, UnitySerializationHolder exists solely for backward compatibility
     * with .NET Framework's BinaryFormatter.  Its only meaningful behaviour is to
     * produce DBNull::Value when the stored unity type is NullUnity (0x0002).
     *
     * The .NET type implements ISerializable and IObjectReference (both tied to
     * BinaryFormatter infrastructure); this port omits those interfaces because
     * BinaryFormatter is not implemented in sharp-runtime.  The public API surface
     * — the NullUnity constant, GetRealObject(), and GetObjectData() — is
     * preserved for source-compatibility.
     *
     * @note This class is @b obsolete even in .NET 6+ and exists only for
     *       legacy deserialization compatibility.  New code should not use it.
     */
    class UnitySerializationHolder final {
    public:
        /**
         * @brief The unity-type code that identifies DBNull.
         *
         * C++ counterpart of .NET UnitySerializationHolder.NullUnity.
         */
        static constexpr intcs NullUnity = 0x0002;

        /**
         * @brief Constructs a holder with the given unity type and optional data string.
         *
         * C++ counterpart of the .NET constructor that reads the unity type and
         * optional data string from a SerializationInfo.
         * @param unityType The unity type code (use NullUnity for DBNull).
         * @param data      Optional diagnostic string (used in error messages).
         */
        explicit UnitySerializationHolder(intcs unityType, const std::string& data = {})
            : unityType_(unityType), data_(data) {}

        /**
         * @brief Returns the real object that this holder represents.
         *
         * C++ counterpart of .NET IObjectReference.GetRealObject().
         * When the stored unity type is NullUnity, returns DBNull::Value.
         * For any other unity type, throws ArgumentException.
         * @throws ArgumentException if the unity type is not NullUnity.
         * @return DBNull::Value — the sole instance of System::DBNull.
         */
        [[nodiscard]] DBNull& GetRealObject() const {
            if (unityType_ != NullUnity) {
                throw ArgumentException(
                    "Invalid unity type: " + (data_.empty() ? std::to_string(unityType_) : data_));
            }
            return DBNull::Value();
        }

        /**
         * @brief Not supported — always throws NotSupportedException.
         *
         * C++ counterpart of .NET ISerializable.GetObjectData(), which the .NET
         * implementation also throws for this type.
         * @throws NotSupportedException always.
         */
        void GetObjectData() const {
            throw NotSupportedException(
                "UnitySerializationHolder does not support GetObjectData.");
        }

        /**
         * @brief Gets the unity type code stored in this holder.
         *
         * C++ counterpart of the private _unityType field exposed for testing.
         */
        [[nodiscard]] intcs getUnityTypeProperty() const noexcept { return unityType_; }

        /**
         * @brief Gets the optional diagnostic data string stored in this holder.
         *
         * C++ counterpart of the private _data field exposed for testing.
         */
        [[nodiscard]] const std::string& getDataProperty() const noexcept { return data_; }

    private:
        intcs       unityType_;
        std::string data_;
    };

} // namespace System

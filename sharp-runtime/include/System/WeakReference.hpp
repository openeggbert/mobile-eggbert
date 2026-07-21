// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>

namespace System {

    /**
     * @brief Represents a weak reference, which references an object while still allowing
     * that object to be reclaimed by garbage collection (shared_ptr reference counting in C++).
     *
     * C++ counterpart of .NET System.WeakReference.
     * Backed by std::weak_ptr<void>. The trackResurrection flag is accepted for
     * API compatibility but has no effect because C++ has no finalizer resurrection.
     */
    class WeakReference {
        std::weak_ptr<void> weakPtr_;
        bool trackResurrection_ = false;

    public:
        /** @brief Constructs an empty WeakReference (target is nullptr). */
        WeakReference() = default;

        /**
         * @brief Initializes a new instance of the WeakReference class, referencing
         * the specified object.
         *
         * C++ counterpart of .NET WeakReference(object).
         * @param target          Shared pointer to track.
         * @param trackResurrection Stored and returned via getTrackResurrectionProperty();
         *                          has no functional effect in C++.
         */
        explicit WeakReference(std::shared_ptr<void> target, bool trackResurrection = false)
            : weakPtr_(target), trackResurrection_(trackResurrection) {}

        // -----------------------------------------------------------------------
        // Properties
        // -----------------------------------------------------------------------

        /**
         * @brief Gets an indication whether the object referenced by the current
         * WeakReference object has been garbage collected.
         *
         * C++ counterpart of .NET WeakReference.IsAlive.
         * @return true if the referenced object has not expired; false otherwise.
         */
        [[nodiscard]] bool getIsAliveProperty() const noexcept {
            return !weakPtr_.expired();
        }

        /**
         * @brief Gets the object (as shared_ptr<void>) this reference refers to,
         * or nullptr if the object has been collected.
         *
         * C++ counterpart of .NET WeakReference.Target getter.
         */
        [[nodiscard]] std::shared_ptr<void> getTargetProperty() const noexcept {
            return weakPtr_.lock();
        }

        /**
         * @brief Sets the object this reference refers to.
         *
         * C++ counterpart of .NET WeakReference.Target setter.
         * @param value New target (may be nullptr to clear the reference).
         */
        void setTargetProperty(std::shared_ptr<void> value) noexcept {
            weakPtr_ = value;
        }

        /**
         * @brief Gets a value indicating whether the object tracked by the WeakReference
         * should be tracked through finalization.
         *
         * C++ counterpart of .NET WeakReference.TrackResurrection.
         * Always returns the value passed to the constructor; has no functional effect
         * in C++ because there is no finalizer resurrection mechanism.
         */
        [[nodiscard]] bool getTrackResurrectionProperty() const noexcept {
            return trackResurrection_;
        }

        // -----------------------------------------------------------------------
        // Methods
        // -----------------------------------------------------------------------

        /**
         * @brief Tries to retrieve the object referenced by this WeakReference.
         *
         * C++ counterpart of .NET WeakReference.TryGetTarget(out T target)
         * (non-generic version using void*).
         * @param target Set to the live object on success, or nullptr on failure.
         * @return true if the target is still alive; false if it has been collected.
         */
        [[nodiscard]] bool TryGetTarget(std::shared_ptr<void>& target) const noexcept {
            target = weakPtr_.lock();
            return target != nullptr;
        }
    };

    // ---------------------------------------------------------------------------

    /**
     * @brief Represents a typed weak reference backed by std::weak_ptr<T>.
     *
     * C++ counterpart of .NET System.WeakReference<T> (generic, sealed).
     * Named WeakReferenceT to avoid a name conflict with the non-generic WeakReference
     * class (C++ cannot have a non-template and a class template with the same name).
     *
     * The trackResurrection flag is accepted for API compatibility but has no
     * functional effect in C++.
     */
    template<typename T>
    class WeakReferenceT {
        std::weak_ptr<T> weakPtr_;
        bool trackResurrection_ = false;

    public:
        /** @brief Constructs an empty typed WeakReference with no target. */
        WeakReferenceT() = default;

        /**
         * @brief Initializes a new instance that references the specified object.
         *
         * C++ counterpart of .NET WeakReference<T>(T target).
         * @param target Shared pointer to track.
         */
        explicit WeakReferenceT(std::shared_ptr<T> target, bool trackResurrection = false)
            : weakPtr_(target), trackResurrection_(trackResurrection) {}

        // -----------------------------------------------------------------------
        // Properties
        // -----------------------------------------------------------------------

        /**
         * @brief Gets a value indicating whether the referenced object is still alive.
         *
         * C++ counterpart of .NET WeakReference.IsAlive (non-API on WeakReference<T>,
         * but commonly needed).
         */
        [[nodiscard]] bool getIsAliveProperty() const noexcept { return !weakPtr_.expired(); }

        /**
         * @brief Gets a value indicating whether the object tracked by this reference
         * should be tracked through finalization.
         *
         * C++ counterpart of .NET WeakReference.TrackResurrection.
         */
        [[nodiscard]] bool getTrackResurrectionProperty() const noexcept {
            return trackResurrection_;
        }

        // -----------------------------------------------------------------------
        // Methods
        // -----------------------------------------------------------------------

        /**
         * @brief Tries to retrieve the object referenced by this WeakReference<T>.
         *
         * C++ counterpart of .NET WeakReference<T>.TryGetTarget(out T target).
         * @param target Set to the live object on success, or nullptr on failure.
         * @return true if the target is still alive; false if it has been collected.
         */
        [[nodiscard]] bool TryGetTarget(std::shared_ptr<T>& target) const noexcept {
            target = weakPtr_.lock();
            return target != nullptr;
        }

        /**
         * @brief Sets the object this reference refers to.
         *
         * C++ counterpart of .NET WeakReference<T>.SetTarget(T target).
         * @param target New target (may be nullptr to clear the reference).
         */
        void SetTarget(std::shared_ptr<T> target) noexcept { weakPtr_ = target; }
    };

} // namespace System

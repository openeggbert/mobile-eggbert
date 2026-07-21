// SPDX-License-Identifier: MS-PL
#pragma once

#include "CNA/CNAHelper.hpp"
#include "System/IDisposable.hpp"

// Opaque forward declaration of SDL's cursor handle. This public header wraps an SDL cursor but must
// not pull <SDL3/SDL.h> into consumers: a strict-XNA header (Mouse.hpp) includes this one, so doing so
// would drag all of SDL into the public XNA include tree. A pointer to the incomplete type is all the
// public API needs; MouseCursor.cpp includes the real SDL header.
struct SDL_Cursor;

namespace Microsoft::Xna::Framework::Input
{
    /**
     * @brief Represents a mouse cursor image.
     *
     * Wraps an SDL_Cursor* and provides the standard stock system cursors as lazily-created,
     * process-lifetime singletons (each getXProperty() below constructs its SDL cursor on first
     * access, matching MonoGame's static-constructor-triggered lazy initialization).
     *
     * @note NOXNA — this is a MonoGame-derived CNA extension. No MouseCursor type exists
     * in XNA 4.0 or FNA.
     */
    NOXNA class MouseCursor : public System::IDisposable
    {
    public:
        /** @brief Creates a default Arrow cursor. */
        NOXNA MouseCursor();

        /**
         * @brief Creates a cursor wrapping the given SDL cursor.
         * @param sdlCursor The SDL cursor to wrap.
         * @param owning If true, this object takes ownership of the SDL cursor.
         */
        NOXNA explicit MouseCursor(SDL_Cursor* sdlCursor, bool owning = false);

        MouseCursor(const MouseCursor&)            = delete;
        MouseCursor& operator=(const MouseCursor&) = delete;
        /** @brief Move-constructs a MouseCursor, transferring SDL cursor ownership. */
        MouseCursor(MouseCursor&& other) noexcept;
        /** @brief Move-assigns a MouseCursor, transferring SDL cursor ownership. */
        MouseCursor& operator=(MouseCursor&& other) noexcept;

        /** @brief Destructor; disposes the cursor if owned. */
        ~MouseCursor() override;

        /**
         * @brief Releases the SDL cursor if owned. Safe to call more than once.
         *
         * @note For the stock system-cursor singletons (getArrowProperty() etc.) this is a
         *       deliberate no-op — they are process-lifetime shared instances, so disposing one
         *       must not free the SDL cursor out from under every other user. Do not `std::move`
         *       a stock-cursor reference either; obtain and use it in place.
         */
        NOXNA void Dispose() override;

        /**
         * @brief Returns the underlying SDL_Cursor pointer (not owned by the caller).
         * @return The SDL_Cursor pointer.
         */
        NOXNA [[nodiscard]] SDL_Cursor* GetSDLCursor() const { return sdlCursor_; }

        /**
         * @brief Gets the default arrow cursor.
         * @return Reference to the shared stock cursor instance.
         */
        NOXNA [[nodiscard]] static MouseCursor& getArrowProperty();

    private:
        SDL_Cursor* sdlCursor_        = nullptr;
        bool        owning_           = false;
        bool        isDisposed_       = false;
        // True for the stock system-cursor singletons: Dispose()/destructor no-op so the shared
        // process-lifetime SDL cursor is never freed (which would corrupt it for every other user
        // and risks a free-after-SDL_Quit at static teardown).
        bool        isSystemSingleton_ = false;

        // Takes the SDL_SystemCursor value as a plain int so the enum stays out of this public header
        // (the .cpp casts it back). id values come from SDL_SYSTEM_CURSOR_* in MouseCursor.cpp.
        NOXNA static MouseCursor MakeSystem(int systemCursorId);
    };
}

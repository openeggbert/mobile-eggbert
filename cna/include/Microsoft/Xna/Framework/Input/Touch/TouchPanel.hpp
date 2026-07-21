// SPDX-License-Identifier: MS-PL
#pragma once

#include <array>
#include <cstdint>
#include <queue>
#include <vector>

#include "CNA/CNAHelper.hpp"
#include "Microsoft/Xna/Framework/Input/Touch/GestureSample.hpp"
#include "Microsoft/Xna/Framework/Input/Touch/GestureType.hpp"
#include "Microsoft/Xna/Framework/Input/Touch/TouchCollection.hpp"
#include "Microsoft/Xna/Framework/Input/Touch/TouchLocation.hpp"
#include "Microsoft/Xna/Framework/Input/Touch/TouchLocationState.hpp"
#include "Microsoft/Xna/Framework/Input/Touch/TouchPanelCapabilities.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace Microsoft::Xna::Framework::Input::Touch
{
    /**
     * @brief Provides access to touch input state and queued gesture samples.
     */
    class TouchPanel final
    {
    public:
        using intcs = SharpRuntime::intcs;

        /** @brief TouchPanel is a static class (XNA `public static class TouchPanel`) and cannot be instantiated. */
        TouchPanel() = delete;

        /**
         * @brief Maximum number of simultaneous touches accepted by the XNA touch panel API.
         * @note NOXNA — FNA declares this `internal const int MAX_TOUCHES` (TouchPanel.cs:23), not
         *       part of the public XNA `TouchPanel` API. Exposed as a public NOXNA constant (mirroring
         *       `GamePad::LeftDeadZone`/`RightDeadZone`/`TriggerThreshold`) since C++ has no
         *       assembly-internal visibility and other translation units (`GestureDetector`, tests)
         *       need it.
         */
        NOXNA static constexpr intcs MAX_TOUCHES = 8;

        /**
         * @brief Marker used when no finger is present for a touch slot.
         * @note NOXNA — FNA declares this `internal const int NO_FINGER` (TouchPanel.cs:26); see
         *       MAX_TOUCHES's note for why it is exposed as a public NOXNA constant in CNA.
         */
        NOXNA static constexpr intcs NO_FINGER = -1;

        /**
         * @brief Sets the display width used for normalized touch coordinates.
         * @param value The display width in pixels.
         */
        static void setDisplayWidthProperty(intcs value);

        /**
         * @brief Sets the display height used for normalized touch coordinates.
         * @param value The display height in pixels.
         */
        static void setDisplayHeightProperty(intcs value);

        /**
         * @brief Gets the gesture types currently enabled for detection.
         * @return The enabled gesture types.
         */
        [[nodiscard]] static GestureType getEnabledGesturesProperty();

        /**
         * @brief Gets whether a touch device is currently known to exist.
         * @note NOXNA — FNA declares `TouchDeviceExists` `internal`, not part of the
         *       public XNA `TouchPanel` API. Exposed for the platform input bridge and
         *       `FrameworkDispatcher`'s `Update()` gate.
         * @return True if a touch device exists; false otherwise.
         */
        NOXNA [[nodiscard]] static bool getTouchDeviceExistsProperty();

        /**
         * @brief Sets whether a touch device is currently known to exist.
         * @note NOXNA — see getTouchDeviceExistsProperty().
         * @param value True if a touch device exists; false otherwise.
         */
        NOXNA static void setTouchDeviceExistsProperty(bool value);

        /**
         * @brief Returns touch panel capabilities.
         * @return The touch panel capabilities.
         */
        [[nodiscard]] static TouchPanelCapabilities GetCapabilities();

        /**
         * @brief Returns the current touch state snapshot.
         * @return The current touch collection.
         */
        [[nodiscard]] static TouchCollection GetState();

        /**
         * @brief Queues a gesture sample for later retrieval via ReadGesture.
         * @note NOXNA — FNA declares `EnqueueGesture` `internal`, not part of the public
         *       XNA `TouchPanel` API. Exposed for `GestureDetector`.
         * @param gesture The gesture sample to enqueue.
         */
        NOXNA static void EnqueueGesture(const GestureSample& gesture);

        /**
         * @brief Handles a normalized platform touch event used by gesture processing.
         * @note NOXNA — FNA declares `INTERNAL_onTouchEvent` `internal`, not part of the
         *       public XNA `TouchPanel` API. Exposed for the platform input bridge.
         * @param fingerId The finger identifier.
         * @param state The touch location state of this event.
         * @param x The normalized x coordinate.
         * @param y The normalized y coordinate.
         * @param dx The x delta since the last event.
         * @param dy The y delta since the last event.
         */
        NOXNA static void INTERNAL_onTouchEvent(
            intcs fingerId,
            TouchLocationState state,
            float x,
            float y,
            float dx,
            float dy
        );

        /**
         * @brief Updates one touch slot with a finger id and pixel position.
         * @note NOXNA — FNA declares `SetFinger` `internal`, not part of the public XNA
         *       `TouchPanel` API.
         * @param index The slot index to update.
         * @param fingerId The finger identifier.
         * @param fingerPos The current finger position in pixels.
         */
        NOXNA static void SetFinger(intcs index, intcs fingerId, const Microsoft::Xna::Framework::Vector2& fingerPos);

        /**
         * @brief Advances touch panel state by one frame.
         *
         * Copies the SetFinger()-driven touch array to its previous-frame snapshot, advances the
         * event-driven InputManager touch map by one frame (see InputManager::AdvanceTouchFrame —
         * promotes Pressed to Moved, retires Released touches), and updates gesture detection.
         * Must be called at most once per frame; GetState() itself no longer mutates state.
         *
         * @note NOXNA — FNA declares `Update` `internal`, not part of the public XNA
         *       `TouchPanel` API. Exposed for `FrameworkDispatcher::Update()`.
         */
        NOXNA static void Update();

    private:
        static intcs displayWidth_;
        static intcs displayHeight_;
        static GestureType enabledGestures_;
        static bool touchDeviceExists_;

        static std::queue<GestureSample> gestures_;
        static std::array<TouchLocation, MAX_TOUCHES> touches_;
        static std::array<TouchLocation, MAX_TOUCHES> previousTouches_;
        static std::vector<TouchLocation> validTouches_;

        static void updateInputManagerTouch(intcs fingerId, TouchLocationState state,
                                            const Microsoft::Xna::Framework::Vector2& position);
    };
}

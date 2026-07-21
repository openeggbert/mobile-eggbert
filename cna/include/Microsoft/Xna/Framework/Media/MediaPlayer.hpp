// SPDX-License-Identifier: MS-PL
#pragma once

#include <chrono>
#include <random>

#include "CNA/CNAHelper.hpp"
#include "Microsoft/Xna/Framework/FrameworkDispatcher.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Media/MediaQueue.hpp"
#include "Microsoft/Xna/Framework/Media/MediaState.hpp"
#include "Microsoft/Xna/Framework/Media/Song.hpp"
#include "System/EventArgs.hpp"
#include "System/EventHandler.hpp"
#include "System/TimeSpan.hpp"

namespace Microsoft::Xna::Framework::Media
{
    /** @brief Static media playback controller for songs and the active song queue. */
    class MediaPlayer final
    {
    public:
        MediaPlayer() = delete;

        /** @brief Raised when the active song changes. */
        static System::EventHandler<System::EventArgs> ActiveSongChanged;

        /** @brief Raised when the playback state changes. */
        static System::EventHandler<System::EventArgs> MediaStateChanged;

        /**
         * @brief Gets the current playback state.
         *
         * @return Current MediaState.
         */
        [[nodiscard]] static MediaState getStateProperty();

        /** @brief Advances playback to the next song in the queue. */
        static void MoveNext();

        /** @brief Stops playback and resets the active queue playback state. */
        static void Stop();

        /** @brief Performs pending media-player maintenance (timer updates, state transitions). */
        NOXNA static void Update();

        /** @brief Raises the deferred ActiveSongChanged event. */
        NOXNA static void OnActiveSongChanged();

        /** @brief Raises the deferred MediaStateChanged event. */
        NOXNA static void OnMediaStateChanged();

        /**
         * @brief Fallback song-end detector for builds without a native track-stopped signal
         * (i.e. without SOUND_ENABLED).
         *
         * Compares elapsed playback time against the song's known duration. Only reports "ended"
         * when duration is genuinely known (greater than zero) -- an unset/zero Duration (e.g. a
         * Song constructed without one) can't be detected this way, so playback simply never
         * auto-advances in that case, rather than false-triggering immediately at time zero.
         * Always compiled (not gated by SOUND_ENABLED) so it can be exercised directly by tests
         * regardless of which audio backend a given build has.
         *
         * @param activeSong The currently active song (may be nullptr).
         * @param elapsed    Elapsed playback time since the song started.
         * @return true if the song should be considered ended.
         */
        NOXNA [[nodiscard]] static bool DetectSongEndedByElapsedTime(
            Song* activeSong, System::TimeSpan elapsed);

    private:
        static bool isMuted_;
        static bool isRepeating_;
        static bool isShuffled_;
        static MediaState state_;
        static float volume_;
        static bool initialized_;
        static SharpRuntime::intcs numSongsInQueuePlayed_;
        static MediaQueue queue_;
        static std::mt19937 random_;

        static bool timerRunning_;
        static std::chrono::steady_clock::time_point timerStart_;
        static std::chrono::duration<double> accumulatedTime_;

        static void setStateProperty(MediaState value);
        static void NextSong(SharpRuntime::intcs direction);
        static void PlaySong(Song* song);

        static void TimerStart();
        static void TimerStop();
        static void TimerReset();
        [[nodiscard]] static System::TimeSpan TimerElapsed();
    };
}

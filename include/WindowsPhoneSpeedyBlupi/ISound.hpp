/**
 * @file ISound.hpp
 * @brief Declares the ISound interface abstracting all audio playback operations for the game.
 *
 * @details Provides the pure-virtual API for loading sound assets, playing positional sound
 * effects, controlling volume, and stopping playback. The concrete implementation is Sound.
 * Positional audio uses the logical 640x480 game-space coordinate system.
 */

#pragma once

namespace WindowsPhoneSpeedyBlupi
{
    enum class SoundChannel : SharpRuntime::ubytecs;

    /**
     * @class ISound
     * @brief Interface for the game's audio playback subsystem.
     *
     * @details ISound abstracts sound effect playback, volume control, and positional audio
     * for gameplay and UI events. The concrete implementation (Sound) loads all
     * sound effect assets from content and manages a list of active Play instances.
     *
     * Responsibilities:
     * - Loading sound effect assets (LoadContent).
     * - Playing sounds by SoundChannel index with positional volume/balance (PlayImage).
     * - Updating the stereo position of a looping sound (PosImage).
     * - Stopping individual channels or all active sounds.
     * - Controlling master audio and MIDI/music volume independently.
     *
     * Does not own or modify gameplay state. Pure audio resource code.
     *
     * Positional audio convention:
     * - The @p pos parameter to PlayImage/PosImage is a point in logical game-space (640x480).
     * - Volume and stereo balance are computed from the distance and horizontal
     *   offset of @p pos relative to the viewport center.
     *
     * @note Sound triggers in Decor are tied to gameplay events. Do not move
     *       PlayImage calls without preserving the original event timing.
     */
    class ISound
    {
    public:
        virtual ~ISound() = default;

        /** @brief Loads all sound effect assets from content. Must be called before any Play call. */
        virtual void LoadContent() = 0;

        /**
         * @brief Initialises the audio device and playback state.
         * @return True if the audio device was created successfully; false on failure.
         */
        virtual bool Create() = 0;

        /**
         * @brief Enables or disables all sound output globally.
         * @param[in] bState True to enable audio output; false to silence it.
         */
        virtual void SetState(bool bState) = 0;

        /**
         * @brief Enables or disables CD/MIDI background music playback.
         * @param[in] bAudio True to enable background music; false to disable it.
         */
        virtual void SetCDAudio(bool bAudio) = 0;

        /**
         * @brief Returns true if audio output is currently enabled.
         * @return True if sound is enabled; false if silenced.
         */
        virtual bool GetEnable() = 0;

        /**
         * @brief Sets the master sound effect volume.
         * @param[in] volume Volume level in the range [0, Sound::MAXVOLUME].
         */
        virtual void SetAudioVolume(int volume) = 0;

        /**
         * @brief Returns the current master sound effect volume.
         * @return Current volume in the range [0, Sound::MAXVOLUME].
         */
        virtual int GetAudioVolume() = 0;

        /**
         * @brief Sets the MIDI/background music volume.
         * @param[in] volume Volume level in the range [0, Sound::MAXVOLUME].
         */
        virtual void SetMidiVolume(int volume) = 0;

        /**
         * @brief Returns the current MIDI/background music volume.
         * @return Current MIDI volume in the range [0, Sound::MAXVOLUME].
         */
        virtual int GetMidiVolume() = 0;

        /** @brief Stops all currently active sound effect instances immediately. */
        virtual void StopAll() = 0;

        /**
         * @brief Plays a sound effect on the given channel at a logical game-space position.
         *
         * @details Volume and stereo balance are computed from @p pos relative to the viewport.
         * If @p rank is -1, the sound asset index is derived from @p channel.
         * If @p bLoop is true, the sound plays indefinitely until explicitly stopped.
         *
         * @param[in] channel The sound effect slot to use.
         * @param[in] pos     Source position in logical game-space (used for spatial audio).
         * @param[in] rank    Override sound asset index, or -1 to use the channel default.
         * @param[in] bLoop   If true, the sound loops until explicitly stopped via Stop().
         * @return True if playback started successfully; false on failure.
         */
        virtual bool PlayImage(SoundChannel channel, TinyPoint pos, int rank = -1, bool bLoop = false) = 0;

        /**
         * @brief Updates the stereo position of a looping sound on the given channel.
         *
         * @details Recomputes volume and stereo balance from @p pos without restarting playback.
         * Intended for moving sound sources (e.g., vehicle engine sounds).
         *
         * @param[in] channel Channel of the active looping sound to reposition.
         * @param[in] pos     New source position in logical game-space.
         * @return True if the channel was found and repositioned; false if not active.
         */
        virtual bool PosImage(SoundChannel channel, TinyPoint pos) = 0;

        /**
         * @brief Stops the sound playing on the given channel.
         * @param[in] channel The channel to stop.
         * @return True if an active sound was found and stopped; false if the channel was silent.
         */
        virtual bool Stop(SoundChannel channel) = 0;
    };
}

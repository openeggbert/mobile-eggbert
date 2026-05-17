#pragma once

namespace WindowsPhoneSpeedyBlupi
{
    enum class SoundChannel : SharpRuntime::ubytecs;

    /**
     * @brief Interface for the game's audio playback subsystem.
     *
     * ISound abstracts sound effect playback, volume control, and positional audio
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
     * - @p pos is a point in logical game-space (640x480 coordinate system).
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

        /** @brief Loads all sound effect assets. Must be called before any Play call. */
        virtual void LoadContent() = 0;

        /** @brief Initialises the audio device and playback state. Returns false on failure. */
        virtual bool Create() = 0;

        /** @brief Enables or disables all sound output globally. */
        virtual void SetState(bool bState) = 0;

        /** @brief Enables or disables CD/MIDI background music. */
        virtual void SetCDAudio(bool bAudio) = 0;

        /** @brief Returns true if audio output is currently enabled. */
        virtual bool GetEnable() = 0;

        /**
         * @brief Sets the master sound effect volume.
         * @param volume Volume level in [0, Sound::MAXVOLUME].
         */
        virtual void SetAudioVolume(int volume) = 0;

        /** @brief Returns the current master sound effect volume. */
        virtual int GetAudioVolume() = 0;

        /**
         * @brief Sets the MIDI/music volume.
         * @param volume Volume level in [0, Sound::MAXVOLUME].
         */
        virtual void SetMidiVolume(int volume) = 0;

        /** @brief Returns the current MIDI/music volume. */
        virtual int GetMidiVolume() = 0;

        /** @brief Stops all currently active sound effect instances. */
        virtual void StopAll() = 0;

        /**
         * @brief Plays a sound effect on the given channel at a logical game-space position.
         *
         * Volume and stereo balance are computed from @p pos relative to the viewport.
         * If @p rank is -1, the sound asset index is derived from @p channel.
         * If @p bLoop is true, the sound plays indefinitely until stopped.
         *
         * @param channel Sound effect slot to use.
         * @param pos Source position in logical game-space (used for spatial audio).
         * @param rank Override sound asset index, or -1 to use the channel default.
         * @param bLoop If true, the sound loops until explicitly stopped.
         * @return True if playback started successfully.
         */
        virtual bool PlayImage(SoundChannel channel, TinyPoint pos, int rank = -1, bool bLoop = false) = 0;

        /**
         * @brief Updates the stereo position of a looping sound on the given channel.
         * @param channel Channel of the active looping sound to reposition.
         * @param pos New source position in logical game-space.
         * @return True if the channel was found and repositioned.
         */
        virtual bool PosImage(SoundChannel channel, TinyPoint pos) = 0;

        /**
         * @brief Stops the sound playing on the given channel.
         * @param channel Channel to stop.
         * @return True if the channel was found and stopped.
         */
        virtual bool Stop(SoundChannel channel) = 0;
    };
}

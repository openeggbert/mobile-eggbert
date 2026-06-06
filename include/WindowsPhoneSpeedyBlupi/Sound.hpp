/**
 * @file Sound.hpp
 * @brief Declaration of the Sound audio subsystem implementation.
 *
 * @details
 * Sound is the concrete realisation of the ISound interface. It manages the
 * complete audio lifecycle for mobile-eggbert:
 * - Asset loading: reads up to 93 WAV files from the content pipeline
 *   (sounds/sound000.wav ... sounds/sound092.wav).
 * - Per-channel playback: each SoundChannel maps directly to a WAV asset
 *   index. Volume and pitch are modulated by the @c tableVolumePitch lookup.
 * - Stereo positioning: GetVolume() and GetBalance() translate a HUD-space
 *   point into XAudio2-compatible volume scalar and pan values.
 * - Active-play management: a @c std::list of Play objects tracks simultaneously
 *   playing instances. The list is pruned when it reaches 10 entries.
 * - Channel-conflict resolution: a second PlayImage() call for the same
 *   channel is silently dropped if the channel is still playing, except for
 *   SoundChannel10 which is always allowed to stack.
 *
 * @note Define @c SOUND_ENABLED before including this file (done in Sound.cpp)
 *       to enable the full implementation. Without it, @c SOUND_DISABLED is
 *       defined and all methods return stub values without touching hardware.
 *
 * @see ISound
 * @see SoundChannel
 */
#pragma once

#include <list>

#include "IGame1.hpp"
#include "GameData.hpp"
#include "ISound.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundEffect.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundEffectInstance.hpp"

namespace WindowsPhoneSpeedyBlupi
{
    /**
     * @class Sound
     * @brief Concrete audio subsystem: loads WAV assets, manages playback
     *        instances, and applies per-channel volume/pitch modulation.
     *
     * @details
     * Sound implements ISound using the CNA (SDL3-based) audio backend.
     * It is non-copyable because it owns non-trivially-copyable SoundEffect
     * objects and a stateful Play list.
     *
     * ### Channel-to-asset mapping
     * The integer value of a SoundChannel enum is used directly as the index
     * into @c soundEffects. SoundChannel0 maps to sound000.wav, etc.  Up to 93
     * channels/assets are loaded at startup by LoadContent().
     *
     * ### Volume/pitch lookup table
     * @c tableVolumePitch is a flat array of 100 (volume, pitch) pairs indexed
     * by channel number:
     * @code
     *   volume[ch] = tableVolumePitch[ch * 2]
     *   pitch[ch]  = tableVolumePitch[ch * 2 + 1]
     * @endcode
     * This allows each sound effect to have individually tuned playback
     * characteristics without per-instance configuration at call sites.
     *
     * ### Stereo positioning
     * GetVolume() and GetBalance() map a HUD-space TinyPoint (logical size
     * 640x480) to an XAudio2-compatible volume scalar and pan value.
     * Sounds placed outside the visible area are attenuated linearly to 0.
     *
     * ### Channel-conflict policy
     * If PlayImage() is called for a channel that already has a playing
     * instance, the new request is silently dropped (returns @c true without
     * creating a new Play).  The exception is SoundChannel10, which may
     * overlap freely.  This prevents a rapidly-repeating trigger from spamming
     * the same sample.
     *
     * @note Not thread-safe. All public methods must be called from the main
     *       game thread.
     * @warning Maximum concurrent play count is capped at 10. When the limit is
     *          reached, stopped instances are evicted; if none are stopped, the
     *          new sound is silently dropped.
     * @see ISound
     * @see SoundChannel
     */
    class Sound : public ISound
    {
        /**
         * @class Play
         * @brief RAII wrapper that owns a single playing SoundEffectInstance.
         *
         * @details
         * Constructed by PlayImage() with a reference to the source SoundEffect
         * and the computed volume/balance/pitch values. The constructor looks
         * up per-channel modifiers from @c tableVolumePitch, applies them, and
         * immediately calls @c sei.Play().
         *
         * getIsFreeProperty() returns @c true once the underlying SoundState
         * transitions to Stopped, signalling that the Play slot may be reused
         * or erased from the @c plays list.
         */
        class Play
        {
            Microsoft::Xna::Framework::Audio::SoundEffectInstance sei; ///< @brief Owned sound-effect instance. Plays immediately on construction.
            const SoundChannel channel; ///< @brief Channel this instance was created for; immutable after construction.

        public:
            /**
             * @brief Returns the channel this Play instance was created for.
             * @return The SoundChannel value passed to the constructor.
             */
            [[nodiscard]] SoundChannel getChannelProperty() const;

            /**
             * @brief Returns @c true if the underlying sound effect has finished
             *        playing and this slot can be reused or erased.
             *
             * @details Queries @c sei.getStateProperty() == SoundState::Stopped.
             *          A looped sound is never free until Stop() is explicitly called.
             * @return @c true if the instance has stopped playing; @c false otherwise.
             */
            [[nodiscard]] bool getIsFreeProperty() const;

            /**
             * @brief Constructs a Play object and immediately starts audio output.
             *
             * @details
             * Applies per-channel modulation from the @c tableVolumePitch table:
             * - The caller-supplied @p volume is multiplied by
             *   @c tableVolumePitch[ToRaw(channel) * 2].
             * - The @p pitch argument is overridden by
             *   @c tableVolumePitch[ToRaw(channel) * 2 + 1]; the caller's value
             *   is ignored if the channel index is within bounds.
             * - Pitch is clamped to @c 0.0 from below before being applied.
             *
             * @param[in] se       Source SoundEffect providing the audio data.
             * @param[in] channel  Logical sound channel; used for tableVolumePitch lookup.
             * @param[in] volume   Base volume scalar [0.0, 1.0]; multiplied by the
             *                     table's volume entry for this channel.
             * @param[in] balance  Stereo pan [-1.0 = full left, 0.0 = centre,
             *                     1.0 = full right].
             * @param[in] pitch    Caller-supplied pitch hint; overridden by the
             *                     tableVolumePitch entry for this channel.
             * @param[in] isLooped @c true to loop the sound until Stop() is called.
             *
             * @post @c sei.Play() has been called; audio output has begun.
             * @note If the channel index falls outside [0, tableVolumePitchLength),
             *       the table lookup is skipped and the raw @p volume is used.
             */
            Play(Microsoft::Xna::Framework::Audio::SoundEffect& se, SoundChannel channel, double volume, double balance,
                 double pitch, bool isLooped);

            /**
             * @brief Stops audio playback immediately.
             * @post @c sei state is Stopped; getIsFreeProperty() returns @c true.
             */
            void Stop();
        };

        /// @brief Total number of elements in @c tableVolumePitch (100 channel pairs x 2 = 200).
        static constexpr short tableVolumePitchLength = 200;

        /**
         * @brief Per-channel volume and pitch lookup table.
         *
         * @details
         * Contains @c tableVolumePitchLength (200) elements arranged as 100
         * consecutive (volume, pitch) pairs, one pair per logical sound channel:
         * @code
         *   volume multiplier : tableVolumePitch[channel * 2]
         *   pitch shift       : tableVolumePitch[channel * 2 + 1]
         * @endcode
         *
         * **Volume multiplier** is in [0.0, 1.0] and is applied to the
         * caller-supplied volume scalar before passing it to
         * SoundEffectInstance. Values below 1.0 permanently lower the loudness
         * of that channel relative to all other channels.
         *
         * **Pitch shift** is an additive value in the XAudio2 pitch convention
         * (0.0 = no shift, 1.0 = one octave up). It replaces the pitch
         * argument passed to the Play constructor; the table value is the
         * sole source of pitch for each channel.
         *
         * The table was ported verbatim from the original Windows Phone XNA
         * C# implementation to preserve the exact audio character of the
         * original game.
         *
         * @warning Access must be bounds-checked: valid indices are
         *          [0, tableVolumePitchLength).  The Play constructor enforces
         *          this check before dereferencing.
         */
        static constexpr double tableVolumePitch[tableVolumePitchLength] =
        {
            1.0, 0.0, 0.5, 1.0, 0.5, 1.0, 1.0, 0.2, 1.0, 0.2,
            1.0, 0.1, 1.0, 0.3, 1.0, 0.2, 1.0, 0.3, 1.0, 0.5,
            1.0, 0.2, 1.0, 0.2, 1.0, 0.1, 1.0, 0.2, 1.0, 0.2,
            1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.2,
            1.0, 0.2, 1.0, 0.2, 0.7, 0.2, 1.0, 0.1, 1.0, 0.1,
            1.0, 0.2, 1.0, 0.2, 1.0, 0.4, 1.0, 0.0, 1.0, 0.0,
            1.0, 0.0, 1.0, 0.0, 1.0, 0.2, 1.0, 0.2, 0.7, 0.4,
            1.0, 0.2, 1.0, 0.4, 1.0, 0.2, 0.5, 1.0, 0.5, 1.0,
            1.0, 0.4, 1.0, 0.2, 1.0, 0.2, 1.0, 0.2, 1.0, 0.2,
            1.0, 0.2, 1.0, 0.2, 1.0, 0.2, 0.6, 0.4, 0.8, 0.1,
            0.6, 0.5, 1.0, 0.2, 1.0, 0.2, 1.0, 0.2, 1.0, 0.2,
            1.0, 0.2, 1.0, 0.2, 1.0, 0.2, 1.0, 0.2, 1.0, 0.2,
            1.0, 0.2, 1.0, 0.2, 1.0, 0.2, 1.0, 0.2, 1.0, 0.0,
            1.0, 0.2, 1.0, 0.2, 1.0, 0.2, 1.0, 0.2, 1.0, 0.0,
            1.0, 0.2, 1.0, 0.2, 1.0, 0.0, 1.0, 0.0, 1.0, 0.2,
            1.0, 0.2, 1.0, 0.2, 1.0, 0.2, 1.0, 0.2, 1.0, 0.2,
            1.0, 0.2, 1.0, 0.2, 1.0, 0.2, 1.0, 0.2, 0.6, 0.4,
            1.0, 0.2, 1.0, 0.2, 1.0, 0.2, 1.0, 0.2, 1.0, 0.2,
            1.0, 0.2, 1.0, 0.2, 1.0, 0.2, 1.0, 0.2, 1.0, 0.2,
            1.0, 0.2, 1.0, 0.2, 1.0, 0.2, 1.0, 0.2, 1.0, 0.2
        };

    public:
        /// @brief Virtual destructor; Sound owns no resources that require custom cleanup.
        virtual ~Sound() = default;

        /// @brief Maximum integer volume level accepted by SetAudioVolume() / returned by GetAudioVolume().
        static constexpr int MAXVOLUME = 20;

    private:
        IGame1* game1; ///< @brief Non-owning pointer to the game host; used to access the content manager. Never null.

        const GameData& gameData; ///< @brief Reference to persistent game configuration; read for the @c sounds enabled flag.

        std::vector<Microsoft::Xna::Framework::Audio::SoundEffect> soundEffects; ///< @brief Loaded WAV assets indexed by SoundChannel integer value.

        std::list<Play> plays; ///< @brief Currently active or recently stopped sound instances. Capped at 10 entries.

        double volume; ///< @brief Global audio volume scalar in [0.0, 1.0]; set by SetAudioVolume() and applied in GetVolume().

    public:
        /**
         * @brief Constructs the Sound subsystem and initialises the master volume to 1.0.
         *
         * @details Sets the XAudio2 master volume to 1.0f via
         *          SoundEffect::setMasterVolumeProperty() (unless SOUND_DISABLED).
         *          Sound assets are not loaded here; call LoadContent() before
         *          the first PlayImage() call.
         *
         * @param[in] game1    Non-owning pointer to the game host. Must remain
         *                     valid for the lifetime of this Sound object.
         * @param[in] gameData Reference to the persistent game data. Must remain
         *                     valid for the lifetime of this Sound object.
         */
        Sound(IGame1* game1, GameData& gameData);

        /// @brief Deleted copy constructor. Sound is not copyable.
        Sound(const Sound&) = delete;
        /// @brief Deleted copy-assignment operator. Sound is not copyable.
        Sound& operator=(const Sound&) = delete;

        /**
         * @brief Loads all WAV sound-effect assets from the content pipeline.
         *
         * @details Loads exactly 93 assets named @c sounds/sound000.wav through
         *          @c sounds/sound092.wav into @c soundEffects. Previously loaded
         *          assets are cleared before loading. If @c Def::getHasSoundProperty()
         *          returns @c false, loading is skipped silently.
         *
         * @pre The game content manager is initialised.
         * @post soundEffects.size() == 93 (or 0 if sound is disabled/unavailable).
         */
        void LoadContent() override;

        /**
         * @brief Performs any additional post-load initialisation. Currently a no-op.
         * @return Always @c true.
         */
        bool Create() override;

        /**
         * @brief Enables or disables the audio subsystem globally (stub; no-op).
         *
         * @details Present for ISound interface compatibility. The original XNA
         *          implementation used this for audio focus management on Windows
         *          Phone; it is not needed on the current target platforms.
         * @param[in] bState Desired enabled state (ignored).
         */
        void SetState(bool bState) override;

        /**
         * @brief Enables or disables CD audio (stub; no-op).
         *
         * @details No CD audio support is implemented. Present for ISound
         *          interface compatibility only.
         * @param[in] bAudio Desired CD audio state (ignored).
         */
        void SetCDAudio(bool bAudio) override;

        /**
         * @brief Returns whether the audio subsystem is enabled.
         * @return Always @c true; the subsystem is always considered enabled.
         */
        bool GetEnable() override;

        /**
         * @brief Sets the global audio volume.
         *
         * @details Stores @c volume / MAXVOLUME as an internal scalar applied
         *          by GetVolume(). Does not affect currently playing instances;
         *          only newly started sounds use the updated volume.
         *
         * @param[in] volume Integer volume level in [0, MAXVOLUME].
         */
        void SetAudioVolume(int volume) override;

        /**
         * @brief Returns the current global audio volume level.
         * @return Integer in [0, MAXVOLUME] corresponding to the current scalar.
         */
        int GetAudioVolume() override;

        /**
         * @brief Sets the MIDI volume (stub; no-op). No MIDI playback is implemented.
         * @param[in] volume Desired MIDI volume (ignored).
         */
        void SetMidiVolume(int volume) override;

        /**
         * @brief Returns the current MIDI volume.
         * @return Always 0; MIDI is not implemented.
         */
        int GetMidiVolume() override;

        /**
         * @brief Stops all currently playing sound effects and clears the play list.
         * @post plays.empty() == true.
         */
        void StopAll() override;

        /**
         * @brief Plays the sound effect associated with @p channel at the given
         *        screen position.
         *
         * @details
         * Steps performed:
         * -# Returns immediately if @c gameData.getSoundsProperty() is @c false.
         * -# Validates that @c ToRaw(channel) is within the @c soundEffects range.
         * -# Channel-conflict check: if the channel is not SoundChannel10 and a
         *    non-free Play for this channel exists, the call is silently dropped
         *    (returns @c true).
         * -# Play-list pruning: if @c plays.size() >= 10, free (stopped) entries
         *    are erased until the count drops below 10. If no free entry can be
         *    found the new sound is dropped.
         * -# Creates a new Play with volume and balance derived from @p pos via
         *    GetVolume() and GetBalance().
         *
         * @param[in] channel  Sound channel (and asset index) to play.
         * @param[in] pos      HUD-space position used for volume and balance
         *                     attenuation. Pass TinyPoint(320, 240) for a centred,
         *                     full-volume sound.
         * @param[in] rank     Unused in the current implementation (default -1).
         * @param[in] bLoop    @c true to loop the sound until Stop() is called.
         * @return Always @c true (errors are silently swallowed).
         *
         * @note SoundChannel10 bypasses the channel-conflict check and may stack
         *       multiple simultaneous instances.
         */
        bool PlayImage(SoundChannel channel, TinyPoint pos, int rank = -1, bool bLoop = false) override;

        /**
         * @brief Updates the stereo position of an already-playing sound (stub).
         *
         * @details Not implemented in this backend; present for ISound interface
         *          compatibility. Position is fixed at Play construction time.
         *
         * @param[in] channel  Channel whose position to update (ignored).
         * @param[in] pos      New HUD-space position (ignored).
         * @return Always @c true.
         */
        bool PosImage(SoundChannel channel, TinyPoint pos) override;

        /**
         * @brief Stops all playing instances on the given channel and removes them
         *        from the play list.
         *
         * @param[in] channel  The channel whose instances should be stopped.
         * @return Always @c true.
         * @post No Play with the given channel remains in @c plays.
         */
        bool Stop(SoundChannel channel) override;

    private:
        /**
         * @brief Computes an attenuated volume scalar for a sound at the given
         *        HUD-space position.
         *
         * @details
         * The logical play area is 640 px wide and 480 px tall. Attenuation is
         * applied independently on each axis and the minimum of the two results
         * is used:
         *
         * - **Horizontal**: full volume for X in [0, 640]. For X < 0, volume
         *   decreases linearly as @c 1 + (X / 640) * 2; for X > 640 the excess
         *   is treated symmetrically. Clamped to [0.0, 1.0].
         * - **Vertical**: full volume for Y in [0, 480]. For Y < 0, volume
         *   decreases as @c 1 + (Y / 480) * 3; for Y > 480 symmetrically.
         *   Clamped to [0.0, 1.0]. The vertical falloff is steeper (factor 3
         *   vs 2) because the play area is narrower in height.
         *
         * The final value is multiplied by the global @c volume scalar set by
         * SetAudioVolume().
         *
         * @param[in] pos  HUD-space position of the sound source.
         * @return Volume scalar in [0.0, 1.0].
         */
        double GetVolume(TinyPoint pos);

        /**
         * @brief Computes a stereo pan value for a sound at the given HUD-space
         *        position.
         *
         * @details Maps @c pos.X linearly from the 640-px-wide logical area to
         *          the XAudio2 pan range [-1.0, 1.0]:
         * @code
         *   balance = clamp(pos.X * 2.0 / 640.0 - 1.0,  -1.0,  1.0)
         * @endcode
         * X = 0 maps to -1.0 (full left), X = 320 maps to 0.0 (centre),
         * X = 640 maps to 1.0 (full right). Values outside [0, 640] are clamped.
         *
         * @param[in] pos  HUD-space position of the sound source.
         * @return Pan scalar in [-1.0, 1.0].
         */
        double GetBalance(TinyPoint pos);
    };
}

/**
 * @file Sound.cpp
 * @brief Implementation of the Sound audio subsystem.
 *
 * @details
 * This file contains the complete implementation of the Sound class declared
 * in Sound.hpp.  Key implementation details that supplement the header
 * documentation are noted below.
 *
 * ### SOUND_DISABLED guard
 * Unlike the header, this file defines @c SOUND_ENABLED (if it is not already
 * defined externally) and tests for @c SOUND_DISABLED at the top of most
 * methods.  When @c SOUND_DISABLED is active every method returns a safe stub
 * value or does nothing; the XAudio2/SDL3 audio backend is never touched.
 * This mode is useful for headless unit-test builds.
 *
 * ### LoadContent asset naming
 * Assets are loaded with a zero-padded three-digit counter:
 * @code
 *   sounds/sound000.wav, sounds/sound001.wav, ..., sounds/sound092.wav
 * @endcode
 * The padding is generated with @c std::setw(3) / @c std::setfill('0') so
 * the filenames are portable across all target platforms.
 *
 * ### Volume attenuation formula (GetVolume)
 * Two independent 1-D attenuation values (one per axis) are computed and the
 * minimum is used as the final scalar:
 * - Horizontal axis (full-volume range 0..640 px):
 *   @code val_x = clamp(1 + (X / 640) * 2,  0, 1)  for X < 0
 *         val_x = clamp(1 - ((X-640)/640)*2, 0, 1)  for X > 640 @endcode
 * - Vertical axis (full-volume range 0..480 px, steeper falloff factor 3):
 *   @code val_y = clamp(1 + (Y / 480) * 3,  0, 1)  for Y < 0
 *         val_y = clamp(1 - ((Y-480)/480)*3, 0, 1)  for Y > 480 @endcode
 * The steeper vertical factor ensures off-screen sounds above or below the
 * play area fade out more quickly than side-exit sounds, matching the
 * original Windows Phone game's audio feel.
 *
 * ### Panning formula (GetBalance)
 * A simple linear map from HUD X coordinate to the XAudio2 pan range:
 * @code
 *   balance = clamp(X * 2.0 / 640.0 - 1.0,  -1.0, 1.0)
 * @endcode
 * X = 0 → -1.0 (full left), X = 320 → 0.0 (centre), X = 640 → +1.0 (full right).
 *
 * @note All methods in this file that manipulate the @c plays list do so
 *       on the main game thread. The Play constructor (which calls sei.Play())
 *       is also on the main thread, so no synchronisation is needed for the
 *       list itself.
 */
#include "WindowsPhoneSpeedyBlupi/Sound.hpp"

#include <algorithm>
#include <iomanip>
#include "WindowsPhoneSpeedyBlupi/def/SoundChannel.hpp"

#ifndef SOUND_ENABLED
#define SOUND_DISABLED
#endif

namespace WindowsPhoneSpeedyBlupi
{
    // C++14 has no inline variables (that's C++17) -- a static constexpr array member that is
    // ODR-used (as this is, via [] indexing) needs this out-of-class definition or the linker
    // cannot find it.
    constexpr double Sound::tableVolumePitch[Sound::tableVolumePitchLength];

    SoundChannel Sound::Play::getChannelProperty() const { return channel; }

    bool Sound::Play::getIsFreeProperty() const
    {
#ifdef SOUND_DISABLED
        return false;
#endif
        return sei.getStateProperty() == Microsoft::Xna::Framework::Audio::SoundState::Stopped;
    }

    /**
     * @brief Constructs a Play object to manage and play a sound effect instance with specified parameters.
     *
     * This constructor initializes a sound effect instance for playback with customization options
     * such as volume, balance, pitch, and looping behavior. It also applies pre-defined volume
     * and pitch adjustments based on the specified channel using a lookup table.
     *
     * @param se A reference to the SoundEffect object that provides the sound data.
     * @param channel An integer representing the audio channel to assign this sound effect.
     *                Channel influences volume and pitch adjustments from the lookup table.
     * @param volume A double specifying the initial playback volume. This is further adjusted
     *               based on the specified channel.
     * @param balance A double specifying the stereo panning of the sound. Value ranges typically
     *                between -1.0 (full left) to 1.0 (full right).
     * @param pitch A double specifying the pitch adjustment of the sound. Values greater than 0
     *              increase the pitch, while values less than 0 lower it (minimum is clamped to 0.0).
     * @param isLooped A boolean indicating whether the sound effect should loop continuously.
     */
    Sound::Play::Play(Microsoft::Xna::Framework::Audio::SoundEffect& se, SoundChannel channel, double volume, double balance,
                      double pitch, bool isLooped) :
        channel(channel),
        sei(se.CreateInstance())
    {
#ifdef SOUND_DISABLED
        int b;
        return;
#endif
        int tableVolumePitchLengthIndex = ToRaw(channel) * 2;
        if (tableVolumePitchLengthIndex >= 0 && tableVolumePitchLengthIndex < tableVolumePitchLength)
        {
            volume *= tableVolumePitch[tableVolumePitchLengthIndex];
            pitch = tableVolumePitch[tableVolumePitchLengthIndex + 1];
        }

        sei.setVolumeProperty((float)volume);
        sei.setPanProperty((float)balance);
        sei.setPitchProperty((float)(pitch < 0.0 ? 0.0 : pitch));
        sei.setIsLoopedProperty(isLooped);
        sei.Play();
    }

    void Sound::Play::Stop()
    {
#ifdef SOUND_DISABLED
        return;
#endif
        sei.Stop();
    }

    Sound::Sound(IGame1* game1, GameData& gameData) :
        game1(game1),
        gameData(gameData)
    {
        // soundEffects = new List<SoundEffect>();
        // plays = new List<Play>();
        volume = 1.0;
#ifndef SOUND_DISABLED
        Microsoft::Xna::Framework::Audio::SoundEffect::setMasterVolumeProperty(1.0f);
#endif
    }

    void Sound::LoadContent()
    {
#ifdef SOUND_DISABLED
        return;
#endif
        if (!Def::getHasSoundProperty())
        {
            return;
        }

        static constexpr SharpRuntime::intcs SOUND_COUNT = 93;

        soundEffects.clear();
        soundEffects.reserve(SOUND_COUNT);

        using Microsoft::Xna::Framework::Audio::SoundEffect;

        for (SharpRuntime::intcs i = 0; i < SOUND_COUNT; ++i)
        {
            std::ostringstream oss;
            oss << "sounds/sound"
                << std::setw(3)
                << std::setfill('0')
                << i
                << ".wav";

            soundEffects.push_back(
                game1->getContentProperty().Load<SoundEffect>(oss.str())
            );
        }
    }

    bool Sound::Create()
    {
        return true;
    }

    void Sound::SetState(bool bState)
    {
    }

    void Sound::SetCDAudio(bool bAudio)
    {
    }

    bool Sound::GetEnable()
    {
        return true;
    }

    void Sound::SetAudioVolume(int volume)
    {
#ifdef SOUND_DISABLED
        return;
#endif
        this->volume = (double)volume / (double)MAXVOLUME;
    }

    int Sound::GetAudioVolume()
    {
#ifdef SOUND_DISABLED
        return 1;
#endif
        return (int)(volume * (double)MAXVOLUME);
    }

    void Sound::SetMidiVolume(int volume)
    {
    }

    int Sound::GetMidiVolume()
    {
        return 0;
    }

    void Sound::StopAll()
    {
#ifdef SOUND_DISABLED
        return;
#endif
        for (auto& play : plays)
        {
            play.Stop();
        }
        plays.clear();
    }

    bool Sound::PlayImage(SoundChannel channel, TinyPoint pos, int rank, bool bLoop)
    {
#ifdef SOUND_DISABLED
        return true;
#endif
        if (!gameData.getSoundsProperty())
        {
            return true;
        }
        const intcs rawChannel = ToRaw(channel);
        if (rawChannel >= 0 && rawChannel < soundEffects.size())
        {
            if (channel != SoundChannel::SoundChannel10 && std::any_of(plays.begin(), plays.end(),
                                             [channel](const Play& p)
                                             {
                                                 return p.getChannelProperty() == channel && !p.getIsFreeProperty();
                                             }))
            {
                return true;
            }

            if (plays.size() >= 10)
            {
                for (auto it = plays.begin(); it != plays.end();)
                {
                    if (it->getIsFreeProperty())
                    {
                        it = plays.erase(it);
                    }
                    else
                    {
                        it++;
                    }

                    if (plays.size() < 10) break;
                }
            }


            plays.emplace_back(soundEffects[ToRaw(channel)], channel, (float)GetVolume(pos), (float)GetBalance(pos), 0.0,
                               bLoop);
        }
        return true;
    }

    bool Sound::PosImage(SoundChannel channel, TinyPoint pos)
    {
        return true;
    }

    bool Sound::Stop(SoundChannel channel)
    {
#ifdef SOUND_DISABLED
        return true;
#endif
        size_t num = 0;

        auto it = plays.begin();
        while (it != plays.end())
        {
            if (it->getChannelProperty() == channel)
            {
                it->Stop();
                it = plays.erase(it); // erase() returns the next valid iterator
            }
            else
            {
                ++it;
            }
        }

        return true;
    }

    double Sound::GetVolume(TinyPoint pos)
    {
#ifdef SOUND_DISABLED
        return 1.0;
#endif
        double val = 1.0;
        if (pos.X < 0)
        {
            val = 1.0 + (double)(pos.X / 640) * 2.0;
        }
        if (pos.X > 640)
        {
            pos.X -= 640;
            val = 1.0 - (double)(pos.X / 640) * 2.0;
        }
        val = std::max(val, 0.0);
        val = std::min(val, 1.0);
        double val2 = 1.0;
        if (pos.Y < 0)
        {
            val2 = 1.0 + (double)(pos.Y / 480) * 3.0;
        }
        if (pos.Y > 480)
        {
            pos.Y -= 480;
            val2 = 1.0 - (double)(pos.Y / 480) * 3.0;
        }
        val2 = std::max(val2, 0.0);
        val2 = std::min(val2, 1.0);
        return std::min(val, val2) * volume;
    }

    double Sound::GetBalance(TinyPoint pos)
    {
#ifdef SOUND_DISABLED
        return 1.0;
#endif
        double val = (double)pos.X * 2.0 / 640.0 - 1.0;
        val = std::max(val, -1.0);
        return std::min(val, 1.0);
    }
}

//
// Created by robertvokac on 5/24/25.
//

#include "WindowsPhoneSpeedyBlupi/Sound.h"

#include <algorithm>

namespace WindowsPhoneSpeedyBlupi {
    int Sound::Play::getChannel() const { return channel; }
    bool Sound::Play::getIsFree() const { return sei.State == Microsoft::Xna::Framework::Audio::SoundState::Stopped; }

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
    Sound::Play::Play(Microsoft::Xna::Framework::Audio::SoundEffect& se, int channel, double volume, double balance, double pitch, bool isLooped):
        channel(channel),
        sei(se.CreateInstance())
    {
        int tableVolumePitchLengthIndex = channel * 2;
        if (tableVolumePitchLengthIndex >= 0 && tableVolumePitchLengthIndex < tableVolumePitchLength)
        {
            volume *= tableVolumePitch[tableVolumePitchLengthIndex];
            pitch = tableVolumePitch[tableVolumePitchLengthIndex + 1];
        }

        sei.setVolume((float)volume);
        sei.setPan((float)balance);
        sei.setPitch((float)(pitch < 0.0 ? 0.0: pitch));
        sei.setIsLooped(isLooped);
        sei.Play();
    }
    void Sound::Play::Stop()
    {
        sei.Stop();
    }

    Sound::Sound(Microsoft::Xna::Framework::Game& game1, GameData& gameData):
    game1(game1),
    gameData(gameData)
    {
        // soundEffects = new List<SoundEffect>();
        // plays = new List<Play>();
        volume = 1.0;
        Microsoft::Xna::Framework::Audio::SoundEffect::setMasterVolume(1.0f);
    }

        void Sound::LoadContent()
        {
            if (Def::getHasSound())
            {
                for (int i = 0; i <= 92; i++)
                {
                    std::ostringstream oss;
                    oss << "sounds/sound" << std::setw(3) << std::setfill('0') << i;
                    std::string assetName = oss.str();

                    using Microsoft::Xna::Framework::Audio::SoundEffect;
                    SoundEffect item = game1.getContent().Load<SoundEffect>(assetName);
                    soundEffects.push_back(item);
                }
            }
        }

        bool Sound::Create() {
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
            this->volume = (double)volume / (double)MAXVOLUME;
        }

         int Sound::GetAudioVolume()
        {
            return (int)(volume * (double)MAXVOLUME);
        }

         void Sound::SetMidiVolume(int volume)
        {
        }

         int Sound::GetMidiVolume()
        {
            return 0;
        }

         void Sound::StopAll() {
             for (auto &play: plays) {
                 play.Stop();
             }
             plays.clear();
         }

         bool Sound::PlayImage(int channel, TinyPoint& pos, int rank, bool bLoop)
        {
            if (!gameData.getSounds())
            {
                return true;
            }
            if (channel >= 0 && channel < soundEffects.size())
            {

                if (channel != 10 && std::any_of(plays.begin(), plays.end(),
    [channel](const Play& p) { return p.getChannel() == channel && !p.getIsFree(); })) {
                    return true;
    }

                if (plays.size() >= 10) {
                    for (auto it = plays.begin(); it != plays.end();) {
                        if (it->getIsFree()) {
                            it = plays.erase(it);
                        } else {
                            it++;
                        }

                        if (plays.size() < 10) break;
                    }
                }


                plays.emplace_back(soundEffects[channel], channel, (float)GetVolume(pos), (float)GetBalance(pos), 0.0, bLoop);

            }
            return true;
        }

         bool Sound::PlayImage(int channel, TinyPoint &pos) {
             return PlayImage(channel, pos, -1, false);
         }

         bool Sound::PosImage(int channel, TinyPoint& pos)
        {
            return true;
        }

         bool Sound::Stop(int channel)
        {
            size_t num = 0;

            auto it = plays.begin();
            while (it != plays.end()) {
                if (it->getChannel() == channel) {
                    it->Stop();
                    it = plays.erase(it); // erase() returns the next valid iterator
                } else {
                    ++it;
                }
            }

            return true;
        }

        double Sound::GetVolume(TinyPoint& pos)
        {
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

         double Sound::GetBalance(TinyPoint& pos)
        {
            double val = (double)pos.X * 2.0 / 640.0 - 1.0;
            val = std::max(val, -1.0);
            return std::min(val, 1.0);
        }

}

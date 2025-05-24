//
// Created by robertvokac on 5/24/25.
//

#include "WindowsPhoneSpeedyBlupi/Sound.h"
namespace WindowsPhoneSpeedyBlupi {
    Sound::Play::Play(Microsoft::Xna::Framework::Audio::SoundEffect& se, int channel, double volume, double balance, double pitch, bool isLooped):

        Channel( [channel]() { return channel; }),
        IsFree( [*this]() { return sei.State == Microsoft::Xna::Framework::Audio::SoundState::Stopped; }),
        channel(channel),
        sei(se.CreateInstance())
    {
        int num = channel * 2;
        if (num >= 0 && num < tableVolumePitchLength)
        {
            volume *= tableVolumePitch[num];
            pitch = tableVolumePitch[num + 1];
        }

        sei.Volume = (float)volume;
        sei.Pan = (float)balance;
        sei.Pitch = (float)(pitch < 0.0 ? 0.0: pitch);
        sei.IsLooped = isLooped;
        sei.Play();
    }
    void Sound::Play::Stop()
    {
        sei.Stop();
    }

    Sound::Sound(Game1& game1, GameData& gameData):
    game1(game1),
    gameData(gameData)
    {

        // soundEffects = new List<SoundEffect>();
        // plays = new List<Play>();
        volume = 1.0;
        Microsoft::Xna::Framework::Audio::SoundEffect::MasterVolume = 1.0f;
    }


        void Sound::LoadContent()
        {
            if (Def::HasSound)
            {
                for (int i = 0; i <= 92; i++)
                {
                    std::ostringstream oss;
                    oss << "sounds/sound" << std::setw(3) << std::setfill('0') << i;
                    std::string assetName = oss.str();

                    Microsoft::Xna::Framework::Audio::SoundEffect item = game1.Content.get().Load<Microsoft::Xna::Framework::Audio::SoundEffect>(assetName);
                    soundEffects.push_back(item);
                }
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

         void Sound::StopAll()
        {
            while (!plays.empty())
            {
                plays[0].Stop();
                plays.erase(plays.begin());
            }
        }

         bool Sound::PlayImage(int channel, TinyPoint& pos)
        {
            return PlayImage(channel, pos, -1, false);
        }

         bool Sound::PlayImage(int channel, TinyPoint& pos, int rank, bool bLoop)
        {
            if (!gameData.Sounds)
            {
                return true;
            }
            if (channel >= 0 && channel < soundEffects.size())
            {

                if (channel != 10 && std::any_of(plays.begin(), plays.end(),
    [channel](const Play& p) { return p.Channel == channel && !p.IsFree; })) {
                    return true;
    }

                if (plays.size() >= 10) {
                    plays.erase(std::remove_if(plays.begin(), plays.end(),
                        [](const Play& p) { return p.IsFree; }), plays.end());
                }

                plays.emplace_back(soundEffects[channel], channel, (float)GetVolume(pos), (float)GetBalance(pos), 0.0, bLoop);

            }
            return true;
        }

         bool Sound::PosImage(int channel, TinyPoint& pos)
        {
            return true;
        }

         bool Sound::Stop(int channel)
        {
            size_t num = 0;
            while (num < plays.size())
            {
                if (plays[num].Channel == channel)
                {
                    plays[num].Stop();
                    plays.erase(plays.begin() + num);
                }
                else
                {
                    num++;
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

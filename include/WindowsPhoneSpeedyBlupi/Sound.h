//
// Created by robertvokac on 5/24/25.
//

#ifndef SOUND_H
#define SOUND_H
#include "Def.h"
#include "Game1.h"
#include "GameData.h"
#include "Microsoft/Xna/Framework/Audio/SoundEffect.h"
#include "Microsoft/Xna/Framework/Audio/SoundEffectInstance.h"
#include "Microsoft/Xna/Framework/Audio/SoundState.h"
#include "NeoSdk/Property.h"


namespace WindowsPhoneSpeedyBlupi {
// WindowsPhoneSpeedyBlupi, Version=1.0.0.5, Culture=neutral, PublicKeyToken=6db12cd62dbec439
// WindowsPhoneSpeedyBlupi.Sound
// using System;
// using System.Collections.Generic;
// using System.Linq;
// using Microsoft.Xna.Framework.Audio;
// using WindowsPhoneSpeedyBlupi;

    class Sound
    {
    private: class Play
        {
    private: Microsoft::Xna::Framework::Audio::SoundEffectInstance sei;
    private: const int channel;

    public:NeoSdk::Property<int> Channel;
    public:NeoSdk::Property<bool> IsFree;


    public: Play(Microsoft::Xna::Framework::Audio::SoundEffect& se, int channel, double volume, double balance, double* pitch, bool isLooped):

        Channel( [channel]() { return channel; }),
        IsFree( [*this]() { return sei.State == Microsoft::Xna::Framework::Audio::SoundState::Stopped; }),
        channel(channel),
        sei(se.CreateInstance())
            {
                int num = channel * 2;
                if (num >= 0 && num < tableVolumePitchLength)
                {
                    volume *= tableVolumePitch[num];
                    *pitch = tableVolumePitch[num + 1];
                }

                sei.Volume = (float)volume;
                sei.Pan = (float)balance;
                sei.Pitch = (float)(pitch == nullptr ? 0.0 : *pitch);
                sei.IsLooped = isLooped;
                sei.Play();
            }

    public :void Stop()
            {
                sei.Stop();
            }
        };

    private: static constexpr short tableVolumePitchLength = 200;
    private: static constexpr double tableVolumePitch[tableVolumePitchLength] =
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

    public: static constexpr int MAXVOLUME = 20;

    private: const Game1 game1;

    private: const GameData gameData;

    private: std::vector<Microsoft::Xna::Framework::Audio::SoundEffect> soundEffects;

    private: std::vector<Play> plays;

    private: double volume;

    public: Sound(Game1& game1, GameData& gameData):
        game1(game1),
        gameData(gameData)
        {

            // soundEffects = new List<SoundEffect>();
            // plays = new List<Play>();
            volume = 1.0;
            Microsoft::Xna::Framework::Audio::SoundEffect::MasterVolume = 1.0f;
        }

    public:
        void LoadContent()
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

     bool Create()
        {
            return true;
        }

         void SetState(bool bState)
        {
        }

         void SetCDAudio(bool bAudio)
        {
        }

         bool GetEnable()
        {
            return true;
        }

         void SetAudioVolume(int volume)
        {
            this.volume = (double)volume / (double)MAXVOLUME;
        }

         int GetAudioVolume()
        {
            return (int)(volume * (double)MAXVOLUME);
        }

         void SetMidiVolume(int volume)
        {
        }

         int GetMidiVolume()
        {
            return 0;
        }

         void StopAll()
        {
            while (plays.Any())
            {
                plays[0].Stop();
                plays.RemoveAt(0);
            }
        }

         bool PlayImage(int channel, TinyPoint pos)
        {
            return PlayImage(channel, pos, -1, false);
        }

         bool PlayImage(int channel, TinyPoint pos, int rank, bool bLoop)
        {
            if (!gameData.Sounds)
            {
                return true;
            }
            if (channel >= 0 && channel < soundEffects.Count)
            {
                if (channel != 10 && plays.Where((Play x) => x.Channel == channel && !x.IsFree).Any())
                {
                    return true;
                }
                if (plays.Count >= 10)
                {
                    int num = 0;
                    while (num < plays.Count)
                    {
                        if (plays[num].IsFree)
                        {
                            plays.RemoveAt(num);
                        }
                        else
                        {
                            num++;
                        }
                    }
                }
                Play item = new Play(soundEffects[channel], channel, (float)GetVolume(pos), (float)GetBalance(pos), null, bLoop);
                plays.Add(item);
            }
            return true;
        }

         bool PosImage(int channel, TinyPoint pos)
        {
            return true;
        }

         bool Stop(int channel)
        {
            int num = 0;
            while (num < plays.Count)
            {
                if (plays[num].Channel == channel)
                {
                    plays[num].Stop();
                    plays.RemoveAt(num);
                }
                else
                {
                    num++;
                }
            }
            return true;
        }

    private:
        double GetVolume(TinyPoint pos)
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

         double GetBalance(TinyPoint pos)
        {
            double val = (double)pos.X * 2.0 / 640.0 - 1.0;
            val = std::max(val, -1.0);
            return std::min(val, 1.0);
        }
    };
}

#endif //SOUND_H

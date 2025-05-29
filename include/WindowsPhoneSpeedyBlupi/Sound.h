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


    public: Play(Microsoft::Xna::Framework::Audio::SoundEffect& se, int channel, double volume, double balance, double pitch, bool isLooped);

    public: void Stop();
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

    public: Sound(Game1& game1, GameData& gameData);
        Sound(const Sound&);
        Sound& operator=(const Sound&);

    public:
        void LoadContent();

        bool Create();

        void SetState(bool bState);

        void SetCDAudio(bool bAudio);

        bool GetEnable();

        void SetAudioVolume(int volume);

        int GetAudioVolume();

        void SetMidiVolume(int volume);

        int GetMidiVolume();

        void StopAll();

        bool PlayImage(int channel, const TinyPoint& pos) const;

        bool PlayImage(int channel, const TinyPoint& pos, int rank, bool bLoop);

        bool PosImage(int channel, TinyPoint& pos);

        bool Stop(int channel);

    private:
        double GetVolume(TinyPoint& pos);
        double GetBalance(TinyPoint& pos);


    };
}

#endif //SOUND_H

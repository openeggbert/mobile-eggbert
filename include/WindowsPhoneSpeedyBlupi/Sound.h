//
// Created by robertvokac on 5/24/25.
//

#ifndef SOUND_H
#define SOUND_H
#include "Def.h"
#include "GameData.h"
#include "SoundI.h"
#include "Microsoft/Xna/Framework/Audio/SoundEffect.h"
#include "Microsoft/Xna/Framework/Audio/SoundEffectInstance.h"
#include "Microsoft/Xna/Framework/Audio/SoundState.h"
#include "CNA/Prop.h"
#include "Microsoft/Xna/Framework/Game.h"


namespace WindowsPhoneSpeedyBlupi {
// WindowsPhoneSpeedyBlupi, Version=1.0.0.5, Culture=neutral, PublicKeyToken=6db12cd62dbec439
// WindowsPhoneSpeedyBlupi.Sound
// using System;
// using System.Collections.Generic;
// using System.Linq;
// using Microsoft.Xna.Framework.Audio;
// using WindowsPhoneSpeedyBlupi;

    class Sound : public SoundI
    {
    private: class Play
        {
    private: Microsoft::Xna::Framework::Audio::SoundEffectInstance sei;
    private: const int channel;

        public: [[nodiscard]] int getChannel() const;
        public: [[nodiscard]] bool getIsFree() const;



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

    public:
        virtual ~Sound() = default;

        static constexpr int MAXVOLUME = 20;

    private: const Microsoft::Xna::Framework::Game game1;

    private: const GameData gameData;

    private: std::vector<Microsoft::Xna::Framework::Audio::SoundEffect> soundEffects;

    private: std::vector<Play> plays;

    private: double volume;

    public: Sound(Microsoft::Xna::Framework::Game& game1, GameData& gameData);
        Sound(const Sound&);
        Sound& operator=(const Sound&);

    public:
        void LoadContent() override;

        bool Create() override;

        void SetState(bool bState) override;

        void SetCDAudio(bool bAudio) override;

        bool GetEnable() override;

        void SetAudioVolume(int volume) override;

        int GetAudioVolume() override;

        void SetMidiVolume(int volume) override;

        int GetMidiVolume() override;

        void StopAll() override;

        bool PlayImage(int channel, TinyPoint& pos, int rank, bool bLoop) override;
        bool PlayImage(int channel, TinyPoint& pos);

        bool PosImage(int channel, TinyPoint& pos) override;

        bool Stop(int channel) override;

    private:
        double GetVolume(TinyPoint& pos);
        double GetBalance(TinyPoint& pos);

    };
}

#endif //SOUND_H

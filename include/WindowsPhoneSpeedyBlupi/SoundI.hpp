//
// Created by robertvokac on 5/24/25.
//

#ifndef SOUNDI_H
#define SOUNDI_H
#include "GameData.hpp"


namespace WindowsPhoneSpeedyBlupi {

    class SoundI {
    protected:
        ~SoundI() = default;
    public:
        virtual void LoadContent() = 0;

        virtual bool Create() = 0;

        virtual void SetState(bool bState) = 0;

        virtual void SetCDAudio(bool bAudio) = 0;

        virtual bool GetEnable() = 0;

        virtual void SetAudioVolume(int volume) = 0;

        virtual int GetAudioVolume() = 0;

        virtual void SetMidiVolume(int volume) = 0;

        virtual int GetMidiVolume() = 0;

        virtual void StopAll() = 0;

        virtual bool PlayImage(int channel, TinyPoint &pos, int rank, bool bLoop) = 0;

        virtual bool PlayImage(int channel, TinyPoint &pos) = 0;

        virtual bool PosImage(int channel, TinyPoint &pos) = 0;

        virtual bool Stop(int channel) = 0;
    };
}

#endif //SOUNDI_H

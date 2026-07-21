// SPDX-License-Identifier: MS-PL
#include "Microsoft/Xna/Framework/Audio/Microphone.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundEffect.hpp"

#ifdef SOUND_ENABLED
#include <SDL3/SDL.h>
#endif

namespace Microsoft::Xna::Framework::Audio
{
    std::vector<Microphone*>* Microphone::micList = nullptr;

    Microphone::~Microphone()
    {
#ifdef SOUND_ENABLED
        if (captureStream_ != nullptr)
        {
            SDL_DestroyAudioStream(captureStream_);
        }
#endif
    }

    SharpRuntime::intcs Microphone::getSampleRateProperty() const
    {
        return SAMPLERATE;
    }

    System::TimeSpan Microphone::GetSampleDuration(SharpRuntime::intcs sizeInBytes) const
    {
        return SoundEffect::GetSampleDuration(sizeInBytes, getSampleRateProperty(), AudioChannels::Mono);
    }

    void Microphone::CheckBuffer()
    {
        if (!BufferReady.Empty() && GetSampleDuration(GetQueuedBytes()) > bufferDuration_)
        {
            BufferReady.Raise(this, System::EventArgs::Empty);
        }
    }

    void Microphone::CheckAllBuffers()
    {
        if (micList != nullptr)
        {
            for (Microphone* microphone : *micList)
            {
                if (microphone != nullptr)
                {
                    microphone->CheckBuffer();
                }
            }
        }
    }

    SharpRuntime::intcs Microphone::GetQueuedBytes() const
    {
#ifdef SOUND_ENABLED
        if (captureStream_ != nullptr)
        {
            // FNA (SDL3_FNAPlatform.GetMicrophoneQueuedBytes) uses SDL_GetAudioStreamQueued, but
            // SDL3's own docs say to prefer SDL_GetAudioStreamAvailable for "how much can I read
            // right now" -- which is what CheckBuffer()/GetData() actually need here.
            const int available = SDL_GetAudioStreamAvailable(captureStream_);
            if (available > 0)
            {
                return static_cast<SharpRuntime::intcs>(available);
            }
        }
#endif

        return 0;
    }

    GetTypeNameCPP(Microphone, "Microsoft.Xna.Framework.Audio.Microphone")
}

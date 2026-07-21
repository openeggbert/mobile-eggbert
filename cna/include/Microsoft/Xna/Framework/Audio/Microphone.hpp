// SPDX-License-Identifier: MS-PL
#pragma once
#include "CNA/CNAHelper.hpp"

#include <string>
#include <vector>

#include "Microsoft/Xna/Framework/Audio/AudioChannels.hpp"
#include "Microsoft/Xna/Framework/Audio/MicrophoneState.hpp"
#include "System/EventArgs.hpp"
#include "System/EventHandler.hpp"
#include "System/Object.hpp"
#include "System/TimeSpan.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"

struct SDL_AudioStream;

namespace Microsoft::Xna::Framework::Audio
{
    /** @brief Represents a microphone capture device. */
    class Microphone : public System::Object
    {
    public:
        /** @brief Name of the microphone device. */
        const std::string Name;

        /** @brief Raised when enough captured data is available to be read. */
        System::EventHandler<System::EventArgs> BufferReady;

        /** @brief Destructor; closes this device's capture stream if it is still open. */
        ~Microphone() override;

        Microphone(const Microphone&) = delete;
        Microphone& operator=(const Microphone&) = delete;
        Microphone(Microphone&&) = delete;
        Microphone& operator=(Microphone&&) = delete;

        /**
         * @brief Gets the capture sample rate in Hz.
         *
         * @return Sample rate.
         */
        [[nodiscard]] SharpRuntime::intcs getSampleRateProperty() const;

        /**
         * @brief Converts a byte count to its playback duration for this microphone's format.
         *
         * @param sizeInBytes Number of PCM data bytes.
         * @return Corresponding duration.
         */
        [[nodiscard]] System::TimeSpan GetSampleDuration(SharpRuntime::intcs sizeInBytes) const;

        /** @brief Calls CheckBuffer() on every known microphone (used by FrameworkDispatcher::Update). */
        NOXNA static void CheckAllBuffers();

        GetTypeNameHPP()

    private:
        // MC-6: FNA has this as `internal void CheckBuffer()` -- it must not be a public C++ API
        // method (CLAUDE.md's Visibility Mapping; also T-1H's own accept criterion). The public,
        // sanctioned bridge for FrameworkDispatcher is the static CheckAllBuffers() above, which
        // already has private-member access to every instance's CheckBuffer() as a same-class
        // static method, so it doesn't need CheckBuffer() itself to be public.
        NOXNA void CheckBuffer();

        System::TimeSpan bufferDuration_;
        SharpRuntime::uintcs handle_;
        MicrophoneState state_;
        SDL_AudioStream* captureStream_ = nullptr;

        // FNA internals (Microphone.cs: micList, SAMPLERATE are both `internal`), not CNA additions.
        static std::vector<Microphone*>* micList;
        static constexpr SharpRuntime::intcs SAMPLERATE = 44100;

        [[nodiscard]] SharpRuntime::intcs GetQueuedBytes() const;
    };
}

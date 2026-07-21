// SPDX-License-Identifier: MS-PL
#include "Microsoft/Xna/Framework/Audio/SoundEffect.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundEffectInstance.hpp"

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstring>
#include <iostream>
#include <vector>

#include "Microsoft/Xna/Framework/Audio/NoAudioHardwareException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/NotSupportedException.hpp"

#ifdef SOUND_ENABLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include "CNA/Internal/Audio/AudioMixer.hpp"
#endif

namespace Microsoft::Xna::Framework::Audio
{
    class SoundEffect::Impl
    {
    public:
#ifdef SOUND_ENABLED
        std::shared_ptr<Mix_Chunk> audio;
        SharpRuntime::intcs sampleRate = 44100;
        SharpRuntime::uintcs channels  = 2;
#endif
        // Live SoundEffectInstance objects created via CreateInstance(), for Dispose()'s
        // cascade (T-3G). Raw, non-owning pointers: the instances' lifetime belongs to their
        // caller; SoundEffectInstance registers/unregisters itself (see SoundEffect::Register/
        // UnregisterInstance). Not gated by SOUND_ENABLED -- this is pure object-lifecycle
        // bookkeeping, independent of the audio backend.
        std::vector<SoundEffectInstance*> instances;
    };

    void SoundEffect::RegisterInstance(const std::shared_ptr<void>& keepAlive, SoundEffectInstance* instance)
    {
        if (!keepAlive || !instance) return;
        static_cast<Impl*>(keepAlive.get())->instances.push_back(instance);
    }

    void SoundEffect::UnregisterInstance(const std::shared_ptr<void>& keepAlive, SoundEffectInstance* instance)
    {
        if (!keepAlive || !instance) return;
        auto& v = static_cast<Impl*>(keepAlive.get())->instances;
        v.erase(std::remove(v.begin(), v.end(), instance), v.end());
    }

    // --- static members ---

    float SoundEffect::MasterVolume_   = 1.0f;

    // --- internal helpers ---

#ifdef SOUND_ENABLED
    namespace
    {
        // AUD-05-006: best-effort detection that the caller passed whole-file container bytes
        // (a common misuse: WAV/RIFF, Ogg, MP3/ID3, or XNB) to a raw PCM16LE constructor instead
        // of raw sample data. These constructors have no way to reject this outright -- a RIFF
        // header, read as PCM16 samples, is still "valid" 16-bit data, just garbage -- so this
        // only ever emits a diagnostic, never throws (see the constructor doc comment,
        // AUD-05-005: the correct fix for real misuse is `SoundEffect(const std::string&)`
        // instead, not a rejection here). Returns nullptr if no known signature is found.
        const char* DetectLikelyContainerSignature(const SharpRuntime::bytecs* data, std::size_t len)
        {
            if (len >= 4 && std::memcmp(data, "RIFF", 4) == 0) return "RIFF/WAVE";
            if (len >= 4 && std::memcmp(data, "OggS", 4) == 0) return "Ogg";
            if (len >= 3 && std::memcmp(data, "ID3", 3) == 0) return "MP3 (ID3 tag)";
            if (len >= 3 && data[0] == 'X' && data[1] == 'N' && data[2] == 'B') return "XNB";
            return nullptr;
        }

        // AUD-05-007: best-effort detection that raw PCM16 statistics look implausible for real
        // audio (e.g. float32 data byte-reinterpreted as PCM16, or Ogg/MP3-compressed data
        // without a recognizable header). Compressed/encoded bitstreams are specifically designed
        // to approach maximum byte-level entropy (~8 bits/byte, statistically close to random
        // noise); real quantized 16-bit audio essentially never does, even for loud/percussive
        // content -- adjacent samples/channels stay correlated, and typical byte-value
        // distributions are skewed rather than uniform. Advisory only, like
        // DetectLikelyContainerSignature above -- never throws, no release rejection, and the
        // 7.9-bit threshold (out of a theoretical max of 8.0) is deliberately conservative to
        // avoid flagging genuinely loud/noisy game audio.
        bool LooksImplausiblyHighEntropyForPcm16(const SharpRuntime::bytecs* data, std::size_t len)
        {
            if (len < 256) return false; // too short to estimate entropy meaningfully
            std::array<std::size_t, 256> histogram{};
            for (std::size_t i = 0; i < len; ++i) histogram[data[i]]++;
            double entropy = 0.0;
            for (std::size_t count : histogram)
            {
                if (count == 0) continue;
                double p = static_cast<double>(count) / static_cast<double>(len);
                entropy -= p * std::log2(p);
            }
            return entropy > 7.9;
        }

        // P9-HARDWARE-002: CNA::Internal::Audio::GetMixer() throws a raw std::runtime_error on
        // its first-ever call if no audio hardware/device is available -- the internal layer
        // stays exception-type-agnostic re: the XNA surface (matches the established
        // XactParser-throws-std/SoundBank-catches-and-converts pattern, CHECKLIST.md). This
        // converts that failure into NoAudioHardwareException at the XNA-facing entry points
        // that can be the very first GetMixer() call in the process, matching FNA's
        // SoundEffect.Device() throwing the identical exception from the identical failure.
        void* GetMixerOrThrowXna()
        {
            try
            {
                return CNA::Internal::Audio::GetMixer();
            }
            catch (const std::exception& ex)
            {
                throw NoAudioHardwareException(ex.what());
            }
        }

        // Wraps raw PCM16LE samples in a minimal in-memory RIFF/WAVE container so
        // Mix_LoadWAV_RW can decode+resample+convert them to the mixer's opened device format --
        // SDL2_mixer (unlike SDL3_mixer's MIX_LoadRawAudio) has no "load already-raw samples at
        // an arbitrary sample rate/channel count" entry point, only file-format loaders.
        std::vector<SharpRuntime::bytecs> WrapPcm16AsWav(
            const SharpRuntime::bytecs* data, std::size_t byteCount,
            SharpRuntime::intcs sampleRate, int channels)
        {
            const std::uint32_t dataSize = static_cast<std::uint32_t>(byteCount);
            const std::uint16_t blockAlign = static_cast<std::uint16_t>(channels * 2);
            const std::uint32_t byteRate = static_cast<std::uint32_t>(sampleRate) * blockAlign;
            const std::uint32_t riffSize = 36 + dataSize;

            std::vector<SharpRuntime::bytecs> wav;
            wav.reserve(44 + byteCount);
            auto put32 = [&wav](std::uint32_t v) {
                wav.push_back(static_cast<SharpRuntime::bytecs>(v & 0xFF));
                wav.push_back(static_cast<SharpRuntime::bytecs>((v >> 8) & 0xFF));
                wav.push_back(static_cast<SharpRuntime::bytecs>((v >> 16) & 0xFF));
                wav.push_back(static_cast<SharpRuntime::bytecs>((v >> 24) & 0xFF));
            };
            auto put16 = [&wav](std::uint16_t v) {
                wav.push_back(static_cast<SharpRuntime::bytecs>(v & 0xFF));
                wav.push_back(static_cast<SharpRuntime::bytecs>((v >> 8) & 0xFF));
            };

            wav.insert(wav.end(), {'R', 'I', 'F', 'F'});
            put32(riffSize);
            wav.insert(wav.end(), {'W', 'A', 'V', 'E', 'f', 'm', 't', ' '});
            put32(16);              // fmt chunk size
            put16(1);                // PCM
            put16(static_cast<std::uint16_t>(channels));
            put32(static_cast<std::uint32_t>(sampleRate));
            put32(byteRate);
            put16(blockAlign);
            put16(16);               // bits per sample
            wav.insert(wav.end(), {'d', 'a', 't', 'a'});
            put32(dataSize);
            wav.insert(wav.end(), data, data + byteCount);
            return wav;
        }
    }
#endif

    // --- public constructors ---

    SoundEffect::SoundEffect(const std::string& assetName)
        : impl_(std::make_shared<Impl>())
    {
        if (assetName.empty())
        {
            return;
        }

#ifdef SOUND_ENABLED
        GetMixerOrThrowXna();

        Mix_Chunk* raw = Mix_LoadWAV(assetName.c_str());
        if (!raw)
        {
            throw System::NotSupportedException(
                "Failed to load sound: " + assetName + " — " + Mix_GetError()
            );
        }

        impl_->audio = {raw, [](Mix_Chunk* p) { if (p) Mix_FreeChunk(p); }};

        // Mix_Chunk carries no format metadata (Mix_LoadWAV always converts to the mixer's
        // opened device format) -- the pre-migration SDL3_mixer implementation queried the
        // decoded format via MIX_GetAudioFormat; SDL2_mixer has no equivalent, so this reports
        // the fixed device format AudioMixer::GetMixer() opens with (S16 stereo 44100 Hz)
        // instead of the source file's own original sample rate/channel count.
        impl_->sampleRate = 44100;
        impl_->channels   = 2;
#else
        (void)assetName;
#endif
    }

    SoundEffect::SoundEffect(
        const std::vector<SharpRuntime::bytecs>& buffer,
        SharpRuntime::intcs sampleRate,
        AudioChannels channels)
        : SoundEffect(buffer, 0, static_cast<SharpRuntime::intcs>(buffer.size()),
                      sampleRate, channels, 0, 0)
    {
    }

    SoundEffect::SoundEffect(
        const std::vector<SharpRuntime::bytecs>& buffer,
        SharpRuntime::intcs offset,
        SharpRuntime::intcs count,
        SharpRuntime::intcs sampleRate,
        AudioChannels channels,
        SharpRuntime::intcs loopStart,
        SharpRuntime::intcs loopLength)
        : impl_(std::make_shared<Impl>())
    {
        // P9-VALIDATION-002: loopStart/loopLength are intentionally NOT validated here, matching
        // FNA's own internal ctor (SoundEffect.cs: `this.loopStart = (uint) loopStart;`) -- a
        // negative value wraps to a huge unsigned value in both FNA and here, identically.
        loopStart_  = static_cast<SharpRuntime::uintcs>(loopStart);
        loopLength_ = static_cast<SharpRuntime::uintcs>(loopLength);

        // P9-VALIDATION-003: offset+count must never be computed as a plain intcs addition --
        // two individually-plausible-looking values can overflow int32 (UB), and on a typical
        // two's-complement wraparound the overflowed sum can come out negative/small, silently
        // passing this check while `buffer.data() + offset` below is a wildly out-of-bounds
        // pointer. FNA gets away without this check because C#'s array bounds checking is the
        // real safety net there; C++ has none, so this has to be exact.
        if (offset < 0 || count < 0)
        {
            throw System::ArgumentOutOfRangeException("count");
        }
        const auto off = static_cast<std::size_t>(offset);
        const auto cnt = static_cast<std::size_t>(count);
        if (off > buffer.size() || cnt > buffer.size() - off)
        {
            throw System::ArgumentOutOfRangeException("count");
        }

#ifdef SOUND_ENABLED
        if (const char* sig = DetectLikelyContainerSignature(buffer.data() + off, cnt))
        {
            std::cerr << "[SoundEffect] Warning: raw PCM buffer starts with a " << sig
                      << " signature, not raw PCM16LE sample data -- passing whole-file bytes "
                      << "to this constructor decodes the container's header as audio samples, "
                      << "producing garbage output. Use SoundEffect(const std::string&) to load "
                      << "a file instead.\n";
        }
        else if (LooksImplausiblyHighEntropyForPcm16(buffer.data() + off, cnt))
        {
            std::cerr << "[SoundEffect] Warning: raw PCM buffer has implausibly high byte-level "
                      << "entropy for real 16-bit audio -- this often indicates compressed "
                      << "(Ogg/MP3, without a recognizable header) or otherwise non-PCM16 data "
                      << "(e.g. float32 samples byte-reinterpreted as PCM16) was passed to this "
                      << "constructor instead of raw PCM16LE samples.\n";
        }

        GetMixerOrThrowXna();

        const std::vector<SharpRuntime::bytecs> wav = WrapPcm16AsWav(
            buffer.data() + offset, static_cast<std::size_t>(count),
            sampleRate, static_cast<int>(channels));

        SDL_RWops* rw = SDL_RWFromConstMem(wav.data(), static_cast<int>(wav.size()));
        Mix_Chunk* raw = rw ? Mix_LoadWAV_RW(rw, 1) : nullptr;
        if (!raw)
        {
            throw System::NotSupportedException(
                std::string("Failed to create sound from buffer: ") + Mix_GetError()
            );
        }

        impl_->audio      = {raw, [](Mix_Chunk* p) { if (p) Mix_FreeChunk(p); }};
        impl_->sampleRate = sampleRate;
        impl_->channels   = static_cast<SharpRuntime::uintcs>(channels);
#else
        (void)offset;
        (void)count;
        (void)sampleRate;
        (void)channels;
#endif
    }

    SoundEffect::~SoundEffect() = default;

    // --- properties ---

    float SoundEffect::getMasterVolumeProperty()
    {
#ifdef SOUND_ENABLED
        // CP-16: query the real SDL2_mixer master gain (matches FNA, which likewise always
        // queries the live FAudio master voice rather than a cached value) so this reflects
        // Mix_MasterVolume's actual current value, not a value that could drift from it.
        GetMixerOrThrowXna();
        return CNA::Internal::Audio::GetMasterGain();
#else
        return MasterVolume_;
#endif
    }

    void SoundEffect::setMasterVolumeProperty(const float& v)
    {
#ifdef SOUND_ENABLED
        // CP-16: SDL2_mixer's master volume is a real global, applied to every channel (including
        // already-playing ones) at mix time -- unlike a per-track-baked-in approach, this needs
        // no per-instance re-application. FNA passes the value straight through, without
        // clamping; Mix_MasterVolume clamps to [0, MIX_MAX_VOLUME] internally instead of
        // rejecting an out-of-range value the way MIX_SetMixerGain used to.
        GetMixerOrThrowXna();
        CNA::Internal::Audio::SetMasterGain(v);
#else
        MasterVolume_ = v;
#endif
    }

    void SoundEffect::setMasterVolumeProperty(float&& v)
    {
        setMasterVolumeProperty(v);
    }

    // --- methods ---

    SoundEffectInstance SoundEffect::CreateInstance() const
    {
        return SoundEffectInstance(*this);
    }

    bool SoundEffect::Play()
    {
        return Play(1.0f, 0.0f, 0.0f);
    }

    bool SoundEffect::Play(float volume, float pitch, float pan)
    {
        if (isDisposed_)
        {
            return false;
        }

        // FNA constructs a real SoundEffectInstance and assigns Volume/Pitch/Pan through its
        // property setters before Play(); mirror their validation here: Pan is range-checked
        // (throws), Pitch is clamped rather than validated.
        if (pan > 1.0f || pan < -1.0f)
        {
            throw System::ArgumentOutOfRangeException("pan");
        }
        pitch = (pitch < -1.0f) ? -1.0f : ((pitch > 1.0f) ? 1.0f : pitch);

#ifdef SOUND_ENABLED
        auto* chunk = static_cast<Mix_Chunk*>(getNativeAudioHandle());
        if (!chunk)
        {
            return false;
        }

        CNA::Internal::Audio::GetMixer();
        CNA::Internal::Audio::Track* track = CNA::Internal::Audio::CreateTrack();
        // AudioMixer's OnChannelFinished frees this Track automatically the moment playback
        // ends -- the pre-migration SDL3_mixer implementation's own FireAndForgetPanState/
        // PendingPanStateCleanup machinery existed purely to solve that same problem for its own
        // per-track callback userdata; AudioMixer now owns that lifecycle generically for every
        // autoDestroy track, fire-and-forget or not.
        track->autoDestroy = true;

        CNA::Internal::Audio::SetTrackAudio(track, chunk);
        // CP-16: master volume is applied once, globally, via Mix_MasterVolume (the mixer's own
        // master gain stage) -- not baked into each track's own gain, which would double-apply it.
        CNA::Internal::Audio::SetTrackGain(track, volume);
        CNA::Internal::Audio::SetTrackPan(track, pan);

        if (pitch != 0.0f)
        {
            // P12-PITCH-001: matches FNA's real exponential octave curve (SoundEffectInstance.cs:
            // 589-591, `Math.Pow(2.0, INTERNAL_pitch)`) via SoundEffectInstance's shared, friended
            // conversion helper -- NOT a linear multiplier.
            const float ratio = SoundEffectInstance::INTERNAL_calculatePitchRatio(pitch);
            CNA::Internal::Audio::SetTrackPitch(track, ratio < 0.01f ? 0.01f : ratio);
        }

        if (!CNA::Internal::Audio::PlayTrack(track))
        {
            CNA::Internal::Audio::DestroyTrack(track);
            return false;
        }

        return true;
#else
        (void)volume; (void)pitch; (void)pan;
        return false;
#endif
    }

    void SoundEffect::Dispose()
    {
        if (!isDisposed_)
        {
            if (impl_)
            {
                // instance->Dispose() unregisters itself from impl_->instances, mutating the
                // live vector -- iterate a snapshot so that's safe (matches FNA's
                // Instances.ToArray() before the foreach in SoundEffect.Dispose()).
                auto instancesSnapshot = impl_->instances;
                for (auto* instance : instancesSnapshot)
                {
                    if (instance) instance->Dispose();
                }
            }
            impl_.reset();
            isDisposed_ = true;
        }
    }

    void* SoundEffect::getNativeAudioHandle() const
    {
#ifdef SOUND_ENABLED
        if (impl_ && impl_->audio)
        {
            return impl_->audio.get();
        }
#endif
        return nullptr;
    }

    // --- static methods ---

    System::TimeSpan SoundEffect::GetSampleDuration(
        SharpRuntime::intcs sizeInBytes,
        SharpRuntime::intcs sampleRate,
        AudioChannels channels)
    {
        const int ch = static_cast<int>(channels);
        if (ch <= 0 || sampleRate <= 0)
        {
            return System::TimeSpan::Zero;
        }
        // Matches FNA: truncate to whole milliseconds. 16-bit PCM => 2 bytes per sample.
        const int samples = sizeInBytes / 2;
        const int ms = static_cast<int>(
            (samples / ch) / (sampleRate / 1000.0f)
        );
        return System::TimeSpan::FromMilliseconds(ms);
    }

    SharpRuntime::intcs SoundEffect::GetSampleSizeInBytes(
        System::TimeSpan duration,
        SharpRuntime::intcs sampleRate,
        AudioChannels channels)
    {
        return static_cast<SharpRuntime::intcs>(
            duration.getTotalSecondsProperty() *
            sampleRate *
            static_cast<int>(channels) *
            2 // 16-bit PCM
        );
    }

    GetTypeNameCPP(SoundEffect, "Microsoft.Xna.Framework.Audio.SoundEffect")
}

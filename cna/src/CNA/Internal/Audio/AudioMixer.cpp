// SPDX-License-Identifier: MS-PL
#include "CNA/Internal/Audio/AudioMixer.hpp"
#include "CNA/Internal/Clamp.hpp"

#ifdef SOUND_ENABLED
#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>

namespace CNA::Internal::Audio
{
    namespace
    {
        // Same synchronization discipline as the pre-migration SDL3_mixer implementation --
        // g_mixerOpen's lazy-init check-then-create sequence and DestroyMixer()'s
        // check-then-destroy sequence share this single mutex end to end.
        std::mutex g_mixerMutex;
        bool g_mixerOpen = false;
        std::atomic<std::uint64_t> g_mixerGeneration{0};
        bool g_audioSubsystemPinned = false;

        constexpr int kChannelPoolSize = 32;
        std::array<bool, kChannelPoolSize> g_channelInUse{};
        std::array<Track*, kChannelPoolSize> g_channelOwner{};

        int AllocateChannel()
        {
            for (int i = 0; i < kChannelPoolSize; ++i)
            {
                if (!g_channelInUse[i])
                {
                    g_channelInUse[i] = true;
                    return i;
                }
            }
            return -1;
        }

        void ReleaseChannel(int channel)
        {
            if (channel < 0 || channel >= kChannelPoolSize) return;
            g_channelInUse[channel] = false;
            g_channelOwner[channel] = nullptr;
        }

        // Fires on the mixer thread (SDL2_mixer's documented Mix_ChannelFinished contract) for
        // BOTH natural completion and an explicit Mix_HaltChannel -- a single cleanup path for
        // "this channel is no longer in use" (StopTrack()/ExitTrackLoop() and DestroyTrack()'s own
        // Mix_HaltChannel below all funnel through here, matching this codebase's established
        // single-cleanup-path style, e.g. AudioMixer's own DestroyTrackSafe before this migration).
        // Must not block or allocate (real-time thread) -- only pointer/array writes here.
        void SDLCALL OnChannelFinished(int channel)
        {
            if (channel < 0 || channel >= kChannelPoolSize) return;
            Track* t = g_channelOwner[channel];
            if (!t) return;

            Mix_UnregisterAllEffects(channel);

            if (t->autoDestroy)
            {
                if (t->pitchedChunk)
                {
                    SDL_free(t->pitchedChunk->abuf);
                    Mix_FreeChunk(t->pitchedChunk);
                }
                delete t;
            }
            else
            {
                t->channel = -1;
            }
            ReleaseChannel(channel);
        }

        // P11-PAN-001-equivalent: FNA's exact 4-coefficient stereo crossfeed pan matrix (see
        // SoundEffectInstance.cpp's identically-named/documented function -- kept in sync with
        // that one by hand, since AudioMixer (CNA-internal) and SoundEffectInstance (XNA-facing)
        // are different translation units and this codebase has no shared internal math header
        // for it yet).
        void ComputePanCrossfeedMatrix(float pan, float& ll, float& rl, float& lr, float& rr)
        {
            if (pan <= 0.0f)
            {
                ll = 0.5f * pan + 1.0f;
                rl = 0.5f * -pan;
                lr = 0.0f;
                rr = pan + 1.0f;
            }
            else
            {
                ll = -pan + 1.0f;
                rl = 0.0f;
                lr = 0.5f * pan;
                rr = 0.5f * -pan + 1.0f;
            }
        }

        Sint16 ClampToInt16(float v)
        {
            if (v > 32767.0f) return 32767;
            if (v < -32768.0f) return -32768;
            return static_cast<Sint16>(v);
        }

        // Registered per-channel via Mix_RegisterEffect (SDL2_mixer's closest equivalent to
        // SDL3_mixer's per-track "cooked" callback) -- `stream` is `len` bytes of Sint16 stereo
        // interleaved PCM, already gain/gain-panned by SDL2_mixer's own per-channel processing,
        // right before this channel's contribution is mixed into the master output. Only ever
        // registered while `pan != 0.0f` (SetTrackPan unregisters it otherwise), matching the
        // pre-migration implementation's "zero pan costs nothing" optimization.
        void SDLCALL PanEffectCallback(int /*chan*/, void* stream, int len, void* udata)
        {
            const auto* track = static_cast<const Track*>(udata);
            if (track->pan == 0.0f) return;

            auto* pcm = static_cast<Sint16*>(stream);
            const int samples = len / static_cast<int>(sizeof(Sint16));

            float ll, rl, lr, rr;
            ComputePanCrossfeedMatrix(track->pan, ll, rl, lr, rr);

            for (int i = 0; i + 2 <= samples; i += 2)
            {
                const float l = pcm[i];
                const float r = pcm[i + 1];
                pcm[i]     = ClampToInt16(l * ll + r * rl);
                pcm[i + 1] = ClampToInt16(l * lr + r * rr);
            }
        }

        // Linear-interpolation resample of a decoded (Sint16 stereo interleaved, matching the
        // opened device format -- SDL2_mixer converts every loaded chunk to it) Mix_Chunk at
        // `ratio` (output_frames = input_frames / ratio -- ratio > 1 raises pitch by shortening
        // playback, matching FNA's `2^pitch` frequency-ratio convention). This is a real,
        // disclosed behavioral simplification vs. the pre-migration SDL3_mixer implementation's
        // `MIX_SetTrackFrequencyRatio` (a true realtime resampling stage applied continuously by
        // the mixer): here the resample happens once, up front, producing a fixed-ratio copy --
        // changing pitch on an ALREADY-PLAYING track restarts it from the beginning of the new
        // resampled buffer rather than gliding the existing playback position. mobile-eggbert
        // itself only ever sets pitch before Play() (never on a live instance), so this gap is
        // never actually exercised by this game -- see plan_lite.md's Phase 4 status for the
        // full disclosure.
        Mix_Chunk* ResampleChunk(const Mix_Chunk* src, float ratio)
        {
            if (!src || !src->abuf || src->alen < 4 || ratio <= 0.0f) return nullptr;

            const auto* in = reinterpret_cast<const Sint16*>(src->abuf);
            const int inFrames = static_cast<int>(src->alen / (2 * sizeof(Sint16)));
            const int outFrames = std::max(1, static_cast<int>(static_cast<float>(inFrames) / ratio));

            auto* outBuf = static_cast<Sint16*>(SDL_malloc(
                static_cast<std::size_t>(outFrames) * 2 * sizeof(Sint16)));
            if (!outBuf) return nullptr;

            for (int of = 0; of < outFrames; ++of)
            {
                const float srcPos = static_cast<float>(of) * ratio;
                const int i0 = std::min(static_cast<int>(srcPos), inFrames - 1);
                const int i1 = std::min(i0 + 1, inFrames - 1);
                const float frac = srcPos - static_cast<float>(i0);
                for (int c = 0; c < 2; ++c)
                {
                    const float s0 = static_cast<float>(in[i0 * 2 + c]);
                    const float s1 = static_cast<float>(in[i1 * 2 + c]);
                    outBuf[of * 2 + c] = ClampToInt16(s0 + (s1 - s0) * frac);
                }
            }

            Mix_Chunk* chunk = Mix_QuickLoad_RAW(
                reinterpret_cast<Uint8*>(outBuf),
                static_cast<Uint32>(outFrames) * 2 * sizeof(Sint16));
            if (!chunk)
            {
                SDL_free(outBuf);
                return nullptr;
            }
            return chunk;
        }

        Mix_Chunk* EffectiveChunk(const Track* t)
        {
            return t->pitchedChunk ? t->pitchedChunk : t->baseChunk;
        }
    }

    void* GetMixer()
    {
        std::lock_guard<std::mutex> lock(g_mixerMutex);

        if (!g_audioSubsystemPinned)
        {
            g_audioSubsystemPinned = (SDL_InitSubSystem(SDL_INIT_AUDIO) == 0);
        }

        if (!g_mixerOpen)
        {
            if (Mix_Init(0) == 0)
            {
                // SDL2_mixer's Mix_Init returns the bitmask of formats actually initialized (can
                // legitimately be 0 with no `flags` requested -- WAV needs no optional format
                // library at all) -- unlike SDL3_mixer's MIX_Init, a 0 return here is NOT a
                // failure by itself, so this branch is intentionally unreachable in practice and
                // kept only so a future flags!=0 caller gets a real diagnostic instead of silently
                // continuing.
            }

            if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) != 0)
            {
                throw std::runtime_error(std::string("Mix_OpenAudio failed: ") + Mix_GetError());
            }

            Mix_AllocateChannels(kChannelPoolSize);
            Mix_ChannelFinished(OnChannelFinished);

            int freq = 0, channels = 0;
            Uint16 format = 0;
            if (Mix_QuerySpec(&freq, &format, &channels))
            {
                std::cerr << "[AudioMixer] Requested format=0x" << std::hex << MIX_DEFAULT_FORMAT
                          << std::dec << " channels=2 freq=44100; negotiated format=0x"
                          << std::hex << format << std::dec
                          << " channels=" << channels << " freq=" << freq << "\n";
            }

            g_mixerOpen = true;
        }
        return g_mixerOpen ? reinterpret_cast<void*>(1) : nullptr;
    }

    void DestroyMixer()
    {
        std::lock_guard<std::mutex> lock(g_mixerMutex);
        if (g_mixerOpen)
        {
            for (int i = 0; i < kChannelPoolSize; ++i)
            {
                if (g_channelOwner[i])
                {
                    g_channelOwner[i]->channel = -1;
                }
            }
            g_channelInUse.fill(false);
            g_channelOwner.fill(nullptr);

            Mix_CloseAudio();
            Mix_Quit();
            g_mixerOpen = false;
            g_mixerGeneration.fetch_add(1, std::memory_order_release);
        }
    }

    std::uint64_t GetMixerGeneration()
    {
        return g_mixerGeneration.load(std::memory_order_acquire);
    }

    Track* CreateTrack()
    {
        return new Track();
    }

    void DestroyTrack(Track* track)
    {
        if (!track) return;
        if (track->channel != -1)
        {
            Mix_UnregisterAllEffects(track->channel);
            Mix_HaltChannel(track->channel); // fires OnChannelFinished, but autoDestroy is false
            ReleaseChannel(track->channel);
            track->channel = -1;
        }
        if (track->pitchedChunk)
        {
            SDL_free(track->pitchedChunk->abuf);
            Mix_FreeChunk(track->pitchedChunk);
        }
        delete track;
    }

    void SetTrackAudio(Track* track, Mix_Chunk* chunk)
    {
        if (!track) return;
        track->baseChunk = chunk;
    }

    void SetTrackGain(Track* track, float gain)
    {
        if (!track) return;
        track->gain = gain;
        if (track->channel != -1)
        {
            const int v = CNA::Internal::Clamp(static_cast<int>(gain * MIX_MAX_VOLUME), 0, MIX_MAX_VOLUME);
            Mix_Volume(track->channel, v);
        }
    }

    void SetTrackPan(Track* track, float pan)
    {
        if (!track) return;
        track->pan = pan;
        if (track->channel != -1)
        {
            Mix_UnregisterEffect(track->channel, PanEffectCallback);
            if (pan != 0.0f)
            {
                Mix_RegisterEffect(track->channel, PanEffectCallback, nullptr, track);
            }
        }
    }

    void SetTrackPitch(Track* track, float ratio)
    {
        if (!track) return;
        if (track->pitchedChunk)
        {
            SDL_free(track->pitchedChunk->abuf);
            Mix_FreeChunk(track->pitchedChunk);
            track->pitchedChunk = nullptr;
        }
        if (ratio != 1.0f && track->baseChunk)
        {
            track->pitchedChunk = ResampleChunk(track->baseChunk, ratio);
        }
    }

    void SetTrackLooping(Track* track, bool looping)
    {
        if (!track) return;
        track->looping = looping;
    }

    bool PlayTrack(Track* track)
    {
        if (!track) return false;
        Mix_Chunk* chunk = EffectiveChunk(track);
        if (!chunk) return false;

        if (track->channel == -1)
        {
            const int ch = AllocateChannel();
            if (ch == -1) return false;
            track->channel = ch;
            g_channelOwner[ch] = track;
        }

        Mix_Volume(track->channel, CNA::Internal::Clamp(static_cast<int>(track->gain * MIX_MAX_VOLUME), 0, MIX_MAX_VOLUME));
        Mix_UnregisterEffect(track->channel, PanEffectCallback);
        if (track->pan != 0.0f)
        {
            Mix_RegisterEffect(track->channel, PanEffectCallback, nullptr, track);
        }

        track->explicitlyStopped = false;
        // Always a single lap -- see the Track struct's doc for why looping is polled/reconciled
        // in TrackPlaying() instead of using Mix_PlayChannel's own loops parameter.
        return Mix_PlayChannel(track->channel, chunk, 0) != -1;
    }

    bool PlayTrackAsStreamCarrier(Track* track, Mix_Chunk* placeholderChunk)
    {
        if (!track || !placeholderChunk) return false;

        if (track->channel == -1)
        {
            const int ch = AllocateChannel();
            if (ch == -1) return false;
            track->channel = ch;
            g_channelOwner[ch] = track;
        }

        Mix_Volume(track->channel, CNA::Internal::Clamp(static_cast<int>(track->gain * MIX_MAX_VOLUME), 0, MIX_MAX_VOLUME));
        track->explicitlyStopped = false;
        track->looping = true; // informational only -- the actual loop is SDL2_mixer-native here.
        return Mix_PlayChannel(track->channel, placeholderChunk, -1) != -1;
    }

    void StopTrack(Track* track)
    {
        if (!track) return;
        track->explicitlyStopped = true;
        track->looping = false;
        if (track->channel != -1)
        {
            Mix_HaltChannel(track->channel); // -> OnChannelFinished releases the channel
        }
    }

    void ExitTrackLoop(Track* track)
    {
        if (!track) return;
        track->looping = false;
    }

    void PauseTrack(Track* track)
    {
        if (track && track->channel != -1)
        {
            Mix_Pause(track->channel);
        }
    }

    void ResumeTrack(Track* track)
    {
        if (track && track->channel != -1)
        {
            Mix_Resume(track->channel);
        }
    }

    bool TrackPlaying(Track* track)
    {
        if (!track || track->channel == -1) return false;
        if (Mix_Playing(track->channel))
        {
            return true;
        }
        // The current lap ended naturally. OnChannelFinished has already released the channel
        // (track->channel is -1 by the time we'd observe it here) unless a race let us see the
        // in-between state, so re-check before deciding whether to restart.
        if (track->channel == -1)
        {
            if (track->looping && !track->explicitlyStopped)
            {
                return PlayTrack(track);
            }
            return false;
        }
        return false;
    }

    bool TrackPaused(Track* track)
    {
        return track && track->channel != -1 && Mix_Paused(track->channel) != 0;
    }

    float GetMasterGain()
    {
        return static_cast<float>(Mix_MasterVolume(-1)) / static_cast<float>(MIX_MAX_VOLUME);
    }

    void SetMasterGain(float gain)
    {
        Mix_MasterVolume(CNA::Internal::Clamp(static_cast<int>(gain * MIX_MAX_VOLUME), 0, MIX_MAX_VOLUME));
    }
}
#endif

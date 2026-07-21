// SPDX-License-Identifier: MS-PL
#include "Microsoft/Xna/Framework/Audio/SoundEffectInstance.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundEffect.hpp"

#include <algorithm>
#include <cmath>
#include <exception>
#include <utility>

#include "System/ArgumentOutOfRangeException.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/ObjectDisposedException.hpp"

#ifdef SOUND_ENABLED
#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>
#include "CNA/Internal/Audio/AudioMixer.hpp"
#endif

namespace Microsoft::Xna::Framework::Audio
{
    // T-4C: per-track DSP state (see SoundEffectInstance.hpp's filterState_ for the ownership/
    // move-safety rationale). Kind/frequency/oneOverQ are written by INTERNAL_apply*Filter (main
    // thread) and read by FilterMixCallback (SDL3_mixer's mixing thread) -- guarded by
    // MIX_LockMixer/UnlockMixer in the writer, relying on SDL3_mixer's own documented guarantee
    // that "the SDL audio device thread [holds this same lock] while actual mixing is in
    // progress" (so the callback itself must NOT also lock -- it would be redundant at best).
    // yl/yb are the filter's per-channel recursive state and are touched ONLY by the mixing
    // thread inside the callback, never by the setters, so they need no synchronization at all.
    // P11-PAN-001 (RFC-1): also holds the crossfeed pan value, since SDL3_mixer only exposes one
    // "cooked" callback slot per track (CHECKLIST.md CP-19) -- this struct is that slot's entire
    // shared state, filter and pan alike, not just the filter anymore. `pan` is written under the
    // same MIX_LockMixer/UnlockMixer discipline as frequency/oneOverQ above.
    struct FilterState
    {
        enum class Kind { None, LowPass, HighPass, BandPass };
        Kind  kind        = Kind::None;
        float frequency   = 0.0f;
        float oneOverQ    = 1.0f;
        float yl[2]       = {0.0f, 0.0f};
        float yb[2]       = {0.0f, 0.0f};
        // P11-PAN-001: current stereo pan, range [-1,1], matching Pan_/Apply3D's own pan. 0.0f
        // (the FilterState default and the Pan property's own default) is the crossfeed matrix's
        // identity, so a never-panned track costs nothing extra in the callback.
        float pan         = 0.0f;
    };

#ifdef SOUND_ENABLED
    namespace
    {
        MIX_Track* AsTrack(void* p)
        {
            return static_cast<MIX_Track*>(p);
        }

        // P12-PITCH-001: the real implementation behind
        // SoundEffectInstance::INTERNAL_calculatePitchRatio (a thin forwarding shim, defined
        // further down) -- lives here, not as a class member body, purely so ApplyTrackProperties
        // below and SoundEffect::Play()'s fire-and-forget path (a friend of SoundEffectInstance,
        // SoundEffect.cpp) can both call it without needing a class-member-only path. Matches
        // FNA exactly (SoundEffectInstance.cs:589-591):
        // `FAudioSourceVoice_SetFrequencyRatio(handle, (float)Math.Pow(2.0, INTERNAL_pitch) *
        // doppler, 0)` -- Pitch's whole [-1,1] range is explicitly octave-based ("-1 octave to +1
        // octave"), an exponential curve, NOT a linear multiplier (a prior version of this file
        // used `(pitch<0)?(1+pitch*0.5f):(1+pitch)`, which only agrees with `2^pitch` at
        // pitch=-1,0,1 and is audibly wrong -- up to ~6%/1 semitone off -- everywhere else;
        // P12-AUDIT-001 found this via a fresh audit, since every pre-existing test only ever
        // exercised the default Pitch=0, where the two formulas coincidentally agree).
        float ComputePitchRatio(float pitch)
        {
            return std::pow(2.0f, pitch);
        }

        // CP-16: master volume is applied once, globally, via MIX_SetMixerGain (SDL3_mixer's own
        // master gain stage), not baked into each track's own gain here -- doing both would
        // double-apply it, and only the mixer-level gain re-applies live to already-playing
        // tracks without this function needing to be called again.
        // P9-3D-005: `doppler` is a multiplier applied on top of the pitch-derived ratio, matching
        // FNA's UpdatePitch() (`(2^INTERNAL_pitch) * doppler`, SoundEffectInstance.cs) -- defaults
        // to 1.0f (no-op) for every caller except Apply3D.
        // P11-PAN-001 (RFC-1): `filterState` receives the pan value instead of MIX_SetTrackStereo
        // computing per-channel gains directly -- SDL3_mixer's own stereo gain has no crossfeed
        // term (CHECKLIST.md CP-19), so it's fixed to unity here and CNA owns 100% of the stereo
        // image via the crossfeed matrix applied in the shared filter/pan cooked callback
        // (ApplyPanCrossfeed below). `filterState` may be null (SOUND_ENABLED-less builds aside,
        // this only happens if EnsureTrackDspState's allocation somehow failed) -- in that case
        // the pan write is simply skipped, matching this function's existing null-`track` guard.
        void ApplyTrackProperties(MIX_Track* track, FilterState* filterState,
                                   float volume, float pan, float pitch, float doppler = 1.0f)
        {
            if (!track) return;

            MIX_SetTrackGain(track, volume);

            static const MIX_StereoGains kUnityStereo{1.0f, 1.0f};
            MIX_SetTrackStereo(track, &kUnityStereo);

            if (filterState)
            {
                MIX_Mixer* mixer = CNA::Internal::Audio::GetMixer();
                MIX_LockMixer(mixer);
                filterState->pan = pan;
                MIX_UnlockMixer(mixer);
            }

            const float ratio = ComputePitchRatio(pitch) * doppler;
            MIX_SetTrackFrequencyRatio(track, ratio < 0.01f ? 0.01f : ratio);
        }

        // AUD-04-008/009: trackGeneration is the CNA::Internal::Audio::GetMixerGeneration() value
        // captured when trackPtr was created (SoundEffectInstance::trackMixerGeneration_) -- if it
        // no longer matches the current generation, AudioMixer::DestroyMixer() has already freed
        // this exact MIX_Track (and every other track its mixer owned), so touching it here via
        // MIX_StopTrack/MIX_DestroyTrack would itself be a use-after-free/double-free. trackPtr is
        // always cleared regardless: whether this call genuinely destroyed it or it was already
        // gone, the caller has no live track either way.
        void DestroyTrackSafe(void*& trackPtr, std::uint64_t trackGeneration)
        {
            MIX_Track* track = AsTrack(trackPtr);
            if (track && trackGeneration == CNA::Internal::Audio::GetMixerGeneration())
            {
                MIX_StopTrack(track, 0);
                MIX_DestroyTrack(track);
            }
            trackPtr = nullptr;
        }

        // P14-ORDER-002: CNA::Internal::Audio::GetMixer() throws a raw std::runtime_error on its
        // very first-ever call if no audio hardware/device is available (P9-HARDWARE-002). Every
        // INTERNAL_apply*Filter call site used to be reachable only after Play() had already
        // called GetMixer() successfully at least once (gated by an `if (!track_) return;` guard
        // this task removes), so a throw here was never actually observable. Now that filter
        // setup can run before Play() too (order-independent, matching how FACT establishes a
        // track's filter atomically alongside the voice itself), this may genuinely be the first
        // GetMixer() call in the process. These are all NOXNA-internal, no-op-if-not-ready methods
        // that never throw -- swallow the failure here rather than let a raw std::runtime_error
        // escape into Cue::Play(), which isn't a sanctioned raw-exception boundary the way
        // XactParser/SoundBank's constructor are.
        MIX_Mixer* TryGetMixer()
        {
            try
            {
                return CNA::Internal::Audio::GetMixer();
            }
            catch (const std::exception&)
            {
                return nullptr;
            }
        }

        // T-4C: FAudio's exact state-variable filter (Chamberlin SVF; see FAudio_internal.c's
        // FAudio_INTERNAL_FilterVoice). Pure math, independent of SDL3_mixer, so it can also be
        // driven directly and synchronously by SoundEffectInstanceTestAccess -- MIX_Track's real
        // callback only fires asynchronously from the mixing thread, which would make a test
        // either flaky or need a real-time wait.
        void ApplyFilter(FilterState& state, float* pcm, int channels, int samples)
        {
            const float f  = state.frequency;
            const float q1 = state.oneOverQ;

            for (int i = 0; i + channels <= samples; i += channels)
            {
                for (int c = 0; c < channels && c < 2; ++c)
                {
                    float& yl = state.yl[c];
                    float& yb = state.yb[c];
                    const float x = pcm[i + c];

                    yl = yl + f * yb;
                    const float yh = x - yl - q1 * yb;
                    yb = f * yh + yb;

                    switch (state.kind)
                    {
                        case FilterState::Kind::LowPass:  pcm[i + c] = yl; break;
                        case FilterState::Kind::HighPass: pcm[i + c] = yh; break;
                        case FilterState::Kind::BandPass: pcm[i + c] = yb; break;
                        default: break; // Not reachable -- caller already checked kind != None.
                    }
                }
            }
        }

        // P11-PAN-001 (RFC-1): the real implementation behind
        // SoundEffectInstance::INTERNAL_calculatePanCrossfeedMatrix (a thin forwarding shim,
        // defined further down) -- lives here, not as a class member body, purely so
        // ApplyPanCrossfeed below (an anonymous-namespace free function, called from the
        // real-time mixing callback) can call it without needing class-member access. Matches
        // FNA's SetPanMatrixCoefficients exactly (SoundEffectInstance.cs,
        // dspSettings.SrcChannelCount == 2 && DstChannelCount == 2 branch): hard panning does NOT
        // eliminate an entire channel -- the two source channels are blended together on
        // whichever output speaker `pan` favors, and the OTHER speaker goes silent, rather than
        // each speaker only ever hearing its own matching input channel (CHECKLIST.md CP-19, the
        // deviation this method fixes).
        void ComputePanCrossfeedMatrix(float pan, float& ll, float& rl, float& lr, float& rr)
        {
            if (pan <= 0.0f)
            {
                // Left speaker blends left/right channels; right speaker gets less of the right
                // channel (and none of the left).
                ll = 0.5f * pan + 1.0f;
                rl = 0.5f * -pan;
                lr = 0.0f;
                rr = pan + 1.0f;
            }
            else
            {
                // Left speaker gets less of the left channel (and none of the right); right
                // speaker blends right/left channels.
                ll = -pan + 1.0f;
                rl = 0.0f;
                lr = 0.5f * pan;
                rr = 0.5f * -pan + 1.0f;
            }
        }

        // Applies the crossfeed matrix above directly to interleaved stereo PCM. Only meaningful
        // for `channels == 2` -- SDL3_mixer forces every track to true stereo output before the
        // cooked callback runs (ApplyTrackProperties's unity MIX_SetTrackStereo call), so this is
        // always satisfied for a real callback invocation; guarded defensively anyway, matching
        // this file's existing style. `pan == 0.0f` (the common, never-panned case) skips the
        // transform entirely -- the matrix would reduce to the identity {1,0,0,1} anyway, so this
        // is a pure optimization, not a behavior branch.
        void ApplyPanCrossfeed(float pan, int channels, float* pcm, int samples)
        {
            if (channels != 2 || pan == 0.0f) return;

            float ll, rl, lr, rr;
            ComputePanCrossfeedMatrix(pan, ll, rl, lr, rr);

            for (int i = 0; i + 2 <= samples; i += 2)
            {
                const float l = pcm[i];
                const float r = pcm[i + 1];
                pcm[i]     = l * ll + r * rl;
                pcm[i + 1] = l * lr + r * rr;
            }
        }

        // Runs this track's entire shared cooked-callback DSP chain: the filter first (if any),
        // then the crossfeed pan matrix (P11-PAN-001, RFC-1) -- both are just float-PCM
        // transforms on the same buffer, run in sequence, matching the RFC-1 design sketch
        // (plan_audio.md P10-PAN-003). Unlike the old ProcessFilterState this replaces, this must
        // NOT bail out early when there's no filter -- pan crossfeed still needs to run for every
        // track, filtered or not.
        void ProcessFilterState(FilterState& state, float* pcm, int channels, int samples)
        {
            if (channels <= 0) return;

            if (state.kind != FilterState::Kind::None)
            {
                ApplyFilter(state, pcm, channels, samples);
            }

            ApplyPanCrossfeed(state.pan, channels, pcm, samples);
        }

        // SDL3_mixer trampoline: fires as a per-track "cooked" callback (after gain/pan/3D are
        // applied, right before this track's audio is mixed into the output -- the closest
        // SDL3_mixer equivalent to FAudio's per-voice filter). `userdata` is the instance's
        // FilterState*, kept alive by its own unique_ptr (see SoundEffectInstance.hpp)
        // independent of the SoundEffectInstance's own address, so this stays valid even if the
        // instance is later moved.
        void SDLCALL FilterMixCallback(void* userdata, MIX_Track* /*track*/,
                                        const SDL_AudioSpec* spec, float* pcm, int samples)
        {
            ProcessFilterState(*static_cast<FilterState*>(userdata), pcm, spec->channels, samples);
        }
    }
#endif

    SoundEffectInstance::SoundEffectInstance()
    {
    }

    SoundEffectInstance::SoundEffectInstance(const SoundEffect& soundEffect)
        // Capture the underlying audio resource (via SoundEffect's private impl_) and the
        // native handle now, while soundEffect is definitely alive -- Play() must never
        // dereference the SoundEffect itself, since a common chaining pattern like
        // SoundEffect(path).CreateInstance() destroys it immediately (CP-7).
        : soundEffectKeepAlive_(soundEffect.impl_)
        , nativeAudioHandle_(soundEffect.getNativeAudioHandle())
        , loopStart_(soundEffect.loopStart_)
        , loopLength_(soundEffect.loopLength_)
    {
        // Register for SoundEffect::Dispose()'s cascade (T-3G, matches FNA's
        // parentEffect.Instances.Add(selfReference) in SoundEffectInstance's ctor).
        SoundEffect::RegisterInstance(soundEffectKeepAlive_, this);
    }

    SoundEffectInstance::~SoundEffectInstance()
    {
        if (!isDisposed_)
        {
            Dispose();
        }
    }

    SoundEffectInstance::SoundEffectInstance(SoundEffectInstance&& other) noexcept
        : soundEffectKeepAlive_(std::move(other.soundEffectKeepAlive_))
        , nativeAudioHandle_(other.nativeAudioHandle_)
        , loopStart_(other.loopStart_)
        , loopLength_(other.loopLength_)
        , track_(other.track_)
        , trackMixerGeneration_(other.trackMixerGeneration_)
        , playing_(other.playing_)
        , hasStarted_(other.hasStarted_)
        , State_(other.State_)
        , IsLooped_(other.IsLooped_)
        , isDisposed_(other.isDisposed_)
        , Volume_(other.Volume_)
        , Pan_(other.Pan_)
        , Pitch_(other.Pitch_)
        , is3D_(other.is3D_)
        // AUDIO-001: persisted spatial state, same move-along-with-is3D_ rationale.
        , attenuation_(other.attenuation_)
        , dopplerFactor_(other.dopplerFactor_)
        , spatialPan_(other.spatialPan_)
        // filterState_ is heap-owned; moving the unique_ptr transfers ownership without moving
        // the FilterState object's address, so the callback registered on `track_` (also just
        // transferred, unchanged) stays valid with no re-registration needed (T-4C).
        , filterState_(std::move(other.filterState_))
    {
        // Re-point Dispose()-cascade tracking from &other to &this (T-3G) -- other's own address
        // must stop being cascade-targeted, since it no longer represents a live instance once
        // moved-from (only skip this for an already-disposed `other`, which was never tracked,
        // or has already unregistered itself).
        if (soundEffectKeepAlive_ && !isDisposed_)
        {
            SoundEffect::UnregisterInstance(soundEffectKeepAlive_, &other);
            SoundEffect::RegisterInstance(soundEffectKeepAlive_, this);
        }

        other.nativeAudioHandle_ = nullptr;
        other.track_       = nullptr;
        other.playing_     = false;
        other.hasStarted_  = false;
        other.State_       = SoundState::Stopped;
        other.isDisposed_  = true;
    }

    SoundEffectInstance& SoundEffectInstance::operator=(SoundEffectInstance&& other) noexcept
    {
        if (this != &other)
        {
#ifdef SOUND_ENABLED
            DestroyTrackSafe(track_, trackMixerGeneration_);
#endif
            // Unregister *this* from whatever SoundEffect it was previously tracked by --
            // once soundEffectKeepAlive_ is overwritten below, *this* address represents a
            // different (or no) instance, and the old SoundEffect must not cascade to it (T-3G).
            if (soundEffectKeepAlive_ && !isDisposed_)
                SoundEffect::UnregisterInstance(soundEffectKeepAlive_, this);

            soundEffectKeepAlive_ = std::move(other.soundEffectKeepAlive_);
            nativeAudioHandle_ = other.nativeAudioHandle_;
            loopStart_   = other.loopStart_;
            loopLength_  = other.loopLength_;
            track_       = other.track_;
            trackMixerGeneration_ = other.trackMixerGeneration_;
            playing_     = other.playing_;
            hasStarted_  = other.hasStarted_;
            State_       = other.State_;
            IsLooped_    = other.IsLooped_;
            isDisposed_  = other.isDisposed_;
            Volume_      = other.Volume_;
            Pan_         = other.Pan_;
            Pitch_       = other.Pitch_;
            is3D_        = other.is3D_;
            // AUDIO-001: persisted spatial state, same move-along-with-is3D_ rationale.
            attenuation_   = other.attenuation_;
            dopplerFactor_ = other.dopplerFactor_;
            spatialPan_    = other.spatialPan_;
            // See the move constructor's identical rationale for why no callback
            // re-registration is needed here (T-4C).
            filterState_ = std::move(other.filterState_);

            // Re-point tracking from &other to &this in the SoundEffect whose keepAlive we just
            // took over (see the move constructor's identical rationale).
            if (soundEffectKeepAlive_ && !isDisposed_)
            {
                SoundEffect::UnregisterInstance(soundEffectKeepAlive_, &other);
                SoundEffect::RegisterInstance(soundEffectKeepAlive_, this);
            }

            other.nativeAudioHandle_ = nullptr;
            other.track_       = nullptr;
            other.playing_     = false;
            other.hasStarted_  = false;
            other.State_       = SoundState::Stopped;
            other.isDisposed_  = true;
        }
        return *this;
    }

#ifdef SOUND_ENABLED
    void* SoundEffectInstance::GetLiveTrackHandle() const
    {
        if (!track_)
        {
            return nullptr;
        }
        if (trackMixerGeneration_ != CNA::Internal::Audio::GetMixerGeneration())
        {
            // AUD-04-008/009: the mixer that owned this track was destroyed (AudioMixer::
            // DestroyMixer(), which frees every MIX_Track it owns) since this track was
            // created -- track_ is a dangling pointer. Clear it (mutating via const_cast, same
            // pattern getStateProperty() already uses to lazily sync cached state from a const
            // getter) so every other accessor sees the same "no track" state from here on,
            // instead of dereferencing freed memory.
            const_cast<SoundEffectInstance*>(this)->track_ = nullptr;
            return nullptr;
        }
        return track_;
    }
#endif

    void SoundEffectInstance::Dispose()
    {
        if (!isDisposed_)
        {
#ifdef SOUND_ENABLED
            DestroyTrackSafe(track_, trackMixerGeneration_);
#endif
            if (soundEffectKeepAlive_)
            {
                SoundEffect::UnregisterInstance(soundEffectKeepAlive_, this);
                soundEffectKeepAlive_.reset();
            }
            playing_    = false;
            State_      = SoundState::Stopped;
            isDisposed_ = true;
        }
    }

    void SoundEffectInstance::Play()
    {
        if (isDisposed_)
        {
            throw System::ObjectDisposedException("SoundEffectInstance");
        }

        // Already playing: no-op, matching FNA exactly (a naive re-Play would otherwise restart
        // the track from the beginning instead of leaving ongoing playback untouched).
        if (getStateProperty() == SoundState::Playing)
        {
            return;
        }

        // Once started, IsLooped can no longer be changed (matches FNA's hasStarted gate).
        hasStarted_ = true;

#ifdef SOUND_ENABLED
        // If paused, resume instead of restarting.
        if (State_ == SoundState::Paused)
        {
            MIX_Track* track = AsTrack(GetLiveTrackHandle());
            if (track)
            {
                MIX_ResumeTrack(track);
                State_   = SoundState::Playing;
                playing_ = true;
                return;
            }
        }

        auto* audio = static_cast<MIX_Audio*>(nativeAudioHandle_);
        if (!audio)
        {
            State_   = SoundState::Stopped;
            playing_ = false;
            return;
        }

        MIX_Mixer* mixer = CNA::Internal::Audio::GetMixer();

        MIX_Track* track = AsTrack(GetLiveTrackHandle());
        if (!track)
        {
            track = MIX_CreateTrack(mixer);
            if (!track)
            {
                State_   = SoundState::Stopped;
                playing_ = false;
                return;
            }
            track_ = track;
            // AUD-04-008/009: captured at the exact moment this track is created, so
            // GetLiveTrackHandle() can detect a later AudioMixer::DestroyMixer() call that
            // frees it out from under this instance.
            trackMixerGeneration_ = CNA::Internal::Audio::GetMixerGeneration();
        }

        if (!MIX_SetTrackAudio(track, audio))
        {
            State_   = SoundState::Stopped;
            playing_ = false;
            return;
        }

        // AUDIO-001: was `ApplyTrackProperties(track, filterState_.get(), Volume_, Pan_, Pitch_)`
        // -- a plain re-Play() after Apply3D() must reapply the persisted spatial attenuation/
        // pan/Doppler too, not just Volume_/Pan_/Pitch_ (see INTERNAL_applyComposedTrackProperties).
        INTERNAL_applyComposedTrackProperties();

        SDL_PropertiesID props = SDL_CreateProperties();
        if (props == 0)
        {
            State_   = SoundState::Stopped;
            playing_ = false;
            return;
        }

        SDL_SetNumberProperty(props, MIX_PROP_PLAY_LOOPS_NUMBER, IsLooped_ ? -1 : 0);

        // CP-17: apply the authored loop region (matches FNA's LoopBegin/LoopLength, only
        // meaningful while IsLooped -- see SoundEffectInstance.cs's Play()). loopStart_==0 &&
        // loopLength_==0 (the common case: no explicit loop region was ever given) leaves both
        // properties at their SDL3_mixer defaults, which loop the entire track -- unchanged
        // behavior for every effect that never had a loop region authored.
        if (IsLooped_ && (loopStart_ != 0 || loopLength_ != 0))
        {
            SDL_SetNumberProperty(props, MIX_PROP_PLAY_LOOP_START_FRAME_NUMBER,
                                   static_cast<Sint64>(loopStart_));
            if (loopLength_ != 0)
            {
                // SDL3_mixer has no separate "loop end" property distinct from "track end" --
                // MAX_FRAME_NUMBER treats this position as EOF for the whole track. Combined with
                // LOOP_START_FRAME_NUMBER above, this matches FNA/XAudio2's LoopBegin/LoopLength
                // exactly: the intro plays once, then only [loopStart_, loopStart_+loopLength_)
                // repeats -- confirmed against real decoded audio via a raw SDL3_mixer callback
                // (P10-LOOP-003/004, SoundEffectInstanceTests.cpp's
                // BoundedLoopRegionPlaysIntroOnceThenRepeatsOnlyTheLoopRegion), correcting an
                // earlier, never-actually-decoded-audio-verified assumption that this truncated
                // the pre-loop intro too (see plan_audio.md's P10-LOOP-003/004 note).
                SDL_SetNumberProperty(props, MIX_PROP_PLAY_MAX_FRAME_NUMBER,
                                       static_cast<Sint64>(loopStart_) + static_cast<Sint64>(loopLength_));
            }
        }

        const bool ok = MIX_PlayTrack(track, props);
        SDL_DestroyProperties(props);

        if (!ok)
        {
            State_   = SoundState::Stopped;
            playing_ = false;
            return;
        }

        playing_ = true;
        State_   = SoundState::Playing;
#else
        State_   = SoundState::Stopped;
        playing_ = false;
#endif
    }

    void SoundEffectInstance::Stop()
    {
        Stop(true);
    }

    void SoundEffectInstance::Stop(bool immediate)
    {
#ifdef SOUND_ENABLED
        MIX_Track* track = AsTrack(GetLiveTrackHandle());
        if (track)
        {
            if (immediate)
            {
                MIX_StopTrack(track, 0);
            }
            else
            {
                // Exit loop so the track plays to the end and stops naturally.
                MIX_SetTrackLoops(track, 0);
            }
        }
#endif
        if (immediate)
        {
            playing_ = false;
            State_   = SoundState::Stopped;
        }
    }


    float SoundEffectInstance::INTERNAL_calculatePitchRatio(float pitch)
    {
        // Forwards to ComputePitchRatio (anonymous namespace, top of this file) -- the single
        // canonical implementation, shared with the real-time-callback-adjacent
        // ApplyTrackProperties() and (via friendship) SoundEffect::Play()'s fire-and-forget path,
        // so there is exactly one copy of this math to keep in sync with FNA (P12-PITCH-001).
        return ComputePitchRatio(pitch);
    }

    void SoundEffectInstance::INTERNAL_calculatePanCrossfeedMatrix(
        float pan, float& ll, float& rl, float& lr, float& rr)
    {
        // Forwards to ComputePanCrossfeedMatrix (anonymous namespace, top of this file) -- the
        // single canonical implementation, shared with the real-time mixing callback
        // (ApplyPanCrossfeed) so there is exactly one copy of this math to keep in sync with FNA.
        ComputePanCrossfeedMatrix(pan, ll, rl, lr, rr);
    }

    void SoundEffectInstance::EnsureTrackDspState()
    {
#ifdef SOUND_ENABLED
        MIX_Track* track = AsTrack(GetLiveTrackHandle());
        if (!track) return;
        if (!filterState_) filterState_ = std::make_unique<FilterState>();
        MIX_SetTrackCookedCallback(track, FilterMixCallback, filterState_.get());
#endif
    }

    void SoundEffectInstance::INTERNAL_applyComposedTrackProperties()
    {
#ifdef SOUND_ENABLED
        MIX_Track* track = AsTrack(GetLiveTrackHandle());
        if (!track) return;

        EnsureTrackDspState(); // must exist before ApplyTrackProperties writes pan
        const float pan = is3D_ ? spatialPan_ : Pan_;
        ApplyTrackProperties(track, filterState_.get(), Volume_ * attenuation_, pan, Pitch_, dopplerFactor_);
#endif
    }

    bool SoundEffectInstance::getIsDisposedProperty() const
    {
        return isDisposed_;
    }

    void SoundEffectInstance::setVolumeProperty(const float& volume)
    {
        Volume_ = volume; // FNA passes the value straight through, without clamping

        // AUDIO-001: was a direct `MIX_SetTrackGain(track, Volume_)`, which erased any spatial
        // attenuation Apply3D had established (FNA's Volume setter never touches the separate
        // output-matrix voice stage that attenuation lives in; SDL3_mixer has only one gain
        // scalar, so CNA must recompose the two explicitly on every write instead).
        INTERNAL_applyComposedTrackProperties();
    }

    void SoundEffectInstance::setVolumeProperty(float&& volume)
    {
        setVolumeProperty(volume);
    }

    void SoundEffectInstance::setPanProperty(const float& pan)
    {
        if (isDisposed_)
        {
            throw System::ObjectDisposedException("SoundEffectInstance");
        }
        if (pan > 1.0f || pan < -1.0f)
        {
            throw System::ArgumentOutOfRangeException("value");
        }
        Pan_ = pan;

        // CP-20: once Apply3D has run at least once, its own pan approximation is what should
        // keep governing the real track output -- matches FNA's `if (is3D) return;` in Pan's
        // setter (SoundEffectInstance.cs). The property itself still always reports what was
        // last set, above.
        if (is3D_)
        {
            return;
        }

        // AUDIO-001: routed through the same shared composition routine Play()/Apply3D()/the
        // Volume/Pitch setters use (was a standalone `filterState_->pan = Pan_` write) -- since
        // is3D_ is false on this path, INTERNAL_applyComposedTrackProperties() picks Pan_ for the
        // live pan exactly as this used to write directly, just via one canonical call site.
        INTERNAL_applyComposedTrackProperties();
    }

    void SoundEffectInstance::setPanProperty(float&& pan)
    {
        setPanProperty(pan);
    }

    void SoundEffectInstance::setPitchProperty(const float& pitch)
    {
        Pitch_ = (pitch < -1.0f) ? -1.0f : ((pitch > 1.0f) ? 1.0f : pitch);

        // AUDIO-001: was a direct ratio write with no Doppler term, which erased any Doppler
        // shift Apply3D had established (FNA's UpdatePitch() always recombines pitch with the
        // last-computed Doppler factor, matching FAudio's single combined frequency-ratio voice
        // stage -- SDL3_mixer's ratio call is the same single combined stage, so CNA must
        // recompose the two explicitly on every write instead).
        INTERNAL_applyComposedTrackProperties();
    }

    void SoundEffectInstance::setPitchProperty(float&& pitch)
    {
        setPitchProperty(pitch);
    }

    bool SoundEffectInstance::getIsLoopedProperty() const
    {
        return IsLooped_;
    }

    void SoundEffectInstance::setIsLoopedProperty(const bool& looped)
    {
        if (hasStarted_)
        {
            throw System::InvalidOperationException();
        }
        IsLooped_ = looped;
    }

    void SoundEffectInstance::setIsLoopedProperty(bool&& looped)
    {
        setIsLoopedProperty(looped);
    }

    SoundState SoundEffectInstance::getStateProperty() const
    {
#ifdef SOUND_ENABLED
        MIX_Track* track = AsTrack(GetLiveTrackHandle());
        if (!track)
        {
            return SoundState::Stopped;
        }
        if (MIX_TrackPaused(track))
        {
            return SoundState::Paused;
        }
        if (MIX_TrackPlaying(track))
        {
            return SoundState::Playing;
        }
        // Track finished playing; sync internal state.
        const_cast<SoundEffectInstance*>(this)->playing_ = false;
        const_cast<SoundEffectInstance*>(this)->State_   = SoundState::Stopped;
        return SoundState::Stopped;
#else
        return State_;
#endif
    }

    GetTypeNameCPP(SoundEffectInstance, "Microsoft.Xna.Framework.Audio.SoundEffectInstance")
}

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
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include "CNA/Internal/Audio/AudioMixer.hpp"
#endif

namespace Microsoft::Xna::Framework::Audio
{
    // T-4C: per-instance filter state (see SoundEffectInstance.hpp's filterState_ for the
    // ownership/move-safety rationale). No public API currently sets `kind` away from `None` --
    // the LowPass/HighPass/BandPass filter setters this state was originally designed for were
    // already unreachable-and-pruned by Phase 2.6 (plan_lite.md), leaving this struct itself (and
    // the always-taken `kind == None` path in EnsureTrackDspState) as-is, since it's still a
    // legitimate shape for that surface to return in if it's ever restored. Pan is no longer
    // tracked here post-Phase-4 (SDL2 migration): AudioMixer::Track owns pan directly now (see
    // AudioMixer.cpp's PanEffectCallback), since SDL2_mixer's Mix_RegisterEffect is a genuine
    // per-channel slot (unlike SDL3_mixer's one-cooked-callback-per-track constraint that made
    // sharing this same struct's slot with pan necessary in the first place).
    struct FilterState
    {
        enum class Kind { None, LowPass, HighPass, BandPass };
        Kind  kind        = Kind::None;
        float frequency   = 0.0f;
        float oneOverQ    = 1.0f;
        float yl[2]       = {0.0f, 0.0f};
        float yb[2]       = {0.0f, 0.0f};
    };

#ifdef SOUND_ENABLED
    namespace
    {
        CNA::Internal::Audio::Track* AsTrack(void* p)
        {
            return static_cast<CNA::Internal::Audio::Track*>(p);
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

        // P11-PAN-001-equivalent: FNA's exact 4-coefficient stereo crossfeed pan matrix,
        // duplicated by hand from AudioMixer.cpp's identically-named/documented function (the
        // two translation units have no shared internal math header for it) so
        // INTERNAL_calculatePanCrossfeedMatrix below keeps working as a standalone, independently
        // unit-testable pure function even though AudioMixer::SetTrackPan now owns the real,
        // live-track application of this same math.
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

        // CP-16: master volume is applied once, globally, via Mix_MasterVolume (SDL2_mixer's own
        // master gain stage), not baked into each track's own gain here -- doing both would
        // double-apply it, and only the master-level gain re-applies live to already-playing
        // tracks without this function needing to be called again.
        // P9-3D-005: `doppler` is a multiplier applied on top of the pitch-derived ratio, matching
        // FNA's UpdatePitch() (`(2^INTERNAL_pitch) * doppler`, SoundEffectInstance.cs) -- defaults
        // to 1.0f (no-op) for every caller except Apply3D.
        // P11-PAN-001-equivalent: `pan` is applied via AudioMixer::SetTrackPan, which owns the
        // crossfeed-matrix pan effect internally now (see AudioMixer.cpp's PanEffectCallback) --
        // this file no longer needs its own per-instance cooked-callback registration for it.
        void ApplyTrackProperties(CNA::Internal::Audio::Track* track,
                                   float volume, float pan, float pitch, float doppler = 1.0f)
        {
            if (!track) return;

            CNA::Internal::Audio::SetTrackGain(track, volume);
            CNA::Internal::Audio::SetTrackPan(track, pan);

            const float ratio = ComputePitchRatio(pitch) * doppler;
            CNA::Internal::Audio::SetTrackPitch(track, ratio < 0.01f ? 0.01f : ratio);
        }

        // AUD-04-008/009: trackGeneration is the CNA::Internal::Audio::GetMixerGeneration() value
        // captured when trackPtr was created (SoundEffectInstance::trackMixerGeneration_) -- if it
        // no longer matches the current generation, AudioMixer::DestroyMixer() has already
        // detached this exact Track's channel (and every other track its mixer owned), so
        // touching it here via StopTrack/DestroyTrack would operate on stale channel state.
        // trackPtr is always cleared regardless: whether this call genuinely destroyed it or it
        // was already gone, the caller has no live track either way.
        void DestroyTrackSafe(void*& trackPtr, std::uint64_t trackGeneration)
        {
            CNA::Internal::Audio::Track* track = AsTrack(trackPtr);
            if (track)
            {
                if (trackGeneration == CNA::Internal::Audio::GetMixerGeneration())
                {
                    CNA::Internal::Audio::StopTrack(track);
                }
                CNA::Internal::Audio::DestroyTrack(track);
            }
            trackPtr = nullptr;
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
            CNA::Internal::Audio::Track* track = AsTrack(GetLiveTrackHandle());
            if (track)
            {
                CNA::Internal::Audio::ResumeTrack(track);
                State_   = SoundState::Playing;
                playing_ = true;
                return;
            }
        }

        auto* chunk = static_cast<Mix_Chunk*>(nativeAudioHandle_);
        if (!chunk)
        {
            State_   = SoundState::Stopped;
            playing_ = false;
            return;
        }

        CNA::Internal::Audio::GetMixer();

        CNA::Internal::Audio::Track* track = AsTrack(GetLiveTrackHandle());
        if (!track)
        {
            track = CNA::Internal::Audio::CreateTrack();
            track_ = track;
            // AUD-04-008/009: captured at the exact moment this track is created, so
            // GetLiveTrackHandle() can detect a later AudioMixer::DestroyMixer() call that
            // detaches it out from under this instance.
            trackMixerGeneration_ = CNA::Internal::Audio::GetMixerGeneration();
        }

        CNA::Internal::Audio::SetTrackAudio(track, chunk);

        // CP-17: the authored loop region (FNA's LoopBegin/LoopLength) is NOT honored post-Phase-4
        // (SDL2 migration) -- classic SDL2_mixer's Mix_Chunk has no notion of a loop point
        // distinct from the whole chunk's own start/end, unlike SDL3_mixer's
        // MIX_PROP_PLAY_LOOP_START_FRAME_NUMBER/MAX_FRAME_NUMBER properties. `IsLooped_` still
        // loops the ENTIRE chunk (see AudioMixer::Track's own looping doc) -- only a bounded
        // sub-region loop is unsupported. mobile-eggbert itself never authors a loop region
        // (every SoundEffect it loads comes from a whole .wav file via the plain string
        // constructor), so this gap is never actually exercised by this game -- see
        // plan_lite.md's Phase 4 status for the full disclosure.
        CNA::Internal::Audio::SetTrackLooping(track, IsLooped_);

        // AUDIO-001: was `ApplyTrackProperties(track, filterState_.get(), Volume_, Pan_, Pitch_)`
        // -- a plain re-Play() after Apply3D() must reapply the persisted spatial attenuation/
        // pan/Doppler too, not just Volume_/Pan_/Pitch_ (see INTERNAL_applyComposedTrackProperties).
        INTERNAL_applyComposedTrackProperties();

        if (!CNA::Internal::Audio::PlayTrack(track))
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
        CNA::Internal::Audio::Track* track = AsTrack(GetLiveTrackHandle());
        if (track)
        {
            if (immediate)
            {
                CNA::Internal::Audio::StopTrack(track);
            }
            else
            {
                // Exit loop so the track plays to the end and stops naturally (see
                // AudioMixer::Track's own doc for how the lazy loop-restart honors this).
                CNA::Internal::Audio::ExitTrackLoop(track);
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
        // Post-Phase-4 (SDL2 migration): pan is applied directly by AudioMixer::SetTrackPan (see
        // ApplyTrackProperties above), which needs no per-instance callback registration --
        // Mix_RegisterEffect is (re)registered by AudioMixer itself on every SetTrackPan/PlayTrack
        // call. This function is kept (rather than removed outright) purely so filterState_
        // keeps getting lazily allocated here, matching its existing lifecycle in case the
        // LowPass/HighPass/BandPass filter setters this struct was designed for are ever restored
        // (see FilterState's own updated doc).
        CNA::Internal::Audio::Track* track = AsTrack(GetLiveTrackHandle());
        if (!track) return;
        if (!filterState_) filterState_ = std::make_unique<FilterState>();
#endif
    }

    void SoundEffectInstance::INTERNAL_applyComposedTrackProperties()
    {
#ifdef SOUND_ENABLED
        CNA::Internal::Audio::Track* track = AsTrack(GetLiveTrackHandle());
        if (!track) return;

        EnsureTrackDspState();
        const float pan = is3D_ ? spatialPan_ : Pan_;
        ApplyTrackProperties(track, Volume_ * attenuation_, pan, Pitch_, dopplerFactor_);
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
        CNA::Internal::Audio::Track* track = AsTrack(GetLiveTrackHandle());
        if (!track)
        {
            return SoundState::Stopped;
        }
        if (CNA::Internal::Audio::TrackPaused(track))
        {
            return SoundState::Paused;
        }
        // TrackPlaying() also reconciles AudioMixer::Track's lazy loop-restart as a side effect
        // (see its own doc) -- this is the call site that makes IsLooped-forever playback keep
        // going across repeated getStateProperty() polls, matching this codebase's existing
        // poll-and-reconcile style (DynamicSoundEffectInstance::Update()).
        if (CNA::Internal::Audio::TrackPlaying(track))
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

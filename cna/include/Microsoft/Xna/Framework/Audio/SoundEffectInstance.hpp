// SPDX-License-Identifier: MS-PL
#pragma once

#include "CNA/CNAHelper.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundState.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "System/IDisposable.hpp"
#include "System/Object.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"

#include <memory>

namespace Microsoft::Xna::Framework::Audio
{
    class SoundEffect;
    class Cue;

    // T-4C: DSP filter state (kind/coefficients/recursive state), fully defined only in
    // SoundEffectInstance.cpp where SDL3_mixer's callback-registration types are available.
    // Namespace-scope (not a nested class of SoundEffectInstance) so the free-function
    // SDL3_mixer callback that reads it doesn't need friend access -- it's still effectively
    // private, since nothing outside SoundEffectInstance.cpp ever names it.
    struct FilterState;

    /** @brief Controls playback of a sound effect instance, including volume, pitch, pan, and looping. */
    class SoundEffectInstance : public System::Object, public System::IDisposable
    {
        friend class SoundEffect;
        NOXNA friend class Cue;

    protected:
        /** @brief Default constructor for use by DynamicSoundEffectInstance. */
        SoundEffectInstance();

        // These members are protected so DynamicSoundEffectInstance can manage its own state.
        void* track_        = nullptr;
        bool  playing_      = false;
        bool  hasStarted_   = false; // true once Play() has been called; never reset (gates IsLooped)
        SoundState State_   = SoundState::Stopped;

        // AUD-04-008/009: the CNA::Internal::Audio::GetMixerGeneration() value captured at the
        // moment track_ was created (Play()). Compared against the current generation by
        // GetLiveTrackHandle() below, so a track_ orphaned by AudioMixer::DestroyMixer() (which
        // frees every MIX_Track the destroyed mixer owned) is detected instead of dereferenced.
        // 0 is a safe "never captured" sentinel: it only matters once track_ is non-null, and
        // track_ is only ever set alongside this field.
        std::uint64_t trackMixerGeneration_ = 0;

        // AUD-04-008/009: returns track_ as a live MIX_Track* (type-erased to void*, same as
        // track_ itself), or nullptr -- clearing track_ as a side effect -- if the mixer that
        // owned it has been destroyed since this track was created. Every accessor that needs a
        // live track handle (in this class and DynamicSoundEffectInstance, which shares track_)
        // must go through this instead of reading track_ directly, since AudioMixer::
        // DestroyMixer() frees every MIX_Track the mixer owned with no way to notify a live
        // SoundEffectInstance directly. const so getStateProperty() (a const query) can call it;
        // mutates via const_cast, matching this class's existing convention for lazily
        // synchronizing cached state from a const getter (see getStateProperty()'s own
        // const_cast on playing_/State_).
        [[nodiscard]] void* GetLiveTrackHandle() const;

        // P13-DYNAMIC-001: recomposes and writes this instance's full set of live track properties
        // (gain, pan, frequency ratio) from Volume_/Pan_/Pitch_ together with the persisted
        // spatial state (attenuation_/dopplerFactor_/spatialPan_, private below) -- the single call
        // site Play(), setVolumeProperty(), setPitchProperty(), setPanProperty(), and Apply3D() all
        // share (AUDIO-001), so a spatial attenuation/pan/Doppler value set by Apply3D survives
        // every one of those calls instead of being overwritten by whichever runs next. Pan uses
        // spatialPan_ once is3D_ has latched, otherwise the plain Pan_ property, matching CP-20.
        // No-op if there is no live track yet (matches every INTERNAL_apply*'s existing null-track
        // guard). Protected (not private) so DynamicSoundEffectInstance::Play() can call it too,
        // now that both classes share the same `track_` (P13-DYNAMIC-001 -- previously
        // DynamicSoundEffectInstance managed its own separate `dynamicTrack_` and never applied any
        // of this).
        void INTERNAL_applyComposedTrackProperties();

    private:
        // Keeps the sound effect's underlying audio resource alive for the lifetime of this
        // instance, independent of whether the originating SoundEffect object itself still
        // exists (e.g. after `SoundEffect(path).CreateInstance()` on a temporary) -- CP-7.
        // Type-erased because SoundEffect::Impl is private and defined only in SoundEffect.cpp.
        std::shared_ptr<void> soundEffectKeepAlive_;
        void* nativeAudioHandle_ = nullptr; // MIX_Audio*, cached while soundEffect was alive

        // Cached from the originating SoundEffect at construction time, same rationale as
        // nativeAudioHandle_ above -- Play() must never dereference the SoundEffect itself
        // (CP-7), so its loop region is copied out while it's definitely still alive rather
        // than read through a stored reference/pointer (CP-17).
        SharpRuntime::uintcs loopStart_  = 0;
        SharpRuntime::uintcs loopLength_ = 0;

        bool  IsLooped_     = false;
        bool  isDisposed_   = false;
        float Volume_       = 1.0f;
        float Pan_          = 0.0f;
        float Pitch_        = 0.0f;

        // CP-20: once Apply3D has been called, matches FNA's `is3D` latch (SoundEffectInstance.cs)
        // -- setPanProperty() still updates the Pan_ property (callers must keep reading back
        // what they last set), but stops writing the real track output, since Apply3D's own pan
        // approximation is what should keep governing the actual output until Apply3D runs
        // again. Never reset back to false once set (matches FNA: is3D is only ever set to true).
        bool  is3D_         = false;

        // AUDIO-001: Apply3D's derived attenuation/pan/Doppler must survive later Play()/Volume/
        // Pitch calls, and a call made before this instance has ever had a track (no track yet to
        // write to) -- matches FNA's own persistent per-instance dspSettings/is3D state
        // (SoundEffectInstance.cs), which Play() and UpdatePitch() both read back from on every
        // subsequent call, not just the one Apply3D() itself made. Defaults are the neutral no-op
        // values, so an instance that never calls Apply3D computes byte-for-byte the same gain/
        // pan/frequency-ratio as before this fix (Volume_ * 1.0f == Volume_, etc). Never reset
        // once Apply3D has run, matching is3D_'s own latch semantics above.
        float attenuation_   = 1.0f;
        float dopplerFactor_ = 1.0f;
        float spatialPan_    = 0.0f;

        // Heap-allocated (not inline) so its address is stable across a move of *this* -- the
        // SDL3_mixer callback holds a raw pointer to it as userdata, and a unique_ptr move
        // transfers ownership without changing that address, so no callback re-registration is
        // needed after moving a SoundEffectInstance with an active filter (T-4C).
        std::unique_ptr<FilterState> filterState_;

        // P11-PAN-001 (RFC-1): lazily allocates filterState_ if this is the first DSP-affecting
        // call for this instance (matching the existing INTERNAL_apply*Filter lazy-allocation
        // pattern) and (re)registers the shared cooked callback on `track_`. Unlike the filter
        // setters, this must run for EVERY playing track, not just filtered ones, since the
        // crossfeed pan matrix below now lives in the same callback and needs to run
        // unconditionally. Idempotent -- safe to call on every Play()/Apply3D(), matching
        // MIX_SetTrackCookedCallback's own documented "may be called... at any time" contract.
        // No-op if track_ hasn't been created yet.
        void EnsureTrackDspState();

        // P12-PITCH-001: converts the XNA `Pitch` property (range [-1,1], "-1 octave to +1
        // octave") to the playback-rate ratio SDL3_mixer's `MIX_SetTrackFrequencyRatio` expects.
        // Matches FNA exactly (SoundEffectInstance.cs:589-591): `(float)Math.Pow(2.0,
        // INTERNAL_pitch)`, an exponential octave curve -- NOT a linear multiplier. Split out as
        // a pure function so it's independently unit-testable and has exactly one implementation
        // shared by setPitchProperty(), Play()'s ApplyTrackProperties(), and
        // SoundEffect::Play(volume,pitch,pan)'s fire-and-forget path (a friend of this class).
        NOXNA static float INTERNAL_calculatePitchRatio(float pitch);

        // P11-PAN-001 (RFC-1): computes FNA's exact 4-coefficient stereo crossfeed pan matrix
        // for a 2-source-channel/2-destination-channel track (SoundEffectInstance.cs's
        // SetPanMatrixCoefficients, the `dspSettings.SrcChannelCount == 2` branch) -- split out
        // as a pure function so it's independently unit-testable. `ll`/`rl`/`lr`/`rr` name
        // each coefficient as "destination-channel-from-source-channel" (e.g. `rl` = the left
        // output's contribution from the right input channel); FNA's own `outputMatrix[0..3]`
        // uses the same left-speaker-first, source-major layout. This same matrix also produces
        // FNA's separate mono-source formula exactly when fed a duplicated-mono signal
        // (L == R), so no separate mono branch is needed by the caller.
        NOXNA static void INTERNAL_calculatePanCrossfeedMatrix(
            float pan, float& ll, float& rl, float& lr, float& rr);

        /**
         * @brief Constructs a SoundEffectInstance bound to the given sound effect.
         *
         * FNA's equivalent constructor is `internal`; only SoundEffect::CreateInstance() (a
         * friend) is meant to call this. Direct external construction is not part of the
         * public API and must not compile.
         *
         * @param soundEffect The sound effect to bind this instance to.
         */
        explicit SoundEffectInstance(const SoundEffect& soundEffect);

    public:
        /** @brief Destroys the instance and releases its audio track. */
        ~SoundEffectInstance() override;

        SoundEffectInstance(const SoundEffectInstance&) = delete;
        SoundEffectInstance& operator=(const SoundEffectInstance&) = delete;

        /** @brief Move-constructs a SoundEffectInstance, transferring ownership of the audio track. */
        NOXNA SoundEffectInstance(SoundEffectInstance&& other) noexcept;

        /** @brief Move-assigns a SoundEffectInstance, transferring ownership of the audio track. */
        NOXNA SoundEffectInstance& operator=(SoundEffectInstance&& other) noexcept;

        /** @brief Starts or resumes playback of this instance. */
        virtual void Play();

        /** @brief Stops playback of this instance immediately. */
        virtual void Stop();

        /**
         * @brief Stops playback of this instance.
         *
         * @param immediate If true, cuts off immediately; if false, allows release tails.
         * @throws System::InvalidOperationException if called on a DynamicSoundEffectInstance
         *         with @p immediate false (there is no authored loop to release into).
         */
        virtual void Stop(bool immediate);

        /** @brief Releases this sound effect instance. */
        void Dispose() override;

        /**
         * @brief Gets whether this instance has been disposed.
         *
         * @return true if disposed; otherwise false.
         */
        [[nodiscard]] virtual bool getIsDisposedProperty() const;

        /**
         * @brief Sets the playback volume. Values are passed through unclamped (matching FNA).
         *
         * @param volume New volume value.
         */
        void setVolumeProperty(const float& volume);

        /** @brief Sets the playback volume (move overload). */
        NOXNA void setVolumeProperty(float&& volume);

        /**
         * @brief Sets the stereo pan. Range [-1 (left), 1 (right)].
         *
         * @param pan New pan value.
         * @throws System::ObjectDisposedException if the instance has been disposed.
         * @throws System::ArgumentOutOfRangeException if @p pan is outside [-1, 1].
         */
        void setPanProperty(const float& pan);

        /** @brief Sets the stereo pan (move overload). */
        NOXNA void setPanProperty(float&& pan);

        /**
         * @brief Sets the pitch adjustment. Range [-1, 1].
         *
         * @param pitch New pitch value.
         */
        void setPitchProperty(const float& pitch);

        /** @brief Sets the pitch adjustment (move overload). */
        NOXNA void setPitchProperty(float&& pitch);

        /**
         * @brief Gets whether the sound loops continuously.
         *
         * @return true if looping; otherwise false.
         */
        [[nodiscard]] virtual bool getIsLoopedProperty() const;

        /**
         * @brief Sets whether the sound loops continuously.
         *
         * @param looped New loop flag.
         * @throws System::InvalidOperationException if the instance has already been played.
         */
        virtual void setIsLoopedProperty(const bool& looped);

        /** @brief Sets whether the sound loops (move overload). */
        NOXNA virtual void setIsLoopedProperty(bool&& looped);

        /**
         * @brief Gets the current playback state.
         *
         * @return Current SoundState.
         */
        [[nodiscard]] virtual SoundState getStateProperty() const;

        GetTypeNameHPP()
    };
}

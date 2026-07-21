// SPDX-License-Identifier: MS-PL
#pragma once

#ifdef SOUND_ENABLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#include <cstdint>

namespace CNA::Internal::Audio
{
    /// Ensures the process-global SDL2_mixer device is open, opening it on first call.
    ///
    /// Unlike SDL3_mixer's `MIX_Mixer*`, classic SDL2_mixer has no per-instance mixer-device
    /// object -- `Mix_OpenAudio`/`Mix_CloseAudio` operate on a single process-global device, so
    /// the pointer this returns carries no meaning beyond "non-null on success"; callers that
    /// only need the side effect (ensure initialized, else throw) may ignore the return value.
    /// Throws std::runtime_error and leaves the mixer uninitialized on failure, so a later call
    /// (from any thread) retries from scratch -- same retry philosophy as the SDL3_mixer version
    /// this replaces.
    ///
    /// AUDIO-002/AUD-04-001: thread-safety and request-vs-negotiated-format logging carried over
    /// unchanged from the SDL3_mixer implementation -- see DestroyMixer()'s and GetMixerGeneration()'s
    /// docs below for the rest of that carried-over rationale.
    void* GetMixer();

    /// Closes the SDL2_mixer device opened by GetMixer(). See GetMixer()'s doc for the shared
    /// AUDIO-002/AUD-04-008/009 rationale (still-relevant despite the SDL2_mixer channel model
    /// replacing SDL3_mixer's per-track object model) -- every Track created via CreateTrack()
    /// whose channel is still attached is force-detached (its `channel` is reset to -1) so a
    /// SoundEffectInstance holding one detects the invalidation via GetMixerGeneration(), exactly
    /// as it did for a freed MIX_Track before this migration.
    void DestroyMixer();

    /// AUD-04-008/009: monotonically increases by exactly one every time DestroyMixer() actually
    /// tears down an open mixer (never on a call where none was open). SoundEffectInstance
    /// captures this value when it creates a Track (Play()) and compares it before every later
    /// use, so a Track orphaned by a DestroyMixer() call is detected instead of dereferenced.
    std::uint64_t GetMixerGeneration();

    /// Persistent handle standing in for SDL3_mixer's `MIX_Track*` object, backed by a
    /// dedicated SDL2_mixer channel this Track owns only while actually playing/paused --
    /// classic SDL2_mixer has no persistent per-sound object, only an integer channel index that
    /// the mixer recycles the instant playback stops, so a Track's `channel` is allocated lazily
    /// in PlayTrack() and released back to the shared pool the moment playback halts (natural
    /// finish or StopTrack()), while the Track object itself survives across many Play() calls,
    /// matching how a SoundEffectInstance reuses one MIX_Track for its whole lifetime.
    ///
    /// Looping is NOT implemented via SDL2_mixer's own `loops` parameter to `Mix_PlayChannel`
    /// (which bakes the loop count into the mixer's internal channel state at play time and
    /// cannot later be told "stop looping, let this lap finish naturally") -- every Play always
    /// asks SDL2_mixer for exactly one lap (`loops=0`), and `TrackPlaying()` lazily restarts a
    /// still-`looping` Track whose lap has ended when polled. This trades a small
    /// (sub-frame-interval) gap on each loop restart for the ability to honor
    /// `SoundEffectInstance::Stop(false)`'s "let the current lap finish, then really stop"
    /// contract, which SDL2_mixer's native loop counter cannot express at all. Matches this
    /// codebase's existing poll-and-reconcile style (see DynamicSoundEffectInstance::Update()).
    struct Track
    {
        int channel = -1;                  ///< Dedicated Mix_Chunk channel, or -1 if not attached.
        Mix_Chunk* baseChunk = nullptr;     ///< Borrowed (not owned) -- the SoundEffect's decoded chunk.
        Mix_Chunk* pitchedChunk = nullptr;  ///< Owned resampled copy, only allocated when pitch != 0.
        float gain = 1.0f;
        float pan = 0.0f;
        bool looping = false;
        bool autoDestroy = false;           ///< true for SoundEffect::Play()'s fire-and-forget tracks.
        bool explicitlyStopped = true;      ///< Blocks the lazy loop-restart once StopTrack() runs.
    };

    /// Allocates a new, not-yet-playing Track. Never fails (plain heap allocation).
    Track* CreateTrack();

    /// Halts and fully destroys @p track (freeing any resampled chunk and returning its channel,
    /// if attached, to the shared pool). Safe to call with a Track whose channel already finished
    /// naturally. No-op if @p track is null.
    void DestroyTrack(Track* track);

    /// Points @p track at a borrowed, already-decoded @p chunk (ownership stays with the caller,
    /// e.g. SoundEffect). Does not itself start playback.
    void SetTrackAudio(Track* track, Mix_Chunk* chunk);

    /// Sets this track's gain (SDL2_mixer's 0-128 integer volume, converted from XNA's [0,1]
    /// float range). Applied immediately if the track is currently attached to a channel.
    void SetTrackGain(Track* track, float gain);

    /// Sets this track's stereo pan (range [-1,1]) and (de)registers the crossfeed-matrix mixing
    /// effect (see SoundEffectInstance.cpp's ComputePanCrossfeedMatrix) on its channel as needed.
    /// `pan == 0.0f` unregisters the effect entirely -- matches the SDL3_mixer version's identical
    /// "zero pan costs nothing" optimization.
    void SetTrackPan(Track* track, float pan);

    /// Rebuilds `pitchedChunk` as a linear-interpolation resample of `baseChunk` at the given
    /// playback-rate ratio (1.0 = unchanged, as returned by `2^pitch` -- see
    /// SoundEffectInstance::INTERNAL_calculatePitchRatio). `ratio == 1.0f` clears any previous
    /// resample and plays `baseChunk` directly. Does not itself restart a live channel --
    /// PlayTrack() picks up the new effective chunk on its next call.
    void SetTrackPitch(Track* track, float ratio);

    /// Sets whether this track should keep repeating once its current lap ends (see the Track
    /// looping note above). Does not itself start or restart playback.
    void SetTrackLooping(Track* track, bool looping);

    /// Starts (or restarts from the beginning) playback of @p track's effective chunk
    /// (`pitchedChunk` if set, else `baseChunk`). Allocates a channel from the shared pool if
    /// none is currently attached. Returns false if there is no chunk set or the channel pool is
    /// exhausted.
    bool PlayTrack(Track* track);

    /// DynamicSoundEffectInstance-only: attaches @p track's channel to @p placeholderChunk,
    /// looped natively via SDL2_mixer's own gapless loop counter (unlike PlayTrack()'s manual
    /// single-lap-plus-poll-restart scheme, which exists to support "exit loop, let this lap
    /// finish" -- a case DynamicSoundEffectInstance never needs, since its own Stop(false) always
    /// throws). The caller is expected to register its own Mix_RegisterEffect on this channel
    /// (which fully overwrites `placeholderChunk`'s silence with real streamed audio) before or
    /// after this call. Applies gain the same way PlayTrack() does; does not touch pan/other
    /// effects.
    bool PlayTrackAsStreamCarrier(Track* track, Mix_Chunk* placeholderChunk);

    /// Immediately halts playback and detaches @p track's channel (returned to the shared pool).
    /// Also clears `looping`, so a lazy TrackPlaying() poll never restarts it afterward -- matches
    /// `SoundEffectInstance::Stop(true)`'s "cut off immediately" contract. No-op if not attached.
    void StopTrack(Track* track);

    /// Like StopTrack(), but leaves `looping` untouched -- used for
    /// `SoundEffectInstance::Stop(false)`'s "let the current lap finish, then stop" contract:
    /// call SetTrackLooping(track, false) first, then let the current lap finish naturally
    /// (TrackPlaying()'s poll will not restart it, since looping is now false).
    void ExitTrackLoop(Track* track);

    /// Pauses playback on @p track's channel, if attached. No-op otherwise.
    void PauseTrack(Track* track);

    /// Resumes a paused @p track's channel, if attached. No-op otherwise.
    void ResumeTrack(Track* track);

    /// Returns whether @p track is audibly playing right now. Reconciles the lazy loop-restart
    /// described on the Track struct as a side effect: a `looping`, not-`explicitlyStopped` track
    /// whose channel has finished its current lap is silently restarted here before returning.
    bool TrackPlaying(Track* track);

    /// Returns whether @p track's channel is currently paused.
    bool TrackPaused(Track* track);

    /// Gets/sets the global master gain (XNA's SoundEffect.MasterVolume), backed by
    /// Mix_MasterVolume's 0-128 integer scale.
    float GetMasterGain();
    void SetMasterGain(float gain);
}
#endif

// SPDX-License-Identifier: MS-PL
#include "Microsoft/Xna/Framework/Media/MediaPlayer.hpp"
#include "CNA/Internal/Clamp.hpp"

#include <algorithm>
#include <atomic>

#ifdef SOUND_ENABLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include "CNA/Internal/Audio/AudioMixer.hpp"
#endif

namespace Microsoft::Xna::Framework::Media
{
    System::EventHandler<System::EventArgs> MediaPlayer::ActiveSongChanged;
    System::EventHandler<System::EventArgs> MediaPlayer::MediaStateChanged;

    bool         MediaPlayer::isMuted_               = false;
    bool         MediaPlayer::isRepeating_           = false;
    bool         MediaPlayer::isShuffled_            = false;
    MediaState   MediaPlayer::state_                 = MediaState::Stopped;
    float        MediaPlayer::volume_                = 1.0f;
    bool         MediaPlayer::initialized_           = false;
    SharpRuntime::intcs MediaPlayer::numSongsInQueuePlayed_ = 0;
    MediaQueue   MediaPlayer::queue_;
    std::mt19937 MediaPlayer::random_(std::random_device{}());

    bool         MediaPlayer::timerRunning_          = false;
    std::chrono::steady_clock::time_point MediaPlayer::timerStart_;
    std::chrono::duration<double> MediaPlayer::accumulatedTime_ =
        std::chrono::duration<double>::zero();

#ifdef SOUND_ENABLED
    namespace
    {
        // SDL2_mixer's Mix_Music API is a much more direct fit for this class's needs than
        // SDL3_mixer's generic per-track model was -- it's purpose-built for exactly one
        // streamed background-music slot at a time, which is all MediaPlayer ever needs.
        Mix_Music* g_music = nullptr;

        std::atomic<bool> g_songEnded{false};

        // Mix_HookMusicFinished's callback takes no userdata/music argument (unlike SDL3_mixer's
        // per-track stopped callback) -- there is only ever one music slot, so none is needed.
        void SDLCALL OnMusicFinished()
        {
            // Called from the audio thread — only set a flag.
            g_songEnded.store(true, std::memory_order_relaxed);
        }

        void DestroyMusic()
        {
            if (g_music)
            {
                Mix_HaltMusic();
                Mix_FreeMusic(g_music);
                g_music = nullptr;
            }
        }

        void ApplyMusicVolume(float vol, bool muted)
        {
            Mix_VolumeMusic(CNA::Internal::Clamp(static_cast<int>((muted ? 0.0f : vol) * MIX_MAX_VOLUME), 0, MIX_MAX_VOLUME));
        }
    }
#endif

    // --- properties ---

    MediaState MediaPlayer::getStateProperty()
    {
        return state_;
    }

    void MediaPlayer::setStateProperty(MediaState value)
    {
        if (state_ != value)
        {
            state_ = value;
            Microsoft::Xna::Framework::FrameworkDispatcher::MediaStateChanged = true;
        }
    }

    // --- public methods ---

    void MediaPlayer::MoveNext()
    {
        NextSong(1);
    }

    void MediaPlayer::Stop()
    {
        if (getStateProperty() == MediaState::Stopped)
        {
            return;
        }

#ifdef SOUND_ENABLED
        DestroyMusic();
        g_songEnded.store(false, std::memory_order_relaxed);
#endif
        TimerStop();
        TimerReset();

        for (SharpRuntime::intcs i = 0; i < queue_.getCountProperty(); ++i)
        {
            if (queue_[i] != nullptr)
            {
                queue_[i]->setPlayCountProperty(0);
            }
        }

        setStateProperty(MediaState::Stopped);
    }

    bool MediaPlayer::DetectSongEndedByElapsedTime(Song* activeSong, System::TimeSpan elapsed)
    {
        if (activeSong == nullptr)
        {
            return false;
        }
        System::TimeSpan duration = activeSong->getDurationProperty();
        return duration > System::TimeSpan::Zero && elapsed >= duration;
    }

    void MediaPlayer::Update()
    {
        Song* activeSong = queue_.getActiveSongProperty();
        if (activeSong == nullptr || getStateProperty() != MediaState::Playing)
        {
            return;
        }

#ifdef SOUND_ENABLED
        bool songEnded = g_songEnded.exchange(false, std::memory_order_relaxed);
#else
        // No native track-stopped signal in this build configuration -- fall back to comparing
        // wall-clock elapsed time against the song's known duration (plan_media.md MEDIA-32).
        bool songEnded = DetectSongEndedByElapsedTime(activeSong, TimerElapsed());
#endif
        if (!songEnded)
        {
            return;
        }

        numSongsInQueuePlayed_ += 1;

        if (numSongsInQueuePlayed_ >= queue_.getCountProperty())
        {
            numSongsInQueuePlayed_ = 0;
            if (!isRepeating_)
            {
                Stop();
                Microsoft::Xna::Framework::FrameworkDispatcher::ActiveSongChanged = true;
                return;
            }
        }

        MoveNext();
    }

    void MediaPlayer::OnActiveSongChanged()
    {
        ActiveSongChanged.Raise(nullptr, System::EventArgs::Empty);
    }

    void MediaPlayer::OnMediaStateChanged()
    {
        MediaStateChanged.Raise(nullptr, System::EventArgs::Empty);
    }

    // --- private helpers ---

    void MediaPlayer::NextSong(SharpRuntime::intcs direction)
    {
        Stop();

        if (queue_.getCountProperty() == 0)
        {
            return;
        }

        if (isRepeating_ && queue_.getActiveSongIndexProperty() >= queue_.getCountProperty() - 1)
        {
            queue_.setActiveSongIndexProperty(0);
            direction = 0;
        }

        if (isShuffled_)
        {
            std::uniform_int_distribution<SharpRuntime::intcs> dist(
                0, queue_.getCountProperty() - 1
            );
            queue_.setActiveSongIndexProperty(dist(random_));
        }
        else
        {
            queue_.setActiveSongIndexProperty(
                static_cast<SharpRuntime::intcs>(
                    Microsoft::Xna::Framework::MathHelper::Clamp(
                        queue_.getActiveSongIndexProperty() + direction,
                        0,
                        queue_.getCountProperty() - 1
                    )
                )
            );
        }

        Song* nextSong = queue_[queue_.getActiveSongIndexProperty()];
        if (nextSong != nullptr)
        {
            PlaySong(nextSong);
        }

        Microsoft::Xna::Framework::FrameworkDispatcher::ActiveSongChanged = true;
    }

    void MediaPlayer::PlaySong(Song* song)
    {
        if (song == nullptr)
        {
            return;
        }

        if (!initialized_)
        {
            initialized_ = true;
        }

#ifdef SOUND_ENABLED
        // Stop and release any previously playing music.
        DestroyMusic();
        g_songEnded.store(false, std::memory_order_relaxed);

        CNA::Internal::Audio::GetMixer();

        // Mix_LoadMUS streams from disk rather than fully predecoding (matches the pre-migration
        // SDL3_mixer call's own `predecode=false` choice for long music tracks).
        g_music = Mix_LoadMUS(song->getHandle().c_str());
        if (!g_music)
        {
            return;
        }

        ApplyMusicVolume(volume_, isMuted_);
        Mix_HookMusicFinished(OnMusicFinished);

        // Report duration from the audio asset. Mix_MusicDuration returns <= 0 for formats it
        // cannot determine duration for (e.g. some streamed formats) -- left unset in that case,
        // matching the pre-migration implementation's own best-effort "only if available" guard.
        const double seconds = Mix_MusicDuration(g_music);
        if (seconds > 0.0)
        {
            song->setDurationProperty(System::TimeSpan::FromSeconds(seconds));
        }

        if (Mix_PlayMusic(g_music, 1) != 0)
        {
            DestroyMusic();
            return;
        }

        song->setPlayCountProperty(song->getPlayCountProperty() + 1);
#endif

        TimerReset();
        TimerStart();
        setStateProperty(MediaState::Playing);
    }

    // --- timer ---

    void MediaPlayer::TimerStart()
    {
        if (!timerRunning_)
        {
            timerStart_    = std::chrono::steady_clock::now();
            timerRunning_  = true;
        }
    }

    void MediaPlayer::TimerStop()
    {
        if (timerRunning_)
        {
            accumulatedTime_ += std::chrono::steady_clock::now() - timerStart_;
            timerRunning_    = false;
        }
    }

    void MediaPlayer::TimerReset()
    {
        accumulatedTime_ = std::chrono::duration<double>::zero();
        if (timerRunning_)
        {
            timerStart_ = std::chrono::steady_clock::now();
        }
    }

    System::TimeSpan MediaPlayer::TimerElapsed()
    {
        auto elapsed = accumulatedTime_;
        if (timerRunning_)
        {
            elapsed += std::chrono::steady_clock::now() - timerStart_;
        }
        return System::TimeSpan::FromSeconds(elapsed.count());
    }
}

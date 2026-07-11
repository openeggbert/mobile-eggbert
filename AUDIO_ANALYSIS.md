# Audio Issue Analysis — High-Pitched Sound Effects & Missing Audio Files

## Source of the report

User comment (YouTube, channel presumably showcasing a Mobile Eggbert build):

> "Will you be able to fix the high pitched sound effects and missing audio files in a near
> future update?"

Follow-up from the same commenter:

> "I'm talking about the audio that's playing in the video. Unless you purposely sped it up,
> which may be why it sounds like that."

This is an **analysis only** document. No code changes were made. It was written after reading
the current `mobile-eggbert` and `cna` source; no runtime/device testing was performed, so several
points below are flagged as needing empirical verification rather than confirmed root causes.

This exact complaint (or an earlier equivalent) is already tracked in `TODO.md`:

```
- [ ] Audio Issue: High-pitched sound effects reported - investigate if audio is being played at incorrect speed/pitch
- [ ] Audio Issue: Some audio files appear to be missing or not loading properly
- [ ] User Report: Audio in gameplay video sounds distorted (possibly sped up) - verify audio playback rate is correct
```

added in the current `develop` HEAD commit (`c2202a1 TODO.md was updated`). This document is the
first deep-dive into that backlog entry.

---

## 1. "High-pitched" / sped-up sound effects

### Ruled out

- **`Sound.hpp:190` (`tableVolumePitch`)** — the per-channel volume/pitch lookup table in
  `WindowsPhoneSpeedyBlupi::Sound` was compared byte-for-byte against the original decompiled C#
  source (`mobile-eggbert-legacy/mobile-eggbert-core/Sound.cs:58`) and is **identical**. A handful
  of channels do have `pitch = 1.0` (one octave up) with `volume = 0.5`, but that is original game
  design (verbatim from the Windows Phone XNA release), not a porting bug.
- **`SoundEffectInstance.cpp:1074` (`setPitchProperty`)** — pitch is correctly clamped to
  `[-1, 1]` and converted via `2^pitch` (the XAudio2/FNA octave-based convention). An earlier,
  audibly-wrong linear approximation was already fixed per the inline comment at lines 76–79.
- **`SoundEffect.cpp:194-224`** (WAV loading via `MIX_LoadAudio`) — the sample rate is **read from
  the actual file** via `MIX_GetAudioFormat()` / `spec.freq`, not hardcoded. Verified against the
  actual assets in `Content/sounds/`: 84 files are 22050 Hz mono, 9 files are 11025 Hz mono — no
  file is 44100 Hz. The loader does not assume a fixed source rate.

### Still suspect (not verified at runtime)

- **`CNA/Internal/Audio/AudioMixer.cpp:28-31`** — the shared SDL3_mixer output device is hardcoded
  to `44100 Hz / stereo / SDL_AUDIO_S16`. Every loaded sound (22050 Hz and 11025 Hz files) must be
  resampled from its native rate up to the 44100 Hz device rate during playback. This is expected
  to happen automatically inside SDL3_mixer (submodule `third_party/SDL_mixer`, pinned at
  `release-3.2.0-23-g3075d3ed`) on a per-track basis, but nothing in the codebase asserts or tests
  this — it is pure reliance on the library's internal behavior. If per-track resampling is not
  actually engaging (e.g. due to a backend-specific SDL3_mixer quirk on Android, or an API misuse
  when a track is bound to the mixer), every sound would play faster and higher-pitched than
  intended — proportionally more so for the 11025 Hz files (4x device rate) than the 22050 Hz
  files (2x device rate).
- The complaint describes the **audio across the whole video** sounding sped up, not one specific
  sound effect. That pattern fits a systemic/device-level resampling issue better than the
  per-channel `tableVolumePitch` table, which only affects specific SFX channels, not audio
  uniformly.
- `TODO.md` already separately notes *"Web version: Sound is a little bit delayed"* — evidence
  that the audio backend already behaves differently across platforms/backends. **Which platform
  the reported video was captured on (Android vs. web/Emscripten) is unknown** and would
  meaningfully narrow this down, since CNA's audio path (SDL3_mixer) differs by backend.

### Suggested next steps (not performed here)

1. Determine which build (Android APK vs. web build) the commenter's video is from.
2. Add temporary diagnostic logging of `MIX_GetAudioFormat()` (detected source freq) vs. the
   actual output device freq at track-start time, on the affected platform.
3. Empirically play a single known 22050 Hz asset (e.g. `sound000.wav`) in isolation on device and
   measure actual playback duration/pitch against the expected value, to confirm or rule out a
   resampling failure.

---

## 2. "Missing audio files"

### Concrete suspect found

- **`cna/include/Microsoft/Xna/Framework/Content/ContentManager.hpp:190-222`
  (`ContentManager::ResolveAssetPath`)** uses `std::filesystem::exists()` to probe whether a
  candidate asset path exists before returning it. On Android, assets packaged inside the APK
  (`android/app/src/main/assets/Content` → symlink to `../../../../../Content`) are **not regular
  files on the device filesystem** — they only exist inside the APK's asset store, reachable via
  `AAssetManager`, not via a POSIX path. `std::filesystem::exists()` only sees the real OS
  filesystem / process working directory, so on Android it will effectively always return `false`
  for packaged assets.
  - There is a fallback (`ResolveAssetPath`, final line: `return base;`) so a constructed path is
    still returned even when the existence probe fails — the loader is not necessarily broken
    outright.
  - Whether the downstream loader (`SoundEffect(path)` → `MIX_LoadAudio` → SDL3's
    `SDL_IOFromFile`) transparently falls back to `AAssetManager` for a path that failed the
    `std::filesystem::exists()` check depends on SDL3's own Android I/O backend behavior. This
    was **not verified empirically** — it needs a real Android run with logging of
    `SDL_GetError()` on any failed `MIX_LoadAudio` call.
- **`android/app/build.gradle:82-99`** — the comment above `sourceSets` claims assets are packaged
  "from the mobile-eggbert root," but the actual `assets.srcDirs = ['src/main/assets']` relies on
  two symlinks:
  ```
  android/app/src/main/assets/Content -> ../../../../../Content
  android/app/src/main/assets/worlds  -> ../../../../../worlds
  ```
  This generally works because Gradle's asset merge task follows symlinks on Linux, but it is a
  fragile point — a different Gradle version, CI environment, or symlink-unaware tooling could
  silently drop or partially package the `sounds/` subtree. This was **not verified** by unzipping
  an actual built APK.
- **`Sound::PlayImage`, `Sound.cpp:230`** — if `soundEffects.size()` ends up smaller than the
  channel index being requested (e.g. because `LoadContent()` partially failed — one `.wav` threw
  during `MIX_LoadAudio` and aborted the loop, or was silently skipped), the bounds check
  `rawChannel >= 0 && rawChannel < soundEffects.size()` simply causes that sound to never play,
  with **no error, no log output**. This would look exactly like "some audio files are missing"
  from a player's perspective, without leaving any trace to grep for in a crash log.

### Ruled out

- `Def::getHasSoundProperty()` (`Def.hpp:195`) is a `constexpr` always returning `true` — not
  platform-gated, so it cannot selectively disable audio on one platform.
- All 93 expected files (`sound000.wav` … `sound092.wav`) are present and non-empty in
  `Content/sounds/` in the repository; there is no gap in the local asset set.

### Suggested next steps (not performed here)

1. Unzip a built release/debug APK and confirm `assets/Content/sounds/*.wav` contains all 93
   files.
2. Add error logging around `MIX_LoadAudio` failures in `SoundEffect.cpp:205-211` (temporarily, or
   permanently if acceptable) so a failed load surfaces in `adb logcat` instead of silently
   producing an unusable `SoundEffect`.
3. Confirm whether `ContentManager::ResolveAssetPath`'s `std::filesystem::exists()` checks should
   be replaced with an SDL3 I/O-based existence check (`SDL_IOFromFile` open/close) so path
   resolution behaves consistently across desktop and Android/packaged builds. This would also fix
   the same class of platform-filesystem mismatch for any other asset type reachable through
   `ContentManager::Load<T>`, not just `SoundEffect`.

---

## Summary

| Symptom | Status | Most likely area |
|---|---|---|
| High-pitched sound effects | Suspected but unconfirmed | SDL3_mixer per-track resampling from source rate (22050/11025 Hz) to the hardcoded 44100 Hz mixer device (`AudioMixer.cpp:28-31`) — needs runtime verification |
| Missing audio files | Suspected but unconfirmed | `ContentManager::ResolveAssetPath`'s use of `std::filesystem::exists()`, which cannot see Android APK assets (`ContentManager.hpp:190-222`); possibly compounded by silent swallow-on-bounds-miss in `Sound::PlayImage` (`Sound.cpp:230`) |

Both root causes proposed here are **candidates derived from static code reading**, not confirmed
via a live run of the app. Before attempting a fix, the recommended first step is targeted runtime
diagnostics (see the "Suggested next steps" under each section) to confirm which mechanism is
actually responsible, since more than one plausible cause exists for each symptom.

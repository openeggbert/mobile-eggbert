# Android Build Instructions — mobile-eggbert

This document explains how to build, install, and run mobile-eggbert as an
Android APK using SDL3, the Android NDK, and CMake.

---

## Prerequisites

| Tool | Recommended version |
|------|---------------------|
| Android Studio | Ladybug (2024.2) or newer |
| Android SDK | API level 35 |
| Android NDK | 28.2.13676358 (installed via SDK Manager) |
| CMake (NDK bundle) | 3.21+ (installed via SDK Manager) |
| Java (JDK) | 17 (bundled with Android Studio) |
| Git | any recent version |

### Install NDK and CMake via Android Studio

1. Open **Android Studio → Settings → SDK Manager → SDK Tools**.
2. Check **NDK (Side by side)** version **28.2.13676358**.
3. Check **CMake** (version 3.21 or higher).
4. Click **Apply** and let Android Studio download and install.

---

## Clone and initialise submodules

```bash
git clone <repository-url> speedy-blupi-2013
cd speedy-blupi-2013
git submodule update --init --recursive
```

The vendored SDL3 / SDL_image / SDL_mixer sources are required.  They live under
`mobile-eggbert/../cna/third_party/`.

---

## Build the debug APK

```bash
cd mobile-eggbert/android
./gradlew assembleDebug
```

The first build downloads Gradle (≈ 150 MB) and compiles the NDK libraries, so
it may take several minutes.  Subsequent incremental builds are much faster.

The produced APK is at:

```
mobile-eggbert/android/app/build/outputs/apk/debug/app-debug.apk
```

---

## Install and run on a device or emulator

Enable **USB debugging** on your device (or start an AVD in Android Studio).

```bash
# Install
adb install app/build/outputs/apk/debug/app-debug.apk

# Launch
adb shell am start -n org.openeggbert.speedyblupi/.SpeedyBlupiActivity

# View logs (filter by the app's tag or by pid)
adb logcat -s SDL SpeedyBlupi
```

To uninstall:

```bash
adb uninstall org.openeggbert.speedyblupi
```

---

## Build a release APK (unsigned)

```bash
./gradlew assembleRelease
```

To distribute the APK you must sign it.  See the
[Android developer documentation on app signing](https://developer.android.com/studio/publish/app-signing).

---

## Supported ABIs

The current Gradle configuration builds for **arm64-v8a** only.  To add other
ABIs (e.g. `x86_64` for the emulator) edit
`android/app/build.gradle` and extend the `abiFilters` list:

```groovy
abiFilters 'arm64-v8a', 'x86_64'
```

---

## Asset layout

Game assets are packaged into the APK as Android assets.  At runtime they are
read via `SDL_IOFromFile` which transparently falls back to the APK
`AAssetManager` when a file is not found in internal storage.

| Source directory | Path in APK |
|------------------|-------------|
| `Content/backgrounds/` | `Content/backgrounds/` |
| `Content/icons/`       | `Content/icons/` |
| `Content/sounds/`      | `Content/sounds/` |
| `worlds/`              | `worlds/` |

---

## Writable / persistent storage

Save data and configuration are written to the app's private internal storage
via `SDL_GetPrefPath("org.openeggbert", "speedyblupi")`.  This storage:

- persists across app restarts,
- is cleared when the app is uninstalled,
- is **not** accessible to other apps.

---

## Troubleshooting

### Gradle build fails: "NDK not found"

Make sure the NDK version in `android/app/build.gradle` (`ndkVersion`)
matches what you installed in the SDK Manager.

### `adb: device not found`

- Enable **Developer Options** and **USB debugging** on your Android device.
- Try `adb devices` to confirm the device is listed.

### App crashes on launch

Check `adb logcat` for native crash details.  Common causes:

- Missing `libmain.so` — rebuild with `./gradlew assembleDebug`.
- Asset not found — verify `git submodule update --init --recursive` completed
  successfully and `Content/` exists under `mobile-eggbert/`.

### Audio is silent

SDL_mixer must be built with OGG/WAV support.  The vendored build enables WAV
by default.  If you see mixer errors in logcat, check that the sound files are
present in the APK using:

```bash
aapt dump resources app/build/outputs/apk/debug/app-debug.apk | grep sound
```

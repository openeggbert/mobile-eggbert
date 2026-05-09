package org.openeggbert.speedyblupi;

import org.libsdl.app.SDLActivity;

/**
 * Speedy Blupi Android entry point.
 *
 * This class simply extends SDLActivity so that:
 *   - The application appears in the launcher as "Speedy Blupi".
 *   - The SDL3 Java glue (SDLActivity) handles all lifecycle, surface, input,
 *     and audio initialisation automatically.
 *
 * Native code lives in libmain.so (built from the game's C++ sources via CMake).
 * SDL3 discovers it by looking for a library named "main".
 */
public class SpeedyBlupiActivity extends SDLActivity {

    /**
     * Returns the name of the native shared library to load.
     * SDL3's Java glue calls this to find our game code.
     */
    @Override
    protected String[] getLibraries() {
        return new String[]{
                "SDL3",
                "main"
        };
    }
}

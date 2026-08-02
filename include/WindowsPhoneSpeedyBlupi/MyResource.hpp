/**
 * @file MyResource.hpp
 * @brief Declares the MyResource class, which provides localised UI strings.
 *
 * @details
 * MyResource is the C++ port of the original Windows Phone C# resource table.
 * It maps integer resource IDs (the TX_* constants) to localised strings for
 * one of four supported locales: French ("fr"), English (default), German
 * ("de"), or Czech ("cs").
 *
 * Strings are used throughout the game for:
 *  - Button labels on the main menu, settings screen, and in-game HUD
 *    (TX_BUTTON_* group, IDs 100–113).
 *  - Player statistics on the ranking / gamer screen
 *    (TX_GAMER_* group, IDs 200–203).
 *  - Trial-mode upsell messages shown on the purchase screen
 *    (TX_TRIAL* group, IDs 300–305).
 *  - In-game tutorial hint messages for the four training worlds
 *    (TX_TRAINING1xx/2xx/3xx/4xx groups, IDs 1000–4009) and their
 *    accelerometer-control equivalents (TX_TRAINING*a groups, IDs 11000–14009).
 *
 * The resource table is populated lazily: the first call to LoadString()
 * triggers EnsureInitialized(), which detects the active locale from the
 * platform default (std::locale("")) and then calls the appropriate
 * InitializeFR(), InitializeEN(), InitializeDE(), or InitializeCS() function.
 * Subsequent
 * calls return strings from the already-populated table.
 *
 * @note All TX_* constants are resource IDs, not array indices.
 * @note The German locale currently falls back to English strings for most
 *       training hints; only the button labels and trial text are translated.
 * @note This is localisation/UI code.  It does not affect gameplay state.
 * @see MyResource::LoadString()
 * @see MyResource::EnsureInitialized()
 */

#pragma once

#include <string>
#include <unordered_map>

#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace WindowsPhoneSpeedyBlupi
{
    using std::string;
    using SharpRuntime::intcs;

    /**
     * @class MyResource
     * @brief Provides localised string resources for the game's UI.
     *
     * @details
     * MyResource is a static-only class (no instances required).  It maps
     * integer resource IDs (TX_* constants) to localised strings for the active
     * language (French, English, or German).  Strings are loaded lazily on the
     * first call to LoadString(); the active language is detected from the
     * platform locale at that point and cannot be changed at runtime.
     *
     * String lookup via LoadString() is O(1) on average (unordered_map).
     * Unknown IDs return the sentinel string @c "???".
     *
     * Resource-ID ranges:
     *  - 100–113  Button / menu labels (TX_BUTTON_*, TX_BUTTON_SETUP_*,
     *             TX_BUTTON_RANKING)
     *  - 200–203  Gamer / ranking screen labels (TX_GAMER_*)
     *  - 300–305  Trial-mode upsell lines (TX_TRIAL*)
     *  - 1000–1022 Training world 1 d-pad hints (TX_TRAINING1xx)
     *  - 2000–2009 Training world 2 d-pad hints (TX_TRAINING2xx)
     *  - 3000–3010 Training world 3 d-pad hints (TX_TRAINING3xx)
     *  - 4000–4009 Training world 4 d-pad hints (TX_TRAINING4xx)
     *  - 11000–11022 Training world 1 accelerometer hints (TX_TRAINING1xxa)
     *  - 12000–12009 Training world 2 accelerometer hints (TX_TRAINING2xxa)
     *  - 13000–13010 Training world 3 accelerometer hints (TX_TRAINING3xxa)
     *  - 14000–14009 Training world 4 accelerometer hints (TX_TRAINING4xxa)
     *
     * @note All TX_* constants are resource IDs, not array indices.
     * @note This is localisation/UI code.  It does not affect gameplay state.
     * @see LoadString()
     */
    class MyResource
    {
    public:
        // -------------------------------------------------------------------
        // Button / menu labels (IDs 100–113)
        // -------------------------------------------------------------------

        static const intcs TX_BUTTON_PLAY;         ///< @brief Resource ID for the "Play" / "Jouer" button label (ID 100).
        static const intcs TX_BUTTON_MENU;         ///< @brief Resource ID for the "Home" / "Menu" button label (ID 101).
        static const intcs TX_BUTTON_BACK;         ///< @brief Resource ID for the "Back" / "Retour" button label (ID 102).
        static const intcs TX_BUTTON_RESTART;      ///< @brief Resource ID for the "Restart" / "Recommencer" button label (ID 103).
        static const intcs TX_BUTTON_CONTINUE;     ///< @brief Resource ID for the "Continue" / "Continuer" button label (ID 104).
        static const intcs TX_BUTTON_BUY;          ///< @brief Resource ID for the "Buy" / "Acheter" button label (ID 105).
        static const intcs TX_BUTTON_SETUP;        ///< @brief Resource ID for the "Setup" / "Réglages" settings button label (ID 107).
        static const intcs TX_BUTTON_SETUP_SOUNDS; ///< @brief Resource ID for the sound-effects toggle label in the settings screen (ID 108).
        static const intcs TX_BUTTON_SETUP_JUMP;   ///< @brief Resource ID for the jump-button position preference label in the settings screen (ID 109).
        static const intcs TX_BUTTON_SETUP_ZOOM;   ///< @brief Resource ID for the auto-zoom preference label in the settings screen (ID 110).
        static const intcs TX_BUTTON_SETUP_ACCEL;  ///< @brief Resource ID for the accelerometer-control preference label in the settings screen (ID 111).
        static const intcs TX_BUTTON_SETUP_RESET;  ///< @brief Resource ID for the "Erase progress" reset button label; contains a {0} placeholder for the player number (ID 112).
        static const intcs TX_BUTTON_RANKING;      ///< @brief Resource ID for the "Ranking" / "Classement" button label (ID 113).

        // -------------------------------------------------------------------
        // Gamer / ranking screen labels (IDs 200–203)
        // -------------------------------------------------------------------

        static const intcs TX_GAMER_TITLE;  ///< @brief Resource ID for the player title line; contains a {0} placeholder for the player number (ID 200).
        static const intcs TX_GAMER_MDOORS; ///< @brief Resource ID for the main-gate counter label; contains {0}/12 placeholders (ID 201).
        static const intcs TX_GAMER_SDOORS; ///< @brief Resource ID for the secondary-gate counter label; contains {0}/52 placeholders (ID 202).
        static const intcs TX_GAMER_LIFES;  ///< @brief Resource ID for the lives counter label; contains a {0} placeholder for the current life count (ID 203).

        // -------------------------------------------------------------------
        // Trial-mode upsell lines (IDs 300–305)
        // -------------------------------------------------------------------

        static const intcs TX_TRIAL1; ///< @brief Resource ID for the trial upsell headline: "Buy the full version" (ID 300).
        static const intcs TX_TRIAL2; ///< @brief Resource ID for trial bullet point 1: "64 exciting stages" (ID 301).
        static const intcs TX_TRIAL3; ///< @brief Resource ID for trial bullet point 2: "Varied backgrounds" (ID 302).
        static const intcs TX_TRIAL4; ///< @brief Resource ID for trial bullet point 3: "An increasing difficulty" (ID 303).
        static const intcs TX_TRIAL5; ///< @brief Resource ID for trial bullet point 4: "New traps" (ID 304).
        static const intcs TX_TRIAL6; ///< @brief Resource ID for trial bullet point 5: "Challenge and fun" (ID 305).

        // -------------------------------------------------------------------
        // Training world 1 — d-pad control hints (IDs 1000–1022)
        // -------------------------------------------------------------------

        static const intcs TX_TRAINING101; ///< @brief Tutorial hint 1-01: how to use the directional wheel (d-pad variant) (ID 1000).
        static const intcs TX_TRAINING102; ///< @brief Tutorial hint 1-02: how to use the Jump button (ID 1001).
        static const intcs TX_TRAINING103; ///< @brief Tutorial hint 1-03: combining right and jump (ID 1002).
        static const intcs TX_TRAINING104; ///< @brief Tutorial hint 1-04: pressing right and jump (ID 1003).
        static const intcs TX_TRAINING105; ///< @brief Tutorial hint 1-05: avoid falling into water (ID 1004).
        static const intcs TX_TRAINING106; ///< @brief Tutorial hint 1-06: empty / no text (ID 1005).
        static const intcs TX_TRAINING107; ///< @brief Tutorial hint 1-07: riding the elevator without jumping (ID 1006).
        static const intcs TX_TRAINING108; ///< @brief Tutorial hint 1-08: jump onto the elevator (ID 1007).
        static const intcs TX_TRAINING109; ///< @brief Tutorial hint 1-09: empty / no text (ID 1008).
        static const intcs TX_TRAINING110; ///< @brief Tutorial hint 1-10: move forward without jumping or stopping (ID 1009).
        static const intcs TX_TRAINING111; ///< @brief Tutorial hint 1-11: empty / no text (ID 1010).
        static const intcs TX_TRAINING112; ///< @brief Tutorial hint 1-12: empty / no text (ID 1011).
        static const intcs TX_TRAINING113; ///< @brief Tutorial hint 1-13: move forward on the platform (ID 1012).
        static const intcs TX_TRAINING114; ///< @brief Tutorial hint 1-14: leave the platform (ID 1013).
        static const intcs TX_TRAINING115; ///< @brief Tutorial hint 1-15: try again, but faster (ID 1014).
        static const intcs TX_TRAINING116; ///< @brief Tutorial hint 1-16: move onto the platform then jump (ID 1015).
        static const intcs TX_TRAINING117; ///< @brief Tutorial hint 1-17: jump while on the platform (ID 1016).
        static const intcs TX_TRAINING118; ///< @brief Tutorial hint 1-18: choose the upper path (ID 1017).
        static const intcs TX_TRAINING119; ///< @brief Tutorial hint 1-19: eggs give extra lives (ID 1018).
        static const intcs TX_TRAINING120; ///< @brief Tutorial hint 1-20: after reaching the top, advance onto the other platform promptly (ID 1019).
        static const intcs TX_TRAINING121; ///< @brief Tutorial hint 1-21: catch the second and last treasure (ID 1020).
        static const intcs TX_TRAINING122; ///< @brief Tutorial hint 1-22: go to the red arrow to finish (ID 1021).
        static const intcs TX_TRAINING123; ///< @brief Tutorial hint 1-23: (additional hint, locale-dependent text) (ID 1022).

        // -------------------------------------------------------------------
        // Training world 2 — d-pad control hints (IDs 2000–2009)
        // -------------------------------------------------------------------

        static const intcs TX_TRAINING201; ///< @brief Tutorial hint 2-01: push the crate to the red dot (ID 2000).
        static const intcs TX_TRAINING202; ///< @brief Tutorial hint 2-02: comment on the crate's usefulness (ID 2001).
        static const intcs TX_TRAINING203; ///< @brief Tutorial hint 2-03: pull the crate to the red dot (ID 2002).
        static const intcs TX_TRAINING204; ///< @brief Tutorial hint 2-04: stack two crates on the red dot (ID 2003).
        static const intcs TX_TRAINING205; ///< @brief Tutorial hint 2-05: stack three crates on the red dot (ID 2004).
        static const intcs TX_TRAINING206; ///< @brief Tutorial hint 2-06: empty / no text (ID 2005).
        static const intcs TX_TRAINING207; ///< @brief Tutorial hint 2-07: empty / no text (ID 2006).
        static const intcs TX_TRAINING208; ///< @brief Tutorial hint 2-08: empty / no text (ID 2007).
        static const intcs TX_TRAINING209; ///< @brief Tutorial hint 2-09: empty / no text (ID 2008).
        static const intcs TX_TRAINING210; ///< @brief Tutorial hint 2-10: empty / no text (ID 2009).

        // -------------------------------------------------------------------
        // Training world 3 — d-pad control hints (IDs 3000–3010)
        // -------------------------------------------------------------------

        static const intcs TX_TRAINING301; ///< @brief Tutorial hint 3-01: pick up a helicopter (ID 3000).
        static const intcs TX_TRAINING302; ///< @brief Tutorial hint 3-02: take off and steer the helicopter (ID 3001).
        static const intcs TX_TRAINING303; ///< @brief Tutorial hint 3-03: leave the helicopter (it dislikes water) (ID 3002).
        static const intcs TX_TRAINING304; ///< @brief Tutorial hint 3-04: diving and underwater steering controls (ID 3003).
        static const intcs TX_TRAINING305; ///< @brief Tutorial hint 3-05: pick up a helicopter and take off (ID 3004).
        static const intcs TX_TRAINING306; ///< @brief Tutorial hint 3-06: collect three treasures then ascend (ID 3005).
        static const intcs TX_TRAINING307; ///< @brief Tutorial hint 3-07: locate the skate in the top-left corner (ID 3006).
        static const intcs TX_TRAINING308; ///< @brief Tutorial hint 3-08: pick up a skate (ID 3007).
        static const intcs TX_TRAINING309; ///< @brief Tutorial hint 3-09: the skate lets you cross hazards safely (ID 3008).
        static const intcs TX_TRAINING310; ///< @brief Tutorial hint 3-10: the skate dislikes water; use jump to cross (ID 3009).
        static const intcs TX_TRAINING311; ///< @brief Tutorial hint 3-11: put down the skate (ID 3010).

        // -------------------------------------------------------------------
        // Training world 4 — d-pad control hints (IDs 4000–4009)
        // -------------------------------------------------------------------

        static const intcs TX_TRAINING401; ///< @brief Tutorial hint 4-01: pick up dynamite sticks (ID 4000).
        static const intcs TX_TRAINING402; ///< @brief Tutorial hint 4-02: warning — do not place dynamite here (ID 4001).
        static const intcs TX_TRAINING403; ///< @brief Tutorial hint 4-03: go get dynamite from the left (ID 4002).
        static const intcs TX_TRAINING404; ///< @brief Tutorial hint 4-04: place the dynamite then run (ID 4003).
        static const intcs TX_TRAINING405; ///< @brief Tutorial hint 4-05: place another stick of dynamite to proceed (ID 4004).
        static const intcs TX_TRAINING406; ///< @brief Tutorial hint 4-06: empty / no text (ID 4005).
        static const intcs TX_TRAINING407; ///< @brief Tutorial hint 4-07: empty / no text (ID 4006).
        static const intcs TX_TRAINING408; ///< @brief Tutorial hint 4-08: empty / no text (ID 4007).
        static const intcs TX_TRAINING409; ///< @brief Tutorial hint 4-09: empty / no text (ID 4008).
        static const intcs TX_TRAINING410; ///< @brief Tutorial hint 4-10: empty / no text (ID 4009).

        // -------------------------------------------------------------------
        // Training world 1 — accelerometer control hints (IDs 11000–11022)
        // These mirror TX_TRAINING1xx but reference tilt/accel input instead
        // of the directional wheel.
        // -------------------------------------------------------------------

        static const intcs TX_TRAINING101a; ///< @brief Accelerometer variant of TX_TRAINING101: tilt the phone (ID 11000).
        static const intcs TX_TRAINING102a; ///< @brief Accelerometer variant of TX_TRAINING102: press Jump (ID 11001).
        static const intcs TX_TRAINING103a; ///< @brief Accelerometer variant of TX_TRAINING103: tilt and press Jump (ID 11002).
        static const intcs TX_TRAINING104a; ///< @brief Accelerometer variant of TX_TRAINING104: tilt and Jump (ID 11003).
        static const intcs TX_TRAINING105a; ///< @brief Accelerometer variant of TX_TRAINING105: avoid falling into water (ID 11004).
        static const intcs TX_TRAINING106a; ///< @brief Accelerometer variant of TX_TRAINING106: empty / no text (ID 11005).
        static const intcs TX_TRAINING107a; ///< @brief Accelerometer variant of TX_TRAINING107: ride the elevator (ID 11006).
        static const intcs TX_TRAINING108a; ///< @brief Accelerometer variant of TX_TRAINING108: jump onto the elevator (ID 11007).
        static const intcs TX_TRAINING109a; ///< @brief Accelerometer variant of TX_TRAINING109: empty / no text (ID 11008).
        static const intcs TX_TRAINING110a; ///< @brief Accelerometer variant of TX_TRAINING110: move forward without stopping (ID 11009).
        static const intcs TX_TRAINING111a; ///< @brief Accelerometer variant of TX_TRAINING111: empty / no text (ID 11010).
        static const intcs TX_TRAINING112a; ///< @brief Accelerometer variant of TX_TRAINING112: empty / no text (ID 11011).
        static const intcs TX_TRAINING113a; ///< @brief Accelerometer variant of TX_TRAINING113: move forward on the platform (ID 11012).
        static const intcs TX_TRAINING114a; ///< @brief Accelerometer variant of TX_TRAINING114: leave the platform (ID 11013).
        static const intcs TX_TRAINING115a; ///< @brief Accelerometer variant of TX_TRAINING115: try again, faster (ID 11014).
        static const intcs TX_TRAINING116a; ///< @brief Accelerometer variant of TX_TRAINING116: move onto the platform then jump (ID 11015).
        static const intcs TX_TRAINING117a; ///< @brief Accelerometer variant of TX_TRAINING117: jump while on the platform (ID 11016).
        static const intcs TX_TRAINING118a; ///< @brief Accelerometer variant of TX_TRAINING118: choose the upper path (ID 11017).
        static const intcs TX_TRAINING119a; ///< @brief Accelerometer variant of TX_TRAINING119: eggs give extra lives (ID 11018).
        static const intcs TX_TRAINING120a; ///< @brief Accelerometer variant of TX_TRAINING120: advance on the next platform promptly (ID 11019).
        static const intcs TX_TRAINING121a; ///< @brief Accelerometer variant of TX_TRAINING121: catch the last treasure (ID 11020).
        static const intcs TX_TRAINING122a; ///< @brief Accelerometer variant of TX_TRAINING122: go to the red arrow (ID 11021).
        static const intcs TX_TRAINING123a; ///< @brief Accelerometer variant of TX_TRAINING123: (locale-dependent text) (ID 11022).

        // -------------------------------------------------------------------
        // Training world 2 — accelerometer hints (IDs 12000–12009)
        // -------------------------------------------------------------------

        static const intcs TX_TRAINING201a; ///< @brief Accelerometer variant of TX_TRAINING201: push the crate (ID 12000).
        static const intcs TX_TRAINING202a; ///< @brief Accelerometer variant of TX_TRAINING202: crate comment (ID 12001).
        static const intcs TX_TRAINING203a; ///< @brief Accelerometer variant of TX_TRAINING203: pull the crate (ID 12002).
        static const intcs TX_TRAINING204a; ///< @brief Accelerometer variant of TX_TRAINING204: stack two crates (ID 12003).
        static const intcs TX_TRAINING205a; ///< @brief Accelerometer variant of TX_TRAINING205: stack three crates (ID 12004).
        static const intcs TX_TRAINING206a; ///< @brief Accelerometer variant of TX_TRAINING206: empty / no text (ID 12005).
        static const intcs TX_TRAINING207a; ///< @brief Accelerometer variant of TX_TRAINING207: empty / no text (ID 12006).
        static const intcs TX_TRAINING208a; ///< @brief Accelerometer variant of TX_TRAINING208: empty / no text (ID 12007).
        static const intcs TX_TRAINING209a; ///< @brief Accelerometer variant of TX_TRAINING209: empty / no text (ID 12008).
        static const intcs TX_TRAINING210a; ///< @brief Accelerometer variant of TX_TRAINING210: empty / no text (ID 12009).

        // -------------------------------------------------------------------
        // Training world 3 — accelerometer hints (IDs 13000–13010)
        // -------------------------------------------------------------------

        static const intcs TX_TRAINING301a; ///< @brief Accelerometer variant of TX_TRAINING301: pick up a helicopter (ID 13000).
        static const intcs TX_TRAINING302a; ///< @brief Accelerometer variant of TX_TRAINING302: take off and steer with tilt (ID 13001).
        static const intcs TX_TRAINING303a; ///< @brief Accelerometer variant of TX_TRAINING303: leave the helicopter (ID 13002).
        static const intcs TX_TRAINING304a; ///< @brief Accelerometer variant of TX_TRAINING304: dive and steer underwater with tilt (ID 13003).
        static const intcs TX_TRAINING305a; ///< @brief Accelerometer variant of TX_TRAINING305: pick up a helicopter and take off (ID 13004).
        static const intcs TX_TRAINING306a; ///< @brief Accelerometer variant of TX_TRAINING306: collect treasures (ID 13005).
        static const intcs TX_TRAINING307a; ///< @brief Accelerometer variant of TX_TRAINING307: locate the skate (ID 13006).
        static const intcs TX_TRAINING308a; ///< @brief Accelerometer variant of TX_TRAINING308: pick up the skate (ID 13007).
        static const intcs TX_TRAINING309a; ///< @brief Accelerometer variant of TX_TRAINING309: pass safely with skate (ID 13008).
        static const intcs TX_TRAINING310a; ///< @brief Accelerometer variant of TX_TRAINING310: skate dislikes water (ID 13009).
        static const intcs TX_TRAINING311a; ///< @brief Accelerometer variant of TX_TRAINING311: put down the skate (ID 13010).

        // -------------------------------------------------------------------
        // Training world 4 — accelerometer hints (IDs 14000–14009)
        // -------------------------------------------------------------------

        static const intcs TX_TRAINING401a; ///< @brief Accelerometer variant of TX_TRAINING401: pick up dynamite (ID 14000).
        static const intcs TX_TRAINING402a; ///< @brief Accelerometer variant of TX_TRAINING402: do not place dynamite here (ID 14001).
        static const intcs TX_TRAINING403a; ///< @brief Accelerometer variant of TX_TRAINING403: go get dynamite (ID 14002).
        static const intcs TX_TRAINING404a; ///< @brief Accelerometer variant of TX_TRAINING404: place dynamite then run (ID 14003).
        static const intcs TX_TRAINING405a; ///< @brief Accelerometer variant of TX_TRAINING405: place another stick to proceed (ID 14004).
        static const intcs TX_TRAINING406a; ///< @brief Accelerometer variant of TX_TRAINING406: empty / no text (ID 14005).
        static const intcs TX_TRAINING407a; ///< @brief Accelerometer variant of TX_TRAINING407: empty / no text (ID 14006).
        static const intcs TX_TRAINING408a; ///< @brief Accelerometer variant of TX_TRAINING408: empty / no text (ID 14007).
        static const intcs TX_TRAINING409a; ///< @brief Accelerometer variant of TX_TRAINING409: empty / no text (ID 14008).
        static const intcs TX_TRAINING410a; ///< @brief Accelerometer variant of TX_TRAINING410: empty / no text (ID 14009).

    private:
        /// @brief In-memory resource table; populated on first access by EnsureInitialized().
        static std::unordered_map<intcs, std::string> resources;

    public:
        /**
         * @brief Returns the localised string for the given resource ID.
         *
         * @details
         * Initialises the resource table on first call via EnsureInitialized().
         * If @p res is not a known resource ID, returns the sentinel string
         * @c "???".
         *
         * @param[in] res Resource ID (one of the TX_* constants).
         * @return Const reference to the localised string, or to the sentinel
         *         string @c "???" if @p res is unknown.
         */
        static const string& LoadString(intcs res);

    private:
        /**
         * @brief Ensures the resource table is populated, calling Init() once.
         *
         * @details
         * Uses a static local boolean initialised via an immediately-invoked
         * lambda to guarantee Init() is called exactly once, regardless of
         * concurrent callers.
         */
        static void EnsureInitialized();

        /**
         * @brief Detects the active locale and delegates to the appropriate
         *        per-language initialiser.
         *
         * @details
         * Reads the platform default locale via @c std::locale("").  If the
         * first two characters of the locale name select a matching language
         * initialiser; otherwise InitializeEN() is called.
         * InitializeDE() is defined but not currently wired up by this function.
         * On any exception from @c std::locale, the language code defaults to
         * "en".
         */
        static void Init();

        /**
         * @brief Helper that constructs a std::string from a character array
         *        literal, including embedded null bytes.
         *
         * @details
         * Some tutorial strings embed control characters (including @c '\0')
         * that serve as button-icon placeholders in the rendering layer.
         * Using @c std::string(text, N-1) preserves those bytes, whereas a
         * plain @c std::string(text) constructor would stop at the first null.
         *
         * @tparam N  Size of the character array including the terminating null.
         * @param[in] text  Character array literal to convert.
         * @return A @c std::string containing exactly @c N-1 characters.
         */
        template <size_t N>
        static std::string MakeResourceString(const char (&text)[N])
        {
            return std::string(text, N - 1);
        }

        /**
         * @brief Populates the resource table with French strings.
         *
         * @details
         * Called by Init() when the platform locale starts with "fr".  Fills
         * @c resources with French translations for all TX_* IDs.
         */
        static void InitializeFR();

        /**
         * @brief Populates the resource table with English strings.
         *
         * @details
         * Called by Init() as the default when the locale is not French.
         * Fills @c resources with English translations for all TX_* IDs.
         */
        static void InitializeEN();

        /**
         * @brief Populates the resource table with Czech strings.
         *
         * @details
         * Called by Init() when the platform locale starts with "cs".  Czech
         * text intentionally uses ASCII-only spelling so it is readable with
         * the game's bitmap font.
         */
        static void InitializeCS();

        /**
         * @brief Populates the resource table with German strings.
         *
         * @details
         * Defined but not currently invoked by Init(); the German locale
         * path falls through to InitializeEN() instead.  The function provides
         * German button/trial labels and French tutorial strings as a
         * partial translation.
         */
        static void InitializeDE();
    };
}

#pragma once

#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace WindowsPhoneSpeedyBlupi
{
    using ushort = unsigned short;
    using SharpRuntime::bytecs;
    using SharpRuntime::intcs;

    /**
     * @brief Manages the persistent game data (save file) for all gamer slots.
     *
     * GameData stores and loads all player progress and settings in a flat byte array
     * that is serialised to/from the platform file system via Read() and Write().
     * This design matches the original game's save-data layout exactly.
     *
     * Layout of the internal byte array:
     * - Bytes 0..(SaveHeaderLength-1): global save header (selected gamer, audio settings, etc.)
     * - Bytes SaveHeaderLength..(TotalLength-1): three gamer blocks, each GamerLength bytes.
     *   - Each gamer block starts with a GamerHeaderLength-byte header (lives, world, options).
     *   - Followed by DoorsLength ints for door state.
     *
     * @note The byte layout is part of the original game's save format.
     *       Do not change field offsets without updating all accessor methods.
     * @note This class does not own gameplay state — it is the persistence layer.
     *       Decor reads/writes door state via InitializeDoors() and MemorizeDoors().
     */
    class GameData
    {
        /** Number of bytes in the global save-file header. */
        static constexpr intcs SaveHeaderLength = 10;

        /** Number of bytes in the per-gamer header (before the door array). */
        static constexpr intcs GamerHeaderLength = 10;

        /** Number of door state entries per gamer. */
        static constexpr intcs DoorsLength = 200;

        /** Total bytes per gamer block (header + doors). */
        static constexpr intcs GamerLength = GamerHeaderLength + DoorsLength;

        /** Maximum number of gamer save slots. */
        static constexpr intcs MaxGamer = 3;

        /** Total byte size of the save data array. */
        static constexpr intcs TotalLength = SaveHeaderLength + GamerLength * MaxGamer;

        /** Flat byte array holding all save data. Indexed by the accessor methods. */
        bytecs data[TotalLength]{};

    public:
        /** @brief Index of the currently selected gamer slot (0..MaxGamer-1). */
        [[nodiscard]] intcs getSelectedGamerProperty() const;
        void setSelectedGamerProperty(const intcs v);

        /** @brief Whether sound effects are enabled. */
        [[nodiscard]] bool getSoundsProperty() const;
        void setSoundsProperty(const bool v);

        /** @brief Whether the jump button is on the right side (true) or left side (false). */
        [[nodiscard]] bool getJumpRightProperty() const;
        void setJumpRightProperty(const bool v);

        /** @brief Whether the auto-zoom camera feature is enabled. */
        [[nodiscard]] bool getAutoZoomProperty() const;
        void setAutoZoomProperty(const bool v);

        /** @brief Whether accelerometer-based tilt movement is active. */
        [[nodiscard]] bool getAccelActiveProperty() const;
        void setAccelActiveProperty(const bool v);

        /** @brief Sensitivity of the accelerometer for movement, in [0,1]. */
        [[nodiscard]] double getAccelSensitivityProperty() const;
        void setAccelSensitivityProperty(double v);

        /** @brief Number of lives remaining for the current gamer. */
        [[nodiscard]] intcs getNbViesProperty() const;
        void setNbViesProperty(const intcs v);

        /** @brief Index of the last world completed by the current gamer. */
        [[nodiscard]] intcs getLastWorldProperty() const;
        void setLastWorldProperty(const intcs v);

        /** @brief Byte offset into the data array where the current gamer's block starts. */
        [[nodiscard]] intcs getGamerOffsetProperty() const;

        GameData();

        /**
         * @brief Reads save data from the platform file system into the data array.
         *
         * If the file does not exist or is invalid, the data is reset to defaults
         * via Initialize(). Call this once at startup.
         */
        void Read();

        /**
         * @brief Writes the current data array to the platform file system.
         *
         * Call after any change to player progress or settings to persist state.
         */
        void Write();

        /**
         * @brief Resets the data array to factory defaults for all gamer slots.
         *
         * Clears all progress and settings. Used by the "Reset" button in setup.
         */
        void Reset();

        /**
         * @brief Copies the door state for the current gamer into @p doors.
         * @param doors Output array of size DoorsLength receiving the door states.
         */
        void GetDoors(intcs doors[]);

        /**
         * @brief Stores @p doors into the save data for the current gamer.
         * @param doors Input array of size DoorsLength with the door states to save.
         */
        void SetDoors(const intcs doors[]);

        /**
         * @brief Retrieves high-level progress statistics for a specific gamer slot.
         *
         * @param gamer        Gamer slot index (0..MaxGamer-1).
         * @param nbVies       Output: number of lives for this gamer.
         * @param mainDoors    Output: count of main doors opened by this gamer.
         * @param secondaryDoors Output: count of secondary doors opened by this gamer.
         */
        void GetGamerInfo(intcs gamer, intcs& nbVies, intcs& mainDoors, intcs& secondaryDoors);

    private:
        void Initialize();

        void Initialize(intcs gamer);

        static intcs GetGamerOffset(intcs gamer);
    };
}

/**
 * @file GameData.hpp
 * @brief Declares the GameData class, which manages persistent save data for up
 *        to three gamer slots.
 *
 * @details
 * GameData serialises all player progress and settings into a flat byte array
 * that mirrors the original Windows Phone save format exactly.  The array is
 * read from / written to the platform file system via Worlds::ReadGameData() and
 * Worlds::WriteGameData().
 *
 * ### Byte-array layout
 * ```
 * Offset  Size  Field
 * ------  ----  -----
 * 0       1     (reserved / version tag, initialised to 1)
 * 1       1     (reserved, initialised to 1)
 * 2       1     selectedGamer   - index of the active gamer slot (0..2)
 * 3       1     sounds          - sound effects enabled (1=on, 0=off)
 * 4       1     jumpRight       - jump button on right side (1=yes, 0=no)
 * 5       1     autoZoom        - auto-zoom camera feature (1=on, 0=off)
 * 6       1     accelActive     - accelerometer tilt movement (1=on, 0=off)
 * 7       1     accelSensitivity- sensitivity as integer 0..100 (divide by 100.0 for [0,1])
 * 8       1     (reserved)
 * 9       1     (reserved)
 *   [SaveHeaderLength = 10 bytes]
 *
 * For each gamer g in {0, 1, 2}  (GamerLength = 210 bytes each):
 *   Offset = SaveHeaderLength + g * GamerLength
 *   +0      1     nbVies     - lives remaining (default 3)
 *   +1      1     lastWorld  - index of the last world reached (default 1)
 *   +2..+9  8     (reserved / padding to GamerHeaderLength = 10)
 *   +10..+209  200  doors[0..199] - door states (0=locked, 1=opened)
 *                   doors[0..179]  = secondary doors
 *                   doors[180..199]= main doors
 *   [GamerHeaderLength = 10 bytes; DoorsLength = 200 bytes]
 *
 * TotalLength = 10 + 3 * 210 = 640 bytes
 * ```
 *
 * @warning Changing any field offset or size silently corrupts existing save
 *          files.  Always update ALL accessor methods and the layout table above
 *          when modifying the array schema.
 *
 * @see Worlds::ReadGameData(), Worlds::WriteGameData()
 */

#pragma once

#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace WindowsPhoneSpeedyBlupi
{
    using ushort = unsigned short;
    using SharpRuntime::bytecs;
    using SharpRuntime::intcs;

    /**
     * @class GameData
     * @brief Manages the persistent save data for all gamer slots.
     *
     * @details
     * GameData stores and loads all player progress and settings in a flat byte
     * array that is serialised to/from the platform file system via Read() and
     * Write().  This design matches the original game's save-data layout exactly.
     *
     * Layout of the internal byte array:
     * - Bytes 0..(SaveHeaderLength-1): global save header (selected gamer, audio
     *   settings, etc.)
     * - Bytes SaveHeaderLength..(TotalLength-1): three gamer blocks, each
     *   GamerLength bytes.
     *   - Each gamer block starts with a GamerHeaderLength-byte header (lives,
     *     world, options).
     *   - Followed by DoorsLength bytes for door state.
     *
     * @note The byte layout is part of the original game's save format.
     *       Do not change field offsets without updating all accessor methods.
     * @note This class does not own gameplay state — it is the persistence layer.
     *       Decor reads/writes door state via InitializeDoors() and MemorizeDoors().
     */
    class GameData
    {
        /** @brief Number of bytes in the global save-file header. */
        static constexpr intcs SaveHeaderLength = 10;

        /** @brief Number of bytes in the per-gamer header (before the door array). */
        static constexpr intcs GamerHeaderLength = 10;

        /** @brief Number of door state entries per gamer (200 total: 180 secondary + 20 main). */
        static constexpr intcs DoorsLength = 200;

        /** @brief Total bytes per gamer block (GamerHeaderLength + DoorsLength). */
        static constexpr intcs GamerLength = GamerHeaderLength + DoorsLength;

        /** @brief Maximum number of gamer save slots. */
        static constexpr intcs MaxGamer = 3;

        /** @brief Total byte size of the save data array (SaveHeaderLength + GamerLength * MaxGamer). */
        static constexpr intcs TotalLength = SaveHeaderLength + GamerLength * MaxGamer;

        /** @brief Flat byte array holding all save data.  Indexed by the accessor methods.
         *
         *  @warning Do not access this array directly outside accessor methods;
         *           incorrect offsets silently corrupt save data.
         */
        bytecs data[TotalLength]{};

    public:
        /**
         * @brief Returns the index of the currently selected gamer slot.
         *
         * @details Reads data[2] and returns it as an intcs.  Valid range is
         *          0..(MaxGamer-1).
         *
         * @return Selected gamer index.
         */
        [[nodiscard]] intcs getSelectedGamerProperty() const;

        /**
         * @brief Sets the currently selected gamer slot.
         *
         * @param[in] v  Gamer index to select (0..MaxGamer-1).
         *
         * @warning Values outside [0, MaxGamer-1] produce an out-of-range gamer
         *          offset in subsequent property accesses.
         */
        void setSelectedGamerProperty(const intcs v);

        /**
         * @brief Returns true if sound effects are enabled.
         *
         * @details Reads data[3]: 1 = enabled, 0 = disabled.
         *
         * @return True when sounds are on.
         */
        [[nodiscard]] bool getSoundsProperty() const;

        /**
         * @brief Enables or disables sound effects.
         *
         * @param[in] v  True to enable sounds; false to disable.
         */
        void setSoundsProperty(const bool v);

        /**
         * @brief Returns true if the jump button is on the right side of the screen.
         *
         * @details Reads data[4]: 1 = right, 0 = left.
         *
         * @return True when jump button is on the right.
         */
        [[nodiscard]] bool getJumpRightProperty() const;

        /**
         * @brief Sets the jump-button side preference.
         *
         * @param[in] v  True for right side; false for left side.
         */
        void setJumpRightProperty(const bool v);

        /**
         * @brief Returns true if the auto-zoom camera feature is enabled.
         *
         * @details Reads data[5]: 1 = enabled, 0 = disabled.
         *
         * @return True when auto-zoom is active.
         */
        [[nodiscard]] bool getAutoZoomProperty() const;

        /**
         * @brief Enables or disables the auto-zoom camera feature.
         *
         * @param[in] v  True to enable auto-zoom; false to disable.
         */
        void setAutoZoomProperty(const bool v);

        /**
         * @brief Returns true if accelerometer-based tilt movement is active.
         *
         * @details Reads data[6]: 1 = active, 0 = inactive.
         *
         * @return True when accelerometer movement is on.
         */
        [[nodiscard]] bool getAccelActiveProperty() const;

        /**
         * @brief Activates or deactivates accelerometer-based tilt movement.
         *
         * @param[in] v  True to enable; false to disable.
         */
        void setAccelActiveProperty(const bool v);

        /**
         * @brief Returns the accelerometer sensitivity as a floating-point value
         *        in [0,1].
         *
         * @details Reads data[7] as an integer in [0,100] and divides by 100.0.
         *
         * @return Sensitivity in [0.0, 1.0].
         */
        [[nodiscard]] double getAccelSensitivityProperty() const;

        /**
         * @brief Sets the accelerometer sensitivity.
         *
         * @details Clamps @p v to [0.0, 1.0] and stores it as an integer in
         *          [0,100] at data[7].
         *
         * @param[in] v  Desired sensitivity in [0.0, 1.0].
         */
        void setAccelSensitivityProperty(double v);

        /**
         * @brief Returns the number of lives remaining for the current gamer.
         *
         * @details Reads data[getGamerOffsetProperty() + 0].
         *
         * @return Current life count.
         */
        [[nodiscard]] intcs getNbViesProperty() const;

        /**
         * @brief Sets the number of lives for the current gamer.
         *
         * @param[in] v  Life count to store.
         */
        void setNbViesProperty(const intcs v);

        /**
         * @brief Returns the index of the last world reached by the current gamer.
         *
         * @details Reads data[getGamerOffsetProperty() + 1].
         *
         * @return Last-world index (1-based by game convention).
         */
        [[nodiscard]] intcs getLastWorldProperty() const;

        /**
         * @brief Sets the last-world index for the current gamer.
         *
         * @param[in] v  World index to store.
         */
        void setLastWorldProperty(const intcs v);

        /**
         * @brief Returns the byte offset into data[] where the current gamer's
         *        block begins.
         *
         * @details Delegates to GetGamerOffset(getSelectedGamerProperty()).
         *          Equals `SaveHeaderLength + GamerLength * selectedGamer`.
         *
         * @return Byte offset in [SaveHeaderLength, TotalLength - GamerLength].
         */
        [[nodiscard]] intcs getGamerOffsetProperty() const;

        /**
         * @brief Constructs a GameData object and initialises the data array to
         *        factory defaults via Initialize().
         */
        GameData();

        /**
         * @brief Reads save data from the platform file system into the data array.
         *
         * @details Calls Worlds::ReadGameData().  If the file does not exist or
         *          cannot be read, the data retains whatever was set by the last
         *          Initialize() call.
         *
         * @post data[] reflects the persisted state if the file existed; otherwise
         *      data[] holds the defaults from the constructor.
         *
         * @note Call this once at startup, before accessing any property.
         */
        void Read();

        /**
         * @brief Writes the current data array to the platform file system.
         *
         * @details Calls Worlds::WriteGameData().  Must be called after any change
         *          to player progress or settings to persist the new state.
         */
        void Write();

        /**
         * @brief Resets the data array to factory defaults for the current gamer slot.
         *
         * @details Calls Initialize(getSelectedGamerProperty()), which resets lives
         *          to 3, last world to 1, and clears all door states for that slot.
         *          Global settings (sounds, zoom, etc.) are not touched.
         */
        void Reset();

        /**
         * @brief Copies the door state for the current gamer into @p doors.
         *
         * @details Reads DoorsLength bytes starting at
         *          `data[getGamerOffsetProperty() + GamerHeaderLength]`.
         *
         * @param[out] doors  Output array of at least DoorsLength elements;
         *                    receives the stored door states (0=locked, 1=opened).
         */
        void GetDoors(intcs doors[]);

        /**
         * @brief Stores door states into the save data for the current gamer.
         *
         * @details Writes DoorsLength bytes to
         *          `data[getGamerOffsetProperty() + GamerHeaderLength]`.
         *
         * @param[in] doors  Source array of at least DoorsLength elements with the
         *                   door states to persist.
         *
         * @warning Values other than 0 or 1 can be stored but have undefined game
         *          behaviour.
         */
        void SetDoors(const intcs doors[]);

        /**
         * @brief Retrieves high-level progress statistics for a specific gamer slot.
         *
         * @details
         * Reads the life count and scans the door array to count opened doors,
         * splitting them into secondary doors (indices 0..179) and main doors
         * (indices 180..199).  A door is considered opened when its stored value
         * equals 1.
         *
         * @param[in]  gamer           Gamer slot index (0..MaxGamer-1).
         * @param[out] nbVies          Number of lives for this gamer.
         * @param[out] mainDoors       Count of main doors opened (out of 20).
         * @param[out] secondaryDoors  Count of secondary doors opened (out of 180).
         */
        void GetGamerInfo(intcs gamer, intcs& nbVies, intcs& mainDoors, intcs& secondaryDoors);

    private:
        /**
         * @brief Resets the entire data array to factory defaults for all slots.
         *
         * @details Sets global-header fields (bytes 0-7) to their default values
         *          (sounds on, jump right, auto-zoom on, accel off, sensitivity 50%,
         *          selected gamer 0), then calls Initialize(i) for each gamer i.
         *
         * @post data[] is fully initialised; no previous state is preserved.
         */
        void Initialize();

        /**
         * @brief Resets the save-data block for a single gamer slot to defaults.
         *
         * @details Sets nbVies = 3, lastWorld = 1, and clears all door states to 0
         *          for the given gamer.
         *
         * @param[in] gamer  Gamer slot index (0..MaxGamer-1).
         */
        void Initialize(intcs gamer);

        /**
         * @brief Computes the byte offset of a gamer block within the data array.
         *
         * @details Returns `SaveHeaderLength + GamerLength * gamer`.
         *
         * @param[in] gamer  Gamer slot index (0..MaxGamer-1).
         * @return Byte offset of the gamer block.
         *
         * @warning Passing an out-of-range @p gamer produces an offset outside the
         *          valid array bounds, causing undefined behaviour on access.
         */
        static intcs GetGamerOffset(intcs gamer);
    };
}

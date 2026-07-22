/**
 * @file Worlds.hpp
 * @brief Declares the Worlds class: static helpers for reading and writing level
 *        files and save-game data in the Speedy Blupi line-based text format.
 *
 * @details
 * ### File format grammar
 * Each persistent game state document (world file or current-game save) consists
 * of one or more **sections**.  Every section occupies exactly one logical line
 * (newline-terminated) with the following structure:
 * @code
 *   <section>: [field=value ]... \n
 * @endcode
 * - Section name and payload are separated by ": " (colon + space).
 * - Each field is written as `name=value ` (trailing space acts as delimiter).
 * - Field values use type-specific encodings:
 *   - int:    decimal integer (e.g. `42`)
 *   - double: decimal float written with C locale, up to 15 significant digits
 *   - bool:   the literal strings `True` or `False` (case-insensitive on read)
 *   - point:  `x;y` (semicolon-separated integers, e.g. `10;20`)
 *   - int[]:  comma-separated integers, zero elements suppressed (e.g. `1,,3,`)
 * - The **Decor** section is special: after the header line, subsequent lines
 *   contain the row data as comma-separated integers (empty cell = -1).
 * - The **Doors** section stores door state as comma-separated integers directly
 *   after the section prefix; value 1 is the default and is suppressed on write.
 *
 * ### Error / fallback behaviour
 * All Get* methods return a safe default when the field is missing or malformed:
 * - GetIntField / GetIntArrayField: 0
 * - GetDoubleField: 0.0
 * - GetBoolField: false
 * - GetPointField: default-constructed TinyPoint {0, 0}
 * - GetDecorField: no value
 *
 * @see GameData
 */

#pragma once

#include "System/Nullable.hpp"
#include <string>
#include <vector>

#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Text/StringBuilder.hpp"
#include "WindowsPhoneSpeedyBlupi/TinyPoint.hpp"

namespace WindowsPhoneSpeedyBlupi
{
    using SharpRuntime::bytecs;
    using SharpRuntime::intcs;
    using std::string;

    /**
     * @class Worlds
     * @brief Provides static helper methods for loading, reading, and writing world
     *        and save-game data.
     *
     * @details
     * This class is a C++ port of the original C# static class Worlds from
     * WindowsPhoneSpeedyBlupi.  All methods are static; the class cannot be
     * instantiated.
     *
     * The public interface is split into two groups:
     * - **I/O helpers** (ReadWorld, ReadGameData, WriteGameData, ReadCurrentGame,
     *   WriteCurrentGame, DeleteCurrentGame): interact with the platform file
     *   system via IsolatedStorage and TitleContainer.
     * - **Serialisation helpers** (GetXxxField / WriteXxxField): parse or emit
     *   individual typed fields within the line-based text format described in
     *   the file-level @ref Worlds.hpp documentation.
     *
     * @note Status: Partial — some methods retain behavioural stubs from the
     *       original C# port.
     *
     * @see GameData
     */
    class Worlds
    {
    public:
        Worlds() = delete;  ///< @brief Not instantiable; all members are static.
        ~Worlds() = delete; ///< @brief Not instantiable; all members are static.

    private:
        static System::Text::StringBuilder output; ///< @brief Accumulation buffer used by the Write* family of methods.

        /**
         * @brief Returns the filename used for persistent game-data storage.
         *
         * @details Maps to the original C# static read-only property
         *          `GameDataFilename`.  The value is "SpeedyBlupi".
         *
         * @return Const reference to the static filename string.
         *
         * @note Status: Ported
         */
        [[nodiscard]] static const std::string& getGameDataFilenameProperty();

        /**
         * @brief Returns the filename used for the current (in-progress) game save.
         *
         * @details Maps to the original C# static read-only property
         *          `CurrentGameFilename`.  The value is "CurrentGame".
         *
         * @return Const reference to the static filename string.
         *
         * @note Status: Ported
         */
        [[nodiscard]] static const std::string& getCurrentGameFilenameProperty();

    public:
        /**
         * @brief Reads a world file and splits its content into lines.
         *
         * @details
         * Constructs the world filename from @p rank (zero-padded to three digits),
         * opens it via TitleContainer::OpenStream(), reads the entire text, and
         * splits it on newline characters.
         *
         * In the original C# code this method returns `null` on failure.  In this
         * C++ port `System::Nullable<T>` with no value represents that state.
         *
         * @param[in] gamer  Gamer identifier.  Present for compatibility with the
         *                   original C# signature; currently unused.
         * @param[in] rank   World rank (0-based).  Determines the filename, e.g.
         *                   `worlds/world001.txt`.
         * @return Optional vector of text lines on success; a default-constructed (no-value) Nullable if the
         *         file cannot be opened or read.
         *
         * @note Status: Partial
         */
        static System::Nullable<std::vector<std::string>> ReadWorld(intcs gamer, intcs rank);

    private:
        /**
         * @brief Returns the asset path for a world file by rank.
         *
         * @details Produces a path of the form `worlds/worldNNN.txt` where NNN is
         *          @p rank formatted to at least three decimal digits.
         *          Corresponds to the original C# method GetWorldFilename.
         *
         * @param[in] gamer  Gamer identifier; unused (present for API compatibility).
         * @param[in] rank   World rank.
         * @return World file path relative to the content root.
         *
         * @note Status: Ported
         */
        static std::string GetWorldFilename(intcs gamer, intcs rank);

    public:
        /**
         * @brief Reads persistent game data from IsolatedStorage into @p data.
         *
         * @details
         * Opens the file identified by getGameDataFilenameProperty(), reads up to
         * @p dataSize bytes, and writes them into @p data.  If the file does not
         * exist or cannot be read, the function returns false and @p data is
         * unchanged.
         *
         * @param[out] data      Destination buffer.
         * @param[in]  dataSize  Maximum number of bytes to read; should equal the
         *                       total size of the GameData byte array.
         * @return True if the file was found and data was successfully read;
         *         false otherwise.
         *
         * @note Status: Partial
         */
        static bool ReadGameData(bytecs data[], size_t dataSize);

        /**
         * @brief Writes persistent game data from @p data to IsolatedStorage.
         *
         * @details
         * Creates or overwrites the file identified by getGameDataFilenameProperty()
         * and writes exactly @p dataSize bytes from @p data.  Errors from
         * IsolatedStorageException are logged and silently swallowed.
         *
         * @param[in] data      Source buffer containing the serialised game state.
         * @param[in] dataSize  Number of bytes to write.
         *
         * @note Status: Partial
         */
        static void WriteGameData(const bytecs data[], size_t dataSize);

        /**
         * @brief Deletes the current-game save file from IsolatedStorage.
         *
         * @details
         * Removes the file identified by getCurrentGameFilenameProperty().  If the
         * file does not exist or deletion fails for any reason, the error is
         * silently ignored.
         *
         * @note Status: Partial
         */
        static void DeleteCurrentGame();

        /**
         * @brief Reads the current-game save text from IsolatedStorage.
         *
         * @details
         * Opens the file identified by getCurrentGameFilenameProperty() and reads
         * it as a UTF-8 string.  Returns a default-constructed (no-value) Nullable when the file does not exist
         * or an IsolatedStorageException is thrown (mirrors the original C# null
         * return on failure).  Returns an empty string if the file exists but is
         * empty.
         *
         * @return Optional string containing the save data; no value on failure.
         *
         * @note Status: Partial
         */
        static System::Nullable<string> ReadCurrentGame();

        /**
         * @brief Writes the current-game save text to IsolatedStorage.
         *
         * @details
         * Encodes @p data as UTF-8 bytes and writes them to the file identified
         * by getCurrentGameFilenameProperty(), creating or overwriting the file.
         *
         * @param[in] data  Save text to persist.
         *
         * @note Status: Partial
         */
        static void WriteCurrentGame(const string& data);

        /**
         * @brief Reads an integer-array field from the first matching section line.
         *
         * @details
         * Scans @p lines for the @p rank-th line whose prefix matches
         * `section + ":"`, then finds `name=` within that line, parses the
         * comma-separated value list, and stores the results into @p array.
         * Elements that fail to parse are written as 0.  If the section or field
         * is not found, @p array is left unchanged.
         *
         * Mirrors the behaviour of the original C# method as closely as practical.
         *
         * @param[in]  lines      Array of text lines from the save/world file.
         * @param[in]  lineCount  Number of lines in @p lines.
         * @param[in]  section    Section name to search for (e.g. "Decor").
         * @param[in]  rank       Zero-based occurrence index within matching sections.
         * @param[in]  name       Field name (e.g. "objects").
         * @param[out] array      Output buffer; receives up to @p arraySize integers.
         * @param[in]  arraySize  Capacity of @p array.
         *
         * @note Status: Partial
         */
        static void GetIntArrayField(
            const string lines[],
            intcs lineCount,
            const string& section,
            intcs rank,
            const string& name,
            intcs array[],
            intcs arraySize);

        /**
         * @brief Reads a boolean field from the first matching section line.
         *
         * @details
         * Locates the field value using TryGetFieldValueText(), then parses it
         * case-insensitively as `"true"` or `"false"`.
         *
         * @param[in] lines      Array of text lines.
         * @param[in] lineCount  Number of lines.
         * @param[in] section    Section name to search for.
         * @param[in] rank       Zero-based occurrence index within matching sections.
         * @param[in] name       Field name.
         * @return Parsed boolean value, or false if the field is missing or
         *         malformed.
         *
         * @note Status: Ported
         */
        static bool GetBoolField(
            const string lines[],
            intcs lineCount,
            const string& section,
            intcs rank,
            const string& name);

        /**
         * @brief Reads an integer field from the first matching section line.
         *
         * @details
         * Locates the field value using TryGetFieldValueText(), then parses it
         * as a decimal integer.
         *
         * @param[in] lines      Array of text lines.
         * @param[in] lineCount  Number of lines.
         * @param[in] section    Section name to search for.
         * @param[in] rank       Zero-based occurrence index within matching sections.
         * @param[in] name       Field name.
         * @return Parsed integer, or 0 if the field is missing or malformed.
         *
         * @note Status: Ported
         */
        static intcs GetIntField(
            const string lines[],
            intcs lineCount,
            const string& section,
            intcs rank,
            const string& name);

        /**
         * @brief Reads a double-precision floating-point field from the first
         *        matching section line.
         *
         * @details
         * Locates the field value using TryGetFieldValueText(), then parses it
         * using std::stod (C locale).
         *
         * @param[in] lines      Array of text lines.
         * @param[in] lineCount  Number of lines.
         * @param[in] section    Section name to search for.
         * @param[in] rank       Zero-based occurrence index within matching sections.
         * @param[in] name       Field name.
         * @return Parsed double, or 0.0 if the field is missing or malformed.
         *
         * @note Status: Partial
         */
        static double GetDoubleField(
            const string lines[],
            intcs lineCount,
            const string& section,
            intcs rank,
            const string& name);

        /**
         * @brief Reads a TinyPoint field from the first matching section line.
         *
         * @details
         * Finds the field value as `x;y` and parses the two semicolon-separated
         * integers independently.  Returns a default-constructed TinyPoint {0,0}
         * if the section, field, or either coordinate is missing or malformed.
         *
         * @param[in] lines      Array of text lines.
         * @param[in] lineCount  Number of lines.
         * @param[in] section    Section name to search for.
         * @param[in] rank       Zero-based occurrence index within matching sections.
         * @param[in] name       Field name.
         * @return Parsed TinyPoint, or {0,0} on failure.
         *
         * @note Status: Ported
         */
        static TinyPoint GetPointField(
            const string lines[],
            intcs lineCount,
            const string& section,
            intcs rank,
            const string& name);

        /**
         * @brief Reads a single decor value from a Decor section.
         *
         * @details
         * Locates the section header line, then navigates to row `x+1` relative to
         * that header line, and retrieves the comma-separated value at column @p y.
         * An empty column is returned as -1 (no decor tile).  Returns no value
         * when the section, row, or column is out of range or the value cannot be
         * parsed.
         *
         * Corresponds to the original C# method that returns `int?`; no value
         * represents null.
         *
         * @param[in] lines      Array of text lines.
         * @param[in] lineCount  Number of lines.
         * @param[in] section    Section name for the decor block.
         * @param[in] x          Row offset from the section header (0-based).
         * @param[in] y          Column index within the row (0-based).
         * @return Optional integer tile value; no value on any error.
         *
         * @note Status: Ported
         */
        static System::Nullable<intcs> GetDecorField(
            const string lines[],
            intcs lineCount,
            const string& section,
            intcs x,
            intcs y);

        /**
         * @brief Reads all door-state values from a Doors section line.
         *
         * @details
         * Finds the first line starting with `section + ":"`, strips the section
         * prefix, splits the remainder on commas, and fills @p doors.  An empty
         * element is treated as 1 (default/open door state).  Elements beyond
         * @p doorsSize are ignored.
         *
         * @param[in]  lines      Array of text lines.
         * @param[in]  lineCount  Number of lines.
         * @param[in]  section    Section name for the doors block.
         * @param[out] doors      Output array receiving the door states.
         * @param[in]  doorsSize  Capacity of @p doors.
         *
         * @note Status: Partial
         */
        static void GetDoorsField(
            const string lines[],
            intcs lineCount,
            const string& section,
            intcs doors[],
            intcs doorsSize);

        /**
         * @brief Clears the internal write buffer.
         *
         * @details Call before beginning a new serialisation sequence to discard
         *          any previously accumulated output.
         *
         * @note Status: Ported
         */
        static void WriteClear();

        /**
         * @brief Starts a new section in the write buffer.
         *
         * @details Appends `section + ": "` to the buffer.  Fields written
         *          afterward (WriteIntField, WriteBoolField, etc.) are appended on
         *          the same line until WriteEndSection() is called.
         *
         * @param[in] section  Name of the section to start.
         *
         * @note Status: Ported
         */
        static void WriteSection(const string& section);

        /**
         * @brief Writes an integer-array field to the write buffer.
         *
         * @details Emits `name=v0,v1,...,vN ` where zero values are suppressed
         *          (written as empty slots) to match the compact format used by the
         *          original C# code.
         *
         * @param[in] name       Field name.
         * @param[in] array      Array of integer values to write.
         * @param[in] arraySize  Number of elements in @p array.
         *
         * @note Status: Ported
         */
        static void WriteIntArrayField(const string& name, const intcs array[], intcs arraySize);

        /**
         * @brief Writes a boolean field to the write buffer.
         *
         * @details Emits `name=True ` or `name=False `.
         *
         * @param[in] name  Field name.
         * @param[in] n     Boolean value to write.
         *
         * @note Status: Partial
         */
        static void WriteBoolField(const string& name, bool n);

        /**
         * @brief Writes an integer field to the write buffer.
         *
         * @details Emits `name=value `.
         *
         * @param[in] name  Field name.
         * @param[in] n     Integer value to write.
         *
         * @note Status: Ported
         */
        static void WriteIntField(const string& name, intcs n);

        /**
         * @brief Writes a double-precision floating-point field to the write buffer.
         *
         * @details Emits `name=value ` using the C locale with up to 15 significant
         *          digits to ensure round-trip fidelity.
         *
         * @param[in] name  Field name.
         * @param[in] n     Double value to write.
         *
         * @note Status: Partial
         */
        static void WriteDoubleField(const string& name, double n);

        /**
         * @brief Writes a TinyPoint field to the write buffer.
         *
         * @details Emits `name=x;y `.
         *
         * @param[in] name  Field name.
         * @param[in] p     Point to write.
         *
         * @note Status: Ported
         */
        static void WritePointField(const string& name, TinyPoint p);

        /**
         * @brief Writes one decor data row to the write buffer.
         *
         * @details Emits a comma-separated line of integers; entries equal to -1
         *          are written as empty slots (no tile).  Terminates with a newline.
         *
         * @param[in] line       Array of tile values for one decor row.
         * @param[in] arraySize  Number of columns in @p line.
         *
         * @note Status: Ported
         */
        static void WriteDecorField(const intcs line[], intcs arraySize);

        /**
         * @brief Writes one doors data row to the write buffer.
         *
         * @details Emits a comma-separated line of door-state integers; entries
         *          equal to 1 (the default/open state) are written as empty slots
         *          to keep the file compact.  Terminates with a newline.
         *
         * @param[in] doors      Array of door-state values.
         * @param[in] arraySize  Number of elements in @p doors.
         *
         * @note Status: Ported
         */
        static void WriteDoorsField(const intcs doors[], intcs arraySize);

        /**
         * @brief Terminates the current section in the write buffer.
         *
         * @details Appends a newline character, ending the current section line.
         *          Call after all fields for a section have been written.
         *
         * @note Status: Ported
         */
        static void WriteEndSection();

        /**
         * @brief Returns the accumulated serialised text from the write buffer.
         *
         * @details Returns the complete document built by previous WriteSection /
         *          WriteXxxField / WriteEndSection calls since the last WriteClear().
         *
         * @return String containing the serialised save/world data.
         *
         * @note Status: Ported
         */
        static string GetWriteString();
    };
}

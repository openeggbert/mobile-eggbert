#pragma once

#include <filesystem>
#include <sstream>
#include <string>
#include <vector>

#include "CppDotNet/CppDotNetHelper.hpp"
#include "WindowsPhoneSpeedyBlupi/TinyPoint.hpp"
#define int_to_string(i) std::to_string(i)

namespace WindowsPhoneSpeedyBlupi {
    using std::string;

    class Worlds {
    public:
        Worlds() = delete;
        ~Worlds() = delete;
    private:
        static std::stringstream output;

        /**
         * @brief Represents a static property that holds the filename for the game's data.
         *
         * This property is initialized using a lambda function that returns the default game data filename,
         * which is "SpeedyBlupi".
         */
         [[nodiscard]] static const std::string & getGameDataFilenameProperty() ;

        /**
         * @brief Represents a static property that holds the filename for the current game.
         *
         * This property is initialized using a lambda function that returns the default filename
         * for the current game, which is "CurrentGame".
         */
         [[nodiscard]] static const std::string & getCurrentGameFilenameProperty() ;

    public:
        static std::vector<std::string> ReadWorld(intcs gamer, intcs rank);

    private:
        static std::string GetWorldFilename(intcs gamer, intcs rank);

    public:
        static bool ReadGameData(CppDotNet::bytecs data[], size_t dataSize);

        static void WriteGameData(CppDotNet::bytecs data[], size_t dataSize);

        static void DeleteCurrentGame();

        static string ReadCurrentGame();

        static void WriteCurrentGame(const string &data);

        static void GetIntArrayField(string lines[], intcs lineCount, const string& section, intcs rank,
                                     const string& name, intcs array[], intcs arraySize);

        static bool GetBoolField(const string lines[], intcs lineCount, const string &section, intcs rank,
                                 const string &name);

        static int GetIntField(const string lines[], intcs lineCount, const string &section, intcs rank,
                               const string &name);

        static double GetDoubleField(const string lines[], intcs lineCount, const string &section, intcs rank,
                                     const string &name);

    private:
        template<typename T>

        static T GetTypedField(const string lines[], intcs lineCount, const string &section, intcs rank,
                               const string &name) {
            for (intcs i = 0; i < lineCount; i++) {
                const string &text = lines[i];
                if (!text.starts_with(section + ":") || rank-- != 0) {
                    continue;
                }
                size_t num = text.find(name + "=");
                if (num == string::npos) return T{};

                num += name.length() + 1;
                size_t num2 = text.find(" ", num);
                if (num2 == string::npos) return T{};

                string value = text.substr(num, num2 - num);
                if constexpr (std::is_same_v<T, bool>) {
                    return value == "true";
                } else if constexpr (std::is_same_v<T, int>) {
                    return stoi(value);
                } else if constexpr (std::is_same_v<T, double>) {
                    return stod(value);
                }
            }
            return T{};
        }

    public:
        static TinyPoint GetPointField(const string lines[], intcs lineCount, const string &section, intcs rank,
                                       const string &name);

        static int GetDecorField(const string lines[], intcs lineCount, const string &section, intcs x, intcs y);

        static void GetDoorsField(const string lines[], intcs lineCount, const string &section, intcs doors[],
                                  intcs &doorCount);

        static void WriteClear();

        static void WriteSection(const string &section);

        static void WriteIntArrayField(const string &name, const intcs array[], const intcs &arraySize);

        static void WriteBoolField(const std::string &name, bool n);

        static void WriteIntField(const string &name, intcs n);

        static void WriteDoubleField(const std::string &name, double n);

        static void WritePointField(const string &name, TinyPoint p);

        static void WriteDecorField(const intcs line[], const intcs &arraySize);

        static void WriteDoorsField(const intcs doors[], const intcs &arraySize);

        static void WriteEndSection();

        static string GetWriteString();
    };
}

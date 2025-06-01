// WindowsPhoneSpeedyBlupi, Version=1.0.0.5, Culture=neutral, PublicKeyToken=6db12cd62dbec439
// WindowsPhoneSpeedyBlupi.Def

//using Microsoft.Xna.Framework.Input;
//using static WindowsPhoneSpeedyBlupi.Def;
#ifndef WORLDS_H
#define WORLDS_H
#include <filesystem>
#include <sstream>
#include <string>
#include <vector>

#include "CNA/CnaHelper.h"
#include "CNA/Prop.h"
#include "WindowsPhoneSpeedyBlupi/TinyPoint.h"
#define int_to_string(i) std::to_string(i)


namespace WindowsPhoneSpeedyBlupi {
    using std::string;


    class Worlds {
    private:
        static std::stringstream output;

        /**
         * @brief Represents a static property that holds the filename for the game's data.
         *
         * This property is initialized using a lambda function that returns the default game data filename,
         * which is "SpeedyBlupi".
         */
        dgetterstatic(std::string, GameDataFilename)

        /**
         * @brief Represents a static property that holds the filename for the current game.
         *
         * This property is initialized using a lambda function that returns the default filename
         * for the current game, which is "CurrentGame".
         */
        dgetterstatic(std::string, CurrentGameFilename)

    public:
        static std::vector<std::string> ReadWorld(int gamer, int rank);

    private:
        static std::string GetWorldFilename(int gamer, int rank);

    public:
        static bool ReadGameData(CNA::byte data[], size_t dataSize);

    public:
        static void WriteGameData(CNA::byte data[], size_t dataSize);

    public:
        static void DeleteCurrentGame();

    public:
        static string ReadCurrentGame();

    public:
        static void WriteCurrentGame(string &data);

    public:
        static void GetIntArrayField(string lines[], int lineCount, const string &section, int rank,
                                     const string &name, int array[], int arraySize);


        static bool GetBoolField(const string lines[], int lineCount, const string &section, int rank,
                                 const string &name);

        static int GetIntField(const string lines[], int lineCount, const string &section, int rank,
                               const string &name);

        static double GetDoubleField(const string lines[], int lineCount, const string &section, int rank,
                                     const string &name);

    private:
        template<typename T>

        static T GetTypedField(const string lines[], int lineCount, const string &section, int rank,
                               const string &name) {
            for (int i = 0; i < lineCount; i++) {
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
        static TinyPoint GetPointField(const string lines[], int lineCount, const string &section, int rank,
                                       const string &name);

        static int GetDecorField(const string lines[], int lineCount, const string &section, int x, int y);

        static void GetDoorsField(const string lines[], int lineCount, const string &section, int doors[],
                                  int &doorCount);

    public:
        static void WriteClear();

    public:
        static void WriteSection(const string &section);

    public:
        static void WriteIntArrayField(const string &name, const int array[], const int &arraySize);

    public:
        static void WriteBoolField(const std::string &name, bool n);

    public:
        static void WriteIntField(const string &name, int n);

    public:
        static void WriteDoubleField(const std::string &name, double n);

    public:
        static void WritePointField(const string &name, TinyPoint p);

    public:
        static void WriteDecorField(const int line[], const int &arraySize);

    public:
        static void WriteDoorsField(const int doors[], const int &arraySize);

    public:
        static void WriteEndSection();

    public:
        static string GetWriteString();
    };
}
#endif // WORLDS_H

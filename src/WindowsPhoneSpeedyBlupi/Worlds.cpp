// WindowsPhoneSpeedyBlupi, Version=1.0.0.5, Culture=neutral, PublicKeyToken=6db12cd62dbec439
// WindowsPhoneSpeedyBlupi.Worlds
// using System;
// using System.Diagnostics;
// using System.Globalization;
// using System.IO;
// using System.IO.IsolatedStorage;
// using System.Text;
// using System.Threading.Tasks;
// using Microsoft.Xna.Framework;
// using WindowsPhoneSpeedyBlupi;
// #if KNI && Web
// using Microsoft.JSInterop;
// using Microsoft.AspNetCore.Components;
// using static System.Runtime.InteropServices.JavaScript.JSType;
// #endif


#include "WindowsPhoneSpeedyBlupi/Worlds.h"

#include <fstream>
#include <iostream>

namespace WindowsPhoneSpeedyBlupi {
    //static class
    igetterstatic(std::string, GameDataFilename, Worlds, "SpeedyBlupi")
    igetterstatic(std::string, CurrentGameFilename, Worlds, "CurrentGame")

    std::vector<std::string> Worlds::ReadWorld(int gamer, int rank) {
        string worldFilename = GetWorldFilename(gamer, rank);

        string text;
        std::vector<std::string> lines;

        try {
            std::ifstream file(worldFilename);
            if (!file.is_open()) {
                throw std::runtime_error("Fatal error. Loading world failed: " + worldFilename);
            }

            std::stringstream buffer;
            buffer << file.rdbuf();
            text = buffer.str();
            file.close();
        } catch (const std::exception &e) {
            std::cerr << e.what() << std::endl;
            return {}; // Return empty vector in case of failure
        }

        std::stringstream ss(text);
        std::string line;
        while (std::getline(ss, line)) {
            lines.push_back(line);
        }

        return lines;
    }

    std::string Worlds::GetWorldFilename(int gamer, int rank) {
        std::ostringstream oss;
        oss << "worlds/world" << std::setw(3) << std::setfill('0') << rank << ".txt";
        return oss.str();
    }

    bool Worlds::ReadGameData(byte data[], size_t dataSize) {
        std::cout << "ReadGameData" << std::endl;

        std::ifstream file(getGameDataFilename(), std::ios::binary);
        if (!file.is_open()) {
            string gdf = getGameDataFilename();
            std::cerr << "Fatal error. Loading game data failed: " << gdf << std::endl;
            return false;
        }

        try {
            file.seekg(0, std::ios::end);
            size_t fileSize = file.tellg();
            file.seekg(0, std::ios::beg);

            size_t count = std::min(dataSize, fileSize);
            file.read(reinterpret_cast<char *>(data), count);
            file.close();
            return true;
        } catch (const std::exception &e) {
            std::cerr << e.what() << std::endl;
            return false;
        }
    }

    void Worlds::WriteGameData(byte data[], size_t dataSize) {
        std::cout << "WriteGameData" << std::endl;


        std::cout << "WriteGameData" << std::endl;

        std::ofstream file(getGameDataFilename(), std::ios::binary | std::ios::trunc);
        if (!file.is_open()) {
            string gdf = getGameDataFilename();
            std::cerr << "Fatal error. Writing game data failed: " << gdf << std::endl;
            return;
        }

        file.write(reinterpret_cast<const char *>(data), dataSize);
        file.close();
    }

    void Worlds::DeleteCurrentGame() {
        std::cout << "DeleteCurrentGame" << std::endl;

        const std::filesystem::path path{getCurrentGameFilename()};
        try {
            if (std::filesystem::exists(path)) {
                std::filesystem::remove(path);
            }
        } catch (const std::exception &e) {
            std::cerr << "Error deleting file: " << e.what() << std::endl;
        }
    }

    string Worlds::ReadCurrentGame() {
        std::cout << "ReadCurrentGame" << std::endl;

        std::ifstream file(getCurrentGameFilename(), std::ios::binary);
        if (!file.is_open()) {
            string cgf = getCurrentGameFilename();
            std::cerr << "Fatal error. Loading current game failed: " << cgf << std::endl;
            return "";
        }

        try {
            file.seekg(0, std::ios::end);
            size_t fileSize = file.tellg();
            file.seekg(0, std::ios::beg);

            std::vector<unsigned char> buffer(fileSize);
            file.read(reinterpret_cast<char *>(buffer.data()), fileSize);
            file.close();

            return std::string(buffer.begin(), buffer.end());
        } catch (const std::exception &e) {
            std::cerr << e.what() << std::endl;
            return "";
        }
    }

    void Worlds::WriteCurrentGame(string &data) {
        std::cout << "WriteCurrentGame" << std::endl;

        std::ofstream file(getCurrentGameFilename(), std::ios::out | std::ios::binary);
        if (file.is_open()) {
            file.write(data.c_str(), data.size());
            file.close();
        }
    }


    void Worlds::GetIntArrayField(const string lines[], int lineCount, const string &section, int rank,
                                  const string &name, int array[], int arraySize) {
        arraySize = 0;
        for (int i = 0; i < lineCount; i++) {
            const string &text = lines[i];
            if (!text.starts_with(section + ":") || rank-- != 0) {
                continue;
            }
            size_t num = text.find(name + "=");
            if (num == string::npos) break;

            num += name.length() + 1;
            size_t num2 = text.find(" ", num);
            if (num2 == string::npos) break;

            std::stringstream ss(text.substr(num, num2 - num));
            string item;
            while (getline(ss, item, ',')) {
                array[arraySize++] = stoi(item);
            }
        }
    }


    bool Worlds::GetBoolField(const string lines[], int lineCount, const string &section, int rank,
                              const string &name) {
        return GetTypedField<bool>(lines, lineCount, section, rank, name);
    }

    int Worlds::GetIntField(const string lines[], int lineCount, const string &section, int rank, const string &name) {
        return GetTypedField<int>(lines, lineCount, section, rank, name);
    }

    double Worlds::GetDoubleField(const string lines[], int lineCount, const string &section, int rank,
                                  const string &name) {
        return GetTypedField<double>(lines, lineCount, section, rank, name);
    }


    TinyPoint Worlds::GetPointField(const string lines[], int lineCount, const string &section, int rank,
                                    const string &name) {
        for (int i = 0; i < lineCount; i++) {
            const string &text = lines[i];
            if (!text.starts_with(section + ":") || rank-- != 0) {
                continue;
            }
            size_t num = text.find(name + "=");
            if (num == string::npos) return TinyPoint{0, 0};

            num += name.length() + 1;
            size_t num2 = text.find(";", num);
            size_t num3 = text.find(" ", num);
            if (num2 == string::npos || num3 == string::npos) return TinyPoint{0, 0};

            return TinyPoint{stoi(text.substr(num, num2 - num)), stoi(text.substr(num2 + 1, num3 - num2 - 1))};
        }
        return TinyPoint{0, 0};
    }

    int Worlds::GetDecorField(const string lines[], int lineCount, const string &section, int x, int y) {
        for (int i = 0; i < lineCount; i++) {
            if (!lines[i].starts_with(section + ":")) continue;

            const string &text = lines[i + 1 + x];
            string items[100]; // Assuming max possible elements
            size_t pos = 0, prev = 0, index = 0;

            while ((pos = text.find(',', prev)) != string::npos && index < 100) {
                items[index++] = text.substr(prev, pos - prev);
                prev = pos + 1;
            }
            items[index++] = text.substr(prev);

            return (y < index && !items[y].empty()) ? stoi(items[y]) : -1;
        }
        return -1;
    }

    void Worlds::GetDoorsField(const string lines[], int lineCount, const string &section, int doors[],
                               int &doorCount) {
        doorCount = 0;
        for (int i = 0; i < lineCount; i++) {
            const string &text = lines[i];
            if (!text.starts_with(section + ":")) continue;

            size_t startPos = section.length() + 2;
            string items[100]; // Assuming max possible elements
            size_t pos = startPos, prev = startPos, index = 0;

            while ((pos = text.find(',', prev)) != string::npos && index < 100) {
                items[index++] = text.substr(prev, pos - prev);
                prev = pos + 1;
            }
            items[index++] = text.substr(prev);

            for (int j = 0; j < index; j++) {
                doors[j] = (items[j].empty()) ? 1 : stoi(items[j]);
            }
            doorCount = index;
        }
    }


    void Worlds::WriteClear() {
        output.clear();
    }

    void Worlds::WriteSection(const string &section) {
        output << section;
        output << ": ";
    }

    void Worlds::WriteIntArrayField(const string &name, const int array[], const int &arraySize) {
        output << name;
        output << "=";
        for (int i = 0; i < arraySize; i++) {
            if (array[i] != 0) {
                output << std::to_string(array[i]);
            }
            if (i < arraySize - 1) {
                output << ",";
            }
        }
        output << " ";
    }

    void Worlds::WriteBoolField(const std::string &name, bool n) {
        std::cout << name << "=" << std::boolalpha << n << " ";
    }


    void Worlds::WriteIntField(const string &name, int n) {
        output << name;
        output << "=";
        output << std::to_string(n);
        output << " ";
    }

    void Worlds::WriteDoubleField(const std::string &name, double n) {
        std::cout << name << "=" << std::setprecision(15) << std::fixed << n << " ";
    }

    void Worlds::WritePointField(const string &name, TinyPoint p) {
        output << name;
        output << "=";
        output << std::to_string(p.X);
        output << ";";
        output << std::to_string(p.Y);
        output << " ";
    }

    void Worlds::WriteDecorField(const int line[], const int &arraySize) {
        for (int i = 0; i < arraySize; i++) {
            if (line[i] != -1) {
                output << int_to_string(line[i]);
            }
            if (i < arraySize - 1) {
                output << ",";
            }
        }
        output << "\n";
    }

    void Worlds::WriteDoorsField(const int doors[], const int &arraySize) {
        for (int i = 0; i < arraySize; i++) {
            if (doors[i] != 1) {
                output << int_to_string(doors[i]);
            }
            if (i < arraySize - 1) {
                output << ",";
            }
        }
        output << "\n";
    }

    void Worlds::WriteEndSection() {
        output << "\n";
    }

    string Worlds::GetWriteString() {
        return output.str();
    }
}

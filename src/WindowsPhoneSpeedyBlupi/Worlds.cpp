#include "WindowsPhoneSpeedyBlupi/Worlds.hpp"

#include <fstream>

#include "CNA/Logger.hpp"

namespace WindowsPhoneSpeedyBlupi {

    std::stringstream Worlds::output;
    using log = CNA::Logger;

    const std::string& Worlds::getGameDataFilenameProperty()
    {
        static std::string GAME_DATA = "SpeedyBlupi";
        return GAME_DATA;
    }
    const std::string& Worlds::getCurrentGameFilenameProperty()
    {
        static std::string CURRENT_GAME = "CurrentGame";
        return CURRENT_GAME;
    }

    std::vector<std::string> Worlds::ReadWorld(intcs gamer, intcs rank) {
        const string& worldFilename = GetWorldFilename(gamer, rank);

        string text;
        std::vector<std::string> lines;

        try {
            std::ifstream file(worldFilename);
            if (!file.is_open()) {
                log::Fatal("Fatal error. Loading world failed: " + worldFilename);
                throw std::runtime_error("Fatal error. Loading world failed: " + worldFilename);
            }

            std::stringstream buffer;
            buffer << file.rdbuf();
            text = buffer.str();
            file.close();
        } catch (const std::exception &e) {
            log::Error(e.what());
            {
                log::Fatal("Fatal error. Loading world failed: " + worldFilename);
                throw std::runtime_error("Fatal error. Loading world failed: " + worldFilename);
            }
            return {}; // Return empty vector in case of failure
        }

        std::stringstream ss(text);
        std::string line;
        while (std::getline(ss, line)) {
            lines.push_back(line);
        }

        return lines;
    }

    std::string Worlds::GetWorldFilename(intcs gamer, intcs rank) {
        std::ostringstream oss;
        oss << "worlds/world" << std::setw(3) << std::setfill('0') << rank << ".txt";
        return oss.str();
    }

    bool Worlds::ReadGameData(CppDotNet::bytecs data[], size_t dataSize) {
        log::Debug("ReadGameData");

        std::ifstream file(getGameDataFilenameProperty(), std::ios::binary);
        if (!file.is_open()) {
            const string& gdf = getGameDataFilenameProperty();
            log::Error("Fatal error. Loading game data failed: " + gdf);
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
            log::Error(e.what());
            return false;
        }
    }

    void Worlds::WriteGameData(CppDotNet::bytecs data[], size_t dataSize) {
        log::Debug("WriteGameData");

        std::ofstream file(getGameDataFilenameProperty(), std::ios::binary | std::ios::trunc);
        if (!file.is_open()) {
            const string& gdf = getGameDataFilenameProperty();
            log::Error("Fatal error. Writing game data failed: " + gdf);
            return;
        }

        file.write(reinterpret_cast<const char *>(data), dataSize);
        file.close();
    }

    void Worlds::DeleteCurrentGame() {
        log::Debug("DeleteCurrentGame");

        const std::filesystem::path path{getCurrentGameFilenameProperty()};
        try {
            if (std::filesystem::exists(path)) {
                std::filesystem::remove(path);
            }
        } catch (const std::exception &e) {
            log::Error (std::string("Error deleting file: ") + e.what());
        }
    }

    string Worlds::ReadCurrentGame() {
        log::Debug("ReadCurrentGame");

        std::ifstream file(getCurrentGameFilenameProperty(), std::ios::binary);
        if (!file.is_open()) {
            string cgf = getCurrentGameFilenameProperty();
            log::Error("Fatal error. Loading current game failed: " + cgf);
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
            log::Error(e.what());
            return "";
        }
    }

    void Worlds::WriteCurrentGame(const string &data) {
        log::Debug("WriteCurrentGame");

        std::ofstream file(getCurrentGameFilenameProperty(), std::ios::out | std::ios::binary);
        if (file.is_open()) {
            file.write(data.c_str(), data.size());
            file.close();
        }
    }


    void Worlds::GetIntArrayField(string lines[], intcs lineCount, const string& section, intcs rank,
                                  const string& name, intcs array[], intcs arraySize) {
        arraySize = 0;
        for (intcs i = 0; i < lineCount; i++) {
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


    bool Worlds::GetBoolField(const string lines[], intcs lineCount, const string &section, intcs rank,
                              const string &name) {
        return GetTypedField<bool>(lines, lineCount, section, rank, name);
    }

    int Worlds::GetIntField(const string lines[], intcs lineCount, const string &section, intcs rank, const string &name) {
        return GetTypedField<intcs>(lines, lineCount, section, rank, name);
    }

    double Worlds::GetDoubleField(const string lines[], intcs lineCount, const string &section, intcs rank,
                                  const string &name) {
        return GetTypedField<double>(lines, lineCount, section, rank, name);
    }


    TinyPoint Worlds::GetPointField(const string lines[], intcs lineCount, const string &section, intcs rank,
                                    const string &name) {
        for (intcs i = 0; i < lineCount; i++) {
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

    intcs Worlds::GetDecorField(const string lines[], intcs lineCount, const string &section, intcs x, intcs y) {
        for (intcs i = 0; i < lineCount; i++) {
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

    void Worlds::GetDoorsField(const string lines[], intcs lineCount, const string &section, intcs doors[],
                               intcs &doorCount) {
        doorCount = 0;
        for (intcs i = 0; i < lineCount; i++) {
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

            for (intcs j = 0; j < index; j++) {
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

    void Worlds::WriteIntArrayField(const string &name, const intcs array[], const intcs &arraySize) {
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
        output << name << "=" << std::boolalpha << n << " ";
    }


    void Worlds::WriteIntField(const string &name, intcs n) {
        output << name;
        output << "=";
        output << std::to_string(n);
        output << " ";
    }

    void Worlds::WriteDoubleField(const std::string &name, double n) {
        output << name << "=" << std::setprecision(15) << std::fixed << n << " ";
    }

    void Worlds::WritePointField(const string &name, TinyPoint p) {
        output << name;
        output << "=";
        output << std::to_string(p.X);
        output << ";";
        output << std::to_string(p.Y);
        output << " ";
    }

    void Worlds::WriteDecorField(const intcs line[], const intcs &arraySize) {
        for (intcs i = 0; i < arraySize; i++) {
            if (line[i] != -1) {
                output << int_to_string(line[i]);
            }
            if (i < arraySize - 1) {
                output << ",";
            }
        }
        output << "\n";
    }

    void Worlds::WriteDoorsField(const intcs doors[], const intcs &arraySize) {
        for (intcs i = 0; i < arraySize; i++) {
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

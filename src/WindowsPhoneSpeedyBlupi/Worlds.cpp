#include "WindowsPhoneSpeedyBlupi/Worlds.hpp"

#include <algorithm>
#include <charconv>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <locale>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include "CNA/Logger.hpp"

namespace WindowsPhoneSpeedyBlupi
{
    using log = CNA::Logger;

    std::stringstream Worlds::output;

    namespace
    {
        [[nodiscard]] bool StartsWith(const std::string& text, const std::string& prefix)
        {
            return text.size() >= prefix.size() &&
                   text.compare(0, prefix.size(), prefix) == 0;
        }

        [[nodiscard]] std::vector<std::string> Split(const std::string& text, char delimiter)
        {
            std::vector<std::string> result;
            std::stringstream ss(text);
            std::string item;
            while (std::getline(ss, item, delimiter))
            {
                result.push_back(item);
            }

            if (!text.empty() && text.back() == delimiter)
            {
                result.emplace_back();
            }

            return result;
        }

        [[nodiscard]] bool TryParseInt(const std::string& s, intcs& result)
        {
            try
            {
                std::size_t pos = 0;
                int value = std::stoi(s, &pos);
                if (pos != s.size())
                {
                    return false;
                }
                result = static_cast<intcs>(value);
                return true;
            }
            catch (...)
            {
                return false;
            }
        }

        [[nodiscard]] bool TryParseDouble(const std::string& s, double& result)
        {
            try
            {
                std::size_t pos = 0;
                double value = std::stod(s, &pos);
                if (pos != s.size())
                {
                    return false;
                }
                result = value;
                return true;
            }
            catch (...)
            {
                return false;
            }
        }

        [[nodiscard]] bool TryParseBool(const std::string& s, bool& result)
        {
            if (s == "True" || s == "true")
            {
                result = true;
                return true;
            }
            if (s == "False" || s == "false")
            {
                result = false;
                return true;
            }
            return false;
        }
    }

    const std::string& Worlds::getGameDataFilenameProperty()
    {
        static const std::string gameDataFilename = "SpeedyBlupi";
        return gameDataFilename;
    }

    const std::string& Worlds::getCurrentGameFilenameProperty()
    {
        static const std::string currentGameFilename = "CurrentGame";
        return currentGameFilename;
    }

    std::optional<std::vector<std::string>> Worlds::ReadWorld(intcs gamer, intcs rank)
    {
        (void)gamer;

        const std::string worldFilename = GetWorldFilename(gamer, rank);

        std::ifstream file(worldFilename, std::ios::binary);
        if (!file.is_open())
        {
            log::Error("Fatal error. Loading world failed: " + worldFilename);
            return std::nullopt;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string text = buffer.str();

        std::vector<std::string> lines;
        std::stringstream textStream(text);
        std::string line;
        while (std::getline(textStream, line, '\n'))
        {
            lines.push_back(line);
        }

        return lines;
    }

    std::string Worlds::GetWorldFilename(intcs gamer, intcs rank)
    {
        (void)gamer;

        std::ostringstream oss;
        oss << "worlds/world" << std::setw(3) << std::setfill('0') << rank << ".txt";
        return oss.str();
    }

    bool Worlds::ReadGameData(bytecs data[], size_t dataSize)
    {
        log::Debug("ReadGameData");

        std::ifstream file(getGameDataFilenameProperty(), std::ios::binary);
        if (!file.is_open())
        {
            return false;
        }

        file.seekg(0, std::ios::end);
        const std::streamsize length = file.tellg();
        if (length < 0)
        {
            return false;
        }

        file.seekg(0, std::ios::beg);
        const size_t count = std::min<size_t>(dataSize, static_cast<size_t>(length));

        file.read(reinterpret_cast<char*>(data), static_cast<std::streamsize>(count));
        return file.good() || file.eof();
    }

    void Worlds::WriteGameData(const bytecs data[], size_t dataSize)
    {
        log::Debug("WriteGameData");

        std::ofstream file(getGameDataFilenameProperty(), std::ios::binary | std::ios::trunc);
        if (!file.is_open())
        {
            log::Error("Writing game data failed: " + getGameDataFilenameProperty());
            return;
        }

        file.write(reinterpret_cast<const char*>(data), static_cast<std::streamsize>(dataSize));
    }

    void Worlds::DeleteCurrentGame()
    {
        log::Debug("DeleteCurrentGame");

        std::remove(getCurrentGameFilenameProperty().c_str());
    }

    std::optional<string> Worlds::ReadCurrentGame()
    {
        log::Debug("ReadCurrentGame");

        std::ifstream file(getCurrentGameFilenameProperty(), std::ios::binary);
        if (!file.is_open())
        {
            return std::nullopt;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    void Worlds::WriteCurrentGame(const string& data)
    {
        log::Debug("WriteCurrentGame");

        std::ofstream file(getCurrentGameFilenameProperty(), std::ios::binary | std::ios::trunc);
        if (!file.is_open())
        {
            log::Error("Writing current game failed: " + getCurrentGameFilenameProperty());
            return;
        }

        file.write(data.data(), static_cast<std::streamsize>(data.size()));
    }

    void Worlds::GetIntArrayField(
        const string lines[],
        intcs lineCount,
        const string& section,
        intcs rank,
        const string& name,
        intcs array[],
        intcs arraySize)
    {
        for (intcs i = 0; i < lineCount; i++)
        {
            const string& text = lines[i];

            if (!StartsWith(text, section + ":") || rank-- != 0)
            {
                continue;
            }

            const std::size_t num = text.find(name + "=");
            if (num == string::npos)
            {
                break;
            }

            const std::size_t start = num + name.length() + 1;
            const std::size_t end = text.find(" ", start);
            if (end == string::npos)
            {
                break;
            }

            const std::vector<std::string> values = Split(text.substr(start, end - start), ',');

            for (intcs j = 0; j < static_cast<intcs>(values.size()) && j < arraySize; j++)
            {
                intcs parsedValue = 0;
                if (TryParseInt(values[j], parsedValue))
                {
                    array[j] = parsedValue;
                }
                else
                {
                    array[j] = 0;
                }
            }

            break;
        }
    }

    bool Worlds::GetBoolField(
        const string lines[],
        intcs lineCount,
        const string& section,
        intcs rank,
        const string& name)
    {
        for (intcs i = 0; i < lineCount; i++)
        {
            const string& text = lines[i];
            if (StartsWith(text, section + ":") && rank-- == 0)
            {
                std::size_t num = text.find(name + "=");
                if (num == string::npos)
                {
                    return false;
                }

                num += name.length() + 1;
                const std::size_t num2 = text.find(" ", num);
                if (num2 == string::npos)
                {
                    return false;
                }

                const string value = text.substr(num, num2 - num);
                bool result = false;
                if (TryParseBool(value, result))
                {
                    return result;
                }
                return false;
            }
        }
        return false;
    }

    intcs Worlds::GetIntField(
        const string lines[],
        intcs lineCount,
        const string& section,
        intcs rank,
        const string& name)
    {
        for (intcs i = 0; i < lineCount; i++)
        {
            const string& text = lines[i];
            if (StartsWith(text, section + ":") && rank-- == 0)
            {
                std::size_t num = text.find(name + "=");
                if (num == string::npos)
                {
                    return 0;
                }

                num += name.length() + 1;
                const std::size_t num2 = text.find(" ", num);
                if (num2 == string::npos)
                {
                    return 0;
                }

                const string s = text.substr(num, num2 - num);
                intcs result = 0;
                if (TryParseInt(s, result))
                {
                    return result;
                }
                return 0;
            }
        }
        return 0;
    }

    double Worlds::GetDoubleField(
        const string lines[],
        intcs lineCount,
        const string& section,
        intcs rank,
        const string& name)
    {
        for (intcs i = 0; i < lineCount; i++)
        {
            const string& text = lines[i];
            if (StartsWith(text, section + ":") && rank-- == 0)
            {
                std::size_t num = text.find(name + "=");
                if (num == string::npos)
                {
                    return 0.0;
                }

                num += name.length() + 1;
                const std::size_t num2 = text.find(" ", num);
                if (num2 == string::npos)
                {
                    return 0.0;
                }

                const string s = text.substr(num, num2 - num);
                double result = 0.0;
                if (TryParseDouble(s, result))
                {
                    return result;
                }
                return 0.0;
            }
        }
        return 0.0;
    }

    TinyPoint Worlds::GetPointField(
        const string lines[],
        intcs lineCount,
        const string& section,
        intcs rank,
        const string& name)
    {
        for (intcs i = 0; i < lineCount; i++)
        {
            const string& text = lines[i];
            if (StartsWith(text, section + ":") && rank-- == 0)
            {
                std::size_t num = text.find(name + "=");
                if (num == string::npos)
                {
                    return TinyPoint{};
                }

                num += name.length() + 1;
                const std::size_t num2 = text.find(";", num);
                if (num2 == string::npos)
                {
                    return TinyPoint{};
                }

                const std::size_t num3 = text.find(" ", num);
                if (num3 == string::npos)
                {
                    return TinyPoint{};
                }

                const string s1 = text.substr(num, num2 - num);
                const string s2 = text.substr(num2 + 1, num3 - num2 - 1);

                intcs x = 0;
                if (!TryParseInt(s1, x))
                {
                    return TinyPoint{};
                }

                intcs y = 0;
                if (!TryParseInt(s2, y))
                {
                    return TinyPoint{};
                }

                TinyPoint result{};
                result.X = x;
                result.Y = y;
                return result;
            }
        }

        return TinyPoint{};
    }

    std::optional<intcs> Worlds::GetDecorField(
        const string lines[],
        intcs lineCount,
        const string& section,
        intcs x,
        intcs y)
    {
        for (intcs i = 0; i < lineCount; i++)
        {
            string text = lines[i];
            if (StartsWith(text, section + ":"))
            {
                const intcs rowIndex = i + 1 + x;
                if (rowIndex < 0 || rowIndex >= lineCount)
                {
                    return std::nullopt;
                }

                text = lines[rowIndex];
                const std::vector<std::string> parts = Split(text, ',');

                if (y < 0 || y >= static_cast<intcs>(parts.size()))
                {
                    return std::nullopt;
                }

                if (parts[y].empty())
                {
                    return static_cast<intcs>(-1);
                }

                intcs result = 0;
                if (TryParseInt(parts[y], result))
                {
                    return result;
                }

                return std::nullopt;
            }
        }

        return std::nullopt;
    }

    void Worlds::GetDoorsField(
        const string lines[],
        intcs lineCount,
        const string& section,
        intcs doors[],
        intcs doorsSize)
    {
        for (intcs i = 0; i < lineCount; i++)
        {
            const string& text = lines[i];
            if (!StartsWith(text, section + ":"))
            {
                continue;
            }

            const string payload = text.substr(section.length() + 2);
            const std::vector<std::string> parts = Split(payload, ',');

            for (intcs j = 0; j < static_cast<intcs>(parts.size()) && j < doorsSize; j++)
            {
                intcs result = 0;
                if (parts[j].empty())
                {
                    doors[j] = 1;
                }
                else if (TryParseInt(parts[j], result))
                {
                    doors[j] = result;
                }
            }
        }
    }

    void Worlds::WriteClear()
    {
        output.str("");
        output.clear();
    }

    void Worlds::WriteSection(const string& section)
    {
        output << section;
        output << ": ";
    }

    void Worlds::WriteIntArrayField(const string& name, const intcs array[], intcs arraySize)
    {
        output << name;
        output << "=";
        for (intcs i = 0; i < arraySize; i++)
        {
            if (array[i] != 0)
            {
                output << array[i];
            }
            if (i < arraySize - 1)
            {
                output << ",";
            }
        }
        output << " ";
    }

    void Worlds::WriteBoolField(const string& name, bool n)
    {
        output << name;
        output << "=";
        output << (n ? "True" : "False");
        output << " ";
    }

    void Worlds::WriteIntField(const string& name, intcs n)
    {
        output << name;
        output << "=";
        output << n;
        output << " ";
    }

    void Worlds::WriteDoubleField(const string& name, double n)
    {
        std::ostringstream ss;
        ss.imbue(std::locale::classic());
        ss << std::setprecision(15) << n;

        output << name;
        output << "=";
        output << ss.str();
        output << " ";
    }

    void Worlds::WritePointField(const string& name, TinyPoint p)
    {
        output << name;
        output << "=";
        output << p.X;
        output << ";";
        output << p.Y;
        output << " ";
    }

    void Worlds::WriteDecorField(const intcs line[], intcs arraySize)
    {
        for (intcs i = 0; i < arraySize; i++)
        {
            if (line[i] != -1)
            {
                output << line[i];
            }
            if (i < arraySize - 1)
            {
                output << ",";
            }
        }
        output << "\n";
    }

    void Worlds::WriteDoorsField(const intcs doors[], intcs arraySize)
    {
        for (intcs i = 0; i < arraySize; i++)
        {
            if (doors[i] != 1)
            {
                output << doors[i];
            }
            if (i < arraySize - 1)
            {
                output << ",";
            }
        }
        output << "\n";
    }

    void Worlds::WriteEndSection()
    {
        output << "\n";
    }

    string Worlds::GetWriteString()
    {
        return output.str();
    }
}
/**
 * @file GameData.cpp
 * @brief Implements the GameData class: flat byte-array accessors and
 *        initialisation logic for the Speedy Blupi persistent save format.
 *
 * @note Byte-array layout schema
 *
 * The entire save state fits in a single array of TotalLength = 640 bytes.
 * The schema below is the canonical reference; every accessor must stay in sync
 * with it.  Silently changing an offset without updating all accessors will
 * corrupt save files for all platforms.
 *
 * ```
 * Global header  (bytes 0-9, SaveHeaderLength = 10)
 *   [0]  byte  reserved / version tag          default: 1
 *   [1]  byte  reserved                        default: 1
 *   [2]  byte  selectedGamer  (0..2)           default: 0
 *   [3]  byte  sounds         (0=off, 1=on)    default: 1
 *   [4]  byte  jumpRight      (0=left, 1=right)default: 1
 *   [5]  byte  autoZoom       (0=off, 1=on)    default: 1
 *   [6]  byte  accelActive    (0=off, 1=on)    default: 0
 *   [7]  byte  accelSensitivity [0..100]       default: 50  (= 0.50 in [0,1])
 *   [8]  byte  reserved                        default: 0
 *   [9]  byte  reserved                        default: 0
 *
 * Per-gamer block  (repeated 3x, GamerLength = 210 bytes each)
 *   Offset = SaveHeaderLength + gamer * GamerLength
 *   [+0]  byte  nbVies     (lives)             default: 3
 *   [+1]  byte  lastWorld  (1-based)           default: 1
 *   [+2]..[+9]  8 bytes reserved               default: 0
 *   [+10]..[+209]  200 bytes  doors[0..199]    default: all 0
 *                  doors[0..179]   = secondary door states
 *                  doors[180..199] = main door states
 *                  0 = locked, 1 = opened
 * ```
 *
 * TotalLength = 10 + 3 * 210 = 640 bytes.
 */

#include "WindowsPhoneSpeedyBlupi/GameData.hpp"

#include <string>

#include "CNA/Logger.hpp"
#include "System/Math.hpp"
#include "WindowsPhoneSpeedyBlupi/Worlds.hpp"

namespace WindowsPhoneSpeedyBlupi
{
    using log = CNA::Logger;
    intcs GameData::getSelectedGamerProperty() const { return data[2]; }
    void GameData::setSelectedGamerProperty(const intcs v) { data[2] = (bytecs)v; }

    bool GameData::getSoundsProperty() const { return data[3] == 1; }
    void GameData::setSoundsProperty(const bool v) { data[3] = (bytecs)(v ? 1u : 0u); }

    bool GameData::getJumpRightProperty() const { return data[4] == 1; }
    void GameData::setJumpRightProperty(const bool v) { data[4] = (bytecs)(v ? 1u : 0u); }
    bool GameData::getAutoZoomProperty() const { return data[5] == 1; }
    void GameData::setAutoZoomProperty(const bool v) { data[5] = (bytecs)(v ? 1u : 0u); }
    bool GameData::getAccelActiveProperty() const { return data[6] == 1; }
    void GameData::setAccelActiveProperty(const bool v) { data[6] = (bytecs)(v ? 1u : 0u); }

    double GameData::getAccelSensitivityProperty() const { return (double)(int)data[7] / 100.0; }

    void GameData::setAccelSensitivityProperty(double v)
    {
        v = System::Math::Max(v, 0.0);
        v = System::Math::Min(v, 1.0);
        data[7] = (bytecs)(v * 100.0);
    }

    intcs GameData::getNbViesProperty() const { return data[getGamerOffsetProperty()]; }
    void GameData::setNbViesProperty(const intcs v) { data[getGamerOffsetProperty()] = (bytecs)v; }
    intcs GameData::getLastWorldProperty() const { return data[getGamerOffsetProperty() + 1]; }
    void GameData::setLastWorldProperty(const intcs v) { data[getGamerOffsetProperty() + 1] = (bytecs)v; }
    intcs GameData::getGamerOffsetProperty() const { return GetGamerOffset(getSelectedGamerProperty()); }

    GameData::GameData()
    {
        Initialize();
    }

    void GameData::Read()
    {
        const bool loaded = Worlds::ReadGameData(data, TotalLength);
        log::Debug("GameData::Read loaded=" + std::to_string(loaded ? 1 : 0)
            + " selectedGamer=" + std::to_string(getSelectedGamerProperty())
            + " lastWorld=" + std::to_string(getLastWorldProperty()));
    }

    void GameData::Write()
    {
        Worlds::WriteGameData(data, TotalLength);
        log::Debug("GameData::Write saved=1 selectedGamer=" + std::to_string(getSelectedGamerProperty())
            + " lastWorld=" + std::to_string(getLastWorldProperty()));
    }

    void GameData::Reset()
    {
        Initialize(getSelectedGamerProperty());
    }

    void GameData::GetDoors(intcs doors[])
    {
        for (intcs i = 0; i < DoorsLength; i++)
        {
            doors[i] = data[getGamerOffsetProperty() + GamerHeaderLength + i];
        }
    }

    void GameData::SetDoors(const intcs doors[])
    {
        for (intcs i = 0; i < DoorsLength; i++)
        {
            data[getGamerOffsetProperty() + GamerHeaderLength + i] = (bytecs)doors[i];
        }
    }
    void GameData::GetGamerInfo(intcs gamer, intcs& nbVies, intcs& mainDoors, intcs& secondaryDoors)
    {
        nbVies = data[GetGamerOffset(gamer)];
        secondaryDoors = 0;
        for (intcs i = 0; i < 180; i++)
        {
            if (data[GetGamerOffset(gamer) + GamerHeaderLength + i] == 1)
            {
                secondaryDoors++;
            }
        }
        mainDoors = 0;
        for (intcs j = 180; j < 200; j++)
        {
            if (data[GetGamerOffset(gamer) + GamerHeaderLength + j] == 1)
            {
                mainDoors++;
            }
        }
    }


    void GameData::Initialize()
    {
        data[0] = 1;
        data[1] = 1;
        data[2] = 0;
        data[3] = 1;
        data[4] = 1;
        data[5] = 1;
        data[6] = 0;
        data[7] = 50;
        setSelectedGamerProperty(0);
        for (intcs i = 0; i < MaxGamer; i++)
        {
            Initialize(i);
        }
    }

    void GameData::Initialize(intcs gamer)
    {
        data[GetGamerOffset(gamer)] = 3;
        data[GetGamerOffset(gamer) + 1] = 1;
        for (intcs i = 0; i < DoorsLength; i++)
        {
            data[GetGamerOffset(gamer) + GamerHeaderLength + i] = 0;
        }
    }

    intcs GameData::GetGamerOffset(intcs gamer)
    {
        return SaveHeaderLength + GamerLength * gamer;
    }
}

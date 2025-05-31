// WindowsPhoneSpeedyBlupi, Version=1.0.0.5, Culture=neutral, PublicKeyToken=6db12cd62dbec439
// WindowsPhoneSpeedyBlupi.GameData


#include "WindowsPhoneSpeedyBlupi/GameData.h"

namespace WindowsPhoneSpeedyBlupi
{

    byte GameData::getSelectedGamer() const { return data[2]; } void GameData::setSelectedGamer(const byte& v) { data[2] = v; }

    bool GameData::getSounds() const { return data[3] == 1; } void GameData::setSounds(const bool& v) {data[3] = (byte)(v ? 1u : 0u); }
    bool GameData::getJumpRight() const { return data[4] == 1; } void GameData::setJumpRight(const bool& v) {data[4] = (byte)(v ? 1u : 0u); }
    bool GameData::getAutoZoom() const { return data[5] == 1; } void GameData::setAutoZoom(const bool& v) {data[5] = (byte)(v ? 1u : 0u); }
    bool GameData::getAccelActive() const { return data[6] == 1; } void GameData::setAccelActive(const bool& v) {data[6] = (byte)(v ? 1u : 0u); }

    double GameData::getAccelSensitivity() const { return (double)(int)data[7] / 100.0; }
    void GameData::setAccelSensitivity(double v) {
        v = std::max(v, 0.0),
        v = std::min(v, 1.0);
        data[7] = (byte)(v * 100.0);}
    int GameData::getNbVies() const { return data[getGamerOffset()]; } void GameData::setNbVies(const int& v) {data[getGamerOffset()] = (byte)v;}
    int GameData::getLastWorld() const { return data[getGamerOffset() + 1]; } void GameData::setLastWorld(const int& v) {data[getGamerOffset() + 1] = (byte)v;}
    int GameData::getGamerOffset() const { return GetGamerOffset(getSelectedGamer()); }

    GameData::GameData() : data{}

    {

        Initialize();
    }

            void GameData::Read()
        {
            Worlds::ReadGameData(data, TotalLength);
        }

        void GameData::Write()
        {
            Worlds::WriteGameData(data, TotalLength);
        }

        void GameData::Reset()
        {
            Initialize(getSelectedGamer());
        }

        void GameData::GetDoors(int doors[])
        {
            for (int i = 0; i < DoorsLength; i++)
            {
                doors[i] = data[getSelectedGamer() + 10 + i];
            }
        }

        void GameData::SetDoors(const int doors[])
        {
            for (int i = 0; i < DoorsLength; i++)
            {
                data[getSelectedGamer() + 10 + i] = (byte)doors[i];
            }
        }

        void GameData::GetGamerInfo(int gamer, int& nbVies, int& mainDoors, int& secondaryDoors)
        {
            nbVies = data[GetGamerOffset(gamer)];
            secondaryDoors = 0;
            for (int i = 0; i < 180; i++)
            {
                if (data[GetGamerOffset(gamer) + 10 + i] == 1)
                {
                    secondaryDoors++;
                }
            }
            mainDoors = 0;
            for (int j = 180; j < 200; j++)
            {
                if (data[GetGamerOffset(gamer) + 10 + j] == 1)
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
            setSelectedGamer(0);
            for (int i = 0; i < MaxGamer; i++)
            {
                Initialize(i);
            }
        }

        void GameData::Initialize(int gamer)
        {
            data[GetGamerOffset(gamer)] = 3;
            data[GetGamerOffset(gamer) + 1] = 1;
            for (int i = 0; i < DoorsLength; i++)
            {
                data[GetGamerOffset(gamer) + 10 + i] = 0;
            }
        }

        int GameData::GetGamerOffset(int gamer)
        {
            return HeaderLength + GamerLength * gamer;
        }
}

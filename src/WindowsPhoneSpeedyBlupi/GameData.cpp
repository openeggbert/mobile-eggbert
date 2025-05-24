// WindowsPhoneSpeedyBlupi, Version=1.0.0.5, Culture=neutral, PublicKeyToken=6db12cd62dbec439
// WindowsPhoneSpeedyBlupi.GameData


#include "NeoSdk/Property.h"
#include "WindowsPhoneSpeedyBlupi/GameData.h"

namespace WindowsPhoneSpeedyBlupi
{



    GameData::GameData() : data{},

    SelectedGamer( [this]() { return data[2]; } , [this](byte value) {data[2] = value; }),
    Sounds( [this]() { return data[3] == 1; } , [this](bool value) {data[3] = (byte)(value ? 1u : 0u); }),
    JumpRight( [this]() { return data[4] == 1; } , [this](bool value) {data[4] = (byte)(value ? 1u : 0u);}),
    AutoZoom( [this]() { return data[5] == 1; } , [this](bool value) {data[5] = (byte)(value ? 1u : 0u);}),
    AccelActive( [this]() { return data[6] == 1; } , [this](bool value) {data[6] = (byte)(value ? 1u : 0u);}),
    AccelSensitivity( [this]() { return (double)(int)data[7] / 100.0; } , [this](double value) {value = std::max(value, 0.0),
        value = std::min(value, 1.0);
        data[7] = (byte)(value * 100.0);}),
    NbVies( [this]() { return data[GamerOffset]; } , [this](bool value) {data[GamerOffset] = (byte)value;}),
    LastWorld( [this]() { return data[GamerOffset + 1]; } , [this](int value) {data[GamerOffset + 1] = (byte)value;}),
    GamerOffset ([this]() { return GetGamerOffset(SelectedGamer); } )
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
            Initialize(SelectedGamer);
        }

        void GameData::GetDoors(int doors[])
        {
            for (int i = 0; i < DoorsLength; i++)
            {
                doors[i] = data[GamerOffset + 10 + i];
            }
        }

        void GameData::SetDoors(const int doors[])
        {
            for (int i = 0; i < DoorsLength; i++)
            {
                data[GamerOffset + 10 + i] = (byte)doors[i];
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
            SelectedGamer = 0;
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

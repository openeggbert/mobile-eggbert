// WindowsPhoneSpeedyBlupi, Version=1.0.0.5, Culture=neutral, PublicKeyToken=6db12cd62dbec439
// WindowsPhoneSpeedyBlupi.GameData


#include "WindowsPhoneSpeedyBlupi/GameData.hpp"

namespace WindowsPhoneSpeedyBlupi
{

    bytecs GameData::getSelectedGamerProperty() const { return data[2]; } void GameData::setSelectedGamerProperty(const bytecs& v) { data[2] = v; }

    bool GameData::getSoundsProperty() const { return data[3] == 1; } void GameData::setSoundsProperty(const bool& v) {data[3] = (bytecs)(v ? 1u : 0u); }
    bool GameData::getJumpRightProperty() const { return data[4] == 1; } void GameData::setJumpRightProperty(const bool& v) {data[4] = (bytecs)(v ? 1u : 0u); }
    bool GameData::getAutoZoomProperty() const { return data[5] == 1; } void GameData::setAutoZoomProperty(const bool& v) {data[5] = (bytecs)(v ? 1u : 0u); }
    bool GameData::getAccelActiveProperty() const { return data[6] == 1; } void GameData::setAccelActiveProperty(const bool& v) {data[6] = (bytecs)(v ? 1u : 0u); }

    double GameData::getAccelSensitivityProperty() const { return (double)(int)data[7] / 100.0; }
    void GameData::setAccelSensitivityProperty(double v) {
        v = std::max(v, 0.0),
        v = std::min(v, 1.0);
        data[7] = (bytecs)(v * 100.0);}
    int GameData::getNbViesProperty() const { return data[getGamerOffsetProperty()]; } void GameData::setNbViesProperty(const int& v) {data[getGamerOffsetProperty()] = (bytecs)v;}
    int GameData::getLastWorldProperty() const { return data[getGamerOffsetProperty() + 1]; } void GameData::setLastWorldProperty(const int& v) {data[getGamerOffsetProperty() + 1] = (bytecs)v;}
    int GameData::getGamerOffsetProperty() const { return GetGamerOffset(getSelectedGamerProperty()); }

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
            Initialize(getSelectedGamerProperty());
        }

        void GameData::GetDoors(int doors[])
        {
            for (int i = 0; i < DoorsLength; i++)
            {
                doors[i] = data[getSelectedGamerProperty() + 10 + i];
            }
        }

        void GameData::SetDoors(const int doors[])
        {
            for (int i = 0; i < DoorsLength; i++)
            {
                data[getSelectedGamerProperty() + 10 + i] = (bytecs)doors[i];
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
            setSelectedGamerProperty(0);
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

// WindowsPhoneSpeedyBlupi, Version=1.0.0.5, Culture=neutral, PublicKeyToken=6db12cd62dbec439
// WindowsPhoneSpeedyBlupi.Def

//using Microsoft.Xna.Framework.Input;
//using static WindowsPhoneSpeedyBlupi.Def;
#ifndef GAMEDATA_H
#define GAMEDATA_H
#include "Worlds.hpp"
#include "CNA/CnaHelper.hpp"


namespace WindowsPhoneSpeedyBlupi
{

    using ushort = unsigned short;
    using CNA::bytecs;
    class GameData
    {
    private:
        static constexpr bytecs HeaderLength = 10;

        static constexpr bytecs DoorsLength = 200;

        static constexpr bytecs GamerLength = 10 + DoorsLength;

        static constexpr bytecs MaxGamer = 3;

        static constexpr ushort TotalLength = HeaderLength + GamerLength * MaxGamer;

        bytecs data[TotalLength];

    public:
    public: [[nodiscard]] bytecs getSelectedGamerProperty() const; public: void setSelectedGamerProperty(const bytecs& v);
    public: [[nodiscard]] bool getSoundsProperty() const; public: void setSoundsProperty(const bool& v);
    public: [[nodiscard]] bool getJumpRightProperty() const; public: void setJumpRightProperty(const bool& v);
    public: [[nodiscard]] bool getAutoZoomProperty() const; public: void setAutoZoomProperty(const bool& v);
    public: [[nodiscard]] bool getAccelActiveProperty() const; public: void setAccelActiveProperty(const bool& v);
    public: [[nodiscard]] double getAccelSensitivityProperty() const; public: void setAccelSensitivityProperty(double v);
    public: [[nodiscard]] int getNbViesProperty() const; public: void setNbViesProperty(const int& v);
    public: [[nodiscard]] int getLastWorldProperty() const; public: void setLastWorldProperty(const int& v);
    public: [[nodiscard]] int getGamerOffsetProperty() const; public: void setGamerOffset(const int& v);


        GameData();

        void Read();

        void Write();

        void Reset();

        void GetDoors(int doors[]);

        void SetDoors(const int doors[]);

        void GetGamerInfo(int gamer, int& nbVies, int& mainDoors, int& secondaryDoors);

    private:
        void Initialize();

        void Initialize(int gamer);

        static int GetGamerOffset(int gamer);
    };


}

#endif // GAMEDATA_H

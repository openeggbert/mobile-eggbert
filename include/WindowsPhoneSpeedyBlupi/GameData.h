// WindowsPhoneSpeedyBlupi, Version=1.0.0.5, Culture=neutral, PublicKeyToken=6db12cd62dbec439
// WindowsPhoneSpeedyBlupi.Def

//using Microsoft.Xna.Framework.Input;
//using static WindowsPhoneSpeedyBlupi.Def;
#ifndef GAMEDATA_H
#define GAMEDATA_H
#include "Worlds.h"


namespace WindowsPhoneSpeedyBlupi
{

    using ushort = unsigned short;
    using CNA::byte;
    class GameData
    {
    private:
        static constexpr byte HeaderLength = 10;

        static constexpr byte DoorsLength = 200;

        static constexpr byte GamerLength = 10 + DoorsLength;

        static constexpr byte MaxGamer = 3;

        static constexpr ushort TotalLength = HeaderLength + GamerLength * MaxGamer;

        byte data[TotalLength];

    public:
    public: [[nodiscard]] byte getSelectedGamer() const; public: void setSelectedGamer(const byte& v);
    public: [[nodiscard]] bool getSounds() const; public: void setSounds(const bool& v);
    public: [[nodiscard]] bool getJumpRight() const; public: void setJumpRight(const bool& v);
    public: [[nodiscard]] bool getAutoZoom() const; public: void setAutoZoom(const bool& v);
    public: [[nodiscard]] bool getAccelActive() const; public: void setAccelActive(const bool& v);
    public: [[nodiscard]] double getAccelSensitivity() const; public: void setAccelSensitivity(double v);
    public: [[nodiscard]] int getNbVies() const; public: void setNbVies(const int& v);
    public: [[nodiscard]] int getLastWorld() const; public: void setLastWorld(const int& v);
    public: [[nodiscard]] int getGamerOffset() const; public: void setGamerOffset(const int& v);


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

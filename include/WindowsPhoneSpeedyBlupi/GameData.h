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
    using byte = unsigned char;
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

        NeoSdk::Property<byte> SelectedGamer;
        NeoSdk::Property<bool> Sounds;
        NeoSdk::Property<bool> JumpRight;
        NeoSdk::Property<bool> AutoZoom;
        NeoSdk::Property<bool> AccelActive;
        NeoSdk::Property<double> AccelSensitivity;
        NeoSdk::Property<int> NbVies;
        NeoSdk::Property<int> LastWorld;
        NeoSdk::Property<int> GamerOffset;

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

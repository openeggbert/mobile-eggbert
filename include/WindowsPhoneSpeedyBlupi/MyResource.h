// WindowsPhoneSpeedyBlupi, Version=1.0.0.5, Culture=neutral, PublicKeyToken=6db12cd62dbec439
// WindowsPhoneSpeedyBlupi.MyResource
//using System.Collections.Generic;
//using System.Globalization;
#ifndef MYRESOURCE_H
#define MYRESOURCE_H
#include <string>
#include <unordered_map>

namespace WindowsPhoneSpeedyBlupi {
    using std::string;
    using ushort = unsigned short;
    
    class MyResource
    {
    public:
        static const ushort TX_BUTTON_PLAY;

        static const ushort TX_BUTTON_MENU;

        static const ushort TX_BUTTON_BACK;

        static const ushort TX_BUTTON_RESTART;

        static const ushort TX_BUTTON_CONTINUE;

        static const ushort TX_BUTTON_BUY;

        static const ushort TX_BUTTON_SETUP;

        static const ushort TX_BUTTON_SETUP_SOUNDS;

        static const ushort TX_BUTTON_SETUP_JUMP;

        static const ushort TX_BUTTON_SETUP_ZOOM;

        static const ushort TX_BUTTON_SETUP_ACCEL;

        static const ushort TX_BUTTON_SETUP_RESET;

        static const ushort TX_BUTTON_RANKING;

        static const ushort TX_GAMER_TITLE;

        static const ushort TX_GAMER_MDOORS;

        static const ushort TX_GAMER_SDOORS;

        static const ushort TX_GAMER_LIFES;

        static const ushort TX_TRIAL1;

        static const ushort TX_TRIAL2;

        static const ushort TX_TRIAL3;

        static const ushort TX_TRIAL4;

        static const ushort TX_TRIAL5;

        static const ushort TX_TRIAL6;

        static const ushort TX_TRAINING101;

        static const ushort TX_TRAINING102;

        static const ushort TX_TRAINING103;

        static const ushort TX_TRAINING104;

        static const ushort TX_TRAINING105;

        static const ushort TX_TRAINING106;

        static const ushort TX_TRAINING107;

        static const ushort TX_TRAINING108;

        static const ushort TX_TRAINING109;

        static const ushort TX_TRAINING110;

        static const ushort TX_TRAINING111;

        static const ushort TX_TRAINING112;

        static const ushort TX_TRAINING113;

        static const ushort TX_TRAINING114;

        static const ushort TX_TRAINING115;

        static const ushort TX_TRAINING116;

        static const ushort TX_TRAINING117;

        static const ushort TX_TRAINING118;

        static const ushort TX_TRAINING119;

        static const ushort TX_TRAINING120;

        static const ushort TX_TRAINING121;

        static const ushort TX_TRAINING122;

        static const ushort TX_TRAINING123;

        static const ushort TX_TRAINING201;

        static const ushort TX_TRAINING202;

        static const ushort TX_TRAINING203;

        static const ushort TX_TRAINING204;

        static const ushort TX_TRAINING205;

        static const ushort TX_TRAINING206;

        static const ushort TX_TRAINING207;

        static const ushort TX_TRAINING208;

        static const ushort TX_TRAINING209;

        static const ushort TX_TRAINING210;

        static const ushort TX_TRAINING301;

        static const ushort TX_TRAINING302;

        static const ushort TX_TRAINING303;

        static const ushort TX_TRAINING304;

        static const ushort TX_TRAINING305;

        static const ushort TX_TRAINING306;

        static const ushort TX_TRAINING307;

        static const ushort TX_TRAINING308;

        static const ushort TX_TRAINING309;

        static const ushort TX_TRAINING310;

        static const ushort TX_TRAINING311;

        static const ushort TX_TRAINING401;

        static const ushort TX_TRAINING402;

        static const ushort TX_TRAINING403;

        static const ushort TX_TRAINING404;

        static const ushort TX_TRAINING405;

        static const ushort TX_TRAINING406;

        static const ushort TX_TRAINING407;

        static const ushort TX_TRAINING408;

        static const ushort TX_TRAINING409;

        static const ushort TX_TRAINING410;

        static const ushort TX_TRAINING101a;

        static const ushort TX_TRAINING102a;

        static const ushort TX_TRAINING103a;

        static const ushort TX_TRAINING104a;

        static const ushort TX_TRAINING105a;

        static const ushort TX_TRAINING106a;

        static const ushort TX_TRAINING107a;

        static const ushort TX_TRAINING108a;

        static const ushort TX_TRAINING109a;

        static const ushort TX_TRAINING110a;

        static const ushort TX_TRAINING111a;

        static const ushort TX_TRAINING112a;

        static const ushort TX_TRAINING113a;

        static const ushort TX_TRAINING114a;

        static const ushort TX_TRAINING115a;

        static const ushort TX_TRAINING116a;

        static const ushort TX_TRAINING117a;

        static const ushort TX_TRAINING118a;

        static const ushort TX_TRAINING119a;

        static const ushort TX_TRAINING120a;

        static const ushort TX_TRAINING121a;

        static const ushort TX_TRAINING122a;

        static const ushort TX_TRAINING123a;

        static const ushort TX_TRAINING201a;

        static const ushort TX_TRAINING202a;

        static const ushort TX_TRAINING203a;

        static const ushort TX_TRAINING204a;

        static const ushort TX_TRAINING205a;

        static const ushort TX_TRAINING206a;

        static const ushort TX_TRAINING207a;

        static const ushort TX_TRAINING208a;

        static const ushort TX_TRAINING209a;

        static const ushort TX_TRAINING210a;

        static const ushort TX_TRAINING301a;

        static const ushort TX_TRAINING302a;

        static const ushort TX_TRAINING303a;

        static const ushort TX_TRAINING304a;

        static const ushort TX_TRAINING305a;

        static const ushort TX_TRAINING306a;

        static const ushort TX_TRAINING307a;

        static const ushort TX_TRAINING308a;

        static const ushort TX_TRAINING309a;

        static const ushort TX_TRAINING310a;

        static const ushort TX_TRAINING311a;

        static const ushort TX_TRAINING401a;

        static const ushort TX_TRAINING402a;

        static const ushort TX_TRAINING403a;

        static const ushort TX_TRAINING404a;

        static const ushort TX_TRAINING405a;

        static const ushort TX_TRAINING406a;

        static const ushort TX_TRAINING407a;

        static const ushort TX_TRAINING408a;

        static const ushort TX_TRAINING409a;

        static const ushort TX_TRAINING410a;

    private: static std::unordered_map<ushort, std::string> resources;

    private: static const constexpr char* DEFAULT_VALUE = "???";

    public: static string LoadString(ushort res);

    private:
        static const bool initialized;

    private: static void Init();

    private: static void InitializeFR();
    private: static void InitializeEN();
    private: static void InitializeDE();
    };
}
#endif // MYRESOURCE_H

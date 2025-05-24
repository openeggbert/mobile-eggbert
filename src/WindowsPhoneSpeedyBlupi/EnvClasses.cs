// WindowsPhoneSpeedyBlupi, Version=1.0.0.5, Culture=neutral, PublicKeyToken=6db12cd62dbec439
// WindowsPhoneSpeedyBlupi.Def

using static WindowsPhoneSpeedyBlupi.EnvClasses;

namespace WindowsPhoneSpeedyBlupi
{

    public static class EnvClasses
    {
        public enum Platform
        {
            Desktop,
            Android,
            iOS,
            Web
        }

    
    }

    public static class Extensions
    {

        //
        public static bool isDesktop(this Platform platform)
        {
            return platform == Platform.Desktop;
        }
        public static bool isAndroid(this Platform platform)
        {
            return platform == Platform.Android;
        }
        public static bool isIOS(this Platform platform)
        {
            return platform == Platform.iOS;
        }
        public static bool isWeb(this Platform platform)
        {
            return platform == Platform.Web;
        }
        public static bool isNotDesktop(this Platform platform)
        {
            return platform != Platform.Desktop;
        }
        public static bool isNotAndroid(this Platform platform)
        {
            return platform != Platform.Android;
        }
        public static bool isNotIOS(this Platform platform)
        {
            return platform != Platform.iOS;
        }
        public static bool isNotWeb(this Platform platform)
        {
            return platform != Platform.Web;
        }
    }
}
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

        public enum Impl
        {
            MonoGame,
            FNA,
            KNI,
            JXNA,
            JSXNA
        }

        public enum ProgrammingLanguage
        {
            CSharp,
            Java,
            JavaScript
        }

    
    }

    public static class Extensions
    {
        public static ProgrammingLanguage getProgrammingLanguage(this Impl impl)
        {
            switch (impl)
            {
                case Impl.MonoGame: return ProgrammingLanguage.CSharp;
                case Impl.FNA: return ProgrammingLanguage.CSharp;
                case Impl.KNI: return ProgrammingLanguage.CSharp;
                case Impl.JXNA: return ProgrammingLanguage.Java;
                case Impl.JSXNA: return ProgrammingLanguage.JavaScript;
                default: throw new System.Exception("Unsupported Impl: " + impl);
            }
        }
        public static bool isMonoGame(this Impl impl)
        {
            return impl == Impl.MonoGame;
        }
        public static bool isFNA(this Impl impl)
        {
            return impl == Impl.FNA;
        }

        public static bool isKNI(this Impl impl)
        {
            return  impl == Impl.KNI;
        }

        public static bool isJXNA(this Impl impl)
        {
            return impl == Impl.JXNA;
        }

        public static bool isJSXNA(this Impl impl)
        {
            return impl == Impl.JSXNA;
        }
        public static bool isNotMonoGame(this Impl impl)
        {
            return impl != Impl.MonoGame;
        }
        public static bool isNotFNA(this Impl impl)
        {
            return impl != Impl.FNA;
        }

        public static bool isNotKNI(this Impl impl)
        {
            return impl != Impl.KNI;
        }

        public static bool isNotJXNA(this Impl impl)
        {
            return impl != Impl.JXNA;
        }

        public static bool isNotJSXNA(this Impl impl)
        {
            return impl != Impl.JSXNA;
        }
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
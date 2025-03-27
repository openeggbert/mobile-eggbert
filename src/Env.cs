using System;
using System.Diagnostics;
using static WindowsPhoneSpeedyBlupi.EnvClasses;

namespace WindowsPhoneSpeedyBlupi
{
    public static class Env
    {
        public static Platform PLATFORM { get; private set; }
        public static Impl IMPL { get; private set; }
        public static bool INITIALIZED { get; private set; }

        public static void init(Impl impl, Platform platformIn)
        {
            if(INITIALIZED)
            {
                throw new Exception("Env was already initialized. Cannot call the init method again.");
            }
            IMPL = impl;
            PLATFORM = platformIn;
            INITIALIZED = true;
        }
    }
}

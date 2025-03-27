using System;
using System.Diagnostics;

namespace WindowsPhoneSpeedyBlupi
{
    public static class DDebug
    {
        private static bool detailedDebugging = false;
        private static bool DetailedDebugging
        {
            get { return detailedDebugging; }
            set { detailedDebugging = value; }
        }
        public static void WriteLine(String msg)
        {
            if (detailedDebugging)
            {
                Debug.WriteLine(msg);
            }
        }
    }
}

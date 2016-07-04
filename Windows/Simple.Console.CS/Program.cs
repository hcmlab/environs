using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace environs.Apps
{
    class Program
    {
        static void Main(string[] args)
        {
            SimpleConsole app = new SimpleConsole();

            if (!app.Init())
                return;

            app.Run();

            app.Stop();
        }
    }
}

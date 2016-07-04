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
            EchoBot app = new EchoBot();

            if (!app.Init())
                return;

            app.Run();

            app.Stop();
        }
    }
}

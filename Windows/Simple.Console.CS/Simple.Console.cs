using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using environs;

namespace environs.Apps
{
    class SimpleConsole
    {
        bool isRunning = true;
        Environs env = null;
        DeviceList deviceList = null;
        int currentListID = -2;

        String appNameCur = "ChatApp";
        String areaNameCur = "Environs";

        public bool Init()
        {
            if (env == null)
            {
                //env = Environs.CreateInstance("Simple.Console", "Environs");
                env = Environs.CreateInstance(appNameCur, areaNameCur);
                if (env == null)
                    return false;
                env.AddObserverForStatus(OnStatus);
                env.Start();
            }

            if (deviceList == null)
            {
                deviceList = env.CreateDeviceList(DeviceClass.All);
                if (deviceList == null)
                    return false;

                deviceList.AddObserver(ListObserver);
            }
            return true;
        }

        public void Stop()
        {
            if (deviceList != null)
            {
                deviceList.RemoveObserver(ListObserver);
                deviceList = null;
            }

            if (env != null)
            {
                env.RemoveObserverForStatus(OnStatus);

                env.SetUseLogToStdout(true);

                env.async = Call.Wait;
                env.Stop();
                env = null;
            }
        }

        void ListObserver(List<DeviceInstance> vanished, List<DeviceInstance> appeared)
        {
            if (vanished != null)
            {
                foreach (DeviceInstance device in vanished)
                {
                    if (device == null)
                        continue;

                    device.RemoveObserverForMessages(OnMessage);
                    device.RemoveObserver(OnDeviceChanged);
                }
            }

            if (appeared != null)
            {
                foreach (DeviceInstance device in appeared)
                {
                    if (device == null)
                        continue;

                    if (device.appContext0 == 0)
                    {
                        device.appContext0 = listIDs++;
                        device.AddObserver(OnDeviceChanged);
                        device.AddObserverForMessages(OnMessage);
                    }
                }
            }
        }


        private void OnDeviceChanged(DeviceInstance device, DeviceInfoFlag notify)
        {
            if (device == null)
                return;

            if (currentListID != -1 && device.appContext0 != currentListID)
                return;
            
            if (notify == DeviceInfoFlag.ConnectProgress)
            {
                if (!device.isConnected)
                    Console.Write(". ");
            }
            else if ((notify & DeviceInfoFlag.IsConnected) == DeviceInfoFlag.IsConnected)
            {
                if (device.isConnected)
                {
                    Console.WriteLine("\n  Device [ " + device.appContext0.ToString().PadLeft(4) + " ] Connected.");
                }
                else {
                    Console.WriteLine("\n  Device [ " + device.appContext0.ToString().PadLeft(4) + " ] Disconnected.");
                }
                Console.Write("> ");
            }
        }


        private void OnStatus(Status status)
        {
            String str;

            switch (status)
            {
                case Status.Disposed:
                    str = "Disposed";
                    break;
                case Status.Uninitialized:
                    str = "Uninitialized";
                    break;
                case Status.Disposing:
                    str = "Disposing";
                    break;
                case Status.Initializing:
                    str = "Initializing";
                    break;
                case Status.Initialized:
                    str = "Initialized";
                    break;
                case Status.Stopped:
                    str = "Stopped";
                    break;
                case Status.StopInProgress:
                    str = "StopInProgress";
                    break;
                case Status.Stopping:
                    str = "Stopping";
                    break;
                case Status.Starting:
                    str = "Starting";
                    break;
                case Status.Started:
                    str = "Started";
                    break;
                case Status.Connected:
                    str = "Connected";
                    break;
                default:
                    str = "Unknown";
                    break;
            }

            Console.WriteLine("Environs: " + str);
            Console.Write("> ");
        }


        public void OnMessage(MessageInstance msg, MessageInfoFlag changedFlags)
        {
            if (msg == null || msg.disposed)
                return;

            String text = msg.text;

            if (text == null || text.Length <= 0 || text[0] == '$')
                return;

            DeviceInstance device = msg.device;
            if (device == null)
                return;

            Console.WriteLine("  Message [ " + device.appContext0.ToString().PadLeft(4) + " ]: " + text);
        }


        public static void PrintSmallHelp()
        {
            Console.WriteLine("  Press ESC to quit; h for help.");
        }

        public static void PrintHelp()
        {
            Console.Write("-------------------------------------------------------\n");
            Console.Write("  h[elp]               - print help\n");
            Console.Write("  l[log]               - toggle logging to file\n");
            Console.Write("  o[utput]             - toggle logging to output (std)\n");
            Console.Write("  p[rint] [page numer] - print available devices\n");
            Console.Write("  q[uit]               - quit application\n\n");

            Console.Write("  c[onnect] id\n");
            Console.Write("  d[isconnect] id\n");
            Console.Write("  sendm id message\n");
            Console.Write("  set env appName areaName\n");
            Console.Write("-------------------------------------------------------\n");
        }


        int listIDs = 1;
        static String lastLine = null;

        public void Run()
        {
            PrintSmallHelp();
            Console.WriteLine("-------------------------------------------------------");

            do
            {
                Console.Write("> ");

                lastLine = Console.ReadLine();
                if (lastLine == null)
                    break;

                // Remove prepending spaces
                lastLine = lastLine.Trim();
                int c = 0;

                if (lastLine.Length <= 0)
                {
                    PrintSmallHelp();
                    continue;
                }

                if (lastLine.Length <= 1 || lastLine[1] == ' ')
                    // One character command detected
                    c = lastLine[0];

                if (c == 0)
                {
                    HandleLine(lastLine);
                }

                else if (c == 27 || c == 'q')
                {
                    break;
                }

                else if (c == 'c')
                {
                    Connect(true, lastLine);
                }
                else if (c == 'd')
                {
                    Connect(false, lastLine);
                }
                else if (c == 'h')
                {
                    PrintHelp();
                }
                else if (c == 'l')
                {
                    env.SetUseLogFile(!env.GetUseLogFile());
                }
                else if (c == 'o')
                {
                    env.SetUseLogToStdout(!env.GetUseLogToStdout());
                }
                else if (c == 'p')
                {
                    PrintDevices(lastLine);
                }
            }
            while (isRunning);
        }


        void PrintDevices(String line)
        {
            int pageID = 0;

            String[] tokens = line.Split(new char[] { ' ' }, 3);
            if (tokens.Length >= 2)
            {
                int.TryParse(tokens[1], out pageID);
            }

            Console.WriteLine("  Devices:");
            Console.WriteLine("----------------------------------------------------------------");

            Collection<DeviceInstance> devices = deviceList.GetDevices();
            if (devices != null)
            {
                int i = 0;
                if (pageID > 0)
                    i = (pageID - 1) * 10;

                for (; i < (int)devices.Count; ++i)
                {
                    DeviceInstance device = devices[i];
                    if (device != null)
                    {
                        Console.WriteLine("  [ " + device.appContext0.ToString().PadLeft(4) + " ] " + device.toString() + "\n");
                    }
                }
            }
            Console.WriteLine("----------------------------------------------------------------");
        }


        void HandleLine(String line)
        {
            if (line == null || line[0] == '\n')
            {
                Console.Write("\n");
                PrintSmallHelp();
            }
            else if (line.StartsWith("connect"))
            {
                Connect(true, line);
            }
            else if (line.StartsWith("disconnect"))
            {
                Connect(false, line);
            }
            else if (line.StartsWith("help"))
            {
                PrintHelp();
            }
            else if (line.StartsWith("log"))
            {
                env.SetUseLogFile(!env.GetUseLogFile());
            }
            else if (line.StartsWith("output"))
            {
                env.SetUseLogToStdout(!env.GetUseLogToStdout());
            }
            else if (line.StartsWith("print"))
            {
                PrintDevices(line);
            }
            else if (line.StartsWith("quit"))
            {
                isRunning = false;
            }
            else if (line.StartsWith("sendm"))
            {
                SendMessage(line);
            }
            else if (line.StartsWith("set"))
            {
                HandleSet(line.Substring(3).Trim());
            }
        }


        void HandleSet(String line)
        {
            if (line == null)
                return;

            if (line.StartsWith("env"))
            {
                HandleSetEnv(line.Substring(3).Trim());
            }
        }


        void HandleSetEnv(String line)
        {
            if (line == null)
                return;

            String[] tokens = line.Split(' ');
            if (tokens.Length < 2)
            {
                Console.WriteLine( "App and Area name are required arguments." );
                return;
            }

            Stop();

            appNameCur = tokens[0].Trim();
            areaNameCur = tokens[1].Trim();
            
            Init();
        }


        DeviceInstance GetDevice(int listID)
        {
            Collection<DeviceInstance> devices = deviceList.GetDevices();
            if (devices == null)
                return null;

            foreach(DeviceInstance device in devices)
            {
                if (device == null)
                    continue;

                if (device.appContext0 == listID)
                    return device;
            }
            return null;
        }


        void SendMessage(String line)
        {
            String [] tokens = line.Split(new char[] { ' ' }, 3);
            if (tokens.Length < 3)
            {
                Console.WriteLine("  Invalid msg: " + line);
                return;
            }

            int listID = 0;
            if (!int.TryParse(tokens[1], out listID))
            {
                Console.WriteLine("  Invalid list id: " + listID);
                return;
            }

            DeviceInstance device = GetDevice(listID);
            if (device == null)
            {
                Console.WriteLine("  List id not found: " + listID);
                return;
            }

            device.SendMessage(tokens[2]);
        }


        void Connect(bool connect, String line)
        {
            String[] tokens = line.Split(new char[] { ' ' }, 3);
            if (tokens.Length < 2)
                return;

            if (!connect && tokens[1].Equals("a"))
            {
                Collection<DeviceInstance> devices = deviceList.GetDevices();
                if (devices == null)
                    return;

                currentListID = -1;

                foreach (DeviceInstance dev in devices)
                {
                    if (dev == null)
                        continue;

                    if (dev.isConnected)
                        dev.Disconnect(Call.Wait);
                }

                currentListID = -2;
                return;
            }

            int listID = 0;
            if (!int.TryParse(tokens[1], out listID))
            {
                Console.WriteLine("  Invalid list id: " + line);
                return;
            }

            DeviceInstance device = GetDevice(listID);
            if (device == null)
            {
                Console.WriteLine("  List id not found: " + listID);
                return;
            }

            currentListID = listID;

            if (connect)
                device.Connect();
            else
                device.Disconnect();
        }
    }
}

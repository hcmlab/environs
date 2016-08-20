#if DEBUG
#define TEST1
#define TESTCONNECT
#define TESTCONNECTENV
#endif
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Threading;


namespace environs.Apps
{
    class EchoBot
    {
        bool isRunning = true;
        Environs env = null;
        DeviceList deviceList = null;

        bool resetAuth = false;
#if TEST1
        String appNameCur = "ChatApp";
#else
        String appNameCur = "ChatApp";
#endif
        String areaNameCur = "Environs";

        public bool Init()
        {
            if (env == null)
            {
                env = Environs.CreateInstance(appNameCur, areaNameCur);
                if (env == null)
                    return false;

                if (resetAuth)
                {
                    // Enable/Disable anonymous authentication to clear auth tokens
                    env.SetUseMediatorAnonymousLogon(true);
                    env.SetUseMediatorAnonymousLogon(false);
                    resetAuth = false;
                }

                env.AddObserverForStatus(OnStatus);

                // Check and query for logon credentials
                env.QueryMediatorLogonCommandLine();

                // Dont apply a filter to the list
                env.SetMediatorFilterLevel(MediatorFilter.None);

                env.Start();
            }

            if (deviceList == null)
            {
                deviceList = env.CreateDeviceList(DeviceClass.All);
                if (deviceList == null)
                    return false;

                deviceList.AddObserver(ListObserver);
            }
#if TEST1
            for (int i = 0; i < testListenerCount; ++i)
            {
                ThreadData td = new ThreadData ();
                td.Nr = i;

                testListerThreads[i] = new Thread(delegate() { TestListerThread(td); });
                testListerThreads[i].Start();
                testListerEvents[i] = new ManualResetEvent(false);
            }
#endif

#if TESTCONNECT
#if TESTCONNECTENV
            EnvironsTester.TestStartStopConnectSend(env, deviceList);
#else
            testThread = new Thread(TestThread);
            testThread.Start();
#endif
#endif
            return true;
        }

        public void Stop()
        {
#if TESTCONNECT
#if TESTCONNECTENV
            EnvironsTester.enableThread = false;
            EnvironsTester.testEvent.Set();
#else
            testRun = false;

            testThreadEvent.Set();
            testThreadEvent.WaitOne();
#endif
#endif
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
                    device.RemoveObserver(DeviceObserver);
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
                        device.AddObserver(DeviceObserver);
                        device.AddObserverForMessages(OnMessage);
                    }
                }
            }
#if TEST1
            int i = ++testListenerCur;

            if (i >= testListenerCount)
                i = testListenerCur = 0;

            testListerEvents[i].Set();
#endif
        }


        void DeviceObserver(DeviceInstance device, DeviceInfoFlag flags)
        {
            if (device == null)
                return;
            
#if TEST1
            if ((flags & DeviceInfoFlag.IsConnected) == DeviceInfoFlag.IsConnected)
            {
                int i = ++testListenerCur;

                if (i >= testListenerCount)
                    i = testListenerCur = 0;

                testListerEvents[i].Set();
            }
#endif
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
            if (msg == null || msg.disposed || msg.sent)
                return;

            String text = msg.text;

            if (text == null || text.Length <= 0)
                return;

            DeviceInstance device = msg.device;
            if (device == null)
                return;

            if (text[0] == '$')
            {
                if (text.StartsWith("$ca$0"))
                {
                    device.SendMessage("$ca$1$Echo");
                }
                else if (text.StartsWith("$ca$8"))
                {
                    device.SendMessage(text);
                }
                else if (text.StartsWith("$ca$4"))
                {
                    device.SendMessage("$ca$5$Hi i am your echo!");
                }
                return;
            }

            Console.WriteLine("  Message [ " + device.appContext0.ToString().PadLeft(4) + " ]: " + text);

            // Send echo ...
            device.SendMessage(text);
        }


        public static void PrintSmallHelp()
        {
            Console.WriteLine("  Press ESC to quit; h for help.");
        }

        public static void PrintHelp()
        {
            Console.Write("-------------------------------------------------------\n");
            Console.Write("  h[elp]               - print help\n");
            Console.Write("  p[rint] [page numer] - print available devices\n");
            Console.Write("  q[uit]               - quit application\n\n");

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

                if (c == 'h')
                {
                    PrintHelp();
                }
                else if (c == 'p')
                {
                    PrintDevices(lastLine);
                }
            }
            while (isRunning);
#if TEST1
            testListerRun = false;

            for (int i = 0; i < testListenerCount; ++i)
            {
                testListerEvents[i].Set();
                testListerEvents[i].WaitOne();
            }
#endif

#if TESTCONNECT
#if TESTCONNECTENV
            EnvironsTester.enableThread = false;
            EnvironsTester.testEvent.Set();
#else
            testRun = false;

            testThreadEvent.Set();
            testThreadEvent.WaitOne();
#endif
#endif
        }


        void PrintDevices(String line)
        {
            int pageID = 0;

            if (line != null)
            {
                String[] tokens = line.Split(new char[] { ' ' }, 3);
                if (tokens.Length >= 2)
                {
                    if (!int.TryParse(tokens[1], out pageID))
                        pageID = 0;
                }
            }

            DeviceList list = deviceList;
            if (list != null)
            {
                Collection<DeviceInstance> devices = list.GetDevices();
                if (devices != null)
                {
                    Console.WriteLine("  Devices: [ " + devices.Count + " ]");
                    Console.WriteLine("----------------------------------------------------------------");

                    int i = 0;
                    if (pageID > 0)
                        i = (pageID - 1) * 10;

                    for (; i < (int)devices.Count; ++i)
                    {
                        DeviceInstance device = devices[i];
                        if (device != null)
                        {
                            Console.WriteLine("  [ " + device.appContext0.ToString().PadLeft(4) + " ] " + device.toString());
                        }
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
            else if (line.StartsWith("help"))
            {
                PrintHelp();
            }
            else if (line.StartsWith("print"))
            {
                PrintDevices(line);
            }
            else if (line.StartsWith("quit"))
            {
                isRunning = false;
            }
            else if (line.StartsWith("set"))
            {
                String arg = line.Substring(3).Trim();

                if (arg != null && arg.StartsWith("env"))
                {
                    HandleSetEnv(arg.Substring(3).Trim());
                }
            }
        }


        void HandleSetEnv(String line)
        {
            if (line == null)
                return;

            String[] tokens = line.Split(' ');
            if (tokens.Length < 2)
            {
                Console.WriteLine("App and Area name are required arguments.");
                return;
            }

            Stop();

            appNameCur = tokens[0].Trim();
            areaNameCur = tokens[1].Trim();
            resetAuth = true;

            Init();
        }

#if TEST1
        struct ThreadData
        {
            public int Nr;
        }

        bool testListerRun = true;

        int testListenerCur = 0;
        const int testListenerCount = 20;
        Thread[] testListerThreads = new Thread[testListenerCount];
        ManualResetEvent[] testListerEvents = new ManualResetEvent[testListenerCount];

        void TestListerThread(ThreadData data)
        {
            try
            {
                int t = data.Nr;

                while (testListerRun)
                {
                    PrintDevices(null);

                    WifiEntry[] list = env.GetWifis();

                    BtEntry[] list1 = env.GetBts();
                    if (list1 != null)
                    {
                        for (int i = 0; i < list1.Length; i++)
                        {
                            BtEntry e = list1[i];
                            System.Console.WriteLine(e.ToString());
                        }
                    }

                    if (testListerEvents[t] != null)
                    {
                        testListerEvents[t].WaitOne();
                        testListerEvents[t].Reset();
                    }
                    else
                    {
                        Thread.Sleep(1000);
                    }
                }
            }
            catch (Exception ex)
            {
                if (ex != null && ex.Message != null)
                    System.Console.WriteLine(ex.Message);
            }
        }
#endif

#if TESTCONNECT

        bool testRun = true;
#if !TESTCONNECTENV
        Thread testThread;
#endif
        ManualResetEvent testThreadEvent = new ManualResetEvent(false);

        void TestThread()
        {
            int min = 500;
            int max = 10000;
            Random rand = new Random();

            try
            {
                while (testRun)
                {
                    if (env.GetStatus() >= Status.Started)
                    {
                        env.Stop();
                        min = 500;
                        max = 2000;
                    }
                    else if (env.GetStatus() == Status.Stopped)
                    {
                        env.ClearStorage();

                        env.Start();
                        min = 20000;
                        max = 30000;
                        //max = 60000;
                    }

                    testThreadEvent.WaitOne(500 + min + (rand.Next(max)));
                }
            }
            catch (Exception ex)
            {
                if (ex != null && ex.Message != null)
                    System.Console.WriteLine(ex.Message);
            }
        }
#endif
    }
}

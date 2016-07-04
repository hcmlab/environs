using System;
using System.Diagnostics;
using System.Linq;
using System.Threading;
using System.Windows;
using System.Windows.Input;

namespace environs.Apps
{
    class EnvironsTester
    {
        private const String className = "EnvironsTester . . . . .";

        static int currentTest = 0;

        Window window = null;
        Environs env = null;

        bool enableThread = false;
        Thread testThread = null;
        bool waiting = false;

        EnvironsTester()
        {
            Utils.Log(1, className, "Construct");
        }


        private void CloserThread()
        {
            Utils.Log(1, className, "CloserThread");

            Thread thread = testThread;
            if (thread != null)
            {
                if (waiting)
                    thread.Interrupt();
                else
                {
                    Utils.LogW("CloserThread: Waiting for test thread to be closed ...");
                    thread.Join();
                }
                testThread = null;
            }

            Utils.Log(1, className, "Construct: Closing window");

            Environs.dispatchSync(new Action(delegate()
            {
                window.Close();
            }));

            if (devList != null)
                devList.DisposeList();

            Utils.Log(1, className, "CloserThread: done");
        }


        private void OnClosing(Object sender, System.ComponentModel.CancelEventArgs e)
        {
            env = null;
            enableThread = false;

            if (testThread != null)
            {
                Utils.Log(1, className, "OnClosing: Invoking closer thread ...");

                e.Cancel = true;

                Thread closer = new Thread(CloserThread);
                if (closer == null)
                    Utils.LogE("OnClosing: Failed to create closer thread!!!");
                else
                    closer.Start();
            }
        }


        internal void appPreviewKeyDown(object sender, KeyEventArgs e)
        {
            Utils.Log(1, className, "appPreviewKeyDown: " + e.Key.ToString());

            if (e.Key == Key.F2)
            {
                enableThread = !enableThread;

                if (currentTest == 0)
                {
                    if (enableThread)
                        TestStartStop();
                }

                if (currentTest == 1)
                {
                    if (enableThread)
                        TestStartStopConnect();
                }

                if (currentTest == 2)
                {
                    if (enableThread)
                        TestStartStopConnectSend();
                }
            }
        }


        public static void TestStartStop(Window w, Environs e)
        {
            currentTest = 0;

            EnvironsTester tester = new EnvironsTester();
            tester.window = w;
            tester.env = e;
            e.async = Environs.CALL_WAIT;
            tester.TestStartStop();
        }


        private void TestStartStop()
        {
            enableThread = true;

            window.Closing += OnClosing;
            window.PreviewKeyDown += appPreviewKeyDown;

            testThread = new Thread(StartStopThread);
            if (testThread == null)
                Utils.LogE("EnvironsTester.Start: Failed to create test thread!!!");
            else
                testThread.Start();
        }


        private void StartStopThread()
        {
            try
            {
                Random rand = new Random();

                while (env != null && enableThread)
                {
                    if (env.status >= Status.Started)
                        env.Stop();
                    else if (env.status == Status.Stopped)
                        env.Start();

                    waiting = true;

                    Thread.Sleep(15000 + rand.Next(200, 3000));

                    waiting = false;
                }
            }
            catch (Exception e)
            {
                Debug.WriteLine(e.Message);
            }
        }

        
        DeviceList devList = null;


        public static void TestStartStopConnect(Window w, Environs e, DeviceList list)
        {
            currentTest = 1;

            EnvironsTester tester = new EnvironsTester();
            tester.window = w;
            tester.env = e;
            e.async = Environs.CALL_WAIT;
            tester.devList = list;
            tester.TestStartStopConnect();
        }


        private void TestStartStopConnect()
        {
            enableThread = true;

            devList.AddObserver(StartStopConnectThread_CollectionChanged);

            window.Closing += OnClosing;
            window.PreviewKeyDown += appPreviewKeyDown;

            testThread = new Thread(StartStopConnectThread);
            if (testThread == null)
                Utils.LogE("EnvironsTester.TestStartStopConnect: Failed to create test thread!!!");
            else
                testThread.Start();
        }


        void StartStopConnectThread_CollectionChanged(object sender, System.Collections.Specialized.NotifyCollectionChangedEventArgs args)
        {
            if (args.NewItems != null)
            {
                foreach (DeviceInstance device in args.NewItems)
                {
                    device.AddObserver(OnDeviceChanged);

                    //Debug.WriteLine("Appeared: " + device.ToString());
                    if (!device.isConnected)
                        device.Connect();
                }
            }
        }


        private void OnDeviceChanged(DeviceInstance device, DeviceInfoFlag Environs_NOTIFY_)
        {
            if (device == null)
                return;

            if ((Environs_NOTIFY_ & DeviceInfoFlag.IsConnected) == DeviceInfoFlag.IsConnected)
            {
                if (!device.isConnected)
                {
                    device.ClearStorage();
                }
            }
        }


        private void StartStopConnectThread()
        {
            int min = 500;
            int max = 10000;
            try
            {
                Random rand = new Random();

                while (env != null && enableThread)
                {
                    if (env.status >= Status.Started)
                    {
                        env.Stop();
                        min = 500;
                        max = 2000;
                    }
                    else if (env.status == Status.Stopped)
                    {
                        env.Start();
                        min = 3000;
                        max = 15000;
                    }
                    waiting = true;

                    Thread.Sleep(500 + rand.Next(min, max));

                    waiting = false;
                }
            }
            catch (Exception e)
            {
                Debug.WriteLine(e.Message);
            }
        }

        public static void TestStartStopConnectSend(Window w, Environs e, DeviceList list)
        {
            Utils.Log(1, className, "TestStartStopConnectSend");

            currentTest = 2;

            EnvironsTester tester = new EnvironsTester();
            tester.window = w;
            tester.env = e;
            e.async = Environs.CALL_WAIT;
            tester.devList = list;

            tester.TestStartStopConnectSend();
        }


        private void OnStatus(Status status)
        {
            Utils.Log(1, className, "OnStatus: " + status);

            Status environsStatus = (Status)status;
            if (environsStatus != Status.Started)
                return;

            try
            {
                DeviceInstance[] devices = devList.GetDevices().ToArray();
                if (devices == null)
                    return;

                foreach (DeviceInstance device in devices)
                {
                    if (device != null && !device.isConnected)
                        device.Connect();
                }
            }
            catch(Exception ex)
            {
                Utils.LogE(ex.Message);
            }
        }


        private void TestStartStopConnectSend()
        {
            Utils.Log(1, className, "TestStartStopConnectSend");

            enableThread = true;

            env.OnStatus += OnStatus;
            //env.SetDebug(3);

            devList.AddObserver(StartStopConnectSendThread_CollectionChanged);

            window.Closing += OnClosing;
            window.PreviewKeyDown += appPreviewKeyDown;


            testThread = new Thread(StartStopConnectSendThread);
            if (testThread == null)
                Utils.LogE("EnvironsTester.StartStopConnectSendThread: Failed to create test thread!!!");
            else
                testThread.Start();
        }


        private void StartStopConnectSendThread()
        {
            Utils.Log(1, className, "StartStopConnectSendThread");

            int min = 500;
            int max = 10000;
            try
            {
                Random rand = new Random();

                while (env != null && enableThread)
                {
                    if (env.status >= Status.Started)
                    {
                        env.Stop();
                        min = 500;
                        max = 2000;

                        ChatUser.DisposeChatUsers();
                    }
                    else if (env.status == Status.Stopped)
                    {
                        env.ClearStorage();

                        env.Start();
                        min = 20000;
                        max = 30000;
                        //max = 60000;
                    }
                    waiting = true;

                    Thread.Sleep(500 + rand.Next(min, max));

                    waiting = false;
                }
            }
            catch (Exception e)
            {
                Debug.WriteLine(e.Message);
            }

            Utils.Log(1, className, "StartStopConnectSendThread: done");
        }


        void StartStopConnectSendThread_CollectionChanged(object sender, System.Collections.Specialized.NotifyCollectionChangedEventArgs args)
        {
            if (args.NewItems != null)
            {
                foreach (DeviceInstance device in args.NewItems)
                {
                    device.AddObserver(OnDeviceChangedSend);

                    //Debug.WriteLine("Appeared: " + device.ToString());
                    if (!device.isConnected)
                        device.Connect();
                }
            }
        }


        private void OnDeviceChangedSend(DeviceInstance device, DeviceInfoFlag changedFlags)
        {
            if (device == null)
                return;

            if ((changedFlags & DeviceInfoFlag.IsConnected) == DeviceInfoFlag.IsConnected)
            {
                if (device.isConnected)
                {
                    Random rand = new Random();

                    if ((rand.Next() % 2) == 0)
                        device.SendFile(1, "TestFile.png", @"C:\Temp\test.png");
                    else
                        device.SendFile(1, "TestFile.png", @"C:\Temp\test1.png");
                }
                else
                {
                    device.ClearStorage();
                }
            }
        }
    }
}

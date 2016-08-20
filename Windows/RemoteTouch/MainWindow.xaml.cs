/**
* ------------------------------------------------------------------
 * Copyright (c) Chi-Tai Dang
 *
 * @author	Chi-Tai Dang
 * @version	1.0
 * @remarks
 *
 * This file is part of the Environs framework developed at the
 * Lab for Human Centered Multimedia of the University of Augsburg.
 * http://hcm-lab.de/environs
 *
 * Environ is free software; you can redistribute it and/or modify
 * it under the terms of the Eclipse Public License v1.0.
 * A copy of the license may be obtained at:
 * http://www.eclipse.org/org/documents/epl-v10.html
 * --------------------------------------------------------------------
*/
using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Diagnostics;
using Hardcodet.Wpf.TaskbarNotification;
using System.IO;
using System.Threading;
using System.Collections.ObjectModel;
using environs;


namespace environs.Apps
{
    /// <summary>
    /// Interactionlogic für MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window, IDisposable
    {
        private const String className = "MainWindow . . . . . . .";

        /// <summary>
        /// The device environment object that automatically handles the environment stuff.
        /// </summary>
        private Environs environs = null;
        private DeviceList deviceList = null;
        private TaskbarIcon taskBar = null;

        private Status environsStatus = 0;

        public MainWindow()
        {
            InitializeComponent();

            this.StateChanged += window_StateChanged;

            portalSinkCallback += PortalSinkEvent;

            PreviewKeyDown += appPreviewKeyDown;

            InitEnvirons();
        }


        #region Initialization

        private void InitEnvirons()
        {
            /// Instantiate surface environment as part of the device environment
            environs = Environs.New(this, "Kinect2Tracker", "Environs");
            if (environs == null)
                return;

            environs.SetUseFullscreen(false);
            environs.SetPortalFromAppWindow(false);

            environs.SetUseDefaultMediator(true);
            environs.SetMediatorFilterLevel(MediatorFilter.None);

            /// Attach to message delegate, in order to display environment messages on the media view
            environs.OnMessage += OnMessage;

            /// Attach to data delegate, in order to receive data sent from other devices
            environs.OnData += OnData;

            environs.OnStatus += OnStatus;

            /// Attach to notification delegate, in order to receive notifications
            environs.OnNotify += OnNotification;

            /// Attach to notification delegate, in order to receive notifications
            environs.OnStatusMessage += OnStatusMessage;

            environs.SetUserName("T@T.t");
            environs.SetUseAuthentication(true);

            environs.SetUseEncoder("libEnv-EncOpenH264");
            environs.SetUseStream (true);

            deviceList = environs.CreateDeviceList(DeviceClass.All);
            if (deviceList != null)
                deviceList.AddObserver(DeviceList_CollectionChanged);

            environs.Start();            
        }


        private void InitializedEvent()
        {
            /**
             * In order to receive this event, this handler has to be given to the environs instance, e.g. in this example, it is done as part of the instance constructor (within the SurfaceMediwView constructor)
             * environs = new SurfaceEnvirons(this, InitializedEvent);
             */
            Utils.Log(1, "InitializedEvent: Environs is now initialized");

            /**
             * Now environs is initialized and ready to use. Furthermore the ui window is visible and fullscreen if requested to do so.
             */

            buildTaskBar();

            updateUIStatus();

            //this.Visibility = Visibility.Hidden;
            updateUI();
        }


        void window_StateChanged(object sender, EventArgs e)
        {
            if (this.WindowState == WindowState.Minimized)
            {
                this.Visibility = Visibility.Hidden;
            }
            else
                updateUI();
        }


        void DeviceList_CollectionChanged(object sender, System.Collections.Specialized.NotifyCollectionChangedEventArgs args)
        {
            if (args.OldItems != null)
            {
                foreach (DeviceInstance device in args.OldItems)
                {
                    Debug.WriteLine("Appeared: " + device.ToString());

                    device.OnPortalRequestOrProvided += OnPortalRequestOrProvided;
                }
            }

            if (args.NewItems != null)
            {
                foreach (DeviceInstance device in args.NewItems)
                {
                    Debug.WriteLine("Appeared: " + device.ToString());
                }
            }

            updateDevices();
        }

        #endregion


        #region Disposing

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (disposing)
            {
                if (taskBar != null)
                {
                    taskBar.Dispose();
                    taskBar = null;
                }
            }
        }

        #endregion


        internal void appPreviewKeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.T)
            {
                if (!isD2Connected)
                {
                    environs.DeviceConnect(0xD2, "Environs", "HCMDefaultApp", Call.NoWait);
                    return;
                }

                const int bufSize = 2048000;

                byte[] buffer = new byte[bufSize];
                buffer[0] = Convert.ToByte('a');
                buffer[1] = Convert.ToByte('b');
                buffer[2] = Convert.ToByte('c');
                buffer[3] = Convert.ToByte('d');
                unsafe
                {
                    fixed (byte* pByte = buffer)
                    {
                        IntPtr intPtr = new IntPtr((void*)pByte);
                        //environs.SendBuffer(0xD2, "Environs", "HCMDefaultApp", 1, 2048, "noname", intPtr, bufSize);
                    }
                }
            }
            else
                if (e.Key == Key.D)
                {
                    if (isD2Connected)
                    {
                    //environs.DeviceDisconnect(0xD2, "Environs", "HCMDefaultApp", Environs.CALL_ASYNC);
                    return;
                    }
                }
        }

        #region Environs notifications

        bool isD2Connected = false;

        private void OnMessage(ObserverMessageContext context)
        {
            /*
             * In order to receive environs messages, this handler has to be attached to a delegate of th environs instance, e.g. in this example, it is done in the InitThread
             * environs.Message += OnMessage;
             */
            if (context.sourceType == (int)Source.Native)
            {
                if (Utils.Log(2))
                    Utils.Log(className, "OnMessage: Environs message by device [0x" + context.destID.ToString("X") + "] [" + context.message + "]");
            }
            else if (context.sourceType == (int)Source.Device)
            {
                if (Utils.Log(2))
                    Utils.Log(className, "OnMessage: Environs message by device [0x" + context.destID.ToString("X") + "] [" + context.message + "]");

                if (taskBar == null)
                    return;

                string title = "Text Message from 0x" + context.destID.ToString("X");

                taskBar.CloseBalloon();
                taskBar.ShowBalloonTip(title, context.message, Properties.Resources.hcm);
            }
        }



        private void OnStatus(Status status)
        {
            updateUI();
        }


        private void OnNotification(ObserverNotifyContext context)
        {
                if (Utils.Log(4) && context.notification != Environs.NOTIFY_PORTAL_LOCATION_CHANGED && context.notification != Environs.NOTIFY_PORTAL_SIZE_CHANGED )
                Utils.Log(className, "Notification: [0x" + context.destID.ToString("X") + "] [" + context.sourceIdent + "] [" + Environs.resolveName(context.notification) + "]");

            int notifType = (context.notification & Environs.MSG_NOTIFY_CLASS);
            string title, msg;

            switch (notifType)
            {
                case Environs.NOTIFY_TYPE_CONNECTION:
                    updateUI();

                switch (context.notification)
                {
                    case Environs.NOTIFY_CONNECTION_MAIN_FAILED:
                        break;

                    case Environs.NOTIFY_CONNECTION_ESTABLISHED:
                        if (context.destID == 0xD2)
                            isD2Connected = true;

                        if (taskBar == null)
                            break;

                        title = "Device connection";
                        msg = "Connected to 0x" + context.destID.ToString("X");

                        taskBar.CloseBalloon();
                        taskBar.ShowBalloonTip(title, msg, Properties.Resources.hcm);

                        //DeviceDisplay props = Environs.getDeviceScreenSizes(deviceID);
                        //if (props != null)
                        //    Utils.Log(0, props.ToString());
                        break;

                    case Environs.NOTIFY_CONNECTION_CLOSED:
                        if (context.destID == 0xD2)
                            isD2Connected = false;
                        if (taskBar == null)
                            break;

                        msg = "Disconnected from 0x" + context.destID.ToString("X");
                        title = "Device connection";

                        taskBar.CloseBalloon();
                        taskBar.ShowBalloonTip(title, msg, Properties.Resources.hcm);
                        break;
                }
                    break;

                case Environs.NOTIFY_TYPE_PORTAL:
                switch (context.notification)
                {
                    case Environs.NOTIFY_PORTAL_STREAM_STARTED:
                        if (taskBar == null)
                            break;

                        msg = "Portal to 0x" + context.destID.ToString("X") + " started";
                        title = "Portal status";

                        taskBar.CloseBalloon();
                        taskBar.ShowBalloonTip(title, msg, Properties.Resources.hcm);


                        //Environs.setRenderOverlayPNG(deviceID, sourceIdent, 0, 0, 0, "plan1.png", 10);
                        break;

                    case Environs.NOTIFY_PORTAL_STREAM_STOPPED:
                        if (taskBar == null)
                            break;

                        msg = "Portal to 0x" + context.destID.ToString("X") + " stopped";
                        title = "Portal status";

                        taskBar.CloseBalloon();
                        taskBar.ShowBalloonTip(title, msg, Properties.Resources.hcm);
                        break;

                    case Environs.NOTIFY_PORTAL_STREAM_INCOMING:
                        if (taskBar == null)
                            break;

                        msg = "Incoming Portal from 0x" + context.destID.ToString("X");
                        title = "Portal status";

                        taskBar.CloseBalloon();
                        taskBar.ShowBalloonTip(title, msg, Properties.Resources.hcm);


                        Environs.dispatch(new Action(delegate()
                        {
                            DeviceWindow deviceWin = new DeviceWindow();
                            //deviceWin.Init(context.destID, context.sourceIdent);
                            deviceWin.Show();
                        }));


                            environs.SetRenderCallback(Call.NoWait, context.sourceIdent, portalSinkCallback, RenderCallbackType.AvContext);

                            //Environs.SetRenderSurface(deviceID, sourceIdent, IntPtr.Zero, (uint)this.ActualWidth, (uint)this.ActualHeight);

                            environs.StartPortalStream(Call.NoWait, context.sourceIdent);
                        break;
                }
                    break;

                case Environs.NOTIFY_TYPE_OPTIONS:
                switch (context.notification)
                {
                    case Environs.NOTIFY_PORTAL_LOCATION_CHANGED:
                        {
                        }
                        break;

                    case Environs.NOTIFY_PORTAL_SIZE_CHANGED:
                        {
                        }
                        break;
                }
                    break;

                case Environs.NOTIFY_TYPE_ENVIRONS:
                updateEnvironsStatus ();

                switch (context.notification)
                {
                    case Environs.NOTIFY_DEVICE_ON_SURFACE:
                        {
                            if (Utils.Log(4))
                                Utils.Log(className, "OnNotification: device id [0x" + context.destID.ToString("X") + "] is on surface");

                        }
                        break;

                    case Environs.NOTIFY_DEVICE_NOT_ON_SURFACE:
                        {
                            if (Utils.Log(4))
                                Utils.Log(className, "OnNotification: device id [0x" + context.destID.ToString("X") + "] is NOT on surface");
                        }
                        break;

                    case Environs.NOTIFY_START_SUCCESS:
                        {
                            Utils.Log(1, "OnNotification: OnNotification: Environs started");

                            Environs.dispatch(new Action(delegate()
                            {
                                if (menuItemEnvirons != null)
                                    menuItemEnvirons.Header = "Stop Environs";
                            }));
                        }
                        break;

                    case Environs.NOTIFY_STOP_SUCCESS:
                        {
                            Utils.Log(1, "OnNotification: OnNotification: Environs stopped");

                            Environs.dispatch(new Action(delegate()
                            {
                                if (menuItemEnvirons != null)
                                    menuItemEnvirons.Header = "Start Environs";
                            }));
                        }
                        break;
                }
                    break;
            }
        }


        private void OnPortalRequestOrProvided(PortalInstance portal)
        {
            //portal.Establish();
        }


        Thread updateEnvironsStatusThread = null;

        private void updateEnvironsStatusThreaded()
        {
            if (environs != null)
                environsStatus = environs.status;
            else
                environsStatus = Status.Uninitialized;

            Environs.dispatch(new Action(delegate()
            {
                switch (environsStatus)
                {
                    case Status.Uninitialized:
                        envStatusEllipse.Fill = Brushes.Red;
                        envStatusLabel.Content = "Uninitialized";
                        envStatusButton.Content = "Start";
                        break;
                    case Status.Initialized:
                        envStatusEllipse.Fill = Brushes.DarkGray;
                        envStatusLabel.Content = "Initialized";
                        envStatusButton.Content = "Start";
                        break;
                    case Status.Stopped:
                        envStatusEllipse.Fill = Brushes.LightGray;
                        envStatusLabel.Content = "Stopped";
                        envStatusButton.Content = "Start";
                        break;
                    case Status.Started:
                        envStatusEllipse.Fill = Brushes.LightGray;
                        envStatusLabel.Content = "Started";
                        envStatusButton.Content = "Stop";
                        break;
                    case Status.Connected:
                        envStatusEllipse.Fill = Brushes.LightGreen;
                        envStatusLabel.Content = "Connected";
                        envStatusButton.Content = "Stop";
                        break;
                }
            }));

            lock (this)
            {
                updateEnvironsStatusThread = null;
            }
        }

        private void updateEnvironsStatus()
        {
            //if (!Environs.isInitialized)
            //    return;
            lock (this)
            {
                if (updateEnvironsStatusThread != null)
                    return;

                updateEnvironsStatusThread = new Thread(updateEnvironsStatusThreaded);
                if (updateEnvironsStatusThread == null)
                    Utils.LogE("updateEnvironsStatus: Failed to create updateEnvironsStatusThread!!!");
                else
                    updateEnvironsStatusThread.Start();
            }
        }


        WriteableBitmap wbm = null;

        private void PortalSinkEvent(int argumentType, IntPtr surface, IntPtr callbackArgument)
        {
            //Debug.WriteLine("[INFO] PortalSinkEvent:");

            if (argumentType != (int)RenderCallbackType.AvContext)
                return;

            Environs.dispatch(new Action(delegate()
            {
                if (!Utils.CreateUpdateBitmap(ref wbm, callbackArgument))
                    return;

                if (wbm == null)
                    return;

                this.Background = new ImageBrush(wbm);
            }));
        }

        public PortalSinkSource portalSinkCallback = null;


        Thread updateDevicesThread = null;

        /// <summary>
        /// Initialization thread for establishing and initializing the device environment
        /// </summary>
        private void updateDevicesThreaded()
        {
            if (environs == null)
                return;

            environsStatus = environs.status;
            if (deviceList == null)
                return;

            devicesAvailable = (ObservableCollection<DeviceInstance>)deviceList.GetDevicesSource();
            int devicesConnected = environs.GetConnectedDevicesCount();

            Environs.dispatch(new Action(delegate()
            {
                if (devicesAvailable == null)
                    labelDevicesAvailable.Content = 0;
                else
                    labelDevicesAvailable.Content = devicesAvailable.Count;
                labelDevicesConnected.Content = devicesConnected;
            }));

            lock (this)
            {
                updateDevicesThread = null;
            }
        }

        private void updateDevices()
        {
            lock (this)
            {
                if (updateDevicesThread != null)
                    return;

                updateDevicesThread = new Thread(updateDevicesThreaded);
                if (updateDevicesThread == null)
                    Utils.LogE("updateDevices: Failed to create updateDevicesThread!!!");
                else
                    updateDevicesThread.Start();
            }
        }


        ObservableCollection<DeviceInstance> devicesAvailable = null;

        private void updateUI()
        {
            updateEnvironsStatus();
            updateDevices();
        }

        private void updateUIStatus()
        {
            if (environs == null)
                return;

            if (environs.GetUseOpenCL())
                envStatusOpenCL.Fill = Brushes.LightGreen;
            else
                envStatusOpenCL.Fill = Brushes.OrangeRed;

            if (environs.GetUseStream())
                envStatusStream.Fill = Brushes.LightGreen;
            else
                envStatusStream.Fill = Brushes.OrangeRed;
        }

        /// <summary>
        /// Event handler that gets called when data from environs is available.
        /// </summary>
        /// <param name="source">The device id that this data is sent from. 0 means that the data is an internal data from this environs instance.</param>
        /// <param name="sourceType">The data type: ENVIRONS_SOURCE_NATIVE = data from environs, ENVIRONS_SOURCE_DEVICE = data was sent from a device.</param>
        /// <param name="msg">The data buffer.</param>
        private void OnData(ObserverDataContext context)
        {
            if (Utils.Log(4))
                Utils.Log(className, "OnData: Received data from device [0x" + context.objID.ToString("X") + "] of size " + context.size);

            /*
             * In order to receive environs data, this handler has to be attached to a delegate of th environs instance, e.g. in this example, it is done in the InitThread
             * environs.Data += OnData;
             */
            if (context.type == (int)Source.Native)
            {
                //String filepath = environs.GetFilePathNative(context.objID, context.fileID);
                //Utils.Log(1, "OnData: File has been stored to " + filepath);

                if (context.fileID > 0 && context.size > 0)
                {
                    Environs.dispatch(new Action(delegate()
                    {
                        //byte[] data = environs.GetFileNative(nativeID, fileID);
                        //if (data != null)
                        //{
                        //    // Save file to desktop
                        //    string path = Environment.GetFolderPath(Environment.SpecialFolder.Desktop) + @"\" + fileName;
                        //    File.WriteAllBytes(path, data);

                        //    if (taskBar != null)
                        //    {
                        //        string title = "Received a file from 0x" + nativeID.ToString("X");
                        //        string msg = fileName + " of size " + size + " stored to desktop";

                        //        taskBar.CloseBalloon();
                        //        taskBar.ShowBalloonTip(title, msg, Properties.Resources.hcm);
                        //    }
                        //}
                    }));
                }
            }
        }


        /// <summary>
        /// Event handler that gets called when environment messages are available.
        /// In this example, it appends the message to the messages-textbox within the UI-thread.
        /// </summary>
        /// <param name="source">The device id that this message is sent from. 0 means that the message is an internal message from this environs instance.</param>
        /// <param name="sourceType">The message type: ENVIRONS_MSG_SOURCE_NATIVE = message from environs, ENVIRONS_MSG_SOURCE_DEVICE = message was sent from a device.</param>
        /// <param name="msg">The message from the environment.</param>
        private void OnStatusMessage(string msg)
        {
        }

#endregion


        #region Taskbar and ContextMenu
        MenuItem menuItemEnvirons = new MenuItem();

        private void buildTaskBar()
        {
            if (taskBar != null)
                return;

            taskBar = new TaskbarIcon();
            taskBar.Icon = Properties.Resources.hcm;
            taskBar.ToolTipText = "RemoteTouch";
            taskBar.LeftClickCommand = ShowSettingsWindowCommand;

            ContextMenu menu = new ContextMenu();
            menuItemEnvirons = new MenuItem();
            //menuItemEnvirons.Header = "Start Environs";
            menuItemEnvirons.Command = ToggleEnvironsCommand;
            menu.Items.Add(menuItemEnvirons);

            MenuItem mi = new MenuItem();
            mi.Header = "Show Settings Window";
            mi.Command = ShowSettingsWindowCommand;
            menu.Items.Add(mi);

            menu.Items.Add(new Separator());

            mi = new MenuItem();
            mi.Header = "Quit";
            mi.Command = QuitCommand;
            menu.Items.Add(mi);

            taskBar.ContextMenu = menu;
        }

        /// <summary>
        /// Start/Stop Environs
        /// </summary>
        private ICommand ToggleEnvironsCommand
        {
            get
            {
                return new DelegateCommand
                {
                    CanExecuteFunc = () => environs.isInitialized,
                    CommandAction = () =>
                    {
                        if (environs == null)
                            return;

                        if (environs.status == Status.Started)
                            environs.Stop();
                        else
                            environs.Start();
                    }
                };
            }
        }


        /// <summary>
        /// Shows settings window
        /// </summary>
        private ICommand ShowSettingsWindowCommand
        {
            get
            {
                return new DelegateCommand
                {
                    CanExecuteFunc = () => true,
                    CommandAction = () =>
                    {
                        if (Application.Current.MainWindow != null)
                        {
                            Application.Current.MainWindow.Show();
                            Application.Current.MainWindow.Activate();
                            Application.Current.MainWindow.WindowState = WindowState.Normal;

                            Application.Current.MainWindow.Activate();
                            Application.Current.MainWindow.Topmost = true;
                            Application.Current.MainWindow.Topmost = false;
                            Application.Current.MainWindow.Focus();
                        }
                    }
                };
            }
        }

        /// <summary>
        /// Quit the application
        /// </summary>
        private ICommand QuitCommand
        {
            get
            {
                return new DelegateCommand
                {
                    CanExecuteFunc = () => true,
                    CommandAction = () =>
                    {
                        Application.Current.MainWindow.Close();
                    }
                };
            }
        }


        /// <summary>
        /// Simplistic delegate command for the demo.
        /// </summary>
        private class DelegateCommand : ICommand
        {
            public Action CommandAction { get; set; }
            public Func<bool> CanExecuteFunc { get; set; }

            public void Execute(object parameter)
            {
                CommandAction();
            }

            public bool CanExecute(object parameter)
            {
                return CanExecuteFunc == null || CanExecuteFunc();
            }

            public event EventHandler CanExecuteChanged
            {
                add { CommandManager.RequerySuggested += value; }
                remove { CommandManager.RequerySuggested -= value; }
            }
        }
        #endregion


        #region Button handler

        private void envStatusButton_Click(object sender, RoutedEventArgs e)
        {
            if (environs == null)
                return;

            environs.Start();

            if (environs.status <= Status.Stopped)
                environs.Start();
            else
            {
                environs.Stop();
                updateEnvironsStatus();
            }
        }

        private void Window_IsVisibleChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            if (environs != null && environs.isInitialized)
            {
                updateEnvironsStatus();
            }
        }

        private void buttonRefresh_Click(object sender, RoutedEventArgs e)
        {
            updateUI();
        }

        private void envStatusStream_MouseUp(object sender, MouseButtonEventArgs e)
        {
            if (environs == null)
                return;
            environs.SetUseStream(!environs.GetUseStream());
            updateUIStatus();
        }

        private void envStatusOpenCL_MouseUp(object sender, MouseButtonEventArgs e)
        {
            if (environs == null)
                return;
            environs.SetUseOpenCL(!environs.GetUseOpenCL());
            updateUIStatus();
        }


        public static bool doTestRun = false;

        private void envStatusLabel_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {

            doTestRun = !doTestRun;

            if (doTestRun)
            {
                Thread init = new Thread(TestThread);
                if (init == null)
                    Debug.WriteLine("[ERROR] -- E -->:Environs: Failed to create test thread!!!");
                else
                    init.Start();
            }
        }

        //private static bool doTestRun = false;

        /// <summary>
        /// Initialization thread for establishing and initializing the device environment
        /// </summary>
        private void TestThread()
        {
            Debug.WriteLine("[INFO]  Environs: threaded initialization started...");

            Random rand = new Random();
            while (doTestRun)
            {
                if (environs == null)
                    break;

                //DeviceInstance[] devices = Environs.GetDevicesAvailable();
                //if (devices != null)
                //{
                //    for (uint i = 0; i < devices.Length; i++)
                //        Debug.WriteLine(devices[i].ToString());
                //}
                if (environs.status == Status.Started)
                {
                    environs.Dispose();
                    updateEnvironsStatus();
                }
                else
                    environs.Start();

                int next = rand.Next(4000);
                Thread.Sleep(next + 1000);
            }
        }

        #endregion

    }
}

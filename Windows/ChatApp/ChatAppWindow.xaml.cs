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
//#define USE_CHATUSER_IMAGESOURCE
using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Media;
using System.Windows.Media.Imaging;

using System.IO;
using System.Threading;
using System.Collections.ObjectModel;
using System.Collections;
using System.Diagnostics;

namespace environs.Apps
{
    /// <summary>
    /// Interactionlogic for ChatAppWindow.xaml
    /// </summary>
    public partial class ChatAppWindow : Window, IDisposable
    {
        private const String className = "ChatAppWindow. . . . . .";


        /// <summary>
        /// The device environment object that automatically handles the environment stuff.
        /// </summary>
        private Environs environs = null;
        bool enabled = true;

        private Status environsStatus = 0;

        internal DeviceList deviceList;

        internal static String statusMessage = "Hi, there!";
        internal static String userImageBase64 = null;

#if USE_CHATUSER_IMAGESOURCE
        internal static ImageSource defaultUserImage = null;
#else
        internal static BitmapImage defaultUserImage = null;
#endif
        internal static bool useInitThread = true;
        Collection<ChatUser> chatsToInit = new Collection<ChatUser>();
        bool initThreadEnabled = false;
        Thread initThread = null;
        ManualResetEvent initThreadEvent = new ManualResetEvent(false);

        bool updaterThreadEnabled = false;
        internal static ManualResetEvent updaterThreadEvent = new ManualResetEvent(false);

        bool statusThreadEnabled = false;
        Thread statusThread = null;
        ManualResetEvent statusThreadEvent = new ManualResetEvent(false);


        public ChatAppWindow()
        {
            InitializeComponent();

            userList.EnableColumnVirtualization = true;
            userList.EnableRowVirtualization = true;
            InitEnvirons();
        }


        #region Initialization

        const String areaName = "Environs";
        const String appName = "ChatApp";


        private void InitEnvirons()
        {
            ChatUser.loginUserName = Environment.UserName + "." + Environment.MachineName + "." + ((new Random()).Next() % 1000);

            Closing += OnClosing;

            environs = Environs.CreateInstance(this, InitializedEvent);
            if (environs == null)
                return;
            environs.ClearStorage();

            // Allow unsecure channels if the device (that we connect to) explicitly requests that
            environs.SetUseCLSForDevicesEnforce(false);
            
            //environs.SetUseCLSForDevices(false);

            environs.LoadSettings(appName, areaName);

            environs.SetUseDefaultMediator(true);
            InitEnvironsChat();

            deviceList = environs.CreateDeviceList(DeviceClass.All);
            if (deviceList != null)
                deviceList.AddObserver(OnListChanged);

            Uri uri = new Uri("pack://application:,,,/Resources/user.png");

#if USE_CHATUSER_IMAGESOURCE
            BitmapImage bi = new BitmapImage();
            if (bi != null)
            {
                bi.BeginInit();
                bi.UriSource = uri;
                bi.CacheOption = BitmapCacheOption.OnLoad;

                bi.EndInit();
                bi.Freeze();

                defaultUserImage = bi;
            }
#else
            defaultUserImage = new BitmapImage();
            if (defaultUserImage != null)
            {
                defaultUserImage.BeginInit();
                defaultUserImage.UriSource = uri;
                defaultUserImage.CacheOption = BitmapCacheOption.OnLoad;

                defaultUserImage.EndInit();
                defaultUserImage.Freeze();
            }
#endif
            var info = Application.GetResourceStream(uri);
            if (info != null)
            {
                var memoryStream = new MemoryStream();
                info.Stream.CopyTo(memoryStream);

                byte[] buffer = memoryStream.ToArray();
                if (buffer != null)
                    userImageBase64 = Convert.ToBase64String(buffer);

                memoryStream.Close();
            }

            messagesScrollViewer = scrollerMessages;

            environs.OnStatus += OnStatus;

            /// Attach to notification delegate, in order to receive notifications
            environs.OnNotify += OnNotification;

            environs.SetUseMediatorAnonymousLogon(true);
            
            StartUpdaterThread();

            StartStatusThread();

            environs.Start();

            StartInitThread();
        }


        private void OnClosing(Object sender, System.ComponentModel.CancelEventArgs e)
        {
            initThreadEnabled = false;
            initThreadEvent.Set();

            if (updaterThreadEnabled)
            {
                updaterThreadEnabled = false;
                updaterThreadEvent.Set();
            }

            statusThreadEnabled = false;
            statusThreadEvent.Set();
        }


        private void InitializedEvent()
        {
            /**
             * In order to receive this event, this handler has to be given to the environs instance, e.g. in this example, it is done as part of the instance constructor (within the SurfaceMediwView constructor)
             * environs = new SurfaceEnvirons(this, InitializedEvent);
             */
            Utils.Log(1, "InitializedEvent: Environs is now initialized");

            statusThreadEvent.Set();
        }


        public void StartInitThread()
        {
            if (!useInitThread)
                return;

            lock (chatsToInit)
            {
                if (initThread != null)
                    return;

                initThread = new Thread(InitThread);
                if (initThread == null)
                    Utils.LogE("StartInitThread: Failed to create initThread!!!");
                else
                    initThread.Start();
            }
        }


        public void InitThread()
        {
            initThreadEnabled = true;
            int count;

            while (initThreadEnabled)
            {
                int needsPingCount = 0;

                ChatUser[] chats = null;

                lock (chatsToInit)
                {
                    if (chatsToInit.Count > 0)
                    {
                        chats = new ChatUser[chatsToInit.Count];
                        chatsToInit.CopyTo(chats, 0);
                    }
                }

                if (chats != null && chats.Length > 0)
                {
                    ArrayList chatsToRemove = new ArrayList();

                    foreach (ChatUser chat in chats)
                    {
                        if (chat.Init())
                            chatsToRemove.Add(chat);
                        else
                        {
                            if (chat.initState == 1)
                                needsPingCount++;
                        }
                    }

                    lock (chatsToInit)
                    {
                        if (chatsToRemove.Count > 0)
                        {
                            foreach (ChatUser chat in chatsToRemove)
                            {
                                chatsToInit.Remove(chat);
                            }
                        }
                        count = chatsToInit.Count;
                    }
                }
                else
                    count = 0;

                int timeout = -1;

                if (enabled)
                {
                    if (count > 0)
                    {
                        if (needsPingCount > 0)
                            timeout = 400;
                        else
                            timeout = 10000;
                    }
                }
                else
                {
                    lock (chatsToInit)
                    {
                        chatsToInit.Clear();
                    }
                }

                if (timeout > 0)
                    initThreadEvent.WaitOne(timeout);
                else
                    initThreadEvent.WaitOne();
                initThreadEvent.Reset();
            }

            lock (chatsToInit)
            {
                initThread = null;

                chatsToInit.Clear();
            }
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
                if (initThreadEvent != null)
                {
                    initThreadEvent.Dispose();
                    initThreadEvent = null;
                }

                if (statusThreadEvent != null)
                {
                    statusThreadEvent.Dispose();
                    statusThreadEvent = null;
                }
            }
        }

        #endregion


        #region Environs Observers

        private void OnStatus(Status status)
        {
            statusThreadEvent.Set();
        }


        private void OnNotification(ObserverNotifyContext context)
        {
            if (Utils.Log(6))
                Utils.Log1(className, "Notify: [0x" + context.destID.ToString("X") + "] [" + context.sourceIdent + "] [" + Environs.resolveName(context.notification) + "]");

            int notifType = (context.notification & Environs.MSG_NOTIFY_CLASS);

            switch (notifType)
            {
                case Notify.Connection.type:
                    statusThreadEvent.Set();
                    break;

                case Notify.Environs.type:
                    statusThreadEvent.Set();
                    break;
            }
        }


        public void StartStatusThread()
        {
            lock (this)
            {
                if (statusThread != null)
                    return;

                statusThread = new Thread(UpdateEnvironsStatusThread);
                if (statusThread == null)
                    Utils.LogE("StartStatusThread: Failed to create initThread!!!");
                else
                    statusThread.Start();
            }
        }
        

        private void UpdateEnvironsStatusThread()
        {
            statusThreadEnabled = true;

            while (statusThreadEnabled)
            {
                Environs env = environs;
                if (env != null && environsStatus != env.status)
                {
                    environsStatus = env.status;

                    Environs.dispatchSync(new Action(delegate ()
                    {
                        switch (environsStatus)
                        {
                            case Status.Uninitialized:
                                ellipseEnvStatus.Fill = Brushes.Red;
                                buttonEnvStartStop.Content = "Start";
                                break;
                            case Status.Initialized:
                                ellipseEnvStatus.Fill = Brushes.DarkGray;
                                buttonEnvStartStop.Content = "Start";
                                break;
                            case Status.Stopped:
                                //enabled = false;
                                ellipseEnvStatus.Fill = Brushes.LightGray;
                                buttonEnvStartStop.Content = "Start";
                                OnEnvironsStopped();

                                ChatUser.DisposeChatUsers();
                                break;
                            case Status.Started:
                                enabled = true;
                                ellipseEnvStatus.Fill = Brushes.YellowGreen;
                                buttonEnvStartStop.Content = "Stop";
                                OnEnvironsStarted(env);
                                break;
                            case Status.Connected:
                                ellipseEnvStatus.Fill = Brushes.Yellow;
                                buttonEnvStartStop.Content = "Stop";
                                break;
                        }
                    }));
                }

                statusThreadEvent.WaitOne();
                statusThreadEvent.Reset();
            }

            lock (this)
            {
                statusThread = null;
            }
        }

        #endregion



        #region Messages list

        public ObservableCollection<MessageInstance> emptyCollection
        {
            get { return emptyMessages; }
        }
        ObservableCollection<MessageInstance> emptyMessages = new ObservableCollection<MessageInstance>();

        public ObservableCollection<MessageInstance> messagesCollection
        {
            get { return currentMessages; }
        }

        ObservableCollection<MessageInstance> currentMessages;

        private void userList_SelectedCellsChanged(object sender, SelectedCellsChangedEventArgs e)
        {
            int row = userList.SelectedIndex;
            if (row < 0)
            {
                BindingOperations.SetBinding(messagesList, DataGrid.ItemsSourceProperty, new Binding("emptyCollection"));
                return;
            }

            DeviceInstance device = deviceList.GetItem(row);
            if (device == null || device.appContext1 == null)
            {
                BindingOperations.SetBinding(messagesList, DataGrid.ItemsSourceProperty, new Binding("emptyCollection"));
                return;
            }

            //device.Connect();

            if (currentMessages == ((ChatUser)device.appContext1).messages)
                return;

            currentMessages = ((ChatUser)device.appContext1).messages;

            messagesList.ItemsSource = null;
            messagesList.ItemsSource = messagesCollection;
        }


        public static ScrollViewer messagesScrollViewer;

        public static void ScrollMessagesToRecent()
        {
            messagesScrollViewer.ScrollToBottom();
            messagesScrollViewer.ScrollToEnd();
        }
        #endregion


        #region Button handler

        private void buttonEnvStartStop_Click(object sender, RoutedEventArgs e)
        {
            if (environs == null)
                return;

            if (environs.status <= Status.Stopped)
                environs.Start();
            else
            {
                environs.Stop();
                if (deviceList != null)
                    deviceList.Reload();
                statusThreadEvent.Set();
            }
        }


        private void buttonSend_Click(object sender, RoutedEventArgs e)
        {
            String msg = message.Text;
            if (msg == null || msg.Length <= 0)
                return;

            int row = userList.SelectedIndex;
            if (row < 0)
                return;

            DeviceInstance device = deviceList.GetItem(row);
            if (device == null || device.appContext1 == null)
                return;

            device.SendMessage(msg);
        }


        private void buttonConnect_Click(object sender, RoutedEventArgs e)
        {
            int row = userList.SelectedIndex;
            if (row < 0)
                return;

            DeviceInstance device = deviceList.GetItem(row);
            if (device == null)
                return;

            if (device.isConnected)
                device.Disconnect();
            else
                device.Connect();
        }

        #endregion
    }
}

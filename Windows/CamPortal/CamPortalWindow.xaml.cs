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
using System.Threading;
using System.Collections.ObjectModel;
using environs;

namespace environs.Apps
{
    /// <summary>
    /// Interactionlogic for CamPortalWindow.xaml
    /// </summary>
    public partial class CamPortalWindow : Window
    {
        private static String className = "CamPortalWindow";

        /// <summary>
        /// The device environment object that automatically handles the environment stuff.
        /// </summary>
        private Environs environs = null;

        private Status environsStatus = 0;

        internal DeviceList deviceList;
        
        public CamPortalWindow()
        {
            InitializeComponent();

            PreviewKeyDown += appPreviewKeyDown;

            InitEnvirons();
        }


        #region Initialization

        private void InitEnvirons()
        {
            environs = Environs.New(this, InitializedEvent, "CamPortal", "Environs");
            if (environs == null)
                return;

            deviceList = environs.CreateDeviceList(DeviceClass.All);
            deviceList.AddObserver(CamPortalWindow_CollectionChanged);

            environs.OnStatus += OnStatus;

            /// Attach to notification delegate, in order to receive notifications
            environs.OnNotify += OnNotification;

            environs.SetUseMediatorAnonymousLogon(true);

            environs.SetUseCapturer("libEnv-CapCamera");
            environs.SetUseRenderer("libEnv-BaseNull");
            environs.Start();
        }


        private void OnPortalRequestOrProvided(PortalInstance portal)
        {
            // If we don't add an observer, then the portal will be discarded
            portal.AddObserver(PortalObserver);

            if (portal.outgoing)
            {
                portal.portalType = PortalType.FrontCam;
            }
            else
                portal.SetRenderCallback(Renderer, 0, 0);

            portal.startIfPossible = true;

            portal.Establish(true);
        }


        void PortalObserver(PortalInstance portal, Notify.Portale notify)
        {
            switch (notify)
            {
                case Notify.Portale.EstablishedResolution:
                    
                    int width = portal.info.width;
                    if (width <= 0)
                        break;

                    int height = portal.info.height;
                    int widthView = (int)drawingContent.ActualWidth;

                    int heightView = (widthView * height) / width;

                    Environs.dispatch(new Action(delegate()
                    {
                        drawingContent.Height = heightView;
                    }));

                    break;
            }
        }


        public ObservableCollection<DeviceInstance> devicesCollection
        {
            get { return (ObservableCollection<DeviceInstance>)deviceList.GetDevicesSource(); }
        }


        void CamPortalWindow_CollectionChanged(object sender, System.Collections.Specialized.NotifyCollectionChangedEventArgs args)
        {
            if (args.NewItems != null)
            {
                foreach (DeviceInstance device in args.NewItems)
                {
                    Debug.WriteLine("Appeared: " + device.ToString());

                    device.OnPortalRequestOrProvided += OnPortalRequestOrProvided;
                }
            }
        }


        private void InitializedEvent()
        {
            /**
             * In order to receive this event, this handler has to be given to the environs instance, e.g. in this example, it is done as part of the instance constructor (within the SurfaceMediwView constructor)
             * environs = new SurfaceEnvirons(this, InitializedEvent);
             */
            Utils.Log(1, "InitializedEvent: Environs is now initialized");
            
            updateUI();
        }

        #endregion

        internal void appPreviewKeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.T)
            {
            }
        }

        #region Environs notifications

        private void OnStatus(Status status)
        {
            updateEnvironsStatus();
        }


        private void OnNotification(ObserverNotifyContext context)
        {
            if (Utils.Log(3))
                Utils.Log(className, "Notification: [0x" + context.destID.ToString("X") + "] [" + context.sourceIdent + "] [" + Environs.resolveName(context.notification) + "]");

            int notifType = (context.notification & Environs.MSG_NOTIFY_CLASS);

            switch (notifType)
            {
                case Notify.Connection.type:
                    updateUI();
                    break;

                case Notify.Environs.type:
                updateEnvironsStatus ();
                    break;
            }
        }


        Thread updateEnvironsStatusThread = null;

        private void updateEnvironsStatusThreaded()
        {
            environsStatus = environs.status;

            Environs.dispatch(new Action(delegate()
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
                        ellipseEnvStatus.Fill = Brushes.LightGray;
                        buttonEnvStartStop.Content = "Start";
                        break;
                    case Status.Started:
                        ellipseEnvStatus.Fill = Brushes.YellowGreen;
                        buttonEnvStartStop.Content = "Stop";
                        break;
                    case Status.Connected:
                        ellipseEnvStatus.Fill = Brushes.Yellow;
                        buttonEnvStartStop.Content = "Stop";
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
        
        private void updateUI()
        {
            updateEnvironsStatus();
        }

#endregion

               

        #region Button handler

        private void buttonEnvStartStop_Click(object sender, RoutedEventArgs e)
        {
            if (environs.status <= Status.Stopped)
                environs.Start();
            else
            {
                environs.Stop();
                updateEnvironsStatus();
            }
        }

        #endregion


        private void devicesGrid_SelectedCellsChanged(object sender, SelectedCellsChangedEventArgs e)
        {
            int row = devicesGrid.SelectedIndex;
            if (row < 0)
            {
                return;
            }

            DeviceInstance device = deviceList.GetItem(row);
            if (device == null)
            {
                return;
            }
        }


        private void buttonSend_Click(object sender, RoutedEventArgs e)
        {
            String msg = message.Text;
            if (msg == null || msg.Length <= 0)
                return;

            int row = devicesGrid.SelectedIndex;
            if (row < 0)
                return;

            DeviceInstance device = deviceList.GetItem(row);
            if (device == null)
                return;

            device.SendMessage(msg);
        }


        private void buttonEstablish_Click(object sender, RoutedEventArgs e)
        {
            int row = devicesGrid.SelectedIndex;
            if (row < 0)
                return;

            DeviceInstance device = deviceList.GetItem(row);
            if (device == null)
                return;

            Thread thread = new Thread(() => EstablishThread(device));
            if (thread != null)
                thread.Start();
        }


        private void EstablishThread(DeviceInstance device)
        {
            if (!device.isConnected)
            {
                if (!device.Connect(Environs.CALL_WAIT))
                    return;
            }

            PortalInstance portal = device.PortalGetIncoming();
            if (portal != null)
            {
                portal.Stop();
                return;
            }

            portal = device.PortalRequest(PortalType.Any);
            if (portal == null)
                return;
            portal.async = Environs.CALL_WAIT;

            portal.AddObserver(PortalObserver);

            portal.SetRenderCallback(Renderer, (int)drawingContent.ActualWidth, (int)drawingContent.ActualHeight);

            portal.startIfPossible = true;
            portal.Establish(true);

            portal.async = Call.NoWait;
        }


        public void Renderer(WriteableBitmap bitmap)
        {
            drawingContent.Background = new ImageBrush(bitmap);
        }
    }
}

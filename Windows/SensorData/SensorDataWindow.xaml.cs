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
using System.Diagnostics;
using System.Threading;
using System.Collections.ObjectModel;

namespace environs.Apps
{
    /// <summary>
    /// Interactionlogic for SensorDataWindow.xaml
    /// </summary>
    public partial class SensorDataWindow : Window
    {
        private static String className = "SensorDataWindow";

        /// <summary>
        /// The device environment object that automatically handles the environment stuff.
        /// </summary>
        private Environs environs = null;

        private Status environsStatus = 0;

        internal DeviceList deviceList;

        private Timer osciTimer = null;
        int osciInterval = 33;

        public SensorDataWindow()
        {
            InitializeComponent();

            PreviewKeyDown += AppPreviewKeyDown;

            accelView1.SetTitle("Accel X");
            accelView2.SetTitle("Accel Y");
            accelView3.SetTitle("Accel Z");

            magneticView1.SetTitle("Magnetic X");
            magneticView2.SetTitle("Magnetic Y");
            magneticView3.SetTitle("Magnetic Z");

            gyroView1.SetTitle("Gyro X");
            gyroView2.SetTitle("Gyro Y");
            gyroView3.SetTitle("Gyro Z");

            orientationView1.SetTitle("Or X");
            orientationView2.SetTitle("Or Y");
            orientationView3.SetTitle("Or Z");

            InitEnvirons();

            osciTimer = new Timer(new TimerCallback(OsciTimer), null, Timeout.Infinite, osciInterval);
            osciTimer.Change(0, 33);
        }


        #region Initialization

        private void InitEnvirons()
        {
            DeviceInstance.notifyPropertyChangedDefault = true;

            environs = Environs.New(this, InitializedEvent, "SensorData", "Environs");
            if (environs == null)
                return;

            environs.SetIsLocationNode(true);

            environs.SetMediatorFilterLevel(MediatorFilter.AreaAndApp);

            environs.OnStatus += OnStatus;

            /// Attach to notification delegate, in order to receive notifications
            environs.OnNotify += OnNotification;

            environs.SetUseMediatorAnonymousLogon(true);

            deviceList = environs.CreateDeviceList(DeviceClass.All);
            deviceList.AddObserver(SensorDataWindow_CollectionChanged);

            environs.Start();
        }


        public ObservableCollection<DeviceInstance> userCollection
        {
            get { return (ObservableCollection<DeviceInstance>)deviceList.GetDevicesSource(); }
        }


        void SensorDataWindow_CollectionChanged(object sender, System.Collections.Specialized.NotifyCollectionChangedEventArgs args)
        {
            if (args.NewItems != null)
            {
                foreach (DeviceInstance device in args.NewItems)
                {
                    Debug.WriteLine("Appeared: " + device.ToString());

                    device.AddObserverForSensors(SensorDataObserver);
                }
            }
        }


        private void SensorDataObserver(environs.SensorFrame pack)
        {
            switch (pack.type)
            {
                case SensorType.Accelerometer:
                    accelView1.UpdateValue(pack.x);
                    accelView2.UpdateValue(pack.y);
                    accelView3.UpdateValue(pack.z);
                    break;
                case SensorType.MagneticField:
                    magneticView1.UpdateValue(pack.x);
                    magneticView2.UpdateValue(pack.y);
                    magneticView3.UpdateValue(pack.z);
                    break;
                case SensorType.Gyroscope:
                    gyroView1.UpdateValue(pack.x);
                    gyroView2.UpdateValue(pack.y);
                    gyroView3.UpdateValue(pack.z);
                    break;

                default:
                    if (pack.type == SensorType.Location)
                    {
                        orientationView1.UpdateValue(pack.x); // latitude
                        orientationView2.UpdateValue(pack.y); // longitude
                        orientationView3.UpdateValue(pack.z); // altitude
                        break;
                    }

                    if (pack.type == SensorType.MotionGravityAcceleration)
                    {
                        //orientationView1.UpdateValue(pack.f1); // gravity
                        //orientationView2.UpdateValue(pack.f2); // gravity
                        //orientationView3.UpdateValue(pack.f3); // gravity

                        orientationView1.UpdateValue(pack.x); // linear acc
                        orientationView2.UpdateValue(pack.y); // linear acc
                        orientationView3.UpdateValue(pack.z); // linear acc
                        break;
                    }

                    orientationView1.UpdateValue(pack.x); // Light in Lux
                    orientationView2.UpdateValue(pack.y);
                    orientationView3.UpdateValue(pack.z);
                    break;
            }
        }

        private void InitializedEvent()
        {
            /**
             * In order to receive this event, this handler has to be given to the environs instance, e.g. in this example, it is done as part of the instance constructor (within the SurfaceMediwView constructor)
             * environs = new SurfaceEnvirons(this, InitializedEvent);
             */
            Utils.Log(1, "InitializedEvent: Environs is now initialized");
            
            UpdateUI();
        }

        #endregion

        internal void AppPreviewKeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.T)
            {
            }
        }

        #region Environs notifications

        private void OnStatus(Status status)
        {
            UpdateEnvironsStatus();
        }


        private void OnNotification(ObserverNotifyContext ctx)
        {
            if (Utils.Log(3))
                Utils.Log(className, "Notification: [0x" + ctx.destID.ToString("X") + "] [" + ctx.sourceIdent + "] [" + Environs.resolveName(ctx.notification) + "]");

            int notifType = (ctx.notification & Environs.MSG_NOTIFY_CLASS);

            switch (notifType)
            {
                case Environs.NOTIFY_TYPE_CONNECTION:
                    UpdateUI();
                    break;

                case Environs.NOTIFY_TYPE_ENVIRONS:
                UpdateEnvironsStatus ();
                    break;
            }
        }


        Thread updateEnvironsStatusThread = null;

        private void UpdateEnvironsStatusThreaded()
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
                        Title = "SensorData 0x" + environs.GetDeviceID().ToString("X");
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

        private void UpdateEnvironsStatus()
        {
            lock (this)
            {
                if (updateEnvironsStatusThread != null)
                    return;

                updateEnvironsStatusThread = new Thread(UpdateEnvironsStatusThreaded);
                if (updateEnvironsStatusThread == null)
                    Utils.LogE("updateEnvironsStatus: Failed to create updateEnvironsStatusThread!!!");
                else
                    updateEnvironsStatusThread.Start();
            }
        }
        
        private void UpdateUI()
        {
            UpdateEnvironsStatus();
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
                UpdateEnvironsStatus();
            }
        }

        #endregion


        private void OsciTimer(Object stateInfo)
        {
            accelView1.IncreaseTimer();
            accelView2.IncreaseTimer();
            accelView3.IncreaseTimer();

            magneticView1.IncreaseTimer();
            magneticView2.IncreaseTimer();
            magneticView3.IncreaseTimer();

            gyroView1.IncreaseTimer();
            gyroView2.IncreaseTimer();
            gyroView3.IncreaseTimer();

            orientationView1.IncreaseTimer();
            orientationView2.IncreaseTimer();
            orientationView3.IncreaseTimer();
        }


        private void userList_SelectedCellsChanged(object sender, SelectedCellsChangedEventArgs e)
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

    }
}

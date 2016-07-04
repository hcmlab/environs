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
using System.IO;
using System.Windows;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using Microsoft.Surface.Presentation.Controls;
using System.Diagnostics;
using System.Threading;
using System.Collections;
using System.Collections.ObjectModel;
using environs;

#if SURFACE2
using Microsoft.Surface.Presentation.Input;
#else
using Microsoft.Surface.Core;
using Microsoft.Surface.Presentation.Controls.ContactVisualizations;
#endif

namespace environs.Apps
{
    /// <remarks>
    /// Interaction logic for SurfaceMediaBrowser.xaml that is common for both surfaces.
    /// </remarks>
    public partial class SurfaceMediaBrowser : SurfaceWindow
    {
        private const string className = "MediaBrowser";

        private int useViewerMode = 0;

        /// <summary>
        /// The device environment object that automatically handles the environment stuff.
        /// </summary>
        private Environs environs = null;

        /// <summary>
        /// Determines whether the pictures on the SurfaceMediaBrowser should be ordered.
        /// </summary>
        private bool orderImages = true;
        private DeviceList deviceList = null;

        private const string applicationname = "MediaBrowser";

        public string ApplicationName { get { return applicationname; } }

        #region MediaViewer common (Surface 2 and Surface 1) initialization

        /// <summary>
        /// Initialize Environs
        /// </summary>
        private void InitEnvirons()
        {
            /// Instantiate surface environment as part of the device environment
            environs = Environs.CreateInstance(this, InitializedEvent, "MediaBrowser", "Environs");
            if (environs == null)
                return;

            // Identify us as surface with the ID myDeviceID
            //Environs.SetDeviceID(0xDD);

            environs.SetUseFullscreen(true);
            environs.SetUseLogFile(true);

            //environs.SetMediatorFilterLevel(MediatorFilter.None);
            environs.SetMediatorFilterLevel(MediatorFilter.AreaAndApp);

            environs.SetUseDefaultMediator(true);
            /// Use custom mediator
            //environs.SetUseDefaultMediator(false);
            //environs.SetMediator("137.250.171.18", 5899);
            //environs.SetUseCustomMediator(true);

            environs.SetUseTouchRecognizer("libEnv-RecGestureThreeFinger", true);

            //environs.SetUseTouchRecognizer("libEnv-RecMouseSimulatorWin", true);
            //environs.SetUseTouchRecognizer("libEnv-RecTouchVisualizer", true);

            environs.SetUseMouseEmulation(false);
            environs.SetUseTouchVisualization(false);

            environs.SetUsePortalDefaultModules();

            // Enable Windows Media Foundation Encoder
            //environs.SetUseEncoder("libEnv-EncMfH264");

            environs.SetUseMediatorAnonymousLogon(true);
            //environs.SetMediatorUserName("t@t.t");

            /// Attach to message delegate, in order to display environment messages on the media view
            environs.OnMessage += OnMessage;

            /// Attach to message delegate, in order to display messages from devices that we are not connected to
            environs.OnMessageExt += OnMessageExt;

            /// Attach to data delegate, in order to receive data sent from other devices
            environs.OnData += OnData;

            environs.OnStatus += OnStatus;

            /// Attach to notification delegate, in order to receive notifications
            environs.OnNotify += OnNotification;

            /// Attach to notification delegate, in order to receive notifications
            environs.OnStatusMessage += OnStatusMessage;

            /// Enable a custom surface tracker module. Instruct Environs to deliver raw images to this module.
            /// Only one module can be used as a sink for raw images fed by Environs
            //environs.TrackerModule = @"libEnv-TrackSurface";
            //environs.SetUseTracker(Environs.TrackerModule);

            /// Enable a custom tracker module. Environs does not need to feed this module with images
            //environs.SetUseTracker("libEnv-TrackKinect");

            deviceList = environs.CreateDeviceList(DeviceClass.All);
            deviceList.AddObserver(DevicesAvailable_Changed);
            
            deviceList.Reload();

            //environs.debug_keys = true;

            environs.Start();

            DeviceItem.tablet = new BitmapImage(new Uri("pack://application:,,,/Resources/Tablet.png"));
            if (DeviceItem.tablet!= null) DeviceItem.tablet.Freeze();
            DeviceItem.smartphone = new BitmapImage(new Uri("pack://application:,,,/Resources/Smartphone.png"));
            if (DeviceItem.smartphone != null) DeviceItem.smartphone.Freeze();
            DeviceItem.monitor = new BitmapImage(new Uri("pack://application:,,,/Resources/Display.png"));
            if (DeviceItem.monitor != null) DeviceItem.monitor.Freeze();

            DeviceItem.surface1 = new BitmapImage(new Uri("pack://application:,,,/Resources/Surface_1.png"));
            if (DeviceItem.surface1 != null) DeviceItem.surface1.Freeze();
            DeviceItem.surface2 = new BitmapImage(new Uri("pack://application:,,,/Resources/Surface_1.png"));
            if (DeviceItem.surface2 != null) DeviceItem.surface2.Freeze();
            DeviceItem.tabletop = new BitmapImage(new Uri("pack://application:,,,/Resources/Surface_1.png"));
            if (DeviceItem.tabletop != null) DeviceItem.tabletop.Freeze();
        }


        private void DevicesAvailable_Changed(object sender, System.Collections.Specialized.NotifyCollectionChangedEventArgs args)
        {
            if (args.OldItems != null)
            {
                foreach (DeviceInstance item in args.OldItems)
                {
                    DeviceItem.DeviceDisappeared(item.deviceID, item.areaName, item.appName);

                    item.RemoveObserverForData(DataObserver);
                    item.RemoveObserverForMessages(MessageObserver);
                }
            }

            if (args.NewItems != null)
            {
                Environs.dispatchSync(new Action(delegate()
                {
                    foreach (DeviceInstance item in args.NewItems)
                    {
                        item.OnPortalRequestOrProvided += OnPortalRequestOrProvided;

                        DeviceItem devItem = DeviceItem.DeviceAppeared(environs, item, MainContent.Items);
                        if (devItem != null)
                        {
                            devItem.portalObserver += PortalObserver;

                            item.AddObserverForData(DataObserver);
                            item.AddObserverForMessages(MessageObserver);
                        }
                    }
                }));
            }
        }


#if SURFACE2
        /// <summary>
        /// A TagVisualizationDefinition that matches all tags.
        /// </summary>
        private class MatchThemAll : TagVisualizationDefinition
        {
            protected override bool Matches(TagData tag)
            {
                return true;
            }

            protected override Freezable CreateInstanceCore()
            {
                return new MatchThemAll();
            }
        }
#endif

        /// <summary>
        /// Initialization of Media View within a thread that is common for both surfaces.
        /// This method also includes creation and initialization of the device environment.
        /// </summary>
        private void InitMediaView()
        {
            Thread init = new Thread(InitThread);
            if (init != null)
                init.Start();
            else
            {
                string errmsg = "[EROR_HALT]: Initialization Error - Failed to create initialization thread for building the media view!";
                Debug.WriteLine(errmsg);
                throw new InvalidProgramException(errmsg);
            }
        }


        /// <summary>
        /// Initialization thread for preparing the media view with its content.
        /// </summary>
        public void InitThread()
        {
            Utils.Log(2, className, "Threaded initialization started...");

            string[] photosFiles = null;

            if (useViewerMode == 1 && Directory.Exists(@".\plans"))
            {
                if (File.Exists(@".\plans\plan0.png"))
                {
                    Environs.dispatch(new Action(delegate()
                    {
                        this.Background = new ImageBrush(new BitmapImage(new Uri(Directory.GetCurrentDirectory() + @".\plans\plan0.png", UriKind.Absolute)));

                        if (g_sviInfoMessage != null)
                        {
                            if (g_sviInfoMessage.Visibility == Visibility.Visible)
                                g_sviInfoMessage.Visibility = Visibility.Hidden;
                        }
                        if (g_sviLatencyIndicator != null)
                        {
                            if (g_sviLatencyIndicator.Visibility == Visibility.Visible)
                                g_sviLatencyIndicator.Visibility = Visibility.Hidden;
                        }
                    }));
                }
                photosFiles = Directory.GetFiles(@".\plans");
            }
            else
            {
                Environs.dispatch(new Action(delegate()
                {
                    this.Background = System.Windows.Media.Brushes.Black;

                    if (g_sviInfoMessage != null)
                    {
                        if (g_sviInfoMessage.Visibility == Visibility.Hidden)
                            g_sviInfoMessage.Visibility = Visibility.Visible;
                    }
                }));

                if (useViewerMode == 2 && Directory.Exists(@".\med"))
                {
                    photosFiles = Directory.GetFiles(@".\med");
                }
                else
                {
                    // Check whether pictures in public exists
                    if (!Directory.Exists(@"C:\Users\Public\Pictures"))
                    {
                        Directory.CreateDirectory(@"C:\Users\Public\Pictures");
                        Directory.CreateDirectory(@"C:\Users\Public\Pictures\Sample Pictures");
                    }
                    // Check whether sample pictures exist
                    else if (!Directory.Exists(@"C:\Users\Public\Pictures\Sample Pictures"))
                    {
                        Directory.CreateDirectory(@"C:\Users\Public\Pictures\Sample Pictures");
                    }

                    photosFiles = Directory.GetFiles(@"C:\Users\Public\Pictures\Sample Pictures");
                }
            }

            if (photosFiles != null)
            {
                Environs.dispatch(new Action(delegate()
                {
                    StatusMessageReceived(photosFiles);
                }));
            }
            else
                Utils.LogE(className, "InitThread: No photo files for MediaViewer found.");

            Utils.Log(1, className, "InitThread: Initialization thread done.");
        }


        #endregion


        #region Howto get notified, when environs has finished initialization of the environs instance

        private void InitializedEvent()
        {
            /*
             * In order to receive this event, this handler has to be given to the environs instance, e.g. in this example, it is done as part of the instance constructor (within the SurfaceMediwView constructor)
             * environs = new SurfaceEnvirons(this, InitializedEvent);
             */

            Utils.Log(2, className, "InitializedEvent: Environs is now initialized");
            /*
             * Now environs is initialized and ready to use. Furthermore the ui window is visible and fullscreen if requested to do so.
             */
            Environs.dispatch(new Action(delegate()
            {
                environs.InitLatencyIndicator(g_tbLatencyIndicator);
            }));
        }

        #endregion


        # region Howto send bulk data
        private bool SendByteBuffer()
        {
            return false;
        }
        #endregion


        # region Howto send bitmap bytes of a memory bitmap to a device

        private bool SendImageBuffer()
        {
            ////lock the original bitmap in memory
            //System.Drawing.Imaging.BitmapData originalData = bmp.LockBits(
            //   new System.Drawing.Rectangle(0, 0, bmp.Width, bmp.Height),
            //   System.Drawing.Imaging.ImageLockMode.ReadOnly, System.Drawing.Imaging.PixelFormat.Format24bppRgb);

            //int size = originalData.Stride * originalData.Height;
            //return Environs.SendBuffer(Environs.debug_device_tagID, 1234, "noname", originalData.Scan0, (uint)size);
            ////return false;
            return false;
        }

        #endregion


        void AddImageBuffer (byte[] buffer)
        {
            if (buffer == null)
                return;
            Environs.dispatch(new Action(delegate ()
            {
                // We are the UI-thread, so no need to dispatch
                MemoryStream byteStream = new MemoryStream(buffer);

                try
                {
                    BitmapImage image = new BitmapImage();
                    image.BeginInit();
                    image.StreamSource = byteStream;
                    image.EndInit();


                    System.Windows.Controls.Image imageWpf = new System.Windows.Controls.Image();
                    imageWpf.Source = (ImageSource)image;
                    image.Freeze();

                    ScatterViewItem svi = null;
                    svi = new ScatterViewItem();
                    svi.Content = imageWpf;
                    svi.BorderBrush = System.Windows.Media.Brushes.Red;
                    svi.BorderThickness = new Thickness(4);

                    // add the new ScatterViewItem to the ScatterView canvas
                    MainContent.Items.Add(svi);
                }
                catch (Exception)
                {

                }
                /*
                // This example shows how to retrieve the received data from data pointer
                // This is not recommended, since it blocks receiving of further data at that time.
                // It's much better to dispatch it to a thread which uses GetFile as exemplarily shown above.
                // This allows environs to receive further files in parallel.
                byte[] dataBytes = new byte[size];
                Marshal.Copy(data, dataBytes, 0, (int)size);

                MemoryStream byteStream = new MemoryStream(dataBytes);


                BitmapImage image = new BitmapImage();
                image.BeginInit();
                image.StreamSource = byteStream;
                image.EndInit();


                System.Windows.Controls.Image imageWpf = new System.Windows.Controls.Image();
                imageWpf.Source = (ImageSource)image;

                ScatterViewItem svi = null;
                svi = new ScatterViewItem();
                svi.Content = imageWpf;
                svi.BorderBrush = System.Windows.Media.Brushes.Red;
                svi.BorderThickness = new Thickness(4);

                // add the new ScatterViewItem to the ScatterView canvas
                MainContent.Items.Add(svi);
                 */
            }));
        }


        #region Device message handling

        public void MessageObserver(MessageInstance messageInst, MessageInfoFlag changedFlags)
        {
        }

        public void DataObserver(FileInstance fileInst, FileInfoFlag changedFlags)
        {
            if (changedFlags != FileInfoFlag.Available)
                return;

            string fn = fileInst.descriptor;
            
            if (fileInst.size > 0 && (fn.Contains(".jpg") || fn.Contains(".jpeg") || fn.Contains(".png") || fn.Contains(".bmp")))
            {
                if (Utils.Log(4))
                    Utils.Log(className, "File has been stored to " + fileInst.path);

                AddImageBuffer(fileInst.data);
            }
        }

        #endregion

        #region Portal Observer implementation

        private void OnPortalRequestOrProvided(PortalInstance portal)
        {
            if (portal == null)
                return;

            // If we don't add an observer, then the portal will be discarded
            portal.AddObserver(PortalObserver);

            portal.startIfPossible = true;

            // A portal request came in.
            // We establish the portal and Start in the observer handler
            portal.Establish(false);
        }


        void PortalObserver(PortalInstance portal, Notify.Portale notify)
        {
            int notifType = ((int)notify & Environs.MSG_NOTIFY_CLASS);

            if (notify == Notify.Portale.Disposed)
            {
                DeviceVisual visual = (DeviceVisual)portal.appContext1;
                if (visual != null)
                    visual.Remove();
            } 
            else if (notify == Notify.Portale.ProviderReady)
            {
                portal.Start();
            }
            else if (notify == Notify.Portale.IncomingEstablished)
            {
                portal.Start();
            } 
            else if (notifType == Environs.NOTIFY_TYPE_PORTAL)
            {
                switch (notify)
                {
                    case Notify.Portale.ProvideStreamAck:
                    case Notify.Portale.ProvideImagesAck:
                        HandlePortalProvideAcknoledged(portal);
                        break;

                    case Notify.Portale.StreamStarted:
                        HandlePortalStarted(portal);
                        break;

                    case Notify.Portale.StreamStopped:
                        HandlePortalStopped(portal);
                        break;

                    case Notify.Portale.StreamIncoming:
                        HandleIncomingPortal(portal);
                        break;
                }
            }
            else if (notifType == Environs.NOTIFY_TYPE_OPTIONS)
            {
                switch (notify)
                {
                    case Notify.Portale.LocationChanged:
                    case Notify.Portale.SizeChanged:
                        HandlePortalChanged(portal, notify);
                        break;
                }
            }
            else if (notify == Notify.Portale.ContactChanged)
            {
                HandlePortalOnSurfaceChanged(portal);
            }
        }
        #endregion


        #region Howto receive data/files from other devices or environs

        /// <summary>
        /// Event handler that gets called when data from environs is available.
        /// </summary>
        /// <param name="source">The device id that this data is sent from. 0 means that the data is an internal data from this environs instance.</param>
        /// <param name="sourceType">The data type: ENVIRONS_SOURCE_NATIVE = data from environs, ENVIRONS_SOURCE_DEVICE = data was sent from a device.</param>
        /// <param name="msg">The data buffer.</param>
        private void OnData(ObserverDataContext ctx)
        {
            /*
             * In order to receive environs data, this handler has to be attached to a delegate of th environs instance, e.g. in this example, it is done in the InitThread
             * environs.OnData += OnData;
             */
            if (Utils.Log(6))
                Utils.Log(className, "Received data from nativeID [0x" + ctx.nativeID.ToString("X") + "] of size [" + ctx.size + "] with descriptor [" + ctx.descriptor + "]");

                Source source = (Source)ctx.type;

            if (source == Source.Native || source == Source.Device)
            {
                if (Utils.Log(4))
                    Utils.Log(className, "Received data from device [0x" + ctx.nativeID.ToString("X") + "] of size [" + ctx.size + "] with descriptor [" + ctx.descriptor + "]");

                string fn = ctx.descriptor.ToLower();

                if (ctx.size > 0 && (fn.Contains(".jpg") || fn.Contains(".jpeg") || fn.Contains(".png") || fn.Contains(".bmp")))
                {
                    String filepath = environs.GetFilePathNative(ctx.nativeID, ctx.fileID);
                    if (Utils.Log(4))
                        Utils.Log(className, "File has been stored to " + filepath);

                    int size = 0;
                    byte[] buffer = environs.GetFile ( true, ctx.nativeID, 0, null, null, ctx.fileID, ref size );

                    AddImageBuffer(buffer);
                }
            }
        }

        #endregion


        /// <summary>
        /// Receive notification from Environs and dispatch them to appropriate handlers
        /// </summary>
        #region Howto receive notifications from environs

        private void OnStatus(Status status)
        {
        }

        private void OnNotification(ObserverNotifyContext ctx)
        {
            //Utils.Log(1, String.Format("OnNotification: deviceID [0x" + deviceID.ToString("X") + "]\t source [{0}]\t[{1}]", sourceIdent, Environs.resolveName(notification)));

        }

        #endregion


        /// <summary>
        /// Handling of portal related notifications of Environs.
        /// Create or removes visuals to indicate connected devices and their established portals.
        /// </summary>
        #region Portal message handling

        private void HandlePortalStarted(PortalInstance portal)
        {
            DeviceVisual visual = (DeviceVisual)portal.appContext1;
            if (visual == null)
                return;

            if (portal.outgoing)
                visual.UpdateContactStatus();
        }


        private void HandlePortalStopped(PortalInstance portal)
        {
            if (!portal.outgoing)
                portal.ReleaseRenderCallback();

            DeviceVisual visual = (DeviceVisual)portal.appContext1;
            if (visual == null)
                return;

            visual.Hide();
        }


        private void HandlePortalProvideAcknoledged(PortalInstance portal)
        {
            //Utils.Log(4, className, "HandlePortalProvideAcknoledged: portal " + portal.portalID);

            DeviceVisual visual = DeviceVisual.GetOrCreate(portal, MainContent.Items);
            if (visual == null)
                return;

            PortalInfoBase info = portal.GetIPortalInfo();
            if (info == null)
                return;

            if (Utils.Log(2))
                Utils.Log(className, String.Format("HandlePortalProvideAcknoledged: Updating device id [0x" + portal.device.deviceID.ToString("X") + "]. x[{0}], y[{1}], w[{2}], h[{3}], a[{4}]",
                info.centerX, info.centerY, info.width, info.height, info.orientation));

            Environs.dispatch(new Action(delegate()
            {
                visual.RebuildVisual();

                visual.Orientation(info.orientation);
                visual.Position(new System.Windows.Point((double)info.centerX, (double)info.centerY));
                visual.Size(info.width, info.height);
            }));
        }


        private void HandleIncomingPortal(PortalInstance portal)
        {
            if (Utils.Log(4))
                Utils.Log(className, "HandleIncomingPortal: portal " + portal.portalID);

            DeviceVisual visual = DeviceVisual.GetOrCreate(portal, MainContent.Items);
            if (visual == null)
                return;

            visual.Show();

//            if (!portal.SetRenderSurface((Control)visual.drawingContent, 0, 0))
//                return;
//            if (!portal.SetRenderCallback(visual.portalSinkHandler))
//                return;
            if ( !portal.SetRenderCallback(visual.Renderer, 0, 0))
                return;

            DeviceDisplay display = portal.device.display;

            // Set device width to 512 and keep aspect ratio
            int width = 512;
            int height = width * display.width / display.height;
            visual.UpdateSize(width, height);
            portal.Start();
        }


        private void HandlePortalChanged(PortalInstance portal, Notify.Portale notify)
        {
            if (Utils.Log(4))
                Utils.Log(className, "HandlePortalChanged: portal " + portal.portalID);

                DeviceVisual visual = (DeviceVisual)portal.appContext1;
            if (visual == null)
                return;

            if (notify == Notify.Portale.LocationChanged)
                visual.UpdatePosition(portal.GetIPortalInfo());
            else
                visual.UpdateSize(portal.info);
        }


        private void HandlePortalOnSurfaceChanged(PortalInstance portal)
        {
            if (Utils.Log(4))
                Utils.Log(className, "HandlePortalOnSurfaceChanged: deviceID [0x" + portal.device.deviceID.ToString("X") + "] is NOT on surface");

            DeviceVisual visual = (DeviceVisual)portal.appContext1;
            if (visual == null)
            {
                Utils.LogE(className, "HandlePortalOnSurfaceChanged: deviceID [0x" + portal.device.deviceID.ToString("X") + "] not found as visual, while it seems to be connected.");
                return;
            }

            if (portal.device.directStatus == 1)
            {
                visual.Hide();
                return;
            }

            if (portal.outgoing && portal.status < PortalStatus.CreatedAskRequest)
            {
                if (!visual.isActiveItemVisual)
                    return;
            }
            visual.UpdateSize(portal.info);
            visual.Show();
        }

        #endregion


        /// <summary>
        /// Receive message from other devices and show them on the surface.
        /// </summary>
        #region Howto receive messages from other devices or environs

        private void addInfoText(String text)
        {
            if (text == null)
                return;

            Environs.dispatch(new Action(delegate()
            {
                String old = g_tbInfoMessage.Text;
                g_tbInfoMessage.Text = old + "\n" + text;

                g_svInfoMessage.ScrollToEnd();
            }));
        }

        /// <summary>
        /// Event handler that gets called when environment messages are available.
        /// In this example, it appends the message to the messages-textbox within the UI-thread.
        /// </summary>
        /// <param name="source">The device id that this message is sent from. 0 means that the message is an internal message from this environs instance.</param>
        /// <param name="sourceType">The message type: ENVIRONS_SOURCE_NATIVE = message from environs, ENVIRONS_SOURCE_DEVICE = message was sent from a device.</param>
        /// <param name="msg">The message from the environment.</param>
        private void OnMessage(ObserverMessageContext ctx)
        {
            /*
             * In order to receive environs messages, this handler has to be attached to a delegate of th environs instance, e.g. in this example, it is done in the InitThread
             * environs.OnMessage += OnMessage;
             */
            String msg = ctx.message;

            if (ctx.sourceType == (int)Source.Device)
            {
                if (Utils.Log(4))
                    Utils.Log(className, "Received message from device [0x" + ctx.destID.ToString("X") + "]: " + msg);

                if (g_tbInfoMessage == null)
                    return;

                addInfoText(msg);
            }
            else //if (sourceType == Types.ENVIRONS_SOURCE_NATIVE)
            {
                if (msg.StartsWith("K:"))
                {
                    if (msg.Length >= 3)
                    {
                        char key = msg[2];
                        if (key == 'c')
                        {
                            useViewerMode++;
                            if (useViewerMode > 2)
                                useViewerMode = 0;
                            InitMediaView();
                        }
                        else if (key == 'o')
                        {
                            orderImages = !orderImages;
                            InitMediaView();
                        }
                        else if (key == 'a')
                        {
                            if (g_sviInfoMessage != null)
                            {
                                if (g_sviInfoMessage.Visibility == Visibility.Hidden)
                                    g_sviInfoMessage.Visibility = Visibility.Visible;
                                else
                                    g_sviInfoMessage.Visibility = Visibility.Hidden;
                            }
                        }
                        else if (key == 'l')
                        {
                            if (g_sviLatencyIndicator != null)
                            {
                                if (g_sviLatencyIndicator.Visibility == Visibility.Hidden)
                                    g_sviLatencyIndicator.Visibility = Visibility.Visible;
                                else
                                    g_sviLatencyIndicator.Visibility = Visibility.Hidden;
                            }
                        }
                        else if (key == 'b')
                        {
                            // Load devicesAll Nearby
                            Collection<DeviceInstance> devices = deviceList.GetDevicesNearby();

                            if (devices == null)
                            {
                                if (Utils.Log(4))
                                    Utils.Log(className, "OnMessage: no devices nearby");
                            }
                            else
                            {
                                if (g_tbInfoMessage != null)
                                {
                                    String deviceText = "";
                                    for (int i = 0; i < devices.Count; i++)
                                    {
                                        deviceText += devices[i] + "\n";
                                        Utils.Log(className, "[INFO] Device " + i + " " + devices[i]);
                                    }
                                    addInfoText(deviceText);
                                }
                            }
                            // Load devicesAll from Mediator
                            devices = deviceList.GetDevicesFromMediator();
                            if (devices == null)
                            {
                                if (Utils.Log(4))
                                    Utils.Log(className, "OnMessage: no devices at mediator");
                                return;
                            }
                            else
                            {
                                if (g_tbInfoMessage != null)
                                {
                                    String deviceText = "";
                                    for (int i = 0; i < devices.Count; i++)
                                    {
                                        deviceText += devices[i] + "\n";
                                        if (Utils.Log(4))
                                            Utils.Log(className, "[INFO] Device " + i + " " + devices[i]);
                                    }
                                    addInfoText(deviceText);
                                }
                            }
                        }
                        else if (key == 'w')
                        {
                            //DeviceInstance device = deviceList.GetDeviceBestMatch(Environs.debug_device_tagID);
                            //if (device != null)
                            //{
                            //    byte[] buffer = device.GetFile(1000);
                            //    if (buffer != null)
                            //    {
                            //        // We are the UI-thread, so no need to dispatch
                            //        MemoryStream byteStream = new MemoryStream(buffer);

                            //        BitmapImage image = new BitmapImage();
                            //        image.BeginInit();
                            //        image.StreamSource = byteStream;
                            //        image.EndInit();


                            //        System.Windows.Controls.Image imageWpf = new System.Windows.Controls.Image();
                            //        imageWpf.Source = (ImageSource)image;

                            //        ScatterViewItem svi = null;
                            //        svi = new ScatterViewItem();
                            //        svi.Content = imageWpf;
                            //        svi.BorderBrush = System.Windows.Media.Brushes.Red;
                            //        svi.BorderThickness = new Thickness(4);

                            //        // add the new ScatterViewItem to the ScatterView canvas
                            //        MainContent.Items.Add(svi);
                            //    }

                            //}
                        }
                    }
                    return;
                }

                if (g_tbInfoMessage == null)
                    return;

                addInfoText(msg);
            }
        }

        private void OnMessageExt(ObserverMessageContext ctx)
        {
            if (Utils.Log(4))
                Utils.Log(className, "Received message from device [0x" + ctx.destID.ToString("X") + "]: " + ctx.message);

            if (g_tbInfoMessage == null)
                return;

            addInfoText(ctx.message);
        }
        #endregion


        #region Howto receive status messages from environs

        private string MessageReceivedCache = null;

        /// <summary>
        /// Event handler that gets called when environment messages are available.
        /// In this example, it appends the message to the messages-textbox within the UI-thread.
        /// </summary>
        /// <param name="source">The device id that this message is sent from. 0 means that the message is an internal message from this environs instance.</param>
        /// <param name="sourceType">The message type: ENVIRONS_MSG_SOURCE_NATIVE = message from environs, ENVIRONS_MSG_SOURCE_DEVICE = message was sent from a device.</param>
        /// <param name="msg">The message from the environment.</param>
        private void OnStatusMessage(string msg)
        {
            if (g_tbInfoMessage == null)
            {
                if (MessageReceivedCache == null)
                    MessageReceivedCache = "";
                MessageReceivedCache += "\n" + msg;
                return;
            }

            Environs.dispatch(new Action(delegate()
            {
                String old = g_tbInfoMessage.Text;
                if (MessageReceivedCache != null)
                {
                    old += "\n" + MessageReceivedCache;
                    MessageReceivedCache = null;
                }
                g_tbInfoMessage.Text = old + "\n" + msg;

                g_svInfoMessage.ScrollToEnd();
            }));
        }

        #endregion

        #region Surface Window UI
        /// <summary>
        /// Loading pictures from the public folder and show them in the media view (wrapped into ScatterViewItems).
        /// </summary>
        /// <param name="photosFiles"></param>
        private void StatusMessageReceived(string[] photosFiles)
        {
            g_add.Center = new System.Windows.Point(this.Width - g_add.Width / 2 - 60, 70);
            g_bin.Center = new System.Windows.Point(this.Width - g_bin.Width / 2 - 60, this.Height - g_bin.Height / 2 - 70);

            ArrayList toRemove = new ArrayList();

            foreach (ScatterViewItem svi in MainContent.Items)
            {
                if (svi.Tag == null)
                    continue;
                String tag = (String)svi.Tag;
                if (tag == null)
                    continue;
                if (tag.ToLower().Contains(".jpg") || tag.ToLower().Contains(".png"))
                {
                    toRemove.Add(svi);
                }
            }

            foreach (ScatterViewItem svi in toRemove)
            {
                MainContent.Items.Remove(svi);
            }

            orderX = 400;
            orderWidth = 400;
            orderY = 200;

            AddPicturesToScatterViewItem(photosFiles);
        }


        double orderX = 400;
        double orderWidth = 400;
        double orderY = 200;
        double orderYstarted;

        private void AddPicturesToScatterViewItem(string[] photosFiles)
        {
            ScatterViewItem svi = null;
            orderYstarted = orderY;

            //g_lbDeviceList.SelectionChanged
            foreach (string pic in photosFiles)
            {
                //if it is a jpeg/png picture 
                if (pic.ToLower().EndsWith(".jpg") || pic.ToLower().EndsWith(".png"))
                {
                    svi = new ScatterViewItem();

                    //set the style for the scatterItem
                    //svi.Style = (Style)FindResource("SVI_Style");
                    // load the file into an Image object to use as the content of this ScatterViewItem
                    svi.Content = LoadImageFromPath(pic);
                    svi.Tag = new String(pic.ToCharArray());

                    //svi.PreviewMouseRightButtonUp += itemRightMouseUp;

                    //add a border 
                    svi.BorderBrush = System.Windows.Media.Brushes.Beige;
                    svi.BorderThickness = new Thickness(1);

                    if (orderImages)
                    {
                        svi.Orientation = 0;
                        //svi.Width = orderWidth;
                        svi.Center = new System.Windows.Point(orderX + (orderWidth / 2), orderY);
                        orderX += orderWidth + 30;
                        if (orderX + (orderWidth / 2) >= this.Width)
                        {
                            orderX = 100;
                            orderY += 250;
                            if (orderY > this.Height)
                            {
                                orderYstarted += 100;
                                orderY = orderYstarted;
                            }
                        }
                    }

                    svi.PreviewMouseLeftButtonDown += new System.Windows.Input.MouseButtonEventHandler(this.dragStart);
                    svi.PreviewMouseLeftButtonUp += new System.Windows.Input.MouseButtonEventHandler(this.delete);
#if SURFACE2
                    svi.PreviewTouchDown += new System.EventHandler<System.Windows.Input.TouchEventArgs>(this.dragStart);
                    svi.PreviewTouchUp += new System.EventHandler<System.Windows.Input.TouchEventArgs>(this.delete);
#else
                    svi.PreviewContactDown += this.dragStart;
                    svi.PreviewContactUp += this.delete;
#endif
                    //svi.PreviewMouseMove += new System.Windows.Input.MouseEventHandler(this.List_MouseMove);
                    // add the new ScatterViewItem to the ScatterView canvas
                    MainContent.Items.Add(svi);
                    // change the border when the scatterItem is selected
                    //svi.Activated += new RoutedEventHandler(ScatterViewItem_Activated);
                    //svi.Deactivated += new RoutedEventHandler(ScatterViewItem_Deactivated);

                    //int c_x = (int)svi.Center.X;
                    //if (svi.ActualCenter.X - svi.ActualWidth < logRight)
                    //    c_x += logRight;
                    //int c_y = (int)svi.Center.Y;
                    //if (svi.ActualCenter.Y - svi.ActualHeight < logBottom)
                    //    c_y += logBottom;

                    //svi.Center = new System.Windows.Point(c_x, c_y);
                }
            }

        }

        private ScatterViewItem scaterviewitem = null;

        private void dragStart(object sender, InputEventArgs e)
        {
            scaterviewitem = sender as ScatterViewItem;
        }

        private void delete(object sender, InputEventArgs e)
        {
            if (scaterviewitem != null)
            {
                double x;
                double y;
                if (e is MouseEventArgs)
                {
                    x = (e as MouseEventArgs).GetPosition(g_bin).X;
                    y = (e as MouseEventArgs).GetPosition(g_bin).Y;

                }
                else
                {
#if SURFACE2
                    x = (e as TouchEventArgs).GetTouchPoint(g_bin).Position.X;
                    y = (e as TouchEventArgs).GetTouchPoint(g_bin).Position.Y;
#else
                    x = (e as Microsoft.Surface.Presentation.ContactEventArgs).GetPosition(g_bin).X;
                    y = (e as Microsoft.Surface.Presentation.ContactEventArgs).GetPosition(g_bin).Y;
#endif
                }

                if (x >= 0 && x <= g_bin.Width && y >= 0 && x <= g_bin.Height)
                {
                    MainContent.Items.Remove(scaterviewitem);
                }
                else
                {
                    // scaterviewitem = null;
                }
            }
        }


        /// <summary>
        /// Helper method that loads an image.
        /// </summary>
        /// <param name="path">The path to an image.</param>
        /// <returns>The image object of type <c>System.Windows.Controls.Image</c></returns>
        private static System.Windows.Controls.Image LoadImageFromPath(string path)
        {
            ImageSourceConverter converter = new ImageSourceConverter();
            System.Windows.Controls.Image image = new System.Windows.Controls.Image();
            image.Source = (ImageSource)converter.ConvertFromString(path);
            return image;
        }


        private double getDirection(double angle, double widthAngle)
        {
            if ((270 - widthAngle) <= angle && angle < (270 + widthAngle))
            {
                return 0;
            }
            else if ((180 - (90 - widthAngle)) <= angle && angle < (180 + (90 - widthAngle)))
            {
                return 270;
            }
            else if ((90 - widthAngle) <= angle && angle < (90 + widthAngle))
            {
                return 180;
            }
            else
            {
                return 90;
            }
        }


        private void addFile(object sender, InputEventArgs e)
        {
            Microsoft.Win32.OpenFileDialog openfiledialog = new Microsoft.Win32.OpenFileDialog();

            // openfiledialog.Title = "Choose a file that you want send";
            openfiledialog.Multiselect = true;

            // Set filter for file extension and default file extension 
            // openfiledialog.Filter = "All|*.jpeg;*.png;*.jpg;*.gif;*.bmp|JPG Files (*.jpg)|JPEG Files (*.jpeg)|*.jpeg|PNG Files (*.png)|*.png|*.jpg|GIF Files (*.gif)|*.gif|Bitmap Files (*.bmp)|*.bmp";
            openfiledialog.Filter = "All Image Files|*.jpeg;*.jpg|JPG Files (*.jpg)|*.jpg|JPEG Files (*.jpeg)|*.jpeg";

            if (openfiledialog.ShowDialog() == true)
            {
                AddPicturesToScatterViewItem(openfiledialog.FileNames);
            }
        }


        #endregion


    }

}
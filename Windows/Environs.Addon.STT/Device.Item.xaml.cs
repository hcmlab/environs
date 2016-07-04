using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using Microsoft.Surface.Presentation.Controls;


namespace environs
{
    /// <summary>
    /// Interaction logic for DeviceItem.xaml
    /// </summary>
    public partial class DeviceItem : UserControl
    {
        private const string className = "DeviceItem";

        Environs env = null;

        public static Dictionary<string, DeviceItem> deviceItems = new Dictionary<string, DeviceItem>();

        public int deviceID = 0;
        public string project = null;
        public string app = null;

        public DeviceInstance deviceInstance = null;
        public ScatterViewItem scatterItem = null;
        public ItemCollection items = null;

        DeviceDisplay display;

        public static Brush forgroundBrush = Brushes.LightGreen;

        private static int nextStartX = 0;

        public PortalObserver portalObserver = null;

        public static BitmapImage tablet = null;
        public static BitmapImage smartphone = null;
        public static BitmapImage monitor = null;
        public static BitmapImage surface1 = null;
        public static BitmapImage surface2 = null;
        public static BitmapImage tabletop = null;

        BitmapImage platformImage = null;

        public DeviceItem()
        {
            Utils.Log(4, className, "Construct");

            InitializeComponent();
        }

        public static String buildKey(int deviceID, String project, String app)
        {
            return deviceID + "_" + project + "_" + app;
        }


        public static DeviceItem DeviceAppeared(Environs env, DeviceInstance deviceInstance, ItemCollection itemCollection)
        {
            string key = buildKey(deviceInstance.deviceID, deviceInstance.areaName, deviceInstance.appName);

            if (deviceItems.ContainsKey(key))
                return null;

            DeviceItem device = new DeviceItem();

            device.deviceInstance = deviceInstance;

            device.items = itemCollection;
            device.env = env;

            device.display = (DeviceDisplay) env.GetDeviceDisplayProps(0, 0);
            device.scatterItem = new ScatterViewItem();
            device.scatterItem.Content = device;
            device.scatterItem.Opacity = 1f;
            device.scatterItem.IsEnabled = true;
            device.scatterItem.CanScale = true;
            device.scatterItem.CanMove = true;
            
            //device.scatterItem.Background = Brushes.Transparent;

            SolidColorBrush br = new SolidColorBrush(Color.FromArgb(0xE6, 0xBF, 0xBF, 0xBF));
            br.Opacity = 0.6f;
            device.scatterItem.Background = br;

            device.textDesc.Foreground = forgroundBrush;
            device.borderMain.BorderBrush = forgroundBrush;
            device.borderDesc.BorderBrush = forgroundBrush;

            device.IdentifyAs(key);
            return device;
        }


        public static void DeviceDisappeared(int deviceID, String project, String app)
        {
            String key = buildKey(deviceID, project, app);

            if (!deviceItems.ContainsKey(key))
                return;

            DeviceItem device = null;

            if (deviceItems.TryGetValue(key, out device))
            {
                // Remove old scatterItem at first
                if (device.items != null)
                {
                    Environs.dispatch(new Action(delegate()
                    {
                        device.items.Remove(device.scatterItem);
                    }));
                }
            }

            deviceItems.Remove(key);
        }


        private bool isPlatformType(Environs.Platforms platform)
        {
            int src = (int)deviceInstance.platform;
            int dst = (int)platform;
            return ((src & dst) == dst);
        }

        private void setPlatformImage()
        {
            if (isPlatformType(Environs.Platforms.Tablet_Flag))
                platformImage = tablet;
            else if (isPlatformType(Environs.Platforms.Smartphone_Flag))
                platformImage = smartphone;
            else if (isPlatformType(Environs.Platforms.MSSUR01))
                platformImage = surface1;
            else if (isPlatformType(Environs.Platforms.SAMSUR40))
                platformImage = surface2;
            else if (isPlatformType(Environs.Platforms.Tabletop_Flag))
                platformImage = surface2;
            else if (isPlatformType(Environs.Platforms.Display_Flag))
                platformImage = monitor;

            if (platformImage != null)
            {
                //ImageBrush ib = new ImageBrush(platformImage);
                //ib.Stretch = Stretch.None;
                //textDesc.Background = ib;

                imgPlatform.Source = platformImage;
                imgPlatform.Margin = new Thickness(5);
                imgPlatform.Visibility = Visibility.Visible;
            }
        }

        /**
         * Add the scatterItem to the internal collection.
         * 
         * @param		deviceID_    Device identifier.
         * @return		success
         */
        public bool IdentifyAs(string key)
        {
            buildDeviceDescription();
            setPlatformImage();

            progressConnect.Style = null;
            progressConnect.MinHeight = 5;

            if (deviceInstance != null)
                deviceInstance.AddObserver(DeviceChangeObserver);

            this.SizeChanged += deviceItem_SizeChanged;

#if SURFACE1
            this.btnPortal_in.PreviewContactUp += btnPortal_in_TouchUp;
            this.btnPortal_out.PreviewContactUp += btnPortal_out_TouchUp;
#else
            this.btnPortal_in.PreviewTouchUp += btnPortal_in_TouchUp;
            this.btnPortal_out.PreviewTouchUp += btnPortal_out_TouchUp;
            this.btnConnect.PreviewTouchUp += btnConnect_TouchUp;
#endif
            this.btnPortal_in.PreviewMouseUp += btnPortal_in_TouchUp;
            this.btnPortal_out.PreviewMouseUp += btnPortal_out_TouchUp;
            this.btnConnect.PreviewMouseUp += btnConnect_TouchUp;


            DeviceItem device = null;
            if (deviceItems.TryGetValue(key, out device))
            {
                // Remove old scatterItem at first
                deviceItems.Remove(key);
            }
            deviceItems.Add(key, this);

            Environs.dispatch(new Action(delegate()
            {
                if (device != null)
                    items.Remove(device.scatterItem);

                items.Add(scatterItem);
            }));

            return true;
        }

        private bool initPosition = true;

        void deviceItem_SizeChanged(object sender, SizeChangedEventArgs e)
        {          
            if (imgPlatform.Visibility != Visibility.Visible)
                return;

            double width = imgPlatform.Width;
            double height = imgPlatform.Height;

            if (height > width)
                imgPlatform.Height = textDesc.ActualHeight * 0.7;
            else
                imgPlatform.Width = textDesc.ActualWidth * 0.3;

            if (initPosition)
            {
                initPosition = false;

                int CenterX = 0;
                int itemWidth = (int)scatterItem.ActualWidth;
                int CenterY = display.height - (int)(((int)scatterItem.ActualHeight >> 1) * 1.5);

                lock (deviceItems)
                {
                    if (nextStartX + itemWidth > display.width)
                        nextStartX = 0;
                    CenterX = nextStartX + (itemWidth >> 1);
                    nextStartX += itemWidth + 15;
                }

                scatterItem.Center = new Point(CenterX, CenterY);
                scatterItem.Orientation = 0f;
            }
        }
        

        #region Observer implementation

        void PortalObserver(PortalInstance portal, int Environs_NOTIFY_)
        {
            //Utils.Log(6, className, "PortalObserver: [" + portal.portalID + "] " + Environs.resolveName(Environs_NOTIFY_));

            switch (Environs_NOTIFY_)
            {
                case Environs.ENVIRONS_OBJECT_DISPOSED:
                    Environs.dispatch(new Action(delegate()
                    {
                        if (portal.outgoing)
                            btnPortal_out.Foreground = System.Windows.Media.Brushes.MediumSeaGreen;
                        else
                            btnPortal_in.Foreground = System.Windows.Media.Brushes.MediumSeaGreen;
                    }));
                    break;

                case Environs.NOTIFY_PORTAL_STREAM_STARTED:
                    Environs.dispatch(new Action(delegate()
                    {
                        if (portal.outgoing)
                            btnPortal_out.Foreground = System.Windows.Media.Brushes.LightGreen;
                        else
                            btnPortal_in.Foreground = System.Windows.Media.Brushes.LightGreen;
                    }));
                    break;

                case Environs.NOTIFY_PORTAL_STREAM_STOPPED:
                    break;

                /*case Environs.NOTIFY_PORTAL_PROVIDER_READY:
                case Environs.NOTIFY_PORTAL_PROVIDE_STREAM_ACK:
                case Environs.NOTIFY_PORTAL_PROVIDE_IMAGES_ACK:
                case Environs.NOTIFY_PORTAL_STREAM_INCOMING:
                case Environs.NOTIFY_PORTAL_IMAGES_INCOMING:
                    if (portal.status < Environs.PORTAL_STATUS_STARTED)
                        portal.Start();
                    break;
                    */

                default: break;
            }

            if (portalObserver != null)
                portalObserver(portal, Environs_NOTIFY_);
        }


        private void DeviceChangeObserver(DeviceInstance sender, int Environs_NOTIFY_)
        {
            if (sender == null)
                return;

            if ((Environs_NOTIFY_ & Environs.DEVICE_INFO_ATTR_ISCONNECTED) != 0)
            {
                bool isConnected = deviceInstance.isConnected;

                Environs.dispatch(new Action(delegate()
                {
                    btnConnect.Content = (isConnected ? "Disconnect" : "Connect");
                    btnConnect.Foreground = isConnected ? Brushes.LightGreen : Brushes.MediumSeaGreen;

                    btnPortal_in.IsEnabled = isConnected;
                    btnPortal_out.IsEnabled = isConnected;

                    if (isConnected)
                    {
                        progressConnect.Value = 100;
                        progressConnect.Foreground = System.Windows.Media.Brushes.LightGreen;
                    }
                    else
                    {
                        progressConnect.Value = 0;
                        progressConnect.Foreground = System.Windows.Media.Brushes.Red;
                    }
                }));

                buildDeviceDescription();
            }
            else if ((Environs_NOTIFY_ & Environs.DEVICE_INFO_ATTR_CONNECT_PROGRESS) != 0)
            {
                Environs.dispatch(new Action(delegate()
                {
                    int value = deviceInstance.connectProgress;
                    if (value > 1000)
                    {
                        /// An error is indicated with the value
                        progressConnect.Foreground = System.Windows.Media.Brushes.Red;
                        value -= 1000;
                    }
                    else
                        progressConnect.Foreground = System.Windows.Media.Brushes.LightGreen;

                    progressConnect.Value = value;
                    //Utils.Log(1, className, "progressConnect:" + value);
                }));
            }
            /*else if ((Environs_NOTIFY_ & Environs.DEVICE_INFO_ATTR_PORTAL_CREATED) != 0)
            {
                if (sender != null)
                {
                    PortalInstance portal = (PortalInstance)sender;
                    portal.AddObserver(PortalObserver);
                }
            }
                */
            else buildDeviceDescription();
        }

        #endregion


        internal void buildDeviceDescription()
        {
            if (deviceInstance == null)
                return;

            string desc = "0x" + deviceInstance.deviceID.ToString("X") + " (" + (deviceInstance.isConnected ? "Connected" : "---") + ")\r\n";

            desc += deviceInstance.deviceName + "\r\n";
            desc += "A: " + deviceInstance.appName + "\r\n";
            desc += "P: " + deviceInstance.areaName + "\r\n";
            desc += deviceInstance.GetBroadcastString(false) + ": " + deviceInstance.ips + "\r\n";

            Environs.dispatch(new Action(delegate()
            {
                textDesc.Text = desc;
            }));
        }


        private void btnConnect_TouchUp(object sender, RoutedEventArgs e)
        {
            if (deviceInstance.isConnected)
                deviceInstance.Disconnect();
            else
                deviceInstance.Connect();
        }


        # region Device Portal handlers

        private void btnPortal_out_TouchUp(object sender, RoutedEventArgs e)
        {
            StopOrCreatePortal(true);
        }


        private void btnPortal_in_TouchUp(object sender, RoutedEventArgs e)
        {
            StopOrCreatePortal(false);
        }


        private void StopOrCreatePortal(bool outgoing)
        {
            if (env.OnPortalRequestOrProvided == null)
                return;

            Thread thread = new Thread(() => StopOrCreatePortalThread(outgoing));
            if (thread != null)
                thread.Start();
        }

        private void StopOrCreatePortalThread(bool outgoing)
        {
            PortalInstance portal = outgoing ? deviceInstance.PortalGetOutgoing() : deviceInstance.PortalGetIncoming();
            if (portal != null)
            {
                portal.Stop();
                return;
            }

            // Invoke a portal request by our device
            //deviceInstance.PortalCreateInvoke(outgoing ? Environs.PORTAL_DIR_OUTGOING : Environs.PORTAL_DIR_INCOMING, Environs.PortalType.Any);
            
            if (outgoing)
                portal = deviceInstance.PortalProvide(Environs.PortalType.Any);
            else
                portal = deviceInstance.PortalRequest(Environs.PortalType.Any);

            if (portal != null)
            {
                portal.AddObserver(PortalObserver);
                portal.Establish(true);
            }
        }
        #endregion
    }
}

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
using System.Threading;
using System.Windows;
using System.Windows.Media;
using System.Windows.Media.Imaging;

using environs;
using environs.lib;

namespace environs.Apps
{
    /// <summary>
    /// Interaktionslogik für DeviceWindow.xaml
    /// </summary>
    public partial class DeviceWindow : Window
    {
        private const String className = "DeviceWindow . . . . . .";

        WriteableBitmap wbm = null;

        Environs env = null;

        PortalSinkSource portalSinkCallback = null;
        int deviceID = 0;
        int portalID = 0;
        PortalInstance portal = null;

        public DeviceWindow()
        {
            InitializeComponent();

            portalSinkCallback += PortalSinkEvent;
        }


        public bool Init(int deviceID, PortalInstance p)
        {
            this.deviceID = deviceID;
            portal = p;
            this.portalID = p.portalID;

            PortalInfo info = portal.info;
            if (info != null)
            {
                this.MinWidth = info.width;
                this.MinHeight = info.height;
            }
            else
            {
                this.MinWidth = 1024;
                this.MinHeight = 768;
            }

            return env.SetRenderCallback(Call.NoWait, portalID, portalSinkCallback, RenderCallbackType.AvContext);
        }


        protected override void OnClosing(System.ComponentModel.CancelEventArgs e)
        {
            if (portal.status > 0)
            {
                e.Cancel = true;


                Thread closer = new Thread(CloserThread);
                if (closer == null)
                    Utils.LogE(className, "[ERROR] -- E -->:OnClosing: Failed to create closer thread!!!");
                else
                {
#if DEBUG
                    closer.Name = "DeviceWindow.CloserThread";
#endif
                    closer.Start();
                }
                return;
            }
            portal.Stop();
            portal.ReleaseRenderSurface();
            //env.StopPortalStream(Environs.CALL_NOWAIT, 0, portalID);
            //env.ReleaseRenderCallback(Environs.CALL_NOWAIT, portalID);

            base.OnClosing(e);
        }


        private void CloserThread()
        {
            if (Utils.Log(4))
                Utils.Log(4, "[VERB]  CloserThread: started...");

            portal.Stop();
            portal.ReleaseRenderSurface();

            //env.StopPortalStream(Environs.CALL_NOWAIT, 0, portalID);
            //env.ReleaseRenderCallback(Environs.CALL_NOWAIT, portalID);

            Environs.dispatch(new Action(delegate()
            {
                this.Close();
            }));
        }

        private void PortalSinkEvent(int argumentType, IntPtr surface, IntPtr callbackArgument)
        {
            //Debug.WriteLine("[INFO] PortalSinkEvent:");

            if (argumentType != Environs.RENDER_CALLBACK_TYPE_AVCONTEXT)
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
    }
}

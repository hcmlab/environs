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
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using Microsoft.Surface;
using Microsoft.Surface.Presentation;
using Microsoft.Surface.Presentation.Controls;
using System.Diagnostics;
using System.Drawing;
using System.Windows.Interop;
using System.Net;
using System.Net.Sockets;
using System.Threading;

using environs;
using System.Windows.Threading;
using System.Runtime.InteropServices;
using System.Collections;
using System.Collections.ObjectModel;
using System.Windows.Media.Animation;

#if SURFACE2
using Microsoft.Surface.Presentation.Input;
using Microsoft.Surface.Presentation.Controls.TouchVisualizations;
#else
using Microsoft.Surface.Core;
using Microsoft.Surface.Presentation.Controls.ContactVisualizations;
#endif

namespace environs.Apps
{
    /// <remarks>
    /// Microsoft Surface 1 specific code
    /// </remarks>
    public partial class SurfaceMediaBrowser : SurfaceWindow
    {
        /// <summary>
        /// Adds handlers for surface specific touch events
        /// </summary>
        private void AddSurfaceTouchHandlers()
        {
            g_add.PreviewContactUp += addFile;
            g_bin.PreviewContactUp += delete;
        }

        /// <summary>
        /// Adds handlers for Application activation events.
        /// Adds handlers for window availability events.
        /// </summary>
        private void AddSurfaceHandlers()
        {
            // Subscribe to surface application activation events
            ApplicationLauncher.ApplicationActivated += OnApplicationActivated;
            ApplicationLauncher.ApplicationPreviewed += OnApplicationPreviewed;
            ApplicationLauncher.ApplicationDeactivated += OnApplicationDeactivated;
        }

        /// <summary>
        /// Removes handlers for Application activation events.
        /// Removes handlers for window availability events.
        /// </summary>
        private void RemoveSurfaceHandlers()
        {
            // Unsubscribe from surface application activation events
            ApplicationLauncher.ApplicationActivated -= OnApplicationActivated;
            ApplicationLauncher.ApplicationPreviewed -= OnApplicationPreviewed;
            ApplicationLauncher.ApplicationDeactivated -= OnApplicationDeactivated;
        }


        /// <summary>
        /// This is called when application has been activated.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void OnApplicationActivated(object sender, EventArgs e)
        {
            //TODO: enable audio, animations here
        }

        /// <summary>
        /// This is called when application is in preview mode.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void OnApplicationPreviewed(object sender, EventArgs e)
        {
            //TODO: Disable audio here if it is enabled

            //TODO: optionally enable animations here
        }

        /// <summary>
        ///  This is called when application has been deactivated.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void OnApplicationDeactivated(object sender, EventArgs e)
        {
            //TODO: disable audio, animations here
        }
    }

}
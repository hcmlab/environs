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
using System.Windows.Input;
using Microsoft.Surface.Presentation.Controls;

#if !SURFACE2
using Microsoft.Surface.Input.Common;
using Microsoft.Surface.Simulator.Automation;
#endif

using System.Reflection;

namespace environs.Apps
{
    /// <summary>
    /// Interaction logic for MediaBrowser.xaml
    /// </summary>
    public partial class SurfaceMediaBrowser : SurfaceWindow
    {        
        /// <summary>
        /// Default constructor.
        /// </summary>
        public SurfaceMediaBrowser()
        {
            InitializeComponent();

            string name = Assembly.GetExecutingAssembly().GetName().Name;

            String uri = "/" + name + ";component/AppStyles.xaml";

            Resources.MergedDictionaries.Add ( new ResourceDictionary 
            { 
                Source = new Uri(uri, UriKind.RelativeOrAbsolute) 
            });

#if SURFACE2
            uri = "/" + name + ";component/XAMLResources.xaml";
#else
            uri = "/" + name + ";component/Surface1/XAMLResources.xaml";
#endif
            Resources.MergedDictionaries.Add(new ResourceDictionary
            {
                Source = new Uri(uri, UriKind.RelativeOrAbsolute)
            });


            AddSurfaceHandlers();

            Background = System.Windows.Media.Brushes.Black;

            /// Initialize Environs
            InitEnvirons();

            /// Initialization of Media View that is common for surface 1 and surface 2. 
            /// Therefore, the code has been moved to the shared file SurfaceMediaView.cs.
            /// This also includes the device environment.
            InitMediaView();
        }


        
        /// <summary>
        /// Occurs when the window is about to close. 
        /// </summary>
        /// <param name="e"></param>
        protected override void OnClosed(EventArgs e)
        {
            base.OnClosed(e);

            RemoveSurfaceHandlers();
        }
        

        private void g_lbDeviceList_PreviewMouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {

        }

    }
}
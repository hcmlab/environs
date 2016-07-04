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
using System.Collections.Generic;
using System.Configuration;
using System.Data;
using System.Linq;
using System.Windows;

namespace environs.Apps
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        public App() : base()
        {
#if SURFACE2
            String res = @"/Microsoft.Surface.Presentation.Generic;v2.0.0.0;31bf3856ad364e35;component/themes/styles.xaml";
#else
            String res = @"/Microsoft.Surface.Presentation.Generic;v1.0.0.0;31bf3856ad364e35;component/themes/generic.xaml";
#endif
            Resources.MergedDictionaries.Add(
                new ResourceDictionary { Source = new Uri(res, UriKind.Relative) });
        }
    }
}
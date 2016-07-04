/**
 * Environs iOSX platform specific imports
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
#ifndef INCLUDE_HCM_ENVIRONS_IOSX_COMMON_IMPORTS_H
#define INCLUDE_HCM_ENVIRONS_IOSX_COMMON_IMPORTS_H

#include "Environs.Platforms.h"
#include "Environs.Build.Opts.h"
#include "Environs.Types.h"

#ifdef ENVIRONS_IOS
    //******** iOS *************
#   import <UIKit/UIKit.h>
#   import <CoreMotion/CoreMotion.h>
#   import <Environs.iOS.h>
#else
#   ifdef ENVIRONS_OSX
        //******** OSX *************
#       import <Environs.OSX.h>
#       import <Cocoa/Cocoa.h>

#       define     UIView          NSView
#       define     UIImage         NSImage
#       define     UIAlertView     NSAlertView
#   endif
#endif

//#endif

#endif  /// end-INCLUDE_HCM_ENVIRONS_IOSX_COMMON_IMPORTS_H





/**
 *	Portal Generator for iOS devices
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
#include "stdafx.h"

#if !defined(DISPLAYDEVICE) && defined(__APPLE__)

/// Compiler flag that enables verbose debug output
#ifndef NDEBUG
//#   define DEBUGVERB
//#   define DEBUGVERBVerb
#endif

#include "Environs.Obj.h"
#include "Portal.Generator.iOS.h"
#include "Device/Device.Mobile.h"
#include "Environs.Modules.h"

// The TAG for prepending to log messages
#define CLASS_NAME	"Portal.Generator.iOS . ."


extern "C" void * CallConv CreateInstanceIOSCam ( int index, int deviceID );
#import "Encoder/Encoder.iOSX.H264.h"


namespace environs 
{
    
    IPortalCapture	* PortalGeneratorMobile::GetCameraInstance ( int cameraID )
    {
        return (IPortalCapture *) CreateInstanceIOSCam ( 0, deviceID );
    }
    
    
    IPortalEncoder	* PortalGeneratorMobile::GetEncoderInstance ( int deviceID, int index )
    {
        return (IPortalEncoder *) new EncoderIOSH264Env ();
    }

} /// -> namespace environs

#endif

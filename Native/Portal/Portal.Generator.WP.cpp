/**
 *	Portal Generator for Windows Phone devices
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

#if !defined(DISPLAYDEVICE) && defined(WINDOWS_PHONE)

/// Compiler flag that enables verbose debug output
#ifndef NDEBUG
//#   define DEBUGVERB
//#   define DEBUGVERBVerb
#endif

#include "Environs.Obj.h"
#include "Portal.Generator.WP.h"
#include "Device/Device.Mobile.h"
#include "Environs.Modules.h"

// The TAG for prepending to log messages
#define CLASS_NAME	"Portal.Generator.Android"



namespace environs 
{
    
    IPortalCapture	* PortalGeneratorMobile::GetCameraInstance ( int cameraID )
    {
        return (IPortalCapture *) environs::API::CreateInstance ( LIBNAME_Capture_Android_Camera, 0, InterfaceType::Capture, deviceID, env );
    }
    
    
    IPortalEncoder	* PortalGeneratorMobile::GetEncoderInstance ( int deviceID, int index )
    {
        if ( opt_useHardwareEncoder ) {
            return (IPortalEncoder *) environs::API::CreateInstance ( LIBNAME_Encoder_Android_HwH264, 0, InterfaceType::Encoder, deviceID, env );
        }
        
        return (IPortalEncoder *) environs::API::CreateInstance ( LIBNAME_Encoder_LibOpenH264, 0, InterfaceType::Encoder, deviceID, env );
        //return (IPortalEncoder *) new EncoderAndroid ();
    }

} /// -> namespace environs

#endif

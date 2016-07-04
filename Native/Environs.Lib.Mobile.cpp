/**
 * Environs Native Layer API exposed by the libraries for mobile devices
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
#define ENVIRONS_NATIVE_MODULE

/// Compiler flag that enables verbose debug output
#ifndef NDEBUG
//#   define DEBUGVERB
//#   define DEBUGVERBVerb
#endif

// Import access to the environs native object
#include "Environs.Obj.h"
using namespace environs;

// Import declarations and exports for the API
#include "Environs.Lib.h"
#include "Environs.Utils.h"
#include "Environs.Crypt.h"
#include "Environs.Mobile.h"
#include "Environs.Modules.h"
//#include "Interop/Stat.h"

#include "Core/Async.Worker.h"
#include "Core/Byte.Buffer.h"
#include "Core/Callbacks.h"
#include "Core/Notifications.h"

#include "Device/Devices.h"
#include "Device/Device.Controller.h"

#ifdef DISPLAYDEVICE
#include "Renderer/Render.OpenCL.h"
#endif

#define CLASS_NAME	"Native.Mobile. . . . . ."


#ifndef DISPLAYDEVICE

namespace environs 
{    
	namespace API
	{
		/*
		* Class:     hcm_environs_Environs
		* Method:    GetSizeOfInputPack
		* Signature: ()I
		*/
		ENVIRONSAPI jint EnvironsProc ( GetSizeOfInputPackN )
		{
			return (jint)sizeof ( environs::InputPack );
		}


		/*
		* Method:    SetPortalTCPN
		* Signature: (Z)Z
		*/
		ENVIRONSAPI void EnvironsFunc ( SetPortalTCPN, jint hInst, EBOOL enable )
		{
			Instance * env = instances [ hInst ];

			env->useTcpPortal = enable;
			env->optBool ( APPENV_SETTING_USE_PORTAL_TCP, enable );
		}


		/*
		* Method:    GetPortalTCPN
		* Signature: ()Z
		*/
		ENVIRONSAPI EBOOL EnvironsFunc ( GetPortalTCPN, jint hInst )
		{
			return instances[hInst]->useTcpPortal;
		}


		/*
		* Method:    SetUseNativeDecoderN
		* Signature: (Z)Z
		*/
		ENVIRONSAPI void EnvironsFunc ( SetUseNativeDecoderN, jint hInst, EBOOL enable )
		{
			environs::opt_useNativeDecoder = (enable ? 1 : 0);

			opt ( hInst, APPENV_SETTING_GL_USE_NATIVE_DECODER, enable );
		}


		/*
		* Method:    GetUseNativeDecoderN
		* Signature: ()Z
		*/
		ENVIRONSAPI EBOOL EnvironsFunc ( GetUseNativeDecoderN, jint hInst )
		{
			return (environs::opt_useNativeDecoder != 0);
		}


		/*
		* Method:    SetUseHardwareEncoderN
		* Signature: (Z)Z
		*/
		ENVIRONSAPI void EnvironsFunc ( SetUseHardwareEncoderN, jint hInst, EBOOL enable )
		{
			environs::opt_useHardwareEncoder = (enable ? 1 : 0);

			opt ( hInst, APPENV_SETTING_GL_USE_HARDWARE_DECODER, enable );
		}


		/*
		* Method:    GetUseHardwareEncoderN
		* Signature: ()Z
		*/
		ENVIRONSAPI EBOOL EnvironsFunc ( GetUseHardwareEncoderN, jint hInst )
		{
			return (environs::opt_useHardwareEncoder != 0);
		}


		/*
		* Method:    SetUseSensorsN
		* Signature: (Z)Z
		*/
		ENVIRONSAPI void EnvironsFunc ( SetUseSensorsN, jint hInst, EBOOL enable )
		{
			opt_useSensors = enable; 

			opt ( hInst, APPENV_SETTING_GL_USE_SENSORS, enable );
		}


		/*
		* Method:    GetUseSensorsN
		* Signature: ()Z
		*/
		ENVIRONSAPI EBOOL EnvironsFunc ( GetUseSensorsN, jint hInst )
		{
			return opt_useSensors;
		}


		/*
		* Method:    SetUsePushNotifications
		* Signature: (Z)Z
		*/
		ENVIRONSAPI void EnvironsFunc ( SetUsePushNotificationsN, jint hInst, EBOOL enable )
		{
			opt_usePushNotifications = enable;
			
			opt ( hInst, APPENV_SETTING_GL_USE_PUSH_NOTIFS, enable );
		}


		/*
		* Method:    GetUsePushNotifications
		* Signature: ()Z
		*/
		ENVIRONSAPI EBOOL EnvironsFunc ( GetUsePushNotificationsN, jint hInst )
		{
			return opt_usePushNotifications;
		}
        
    }
}

#endif




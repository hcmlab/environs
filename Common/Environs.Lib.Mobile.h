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
#pragma once
#ifndef INCLUDE_HCM_ENVIRONS_NATIVE_MOBILE_API_H
#define INCLUDE_HCM_ENVIRONS_NATIVE_MOBILE_API_H


namespace environs 
{
#ifndef DISPLAYDEVICE

#include "Interop/jni.h"
#include "Environs.Lib.Inc.h"
	
	namespace API 
	{

#ifdef __cplusplus
		extern "C"
		{
#endif
			LIBEXPORT void CallConv			TouchDispatchN ( int hInst, int portalID, void * buffer, int count, EBOOL init );
			LIBEXPORT void CallConv			touch1 ( int deviceID, int id, int x, int y, unsigned char event );
			LIBEXPORT void CallConv			touch2 ( int deviceID, int id, int x, int y, float angle, float size, float axisx, float axisy, unsigned char event );

			LIBEXPORT int CallConv			GetSizeOfBufferHeaderN ();
			LIBEXPORT void * CallConv		ReceiveStreamPackN ( int deviceID, const char * areaName, const char * appName, int portalID );

			LIBEXPORT int CallConv			GetSizeOfTouchPackN ();
			
			LIBEXPORT EBOOL CallConv		SetPortalWidthHeightN ( int deviceID, int width, int height );

			LIBEXPORT EBOOL CallConv		GetUseSensorsN ( jint hInst );
			LIBEXPORT void CallConv			SetUseSensorsN ( jint hInst, EBOOL enable );

			LIBEXPORT EBOOL CallConv		GetUsePushNotificationsN ( int hInst );
			LIBEXPORT void CallConv			SetUsePushNotificationsN ( int hInst, EBOOL enable );

			LIBEXPORT EBOOL CallConv		GetPortalTCPN ( jint hInst );
			LIBEXPORT void CallConv			SetPortalTCPN ( jint hInst, EBOOL enable );

			LIBEXPORT EBOOL CallConv		GetUseNativeDecoderN ( jint hInst );
			LIBEXPORT void CallConv			SetUseNativeDecoderN ( jint hInst, EBOOL enable );

			LIBEXPORT EBOOL CallConv		GetUseHardwareEncoderN ( jint hInst );
			LIBEXPORT void CallConv			SetUseHardwareEncoderN ( jint hInst, EBOOL enable );
            
            LIBEXPORT int CallConv			AcquirePortalSendIDN ( int portalUnitType, int portalID );
            LIBEXPORT void CallConv			ReleasePortalSendIDN ( int sendID );
            
            LIBEXPORT EBOOL CallConv		SendTcpPortalN ( int sendID, int portalUnitFlags, jobject prefixBuffer, int prefixSize, jobject byteBuffer, int offset, int contentSize );
            
            LIBEXPORT EBOOL CallConv		SendPortalInitN ( int hInst, int async, int portalID, int width, int height );

#ifdef __cplusplus
		}
#endif

	}
    
#endif
}

#endif	// INCLUDE_HCM_ENVIRONS_NATIVE_MOBILE_API_H




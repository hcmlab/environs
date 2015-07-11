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

#include "Environs.Lib.Inc.h"
	
	namespace API 
	{

#ifdef __cplusplus
		extern "C"
		{
#endif
			//LIBEXPORT void CallConv			allocNative ();
			LIBEXPORT void CallConv			SetOrientations ( float x, float y, float z );
			LIBEXPORT void CallConv			SetAccelerometers ( float x, float y, float z );

			LIBEXPORT void CallConv			TouchDispatch ( int deviceID, void * buffer, int count, EBOOL init );
			LIBEXPORT void CallConv			touch1 ( int deviceID, int id, int x, int y, unsigned char event );
			LIBEXPORT void CallConv			touch2 ( int deviceID, int id, int x, int y, float angle, float size, float axisx, float axisy, unsigned char event );

			LIBEXPORT int CallConv			GetSizeOfBufferHeader ();
			LIBEXPORT void * CallConv		ReceiveStreamPack ( int deviceID, const char * projName, const char * appName, int portalID );

			LIBEXPORT int CallConv			GetSizeOfTouchPack ();

			LIBEXPORT int CallConv          GetActivePortalID ();
			LIBEXPORT int CallConv          GetRequestedPortalID ();

			LIBEXPORT void CallConv			SetPortalViewDimsAuto ( EBOOL enable );
			LIBEXPORT EBOOL CallConv		GetPortalViewDimsAuto ();
			LIBEXPORT void CallConv			SetPortalViewDims ( int nativeID, int left, int top, int right, int bottom );

			LIBEXPORT EBOOL CallConv		SetPortalWidthHeight ( int deviceID, int width, int height );

			LIBEXPORT EBOOL CallConv		GetUseSensors ();
			LIBEXPORT void CallConv			SetUseSensors ( EBOOL enable );

			LIBEXPORT EBOOL CallConv		GetUsePushNotifications ();
			LIBEXPORT void CallConv			SetUsePushNotifications ( EBOOL enable );

			LIBEXPORT EBOOL CallConv		GetPortalTCP ();
			LIBEXPORT void CallConv			SetPortalTCP ( EBOOL enable );

			LIBEXPORT EBOOL CallConv		GetUseNativeDecoder1 ();
			LIBEXPORT void CallConv			SetUseNativeDecoder1 ( EBOOL enable );

			LIBEXPORT EBOOL CallConv		GetUseHardwareEncoder ();
			LIBEXPORT void CallConv			SetUseHardwareEncoder1 ( EBOOL enable );

			LIBEXPORT EBOOL CallConv		GetPortalNativeResolution ();
			LIBEXPORT void CallConv			SetPortalNativeResolution ( EBOOL enable );

			LIBEXPORT EBOOL CallConv		GetPortalAutoStart ();
			LIBEXPORT void CallConv			SetPortalAutoStart ( EBOOL enable );
            
            LIBEXPORT int CallConv			AcquirePortalSendID ( int portalUnitType, int portalID );
            LIBEXPORT void CallConv			ReleasePortalSendID ( int sendID );
            
            LIBEXPORT EBOOL CallConv		SendTcpPortal ( int sendID, int portalUnitFlags, jobject prefixBuffer, int prefixSize, jobject byteBuffer, int offset, int contentSize );
            
            LIBEXPORT EBOOL CallConv		SendPortalInit ( int async, int portalID, int width, int height );

#ifdef __cplusplus
		}
#endif

	}
    
#endif
}

#endif	// INCLUDE_HCM_ENVIRONS_NATIVE_MOBILE_API_H




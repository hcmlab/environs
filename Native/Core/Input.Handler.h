/**
 * Device Marker Handling
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
#ifndef INCLUDE_HCM_ENVIRONS_DEVICE_MARKER_HANDLING_H
#define INCLUDE_HCM_ENVIRONS_DEVICE_MARKER_HANDLING_H

#include "Human.Input.Decl.h"

#ifdef __cplusplus
namespace environs
{
#endif

#ifdef CLI_CPP
#	define INPUTTYPE	environs::InputPack
#else
#	define INPUTTYPE	Input
#endif

	CLASS MarkerDevicePack;

#ifdef DISPLAYDEVICE

	PUBLIC_CLASS InputHandler
	{
	public:
		static pthread_mutex_t  accessMutex;

		static bool Init ();
		static void Release ();

		static bool HandleDeviceMarker ( int hInst, int nativeID, INPUTTYPE OBJ_ptr input );
		static bool IsFingerInDeviceMarker ( INPUTTYPE OBJ_ptr input, int &flags );
		static void ClearDeviceMarkers ();

#ifndef CLI_CPP
		static NLayerMapRaw ( int, MarkerDevicePack ) markerDevices;

		static bool HandleUniqueInputID ( int nativeID, Input * input );
		static void FilterInput ( int hInst, int nativeID, Input * input );
		static bool ClearTouchDevices ();
#endif


	private:
		static bool allocated	Cli_Only ( = false );
	};


#define	HandleIsFingerInDeviceMarker(state,input,flags)		\
				state = ( !environs::opt_useDeviceMarkerHandler ||  input->pack.raw.type != INPUT_TYPE_FINGER || !IsFingerInDeviceMarker ( input, flags ) );

#define	HandleDeviceMarkerInput(hInst,deviceID,input)	\
			if ( environs::opt_useDeviceMarkerHandler && input->pack.raw.type == INPUT_TYPE_MARKER &&  HandleDeviceMarker ( hInst, deviceID, input ) ) \
				return;
#else

#define	HandleIsFingerInDeviceMarker(state,input,flags)
#define	HandleDeviceMarkerInput(hInst,deviceID,input)
#define IsFingerInDeviceMarker(input,flags)
#define	ClearDeviceMarkers()

#endif

#ifdef __cplusplus
}
#endif



#endif	/// INCLUDE_HCM_ENVIRONS_DEVICE_MARKER_HANDLING_H

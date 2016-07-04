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
#include "stdafx.h"

/// Compiler flag that enables verbose debug output
#ifndef NDEBUG
//#   define DEBUGVERB
//#   define DEBUGVERBVerb
#endif

#ifdef DISPLAYDEVICE

#if ( defined(CLI_CPP) )
#	define	CLI_CPP_DEVINFO
#endif

#if ( defined(CLI_CPP) )
#	include "Environs.Cli.Forwards.h"
#endif

#include "Interop.h"
#include "Interop/Threads.h"
#include "Interop/Smart.Pointer.h"

#if ( defined(CLI_CPP) )
#	include "Environs.Cli.h"
#endif

#include "Core/Input.Handler.h"

#if ( defined(CLI_CPP) )
#	include "Device.Info.h"
#	include "Device/Device.Instance.Cli.h"
#else
#	include "Callbacks.h"
#	include "Kernel.h"
#	include "Environs.Obj.h"
#	include "Device/Devices.h"
#	include "Core/Byte.Buffer.h"
#	include <map>
	
	using namespace std;
#endif

#include "Environs.Lib.h"
#include "Environs.Utils.h"
#include "Environs.Native.h"

using namespace environs;

// The TAG for prepending in log messages
#define CLASS_NAME	"Device.Marker. . . . . ."


namespace environs
{

	PUBLIC_CLASS MarkerDevicePack
	{
	public:

#if ( defined(CLI_CPP) )
		environs::InputPack ^	input;
#else
		lib::DevicePack         devPack;
		Input					input;
#endif
		INTEROPTIMEVAL			lastEvent;
		int						occlusionValue;
		int						portalID;

#if ( defined(CLI_CPP) )
		DeviceInstance		^	device;

		MarkerDevicePack () { device = nill; }
#else
		DeviceBase			*	device;

		MarkerDevicePack () : devPack ( EmptyStructValue ( lib::DevicePack ) ), input ( EmptyStructValue ( Input ) ), lastEvent ( 0 ), occlusionValue ( 0 ), portalID ( 0 ), device ( nill )
		{
		}
#endif

		~MarkerDevicePack ()
		{
			if ( device != nill ) {
				C_Only ( UnlockDevice ( device ); )
				device = nill;
			}
		};
	};


#ifndef CLI_CPP
	bool					InputHandler::allocated       = false;

	pthread_mutex_t			InputHandler::accessMutex;

	map<int, map<long long, long long>*>	touchDevices;

	NLayerMapRaw ( int, MarkerDevicePack ) InputHandler::markerDevices;
#endif

	bool InputHandler::Init ()
	{
		if ( !allocated ) {
			if ( !LockInitA ( accessMutex ) )
				return false;

			allocated = true;
		}

		ClearDeviceMarkers ();

#if ( defined(CLI_CPP) )
		return true;
#else
		return ClearTouchDevices ();
#endif
	}


	void InputHandler::Release ()
	{
        if ( allocated ) {
            allocated = false;
            
			LockDisposeA ( accessMutex );
		}
	}


#ifndef CLI_CPP
	bool InputHandler::HandleUniqueInputID ( int nativeID, Input * input )
	{
		CVerbVerbIDN ( "HandleUniqueInputID" );

		if ( pthread_mutex_lock ( &accessMutex ) ) {
			CErrIDN ( "HandleUniqueInputID: Failed to lock access." );
			return false;
		}

		bool                    success     = false;
		int                     touchID     = 0;
		int						flags		= 0;
		map<long long, long long>*    touchDevice;

		// Get the mapping
		long long uniqueID = ( ( ( long long ) input->pack.raw.id ) << 4 ) | ( input->pack.raw.type & 0xF );

		map<long long, long long>::iterator            touchMapIt;
		map<int, map<long long, long long>*>::iterator touchDeviceIt = touchDevices.find ( nativeID );

		if ( touchDeviceIt != touchDevices.end () )
		{
			touchDevice = touchDeviceIt->second;
		}
		else {
			if ( input->pack.raw.state == INPUT_STATE_DROP ) {
				CErrIDN ( "HandleUniqueInputID: Invalid state! Failed to find touchDevice for drop event.!" );
				goto Finish;
			}

			touchDevice = new map<long long, long long> ();
			if ( !touchDevice ) {
				CErrIDN ( "HandleUniqueInputID: Failed to create touchDevice!" );
				goto Finish;
			}
			touchDevices [ nativeID ] = touchDevice;
			CVerbVerbIDN ( "HandleUniqueInputID: Added device" );
		}

		CVerbVerbArgIDN ( "HandleUniqueInputID: Search for uniqueID [%lld], id [%i], type [%i]", uniqueID, input->pack.raw.id, input->pack.raw.type );
		touchMapIt = touchDevice->find ( uniqueID );

		if ( touchMapIt != touchDevice->end () ) {
			CVerbVerbIDN ( "HandleUniqueInputID: Found uniqueID" );
			touchID = ( int ) ( touchMapIt->second & 0xFFFFFFFF );
			flags = ( int ) ( touchMapIt->second >> 32 );

			if ( input->pack.raw.state == INPUT_STATE_DROP )
			{
				touchDevice->erase ( touchMapIt );
				CVerbVerbArgIDN ( "HandleUniqueInputID: Dropped uniqueID [%lld]", uniqueID );

				if ( !touchDevice->size () ) {
					touchDevices.erase ( touchDeviceIt );
					CVerbVerbIDN ( "HandleUniqueInputID: Dropped device" );
				}
			}
		}
		else {
			CVerbVerbIDN ( "HandleUniqueInputID: uniqueID not found" );
			if ( input->pack.raw.state == INPUT_STATE_DROP ) {
				CErrArgIDN ( "HandleUniqueInputID: Invalid state! Failed to find mapped id for drop event! uniqueID [%lld], id [%i], type [%i]", uniqueID, input->pack.raw.id, input->pack.raw.type );
				goto Finish;
			}

			touchID = Kernel::GetUniqueInputID ();
		}

		CVerbVerbArgIDN ( "HandleUniqueInputID: Translate source id [%i/%lld] to id [%i]", input->pack.raw.id, uniqueID, touchID );
		//CVerbVerbArgIDN ( "HandleUniqueInputID: x/y [%i/%i] angle [%f]", input->pack.raw.x, input->pack.raw.y, input->pack.raw.angle );
		input->pack.uniqueID = touchID;
		input->idHandlerManaged = true;

#ifndef DISPLAYDEVICE
		success = true;
#endif

		HandleIsFingerInDeviceMarker ( success, input, flags );

		input->flags |= flags;

		( *touchDevice ) [ uniqueID ] = ( ( ( long long ) flags ) << 32 ) | touchID;

	Finish:
		if ( pthread_mutex_unlock ( &accessMutex ) ) {
			CErrIDN ( "HandleUniqueInputID: Failed to unlock access." );
			return false;
		}
		return success;
	}


	void CallBackConv InputHandler::FilterInput ( int hInst, int nativeID, Input * input )
	{
		CVerbVerbIDN ( "FilterInput" );

		Instance * env = instances [ hInst ];

		if ( !env->callbacks.OnInputDelegate || !input )
			return;

		if ( input->pack.uniqueID == -1 )
			return;

		bool submit = true;

		if ( !input->pack.uniqueID || input->idHandlerManaged )
		{
			if ( !HandleUniqueInputID ( nativeID, input ) )
				return;
		}

		HandleDeviceMarkerInput ( hInst, nativeID, input );

		if ( submit )
			env->callbacks.OnInputDelegate ( hInst, nativeID, input );
	}


	bool InputHandler::ClearTouchDevices ()
	{
		CVerb ( "ClearTouchDevices" );

		if ( !LockAcquire ( &accessMutex, "ClearTouchDevices" ) )
			return false;

		if ( touchDevices.size () )
		{
			map<int, map<long long, long long>*>::iterator touchDevicesIt = touchDevices.begin ();

			while ( touchDevicesIt != touchDevices.end () )
			{
				if ( touchDevicesIt->second ) {
					touchDevicesIt->second->clear ();
					delete touchDevicesIt->second;
				}

				++touchDevicesIt;
			}

			touchDevices.clear ();
		}

		if ( !LockRelease ( &accessMutex, "ClearTouchDevices" ) )
            return false;
        
        ClearDeviceMarkers ();

		return true;
	}

#endif

	bool InputHandler::HandleDeviceMarker ( int hInst, int nativeID, INPUTTYPE OBJ_ptr input )
	{
		CVerbVerbIDN ( "HandleDeviceMarker" );

		char			*	areaName		= 0;
		char			*	appName			= 0;
#ifndef CLI_CPP

#ifdef CLI_CPP
		DeviceInstance	^	device			= nill;
#else
		DeviceBase		*	device			= 0;
#endif
		bool                success			= false;
		MarkerDevicePack OBJ_ptr markerDevice	= nill;
		int					markerValue		= input->pack.raw.value;

#ifndef CLI_CPP
		if ( !LockAcquireA ( accessMutex, "HandleDeviceMarker" ) )
			return false;

		// Search for the markerDevice mapping
		map<int, MarkerDevicePack *>::iterator markerDevicesIt = markerDevices.find ( markerValue );

		if ( markerDevicesIt != markerDevices.end () )
		{
			markerDevice = markerDevicesIt->second;
		}
		else {
			if ( input->pack.raw.state == INPUT_STATE_DROP ) {
				CErrIDN ( "HandleDeviceMarker: Invalid state! Failed to find markerDevice for drop event.!" );
				goto Finish;
			}

			markerDevice = new__obj ( MarkerDevicePack );
			if ( markerDevice == nill ) {
				CErrIDN ( "HandleDeviceMarker: Failed to create markerDevice!" );
				goto Finish;
			}

			char * buffer = ( char * ) &markerDevice->devPack;
            
            sp ( MediatorClient ) mediator = instances [ hInst ]->mediator MED_WP;
			if ( mediator )
				mediator->GetDevicesAvailableCachedBestMatch ( &buffer, sizeof ( lib::DevicePack ), markerValue );

			markerDevices [ markerValue ] = markerDevice;

			CVerbVerbIDN ( "HandleDeviceMarker: Added device" );
		}

		if ( *markerDevice->devPack.device.areaName )
			areaName = markerDevice->devPack.device.areaName;
		if ( *markerDevice->devPack.device.appName )
			appName = markerDevice->devPack.device.appName;

		if ( input->pack.raw.state == INPUT_STATE_DROP )
		{
			markerDevices.erase ( markerDevicesIt );
			CVerbVerbIDN ( "HandleDeviceMarker: Dropped device" );
		}
		else 
		{
			device = markerDevice->device;

			if ( !device )
				device = markerDevice->device = GetDevice ( instances [ hInst ], markerValue, areaName, appName );

			if ( markerDevice->portalID <= 0 ) {
				if ( device ) {
					markerDevice->portalID = device->GetPortalID ( PORTAL_DIR_OUTGOING );
				}
			}

			if ( !markerDevice->occlusionValue )
			{
				// Determine device size
				if ( device ) {
					CVerbIDN ( "HandleDeviceMarker: Calculating occluded area using screenprops." );

					int width_px = (int) ((device->display.width_mm * native.display.width_mm) / native.display.width);
					int height_px = (int) ((device->display.height_mm * native.display.height_mm) / native.display.height);

					markerDevice->occlusionValue = (width_px > height_px ? width_px : height_px);

					// Add 15%
					markerDevice->occlusionValue = (int) ((float) markerDevice->occlusionValue * 1.15);
				}
				else {
					CVerbIDN ( "HandleDeviceMarker: Failed to find device for screenprops." );
				}
			}
		}

		success = true;

	Finish:
		if ( !LockReleaseA ( accessMutex, "HandleDeviceMarker" ) )
			return false;

			if ( !success )
			return false;

		lib::InputPackRaw * prev = &markerDevice->input.pack.raw;
		lib::InputPackRaw * cur = &input->pack.raw;

		// Check whether we had an update within the last 33 ms
		//
		if ( input->pack.raw.state == INPUT_STATE_CHANGE )
		{
			INTEROPTIMEVAL curTime = GetEnvironsTickCount ();

			if ( markerDevice->lastEvent )
			{
				//CLogArgID ( "HandleDeviceMarker: Time diff [%i]", curTime - markerDevice->lastEvent );

				if ( ( curTime - markerDevice->lastEvent ) < 66 ) {
					CVerbVerbArgIDN ( "HandleDeviceMarker: Skipping update [%i]", curTime - markerDevice->lastEvent );
#ifdef USE_INPUT_PACK_STDINT_CPP
					markerDevice->input.Copy ( &input->pack.raw, INPUTPACK_V3_SIZE );
#else
					markerDevice->input.Copy ( &input->pack.raw, INPUTPACK_V3_INT_SIZE );
#endif
					return true;
				}
			}
			markerDevice->lastEvent = curTime;
		}

		// Check whether the marker has "really changed"
		//
		if ( !opt_useDeviceMarkerReducedPrecision
			|| input->pack.raw.state != INPUT_STATE_CHANGE
			|| prev->x != cur->x || prev->y != cur->y
			|| abs ( ( int ) prev->angle - ( int ) cur->angle ) ) {

			if ( input->pack.raw.state == INPUT_STATE_ADD ) {
				if ( device )
					device->UpdatePosition ( markerDevice->portalID, input->pack.raw.x, input->pack.raw.y, input->pack.raw.angle );
				else
					API::DeviceDetectedN ( hInst, input->pack.raw.value, areaName, appName, CALL_NOWAIT, input->pack.raw.x, input->pack.raw.y, input->pack.raw.angle );
			}
			else if ( input->pack.raw.state == INPUT_STATE_CHANGE ) {
				if ( device )
					device->UpdatePosition ( markerDevice->portalID, input->pack.raw.x, input->pack.raw.y, input->pack.raw.angle );
				/*else
					API::DeviceUpdatedN ( hInst, nativeID, CALL_NOWAIT, input->pack.raw.x, input->pack.raw.y, input->pack.raw.angle );*/
			}
			else if ( input->pack.raw.state == INPUT_STATE_DROP ) {
				if ( device )
					device->UpdatePosition ( markerDevice->portalID, input->pack.raw.x, input->pack.raw.y, input->pack.raw.angle );
				/*else
					API::DeviceRemovedN ( hInst, nativeID, CALL_NOWAIT, input->pack.raw.x, input->pack.raw.y, input->pack.raw.angle );*/
			}
		}

		if ( input->pack.raw.state == INPUT_STATE_DROP ) {
			delete ( markerDevice );
		}
		else {
			// Take over the new input
#ifdef USE_INPUT_PACK_STDINT_CPP
			markerDevice->input.Copy ( &input->pack.raw, INPUTPACK_V3_SIZE );
#else
			markerDevice->input.Copy ( &input->pack.raw, INPUTPACK_V3_INT_SIZE );
#endif
		}
#endif

#endif
		return true;
	}


	bool InputHandler::IsFingerInDeviceMarker ( INPUTTYPE OBJ_ptr input, int &flags )
	{
		bool fingerIsWithin = false;

#ifndef CLI_CPP

		if ( !LockAcquireA ( accessMutex, "IsFingerInDeviceMarker" ) )
			return false;

		map<int, MarkerDevicePack *>::iterator markerDevicesIt = markerDevices.begin ();

		while ( markerDevicesIt != markerDevices.end () )
		{
			MarkerDevicePack *  markerDevice	= markerDevicesIt->second;
			if ( markerDevice ) {
				lib::InputPackRaw * marker = &markerDevice->input.pack.raw;
				lib::InputPackRaw * cur = &input->pack.raw;

				CVerbVerbArg ( "IsFingerInDeviceMarker: Finger [%i/%i] Device [%i/%i].", cur->x, cur->y, marker->x, marker->y );

				if ( abs ( marker->x - cur->x ) < markerDevice->occlusionValue && abs ( marker->y - cur->y ) < markerDevice->occlusionValue ) {
					if ( cur->state != INPUT_STATE_DROP || !( flags & 0x1000 ) )
						fingerIsWithin = true;
					CVerbVerb ( "IsFingerInDeviceMarker: Fingeris INSIDE area occluded by device." );
					break;
				}
			}
			++markerDevicesIt;
		}

		if ( !fingerIsWithin )
			flags |= 0x1000;

		if ( !LockReleaseA ( accessMutex, "IsFingerInDeviceMarker" ) )
			return false;
#endif
		return fingerIsWithin;
	}


	void InputHandler::ClearDeviceMarkers ()
	{
#ifndef CLI_CPP
		LockAcquireA ( accessMutex, "ClearDeviceMarkers" );

		map<int, MarkerDevicePack *>::iterator markerDevicesIt = markerDevices.begin ();

		while ( markerDevicesIt != markerDevices.end () )
		{
			if ( markerDevicesIt->second )
				delete__obj ( markerDevicesIt->second );

			++markerDevicesIt;
		}
		markerDevices.clear ();

		LockReleaseA ( accessMutex, "ClearDeviceMarkers" );
#endif
	}
}

#endif 


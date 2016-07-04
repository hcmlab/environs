/**
 * DeviceController for the Windows platform
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

#if defined(_WIN32) && defined(DISPLAYDEVICE)

/// Compiler flag that enables verbose debug output
#ifndef NDEBUG
//#   define DEBUGVERB
//#   define DEBUGVERBVerb
#endif

#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "Environs.Obj.h"
#include "Device.Display.Win.h"
#include "Portal/Portal.Generator.Windows.h"
#include "Portal/Portal.Receiver.Windows.h"
using namespace environs;
using namespace environs::API;


#define	CLASS_NAME 	"Device.Controller. . . ."


/* Namespace: environs -> */
namespace environs
{
    
    
    DeviceController::DeviceController ( int deviceID ) : DevicePlatform ( deviceID )
    {
        CVerbID ( "Constructor" );
    }
    
    
    DeviceController::DeviceController ( int deviceID, bool isInteractChannel, int sock, struct sockaddr_in * addr )
    : DevicePlatform ( deviceID, isInteractChannel, sock, addr )
    {
        CVerbID ( "Constructor" );
    }
    
    
    DeviceController::~DeviceController ( )
    {
        CVerbID ( "Destructor" );
        
        CVerbID ( "Destructor done" );
    }
    
    
    /**
     * Connect to device with the given ID.
     *
     * @param deviceID	Destination device ID
     * @return	0 Connection can't be conducted (maybe environs is stopped or the device ID is invalid)
     * @return	1 A connection to the device already exists or a connection task is already in progress)
     * @return	2 A new connection has been triggered and is in progress
     */
	int DeviceController::DeviceDetected ( int hInst, int Environs_CALL_, int deviceID, const char * areaName, const char * appName, int x, int y, float angle )
    {
		return DevicePlatform::DeviceDetected ( hInst, Environs_CALL_, deviceID, areaName, appName, x, y, angle );
    }

	void DeviceController::CreatePortalGeneratorPlatform ( int portalIndex )
	{
		CVerbID ( "CreatePortalGeneratorPlatform" );

		portalGenerators [ portalIndex ] = new PortalGeneratorWindows ();
		if ( portalGenerators [ portalIndex ] )
			portalGenerators [ portalIndex ]->env = env;
	}
    
	void DeviceController::CreatePortalReceiverPlatform ( int portalIndex )
    {
        CVerbID ( "CreatePortalReceiverPlatform" );

		portalReceivers [ portalIndex ] = new PortalReceiverPlatform ();
        
		if ( !portalReceivers [portalIndex] ) {
            CErrID ( "CreatePortalReceiverPlatform: Failed to create portal receiver for windows display device." );
        }
		portalReceivers [ portalIndex ]->env = env;
	}


	void DeviceController::InjectTouch ( Input * in )
	{
		if ( localAddr.sin_addr.s_addr )
		{
			int send_size = sizeof ( Input );
			if ( in->pack.raw.state == INPUT_STATE_ADD ) {
				in->x_raw = (int) (size_t) env->appWindowHandle;
				in->y_raw = app_pid;
			}

			int sent_size = sendto ( udpSocket, (const char*) in, send_size, 0, (sockaddr *) &localAddr, sizeof ( localAddr ) );
			if ( sent_size != send_size )
			{
                CErrArgID ( "InjectTouch: Send failed %i != %i", sent_size, send_size );
                
                if ( IsValidFD ( udpSocket ) ) { LogSocketErrorF ( "DeviceBase.InjectTouch" ); }
			}
		}
	}

} // <-- namespace environs


#endif /// end->DISPLAYDEVICE


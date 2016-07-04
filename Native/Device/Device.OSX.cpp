/**
 * DeviceController for the iOS platform
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

#ifdef ENVIRONS_OSX

/// Compiler flag that enables verbose debug output
#ifndef NDEBUG
//#   define DEBUGVERB
//#   define DEBUGVERBVerb
#endif

#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "Environs.Obj.h"
using namespace environs;
using namespace environs::API;

#include "Device.OSX.h"
#include "Core/Kernel.Mobile.h"
#include "Portal/Portal.Generator.OSX.h"

#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>

#include "Portal/Portal.Receiver.iOSX.h"



#define	CLASS_NAME 	"Device.Controller. . . ."


/* Namespace: environs -> */
namespace environs
{
    
    
    DeviceController::DeviceController ( int deviceID ) : DevicePlatform ( deviceID )
    {
        CVerbID ( "Constructor" );
    }
    
    
    DeviceController::DeviceController ( int deviceID, bool mainChannel, int sock, struct sockaddr_in * addr )
    : DevicePlatform ( deviceID, mainChannel, sock, addr )
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

		portalGenerators [ portalIndex ] = new PortalGeneratorOSX ();
		portalGenerators [ portalIndex ]->env = env;
    }

    
    void DeviceController::CreatePortalReceiverPlatform ( int portalIDent )
    {
        CVerbID ( "CreatePortalReceiverPlatform" );

		portalReceivers [ portalIDent ] = ( PortalReceiver * ) new PortalReceiveriOSX ();

		if ( !portalReceivers [ portalIDent ] ) {
			CErrID ( "CreatePortalReceiverPlatform: Failed to create portal receiver for a mobile device." );
		}
		portalReceivers [ portalIDent ]->env = env;
    }

} // <-- namespace environs


#endif /// end->ENVIRONS_OSX


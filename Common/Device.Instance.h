/**
 * Device information
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
#ifndef INCLUDE_HCM_ENVIRONS_DEVICEINFO_H
#define INCLUDE_HCM_ENVIRONS_DEVICEINFO_H

#include "Environs.Types.h"
#include "Interop/Threads.h"

//#define		MAX_NAMEPROPERTY    	30

/* Namespace: environs -> */
namespace environs
{

#define DEVICES_HEADER_SIZE				20
#define DEVICE_PACKET_SIZE				sizeof(DeviceInfo)

//#define DEVICE_PACKET_SIZE				(sizeof(DeviceInstance) - (sizeof(DeviceInstance *) + sizeof(ApplicationDevices *)))
//#define DEVICE_PACKET_SIZE				(sizeof(DeviceInstance) - sizeof(DeviceInstance *))
//#define MAX_DEVICE_PACKETS_RESPONSE		(( (BUFFERSIZE - 4) - (2 * DEVICES_HEADER_SIZE)) / DEVICE_PACKET_SIZE)
    
    
    /**
     * Environs DeviceInstance struct Start bytes
     */
	typedef struct _DeviceInfo
	{
		/** The device ID within the environment */
        int				deviceID;	// 4
        
        /** The ID that is used by the native layer to identify this particular device within the environment: -1 means that this device is not connected and therefore not actively managed. */
        int				nativeID;	// 4

		/** IP from device. The IP address reported by the device which it has read from network configuration. */
		unsigned int 	ip;	// 4

		/** IP external. The IP address which was recorded by external sources (such as the Mediator) during socket connections.
		* This address could be different from IP due to NAT, Router, Gateways behind the device.
		*/
		unsigned int 	ipe; // 4 The external IP or the IP resolved from the socket address

		/** The tcp port on which the device listens for device connections. */
		unsigned short	tcpPort; // 2

		/** The udp port on which the device listens for device connections. */
		unsigned short	udpPort;  // 2

		/** The number of alive updates noticed by the mediator layer since its appearance within the application environment. */
		unsigned int	updates; // 4

		/** A value that describes the device platform. */
		int				platform; // 4

		/** BroadcastFound is a value of DEVICEINFO_DEVICE_* and determines whether the device has been seen on the broadcast channel of the current network and/or from a Mediator service. */
		char			broadcastFound; // 1
		bool            unavailable;  // 1

		/** isConnected is true if the device is currently in the connected state. */
		bool			isConnected; // 1
		char			internalUpdates;  // 1

		/** Used internally by native layer. */
		char            internalType;  // 1

		/** The device name. */
		char			deviceName [MAX_NAMEPROPERTY + 1]; // 31

		/** The project name of the appliction environment. */
		char			projectName [MAX_NAMEPROPERTY + 1]; // 31

		/** The applcation name of the appliction environment. */
		char			appName [MAX_NAMEPROPERTY + 1]; // 31
	}
	DeviceInfo;


	typedef struct _DeviceHeader
	{
		unsigned int	deviceCountAvailable;
		unsigned int	startIndex;
		unsigned int	deviceCount;
	}
	DeviceHeader;


	typedef struct DevicePackage
	{
		DeviceHeader	header;
		unsigned int    pad0;
		unsigned int    pad1;
		DeviceInfo		device;
		unsigned int    pad2;
	}
	DevicePackage;


} /* namepace Environs */


#endif // INCLUDE_HCM_ENVIRONS_DEVICEINFO_H

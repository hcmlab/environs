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
#pragma once
#ifndef INCLUDE_HCM_ENVIRONS_DEVICEINFO_H
#define INCLUDE_HCM_ENVIRONS_DEVICEINFO_H

#include "Environs.Types.h"
#include "Environs.Platform.Decls.h"
#include "Interop/Threads.h"


/** Place declarations to global namespace for plain C */
#ifdef __cplusplus

/* Namespace: environs -> */
namespace environs
{
	namespace lib
	{
#endif
        
		NET_PACK_PUSH1

		/**
		* Environs DeviceInstance struct Start bytes
		*/
		typedef struct DeviceInfo
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
			char            hasAppEnv;  // 1

										   /** The device name. */
			char			deviceName [ MAX_LENGTH_DEVICE_NAME ]; // 31

																   /** The area name of the appliction environment. */
			char			areaName [ MAX_LENGTH_AREA_NAME ]; // 31

															   /** The application name of the appliction environment. */
			char			appName [ MAX_LENGTH_APP_NAME ]; // 31
            
            /** Padding to fullfil 4 byte alignment. Compiler will do it anyway. */
            unsigned short	flags;  // 2
            
#ifndef MEDIATORDAEMON
			OBJIDType		objID; // 4
#endif
		}
        NET_PACK_ALIGN DeviceInfo;
        
        
#define DEVICE_INFO_CLIENT_SIZE     ( sizeof(DeviceInfo) - sizeof(OBJIDType) )
        
        /**
         * Environs DeviceInstance struct Start bytes
         */
        typedef struct DeviceInfoShort
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
            char            hasAppEnv;  // 1
            
            /** The device name. */
            char			deviceName [ MAX_LENGTH_DEVICE_NAME ]; // 31
        }
		NET_PACK_ALIGN DeviceInfoShort;


		typedef struct DeviceHeader
		{
			unsigned int	deviceCountAvailable;
			unsigned int	startIndex;
			unsigned int	deviceCount;
		}
		NET_PACK_ALIGN DeviceHeader;


		typedef struct DevicePack
		{
			DeviceHeader	header;
			unsigned int    pad0;
			unsigned int    pad1;
			DeviceInfo		device;
			unsigned int    pad2;
		}
		NET_PACK_ALIGN DevicePack;

		NET_PACK_POP

#define DEVICES_HEADER_SIZE				20
#define DEVICES_HEADER_SIZE_V6			24
#define DEVICE_PACKET_SIZE				sizeof(DeviceInfo)
#define DEVICE_MEDIATOR_PACKET_SIZE		(sizeof(DeviceInfo) - sizeof(OBJIDType))
        
#ifdef __cplusplus
    }



#ifdef CLI_CPP
	PUBLIC_CLASS DeviceInfo
	{
	public:
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
		char            hasAppEnv;  // 1

		/** The device name. */
		System::String^	deviceName; // 31

		/** The area name of the appliction environment. */
		System::String^	areaName; // 31

		/** The application name of the appliction environment. */
        System::String^	appName; // 31
        
        /** The udp port on which the device listens for device connections. */
        unsigned short	flags;  // 2
        
		OBJIDType		objID;
	};
#else
    
    typedef lib::DeviceInfo         DeviceInfo;
    typedef lib::DeviceInfoShort	DeviceInfoShort;
    
#endif


} /* namepace Environs */
#endif

#endif // INCLUDE_HCM_ENVIRONS_DEVICEINFO_H

/**
 * DeviceController for the Windows Phone platform
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
#ifndef DISPLAYDEVICE

#ifndef INCLUDE_HCM_ENVIRONS_DEVICECONTROLLER_IOS_H_
#define INCLUDE_HCM_ENVIRONS_DEVICECONTROLLER_IOS_H_


/**** Includes ****/
#include "Environs.Native.h"
#include "Devices.h"
#include "Device.Mobile.h"

#include "winsock2.h"
#include "Interop/Threads.h"

#include <map>
using namespace std;


/* Namespace: Environs -> */
namespace environs
{

	class DeviceController;

    
    /**
     *	DeviceController for the Windows Phone platform
     *
     *	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
     *	@version	1.0
     * ****************************************************************************************
     */
	class DeviceController : public DevicePlatform
	{
	public:
		DeviceController ( int deviceID );
		DeviceController ( int deviceID, bool mainChannel, int sock, struct sockaddr_in * addr );
		void					Construct ();

		~DeviceController ();
		void					Release ();
		static void 			RemoveAllDevices ();

		static int				activePortalID;
		static int				requestedPortalID;
		static struct sockaddr_in activePortalUdpAddr;
		static int				activePortalSocket;

		void					PerformEnvironsTouch ( Input * touch );
		void					UpdatePosition ( int x, int y );
		void					UpdatePortalsize ( int width, int height );

		static bool				SendDataPacket ( const char * msg, int length );

		void					CreatePortalReceiverPlatform ( int portalIDent );
		void					CreatePortalGeneratorCustom ( int portalID );
        
		static int              DeviceDetected ( int hInst, int Environs_CALL_, int deviceID, const char * areaName, const char * appName, int x, int y, float angle );
        
	private:
		void					TuneReceiveBuffer ( int sock );

		void					OnTcpListenerClosed ();
		void					OnConnectionEstablished ();
		void					OnUdpConnectionEstablished ();

		void 					HandleOptionsMessage ( unsigned short payloadType, char * payload );

		void					ProccessPortalProvided ( int portalID, PortalStreamType_t streamType );
		void					ProccessPortalStartAck ( int portalID );
		void					ProccessPortalPauseAck ( int portalID );
		void					ProccessPortalStopAck ( int portalID );

		bool 					EvaluateDeviceConfig ( char * msg );
	};


}

#endif /* INCLUDE_HCM_ENVIRONS_DEVICECONTROLLER_IOS_H_ */

#endif /// end->DISPLAYDEVICE


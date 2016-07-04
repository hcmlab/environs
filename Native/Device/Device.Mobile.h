/**
 * DeviceController for mobile device platforms
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
#ifndef INCLUDE_HCM_ENVIRONS_DEVICECONTROLLER_H_
#define INCLUDE_HCM_ENVIRONS_DEVICECONTROLLER_H_

#ifndef DISPLAYDEVICE


/**** Includes ****/
#include "Environs.Native.h"
#include "Devices.h"

#ifdef WINDOWS_PHONE
#	include "winsock2.h"
#	include "Interop/Threads.h"
#else
#	include <sys/socket.h>
#	include <netinet/in.h>
#	include <pthread.h>
#endif
#include <map>
using namespace std;

/* Namespace: Environs -> */
namespace environs
{

	class DeviceController;

	// DeviceEntity is one item within our static device list
	typedef struct DeviceEntity
	{
		unsigned int 			id;
		pthread_t				handshakeThread;
		DeviceController 	*	device;
		DeviceEntity		*	next;
	}
	DeviceEntity;


	class DevicePlatform : public DeviceBase
	{
	public:
        
        DevicePlatform ( );
		DevicePlatform ( int deviceID );
		DevicePlatform ( int deviceID, bool isInteractChannel, int sock, struct sockaddr_in * addr );
		void					Construct ();

		~DevicePlatform ();

		void					Release ();
		static void 			RemoveAllDevices ();

		void					PerformEnvironsTouch ( Input * pack );
		void					UpdatePosition ( int x, int y );
		void					UpdatePortalsize ( int width, int height );

		virtual void			CreatePortalGeneratorPlatform ( int portalIDent );
        
		static int				DeviceDetected ( int hInst, int Environs_CALL_, int deviceID, const char * areaName, const char * appName, int x, int y, float angle );

	private:
		void					TuneReceiveBuffer ( int sock );

		void					OnInteractListenerClosed ();
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

#endif /// end->DISPLAYDEVICE

#endif /* _ENVIRONS_DEVICECONTROLLER_H_ */

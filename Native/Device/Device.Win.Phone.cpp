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
#include "stdafx.h"

#ifndef DISPLAYDEVICE

#include <errno.h>

//#define DEBUGVERB

#include "Environs.Obj.h"
using namespace environs::API;

#include "Environs.Modules.h"

#include "Device.Controller.h"
#include "Mediator.h"
#include "Core/Kernel.Mobile.h"
#include "Portal/Portal.Stream.h"
#include "Portal/Portal.Generator.Mobile.h"

#ifdef WINDOWS_PHONE
#include "winsock2.h"
#else
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#ifdef ENABLE_NATIVE_H264DECODER
#ifdef ANDROID
#include "Capture/AndroidScreen.h"
#include "Portal/PortalReceiverAndroid.h"
#else
#include "Portal/Portal.Generator.WP.h"
#endif
#endif

#define	CLASS_NAME 	"DeviceController"


/* Namespace: environs -> */
namespace environs
{


	extern void UpdateString ( const char * src, char ** dest );


	bool startTouchSource ( int deviceID, const char * areaName, const char * appName )
	{
		return true;
	}


	bool stopTouchSource ( int deviceID, const char * areaName, const char * appName )
	{

		return true;
	}


#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable: 4996 )
#endif

	bool DeviceController::SendDataPacket ( const char * msg, int length )
	{
		if ( length <= 0 ) {
			CErr ( "SendDataPacket: Length of message <= 0!" );
			return false;
		}

		//	if ( activePortalSocket <= 0 || !activePortalUdpAddr.sin_addr.s_addr || !activePortalUdpAddr.sin_port )
		//		return false;

		if ( activePortalSocket == -1 ) //|| !activePortalUdpAddr.sin_addr.s_addr || !activePortalUdpAddr.sin_port )
			return false;

		//CLogArgID ( "Sending packet to IP [%s] port [%d] socket [%d] ", inet_ntoa (g_activePortalUdpAddr.sin_addr), ntohs ( g_activePortalUdpAddr.sin_port ), g_activePortalSocket );

		//	int sentBytes = sendto ( activePortalSocket, msg, length, 0, (struct sockaddr*)&activePortalUdpAddr, sizeof(struct sockaddr));
		int sentBytes = ( int ) send ( activePortalSocket, msg, length, 0 );

		if ( sentBytes < length ) {
			CErrArg ( "SendDataPacket: Failed to send packet to device! sent bytes and length of packet mismatch (%i/%i); (%s)",
				sentBytes, length, strerror ( errno ) );

			activePortalSocket = -1;

			return false;
		}

		return true;
	}

#ifdef _WIN32
#pragma warning( pop )
#endif


	DeviceController::DeviceController ( int deviceID ) : DevicePlatform ( deviceID )
	{
		CVerbID ( "Constructor" );
	}


	DeviceController::DeviceController ( int deviceID, bool mainChannel, int sock, struct sockaddr_in * addr )
		: DevicePlatform ( deviceID, mainChannel, sock, addr )
	{
		CVerbID ( "Constructor" );
	}


	DeviceController::~DeviceController ()
	{
		CInfoID ( "Destructor" );

		Release ();


		CVerbID ( "Destructor done" );
	}


	void DeviceController::Release ()
	{
		CVerbID ( "Release" );

		stopTouchSource ( deviceID, deviceAreaName, deviceAppName );

		DeviceController::activePortalSocket = -1;

		// try using virtual destructor of Network and calling ~Network from DeviceController
		//Networker::Release();

		/// Close connector thread at very fist in case we are about to connect to the deviceID
		/// Cause connection makes use of our virtual Handshake methods
		/// Otherwise the app will crash.
		CloseConnectorThread ();
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


	void DeviceController::CreatePortalGeneratorCustom ( int portalID )
	{
		CVerbID ( "CreatePortalSourceCustom" );

		unsigned int id = portalID & 0xFF;

		portalGenerators [ id ] = new PortalGeneratorMobile ();
	}


	void DeviceController::CreatePortalReceiverPlatform ( int portalIDent )
	{
		CVerbID ( "CreatePortalReceiverPlatform" );

	}


	void DeviceController::TuneReceiveBuffer ( int sock )
	{
#ifdef DISABLE_BUFFER_TUNING_FOR_NAT
		if ( behindNAT )
			return;
#endif

		int value = PORTAL_SOCKET_BUFFER_SIZE_NORMAL;
		/*if ( opt_useNativeResolution )
			value = PORTAL_SOCKET_BUFFER_SIZE_NATRES;*/

		if ( setsockopt ( sock, SOL_SOCKET, SO_RCVBUF, ( char * ) &value, sizeof ( value ) ) < 0 ) {
			CErrArgID ( "TuneReceiveBuffer: Failed to set socket receive buffer size! [%i bytes]", value );
			LogSocketError ();
		}
	}


	void DeviceController::ProccessPortalProvided ( int portalID, PortalStreamType_t streamType )
	{
		CVerbArgID ( "ProccessPortalProvided: type [%i]", streamType );

		if ( portalID & PORTAL_DIR_INCOMING )
		{
			/*memcpy ( &activePortalUdpAddr, &udpAddr, sizeof ( activePortalUdpAddr ) );
			activePortalSocket = udpSocketID;*/

			if ( !startTouchSource ( deviceID, deviceAreaName, deviceAppName ) ) {
				CWarnID ( "ProccessPortalProvided: Starting touch source failed." );
			}

			requestedPortalID = deviceID;
		}

		DeviceBase::ProccessPortalProvided ( portalID, streamType );
	}


	void DeviceController::ProccessPortalStartAck ( int portalID )
	{
		CVerbID ( "ProccessPortalStartAck" );

		if ( portalID & PORTAL_DIR_INCOMING )
		{
			activePortalID = deviceID;

			/*memcpy ( &activePortalUdpAddr, &udpAddr, sizeof ( activePortalUdpAddr ) );
			activePortalSocket = udpSocketID;

			if ( kernel->touchSource )
				kernel->touchSource->deviceID = deviceID;*/
		}


		DeviceBase::ProccessPortalStartAck ( portalID );
	}


	void DeviceController::ProccessPortalPauseAck ( int portalID )
	{
		CVerbID ( "ProccessPortalPauseAck" );

		if ( portalID & PORTAL_DIR_INCOMING )
		{
			stopTouchSource ( deviceID, deviceAreaName, deviceAppName );

			if ( activePortalID == deviceID )
				activePortalID = 0;
		}

		DeviceBase::ProccessPortalPauseAck ( portalID );
	}


	void DeviceController::ProccessPortalStopAck ( int portalID )
	{
		CVerbID ( "ProccessPortalStopAck" );

		if ( portalID & PORTAL_DIR_INCOMING )
		{
			stopTouchSource ( deviceID, deviceAreaName, deviceAppName );

			if ( activePortalID == deviceID )
				activePortalID = 0;
		}

		DeviceBase::ProccessPortalStopAck ( portalID );
	}


	void DeviceController::HandleOptionsMessage ( unsigned short payloadType, char * payload )
	{
		CVerbID ( "HandleOptionsMessage" );

		if ( payloadType == MSG_OPT_CONTACT_DIRECT_SET ) {
			/*inDirectContact = ( *( ( unsigned int * ) payload ) != 0 );

			CLogArgID ( "HandlePortalMessage: Device / Display contact has changed [%d]!", inDirectContact );


			onEnvironsNotifier ( env, deviceID, deviceAreaName, deviceAppName, NOTIFY_CONTACT_DIRECT_CHANGED );*/
		}
		else
			DeviceBase::HandleOptionsMessage ( payloadType, payload );
	}


	void DeviceController::OnTcpListenerClosed ()
	{
		CVerbID ( "OnTcpListenerClosed" );

		stopTouchSource ( deviceID, deviceAreaName, deviceAppName );

		DeviceBase::OnTcpListenerClosed ();
	}


	void DeviceController::OnConnectionEstablished ()
	{
		CVerbID ( "OnConnectionEstablished" );

		DeviceBase::OnConnectionEstablished ();
	}


	void DeviceController::OnUdpConnectionEstablished ()
	{
		CVerbID ( "OnUdpConnectionEstablished" );

		//activePortalSocket  = udpSocketID;
		//memcpy ( &activePortalUdpAddr, &udpAddr, sizeof ( activePortalUdpAddr ) );

		DeviceBase::OnUdpConnectionEstablished ();
	}


	void DeviceController::PerformEnvironsTouch ( Input * touch )
	{
	}


	void DeviceController::UpdatePosition ( int x, int y )
	{
	}


	void DeviceController::UpdatePortalsize ( int width, int height )
	{
	}



} // <-- namespace environs


using namespace environs;

#endif /// end->DISPLAYDEVICE


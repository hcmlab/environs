/**
 * DeviceBase for stationary display devices
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
#ifndef INCLUDE_HCM_ENVIRONS_DEVICECONTROLLER_PLATFORM_H
#define INCLUDE_HCM_ENVIRONS_DEVICECONTROLLER_PLATFORM_H

#ifdef DISPLAYDEVICE

#include "Devices.h"
#include "Portal/Portal.Generator.h"
#include "Human.Input.Decl.h"

#include <vector>

#ifdef _WIN32
#	pragma warning( push )
#	pragma warning( disable: 4458 )
#	include <gdiplus.h>
#   pragma warning( pop )
#endif


namespace environs
{
	/**
	*	DeviceBase for stationary display devices
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@version	1.0
	*	@remarks
	* ****************************************************************************************
	*/
	class DevicePlatform : public DeviceBase
	{
	public:
		static bool					inject_touch;
		static int					app_pid;

		float						orientationLast;
		float						deviceAzimut;
		float						deviceAzimutLast;

		int							frameNumber;
		int							seqNumberOrientation;

	private:
		friend class PortalGenerator;

		bool						allocated;

	public:
		bool                        Init ( const sp ( Instance ) &envObj, const char * areaName, const char * appName );

		void						PerformEnvironsTouch ( Input * touch );

	private:
#ifdef ENABLE_RECOGNIZERS_OBJECT_USAGE
		Recognizers				*	touchRecognizers;
#endif

		static pthread_cond_t		hPortalUpdateTimer;

		pthread_mutex_t				inputsMutex;
		int							inputsCountCurrent;
		std::vector<Input *>	*	inputs;
		std::vector<Input *>	*	inputsTemp;

		void						TuneReceiveBuffer ( int sock );

		void						OnInteractListenerClosed ( );
		void						OnPreConnectionEstablished ( );

		virtual void				CreatePortalGeneratorPlatform ( int portalIndex );
		virtual void				CreatePortalReceiverPlatform ( int portalIndex );

		void						HandleOptionsMessage ( unsigned short payloadType, char * payload );
		
		void						TranslateToDisplayCoord ( RenderDimensions * dims, environs::lib::InputPackRaw * pack );

		bool						EvaluateDeviceConfig ( char * msg );
		void						SendStreamPacketOverTcp ( );
		void						ClearTouches ( bool execTouch );
		virtual void				InjectTouch ( Input * pack ) {};

	public:
		DevicePlatform ( int deviceID );
		DevicePlatform ( int deviceID, bool isInteractChannel, int sock, struct sockaddr_in * addr );
		void                        Construct ( );

		~DevicePlatform ( void );
        void						Release ( );
        virtual void				DisposePlatform ();


		static int					DeviceDetected ( int hInst, int Environs_CALL_, int deviceID, const char * areaName, const char * appName, int x, int y, float angle );

		bool						IsIP ( unsigned int ip );

		void						HandleTouchPacket ( char * buffer, int length );

	protected:
		sockaddr_in					localAddr;
	};


} /* namespace environs */

#endif /// end->DISPLAYDEVICE

#endif	/// INCLUDE_HCM_ENVIRONS_DEVICECONTROLLER_PLATFORM_H


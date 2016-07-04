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
#pragma once
#if defined(_WIN32) && defined(DISPLAYDEVICE)

#ifndef INCLUDE_HCM_ENVIRONS_DEVICECONTROLLER_WINDOWS_DISPLAY_H_
#define INCLUDE_HCM_ENVIRONS_DEVICECONTROLLER_WINDOWS_DISPLAY_H_


/**** Includes ****/
#include "Device.Display.h"


/* Namespace: environs -> */
namespace environs
{
    /**
     *	DeviceController for the Windows platform
     *
     *	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
     *	@version	1.0
     * ****************************************************************************************
     */
	class DeviceController : public DevicePlatform
	{
	public:
		DeviceController ( int deviceID );
        DeviceController ( int deviceID, bool isInteractChannel, int sock, struct sockaddr_in * addr );
        
        ~DeviceController ();
        
		static int              DeviceDetected ( int hInst, int Environs_CALL_, int deviceID, const char * areaName, const char * appName, int x, int y, float angle );
		void					InjectTouch ( Input * pack );

		void					CreatePortalGeneratorPlatform ( int portalIndex );
		void					CreatePortalReceiverPlatform ( int portalIndex );
	};

}

#endif /* INCLUDE_HCM_ENVIRONS_DEVICECONTROLLER_WINDOWS_DISPLAY_H_ */

#endif /// end->DISPLAYDEVICE


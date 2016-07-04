/**
 *	Portal Device Mapping and Management
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
#ifndef INCLUDE_HCM_ENVIRONS_PORTAL_DEVICE_MANAGEMENT_H
#define INCLUDE_HCM_ENVIRONS_PORTAL_DEVICE_MANAGEMENT_H


#include "Portal/Portal.Generator.h"
#include "Portal/Portal.Receiver.h"

#define USE_PORTAL_DEVICES_INTERLOCK_COUNTERS

namespace environs 
{
    class DeviceBase;
    
	/**
	*	Portal Device Mapping and Management
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@version	1.0
	* ****************************************************************************************
	*/
	typedef struct PortalDevice {
		//pthread_mutex_t     mutex;
        
        LONGSYNC           ticketCount;
        LONGSYNC           door;
        
		int                 portalUnitType;
		int                 portalID;
		DeviceBase      *   device;
		PortalStream    *   stream;

		PortalGenerator *   generator;

		PortalReceiver  *   receiver;

		int                 frameCounter;
	}
    PortalDevice;
    
    extern PortalDevice portalDevices [MAX_PORTAL_INSTANCES];
    

    extern bool InitPortalDevices ();
    extern void ReleasePortalDevices ();
	extern void StopAllPortalDevices ( DeviceBase * device );

    extern int  GetFreePortalSlot ( DeviceBase * device, int portalID );

	extern void DisposePortalDevice ( int portalID );

	extern PortalDevice * GetLockedPortalDevice ( int portalID, bool checkDevice = true );

	extern bool HoldPortalDevice ( PortalDevice * portal );
	extern bool ReleasePortalDevice ( PortalDevice * portal );

	extern PortalDevice * HoldPortalDeviceID ( int portalIndex );
	extern bool ReleasePortalDeviceID ( int portalIndex );

    extern bool ReleasePortalDeviceID ( int portalID );
    extern DeviceBase * GetDeviceIncLock ( int portalID );
    
} /* namespace environs */


#endif // INCLUDE_HCM_ENVIRONS_PORTAL_DEVICE_MANAGEMENT_H

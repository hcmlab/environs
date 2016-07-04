/**
 * Device management for devices
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
#ifndef INCLUDE_HCM_ENVIRONS_DEVICEMANAGER_H_
#define INCLUDE_HCM_ENVIRONS_DEVICEMANAGER_H_

#include "Device.Base.h"

namespace environs
{
#ifdef __cplusplus

    class Instance;
    
    extern bool							RemoveDevice ( Instance * env, int deviceID, const char * areaName, const char * appName, bool useLock = true );
    extern bool							WaitForDeviceDeletion ( Instance * env, int deviceID, const char * areaName, const char * appName, bool keepLocked = false );
    
    extern void                         DismissOtherDevices ( Instance * env, int skipNativeID, int deviceID, const char * areaName, const char * appName );
    
    extern DeviceBase		*			GetDevice ( Instance * env, int deviceID, const char * areaName, const char * appName, bool useLock = true );
	extern sp ( DeviceController )      GetDeviceSP ( Instance * env, int deviceID, const char * areaName, const char * appName, bool useLock = true );
#endif
    
    extern pthread_mutex_t              devicesAccessMutex;
    extern bool							RemoveAllDevices ( Instance * env );
    extern bool							PrepareRemovalOfAllDevices ( Instance * env );

	extern bool                         InitDevicesMap ();
    extern void                         DisposeDevicesMap ();

#ifdef __cplusplus
    extern int							AddDevice ( const sp ( DeviceController ) &device, bool useLock = true );
    extern bool							TryAddDevice ( const sp ( DeviceController ) &device );
#endif

    extern bool                         RemoveDevice ( int nativeID, bool useLock = true );
    extern DeviceBase *                 GetDevice ( int nativeID, bool useLock = true );
    
	extern void							IncLockDevice ( DeviceBase * device );
	extern void							UnlockDevice ( DeviceBase * device );
    
    extern DeviceBase		**			GetDeviceMap ();
	extern int                          GetConnectedDevicesManagedCount ( );
    
    extern int                          GetConnectedDevicesManagedLast ( );
    
	extern void							TriggerCleanUpDevices ( );
    extern int                          GetActivePortals ( );

} // <-- namespace environs

#endif /* INCLUDE_HCM_ENVIRONS_DEVICEMANAGER_H_ */

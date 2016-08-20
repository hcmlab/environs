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
#include "stdafx.h"

/// Compiler flag that enables verbose debug output
#ifndef NDEBUG
//#   define DEBUGVERB
//#   define DEBUGVERBVerb
#endif

#ifdef _WIN32
#   pragma warning( disable: 4503 )
#endif

#include "Devices.h"
#include "Device/Device.Controller.h"
#include "Environs.Obj.h"
#include "Environs.Sensors.h"
#include "Environs.Utils.h"
#include "Tracer.h"
#include <string>

using namespace environs::API;
using namespace std;

// The TAG for prefixing log messages
#define CLASS_NAME	"Devices. . . . . . . . ."

namespace environs
{
	pthread_mutex_t					devicesAccessMutex;
    
    int                             devicesMapCount = 0;
    int                             devicesMapLast = 0;
    
    DeviceBase *                    devicesMap [ MAX_CONNECTED_DEVICES ];
	    
    bool InitDevicesMap ()
    {
        Zero ( devicesMap );
         
        if ( !LockInitA ( devicesAccessMutex ) )
            return false;
        
        devicesMapCount = 0;
        
		return true;
    }
    
    
    void DisposeDevicesMap ()
    {
        LockDisposeA ( devicesAccessMutex );
    }
    
    
    DeviceBase ** GetDeviceMap ()
    {
        return devicesMap;
    }
    
    
    int GetConnectedDevicesManagedCount ( )
    {
        CVerb ( "GetConnectedDevicesManagedCount" );
        
        return devicesMapCount;
    }
    
    
    int GetConnectedDevicesManagedLast ( )
    {
        CVerb ( "GetConnectedDevicesManagedLast" );
        
        return devicesMapLast;
    }
    
    
    int GetFreeSlot ()
    {
        for ( int nativeID = 1; nativeID < MAX_CONNECTED_DEVICES; ++nativeID ) {
            if ( !devicesMap [ nativeID ] ) {
                return nativeID;
            }
        }
        return 0;
    }
    
    
    void UpdateLast ( int start )
    {
        if ( start < 1 ) {
            devicesMapLast = 0;
            return;
        }
        
        for ( int nativeID = start; nativeID > 0; --nativeID ) {
            if ( devicesMap [ nativeID ] ) {
                devicesMapLast = nativeID;
                return;
            }
        }
        devicesMapLast = 0;
    }

    
    
    void IncLockDevice ( DeviceBase * device )
    {
        if ( !device ) {
            //CErr ( "IncLockDevice: invalid (NULL) argument." );
            return;
        }
        
#ifdef DEBUGVERB
        int deviceID = device->deviceID;
        CVerbID ( "IncLockDevice" );
#endif
        
        __sync_add_and_fetch ( &device->accessLocks, 1 );
    }
    
    
    void UnlockDevice ( DeviceBase * device )
    {
        if ( !device ) {
            //CErr ( "UnlockDevice: invalid (NULL) argument." );
            return;
        }
        
#ifdef DEBUGVERB
        int deviceID = device->deviceID;
        CVerbID ( "UnlockDevice" );
#endif
        
        __sync_sub_and_fetch ( &device->accessLocks, 1 );
    }
    

#ifndef NDEBUG
	extern void _EnvPushPanicMessage ( const char * msg );
#endif

    /*
	* Note: using const & to deviceSP somehow modifies the use_count and leads to inconsistent checks
	*/
	void DisposeDevice ( sp ( DeviceController ) const & deviceSP )
	{
		CVerb ( "DisposeDevice" );

		if ( !deviceSP ) return;

        DeviceBase * device = deviceSP.get ();
        
		int deviceID = device->deviceID;

		CVerbsID ( 10, "DisposeDevice" );

        if ( device->env )
            DisposeSensorSender ( device->env->hEnvirons, device );

		/// Try releasing and then wait if neccessary
		device->DisposePlatform ();

		device->Dispose ();

		CVerbsArgID ( 10, "DisposeDevice: [ %u ]", device->accessLocks );

        int count = 0;

		if ( ___sync_val_compare_and_swap ( &device->accessLocks, OBJECTSTATE_DELETEABLE_1, OBJECTSTATE_DELETED ) != OBJECTSTATE_DELETEABLE_1 )
        {
			// Acquire a reference lock
			__sync_add_and_fetch ( &device->accessLocks, 1 );

			int waitCycles = 0;

			while ( ___sync_val_compare_and_swap ( &device->accessLocks, OBJECTSTATE_DELETEABLE_2, OBJECTSTATE_DELETED ) != OBJECTSTATE_DELETEABLE_2 )
			{
				if ( device->accessLocks < OBJECTSTATE_DELETEABLE_2 ) {
					CWarnArgID ( "DisposeDevice: Unexpected device access state [ %u ].", ( unsigned int ) device->accessLocks );
					break;
				}

				if ( !(count % 50) ) {
					CWarnArgID ( "DisposeDevice: Device is being accessed [ %u times ] somewhere ...", ( unsigned int ) device->accessLocks );
					
					waitCycles++;
					if ( waitCycles > 100 ) {
						// If we have waited for so long, then we assume that an exception caused the unbalanced counter
						// As we're still counting, we assume that an exception handler at application layers has "solved" the "problem"
#ifndef NDEBUG
						_EnvPushPanicMessage ( "Devices.DisposeDevice: waited > 100." );
#endif
						break;
					}
				}
				count++;

                if ( native.gcThread.WaitOne ( "DisposeDevice", 1000 ) == 0 )
                    Sleep ( 200 );
                else
                    native.gcThread.ResetSync ( "DisposeDevice" );
			}
        }

        while ( deviceSP.use_count () > 1 )
        {
            if ( !( count % 50 ) ) {
                CWarnArgID ( "DisposeDevice: SP is being shared [ %i times ] somewhere ...", ((int) deviceSP.use_count ()) - 1 );
            }
            count++;

            if ( native.gcThread.WaitOne ( "DisposeDevice", 1000 ) == 0 )
                Sleep ( 200 );
            else
                native.gcThread.ResetSync ( "DisposeDevice" );
        }
        
        CVerbsID ( 6, "DisposeDevice: Device destroyed." );
	}

    
    void * GC_EnvironsDevices ( void * arg )
    {
        CVerbs ( 6, "GC.Devices: thread started..." );
        CVerbsVerbArg ( 10, "GC.Devices: thread id [ %16X ]", GetCurrentThreadId () );
        
        int		disposed, alive, runs;
		bool	runThrough;
        
        unsigned int now;
        
    NextRun:
        disposed	= 0;
        runs		= 0;
        
        if ( !LockAcquireA ( devicesAccessMutex, "GC.Devices" ) )
            goto Finish;

	ReRun:
		runThrough = true;
		alive = 0;
		runs++;

        now = GetEnvironsTickCount32 ();
        
		CVerbsArg ( 5, "GC.Devices: Run [ %i ].", runs );
        
        for ( int nativeID = 1; nativeID <= devicesMapLast; ++nativeID )
        {
            DeviceBase * device = devicesMap [ nativeID ];
            if ( !device )
                continue;
            
            if ( device->deviceStatus > DeviceStatus::Deleteable ) {
                if ( device->deviceStatus == DeviceStatus::Connected )
                    continue;
                
                if ( (now - device->connectTime) < 60000 )
                    continue;
                
                CVerbsArg ( 5, "GC.Devices: Connect takes more than 60s. Disposing [ 0x%X ].", device->deviceID );
            }
            
            device->deviceStatus = DeviceStatus::Deleteable;
            
			if ( device->IsPreDisposed () ) {
				continue;
			}

			//LockAcquireVA ( device->spLock, "GC.Devices" );

            //sp ( DeviceController ) deviceSP = device->myself;

			//LockReleaseVA ( device->spLock, "GC.Devices" );

            //if ( !deviceSP )
            //    continue;
            
            IncLockDevice ( device );
            
            LockReleaseVA ( devicesAccessMutex, "GC.Devices" );
            
            device->PreDispose ();
            
            UnlockDevice ( device );
            
            LockAcquireVA ( devicesAccessMutex, "GC.Devices" );
        }

        for ( int nativeID = 1; nativeID <= devicesMapLast; ++nativeID )
        {
            DeviceBase * device = devicesMap [ nativeID ];
            if ( !device )
                continue;
            
            if ( device->deviceStatus != DeviceStatus::Deleteable ) {
                alive++;
                continue;
            }
            
			if ( !device->IsPreDisposed () ) {
				// Device is not predisposed, let's run the predisposal again.
				// A new garbage device seems to have appeared
				goto ReRun;
			}

            devicesMap [ nativeID ] = 0;
            devicesMapCount--;
            disposed++;
            
#ifdef ENABLE_DEVICEBASE_WEAK_REFERENCE
            sp ( DeviceController ) deviceSP = device->myselfSP;
            
            device->myselfSP = 0;
#else
			LockAcquireVA ( device->spLock, "GC.Devices" );

            sp ( DeviceController ) deviceSP = device->myself;

			device->myself = 0;

			LockReleaseVA ( device->spLock, "GC.Devices" );
#endif
			device->nativeID = 0;

			if ( !LockReleaseA ( devicesAccessMutex, "GC.Devices" ) )
				return 0;
            
            DisposeDevice ( deviceSP );
			deviceSP = 0;

			if ( !LockAcquireA ( devicesAccessMutex, "GC.Devices" ) )
				return 0;
			runThrough = false;
        }
        
        UpdateLast ( devicesMapLast );

		if ( !runThrough )
			goto ReRun;
        
        LockReleaseVA ( devicesAccessMutex, "GC.Devices" );
                
#ifndef NDEBUG
        CVerbsArg ( 10, "GC.Devices: Disposed [ %i ] devices. Devices in area/app [ %i ]", disposed, alive );
        
        if ( alive != disposed ) {
            CVerbsArg ( 1, "GC.Devices: Disposed [ %i ] devices. Devices in area/app [ %i ]", disposed, alive );
        }
#else
        if ( alive != disposed ) {
            CLogArg ( "GC.Devices: Disposed [ %i ] devices. Devices in area/app [ %i ]", disposed, alive );
        }
#endif
        
        if ( devicesMapLast > 0 && native.gcThread.WaitOne ( "GC.Devices", 10000 ) > 0 )
        {
            native.gcThread.ResetSync ( "GC.Devices" );
            
            if ( devicesMapLast > 0 ) {
                goto NextRun;
            }
        }
        
    Finish:
        CVerbs ( 0, "GC.Devices: thread terminated." );
        
        native.gcThread.Detach ( "GC.Devices" );
        return 0;
    }


	void TriggerCleanUpDevices ( )
	{
		CVerbs ( 10, "TriggerCleanUpDevices" );

        if ( !native.gcThread.Run ( pthread_make_routine ( &GC_EnvironsDevices ), 0, "TriggerCleanUpDevices" ) )
            native.gcThread.Notify ( "TriggerCleanUpDevices" );
	}

	//
    // If the nativeID is >= 0, then devicesAccessMutex is locked and needs to be unlocked
	// Otherwise, the mutex is not locked
	//
	int GetNativeIDLocked ( Instance * env, int deviceID, const char * areaName, const char * appName, bool useLock, bool keepLocked = false )
    {
        CVerbID ( "GetNativeIDLocked" );
        
        if ( useLock && !LockAcquireA ( devicesAccessMutex, "GetNativeIDLocked" ) ) {
            return -1;
        }
        
        const char * careaName = areaName;
        const char * cappName  = appName;
        
        if ( !areaName || !*areaName ) {
            careaName = env->areaName;
            areaName = 0;
        }
        if ( !appName || !*appName ) {
            cappName = env->appName;
            appName = 0;
        }
        
        for ( int nativeID = 1; nativeID <= devicesMapLast; nativeID++ )
		{
            DeviceBase * device = devicesMap [ nativeID ];
            if ( device )
            {
                if ( device->deviceID == deviceID )
                {
                    const char * dareaName = device->deviceAreaName ? device->deviceAreaName : env->areaName;
                    const char * dappName  = device->deviceAppName ? device->deviceAppName : env->appName;
                    
                    if (  ( careaName == dareaName || !strncmp ( dareaName, careaName, MAX_NAMEPROPERTY ) )
                        &&
                        ( cappName == dappName || !strncmp ( dappName, cappName, MAX_NAMEPROPERTY ) )
                        ) {
                        CVerbVerbID ( "GetNativeIDLocked: Found." );
                        
                        if ( device->deviceStatus == DeviceStatus::Deleteable ) {
                            CVerbVerbID ( "GetNativeIDLocked: Device is marked as disposed." );
                            break;
                        }
                        return nativeID;
                    }
                }
            }
        }

		if ( useLock && !keepLocked ) {
			LockReleaseA ( devicesAccessMutex, "GetNativeIDLocked" );
		}
        return 0;
    }
    
    
    bool RemoveDevice ( int nativeID, bool useLock )
    {
        CVerbIDN ( "RemoveDevice" );
        
		if ( useLock && !LockAcquireA ( devicesAccessMutex, "RemoveDevice" ) )
			return false;
        
        DeviceBase * device = devicesMap [ nativeID ];
        if ( device ) {
#ifdef DEBUGVERB
            int deviceID = device->deviceID;
#endif
            devicesMap [ nativeID ] = 0;
            devicesMapCount--;
            
#ifdef ENABLE_DEVICEBASE_WEAK_REFERENCE
            sp ( DeviceController ) deviceSP = device->myselfSP;
            
            device->myselfSP = 0;
#else
			LockAcquireVA ( device->spLock, "RemoveDevice" );

            sp ( DeviceController ) deviceSP = device->myself;

			device->myself = 0;

			LockReleaseVA ( device->spLock, "RemoveDevice" );
#endif
			device->nativeID = 0;
            
            if ( nativeID == devicesMapLast )
                UpdateLast ( nativeID );
            
            CVerbID ( "RemoveDevice: Setting deviceStatus to Deleteable." );
            device->deviceStatus = DeviceStatus::Deleteable;
            
            if ( useLock ) {
				LockReleaseA ( devicesAccessMutex, "RemoveDevice" );
            }
            useLock = false;
            
            DisposeDevice ( deviceSP );
			deviceSP = 0;
        }
        
        if ( useLock ) {
            LockReleaseVA ( devicesAccessMutex, "RemoveDevice" );
        }
        return (device != 0);
    }
    
    
    void DismissOtherDevices ( Instance * env, int skipNativeID, int deviceID, const char * areaName, const char * appName )
    {
        CVerbID ( "DismissOtherDevices" );
        
        if ( !LockAcquireA ( devicesAccessMutex, "DismissOtherDevices" ) )
            return;
        
        const char * careaName = areaName;
        const char * cappName  = appName;
        
        if ( !areaName || !*areaName ) {
            careaName = env->areaName;
            areaName = 0;
        }
        if ( !appName || !*appName ) {
            cappName = env->appName;
            appName = 0;
        }
        
        int found = 0;
        
        for ( int nativeID = 1; nativeID <= devicesMapLast; nativeID++ )
        {
            if ( nativeID == skipNativeID )
                continue;
            
            DeviceBase * device = devicesMap [ nativeID ];
            if ( device )
            {
                const char * dareaName = device->deviceAreaName ? device->deviceAreaName : env->areaName;
                const char * dappName  = device->deviceAppName ? device->deviceAppName : env->appName;
                
                if ( device->deviceID == deviceID && ( careaName == dareaName || !strncmp ( dareaName, careaName, MAX_NAMEPROPERTY ) )
                    &&
                    ( cappName == dappName || !strncmp ( dappName, cappName, MAX_NAMEPROPERTY ) )
                    ) {
                    CVerbVerbID ( "DismissOtherDevices: Found." );
                    
                    device->deviceStatus = DeviceStatus::Deleteable;
                    CVerbVerbID ( "DismissOtherDevices: Marked device as disposed." );
                    
                    found++;
                }
            }
        }
        
        LockReleaseA ( devicesAccessMutex, "DismissOtherDevices" );
        
        if ( found > 0 )
            TriggerCleanUpDevices ();
    }
    
    
	bool RemoveDevice ( Instance * env, int deviceID, const char * areaName, const char * appName, bool useLock )
    {
        CVerbID ( "RemoveDevice" );
        
		int nativeID = GetNativeIDLocked ( env, deviceID, areaName, appName, useLock );        
		if ( nativeID <= 0 )
			return false;

		DeviceBase * device = devicesMap [ nativeID ];
		if ( device ) {
			devicesMap [ nativeID ] = 0;
            devicesMapCount--;
            
#ifdef ENABLE_DEVICEBASE_WEAK_REFERENCE
            sp ( DeviceController ) deviceSP = device->myselfSP;
            
            device->myselfSP = 0;
#else
			LockAcquireVA ( device->spLock, "RemoveDevice" );

            sp ( DeviceController ) deviceSP = device->myself;

			device->myself = 0;

			LockReleaseVA ( device->spLock, "RemoveDevice" );
#endif
			device->nativeID = 0;

			if ( nativeID == devicesMapLast )
				UpdateLast ( nativeID );

			CVerbID ( "RemoveDevice: Setting deviceStatus to Deleteable." );
			device->deviceStatus = DeviceStatus::Deleteable;

			if ( useLock ) {
				LockReleaseA ( devicesAccessMutex, "RemoveDevice" );
			}
			useLock = false;

			DisposeDevice ( deviceSP );
			deviceSP = 0;
		}

		if ( useLock ) {
			LockReleaseA ( devicesAccessMutex, "RemoveDevice" );
		}
        return (device != 0);
    }
    
    
	bool RemoveAllDevices ( Instance * env )
    {
        CVerb ( "RemoveAllDevices" );
        
		if ( !LockAcquireA ( devicesAccessMutex, "RemoveAllDevices" ) )
			return false;

		for ( int nativeID = 1; nativeID <= devicesMapLast; ++nativeID )
		{
			DeviceBase * device = devicesMap [ nativeID ];
			if ( !device )
				continue;

			if ( env && device->env != env )
				continue;
			
			device->deviceStatus = DeviceStatus::Deleteable;

			if ( device->IsPreDisposed () )
				continue;

			//LockAcquireVA ( device->spLock, "RemoveAllDevices" );

			//sp ( DeviceController ) deviceSP = device->myself;

			//LockReleaseVA ( device->spLock, "RemoveAllDevices" );

			//if ( !deviceSP )
			//	continue;
            IncLockDevice ( device );

			LockReleaseVA ( devicesAccessMutex, "RemoveAllDevices" );

			device->PreDispose ();
            
            UnlockDevice ( device );

			LockAcquireVA ( devicesAccessMutex, "RemoveAllDevices" );
		}
        
        for ( int nativeID = 1; nativeID <= devicesMapLast; ++nativeID )
        {
            if ( devicesMap [ nativeID ] )
            {
                DeviceBase * device = devicesMap [ nativeID ];
                
				if ( env && device->env != env )
					continue;

                CVerbArg ( "RemoveAllDevices: Setting deviceStatus of [%i] to Deleteable.", device->deviceID );
                device->deviceStatus = DeviceStatus::Deleteable;
                
                devicesMap [ nativeID ] = 0;
                
#ifdef ENABLE_DEVICEBASE_WEAK_REFERENCE
                sp ( DeviceController ) deviceSP = device->myselfSP;
                
                device->myselfSP = 0;
#else
				LockAcquireVA ( device->spLock, "RemoveAllDevices" );

				sp ( DeviceController ) deviceSP = device->myself;

				device->myself = 0;

				LockReleaseVA ( device->spLock, "RemoveAllDevices" );
#endif
                LockReleaseVA ( devicesAccessMutex, "RemoveAllDevices" );
                
                DisposeDevice ( deviceSP );
				deviceSP = 0;
                
                LockAcquireVA ( devicesAccessMutex, "RemoveAllDevices" );
            }
        }
        
        devicesMapLast = 0;
        devicesMapCount = 0;
        
        LockReleaseVA ( devicesAccessMutex, "RemoveAllDevices" );

		// Wait for GC to detach if it is still "working" on disposal of devices
        
        native.gcThread.Notify ( "RemoveAllDevices" );
        
		native.gcThread.Join ( "RemoveAllDevices" );

        return true;
    }
    
    
    bool PrepareRemovalOfAllDevices ( Instance * env )
    {
        CVerb ( "PrepareRemovalOfAllDevices" );
        
        if ( !LockAcquireA ( devicesAccessMutex, "PrepareRemovalOfAllDevices" ) )
            return false;

        for ( int nativeID = 1; nativeID <= devicesMapLast; ++nativeID )
        {
            if ( devicesMap [ nativeID ] )
            {
                DeviceBase * device = devicesMap [ nativeID ];
                
                if ( env && device->env != env )
                    continue;
                
                CVerbArg ( "PrepareRemovalOfAllDevices: Setting deviceStatus of [%i] to Deleteable.", device->deviceID );
                device->deviceStatus = DeviceStatus::Deleteable;

				IncLockDevice ( device );
                
                LockReleaseVA ( devicesAccessMutex, "PrepareRemovalOfAllDevices" );
                
                device->PreDispose ();

				UnlockDevice ( device );
                
                LockAcquireVA ( devicesAccessMutex, "PrepareRemovalOfAllDevices" );
            }
        }
        
        LockReleaseVA ( devicesAccessMutex, "PrepareRemovalOfAllDevices" );
        
        return true;
    }
    
    
    bool TryAddDevice ( const sp ( DeviceController ) & deviceSP )
    {
		DeviceBase * device = ( DeviceBase * ) deviceSP.get ();

		if ( !device ) {
			CVerbVerb ( "TryAddDevice: Invalid device." );
			return false;
		}
        
        int deviceID = device->deviceID;
        
        CVerbID ( "TryAddDevice" );
        
		if ( !WaitForDeviceDeletion ( device->env, deviceID, device->deviceAreaName, device->deviceAppName, true ) )
            return false;
        
        bool success = false;
		int nativeID = 0;

		if ( device->env && device->env->environsState >= environs::Status::Starting )
		{
			nativeID = GetFreeSlot ();
			if ( nativeID > 0 ) {
				devicesMap [ nativeID ] = device;

                TraceDeviceBaseMapAdd ( device, nativeID );

				device->myself = deviceSP;
                
#ifdef ENABLE_DEVICEBASE_WEAK_REFERENCE
                device->myselfSP = deviceSP;
#endif
				device->nativeID = nativeID;

				devicesMapCount++;

				if ( nativeID > devicesMapLast )
					devicesMapLast = nativeID;

				success = true;
			}
		}
        
        if ( !LockReleaseA ( devicesAccessMutex, "TryAddDevice" ) ) {
            return false;
        }
        
        if ( success ) {
            sp ( MediatorClient ) mediator = device->env->mediator MED_WP;
            if ( mediator )
                mediator->UpdateDeviceState ( device, nativeID );
        }
        
        return success;
    }
    
    
	bool WaitForDeviceDeletion ( Instance * env, int deviceID, const char * areaName, const char * appName, bool keepLocked )
    {
        CVerbID ( "WaitForDeviceDeletion" );
        if ( !deviceID )
            return false;

        INTEROPTIMEVAL start    = 0;
        INTEROPTIMEVAL diff     = 0;
        int          trys       = 15;
        bool         success    = true;
        DeviceBase * pingDevice = 0;
        
        do
        {
			int nativeID = GetNativeIDLocked ( env, deviceID, areaName, appName, true, true );
            if ( nativeID < 0 )
                return false;

			if ( nativeID == 0 )
				return true;
            
            DeviceBase * device = devicesMap [ nativeID ];
            
			if ( device->deviceStatus > DeviceStatus::Deleteable ) {
				CVerbVerbID ( "WaitForDeviceDeletion: Ok." );
                
                if ( device->deviceStatus >= DeviceStatus::Connected ) {
                    pingDevice = device;
                    
                    IncLockDevice ( pingDevice );
                    break;
                }
                else if ( device->IsConnectingValid () )
                    break;
			}
            
            if ( !LockReleaseA ( devicesAccessMutex, "WaitForDeviceDeletion" ) )
                return false;

            if ( start == 0 ) {
                start = GetEnvironsTickCount32 ();
            }
            else {
                INTEROPTIMEVAL now = GetEnvironsTickCount32 ();

                diff = now - start;
                if ( diff >= 3000 ) {
                    CVerbVerb ( "WaitForDeviceDeletion: Failed." );
                    success = false; break;
                }
            }

            native.gcThread.WaitOne ( "WaitForDeviceDeletion", ( int ) ( 3000 - diff ) );

        } while ( true );
        
        if ( trys > 0 && !keepLocked && !LockReleaseA ( devicesAccessMutex, "WaitForDeviceDeletion" ) ) {
            if ( pingDevice ) {
                UnlockDevice ( pingDevice );
            }
            return false;
        }
        
        if ( pingDevice ) {
            // We send the ping only if we are not holding the lock
            // Ping may block forever which completely stops working of deviceMap access
            if ( !keepLocked )
                pingDevice->SendPing ();
            
            UnlockDevice ( pingDevice );
        }
        return success;
    }
    
    
    /// -1 means error
    /// Any other value means success
    int AddDevice ( const sp ( DeviceController ) &deviceSP, bool useLock )
    {
        int nativeID = 0;
		DeviceBase * device = deviceSP.get ();

        int deviceID = device->deviceID;
        
        CVerbID ( "AddDevice" );
        
        if ( useLock )
			RemoveDevice ( device->env, deviceID, device->deviceAreaName, device->deviceAppName );
        
		if ( useLock && !LockAcquireA ( devicesAccessMutex, "AddDevice" ) )
			return 0;
        
		if ( device->env && device->env->environsState >= environs::Status::Starting )
		{
			nativeID = GetFreeSlot ();
			if ( nativeID > 0 ) {
				devicesMap [ nativeID ] = device;

                TraceDeviceBaseMapAdd ( device, nativeID );

                device->myself          = deviceSP;
                
#ifdef ENABLE_DEVICEBASE_WEAK_REFERENCE
                device->myselfSP = deviceSP;
#endif
				device->nativeID        = nativeID;
				devicesMapCount++;

				if ( nativeID > devicesMapLast )
					devicesMapLast = nativeID;
			}
		}
        
        if ( useLock ) {
            LockReleaseVA ( devicesAccessMutex, "AddDevice" );
        }
        
        if ( nativeID > 0 ) {
            CVerbVerbID ( "AddDevice: Ok." );
            sp ( MediatorClient ) mediator = device->env->mediator MED_WP;
            if ( mediator )
                mediator->UpdateDeviceState ( device, nativeID );
        }
        return nativeID;
    }
    
    
    DeviceBase * GetDevice ( int nativeID, bool useLock )
    {
		if ( nativeID <= 0 || nativeID >= MAX_CONNECTED_DEVICES )
			return 0;

        if ( useLock && !LockAcquireA ( devicesAccessMutex, "GetDevice" ) )
            return 0;
        
        DeviceBase * device = devicesMap [ nativeID ];
        
        if ( device ) {            
            CVerbIDN ( "GetDevice: found" );
            
            if ( device->deviceStatus != DeviceStatus::Deleteable )
            {
                CVerbIDN ( "GetDevice: is Enabled" );
                
                __sync_add_and_fetch ( &device->accessLocks, 1 );
            }
            else {
                CVerbIDN ( "GetDevice: is Deleteable" );

                TriggerCleanUpDevices ( );
                device = 0;
            }
        }
        
        if ( useLock ) {
			LockReleaseA ( devicesAccessMutex, "GetDevice" );
        }
        return device;
        
    }
    

	sp ( DeviceController ) GetDeviceSP ( Instance * env, int deviceID, const char * areaName, const char * appName, bool useLock )
	{
		CVerbVerbID ( "GetDeviceSP" );

		DeviceBase * device = GetDevice ( env, deviceID, areaName, appName, useLock );
		if ( !device )
			return 0;
        
#ifdef ENABLE_DEVICEBASE_WEAK_REFERENCE
        sp ( DeviceController ) deviceSP = device->myself.lock ();
#else
		LockAcquireVA ( device->spLock, "GetDeviceSP" );

		sp ( DeviceController ) deviceSP = device->myself;

		LockReleaseVA ( device->spLock, "GetDeviceSP" );
#endif
		UnlockDevice ( device );
		return deviceSP;
	}

    
	DeviceBase * GetDevice ( Instance * env, int deviceID, const char * areaName, const char * appName, bool useLock )
    {
        CVerbArgID ( "GetDevice: [ %s ]", useLock ? "Get lock..." : "No lock" );
        
        DeviceBase * device =  0;
        
		int nativeID = GetNativeIDLocked ( env, deviceID, areaName, appName, useLock );
		if ( nativeID <= 0 )
			return 0;

		device = devicesMap [ nativeID ];
		if ( device ) {
			CVerbID ( "GetDevice: found" );

			if ( device->deviceStatus != DeviceStatus::Deleteable )
			{
				CVerbID ( "GetDevice: is Enabled" );

				__sync_add_and_fetch ( &device->accessLocks, 1 );
			}
			else {
				CVerbID ( "GetDevice: is Deleteable" );

				TriggerCleanUpDevices ();
				device = 0;
			}
		}
        
        if ( useLock && !LockReleaseA ( devicesAccessMutex, "GetDevice" ) ) {
            CWarnID ( "GetDevice: Failed to release mutex." );
        }
        return device;
    }
    
    
    int GetActivePortals ( )
    {
		if ( !LockAcquireA ( devicesAccessMutex, "GetActivePortals" ) )
			return 0;
        
        int count = 0;
        
        for ( int nativeID = 1; nativeID <= devicesMapLast; ++nativeID )
        {
            if ( devicesMap [ nativeID ] && devicesMap [ nativeID ]->GetPortalEnabled ( 0 ) )
                count++;
        }
        
        LockReleaseVA ( devicesAccessMutex, "GetActivePortals" );
        
        return count;
    }

    
    

} // <-- namespace environs



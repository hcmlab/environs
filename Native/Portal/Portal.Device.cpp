/**
 *	Platform Portal Generator
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

#include "Environs.Obj.h"
#include "Portal.Device.h"
#include "Device/Devices.h"


// The TAG for prepending to log messages
#define CLASS_NAME	"Portal.Device. . . . . ."

#define USE_PORTAL_DEVICES_INTERLOCK_COUNTERS

namespace environs 
{
	PortalDevice portalDevices [ MAX_PORTAL_INSTANCES ];
    
    pthread_mutex_t     portalDevicesMutex;

	bool InitPortalDevices () 
	{
		CVerb ( "InitPortalDevices" );

        Zero ( portalDevices );
        
        if ( !LockInit ( &portalDevicesMutex ) )
            return false;
        
		return true;
	}
    
    
    void ReleasePortalDevices ()
    {
        CVerb ( "ReleasePortalDevices" );
        
        LockDispose ( &portalDevicesMutex );
    }
    
    
	bool HoldPortalDevice ( PortalDevice * portal )
    {
		CVerbVerb ( "HoldPortalDevice" );
        
        if ( __sync_add_and_fetch ( &portal->ticketCount, 1 ) > 1 )
        {
            if ( ___sync_val_compare_and_swap ( &portal->door, 1, 1 ) == 1 )
                return true;
        }
        __sync_sub_and_fetch ( &portal->ticketCount, 1 );
		return false;
	}


	bool ReleasePortalDevice ( PortalDevice * portal ) {
		CVerbVerb ( "ReleasePortalDevice" );
        
        __sync_sub_and_fetch ( &portal->ticketCount, 1 );
        
		return true;
    }
    
    
    bool ReleasePortalDeviceID ( int portalID )
    {
        CVerbVerb ( "ReleasePortalDeviceID" );
        
        int id = GetPortalDeviceID ( portalID );
        
        CVerbArg ( "ReleasePortalDeviceID: portalID [%i] - id [%i]", portalID, id );
        
        if ( IsInvalidPortalDeviceID ( id ) ) {
            CErrArg ( "ReleasePortalDeviceID: Invalid id [%i]", id );
            return false;
        }
        
        PortalDevice * portal = portalDevices + id;
        if ( !portal ) {
            CVerbArg ( "DisposePortalDevice: PortalDevice is missing for [%i]", id );
            return false;
        }
        __sync_sub_and_fetch ( &portal->ticketCount, 1 );
        
        return true;
    }


	PortalDevice * HoldPortalDeviceID ( int portalIndex )
	{
		CVerbVerb ( "HoldPortalDeviceID" );

		if ( IsInvalidPortalDeviceID ( portalIndex ) ) {
			CVerbArg ( "HoldPortalDeviceID: Invalid portalDevice index [%i]", portalIndex );
			return 0;
		}

		PortalDevice * portal = portalDevices + portalIndex;
        if ( portal ) {
            if ( __sync_add_and_fetch ( &portal->ticketCount, 1 ) > 1 )
            {
                if ( ___sync_val_compare_and_swap ( &portal->door, 1, 1 ) == 1 )
                    return portal;
            }
            __sync_sub_and_fetch ( &portal->ticketCount, 1 );
		}

		return 0;
	}


	void StopAllPortalDevices ( DeviceBase * device )
	{
		CVerbArg ( "StopAllPortalDevices: deviceID [ %X ] - nativeID [ %X ]", device->deviceID, device->nativeID );
		
		if ( !LockAcquire ( &portalDevicesMutex, "StopAllPortalDevices" ) )
			return;

		for ( int i = 0; i < MAX_PORTAL_INSTANCES; i++ )
		{
			if ( portalDevices [i].device == device )
			{
				PortalDevice * portal = portalDevices + i;

				if ( portal->receiver ) {
					CLogArg ( "StopAllPortalDevices: Stopping incoming portal [ %X ]", portal->portalID );
					portal->receiver->StopNonBlock ();
				}
				else if ( portal->generator ) {
					CLogArg ( "StopAllPortalDevices: Stopping outgoing portal [ %X ]", portal->portalID );
					portal->generator->Stop ();
				}
			}
		}

		if ( !LockRelease ( &portalDevicesMutex, "StopAllPortalDevices" ) )
			return;
		CVerb ( "StopAllPortalDevices: done" );
	}


	void DisposePortalDevice ( int portalID )
    {
        int id = GetPortalDeviceID ( portalID );
        
        CVerbArg ( "DisposePortalDevice: portalID [%i] - id [%i]", portalID, id );
        
        if ( IsInvalidPortalDeviceID ( id ) ) {
            CErrArg ( "DisposePortalDevice: Invalid id [%i]", id );
            return;
        }
        
        int count = 0;
        long value;
        
        PortalDevice * portal = portalDevices + id;
        if ( !portal->device ) {
            CVerbArg ( "DisposePortalDevice: Device is missing for [%i]", id );
            return;
        }
        
        // Close the door
        __sync_sub_and_fetch ( &portal->door, 1 );
        
        // Give back our ticket
        value = __sync_sub_and_fetch ( &portal->ticketCount, 1 );
        
		while ( value != 0 )
		{
			if ( !(count % 50) ) {
				CLog ( "DisposePortalDevice: Waiting for access instances to be released ..." );
			}
			count++;
            
			Sleep ( 100 );
            
            value = ___sync_val_compare_and_swap ( &portal->ticketCount, 0, 0 );
        }
        
		if ( !LockAcquire ( &portalDevicesMutex, "DisposePortalDevice" ) )
			return;
        
		ZeroStruct ( *portal, PortalDevice );

		if ( !LockRelease ( &portalDevicesMutex, "DisposePortalDevice" ) )
			return;
	}
    
#ifndef NDEBUG
    int lastFreeSlot = 0;
#endif
    
    int GetFreePortalSlot ( DeviceBase * device, int portalID )
    {
        CVerb ( "GetFreePortalSlot" );

		if ( !LockAcquire ( &portalDevicesMutex, "GetFreePortalSlot" ) )
			return -1;
        
#ifdef NDEBUG
        int foundID = 0;
#else
        lastFreeSlot++;
        if ( lastFreeSlot > (MAX_PORTAL_INSTANCES/2))
            lastFreeSlot = 0;
        
        int foundID = lastFreeSlot;
#endif
        
        for ( ; foundID < MAX_PORTAL_INSTANCES; foundID++ )
        {
            if ( !portalDevices [foundID].device )
            {
                PortalDevice * portal = portalDevices + foundID;
                
				ZeroStruct ( *portal, PortalDevice );
                
                portalID = portalID | (foundID << 24);
                
                __sync_add_and_fetch ( &portal->ticketCount, 1 );
                
                __sync_add_and_fetch ( &portal->door, 1 );
                
                portal->device = device;
                portal->portalID = portalID;
                break;
            }
        }

		if ( !LockRelease ( &portalDevicesMutex, "GetFreePortalSlot" ) )
			return -1;
        
        if ( foundID >= MAX_PORTAL_INSTANCES )
            foundID = -1;
        return foundID;
    }


	PortalDevice * GetLockedPortalDevice ( int portalID, bool checkDevice )
	{
		CVerbVerbArg ( "GetLockedPortalDevice: id [ 0x%X ]", portalID );

		int id = GetPortalDeviceID ( portalID );

		if ( IsInvalidPortalDeviceID ( id ) ) {
			CVerbArg ( "GetLockedPortalDevice: Invalid id [ 0x%X ]", portalID );
			return 0;
		}

		PortalDevice * portal = portalDevices + id;

		if ( !HoldPortalDevice ( portal ) ) {
			CVerbArg ( "GetLockedPortalDevice: Lock failed id [ 0x%X ]", portalID );
			return 0;
		}

		if ( checkDevice && !portal->device ) {
			CVerbArg ( "GetLockedPortalDevice: Invalid Device [ 0x%X ]", portalID );
			ReleasePortalDevice ( portal );
			return 0;
		}

		return portal;
	}
    
    
    DeviceBase * GetDeviceIncLock ( int portalID )
    {
        PortalDevice * portal = GetLockedPortalDevice ( portalID, true );
        if ( !portal )
            return 0;
        
        DeviceBase * device = portal->device;
        
        IncLockDevice ( device );
        
        ReleasePortalDevice ( portal );
        
        return device;
    }
    

} /// -> namespace environs


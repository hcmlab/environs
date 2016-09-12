/**
 * Mediator client class that implements the mediator functionality on devices based on
 * the base mediator functionality of Mediator.h.
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
//#   define DEBUGVERBList
//#   define DEBUGVERBListener
#endif

#include "Mediator.Client.h"
#include "Environs.Obj.h"
#include "Environs.Utils.h"
#include "Environs.Crypt.h"
#include "Environs.Mobile.h"
#include "Environs.Lib.h"
#include "Core.h"
#include "Device/Devices.h"
#include "Device/Device.Controller.h"
#include "Device/Device.Base.h"
#include "Stunt.Request.h"
#include "Interop/Sock.h"
#include "Tracer.h"
#include <map>
#include <string>
#include <cstring>

#include <sys/types.h>
#include <errno.h>
#include <cerrno>

#ifndef WINDOWS_PHONE
#	include <stdio.h>
#	include <string.h>
#endif
#include <fcntl.h>

#if !defined(WINDOWS_PHONE) && !defined(_WIN32)
#	include <stdlib.h>
#endif

#ifndef NDEBUG
#   ifdef _WIN32
#       include "psapi.h"

#       define MAX_MEMORY_CHECK_BREAK	900000000
#       pragma comment (lib, "Psapi.lib")
#   endif
//#   define MEDIATOR_USE_TCP_NODELAY
//#   define DEBUG_TIMEOUT_ISSUE
//#   define DEBUG_TIMEOUT_ISSUE1
#endif

using namespace environs::lib;

#ifdef NDEBUG
#   ifdef DEBUGVERBListener
#       undef DEBUGVERBListener
#   endif
#       undef CListenerLog
#       define CListenerLog(l,m)
#       undef CListenerLogArg
#       define CListenerLogArg(...)
#else
#   if (!defined(DEBUGVERBListener) && defined(DEBUGVERBVerb))
#       define DEBUGVERBListener
#   endif
#   ifdef DEBUGVERBListener
#       define CListenerLog(l,m)          if (g_Debug>l) { CLog(m); }
#       define CListenerLogArg(l,m,...)   if (g_Debug>l) { CLogArg(m, __VA_ARGS__); }
#   else
#       define CListenerLog(l,m)
#       define CListenerLogArg(l,...)
#   endif
#endif

#define CLASS_NAME	"Mediator.Client. . . . ."

#define MEDIATOR_SECURE_CHANNEL_BUFFER_SIZE 2048


#if !defined(NDEBUG)
//#   define MEDIATOR_RECEIVE_TRACER
//#   define PRINT_DEVICE_LIST
#endif


namespace environs
{
#if defined(PRINT_DEVICE_LIST)
	void PrintDeviceList ( const char * listName, void * deviceList );
#endif

#define PRINT_DEVICE_MAP

#if defined(PRINT_DEVICE_MAP1)
	void PrintDevicesMap ( std::map<const char *, DeviceInstanceItem *, compare_char_key> &devMap, bool useLock );
#else
#define PrintDevicesMap(a,b)
#endif

    unsigned int                    MediatorClient::IPe = 0;
    bool                            MediatorClient::behindNAT = true;

#ifndef ANDROID
    int                             MediatorClient::wifiCurrentRSSI = 0;
    unsigned long long              MediatorClient::wifiCurrentBSSID = 0;
#endif

#ifdef ENABLE_EXT_BIND_IN_STUNT
    unsigned int                    MediatorClient::primaryInterface = 0;
#endif
    
#define GET_DEVICE_SOURCE_STRING(src)	(src == DEVICEINFO_DEVICE_BROADCAST ? "Nearby" : (src == DEVICEINFO_DEVICE_MEDIATOR ? "Mediator" : "Mediator+Nearby"))

#define DEVICE_APPENV_EQUAL(src,p,a)	(!strncmp ( src->areaName, p, sizeof ( src->areaName ) ) && !strncmp ( src->appName, a, sizeof ( src->appName ) ))



#define DEVICE_INFO_ID_EQUAL(a,b)           (a->deviceID == b->deviceID && !strncmp ( a->areaName, b->areaName, sizeof ( a->areaName ) ) && !strncmp ( a->appName, b->appName, sizeof ( a->appName ) ) )

#define DEVICE_INFO_IDD_EQUAL(a,b)          (a->deviceID == b->deviceID)
#define DEVICE_INFO_IDD_GREATEREQUAL(a,b)  (a->deviceID >= b->deviceID)
#define DEVICE_INFO_IDP_COMPVAL(a,b)        strncmp ( a->areaName, b->areaName, sizeof ( a->areaName ) )
#define DEVICE_INFO_IDA_COMPVAL(a,b)        strncmp ( a->appName, b->appName, sizeof ( a->appName ) )

	int DeviceCompareAndTakeOver ( DeviceInstanceNode * listDevice, DeviceInfo * device, bool notify );


    bool IsSameAppEnv ( Instance * env, const char * appName, const char * areaName )
    {
        return ( (!appName || !*appName || !strncmp ( env->appName, appName, MAX_NAMEPROPERTY ) )
                && (!areaName || !*areaName || !strncmp ( env->areaName, areaName, MAX_NAMEPROPERTY ) ) );
    }


	INLINEFUNC bool AssertLength ( const char * str, int length )
	{
		int i = 0;

		while ( i < length ) {
			if ( !str [ i ] ) return true;
			i++;
		}
		return false;
	}


#ifdef USE_MEDIATOR_OPT_KEY_MAPS_COMP

	INLINEFUNC int BuildMapKey ( AppAreaKey * appArea, DeviceInfo * packet )
	{
        //AppAreaKey * buffer = ( AppAreaKey * ) appArea;

		appArea->deviceID = packet->deviceID;

		packet->areaName [ MAX_LENGTH_AREA_NAME - 1 ]   = 0;
		packet->appName [ MAX_LENGTH_APP_NAME - 1 ]     = 0;

		return (snprintf ( appArea->appArea, MAX_NAMEPROPERTY * 2, "%s %s", packet->areaName, packet->appName ) + 4);
	}

	INLINEFUNC int BuildMapKey ( AppAreaKey * appArea, int deviceID, const char * areaName, const char * appName )
	{
        //AppAreaKey * buffer = ( AppAreaKey * ) appArea;

		appArea->deviceID = deviceID;
        
        if ( !AssertLength ( areaName, MAX_NAMEPROPERTY ) || !AssertLength ( appName, MAX_NAMEPROPERTY ) )
            return 0;

		return (snprintf ( appArea->appArea, MAX_NAMEPROPERTY * 2, "%s %s", areaName, appName ) + 4);
	}

#else

	INLINEFUNC int BuildMapKey ( char * buffer, DeviceInfo * packet )
	{
        if ( !buffer || !packet ) {
            _EnvDebugBreak ();
			return 0;
		}

		packet->areaName [ MAX_LENGTH_AREA_NAME - 1 ]   = 0;
		packet->appName [ MAX_LENGTH_APP_NAME - 1 ]     = 0;
        
        return snprintf ( buffer, MAX_DEVICE_INSTANCE_KEY_LENGTH, MEDIATOR_APP_AREA_KEY_FORMAT, packet->deviceID, packet->areaName, packet->appName );
	}

    
	INLINEFUNC int BuildMapKey ( char * buffer, int deviceID, const char * areaName, const char * appName )
	{
        if ( !buffer || !areaName || !appName ) {
            _EnvDebugBreak ();
			return 0;
		}

		if ( !AssertLength ( areaName, MAX_NAMEPROPERTY ) || !AssertLength ( appName, MAX_NAMEPROPERTY ) )
			return 0;
        
        return snprintf ( buffer, MAX_DEVICE_INSTANCE_KEY_LENGTH, MEDIATOR_APP_AREA_KEY_FORMAT, deviceID, areaName, appName );
	}

#endif
    
	const char * getChanelDescriptor ( char c )
	{
		switch ( c )
		{
		case MEDIATOR_STUNT_CHANNEL_MAIN:
			return "Interact Channel";
		case MEDIATOR_STUNT_CHANNEL_BULK:
			return "ComDat   Channel";
		case MEDIATOR_STUNT_CHANNEL_PORTAL:
			return "Portal Channel";
		case MEDIATOR_STUNT_CHANNEL_VERSATILE:
			return "Arbitrary Channel";
		}
		return "Unknown";
	}


#ifndef _WIN32
	void IncreaseThreadPriority ( const char * name )
	{
		int                 policy      = 0;
		pthread_attr_t      attr;
		struct sched_param  param;
		pthread_t           threadId    = pthread_self ();

		pthread_attr_init ( &attr );

		if ( pthread_attr_getschedpolicy ( &attr, &policy ) == 0 ) {
			if ( policy != SCHED_RR ) {
				CVerbArg ( "%s: thread priority is not BATCH, it is %d", name, policy );
			}
			CVerbArg ( "%s: thread priority is %d", name, param.sched_priority );

			param.sched_priority = sched_get_priority_max ( policy );
			if ( pthread_setschedparam ( threadId, policy, &param ) == 0 ) {
				CVerbArg ( "%s: rised thread priority to %d", name, param.sched_priority );
			}
		}
		pthread_attr_destroy ( &attr );
	}
#endif


	DeviceInstanceNode::~DeviceInstanceNode ()
	{
        if ( hEnvirons > 0 && hEnvirons < ENVIRONS_MAX_ENVIRONS_INSTANCES )
        {
            sp ( Instance ) envSP = native.instancesSP [ hEnvirons ] MED_WP;
            if ( envSP )
                envSP->asyncWorker.DisposeSends ( &info );
            
        }

        TraceDeviceInstanceNodeRemove ();

        TraceDeviceInstanceNodesRemove ( this );
	}


	MediatorClient::MediatorClient ()
	{
		CVerb ( "Construct" );

		allocatedClient         = false;
		env						= 0;

		connectFails			= 0;
		registerFails			= 0;

		devices					= 0;
		devicesAvailable		= 0;
        devicesMapAvailable     = 0;

        subscribedToNotifications = true;
        subscribedToMessages    = true;

		secureChannelAuth		= 0;

        registratorState        = ENVIRONS_THREAD_NO_THREAD;

		Zero ( registratorThreadID );

        aliveThreadRestarts         = 0;
        
        sendThreadAlive             = false;
        sendThreadRestarts          = 0;
		sendThreadDisposeContexts	= false;

        deviceMediatorCached		= 0;
		deviceMediatorCacheDirty	= true;
		deviceMediatorCachedCount	= 0;
		deviceAvailableCached		= 0;
		deviceAvailableCacheDirty	= true;
		deviceAvailableCachedCount	= 0;
		deviceChangeIndicator		= 0;

		deviceChangePackets			= 0;
		deviceChangePacketsEnd		= 0;

		deviceMediatorQueryCount	= 0;
		devicesMapUpdates			= 0;

        Zero ( udpStatusMessage );

		udpStatusMessageLen			= 0;
		udpStatusMessageOffset		= 0;
	}


	MediatorClient::~MediatorClient ()
	{
		CVerbs ( 0, "Destruct ..." );

		Stop ( true );

		Mediator::ReleaseMediators ();

        if ( devicesMapAvailable )
            delete devicesMapAvailable;

        if ( allocatedClient ) {
            allocatedClient = false;
            
            //if ( aliveThread.Lock ( "Destruct" ) ) {
                CVerb ( "Destruct: Clearing device change packets..." );

                DeviceChangePacket * packets = deviceChangePackets;
                deviceChangePackets = 0;

                while ( packets )
                {
                    DeviceChangePacket * next = packets->next;
                    free ( packets );
                    packets = next;
                }
                deviceChangePacketsEnd = 0;

            //    aliveThread.Unlock ( "Destruct" );
            //}


			if ( LockAcquireA ( devicesCacheLock, "Destruct" ) ) {

				free_m ( deviceMediatorCached );

				deviceMediatorCachedCount = 0;

				free_m ( deviceAvailableCached );

				deviceAvailableCachedCount	= 0;

				LockReleaseA ( devicesCacheLock, "Destruct" );
			}

			LockDisposeA ( devicesMapLock );
			LockDisposeA ( devicesCacheLock );
            LockDisposeA ( registerLock );
            LockDisposeA ( stuntThreadsLock );
            LockDisposeA ( sendQueueLock );
		}

		CVerb ( "Destruct: Done." );
	}


	void * MediatorClient::MediatorCallbackStarter ( void * arg )
	{
		CVerb ( "MediatorCallback started..." );

		Core::MediatorEvent ( arg );

		return 0;
	}


	bool MediatorClient::Init ( const sp ( Instance ) & obj )
	{
		CVerb ( "Init" );

		envSP = obj;
		if ( !envSP )
			return false;

		env = envSP.get ();

		if ( !Mediator::Init () ) {
			CErr ( "Init: Mediator init failed." );
			return false;
		}

		mediator.connection.instance.thread.autoreset = false;

		if ( !allocatedClient )
		{
			if ( !LockInitA ( registerLock ) )
                return false;
            
            if ( !LockInitA ( stuntThreadsLock ) )
                return false;

            if ( !aliveThread.Init () )
                return false;
            
            if ( !sendThread.Init () )
                return false;

            if ( !LockInitA ( sendQueueLock ) )
                return false;
            
			if ( !LockInitA ( devicesCacheLock ) )
				return false;

			if ( !LockInitA ( devicesMapLock ) )
				return false;

            devicesMapAvailable = new std::map<APPAREATYPE *, DeviceInstanceNode *, compare_char_key> ();
            if ( !devicesMapAvailable )
                return false;

			allocatedClient = true;
		}

		return true;
	}


	bool MediatorClient::InitInstance ()
	{
		CVerb ( "InitInstance" );

		return true;
	}


	void MediatorClient::Stop ( bool wait )
	{
		CVerb ( "Stop" );
        
		// Signal stop status to threads
        isRunning = false;

        StopSendThread ();

		// Wait for each thread to terminate
		// We must close the threads (before Mediator-Base as we provide the thread implementation, e.g. Broadcastthread)
        Mediator::ReleaseThreads ( wait );
        
        StopAliveThread ( wait );
        
        if ( wait ) {
            Mediator::ReleaseMediators ();
            
            JoinThread ( 0, &registratorState, registratorThreadID, 0, "Mediator registrator" );
            
            StuntThreadsDispose ();
        }
        else {
            Mediator::StopMediators ();
        }
        
		DevicesMediatorClear ();

		ReleaseDevices ();
	}


	void MediatorClient::StopAliveThread ( bool wait )
	{
		CVerb ( "StopAliveThread" );

		aliveRunning = false;

        aliveThread.Notify ( "StopAliveThread" );
        
		if ( wait )
			aliveThread.Join ( "StopAliveThread" );
	}


	bool MediatorClient::StartAliveThread ()
	{
		CVerb ( "StartAliveThread" );

		bool success = true;

		if ( !isRunning )
			return false;
        
        aliveRunning = true;
        
        success = ( aliveThread.Run ( pthread_make_routine ( &AliveThreadStarter ), ( void * ) this, "StartAliveThread" ) != 0 );
        if ( !success )
            success = aliveThread.isRunning ();

		return success;
	}


	void MediatorClient::ReleaseDevices ()
	{
		CVerb ( "ReleaseDevices" );

		if ( !LockAcquireA ( devicesLock, "ReleaseDevices" ) )
			return;

		DeviceInstanceNode * device = devices;

		while ( device ) {
			DeviceInstanceNode * toDispose = device;
			device = device->next;
			CVerbArg ( "[0x%X].ReleaseDevices: Disposing device", toDispose->info.deviceID );

            if ( toDispose->info.broadcastFound == DEVICEINFO_DEVICE_BROADCAST_AND_MEDIATOR ) {
                toDispose->info.broadcastFound = DEVICEINFO_DEVICE_MEDIATOR;
            }
            toDispose->baseSP = 0;
		}

		devices             = 0;
		devicesAvailable    = 0;

		LockReleaseVA ( devicesLock, "ReleaseDevices" );
	}


	unsigned int MediatorClient::GetLocalIPe ()
	{
		CVerb ( "GetLocalIPe" );

		return IPe;
	}


	sp ( ApplicationDevices ) MediatorClient::GetDeviceList ( char * areaName, char * appName, pthread_mutex_t ** mutex, int ** pDevicesAvailable, DeviceInstanceNode ** &list  )
	{
		*mutex = &devicesLock;
        list = &devices;

		if ( pDevicesAvailable )
			*pDevicesAvailable = &devicesAvailable;
		return 0;
	}


	void MediatorClient::UpdateDeviceInstance ( const sp ( DeviceInstanceNode ) & device, bool added, bool changed )
	{
		if ( LockAcquireA ( devicesMapLock, "UpdateDeviceInstance" ) ) {
			if ( added ) {
				DeviceAdd ( device.get (), 0 );
			}
			else if ( changed )
				DeviceChange ( device.get (), 0 );
			LockReleaseVA ( devicesMapLock, "UpdateDeviceInstance" );
		}
	}


	void MediatorClient::RemoveDevice ( DeviceInstanceNode * device, bool useLock, bool forceUnlock )
	{
		CVerb ( "RemoveDevice" );

		if ( !device )
			return;

#ifdef USE_MEDIATOR_OPT_KEY_MAPS_COMP
        AppAreaKey deviceKey;

        AppAreaKey * key = &deviceKey;
#else
		char key [MAX_DEVICE_INSTANCE_KEY_LENGTH];
#endif
		CVerb ( "RemoveDevice: Valid device" );

		if ( useLock && !LockAcquireA ( devicesLock, "RemoveDevice" ) )
			return;

		if ( device == devices ) {
			if ( devices->next ) {
				CVerbArg ( "RemoveDevice: [ 0x%X ] Re-locating client to root of list", devices->next->info.deviceID );

				devices = devices->next;
				devices->prev = 0;
				devicesAvailable--;
			}
			else {
				CVerb ( "RemoveDevice: [ 0x%X ] Clearing list. No more devices." );

				devices = 0;
				devicesAvailable = 0;
			}
		}
		else {
			if ( !device->prev ) {
				CErrArg ( "RemoveDevice: Serious inconsistency error!!! Failed to lookup device list. Missing previous device for ID [ 0x%X ]", device->info.deviceID );

				/// The best we can do is to assume that we are the root node of the list
				if ( devices->next ) {
					CVerbArg ( "RemoveDevice: [ 0x%X ] Re-locating client to root of list", devices->next->info.deviceID );

					devices = devices->next;
					devices->prev = 0;
					devicesAvailable--;
				}
				else {
					devices = 0;
					devicesAvailable = 0;
				}
			}
			else {
				if ( device->next ) {
					CVerbArg ( "RemoveDevice: [ 0x%X ] Relinking next client to previous client [ 0x%X ]", device->next->info.deviceID, device->prev->info.deviceID );
					device->prev->next = device->next;
					device->next->prev = device->prev;
				}
				else {
					CVerbArg ( "RemoveDevice: [ 0x%X ] Finish list since client was the last one", device->info.deviceID );
					device->prev->next = 0;
				}

				devicesAvailable--;

				CVerbArg ( "RemoveDevice: [ 0x%X ] Disposing device", device->info.deviceID );
			}
		}

#ifdef USE_MEDIATOR_OPT_KEY_MAPS_COMP
        memcpy ( key, &device->key, sizeof(AppAreaKey) );
#else
        memcpy ( key, device->key, sizeof(key) );
#endif
		CVerbArg ( "RemoveDevice: [ 0x%X ] Disposing device", device->info.deviceID );


        sp ( DeviceController ) deviceSP = device->deviceSP.lock ();
        if ( deviceSP ) {
            DeviceBase * pingDevice = (DeviceBase *) deviceSP.get ();
            
            if ( IsValidFD ( pingDevice->comDatSocket ) )
            {
                SocketTimeout ( pingDevice->comDatSocket, -1, 1 );
                
                pingDevice->SendPing ();
                
                SocketTimeout ( pingDevice->comDatSocket, -1, 0 );
            }

            deviceSP.reset ();
        }

        device->baseSP = 0;

		if ( useLock || forceUnlock ) {
			LockReleaseVA ( devicesLock, "RemoveDevice" );
		}

		if ( LockAcquireA ( devicesMapLock, "RemoveDevice" ) ) {

			DeviceRemove ( key, false );

			LockReleaseVA ( devicesMapLock, "RemoveDevice" );
		}
	}


	void MediatorClient::RemoveDevice ( unsigned int ip, char * msg )
	{
		CVerbVerb ( "RemoveDevice from broadcast" );

		if ( !isRunning || !msg )
			return;

		int deviceID;
		//char * deviceName = 0;
		char * areaName = 0;
		char * appName = 0;

		// Get the id at first (>0)
		int * pInt = ( int * ) ( msg + MEDIATOR_BROADCAST_DEVICEID_START );
		deviceID = *pInt;
		if ( !deviceID )
			return;

		// Get the areaName, appname, etc..

		unsigned char * sizesDst = ( unsigned char * ) ( msg + MEDIATOR_BROADCAST_DESC_START );

		// Get the areaName, appname, etc..
		appName  = ( char * ) ( sizesDst + 2 );
		areaName = appName + *sizesDst;

		if ( !*appName || *sizesDst >= MAX_NAMEPROPERTY || !*areaName )
			return;

		sizesDst++;

		if ( *sizesDst >= MAX_NAMEPROPERTY )
			return;

		areaName [ *sizesDst ] = 0;

		if ( !LockAcquireA ( devicesLock, "RemoveDevice BC" ) )
			return;

		CVerbVerbID ( "RemoveDevice BC:" );

		sp ( DeviceInstanceNode ) deviceSP = 0;

		DeviceInstanceNode * device = devices;

		if ( device ) {
			while ( device ) {
				if ( device->info.deviceID == deviceID ) {
					if ( !strncmp ( device->info.areaName, areaName, sizeof ( device->info.areaName ) ) &&
						!strncmp ( device->info.appName, appName, sizeof ( device->info.appName ) ) )
					{
						deviceSP = device->baseSP;
#ifndef NDEBUG
						if ( !deviceSP ) {
							CWarn ( "RemoveDevice BC: Device found in list. However, SP has been released!" );
						}
#endif
						break;
					}
				}

				device = device->next;
			}
		}

		LockReleaseVA ( devicesLock, "RemoveDevice BC" );

		if ( deviceSP )
			RemoveDevice ( deviceSP.get () );
    }
    
    
#ifdef ENABLE_EXT_BIND_IN_STUNT
    void MediatorClient::MatchExtIPWithInterfaces ()
    {
        NetPack * net = &localNets;
        
        while ( net ) {
            if ( IPe == net->ip ) {
                primaryInterface = IPe;
                break;
            }
            net = net->next;
        }
    }

    
    //
    // We need to do a bind if multiple network interfaces (virtual NATs) are available or for multi-homed hosts
    //
    void MediatorClient::BindSocketToLocalInterface ( int sock )
    {
        if ( !primaryInterface )
            return;
        
        CVerb ( "BindSocketToLocalInterface" );
        
        struct sockaddr_in	addr;
        Zero ( addr );
        
        addr.sin_family			= PF_INET;
        addr.sin_addr.s_addr	= primaryInterface;
        
        int rc = ::bind ( sock, ( const sockaddr * ) &addr, sizeof ( struct sockaddr ) );
        if ( rc ) {
            CWarnArg ( "BindSocketToLocalInterface: Failed to bind socket to local interface [ %s ] ...", inet_ntoa ( ( ( struct sockaddr_in * ) &addr )->sin_addr ) );
            
            LogSocketErrorF ( "MediatorClient.BindSocketToLocalInterface" );
        }
#ifndef NDEBUG
        else {
            CVerbsArg ( 4, "BindSocketToLocalInterface: Bound to [ %s : %i ]", inet_ntoa ( ( ( struct sockaddr_in * ) &addr )->sin_addr ), ntohs ( addr.sin_port ) );
        }
#endif
    }
#endif

	void MediatorClient::BuildBroadcastMessage ( bool withStatus )
	{
		CVerb ( "BuildBroadcastMessage" );

		// Format Environs.Device DeviceID

		// 12; 4; 4; 2; 2; => 24 byte; max. 50 byte for areaName
		strlcpy ( broadcastMessage + 4, MEDIATOR_BROADCAST_DEVICEINFO, 24 );

        broadcastMessage [ 14 ] = MEDIATOR_PROTOCOL_VERSION;
        broadcastMessage [ 15 ] = ( char ) env->mediatorFilterLevel;

		int * pInt = ( int * ) ( broadcastMessage + MEDIATOR_BROADCAST_DEVICEID_ABS_START );
		*pInt = env->deviceID;

		unsigned int * pUInt = ( unsigned int * ) pInt;
		pUInt++;
		*pUInt = localNets.ip;

		unsigned short * pShort = ( unsigned short * ) ( broadcastMessage + MEDIATOR_BROADCAST_PORTS_ABS_START );
		*pShort = env->tcpPort;
		pShort++;
		*pShort = env->udpPort;

		pUInt += 2;

		*pUInt = ( unsigned int ) native.platform;
		pUInt++;

		*pUInt = 0;

		unsigned char * descStart = ( unsigned char * ) ( broadcastMessage + MEDIATOR_BROADCAST_DESC_ABS_START );

		if ( !BuildAppAreaField ( descStart, env->appName, env->areaName, false ) )
			return;

		broadcastMessageLen = MEDIATOR_BROADCAST_DESC_ABS_START + ( unsigned int ) ( *descStart + *( descStart + 1 ) ) + 2;

		broadcastMessage [ broadcastMessageLen ] = 0;

		// Append ext for mediator registration
		descStart = ( unsigned char * ) ( broadcastMessage + broadcastMessageLen );

		if ( !BuildAppAreaField ( descStart, native.deviceName, native.deviceUID, true ) )
			return;

		broadcastMessageLen += ( unsigned int ) ( *descStart + 2 );

		broadcastMessageLenExt = broadcastMessageLen + ( unsigned int ) *( descStart + 1 );

		broadcastMessage [ broadcastMessageLenExt ] = 0;

		pInt = reinterpret_cast<int *>( broadcastMessage );
		*pInt = randBroadcastToken;

		if ( withStatus )
			SyncDeviceUpdateFlags ( 0, false );
    }


    void MediatorClient::SyncDeviceUpdateFlags ( DeviceInstanceNode * device, bool set )
    {
        CVerb ( "SyncDeviceUpdateFlags" );

        int             *   pInt;

        if ( !device )
        {
            // 4 - 12 - 4 - 4 -

            // Size +4 / 4

            strlcpy ( udpStatusMessage + 4, MEDIATOR_BROADCAST_DEVICE_UPDATE, 24 ); // + 12 / 16 (10)

            pInt = ( int * ) ( udpStatusMessage + MEDIATOR_BROADCAST_STATUS_SRCDEVICEID_ABS_START );
            *pInt = env->deviceID; // +4 / 20

			pInt++;
            *pInt = 0; // +4 / 24

            // +16 / 40

			unsigned char * descStart = ( unsigned char * ) ( udpStatusMessage + MEDIATOR_BROADCAST_STATUS_DESC_ABS_START );

			if ( !BuildAppAreaField ( descStart, env->appName, env->areaName, false ) )
				return;

			udpStatusMessageOffset = MEDIATOR_BROADCAST_STATUS_DESC_ABS_START + ( unsigned int ) ( *descStart + *( descStart + 1 ) ) + 2;

			udpStatusMessage [ udpStatusMessageOffset ] = 0;
            return;
        }

		int dataSize;

		char * tmpMsg = ( char * ) malloc ( udpStatusMessageOffset + ( MAX_NAMEPROPERTY * 2 ) + 4 );
		if ( !tmpMsg )
			return;

		memcpy ( tmpMsg, udpStatusMessage, udpStatusMessageOffset );
		
		pInt = reinterpret_cast<int *>( tmpMsg + MEDIATOR_BROADCAST_STATUS_DEVICEID_ABS_START );
		*pInt = device->info.deviceID;

		pInt++;
		*pInt = ( int ) set;

		pInt++;
		*pInt = ( int ) ( device->info.flags & ( unsigned short ) ( DeviceFlagsInternal::NotifyMask | DeviceFlagsInternal::CPNotifyMask ) );

		unsigned char * descStart = ( unsigned char * ) ( tmpMsg + udpStatusMessageOffset );

		if ( BuildAppAreaField ( descStart, device->info.appName, device->info.areaName, false ) )
		{
			CVerbArg ( "SyncDeviceUpdateFlags: To [ 0x%X : %s / %s ]", device->info.deviceID, device->info.appName, device->info.areaName );
			CVerbsArg ( 2, "SyncDeviceUpdateFlags: Flags [ 0x%X ]", *pInt );

			dataSize = udpStatusMessageOffset + ( unsigned int ) ( *descStart + *( descStart + 1 ) ) + 2;

			pInt = reinterpret_cast<int *>( tmpMsg );
			*pInt = randBroadcastToken;
            
            if ( SendData ( device, tmpMsg, dataSize ) )
                return;
		}
        
        free ( tmpMsg );
    }


    void MediatorClient::HandleDeviceUpdateMessage ( char * msg )
    {
        int * pInt = reinterpret_cast<int *>( msg + MEDIATOR_BROADCAST_STATUS_SRCDEVICEID_ABS_START );

        int deviceIDSrc = *pInt; pInt++;
        int deviceIDDst = *pInt;

        if ( deviceIDDst != env->deviceID )
            return;

		unsigned char * sizesDst = ( unsigned char * ) ( msg + MEDIATOR_BROADCAST_STATUS_DESC_ABS_START );

        size_t appLen  = *sizesDst;
        size_t areaLen = *(sizesDst + 1);

        // Get the areaName, appname, etc..
		char * appName  = ( char * ) ( sizesDst + 2 );
		char * areaName = appName + *sizesDst;

		if ( !*appName || !appLen || appLen >= MAX_NAMEPROPERTY || !*areaName || !areaLen || areaLen >= MAX_NAMEPROPERTY )
			return;

        appName [ appLen - 1 ]   = 0;
        areaName [ areaLen - 1 ] = 0;

        if ( env->mediatorFilterLevel >= MediatorFilter::Area )
        {
            if ( strncmp ( env->areaName, areaName, areaLen ) )
                return;

            if ( env->mediatorFilterLevel >= MediatorFilter::AreaAndApp )
            {
                if ( strncmp ( env->appName, appName, appLen ) )
                    return;
            }
        }

        sp ( DeviceInstanceNode ) deviceSP = GetDeviceSP ( deviceIDSrc, areaName, appName );
        if ( !deviceSP ) {
            CVerbArg ( "HandleDeviceUpdateMessage: Device not found [ 0x%X : %s / %s ]", deviceIDSrc, appName, areaName  );
            return;
        }

		DeviceInstanceNode * device = deviceSP.get ();

        pInt = reinterpret_cast<int *>( msg + MEDIATOR_BROADCAST_STATUS_CLEAR_SET_ABS_START );

        bool set     = (*pInt != 0); pInt++;
        int  cpflags = ( ( *pInt << 8 ) & DeviceFlagsInternal::CPNotifyMask );
        int  flags   = ( ( *pInt >> 8 ) & DeviceFlagsInternal::NotifyMask );

        bool doBackPropagate = false;
        bool doNotify        = false;

        unsigned short deviceFlags = device->info.flags;

        CVerbArg ( "HandleDeviceUpdateMessage: For [ 0x%X : %s / %s ]", deviceIDSrc, appName, areaName  );
        CVerbsArg ( 2, "HandleDeviceUpdateMessage: CPFlags [ 0x%X ] Flags [ 0x%X ] DeviceFlags [ 0x%X ]", cpflags, flags, deviceFlags );

        if ( set ) {
            if ( ( deviceFlags & DeviceFlagsInternal::CPNotifyMask ) != cpflags )
            {
                doNotify = true;
                device->info.flags |= cpflags;
            }

			if ( ( deviceFlags & DeviceFlagsInternal::CPPlatformReady ) != ( cpflags & DeviceFlagsInternal::CPPlatformReady ) )
				doBackPropagate = true;
            else if ( flags && ( deviceFlags & DeviceFlagsInternal::NotifyMask ) != flags )
            {
                doBackPropagate = true;
            }
        }

        if ( doNotify ) {
            CVerbsArg ( 2, "HandleDeviceUpdateMessage: Flags [ 0x%X ]", cpflags );

            if ( deviceFlags & DeviceFlagsInternal::PlatformReady )
                API::onEnvironsNotifierContext1 ( env, device->info.objID, Notify::Environs::DeviceFlagsUpdate, cpflags, 0, set );
#ifndef NDEBUG
            else {
                CVerb ( "HandleDeviceUpdateMessage: Skipping delivery of device status due to PLATFORM NOT READY." );
            }
#endif
        }

        if ( doBackPropagate ) {
            CVerbsArg ( 2, "HandleDeviceUpdateMessage: Backpropagate flags [ 0x%X ]", deviceFlags );

            // Note: We got this sync update through broadcasts,
            //       but the route backwards through broadcasts may not necessarily work
            //       So, we try both: Mediator server and broadcasts

            SetDeviceFlags ( device, true );

            CVerb ( "HandleDeviceUpdateMessage: Do Backpropagate due to PLATFORM BECAME READY." );
        }
    }

    
    bool MediatorClient::SendData ( DeviceInstanceNode * device, char * data, int dataSize )
    {
        CVerbVerb ( "SendData" );
        
        if ( !device || IsInvalidFD ( broadcastSocket ) || !broadcastRunning )
            return false;
        
        if ( native.networkStatus <= NetworkConnection::NoInternet )
            return false;
        
        CVerbVerbArg ( "SendData: Device status message [ %s ] ( %d )...", data + 4, dataSize );
        
        return SendBufferOrEnqueueBC ( false, data, dataSize, device->info.ip, device->info.udpPort );
    }


	void MediatorClient::OnStarted ()
	{
		strlcpy ( broadcastMessage + 4, MEDIATOR_BROADCAST_GREET, sizeof ( MEDIATOR_BROADCAST_GREET ) );

		connectFails    = 0;
		registerFails   = 0;

        StartAliveThread ();
        
        StartSendThread ();

		DevicesHasChanged ( MEDIATOR_DEVICE_RELOAD );
    }
    
    
    bool MediatorClient::StartSendThread ()
    {
        CVerb ( "StartSendThread" );
        
        bool success = true;
        
        if ( !isRunning )
            return false;
        
        sendThreadAlive = true;
        
        success = (sendThread.Run ( pthread_make_routine ( &SendThreadStarter ), ( void * ) this, "StartSendThread" ) != 0);
        
        return success;
    }
    
    
    void MediatorClient::StopSendThread ()
    {
        CVerb ( "StopSendThread" );
        
        sendThreadAlive = false;

        sendThread.Notify ( "StopSendThread" );
        
        sendThread.Join ( "StopSendThread" );
    }
    
    
    void * MediatorClient::SendThreadStarter ( void * arg )
    {
        MediatorClient * mediator = ( MediatorClient * ) arg;
        if ( !mediator ) {
            CErr ( "SendThreadStarter: called with (NULL) argument!" );
            return 0;
        }
        
        mediator->SendThread ();
        
        mediator->sendThread.Detach ( "SendThreadStarter" );
        
        if ( mediator->isRunning && mediator->sendThreadAlive && mediator->env->environsState >= environs::Status::Starting )
        {
            if ( mediator->sendThreadRestarts <= 3 )
            {
                // Let's restart the thread. Something has shut us down unexpectedly
                // while all indicators expect us to be running for a working mediator layer
                mediator->sendThreadRestarts++;
                
                if ( mediator->StartSendThread ( ) )
                {
                    CWarn ( "SendThreadStarter: Restarted thread." );
                    return 0;
                }
            }
            
            // Restart has failed 3 times ... or restart has failed at all
            // We're giving up here and shut down alive thread
            
            CErr ( "SendThreadStarter: Failed to restart thread !!!" );
        }
        return 0;
    }
    
    
    void MediatorClient::SendContextsCompress ( lib::QueueVector * queue )
    {
        SendContext * ctx = ( SendContext * ) queue->first ();
        if ( !ctx || ctx->sendBuffer )
            return;
        
        // Look how many contexts we can pack into one send to encrypt
        // Max size is half of client buffer of mediator listeners
        int				contexts	= 0, contextsDelete;
        int				remainSize	= MEDIATOR_CLIENT_MAX_BUFFER_SIZE_MAX;
        
        SendContext **	items		= ( SendContext ** ) queue->items;
        int				cur			= ( int ) queue->next;
        int				capacity	= ( int ) queue->capacity;
        unsigned int	lastSize	= 0;
        
        while ( contexts < ( int ) queue->size_ )
        {
            SendContext * item = items [ cur ];
            
            if ( item->sendBuffer || !item->buffer ) {
                cur--;
                break;
            }
            
            lastSize = item->size;
            remainSize -= lastSize;
            contexts++;
            
            if ( remainSize <= 1024 )
                break;
            
            cur++;
            if ( cur >= capacity )
                cur = 0;
        }
        
        if ( remainSize < 0 ) {
            contexts--;
            remainSize -= lastSize;
        }
        
        if ( contexts <= 1 )
            return;
        
        int requiredSize = ( MEDIATOR_CLIENT_MAX_BUFFER_SIZE_MAX - remainSize ) + 128;
        
        char * tmp = ( char * ) malloc ( requiredSize );
        if ( tmp )
        {
            // Merge previous contexts to current context
            char *			curPtr		= tmp;
            unsigned int	curSize		= 0;
            
            contextsDelete = contexts - 1;
            
            for ( int i = 0; i < contexts; ++i )
            {
                if ( i < contextsDelete ) {
                    // Removed previous contexts
                    ctx = ( SendContext * ) queue->pop ();
                }
                else {
                    ctx = ( SendContext * ) queue->first ();
                }
                
                if ( ctx->buffer )
                {
                    unsigned int itemSize = ctx->size;
                    
                    unsigned int headerSize = *( reinterpret_cast<unsigned int *>( ctx->buffer ) );
                    if ( headerSize != itemSize ) {
                        * ( reinterpret_cast<unsigned int *>( ctx->buffer ) ) = itemSize;
                    }
                    
                    memcpy ( curPtr, ctx->buffer, itemSize );
                    
                    curSize += itemSize;
                    curPtr += itemSize;
                }
                
                if ( i < contextsDelete ) // Skip deletion of the last context. We reuse it.
                    delete ctx;
            }
            
            ctx = ( SendContext * ) queue->first ();
            
            if ( ctx->sendBuffer && ctx->freeSendBuffer ) {
                free ( ctx->sendBuffer );
                ctx->sendBuffer = 0; ctx->freeSendBuffer = false;
            }
            
            if ( ctx->buffer )
                free ( ctx->buffer );
            
            ctx->buffer = tmp;
            ctx->size   = curSize;
        }
    }
    
    
    void  MediatorClient::SendThread ()
    {
        CVerb ( "SendThread: started ..." );
        
        pthread_setname_current_envthread ( "MediatorClient::SendThread" );
        
        MediatorInstance    *   inst    = 0;
        ThreadInstance      *   client  = 0;
        lib::QueueVector    *   queue;
        SendContext         *   ctx;
        
        bool                    checkSocket = false;
        bool                    checkBC     = false;
        bool                    pollMediator = true;
        bool                    pollBC      = true;

        int                     iMediator   = 0;
        int                     iBC         = 0;
        
		struct pollfd           desc [ 2 ];
		desc [ 0 ].events = POLLOUT | POLLERRMASK;
        desc [ 1 ].events = POLLOUT | POLLERRMASK;

		int rc;
        int count;

        while ( sendThreadAlive && env->environsState >= environs::Status::Starting )
        {
            if ( ( pollMediator || pollBC ) && client )
            {
            Retry:
                count = 0;

                if ( pollMediator && IsValidFD ( client->socket ) ) {
                    desc[0].fd          = client->socket;
                    desc[0].revents     = 0;
                    iMediator           = 0;
                    count++;
                }
                else iMediator = -1;
                
                if ( pollBC && IsValidFD ( broadcastSocket ) ) {
                    desc[count].fd          = broadcastSocket;
                    desc[count].revents     = 0;
                    iBC                     = count;
                    count++;
                }
                else iBC = -1;

                if ( !count ) {
                    pollMediator = pollBC = false;
                    continue;
                }

				rc = poll ( desc, count, -1 );

				if ( !isRunning || !sendThreadAlive )
					break;

				if ( rc == -1 || ( desc [ 0 ].revents & POLLERRMASK ) ) {
					CVerb ( "SendThread: Socket has been closed" );
					LogSocketErrorF ( "MediatorClient.SendThread" );
                    
                    pollMediator = pollBC = false;
					continue;
				}

				if ( rc == 0 )
					goto Retry;
                pollMediator = pollBC = false;
            }
            else {
                checkSocket = false; checkBC = false;
                
                if ( !client || ( client->sendQueuePrior.size_ <= 0 && client->sendQueue.size_ <= 0 && sendQueue.size_ <= 0 ) )
                {
                    sendThread.WaitOne ( "SendThread", -1, true, true );
                    
                    sendThread.ResetSync ( "SendThread", false );
                    
                    if ( !sendThread.UnlockCond ( "SendThread" ) )
                        break;
                }
            }
            
			if ( sendThreadDisposeContexts ) {
				DisposeSendContexts ( client );
				sendThreadDisposeContexts = false;
            }
            
            bool skipNextQueue          = false;
            bool changeToPriorityQueue  = false;

            if ( !inst ) {
                inst = GetAvailableMediator ();
                if ( inst ) {
                    client = &inst->connection.instance;
                }
                
                if ( !client )
                    goto DoBC;
            }
            
        ContinueWithPriorityQueue:
            while ( client->sendQueuePrior.size_ > 0 )
            {
                if ( checkSocket ) {
                    if ( iMediator >= 0 && ( desc [ iMediator ].revents & POLLOUT ) == 0 ) {
                        pollMediator = true; skipNextQueue = true; break;
                    }
                    checkSocket = false;
                }
                
                bool locked = false;
                
                if ( client->sendQueue.size_ > 0 )
                {
                    if ( !LockAcquireA ( client->sendQueueLock, "SendThread" ) )
                        break;
                    locked = true;
                    
                    ctx = ( SendContext * ) client->sendQueue.first ();
                    
                    if ( ctx->sendCurrent ) {
                        changeToPriorityQueue = true;
                        LockReleaseA ( client->sendQueueLock, "SendThread" );
                        break;
                    }
                }
                
                queue = &client->sendQueuePrior;
                
                if ( !locked && !LockAcquireA ( client->sendQueueLock, "SendThread" ) )
                    break;
                
                ctx = ( SendContext * ) queue->first ();
                
                LockReleaseA ( client->sendQueueLock, "SendThread" );
                
                if ( !ctx )
                    break;
                
                if ( !SendBuffer ( client, ctx ) ) {
                    pollMediator = true; skipNextQueue = true; checkSocket = true;
                    break;
                }
                
                if ( !LockAcquireA ( client->sendQueueLock, "SendThread" ) )
                    break;
                
                ctx = ( SendContext * ) queue->pop ();
                
                LockReleaseA ( client->sendQueueLock, "SendThread" );
                
                if ( ctx )
                    delete ctx;
            }
            
            while ( !skipNextQueue && client->sendQueue.size_ > 0 )
            {
                if ( checkSocket ) {
                    if ( iMediator >= 0 && ( desc [ iMediator ].revents & POLLOUT ) == 0 ) {
                        pollMediator = true; break;
                    }
                    checkSocket = false;
                }
                queue = &client->sendQueue;
                
                LockAcquireA ( client->sendQueueLock, "SendThread" );
                
                if ( !changeToPriorityQueue && queue->size_ > 2 ) {
                    
                    SendContextsCompress ( queue );
                }

                ctx = ( SendContext * ) queue->first ();
                
                LockReleaseA ( client->sendQueueLock, "SendThread" );
                
                if ( !ctx )
                    break;
                
                if ( !SendBuffer ( client, ctx ) ) {
                    pollMediator = true; checkSocket = true;
                    break;
                }
                
                if ( !LockAcquireA ( client->sendQueueLock, "SendThread" ) )
                    break;
                
                ctx = ( SendContext * ) queue->pop ();
                
                LockReleaseA ( client->sendQueueLock, "SendThread" );
                
                if ( ctx )
                    delete ctx;
                
                if ( changeToPriorityQueue || client->sendQueuePrior.size_ > 0 ) {
                    changeToPriorityQueue = false;
                    goto ContinueWithPriorityQueue;
                }
            }
            
        DoBC:
            while ( sendQueue.size_ > 0 )
            {
                if ( checkBC ) {
                    if ( iBC >= 0 && ( desc [ iBC ].revents & POLLOUT ) == 0 ) {
                        pollBC = true; break;
                    }
                    checkBC = false;
                }
                
                if ( !LockAcquireA ( sendQueueLock, "SendThread" ) )
                    break;
                
                ctx = ( SendContext * ) sendQueue.first ();
                
                LockReleaseA ( sendQueueLock, "SendThread" );
                
                if ( !ctx )
                    break;
                
                if ( !SendBufferBC ( ctx ) ) {
                    pollBC = true; checkBC = true;
                    break;
                }
                
                if ( !LockAcquireA ( sendQueueLock, "SendThread" ) )
                    break;
                
                ctx = ( SendContext * ) sendQueue.pop ();
                
                LockReleaseA ( sendQueueLock, "SendThread" );
                
                if ( ctx )
                    delete ctx;
            }
            
            if ( !pollMediator ) {
                pollMediator = ( client->sendQueuePrior.size_ > 0 || client->sendQueue.size_ > 0 );
            }
            
            if ( !pollBC ) {
                pollBC = ( sendQueue.size_ > 0 );
            }
        }
        
        DisposeSendContexts ( client );

		DisposeSendContextsBC ();
        
        CVerb ( "SendThread: bye bye ..." );
    }
    
    
    int MediatorClient::SendBuffer ( ThreadInstance * client, SendContext * ctx )
    {
        CVerbVerbArg ( "SendBuffer [ %i ]", client->socket );
        
        int             rc          = 0;
        char *          toSend      = 0;
        unsigned int    toSendLen   = 0;
        
        CVerbArg ( "SendBuffer [ %i ]", client->socket );
        
        if ( IsInvalidFD ( client->socket ) )
            return -1;
        
        if ( !ctx->sendBuffer )
        {
            char * sendLoad = ctx->buffer;
            
            if ( !sendLoad )
                return 1;
            
            if ( client->encrypt )
            {
                char * cipher = 0;
                toSendLen = ctx->size;
                
                if ( !AESEncrypt ( &client->aes, sendLoad, &toSendLen, &cipher ) || !cipher ) {
                    CErrArg ( "SendBuffer [ %i ]: Failed to encrypt AES message.", client->socket );
                    
                    // Failed to encrypt. For now, we just skip this send context and go on to the next one
                    return 1;
                }
                
                ctx->freeSendBuffer = true;
                ctx->sendBuffer		= cipher;
                ctx->sendSize		= toSendLen;
            }
            else {
                ctx->sendBuffer		= sendLoad;
                ctx->sendSize		= ctx->size;
            }
        }
        
        toSend      = ctx->sendBuffer;
        toSendLen   = ctx->sendSize;
        
        CVerbArg ( "SendBuffer [ %i ]: [ %i ] bytes", client->socket, toSendLen );
        
        if ( IsValidFD ( (int) client->socket ) )
        {
#if defined(DEBUG_SEQNR)
            if ( ctx->seqNr ) {
                CLogArg ( "SendBuffer: seqNr [ %i ] size [ %u : %u ]", ctx->seqNr, ctx->sendSize, ctx->sendCurrent );
            }
#endif
            rc = ( int ) send ( ( int ) client->socket, toSend + ctx->sendCurrent, toSendLen - ctx->sendCurrent, MSG_NOSIGNAL );
            if ( rc < 0 )
            {
                SOCKET_Check_Val ( check );
                
                if ( SOCKET_Check_Retry ( check ) ) {
                    return 0;
                }
            }
            
            else if ( rc >= 0 )
            {
                ctx->sendCurrent += rc;
                if ( ctx->sendCurrent == toSendLen ) {
                    rc = 1;
                }
                
                else rc = 0;
            }
        }
        
        CVerbArg ( "SendBuffer [ %i ]: [ %i ] bytes. Done [ %i ].", client->socket, toSendLen, rc );
        return rc;
    }
    
    
#ifdef USE_MEDIATOR_TRYSEND_BEFORE_ENQUEUE
    bool MediatorClient::SendBufferOrEnqueue ( ThreadInstance * client, void * msg, unsigned int size )
    {
        CVerbVerbArg ( "SendBufferOrEnqueue [ %i ]", client->socket );
        
        if ( client->sendQueue.size_ > 0 ) {
            return PushSend ( client, msg, size );
        }
        
        bool			success		= false;
        int             rc          = -1;
        char *          cipher      = 0;
        char *			toSend		= ( char * ) msg;
        unsigned int    toSendLen   = size;
        unsigned int    bytesSent   = 0;
        
        CVerbArg ( "SendBufferOrEnqueue [ %i ]", client->socket );
        
        if ( IsInvalidFD ( ( int ) client->socket ) )
            return false;
        
        if ( client->encrypt ) {
            if ( !AESEncrypt ( &client->aes, ( char * ) msg, &toSendLen, &cipher ) || !cipher ) {
                CErrArg ( "SendBufferOrEnqueue [ %i ]: Failed to encrypt AES message.", client->socket );
                return false;
            }
            toSend = cipher;
        }
        
        CVerbArg ( "SendBufferOrEnqueue [ %i ]: [ %i ] bytes", client->socket, toSendLen );
        
        if ( isRunning && IsValidFD ( client->socket ) )
        {
            bool push = false;
            
            rc = ( int ) send ( ( int ) client->socket, toSend, toSendLen, MSG_NOSIGNAL );
            if ( rc >= 0 )
            {
                bytesSent += rc;
                if ( bytesSent == toSendLen )
                {
                    success = true;
                }
                else push = true;
            }
            else {
                SOCKET_Check_Val ( check );
                
                if ( SOCKET_Check_Retry ( check ) )
                    push = true;
            }
            
            if ( push && PushSend ( client, toSend, toSendLen, bytesSent, cipher == 0 ) )
            {
                cipher = 0;
                success = true;
            }
        }
        
        CVerbArg ( "SendBufferOrEnqueue [ %i ]: [ %i ] bytes. Done.", client->socket, toSendLen );
        
        free_n ( cipher );
        
        return success;
    }
#endif
    
    
    bool MediatorClient::PushSend ( ThreadInstance * client, void * buffer, unsigned int size, unsigned int seqNr )
    {
        bool success = false;
        
#if defined(DEBUG_SEQNR)
        if ( seqNr ) {
            CLogArg ( "PushSend: seqNr [ %i ]", seqNr );
        }
#endif
        SendContext * ctx = new SendContext ();
        while ( ctx )
        {
            ctx->buffer = ( char * ) malloc ( size );
            if ( !ctx->buffer )
                break;
            
            memcpy ( ctx->buffer, buffer, size );
            
            ctx->size   = size;
            
            if ( !LockAcquireA ( client->sendQueueLock, "PushSend" ) )
                break;
            
            if ( IsValidFD ( ( int ) client->socket ) ) {
                if ( seqNr )
                {
                    ctx->seqNr  = seqNr;
                    
                    success = client->sendQueuePrior.push ( ctx );
                }
                else
                    success = client->sendQueue.push ( ctx );
            }

            LockReleaseA ( client->sendQueueLock, "PushSend" );
            
            if ( !success )
                break;
            
            sendThread.Notify ( "PushSend" );
            return true;
        }
        
        if ( ctx ) {
            delete ctx;
        }
        
        return false;
    }
    
    
    bool MediatorClient::PushSend ( ThreadInstance * client, char * toSend, unsigned int toSendSize, unsigned int toSendCurrent, bool copy )
    {
        bool success = false;
        
        SendContext * ctx = new SendContext ();
        while ( ctx )
        {
#ifdef XCODE_ANALYZER_BUG
            ctx->freeSendBuffer = false;
#endif
            if ( copy ) {
                ctx->buffer = ( char * ) malloc ( toSendSize );
                if ( !ctx->buffer )
                    break;
                
                memcpy ( ctx->buffer, toSend, toSendSize );
            }
            else {
                ctx->size			= 0;
            }
            
            ctx->sendBuffer		= toSend;
            ctx->sendSize		= toSendSize;
            ctx->sendCurrent	= toSendCurrent;
            
            if ( !LockAcquireA ( client->sendQueueLock, "PushSend" ) )
                break;
            
            if ( IsValidFD ( ( int ) client->socket ) )
                success = client->sendQueue.push ( ctx );

            LockReleaseA ( client->sendQueueLock, "PushSend" );
            
            if ( !success )
                break;
            
            if ( !copy )
                ctx->freeSendBuffer	= true;
            
            sendThread.Notify ( "PushSend" );
            return true;
        }
        
        if ( ctx ) {
            delete ctx;
        }
        return false;
    }
    
    
    bool MediatorClient::PushSendBC ( bool copy, void * buffer, unsigned int size, unsigned int ip, unsigned short port )
    {
        bool success = false;
        
        SendContext * ctx = new SendContext ();
        while ( ctx )
        {
            if ( copy )
            {
                ctx->buffer = ( char * ) malloc ( size );
                if ( !ctx->buffer )
                    break;
                
                memcpy ( ctx->buffer, buffer, size );
            }
            else
                ctx->buffer = ( char * ) buffer;
            
            ctx->size   = size;
            
            Zero ( ctx->addr );
            
            ctx->addr.sin_family         = PF_INET;
            ctx->addr.sin_port           = htons ( port );
            ctx->addr.sin_addr.s_addr    = ip;
            
            if ( !LockAcquireA ( sendQueueLock, "PushSendBC" ) )
                break;
            
            if ( IsValidFD ( broadcastSocket ) )
                success = sendQueue.push ( ctx );
            
            LockReleaseA ( sendQueueLock, "PushSendBC" );
            
            if ( !success )
                break;
            
            sendThread.Notify ( "PushSendBC" );
            return true;
        }
        
        if ( ctx ) {
            if ( !copy ) {
                ctx->buffer = 0;
            }
            delete ctx;
        }
        
        return false;
    }
    
    
    bool MediatorClient::SendBufferOrEnqueueBC ( bool copy, void * buffer, unsigned int size, unsigned int ip, unsigned short port )
    {
        if ( sendQueue.size_ > 0 ) {
            return PushSendBC ( copy, buffer, size, ip, port );
        }
        
        if ( IsValidFD ( (int) broadcastSocket ) )
        {
            struct 	sockaddr_in addr;
            
            Zero ( addr );
            
            addr.sin_family         = PF_INET;
            addr.sin_port           = htons ( port );
            addr.sin_addr.s_addr    = htonl ( ip );

			int rc = ( int ) sendto ( broadcastSocket, ( const char * ) buffer, size, 0, ( struct sockaddr * ) &addr, sizeof ( struct sockaddr ) );
            if ( rc < 0 )
            {
                SOCKET_Check_Val ( check );
                
                if ( SOCKET_Check_Retry ( check ) ) {
                    return PushSendBC ( copy, buffer, size, ip, port );
                }
            }
            
            else if ( rc > 0 )
            {
				if ( rc == ( int ) size ) {
                    if ( !copy )
                        free ( buffer );
                    return true;
                }
                else {
                    CVerbArg ( "SendBufferOrEnqueueBC: Failed [ %i != %u ].", rc, size );
                }
            }
        }
        return false;
    }
    
    
    int MediatorClient::SendBufferBC ( SendContext * ctx )
    {
        CVerbVerbArg ( "SendBufferBC [ %i ]", broadcastSocket );
        
        int             rc          = -1;
        
        CVerbArg ( "SendBufferBC [ %i ]: [ %i ] bytes", broadcastSocket, ctx->size );
        
        if ( IsValidFD ( (int) broadcastSocket ) )
        {
            rc = ( int ) sendto ( broadcastSocket, ctx->buffer, ctx->size, 0, ( struct sockaddr * ) &ctx->addr, sizeof ( struct sockaddr ) );
            if ( rc < 0 )
            {
                SOCKET_Check_Val ( check );
                
                if ( SOCKET_Check_Retry ( check ) ) {
                    return 0;
                }
            }
            
            else if ( rc > 0 )
            {
				if ( rc == ( int ) ctx->size ) {
                    rc = 1;
                }
                else {
                    CVerbArg ( "SendBufferBC: Failed [ %i != %u ].", rc, ctx->size );
                }
            }
        }
        
        CVerbArg ( "SendBufferBC [ %i ]: [ %i ] bytes. Done [ %i ].", broadcastSocket, ctx->size, rc );
        return rc;
    }
    
    
    bool WaitForSend ( int &sock )
    {
        struct pollfd desc;
        
        desc.events = POLLOUT | POLLERRMASK;
        desc.fd = ( int ) sock;
        desc.revents = 0;
        
        if ( IsValidFD ( sock ) )
        {
            int rc = poll ( &desc, 1, 10 );
            if ( rc > 0 ) {
                if ( desc.revents & POLLOUT )
                    return true;
            }
        }
        return false;
    }
    
    
    int MediatorClient::PushSend ( void * buffer, unsigned int size, unsigned int seqNr )
    {
#if defined(DEBUG_SEQNR)
        if ( seqNr ) {
            CLogArg ( "PushSend: seqNr [ %i ]", seqNr );
        }
#endif
        MediatorInstance * med = GetAvailableMediator ();
        if ( med ) {
            if ( PushSend ( &med->connection.instance, buffer, size, seqNr ) )
                return size;
        }
        return -1;
    }
    
    
    int MediatorClient::SendBufferNoQueue ( ThreadInstance * client, void * buffer, unsigned int size )
    {
        CVerbVerbArg ( "SendBufferNoQueue [ %i ]", client->socket );
        
        int             rc          = -1;
        unsigned int    toSendLen   = size;
        char *          cipher      = 0;
        int             repeats     = 0;
        
        CVerbArg ( "SendBufferNoQueue [ %i ]", client->socket );
        
        if ( IsInvalidFD ( client->socket ) )
            return -1;
        
        char * toSend = (char *) buffer;
        if ( !toSend )
            return -1;
        
        if ( client->encrypt )
        {
            if ( !AESEncrypt ( &client->aes, toSend, &toSendLen, &cipher ) || !cipher ) {
                CErrArg ( "SendBufferNoQueue [ %i ]: Failed to encrypt AES message.", client->socket );
                
                // Failed to encrypt. For now, we just skip this send context and go on to the next one
                return -1;
            }
            
            toSend      = cipher;
        }
        
        CVerbArg ( "SendBufferNoQueue [ %i ]: [ %i ] bytes", client->socket, toSendLen );
        
    Retry:
        if ( IsValidFD ( ( int ) client->socket ) )
        {
            rc = ( int ) send ( ( int ) client->socket, toSend, toSendLen, MSG_NOSIGNAL );
            if ( rc < 0 )
            {
                SOCKET_Check_Val ( check );
                
                if ( SOCKET_Check_Retry ( check ) ) {
                    repeats++;
                    if ( repeats < 3 )
                    if ( WaitForSend ( client->socket ) )
                        goto Retry;
                }
            }
            
            else if ( rc != ( int ) toSendLen ) {
                CErrArg ( "SendBufferNoQueue [ %i ]: Failed to send complete message [ %i : %i ].", client->socket, toSendLen, rc );
            }
            else {
                rc = size;
            }
        }
        
        free_n ( cipher );
        
        CVerbArg ( "SendBufferNoQueue [ %i ]: [ %i ] bytes. Done [ %i ].", client->socket, toSendLen, rc );
        return rc;
    }
    
    
    void MediatorClient::DisposeSendContextsBC ()
    {
        LockAcquireA ( sendQueueLock, "DisposeSendContextsBC" );
        
        while ( sendQueue.size_ > 0 )
        {
            SendContext * ctx = ( SendContext * ) sendQueue.pop ();
            if ( ctx )
                delete ctx;
        }
        
        LockReleaseA ( sendQueueLock, "DisposeSendContextsBC" );
    }
    
    
    void * MediatorClient::AliveThreadStarter ( void * arg )
	{
		MediatorClient * mediator = ( MediatorClient * ) arg;
		if ( !mediator ) {
			CErr ( "AliveThreadStarter: called with (NULL) argument!" );
			return 0;
		}
        
		mediator->AliveThread ();
        
        mediator->aliveThread.Detach ( "AliveThreadStarter" );
        
        if ( mediator->isRunning && mediator->aliveRunning && mediator->env->environsState >= environs::Status::Starting )
        {
            if ( mediator->aliveThreadRestarts <= 3 )
            {
                // Let's restart the thread. Something has shut us down unexpectedly
                // while all indicators expect us to be running for a working mediator layer
                mediator->aliveThreadRestarts++;
                
                if ( mediator->StartAliveThread ( ) )
                {
                    CWarn ( "AliveThreadStarter: Restarted thread." );
                    return 0;
                }
            }
            
            // Restart has failed 3 times ... or restart has failed at all
            // We're giving up here and shut down alive thread
            
            CErr ( "AliveThreadStarter: Failed to restart thread !!!" );
        }
        
        return 0;
	}


	void MediatorClient::AliveThread ()
	{
		CVerb ( "AliveThread: started ..." );

		pthread_setname_current_envthread ( "MediatorClient::AliveThread" );

		if ( !isRunning )
			return;

		INTEROPTIMEVAL 	heartbeatRate		= INTEROPTIMEMS ( 3 * 60 * 1000 );
		int				deviceChanges		= 0; // MEDIATOR_DEVICE_CHANGE_MEDIATOR | MEDIATOR_DEVICE_CHANGE_NEARBY;
		INTEROPTIMEVAL	lastBeat			= 0;


		MediatorInstance    * med   = &mediator;
        ThreadInstance      * inst  = &med->connection.instance;

        SendBroadcast ();

		while ( aliveRunning && env->environsState >= environs::Status::Starting )
		{
			if ( deviceChanges )
			{
				if ( deviceChanges < 0 )
					DevicesAvailableReload ();
				else
					DevicePacketUpdater ();
				deviceChanges = 0;
			}

			INTEROPTIMEVAL now = GetEnvironsTickCount ();

            CVerbsArg ( 5, "AliveThread: diff [ %10i ] heartbeat [ %i ]", (int) (now - lastBeat), (int) heartbeatRate );

			if ( native.networkStatus > NETWORK_CONNECTION_NO_NETWORK &&
                med->ip && med->port && med->enabled && env->environsState >= environs::Status::Starting )
            {
                bool tryRegister = false;

                if ( med->listening )
                {
                    if ( ( now - lastBeat ) >= heartbeatRate )
                    {
                        CVerbs ( 5, "AliveThread: check devices" );

                        int mediatorDevCount = GetDevicesFromMediatorCount ();

                        CVerbsArg ( 2, "AliveThread: check devices [ %i : %i ]", mediatorDevCount, deviceMediatorQueryCount );

                        if ( mediatorDevCount != deviceMediatorQueryCount || mediatorDevCount != (deviceMediatorCachedCount + 1) )
                        {
                            CVerbVerb ( "AliveThread: MediatorDevCount update" );
                            deviceChanges = MEDIATOR_DEVICE_RELOAD;
                        }
                        else {
                            VerifySockets ( inst, false );

                            tryRegister = true;
                        }

                        lastBeat = now;
                    }
                }
                else {
                    tryRegister = true;
                }

                if ( tryRegister && IsInvalidFD ( inst->socket ) && isRunning ) {
                    CVerbVerb ( "AliveThread: Trying to register at a Mediator ..." );

					if ( connectFails > ENVIRONS_MEDIATOR_MAX_TRYS ) {
						// If mediator connect has been suspended due to many connect errors
						// then reuse the counter to enable a calm down phase of approx. 4 minutes
						connectFails++;

						// If the alivethread has passed this 4 times
						// then let's assume that the alive thread took 1 minute each (as no mediator reload was possible)
						// (In reality, the alive thread may have taken less or more than a minute as other entities are reusing the alive thread for watchdog works)
						// and reset the counter to reenable connects again with next alive thread round
						if ( connectFails > (ENVIRONS_MEDIATOR_MAX_TRYS + 4) )
							connectFails = 0;
					}
					else
						RegisterAtMediators ( false );
                }
			}
            
			vsp ( StuntRegisterContext ) toDispose;

            if ( LockAcquireA ( inst->stuntSocketLock, "AliveThread" ) )
            {
                now = GetEnvironsTickCount32 ();
                
                msp ( std::string, StuntRegisterContext )::iterator it = inst->stuntSocketsLog.begin ();
                
                while ( it != inst->stuntSocketsLog.end () )
                {
                    const sp ( StuntRegisterContext ) &ctx = it->second;
                    
                    if ( now - ctx->registerTime > 30000 )
                    {
						toDispose.push_back ( it->second );

                        inst->stuntSocketsLog.erase ( it++ );
                    }
                    else ++it;
                }
                
                LockReleaseA ( inst->stuntSocketLock, "AliveThread" );
            }

			toDispose.clear ();

            TraceAliveLocker ( "AliveThread" );
            if ( !aliveThread.Lock ( "AliveThread" ) )
				break;

			if ( deviceChanges )
				deviceChangeIndicator |= deviceChanges;

			if ( !deviceChangeIndicator )
            {
                TraceAliveUnlocker ( "AliveThread" );
                if ( !aliveThread.Unlock ( "AliveThread" ) )
                    break;

				// Check broadcast devices for vanished devices
				VanishedDeviceWatcher ();

				// Query greet updates if necessary
                unsigned int now32 = (now & 0xFFFFFFFF);

				if ( now32 > lastGreetUpdate && ( now32 - lastGreetUpdate ) > 60000 ) {
					// As we ignore our own greets, we need to modify the last greet timestamp
					// Number of milliseconds since system has started
					lastGreetUpdate = GetEnvironsTickCount32 ();

					strlcpy ( broadcastMessage + 4, MEDIATOR_BROADCAST_GREET, sizeof ( MEDIATOR_BROADCAST_GREET ) );
					SendBroadcast ();
                }
                
                aliveThread.WaitOne ( "AliveThread", 60000, true, true );
                
                // WaitOne is based on conditional lock, which is ignored for windows
                // as Windows platforms don't require a lock for events/signals
                // Ignoring the lock is much more performant in most cases
                
                // However, here we need to manually lock for windows
#ifdef _WIN32
                TraceAliveLocker ( "AliveThread 1" );
                if ( !aliveThread.Lock ( "AliveThread" ) )
                    break;
#endif
                aliveThread.ResetSync ( "AliveThread", false );
			}

			deviceChanges           = deviceChangeIndicator;
			deviceChangeIndicator   = 0;
            
            TraceAliveUnlocker ( "AliveThread 1" );
            if ( !aliveThread.Unlock ( "AliveThread" ) )
                break;
#ifdef _WIN32
#ifndef NDEBUG
            PROCESS_MEMORY_COUNTERS_EX status;
            
            GetProcessMemoryInfo ( GetCurrentProcess (), ( PPROCESS_MEMORY_COUNTERS ) &status, sizeof ( status ) );
            
            if ( status.WorkingSetSize > MAX_MEMORY_CHECK_BREAK || status.PrivateUsage > MAX_MEMORY_CHECK_BREAK )
            {
                keybd_event ( VK_ESCAPE, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0 );
                
                keybd_event ( VK_ESCAPE, 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0 );
                //_EnvDebugBreak ();
                break;
            }
#endif
#endif
        }
        
        CVerb ( "AliveThread: bye bye ..." );
	}


	bool MediatorClient::SendUDPFIN ( int sock )
	{
		if ( sock < 0 )
            return false;
        
        if ( IsInvalidFD ( native.udpSignalSender ) && !native.InitSignalSender () )
            return false;
        
        struct sockaddr_in  addr;
        Zero ( addr );
        
        socklen_t length = sizeof ( sockaddr );
        
#ifdef DEBUGVERB
        int ret =
#endif
            getsockname ( sock, ( struct sockaddr * )&addr, &length );
        
#ifdef DEBUGVERB
        if ( !ret ) {
            char * ip = inet_ntoa ( addr.sin_addr );
            if ( ip ) {
                CVerbArg ( "SendUDPFIN: [ %s : %i ]", ip, ntohs ( addr.sin_port ) );
            }
        }
#endif        
        sendto ( native.udpSignalSender, "FIN;", 4, 0, ( struct sockaddr * )&addr, sizeof ( struct sockaddr ) );
        sendto ( native.udpSignalSender, "FIN;", 4, 0, ( struct sockaddr * )&addr, sizeof ( struct sockaddr ) );
        
        addr.sin_addr.s_addr = htonl ( INADDR_LOOPBACK );
        
        sendto ( native.udpSignalSender, "FIN;", 4, 0, ( struct sockaddr * )&addr, sizeof ( struct sockaddr ) );
        sendto ( native.udpSignalSender, "FIN;", 4, 0, ( struct sockaddr * )&addr, sizeof ( struct sockaddr ) );
        return true;
    }

    
    bool MediatorClient::SocketKeepAlive ( int sock, int opt )
    {
        if ( sock < 0 )
            return false;
        
        int ret = setsockopt ( sock, SOL_SOCKET, SO_KEEPALIVE, ( const char * ) &opt, sizeof ( opt ) );
        if ( ret < 0 ) {
            CErr ( "AllocateMainSocket: Failed to set keepalive on socket." ); LogSocketError ();
            return false;
        }
        return true;
    }
    

    int MediatorClient::Receive ( int &sock, char * buffer, unsigned int bufferSize, unsigned int minReceiveBytes, const char * receiver )
    {
        CVerbVerbArg ( "[ %s ].Receive", receiver );

        char                ip [ 20 ];
        *ip = 0;

#ifndef NDEBUG
        struct sockaddr_in  addr;
        Zero ( addr );

        socklen_t length = sizeof ( sockaddr );

        int ret = getpeername ( sock, ( struct sockaddr * )&addr, &length );
        if ( !ret ) {
            char * s = inet_ntoa ( addr.sin_addr );
            if ( s ) {
                strlcpy ( ip, s, sizeof(ip) );
            }
        }

        CVerbsArg ( 5, "[ %s ].Receive from [ %s ]", receiver, ip );
#endif
        if ( !buffer || !bufferSize ) {
            CErrArg ( "[ %s ].Receive: Called with NULL argument", receiver );
            return -1;
        }

        if ( !SocketTimeout ( sock, WAIT_TIME_FOR_RECEIVING_TCP_MSG, -1 ) )
            return -1;
        
        int		rc				= -1;

        int		bytesOverall	= 0;
        int		bytesReceived	= 0;
        char *	recBuffer		= buffer;
		bool	repeated		= false;
        unsigned int bufferBytesRemain = bufferSize;

    ReceiveNext:
        CVerbsArg ( 6, "[ %s ].Receive: receiving on socket [ %i : %s ] ...", receiver, sock, ip );

#ifdef MEDIATOR_RECEIVE_TRACER
		unsigned int start_ms_32 = GetEnvironsTickCount32 ();
#endif
        bytesReceived = ( int ) recv ( sock, recBuffer, bufferBytesRemain, 0 );
        
#ifdef MEDIATOR_RECEIVE_TRACER
		unsigned int end_ms_32 = GetEnvironsTickCount32 (); unsigned int diff_ms_32 = end_ms_32 - start_ms_32;
		CVerbsArg ( 1, "[ %s ].Receive: rc [ %i : %u ms ] repeated [ %i ]", receiver, bytesReceived, diff_ms_32, (int)repeated );
#endif
        CVerbsArg ( 6, "[ %s ].Receive: rc [ %i ]", receiver, bytesReceived );
        if ( bytesReceived <= 0 ) 
		{
            rc = bytesReceived;
            CErrArg ( "[ %s ].Receive: rc [ %i : %s ] errno [ %i ]", receiver, rc, ip, errno );
			
			if ( rc == 0 && !repeated ) {
                if ( errno == 0 && IsValidFD ( sock ) ) {
                    CVerbsArg ( 1, "[ %s ].Receive: Repeating once.", receiver );
					repeated = true;
					goto ReceiveNext;
				}
			}

            if ( IsValidFD ( sock ) ) { LogSocketErrorF ( "MediatorClient.Receive" ); }
        }
        else {
            bytesOverall += bytesReceived;
            if ( minReceiveBytes )
            {
                if ( bytesOverall < ( int ) minReceiveBytes ) {
                    CVerbsArg ( 6, "[ %s ].Receive: min. bytes received not reached [ %i : %i : %s ]. Receive next ...", receiver, bytesOverall, minReceiveBytes, ip );

                    bufferBytesRemain -= bytesReceived;
                    recBuffer += bytesReceived;

                    goto ReceiveNext;
                }
            }
            rc = bytesOverall;
        }
        
        if ( rc >= 0 && !SocketTimeout ( sock, 0, -1 ) )
            return -1;

        CVerbsArg ( 6, "[ %s ].Receive: rc [ %i : %s ].", receiver, rc, ip );
        return rc;
    }


    int	MediatorClient::ReceiveOneMessage ( bool encrypt, AESContext * aes, int &sock, char * buffer, unsigned int bufferSize, char *& decrypted )
    {
        CVerb ( "ReceiveOneMessage" );

        int				bytesReceived	= 0;
        unsigned int	length			= 0;
        int				bufferRemaining = ( int ) bufferSize;
        char		*	pBuffer			= buffer;
        ComMessageHeader * header		= reinterpret_cast<ComMessageHeader *>( buffer );

        unsigned int	recMsgLength	= 0;

        while ( true )
        {
            pBuffer += bytesReceived;
            bufferRemaining = bufferSize - length;

            if ( bufferRemaining <= 0 ) {
                CErr ( "ReceiveOneMessage: Preventing receive buffer overflow. Aborting receive." );
                return -1;
            }

            bytesReceived = MediatorClient::Receive ( sock, pBuffer, bufferRemaining, 0, "ReceiveOneMessage" );
            if ( bytesReceived <= 0 ) {
                CWarn ( "ReceiveOneMessage: Socket error" );
                VerbLogSocketError ();
                return -1;
            }
            length += bytesReceived;

            if ( length >= 8 && recMsgLength == 0 ) {
                if ( encrypt )
                    recMsgLength = *( reinterpret_cast<unsigned int *>( buffer ) ) & 0xFFFFFF;
                else
                    recMsgLength = header->length;

                if ( recMsgLength >= bufferSize ) {
                    CErrArg ( "ReceiveOneMessage: Expected buffer overflow [ %u : %u ]! [ %u bytes : %s ] Aborting receive.", recMsgLength, bufferSize, length, encrypt ? "E" : "P" );
                    return -1;
                }
            }

            if ( length >= recMsgLength )
                break;
        }

        if ( encrypt && recMsgLength > 0 ) {
            if ( recMsgLength <= 4 )
                return -1;

            if ( !AESDecrypt ( aes, buffer, &recMsgLength, &decrypted ) )
                return -1;
        }

        return recMsgLength;
    }


	void * MediatorClient::BroadcastThread ()
	{
		CVerb ( "BroadcastThread: started ..." );

		pthread_setname_current_envthread ( "MediatorClient::BroadcastThread" );

		int			success;
		int			bytesReceived;
		socklen_t	addrLen;
		char	*	msg;
		char		buffer [ BUFFERSIZE ];

		int		*	pToken = reinterpret_cast<int *>( buffer );

		unsigned int ip;

        broadcastThread.Notify ( "MediatorClient::BroadcastThread" );

		if ( IsInvalidFD ( broadcastSocket ) ) {
			CErr ( "BroadcastThread: Invalid broadcast socket!" );
			return 0;
		}

		struct 	sockaddr_in	addr;
		Zero ( addr );

		addr.sin_family         = AF_INET;
		addr.sin_addr.s_addr	= htonl ( INADDR_ANY ); //htonl ( INADDR_BROADCAST );
		addr.sin_port           = htons ( GET_MEDIATOR_BASE_PORT );

		success = ::bind ( broadcastSocket, ( struct sockaddr * ) &addr, sizeof ( addr ) );
		if ( success < 0 ) {
            CErrArg ( "BroadcastThread: Failed to bind broadcast listener socket to port [ %i ]", GET_MEDIATOR_BASE_PORT );
            
            if ( IsValidFD ( broadcastSocket ) ) { LogSocketErrorF ( "MediatorClient.BroadcastThread" ); }

			return 0;
		}
		CVerbArg ( "BroadcastThread bound socket to port %i", GET_MEDIATOR_BASE_PORT );

		addrLen = sizeof ( addr );

		broadcastRunning = true;

        // Send helo
		strlcpy ( broadcastMessage + 4, MEDIATOR_BROADCAST_GREET, sizeof ( MEDIATOR_BROADCAST_GREET ) );
		SendBroadcast ( true, false, true );
        
#ifdef USE_MEDIATOR_NON_BLOCK_BROADCAST_WINSOCK
		if ( WSAEventSelect ( broadcastSocket, broadcastEvent, FD_READ | FD_CLOSE ) == SOCKET_ERROR )
		{
			CErrArg ( "BroadcastThread: Failed to register broadcastSocket event [ %d ]!", WSAGetLastError () );
			return 0;
		}
#else
        struct pollfd desc;
        desc.events = POLLIN | POLLERRMASK;
        
        if ( !SetNonBlockSocket ( broadcastSocket, true, "BroadcastThread" ) )
            return 0;
#endif
		while ( isRunning && IsValidFD ( broadcastSocket ) )
        {
            // Wait for broadcast
            bytesReceived = ( int ) recvfrom ( broadcastSocket, buffer, BUFFERSIZE, 0, ( struct sockaddr * ) &addr, &addrLen );
            
            if ( bytesReceived == 0 ) {
                LogSocketErrorF ( "MediatorClient.BroadcastThread" );
                CLogArg ( "BroadcastThread: connection/socket [ %i ] closed by someone; Bytes [ %i ]!", broadcastSocket, bytesReceived );
                break;
            }
            
            if ( bytesReceived < 0 )
            {
                SOCKET_Check_Val ( check );
                
                if ( SOCKET_Check_Retry ( check ) )
                {
#ifdef USE_MEDIATOR_NON_BLOCK_BROADCAST_WINSOCK
					WSAWaitForMultipleEvents ( 1, &broadcastEvent, FALSE, WSA_INFINITE, FALSE );

					WSAResetEvent ( broadcastEvent );

					if ( IsInvalidFD ( broadcastSocket ) )
						break;
#else
                Retry:
                    desc.fd         = broadcastSocket;
                    desc.revents    = 0;
                    
                    if ( IsInvalidFD ( broadcastSocket ) )
                        break;
                    
                    int rc = poll ( &desc, 1, -1 );
                    
                    if ( !isRunning )
                        break;
                    
                    if ( rc == -1 || (desc.revents & POLLERRMASK) ) {
                        CVerb ( "BroadcastThread: Socket has been closed" );
                        LogSocketErrorF ( "MediatorClient.BroadcastThread" );
                        break;
                    }
                    
                    if ( rc == 0 )
                        goto Retry;
#endif
					continue;
                }
                break;
            }
            
            if ( env->environsState <= environs::Status::Stopping )
                continue;

			ip = addr.sin_addr.s_addr;

			if ( *pToken == randBroadcastToken ) {
				if ( IsLocalIP ( ip ) )
					continue;

				// We need to renew our broadcast token
				BroadcastGenerateToken ();
			}

			msg = buffer + 4;

			if ( msg [ 0 ] != 'E' || msg [ 1 ] != '.' || bytesReceived >= BUFFERSIZE )
				continue;

			buffer [ bytesReceived ] = 0;

			CVerbVerbArg ( "BroadcastThread: Read %d bytes from [ %s ]; Message: [ %s ]", ( int ) bytesReceived, inet_ntoa ( *( ( struct in_addr * ) &ip ) ), msg );

			if ( msg [ 2 ] == 'D' )
            {
                broadcastReceives++;

				if ( msg [ 4 ] == 'B' )
				{
					RemoveDevice ( ip, msg );
                }
                else if ( msg [ 4 ] == 'D' )
                {
                    HandleDeviceUpdateMessage ( buffer );
                }
				else if ( UpdateDevices ( ip, msg, 0, 0, true ) && msg [ 4 ] == 'H' )
				{
					// If the message is a greeting of a new device saying hallo ...

					// Number of milliseconds since system has started
					lastGreetUpdate = GetEnvironsTickCount32 ();

					strlcpy ( broadcastMessage + 4, MEDIATOR_BROADCAST_DEVICEINFO, sizeof ( MEDIATOR_BROADCAST_DEVICEINFO ) );
					SendBroadcast ();
                    
                    broadcastThreadRestarts = 0;
				}
			}
			// Check whether the message was sent by a mediator
			/*else if ( msg [ 2 ] == 'M' )
			{
				if ( HandleMediatorMessage ( ip, msg ) ) {
					CVerbVerbArg ( "BroadcastThread: Registered mediator [ %s ]", inet_ntoa ( *( ( struct in_addr * ) &ip ) ) );

					RegisterAtMediators ( false );
				}
			}
			*/
		}

		broadcastRunning = false;
        
		CVerbs ( 1, "BroadcastThread: bye bye ..." );
		return NULL;
	}


	MediatorInstance * MediatorClient::HandleMediatorMessage ( unsigned int ip, char * msg )
	{
		CVerbVerbArg ( "HandleMediatorMessage: [ %s ]", msg );

		if ( mediator.listening || mediator.connection.instance.socket >= 0 ) {
			return 0;
		}

		if ( mediator.next ) {
			if ( mediator.next->listening || mediator.next->connection.instance.socket >= 0 ) {
				return 0;
			}
		}

		unsigned short port = *( ( unsigned short * ) ( msg + 12 ) );
		if ( !ip || !port )
			return 0;

		/// For now we support only one Mediator at any time
		MediatorInstance * med = IsKnownMediator ( ip, port );
		if ( med )
			return med;
		return 0;
		/*
		CVerbArg ( "HandleMediatorMessage: Setting new Mediator [%s]", inet_ntoa ( *((struct in_addr *) &ip) ) );

		med = &mediator;

		med->ip = ip;
		med->port = port;
		med->connection.instance.socket = INVALID_FD;
		med->connection.instance.stuntSocket = INVALID_FD;

		if ( !med->connection.buffer ) {
		med->connection.buffer = (char *) malloc ( MEDIATOR_REC_BUFFER_SIZE_MAX );
		if ( !med->connection.buffer ) {
		CErr ( "HandleMediatorMessage: ERROR - Failed to allocate memory for new Mediator!" );
		return 0;
		}
		}

		return AddMediator ( med );
		*/
	}


	int MediatorClient::RequestDeviceID ( MediatorInstance * med )
	{
		CVerb ( "RequestDeviceID" );

		int                 deviceID	= 0;

		ThreadInstance	*	inst = &med->connection.instance;

		MediatorReqMsg      buffer;

#ifndef NDEBUG
		// Initialize buffer for valgrind ...
		// Acutally it is intended to send some few more (uninitialized) bytes to the target.
		// The target access only specified areas.
		Zero ( buffer );
#endif

        int             *	pUI			= 0;
		unsigned int		toSendSize;
		char			*	cipher		= 0;
		char			*	decrypt		= 0;
		int					bytes;

		do
		{
			if ( IsInvalidFD ( inst->socket ) ) {
				CVerb ( "RequestDeviceID: No connection to mediator available." ); break;
			}

			MediatorReqHeader * req = ( MediatorReqHeader * ) &buffer;

			req->cmd0 = 'E';
			req->cmd1 = MEDIATOR_OPT_BLANK;
			req->opt0 = 'D';
			req->opt1 = MEDIATOR_OPT_DEVICE_LIST_DEVICE_ID;

            if ( !BuildAppAreaField ( req->sizes, "", native.deviceUID, true ) )
                break;

			unsigned char * sizes = req->sizes + 2 + req->sizes [ 0 ] + req->sizes [ 1 ];

            if ( !BuildAppAreaField ( sizes, env->appName, env->areaName, false ) )
                break;

			toSendSize = sizeof ( MediatorReqHeader ) + req->sizes [ 0 ] + req->sizes [ 1 ] + 2 + sizes [ 0 ] + sizes [ 1 ];

            req->size = toSendSize;

			if ( !AESEncrypt ( &inst->aes, (char *) &buffer, &toSendSize, &cipher ) ) {
				CErr ( "RequestDeviceID: AES encrypt failed!" ); break;
			}

            CVerbVerbArg ( "RequestDeviceID: Send packet of size [ %d ]  ...", toSendSize );

            bytes = ( int ) send ( inst->socket, cipher, toSendSize, MSG_NOSIGNAL );
			if ( bytes != ( int ) toSendSize ) {
                CLogArg ( "RequestDeviceID: Send (request) failed %i != %i", bytes, toSendSize );
                
                if ( IsValidFD ( inst->socket ) ) { LogSocketErrorF ( "MediatorClient.RequestDeviceID" ); }
                break;
			}

            bytes = ReceiveOneMessage ( inst->encrypt != 0, &inst->aes, inst->socket, (char *) &buffer, sizeof ( buffer ), decrypt  );
            if ( bytes <= 0 ) {
                CWarn ( "RequestDeviceID: Socket to device has been closed." );
                break;
            }

			if ( bytes > 4 || !decrypt ) {
				CVerbArg ( "RequestDeviceID: Response size is invalid [ %i ]", bytes ); break;
			}

			pUI			= ( int * ) decrypt;
			deviceID	= *pUI;
		}
		while ( 0 );

        free_n ( decrypt );
        free_n ( cipher );

		return deviceID;
    }


    bool MediatorClient::RegisterAtMediators ( bool wait )
    {
        if ( !isRunning )
            return false;

        if ( ___sync_val_compare_and_swap ( &registratorState, ENVIRONS_THREAD_NO_THREAD, ENVIRONS_THREAD_DETACHEABLE ) != ENVIRONS_THREAD_NO_THREAD ) {
            CVerbVerb ( "RegisterAtMediators: A registrator is already running" );
            return true;
        }

        bool success = false;

        if ( native.networkStatus < NetworkConnection::NoInternet ) {
            CVerb ( "RegisterAtMediators: No network connection available." );

            StopAliveThread ( true );
            
            StopSendThread ();
            
            goto Finish;
        }

        if ( !env->useCustomMediator && !env->useDefaultMediator ) {
            CVerb ( "RegisterAtMediators: Mediator usage is disabled by settings." );
			goto Finish;
        }

        if ( wait ) {
            success = RegisterAtMediatorsDo ();
        }
        else {
			if ( LockAcquireA ( registerLock, "RegisterAtMediators" ) )
			{
				registratorSP = env->myself;

				if ( registratorSP ) {
					int s = pthread_create ( &registratorThreadID, 0, &MediatorClient::MediatorRegistrator, ( void * ) this );
					if ( s != 0 )
					{
						registratorSP.reset ();

						CErr ( "RegisterAtMediators: Error creating thread." );
					}
					else success = true;
				}

				LockReleaseVA ( registerLock, "RegisterAtMediators" );
				if ( success )
					return true;
			}
        }

	Finish:
        registratorState = ENVIRONS_THREAD_NO_THREAD;

        return success;
    }


    void * MediatorClient::MediatorRegistrator ( void *arg )
    {
        if ( arg ) {
            MediatorClient * client = ( MediatorClient * ) arg;

			sp ( Instance ) envSP;

			if ( LockAcquireA ( client->registerLock, "RegisterAtMediators" ) )
			{
				envSP = client->registratorSP;

				client->registratorSP.reset ();

				LockReleaseVA ( client->registerLock, "RegisterAtMediators" );
			}

			if ( envSP )
				client->RegisterAtMediatorsDo ();

            DetachThread ( 0, &client->registratorState, client->registratorThreadID, "Registrator Thread" );
        }

        return 0;
    }


	bool MediatorClient::RegisterAtMediatorsDo ()
	{
		// Send to all of the mediators
		MediatorInstance * med = &mediator;

		bool ret = true;

		if ( !isRunning ) {
			return false;
		}

		CVerb ( "RegisterAtMediatorsDo" );

		if ( !med->ip || !med->port ) {
			if ( env->environsState >= environs::Status::Starting ) {
				CWarn ( "RegisterAtMediatorsDo: No mediators to register! Environment managing is based on broadcasts." );
			}
		}
		else
		{
			while ( med ) {
				if ( !RegisterAtMediator ( med ) )
					ret = false;

				med = med->next;
			}
		}
            
		return ret;
	}


	bool MediatorClient::ConnectToMediator ( MediatorInstance * med )
	{
		CVerb ( "ConnectToMediator" );

		if ( connectFails > ENVIRONS_MEDIATOR_MAX_TRYS ) {
			CWarn ( "ConnectToMediator: Too many connect errors detected." );
			return false;
		}

		int                 sock    = INVALID_FD;
		bool                ret     = false;
		struct sockaddr_in  addr;
		ThreadInstance *    inst    = &med->connection.instance;

		while ( IsInvalidFD ( inst->socket ) )
        {
            int rc;
            
			CVerb ( "ConnectToMediator: Creating new sockets" );

			pthread_cond_preparev ( &med->connection.receiveEvent );
            
            inst->thread.Lock ( "ConnectToMediator" );
            
            // Create socket
            sock = ( int ) socket ( PF_INET, SOCK_STREAM, 0 ); // IPPROTO_TCP (using 0 and let the service provider choose the protocol)
            if ( IsInvalidFD ( sock ) ) {
                CErr ( "ConnectToMediator: Failed to create a socket!" ); inst->thread.Unlock ( "ConnectToMediator" ); break;
            }
            CSocketTraceAdd ( sock, "MediatorClient ConnectToMediator sock" );
            
            inst->socket = sock;

            inst->thread.Unlock ( "ConnectToMediator" );
            
            DisableSIGPIPE ( sock );
            
            int value = 1;
            
#ifdef MEDIATOR_USE_TCP_NODELAY
            rc = setsockopt ( sock, IPPROTO_TCP, TCP_NODELAY, ( const char * ) &value, sizeof ( value ) );
            if ( rc < 0 ) {
                CErr ( "ConnectToMediator: Failed to set TCP_NODELAY on socket" );
                
                if ( IsValidFD ( inst->socket ) ) { LogSocketErrorF ( "MediatorClient.ConnectToMediator" ); }
            }
#endif
            // - Load send buffer size
			socklen_t valSize = sizeof ( value );
            
            rc = getsockopt ( inst->socket, SOL_SOCKET, SO_SNDBUF, ( char * ) &value, &valSize );
            if ( rc < 0 ) {
                CErr ( "ConnectToMediator: Failed to query send buffer size!" );
                
                if ( IsValidFD ( inst->socket ) ) { LogSocketErrorF ( "MediatorClient.ConnectToMediator" ); }
            }
            else {
                CVerbArg ( "ConnectToMediator: Send buffer size [ %i ]", value );
                
                int target = MEDIATOR_CLIENT_MAX_BUFFER_SIZE;
                target <<= 1;
                
                if ( value < target ) {
                    rc = setsockopt ( inst->socket, SOL_SOCKET, SO_RCVBUF, ( const char * ) &target, sizeof ( target ) );
                    if ( rc < 0 ) {
                        CErrArg ( "ConnectToMediator: Failed to set receive buffer size to [ %u ].", target );
                        
                        if ( IsValidFD ( inst->socket ) ) { LogSocketErrorF ( "MediatorClient.ConnectToMediator" ); }
                    }
                    else {
                        CVerbArg ( "ConnectToMediator: Receive buffer size set to [ %i ].", target );
                    }
                }
            }

			// Initialize communication SockAddr-struct
			Zero ( inst->addr );

			inst->addr.sin_family		= PF_INET;
			inst->addr.sin_port			= htons ( med->port );
			inst->addr.sin_addr.s_addr	= med->ip;

			CInfoArg ( "ConnectToMediator: IP [ %s : %d ]", inet_ntoa ( *( ( struct in_addr * ) &med->ip ) ), med->port );

			rc = Connect ( 0, inst->socket, ( struct sockaddr * )&inst->addr, 30, "Mediator listener" );
			if ( rc != 0 ) {
				connectFails++;
				CLogArg ( "ConnectToMediator: Failed connecting to mediator [ %s : %d ]", inet_ntoa ( *( ( struct in_addr * ) &med->ip ) ), med->port ); //LogSocketErrorF ( "MediatorClient.ConnectToMediator" );
				break;
			}
			connectFails = 0;

			if ( !GetMediatorLocalEndpoint ( med, &addr ) ) {
				CErr ( "ConnectToMediator: Failed to retrieve mediator local endpoint" ); inst->socket = INVALID_FD; break;
			}

			CVerbArg ( "ConnectToMediator: Local mediator endpoint [ %s : %d ]", inet_ntoa ( addr.sin_addr ), ntohs ( addr.sin_port ) );

			sock = INVALID_FD;
			ret = true;
			break;
		}

        if ( IsValidFD ( sock ) ) {
            inst->thread.Lock ( "ConnectToMediator" );
            
            sock = inst->socket;
            
            if ( IsValidFD ( sock ) ) {
                inst->socket = INVALID_FD;
                
                ShutdownCloseSocket ( sock, true, "MediatorClient.ConnectToMediator" );
            }
            
            inst->thread.Unlock ( "ConnectToMediator" );

			DevicesHasChanged ( MEDIATOR_DEVICE_RELOAD );

			API::onEnvironsNotifier1 ( env, NOTIFY_MEDIATOR_SERVER_DISCONNECTED );
		}
		return ret;
	}


	void MediatorClient::ApplyAnonymousCredentials ()
	{
		strlcpy ( env->UserName, MEDIATOR_ANONYMOUS_USER, sizeof ( env->UserName ) );

		*env->DefaultMediatorToken = 0;
		*env->CustomMediatorToken = 0;
		*env->DefaultMediatorUserName = 0;
		*env->CustomMediatorUserName = 0;

		Zero ( env->UserPassword );
		strlcpy ( env->UserPassword, MEDIATOR_ANONYMOUS_PASSWORD, sizeof ( env->UserPassword ) );
	}


    void MediatorClient::RequestLogonCredentials ()
	{
		env->UserPassword [ 1 ] = 1;
		API::onEnvironsNotifier1 ( env, NOTIFY_MEDIATOR_SERVER_PASSWORD_MISSING );
	}


    extern bool CreateAppID ( char * buffer, unsigned int bufSize );


	bool MediatorClient::HasMediatorCredentials ( MediatorInstance * med )
	{
		CVerb ( "HasMediatorCredentials" );

		if ( !env->useAuth )
			return true;


        if ( !*native.deviceUID ) {
			CreateAppID ( native.deviceUID, sizeof ( native.deviceUID ) );

			if ( !*native.deviceUID )
				return false;
        }

		if ( !*env->UserName )
		{
			if ( env->useAnonymous )
			{
				ApplyAnonymousCredentials ();
			}
			else if ( !*env->UserName ) {
				RequestLogonCredentials ();
				return false;
			}

			return true;
		}

		if ( IsAnonymousUser ( env->UserName ) )
		{
			if ( !env->useAnonymous ) {
				RequestLogonCredentials ();
				return false;
			}

			ApplyAnonymousCredentials ();
			return true;
		}

		if ( *env->UserPassword )
			return true;

		/// Look for an auth token
        if ( !med )
            med = &this->mediator;

		char * tok = ( med == &this->mediator ? env->DefaultMediatorToken : env->CustomMediatorToken );
		if ( *tok ) {
			if ( !IsAnonymousUser ( med == &this->mediator ? env->DefaultMediatorUserName : env->CustomMediatorUserName ) )
			{
				CVerbArg ( "HasMediatorCredentials: Using deviceUID/authToken for %s Mediator.", med == &this->mediator ? "default" : "custom" );
				return true;
			}
		}

		if ( env->useAnonymous ) {
			ApplyAnonymousCredentials ();
			return true;
		}

		RequestLogonCredentials ();
		return false;
	}


	bool MediatorClient::RegisterAtMediator ( MediatorInstance * med )
	{
		CVerb ( "RegisterAtMediator" );

		if ( ( !env->useCustomMediator && !env->useDefaultMediator ) || env->environsState < environs::Status::Starting )
			return false;

		if ( !med->ip || !med->port )
			return false;

		med->mediatorObject = ( void * ) this;

		bool ret = false;
		int sock = INVALID_FD;

		// Check validity of socket
		ThreadInstance * inst = &med->connection.instance;
        if ( IsValidFD ( inst->socket ) )
        {
            inst->thread.Lock ( "RegisterAtMediator" );
            
            VerifySockets ( inst, true );
            
            inst->thread.Unlock ( "RegisterAtMediator" );
        }

		if ( !HasMediatorCredentials ( med ) )
			return false;

		if ( IsInvalidFD ( inst->socket ) && !ConnectToMediator ( med ) ) {
			CVerb ( "RegisterAtMediator: Connect to mediator failed." ); goto Finish;
		}

		if ( !med->listening )
        {
			inst->sessionID = 0;

			//
			// Create listener thread
			med->connection.instance.thread.ResetSync ( "RegisterAtMediator" );

            if ( !med->connection.instance.thread.Run ( pthread_make_routine ( &MediatorClient::MediatorListenerStarter ), ( void * ) med, "RegisterAtMediator", true ) )
            {
                if ( !med->connection.instance.thread.isRunning () )
                {
                    CErr ( "RegisterAtMediator: Error creating thread for client request" );
                    sock = inst->socket; inst->socket = INVALID_FD; goto Finish;
                }
            }
		}
        else {
			ret = true;
		}

	Finish:
        if ( IsValidFD ( sock ) ) {
			ShutdownCloseSocket ( sock, true, "MediatorClient.RegisterAtMediator" );

			DevicesHasChanged ( MEDIATOR_DEVICE_RELOAD );

			API::onEnvironsNotifier1 ( env, NOTIFY_MEDIATOR_SERVER_DISCONNECTED );
		}

		return ret;
	}
    

	void MediatorClient::RemoveStuntDevice ( DeviceBase * device )
	{
		MediatorInstance * med = GetAvailableMediator ();
		if ( !med )
			return;

		ThreadInstance * inst = &med->connection.instance;

		char				keyID [ 36 + sizeof ( AppAreaBuffer ) ];
		char			*	key			= keyID + 4;
		char			*	appName		= 0;
		char			*	areaName	= 0;
		bool				found		= false;

		if ( device->deviceAreaName && device->deviceAreaName != env->areaName )
			areaName = device->deviceAreaName;

		if ( device->deviceAppName && device->deviceAppName != env->appName )
			appName = device->deviceAppName;

		if ( !BuildAppAreaID ( ( char * ) keyID, device->deviceID, appName, areaName, MEDIATOR_STUNT_CHANNEL_MAIN, device->connectToken ) ) {
			CErr ( "RemoveStuntDevice: Failed to build app area id!" );
			return;
		}

		sp ( StuntRegisterContext ) toDispose;

		if ( !LockAcquireA ( inst->stuntSocketLock, "RemoveStuntDevice" ) )
			return;

		const std::map < std::string, sp ( StuntRegisterContext ) >::iterator &foundIt = inst->stuntSocketsLog.find ( key );

		if ( foundIt != inst->stuntSocketsLog.end () )
		{
			toDispose = foundIt->second;

			inst->stuntSocketsLog.erase ( foundIt );
			found = true;
		}

		LockReleaseA ( inst->stuntSocketLock, "RemoveStuntDevice" );

		if ( !found )
			return;

		toDispose.reset ();

		char  buffer [ sizeof ( StuntClearTarget ) + sizeof ( AppAreaBuffer ) ];

		StuntClearTarget * msg = ( StuntClearTarget * ) buffer;

		if ( appName && areaName ) {
			if ( !BuildAppAreaField ( msg->sizes, appName, areaName, false ) ) {
				CErr ( "RemoveStuntDevice: Failed to build app area filed!" );
				return;
			}
		}
		else {
			msg->sizes [ 0 ] = 1; msg->sizes [ 1 ] = 1;

			char * tmp = ( char * ) ( msg->sizes + 2 ); *tmp = 0; tmp++; *tmp = 0;
		}

		msg->version = MEDIATOR_PROTOCOL_VERSION;
		msg->ident [ 0 ] = MEDIATOR_CMD_STUNT_CLEAR;
		msg->ident [ 1 ] = MEDIATOR_OPT_NULL;
		msg->ident [ 2 ] = MEDIATOR_OPT_NULL;

		msg->deviceID = device->deviceID;
		msg->size = sizeof ( StuntClearTarget ) + msg->sizes [ 0 ] + msg->sizes [ 1 ];
        
        SendBufferOrEnqueue ( inst, msg, msg->size, 0 );
	}


	void HandleStuntContextFailed ( RegisterStuntThread * req )
	{
		int                         sockToClose = INVALID_FD;

		sp ( StuntRegisterContext ) toDispose;
		char					*	key			= req->keyID + 4;
		ThreadInstance			*	inst		= &req->med->connection.instance;
		char						channel     = req->channel;

		if ( *key && LockAcquireA ( inst->stuntSocketLock, "HandleStuntContextFailed" ) )
		{
			const std::map < std::string, sp ( StuntRegisterContext ) >::iterator it = inst->stuntSocketsLog.find ( key );

			if ( it != inst->stuntSocketsLog.end () )
			{
				bool doErase = false;

				StuntRegisterContext * ctx = it->second.get ();

				int * s;
				int * s1;

				if ( channel == MEDIATOR_STUNT_CHANNEL_MAIN ) {
					s = &ctx->sockI;
					s1 = &ctx->sockC;
				}
				else {
					s = &ctx->sockC;
					s1 = &ctx->sockI;
				}

				if ( IsValidFD ( *s1 ) ) {
					sockToClose = *s; *s = INVALID_FD;
				}
				else
					doErase = true;

				if ( doErase ) {
					CWarnsArg ( CONCURRENT_STUNT_LOGLEVEL, "[ %s ].RegisterStuntSocket: Erasing context for [ %s ]", getChannel (), key );

					toDispose = it->second;

					inst->stuntSocketsLog.erase ( it );
				}
			}

			LockReleaseA ( inst->stuntSocketLock, "HandleStuntContextFailed" );

            if ( IsValidFD ( sockToClose ) ) {
				ShutdownCloseSocket ( sockToClose, true, "HandleStuntContextFailed" );
			}
		}
	}


	bool MediatorClient::RegisterStuntSocket ( bool invokeThread, MediatorInstance * med, int deviceID, const char * appName, const char * areaName, struct sockaddr_in * addr, char channel, unsigned int token, bool isMediatorRequest )
	{
        CVerbsArg ( 6, "[ %s ].RegisterStuntSocket: Context [ %u ] [ %s request : %s ] [ %s : %s : 0x%X ]", getChannel (), token, isMediatorRequest ? "Mediator" : "App code", invokeThread ? "Invoking thread" : "Exec here", appName ? appName : env->appName, areaName ? areaName : env->areaName, deviceID );

		if ( !med->enabled || env->environsState < environs::Status::Starting ) {
			CVerbs ( 5, "RegisterStuntSocket: Mediator is not ready." );
			return false;
		}

		sp ( RegisterStuntThread ) threadSP		= std::make_shared < RegisterStuntThread > ();

		RegisterStuntThread * req				= threadSP.get ();
		if ( !req )
			return false;

		int                 sock				= INVALID_FD, value, rc;
		int					success				= 0;
		bool                eraseOnErr			= false, locked = false;

		StuntSockRegTarget * regTarget			= ( StuntSockRegTarget * ) ( req->buffer + MEDIATOR_BROADCAST_SPARE_ID_LEN );

		char			*	keyID				= req->keyID;
		char			*	key					= keyID + 4;

		unsigned int        now;

		sp ( StuntRegisterContext ) ctxSP;
		StuntRegisterContext    *   ctx			= 0;
		int                     *   s           = 0;
		int                         sockToClose = INVALID_FD;

		ThreadInstance * inst = &med->connection.instance;
		if ( IsInvalidFD ( inst->socket ) ) {
			CErr ( "RegisterStuntSocket: Socket to Mediator is not available!" );
			return false;
		}

		if ( !inst->sessionID ) {
			CWarns ( 2, "RegisterStuntSocket: No session id received yet!" );
			return false;
		}

		if ( IsSameAppEnv ( env, appName, areaName ) ) {
			appName     = 0;
			areaName    = 0;

			regTarget->sizes [ 0 ] = 1; regTarget->sizes [ 1 ] = 1;

			char * tmp = ( char * ) ( regTarget->sizes + 2 ); *tmp = 0; tmp++; *tmp = 0;
		}
		else {
			if ( !BuildAppAreaField ( regTarget->sizes, appName, areaName, false ) ) {
				CErrArg ( "[ %s ].RegisterStuntSocket: Failed to build app area filed!", getChannel () );
				return false;
			}
		}

		if ( !BuildAppAreaID ( ( char * ) keyID, deviceID, appName, areaName, channel, token ) ) {
			CErrArg ( "[ %s ].RegisterStuntSocket: Failed to build app area id!", getChannel () );
			return false;
		}

		std::map < std::string, sp ( StuntRegisterContext ) >::iterator foundIt;

		if ( !LockAcquireA ( inst->stuntSocketLock, "RegisterStuntSocket" ) )
			return false;
		locked = true;

		if ( !med->enabled || env->environsState < environs::Status::Starting ) {
			CVerbs ( 5, "RegisterStuntSocket: Mediator is not ready." );
			goto Finalize;
		}

		foundIt = inst->stuntSocketsLog.find ( key );

		now = GetEnvironsTickCount32 ();

		if ( foundIt == inst->stuntSocketsLog.end () )
		{
			ctxSP = std::make_shared < StuntRegisterContext > ();
			if ( ctxSP )
			{
				ctx = ctxSP.get (); eraseOnErr = true;
				s = ( channel == MEDIATOR_STUNT_CHANNEL_MAIN ? &ctx->sockI : &ctx->sockC );

				CVerbsArg ( CONCURRENT_STUNT_LOGLEVEL, "[ %s ].RegisterStuntSocket: Created new context [ %u ] for [ %s ]!", getChannel (), token, key );

				ctx->token          = token;
				ctx->registerTime   = now;

				inst->stuntSocketsLog [ key ] = ctxSP;

				success = 1;
			}
		}
		else {
			ctxSP = foundIt->second;

			ctx = ctxSP.get (); eraseOnErr = true;
			s = ( channel == MEDIATOR_STUNT_CHANNEL_MAIN ? &ctx->sockI : &ctx->sockC );

			if ( now - ctx->registerTime > 10000 ) {
				// Reuse the context if the last registration attempt has been done more than 10 seconds before
				ctx->registerTime   = now;
				ctx->token          = token;
				success             = 1;

				CVerbsArg ( CONCURRENT_STUNT_LOGLEVEL, "[ %s ].RegisterStuntSocket: Reused context [ %u ] for [ %s ]!", getChannel (), token, key );
			}
			else {
				if ( ctx->token != token ) {
					// The mediator has higher priority and mediates which one determines the token
					if ( !isMediatorRequest ) {
						CWarnArg ( "[ %s ].RegisterStuntSocket: Tokens do not match [ %u != %u ]!", getChannel (), ctx->token, token );
					}
					else {
						CVerbsArg ( CONCURRENT_STUNT_LOGLEVEL, "[ %s ].RegisterStuntSocket: Overwrite old context [ %i ] with [ %i ] due to Mediator assignment for [ %s ]!", getChannel (), ctx->token, token, key );

						ctx->token = token;
						success = 1;
					}
				}
				else {
					// Otherwise (token is the same), then check whether we already have done a working stunt before (or about to do it)

					if ( IsInvalidFD ( *s ) ) {
						CVerbsArg ( CONCURRENT_STUNT_LOGLEVEL, "[ %s ].RegisterStuntSocket: Going to register [ %u ] for [ %s ]!", getChannel (), token, key );
						success = 1;
					}
					else {
						CWarnsArg ( CONCURRENT_STUNT_LOGLEVEL, "[ %s ].RegisterStuntSocket: Registration of [ %u ] for [ %s ] already done!", getChannel (), token, key );
						success = -1;
					}
				}
			}
		}

		if ( success <= 0 )
			goto Finalize;
		success = 0;

		// Create socket
		sock = ( int ) socket ( PF_INET, SOCK_STREAM, 0 ); // IPPROTO_TCP (using 0 and let the service provider choose the protocol)
		if ( IsInvalidFD ( sock ) ) {
			CErr ( "RegisterStuntSocket: Failed to create a socket!" );
			goto Finalize;
		}

		sockToClose = *s;
		*s = sock;

		LockReleaseA ( inst->stuntSocketLock, "RegisterStuntSocket" );
		locked = false;

		CSocketTraceAdd ( sock, "MediatorClient RegisterStuntSocket stuntSocket" );

		DisableSIGPIPE ( sock );

        if ( IsValidFD ( sockToClose ) ) {
			ShutdownCloseSocket ( sockToClose, true, "RegisterStuntSocket" );
		}

		if ( !req->Set ( med, ctxSP, s, addr, deviceID, channel, token ) ) {
			CVerbsArg ( 2, "RegisterStuntSocket: Failed to init context [ %u ].", token );
			goto Finalize;
		}

		value = 1;
		rc = setsockopt ( *s, SOL_SOCKET, SO_REUSEADDR, ( const char * ) &value, sizeof ( value ) );
		if ( rc < 0 ) {
            CErrArg ( "[ %s ].RegisterStuntSocket: Failed to set reuseAddr on stunt socket to mediator.", getChannel () );
            
            if ( IsValidFD ( *s ) ) { LogSocketErrorF ( "MediatorClient.RegisterStuntSocket" ); }
			goto Finalize;
		}

#if defined(SO_REUSEPORT)
		value = 1;
		rc = setsockopt ( *s, SOL_SOCKET, SO_REUSEPORT, ( const char * ) &value, sizeof ( value ) );
		if ( rc < 0 ) {
            CErr ( "RegisterStuntSocket: Failed to set reusePort on socket." );
            
            if ( IsValidFD ( *s ) ) { LogSocketErrorF ( "MediatorClient.RegisterStuntSocket" ); }
			//goto Finalize;
		}
#endif

		if ( invokeThread ) {
			if ( !req->Init ( env->mediator.lock () ) ) {
				CVerbs ( 2, "RegisterStuntSocket: Mediator is not ready." );
			}
			else {
                req->aliveSP = threadSP;

                if ( req->aliveSP )
                {
                    if ( LockAcquireA ( stuntThreadsLock, "RegisterStuntSocket" ) )
                    {
                        stuntThreads [ req ] = threadSP;

                        LockReleaseA ( stuntThreadsLock, "RegisterStuntSocket" );
                    }

                    if ( req->Run ( pthread_make_routine ( &MediatorClient::StuntThread ), req, "RegisterStuntSocket" ) )
                        success = 1;
                    else {
                        req->aliveSP.reset ();
                        CVerbsArg ( 2, "[ %s ].RegisterStuntSocket: Failed to start thread for [ %s ]", getChannel (), key );
                    }
                }
			}
		}
		else {
			if ( RegisterStuntSocketDo ( req ) )
				success = 1;
		}

	Finalize:
		if ( locked ) {
			LockReleaseA ( inst->stuntSocketLock, "RegisterStuntSocket" );
		}

		if ( !success && eraseOnErr && req->med )
			HandleStuntContextFailed ( req );

		return ( success != 0 );
	}


	void * MediatorClient::StuntThread ( void * arg )
	{
		if ( !arg ) return 0;

        RegisterStuntThread *	req			= ( RegisterStuntThread * ) arg;

        // Take over the threadSP
        sp ( RegisterStuntThread ) threadSP = req->aliveSP;

        req->aliveSP.reset ();

        // Keep an SP to safely access the mediator
        sp ( MediatorClient )	mediatorSP	= req->mediator;
        MediatorClient		*	mediator	= mediatorSP.get ();
        
        MediatorInstance	*	med			= req->med;
        
		if ( med && mediator->isRunning && med->enabled && mediator->env->environsState >= environs::Status::Starting ) {
			if ( !mediator->RegisterStuntSocketDo ( req ) )
			{
				HandleStuntContextFailed ( req );
			}
		}
		else {
			CVerbs ( 4, "StuntThread: Mediator is not ready." );
		}

		if ( LockAcquireA ( mediator->stuntThreadsLock, "StuntThread" ) )
		{
			msp ( void *, RegisterStuntThread )::iterator it = mediator->stuntThreads.find ( req );

			if ( it != mediator->stuntThreads.end () )
				mediator->stuntThreads.erase ( it );

			LockReleaseA ( mediator->stuntThreadsLock, "StuntThread" );
		}
		return 0;
	}


	void MediatorClient::StuntThreadsDispose ()
	{
		CVerbs ( 6, "StuntThreadsDispose" );

		vsp ( RegisterStuntThread ) threads;

		if ( LockAcquireA ( stuntThreadsLock, "StuntThreadsDispose" ) )
		{
			if ( stuntThreads.size () > 0 )
			{
				msp ( void *, RegisterStuntThread )::iterator it = stuntThreads.begin ();

				while ( it != stuntThreads.end () )
				{
					threads.push_back ( it->second );
					++it;
				}

				stuntThreads.clear ();
			}

			LockReleaseA ( stuntThreadsLock, "StuntThreadsDispose" );
		}

		vsp ( RegisterStuntThread )::iterator it = threads.begin ();

		while ( it != threads.end () )
		{
			RegisterStuntThread * stunt = ( *it ).get ();

            sp ( StuntRegisterContext ) ctxSP = stunt->ctxSP;
            if ( ctxSP )
            {
                int sock = ctxSP->sockC;
                if ( IsValidFD ( sock ) ) {
                    ShutdownCloseSocket ( sock, false, "StuntThreadsDispose comDat" );
                }

                sock = ctxSP->sockI;
                if ( IsValidFD ( sock ) ) {
                    ShutdownCloseSocket ( sock, false, "StuntThreadsDispose interact" );
                }
            }

			++it;
        }

        it = threads.begin ();

        while ( it != threads.end () )
        {
            RegisterStuntThread * stunt = ( *it ).get ();

            CVerbs ( 1, "StuntThreadsDispose: Waiting for stunt thread ..." );
            
            stunt->Join ( "StuntThreadsDispose" );
            
            ++it;
        }
	}


	bool MediatorClient::RegisterStuntSocketDo ( RegisterStuntThread * req )
	{
		CVerbs ( 6, "RegisterStuntSocketDo" );

		int                 rc;
		struct sockaddr     adr;
		int					success     = 0;
		unsigned int		toSendSize	= 0;
		int					bytesSent	= 0;
		unsigned int    *   pUInt		= 0;
		char			*	cipher		= 0;
		StuntSockRegPack *	regPack		= 0;

		char                channel     = req->channel;
		char            *   buffer      = req->buffer;
		char            *   toSend      = buffer;

		StuntSockRegTarget* regTarget   = ( StuntSockRegTarget * ) ( req->buffer + MEDIATOR_BROADCAST_SPARE_ID_LEN );

		// To take over
		int             *   s           = req->s;
		StuntRegisterContext * ctx      = req->ctxSP.get ();

		ThreadInstance * inst = &req->med->connection.instance;
		if ( IsInvalidFD ( inst->socket ) ) {
			CErr ( "RegisterStuntSocketDo: Socket to Mediator is not available!" );
			return false;
		}

		memcpy ( &adr, ( struct sockaddr * ) &inst->addr, sizeof ( struct sockaddr ) );

		rc = Connect ( 0, *s, &adr, 30, "Stunt socket reg." );
		if ( rc != 0 ) {
			CErrArg ( "[ %s ].RegisterStuntSocketDo: Failed connecting stunt socket to mediator [ %s : %d ]", getChannel (), inet_ntoa ( inst->addr.sin_addr ), req->med->port );
            
            if ( IsValidFD ( *s ) ) { LogSocketErrorF ( "MediatorClient.RegisterStuntSocketDo" ); }
			goto Finalize;
		}

		memcpy ( buffer, broadcastMessage, MEDIATOR_BROADCAST_SPARE_ID_LEN );

		regTarget->deviceID     = req->deviceID;
		regTarget->channelType  = channel;
		regTarget->token        = req->token;
		regTarget->size         = sizeof ( StuntSockRegTarget ) + regTarget->sizes [ 0 ] + regTarget->sizes [ 1 ];

		toSendSize = MEDIATOR_BROADCAST_SPARE_ID_LEN + regTarget->size;

		pUInt  = reinterpret_cast<unsigned int *>( buffer );
		*pUInt = toSendSize;

		if ( inst->encrypt ) {
			if ( !certificate ) {
				CErrArg ( "[ %s ].RegisterStuntSocketDo: No Mediator certificate available!", getChannel () ); goto Finalize;
			}

			if ( !AESEncrypt ( &inst->aes, buffer, &toSendSize, &cipher ) ) {
				CErrArg ( "[ %s ].RegisterStuntSocketDo: AES encrypt failed!", getChannel () ); goto Finalize;
			}

			/// (4) size, (4) req ident, (4) req pack length, (4) stuntID, ... req...
#ifdef NDEBUG
			regPack = ( StuntSockRegPack * ) malloc ( toSendSize + 24 + ( *( ( unsigned int * ) certificate ) & 0xFFFF ) );
#else
			regPack = ( StuntSockRegPack * ) calloc ( 1, toSendSize + 24 + ( *( ( unsigned int * ) certificate ) & 0xFFFF ) );
#endif
            if ( !regPack ) {
                CErrArg ( "[ %s ].RegisterStuntSocketDo: Memory allocation failed!", getChannel () ); goto Finalize;
            }
            
            regPack->sizeReq = 16;
            
            memcpy ( regPack->ident, MEDIATOR_STUNT_SOCKET_REQ, 4 );
            
            regPack->sessionID = req->med->connection.instance.sessionID;
            
            regPack->sizePayload = toSendSize;
            memcpy ( &regPack->payload, cipher, toSendSize );
            
            toSendSize += 24;
            
            if ( !EncryptMessage ( 0, certificate, ( char * ) &regPack->sizeReq, &toSendSize ) || !toSendSize ) {
                CErrArg ( "[ %s ].RegisterStuntSocketDo: Failed to encrypt response.", getChannel () ); goto Finalize;
            }
            
            toSendSize += 4;
            regPack->sizeEncrypted = toSendSize | 0x80000000;
            
            CVerbVerbArg ( "[ %s ].RegisterStuntSocketDo: Send packet of size [ %d ]  ...", getChannel (), toSendSize );
            
            toSend = ( char * ) &regPack->sizeEncrypted;
        }
        
        CVerbVerbArg ( "[ %s ].RegisterStuntSocketDo: Send packet of size [ %d ]  ...", getChannel (), toSendSize );
        
        if ( !SocketTimeout ( *s, 6, 6 ) ) {
            goto Finalize;
        }
        
        bytesSent = ( int ) send ( *s, toSend, toSendSize, MSG_NOSIGNAL );
        if ( bytesSent != ( int ) toSendSize ) {
            CErrArg ( "[ %s ].RegisterStuntSocketDo: Sending of handshake failed [ %i != %i ]", getChannel (), bytesSent, toSendSize );
            
            if ( IsValidFD ( *s ) ) { LogSocketErrorF ( "MediatorClient.RegisterStuntSocketDo" ); }
            goto Finalize;
        }
        else {
            // Wait 4 seconds for response
            
            if ( !req->med->enabled || env->environsState < environs::Status::Starting ) {
                CVerbs ( 5, "RegisterStuntSocketDo: Mediator is not ready." );
                goto Finalize;
            }

            int bytes; bool repeated = false;
            
        Retry:
            bytes = ( int ) recv ( *s, buffer, MEDIATOR_BROADCAST_SPARE_ID_LEN, 0 );
            if ( bytes < 4 )
            {
                if ( !repeated ) {
                    SOCKET_Check_Val ( check );
                    
                    if ( SOCKET_Check_Retry ( check ) ) {
                        if ( IsValidFD ( *s ) ) {
                            CVerbsArg ( 1, "[ %s ].RegisterStuntSocketDo: Receive timed out. Try again once ...", getChannel () );
                            repeated = true;
                            goto Retry;
                        }
                    }
                }
                
                CWarnsArg ( 1, "[ %s ].RegisterStuntSocketDo: Failed to receive response [ %i != 4 ]", getChannel (), bytes );
                
                if ( IsValidFD ( *s ) ) { LogSocketErrorNOK (); }
            }
            
            socklen_t length = sizeof ( struct sockaddr );
            
            struct sockaddr_in * a = ( channel == MEDIATOR_STUNT_CHANNEL_MAIN ? &ctx->addrI : &ctx->addrC );
            
            int ret = getsockname ( *s, ( struct sockaddr * ) a, &length );
            if ( ret )
                goto Finalize;
            
            CVerbsArg ( CONCURRENT_STUNT_LOGLEVEL, "[ %s ].RegisterStuntSocketDo: [ %s : %d ]", getChannel (), inet_ntoa ( a->sin_addr ), ntohs ( a->sin_port ) );
            
            if ( req->addr )
                *req->addr = *a;
            
            success = 1;
            
            SocketTimeout ( *s, 0, 0 );
        }
        
    Finalize:
        free_n ( cipher );
        free_n ( regPack );
        
        return ( success != 0 );
    }


	bool MediatorClient::SecureChannelAuth ( MediatorInstance * med )
	{
		CVerb ( "SecureChannelAuth" );

		bool	ret = false;

		/// We support certificates up to a size of 2k
		char				certBuffer [ ENVIRONS_MAX_KEYBUFFER_SIZE ];

		unsigned int	*	pUI = ( unsigned int * ) certBuffer;

		char				buffer [ MEDIATOR_SECURE_CHANNEL_BUFFER_SIZE ];

		unsigned int		instructOrSize;
		unsigned int		hashLen			= 0;

		unsigned int		challenge;
		unsigned int		certSize;
		char			*	cert;
		char			*	hash		= 0;
		char			*	cipher		= 0;
		int					msgSize     = 0;
        bool				passFailed = false;
        
        ThreadInstance  *   inst        = &med->connection.instance;

#ifndef NDEBUG
		// Initialize buffer for valgrind ...
		// Acutally it is intended to send a few more (uninitialized padding) bytes to the target.
		// The target access only specified areas.
		Zero ( buffer );
#endif
		do
		{
			med->connection.instance.encrypt = 0;

			/// At first, send a tls request
			memcpy ( certBuffer + 4, "HCLS", 4 );
			*pUI = 8;
            
            if ( !SendBufferNoQueue ( inst, certBuffer, 8 ) ) {
                CErr ( "SecureChannelAuth: Send HCM CLS request failed." ); break;
            }

            char *			recBuffer		= certBuffer;
            unsigned int	recBufferSize	= sizeof ( certBuffer );
            int				bytesReceived	= 0;
            int             msgLength       = 0;
            int             minRec          = 16;

        ReceiveNext:
			bytesReceived = Receive ( med->connection.instance.socket, recBuffer, recBufferSize, minRec, "SecureChannelAuth" );
			if ( bytesReceived <= 0 ) {
				CVerbArg ( "SecureChannelAuth: Socket [ %i ] closed by someone; Bytes [ %i ]!", med->connection.instance.socket, bytesReceived ); break;
			}

            msgSize += bytesReceived;

            if ( !msgLength ) {
                msgLength = *((unsigned int *) (certBuffer + 12)); // Length of the certificate
                msgLength += 16;

                if ( msgLength >= DEVICE_HANDSHAKE_BUFFER_MAX_SIZE ) {
                    CErrArg ( "SecureChannelAuth: Message size [ %u ] would overflow receive buffer. Aborting transfer.", msgLength );
                    break;
                }
            }

            if ( msgSize < msgLength ) {
                CVerb ( "SecureChannelAuth: Received message is not complete." );

                recBuffer += bytesReceived;
                recBufferSize -= bytesReceived;

                minRec = 0;
                goto ReceiveNext;
            }

			/// Get the instruction (4 bytes: 1234 means multiply with 42) and random number (4 bytes)
			instructOrSize = *pUI++;

			/// Grab the challenge
			challenge = *pUI++;
			certSize = *pUI++; pUI++;
			cert = ( char * ) pUI;

			if ( certSize + 12 > ( unsigned int ) msgSize ) {
				CErr ( "SecureChannelAuth: Potential challenge received would overflows receive buffer." ); break;
			}

			if ( instructOrSize != 1234 ) {
				CErrArg ( "SecureChannelAuth: I don't understand the challenge instruction [ %u ].", instructOrSize ); break;
			}

			unsigned int	responseLen = 0;
			char		*	userName	= 0;
			unsigned int	nameLen		= 0;
			char		*	pass		= 0;

			challenge *= 42;
			if ( env->useAuth )
			{
				if ( *env->UserPassword && *env->UserName ) {
					nameLen = ( unsigned int ) strlen ( env->UserName );
					if ( !nameLen ) {
						CErr ( "SecureChannelAuth: Authentication required, but no user/pass available." ); break;
					}

					userName = env->UserName;
					pass = env->UserPassword;
					CVerb ( "SecureChannelAuth: Using user/pass." );
				}
				else {
					nameLen = ( unsigned int ) strlen ( native.deviceUID );
					if ( !nameLen ) {
						CErr ( "SecureChannelAuth: Authentication required, but deviceUID invalid." ); break;
					}
					userName = native.deviceUID;

					pass = ( med == &this->mediator ? env->DefaultMediatorToken : env->CustomMediatorToken );

					CVerbArg ( "SecureChannelAuth: Using deviceUID/authToken for %s Mediator.", med == &this->mediator ? "default" : "custom" );

					if ( !*userName || !*pass ) {
						CErr ( "SecureChannelAuth: Missing user/pass and deviceUID/authToken." );

						if ( env->UserPassword [ 1 ] != 1 ) {
							env->UserPassword [ 1 ] = 1;
							API::onEnvironsNotifier1 ( env, NOTIFY_MEDIATOR_SERVER_PASSWORD_MISSING );
						}
						break;
					}
				}
			}

			CVerbVerbArg ( "SecureChannelAuth: Auth user[%s] pass[%s]", userName, ConvertToHexSpaceString ( pass, 64 ) );

			if ( !BuildEnvironsResponse ( challenge, userName, pass, &hash, &hashLen ) || !hash ) {
				CErr ( "SecureChannelAuth: Failed to build response with user/pass." ); break;
			}
			CVerbVerbArg ( "SecureChannelAuth: Response [%s]", ConvertToHexSpaceString ( hash, hashLen ) );

			unsigned int lenPad = GetPad ( nameLen, MEDIATOR_MESSAGE_UNIT_ALIGN );
			if ( lenPad )
				AddBit ( nameLen, MEDIATOR_MESSAGE_LENGTH_FLAG_PAD );

			pUI = ( unsigned int * ) ( buffer + 4 );

			*pUI = MEDIATOR_PROTOCOL_VERSION; pUI++;

			*pUI = nameLen; pUI++;
			instructOrSize = 12;

			if ( nameLen ) {
				ClearBit ( nameLen, MEDIATOR_MESSAGE_LENGTH_FLAG_PAD );

				memcpy ( pUI, userName, nameLen );
				instructOrSize += nameLen + lenPad;

				pUI = ( unsigned int * ) ( buffer + instructOrSize );
			}

			*pUI = hashLen; pUI++;

			memcpy ( pUI, hash, hashLen );

			*( ( unsigned int * ) cert ) |= env->CLSPadding;

			responseLen = hashLen + instructOrSize;
			if ( !EncryptMessage ( 0, cert, buffer + 4, &responseLen ) || !responseLen ) {
				CErr ( "SecureChannelAuth: Failed to encrypt response." ); break;
			}

			if ( !certificate )
			{
				/// Cache Mediator certificate
				LockAcquireVA ( localNetsLock, "SecureChannelAuth" );

				if ( !certificate ) 
				{
					certSize = *( ( unsigned int * ) cert ) & 0xFFFF;

					char * certNew = ( char * ) malloc ( certSize + 4 );
					if ( !certNew ) {
						CErr ( "SecureChannelAuth: Memory allocation for certificate failed." );
						LockReleaseVA ( localNetsLock, "SecureChannelAuth" ); 
						break;
					}
					memcpy ( certNew, cert, certSize + 4 );

					certificate = certNew;
				}

				LockReleaseVA ( localNetsLock, "SecureChannelAuth" );
			}

			/// Send our handshake/response/challenge
			pUI = reinterpret_cast<unsigned int *>( buffer );
			*pUI = responseLen + 4;

			passFailed = true;
            
            if ( !SendBufferNoQueue ( inst, buffer, *pUI ) ) {
                CErr ( "SecureChannelAuth: Send of response failed." ); break;
            }

            recBuffer		= buffer;
            recBufferSize	= MEDIATOR_SECURE_CHANNEL_BUFFER_SIZE;
            msgLength       = 0;
            msgSize         = 0;
            minRec          = 4;

        ReceiveNextAuth:
			bytesReceived = Receive ( med->connection.instance.socket, recBuffer, recBufferSize, minRec, "SecureChannelAuth" );
			if ( bytesReceived <= 0 ) {
				CVerbArg ( "SecureChannelAuth: connection/socket [ %i ] closed by someone; Bytes [ %i ]!", med->connection.instance.socket, bytesReceived ); break;
			}

            msgLength += bytesReceived;

            if ( !msgSize ) {
                unsigned int * pUI1 = reinterpret_cast<unsigned int *>( buffer );

                msgSize = *pUI1;
                msgSize &= 0xFFFFFF;

                if ( msgSize >= MEDIATOR_SECURE_CHANNEL_BUFFER_SIZE ) {
                    CErrArg ( "SecureChannelAuth: Message size [ %u ] would overflow receive buffer. Aborting transfer.", msgSize );
                    break;
                }
            }

            if ( msgLength < msgSize ) {
                CVerb ( "SecureChannelAuth: Received message is not not complete." );

                recBuffer += bytesReceived;
                recBufferSize -= bytesReceived;
                minRec = 0;
                goto ReceiveNextAuth;
            }

			AESDisposeKeyContext ( &med->connection.instance.aes );

			if ( !AESDeriveKeyContext ( hash, hashLen, &med->connection.instance.aes ) ) {
				CErr ( "SecureChannelAuth: Failed to derive AES session keys." ); break;
			}

			/// Make sure that we have the correct size and the correct encoded package
			pUI = reinterpret_cast<unsigned int *>( buffer );

			unsigned int rawSize = *pUI;
			rawSize &= 0xFFFFFF;
			if ( ( int ) rawSize < msgSize ) {
				CWarnArg ( "SecureChannelAuth: Received more bytes [ %i ] than expected [ %u ] for now.", msgSize, rawSize );
				msgSize = rawSize;
			}

			if ( !AESDecrypt ( &med->connection.instance.aes, buffer, ( unsigned int * ) &msgSize, &cipher ) || !cipher ) {
				CErr ( "SecureChannelAuth: Failed to decrypt AES message." ); break;
			}
			med->connection.instance.encrypt = 1;

			if ( msgSize == 4 && cipher [ 0 ] == 'a' && cipher [ 1 ] == 'e' && cipher [ 2 ] == ';' && cipher [ 3 ] == ';' ) {
				ret = true; passFailed = false;
				med->connection.instance.authenticated = true;
			}
		}
		while ( 0 );

        free_n ( hash );
        free_n ( cipher );

		if ( passFailed ) {
			if ( env->UserPassword [ 1 ] != 1 ) {
				env->UserPassword [ 1 ] = 1;
				API::onEnvironsNotifier1 ( env, NOTIFY_MEDIATOR_SERVER_PASSWORD_FAIL );
			}
			else // reset notification sent flag. allow subsequent notification.
				env->UserPassword [ 1 ] = 0;

			char * tok = ( med == &this->mediator ? env->DefaultMediatorToken : env->CustomMediatorToken );

			CVerbArg ( "SecureChannelAuth: Invalidating deviceUID/authToken for  %s Mediator.", med == &this->mediator ? "default" : "custom" );

			*tok = 0;
		}

		return ret;
	}


	void MediatorClient::SetFilterMode ( MediatorInstance * med )
	{
		MediatorMsg msg;
		Zero ( msg );

		msg.size = 12;
		msg.cmd0 = MEDIATOR_PROTOCOL_VERSION;
		msg.cmd1 = MEDIATOR_CMD_SET_FILTERMODE;
		msg.opt0 = MEDIATOR_OPT_NULL;
		msg.opt1 = MEDIATOR_OPT_NULL;
		msg.ids.id2.msgID = env->mediatorFilterLevel;
        
        if ( !SendBufferNoQueue ( &med->connection.instance, ( char * ) &msg, msg.size ) ) {
            CErr ( "SetFilterMode: Setting filtermode failed." );
        }
	}


    void MediatorClient::SetNotificationSubscription ( bool enable )
    {
        bool stateBefore = subscribedToNotifications;
        if ( stateBefore == enable )
            return;
        CLogArg ( "SetNotificationSubscription: [ %s ]", enable ? "enabled" : "disabled" );

        subscribedToNotifications = enable;

        if ( !isRunning )
            return;

        if ( !stateBefore ) {
            DevicesHasChanged ( MEDIATOR_DEVICE_RELOAD );
        }

        MediatorMsg msg;
        Zero ( msg );

        msg.size = 12;
        msg.cmd0 = MEDIATOR_PROTOCOL_VERSION;
        msg.cmd1 = MEDIATOR_CMD_NOTIFICATION_SUBSCRIBE;
        msg.opt0 = MEDIATOR_OPT_NULL;
        msg.opt1 = MEDIATOR_OPT_NULL;
        msg.ids.id2.msgID = (enable ? 1 : 0);
        
        PushSend ( &msg, 12 );
    }


    bool MediatorClient::GetNotificationSubscription ()
    {
        return subscribedToNotifications;
    }


    void MediatorClient::SetMessagesSubscription ( bool enable )
    {
        bool stateBefore = subscribedToMessages;
        if ( stateBefore == enable )
            return;

        CLogArg ( "SetMessagesSubscription: [ %s ]", enable ? "enabled" : "disabled" );

        subscribedToMessages = enable;

        if ( !isRunning )
            return;

        MediatorMsg msg;
        Zero ( msg );

        msg.size = 12;
        msg.cmd0 = MEDIATOR_PROTOCOL_VERSION;
        msg.cmd1 = MEDIATOR_CMD_NOTIFICATION_SUBSCRIBE;
        msg.opt0 = MEDIATOR_OPT_NULL;
        msg.opt1 = MEDIATOR_OPT_NULL;
        msg.ids.id2.msgID = (enable ? 1 : 0);
        
        PushSend ( &msg, 12 );
    }


    bool MediatorClient::GetMessagesSubscription ()
    {
        return subscribedToMessages;
    }



	/**
	* Populate flags to the mediator daemon.
	*
	* @param	device	privdes flags to set or clear. (of type DeviceFlagsInternal::Observer*)
	* @param	set		true = set, false = clear.
	*/
	void MediatorClient::SetDeviceFlags ( DeviceInstanceNode * device, bool set )
	{
		CVerbsArg ( 6, "SetDeviceFlags: [ %s ]", set ? "set" : "clear" );

		if ( !isRunning || !device )
            return;

		// Send through udp only if the device is not only managed by a Mediator server
		// or not behind a NAT
		if ( device->info.broadcastFound != DEVICEINFO_DEVICE_MEDIATOR || device->info.ip == device->info.ipe )
			SyncDeviceUpdateFlags ( device, set );

        MediatorInstance * med = GetAvailableMediator ();
		if ( med )
		{
			MediatorStatusMsgExt raw;
            Zero ( raw );

            MediatorStatusMsg * msg = (MediatorStatusMsg *) &raw;

			msg->cmd0 = MEDIATOR_PROTOCOL_VERSION;
			msg->cmd1 = MEDIATOR_CMD_DEVICE_FLAGS;

            msg->status1 = ( int ) ( device->info.flags & ( unsigned short ) (DeviceFlagsInternal::NotifyMask | DeviceFlagsInternal::CPNotifyMask) );
			msg->status2 = set;

            msg->deviceID    = device->info.deviceID;
            msg->sizes [ 0 ] = 1;
            msg->sizes [ 1 ] = 1;

            CVerbsArg ( 6, "SetDeviceFlags: objID [ %i ]\tFlags [ 0x%X ]\t[ %s ]", device->info.objID, msg->status1, set ? "set" : "clear" );

			if ( env->mediatorFilterLevel <= MediatorFilter::Area )
			{
                if ( *device->info.areaName && *device->info.appName && !BuildAppAreaField ( msg->sizes, device->info.appName, device->info.areaName, false ) ) {
                    CVerb ( "SetDeviceFlags: Skipping flags update due to Error in app/area." );
                    return;
                }
			}
			msg->size = sizeof ( MediatorStatusMsg ) + msg->sizes [ 0 ] + msg->sizes [ 1 ];
            
            if ( !PushSend ( &med->connection.instance, msg, msg->size, 0 ) ) {
                CVerb ( "SetDeviceFlags: Flags update to Mediator failed." );
            }
		}
        else { CVerb ( "SetDeviceFlags: Skipping flags update due to Mediator service not available." ); }
    }


	bool MediatorClient::RegisterAuth ( MediatorInstance * med )
	{
		CVerb ( "RegisterAuth: Sending handshake" );

		char buffer [ sizeof ( broadcastMessage ) + sizeof ( native.deviceUID ) ];
		int bytes = 0;
		unsigned int * pUI = ( unsigned int * ) buffer;

		if ( !env->useCLS ) {
			if ( !env->useAuth ) {

				med->connection.instance.encrypt = 0;
				goto AuthenticationOK;
			}

			CErr ( "RegisterAuth: Authentication without an enabled crypto transport layer is not supported." ); return false;
		}

		if ( !med->connection.instance.authenticated ) {
			if ( ___sync_val_compare_and_swap ( &secureChannelAuth, 0, 1 ) != 0 )
			{
				CErr ( "RegisterAuth: An authentication is ongoing ..." );
				return false;
			}

			bool success = SecureChannelAuth ( med );

			secureChannelAuth = 0;

			if ( !success ) {
				CErr ( "RegisterAuth: Secure Channel, Challenge/Response failed." );
				return false;
			}
		}

	AuthenticationOK:
		if ( !env->deviceID ) {
			env->deviceID = RequestDeviceID ( med );
			if ( env->deviceID ) {
				environs::API::SetDeviceID ( env->hEnvirons, env->deviceID );
			}
			else {
				CErr ( "RegisterAuth: Failed to retrieve a DeviceID." );
				return false;
			}
		}

        BuildBroadcastMessage ( false );

		bytes = broadcastMessageLenExt;
		memcpy ( buffer, broadcastMessage, bytes );

		buffer [ bytes ] = 0;
		*pUI = bytes;
        
        if ( !SendBufferNoQueue ( &med->connection.instance, buffer, bytes ) ) {
            CErr ( "RegisterAuth: Handshake failed." ); return false;
        }

		/// Notify the app about this event?

		CVerb ( "RegisterAuth: successfull." );
		return true;
	}


	bool MediatorClient::CheckCertificateAndKey ( MediatorInstance * med )
	{
		bool ret = false;
		char * signReq = 0;

		if ( !opt_privKey || !opt_pubCert ) {
			CVerb ( "CheckCertificateAndKey: Requesting self-signed certificate and key from Mediator." );

			char keyReq [ 20 ];
			memcpy ( keyReq + 4, "3ts;", 4 );
			keyReq [ 4 ] = MEDIATOR_PROTOCOL_VERSION;
			keyReq [ 5 ] = MEDIATOR_CMD_HELP_TLS_GEN;

			unsigned int * pUI = reinterpret_cast<unsigned int *>( keyReq );
			*pUI = 8;
            
            SendBufferNoQueue ( &med->connection.instance, keyReq, 8 );
			ret = true;
		}
		else
			if ( !( *( ( unsigned int * ) opt_pubCert ) & ENVIRONS_CERT_SIGNED_FLAG ) )
			{
				CVerb ( "CheckCertificateAndKey: Requesting certificate signature from Mediator." );
				do
				{
					unsigned int certSize = *( ( unsigned int * ) opt_pubCert ) & 0xFFFF;
					
					// Code analysis seem to allow size_t to be negative
					if ( certSize <= 0 )
						break;
					certSize += 4;

					signReq = ( char * ) malloc ( certSize + 8 );
					if ( !signReq )
						break;

					memcpy ( signReq + 4, "3ts;", 4 );
					signReq [ 4 ] = MEDIATOR_PROTOCOL_VERSION;
					signReq [ 5 ] = MEDIATOR_CMD_HELP_TLS_GEN;

					memcpy ( signReq + 8, opt_pubCert, certSize );

					*( ( unsigned int * ) signReq ) = certSize + 8;
                    
                    SendBufferNoQueue ( &med->connection.instance, signReq, certSize + 8 );

					ret = true;
				}
				while ( 0 );
			}
			else ret = true;

            free_n ( signReq );
			return ret;
	}


	void MediatorClient::HandleCertificateResponse ( MediatorMsg * msg, unsigned int msgSize )
	{
		if ( msg->opt0 == MEDIATOR_OPT_NULL ) {
			CVerb ( "HandleCertificateResponse: Taking over signed certificate/keys from Mediator." );
			/// update the certificate/private key
			char * cert = 0;
			char * key = 0;

			unsigned int certLen = ( msg->ids.id2.msgID & 0xFFFF ) + 4;
			if ( ( certLen + 10 ) >= msgSize ) {
				CErrArg ( "HandleCertificateResponse: Invalid size of certificate [ %u ].", certLen );
				return;
			}

			unsigned int keyLen = ( *( ( unsigned int* ) ( ( ( char * ) msg ) + 8 + certLen ) ) & 0xFFFF ) + 4;

			if ( certLen + keyLen + 8 <= msgSize )
			{
				do
				{
					cert = ( char * ) malloc ( certLen );
					if ( !cert )
						break;
					key = ( char * ) malloc ( keyLen );
					if ( !key )
						break;

					memcpy ( cert, &msg->ids.id2.msgID, certLen );
					memcpy ( key, ( ( char * ) &msg->ids.id2.msgID ) + certLen, keyLen );

					if ( !SavePrivateBinary ( BuildDataStorePath ( ENVIRONS_PUBLIC_CERTNAME ), cert, certLen ) ) {
						CErr ( "HandleCertificateResponse: Failed to privately store RSA public certificate." ); break;
					}

					if ( !SavePrivateBinary ( BuildDataStorePath ( ENVIRONS_PRIVATE_KEYNAME ), key, keyLen ) ) {
						CErr ( "HandleCertificateResponse: Failed to privately store RSA private key." ); break;
					}

                    if ( UpdateKeyAndCert ( key, cert ) ) {
                        free_n ( opt_pubCert );
                        free_n ( opt_privKey );

						opt_pubCert = cert;
						opt_privKey = key;
					}
					cert	= 0;
					key		= 0;
					CVerb ( "HandleCertificateResponse: Updated cert/key with help of the Mediator." );
				}
				while ( 0 );

                free_n ( cert );
                free_n ( key );
			}
			return;
		}

		if ( msg->opt0 == MEDIATOR_OPT_SIGNED_CERT_RESPONSE ) {
			/// update the (signed) certificate
			CVerb ( "HandleCertificateResponse: Taking over signed certificate from Mediator." );
			char * cert = 0;

			//unsigned int certLen = (*((unsigned int*) (msg + 8)) & 0xFFFF) + 4;
			unsigned int certLen = ( msg->ids.id2.msgID & 0xFFFF ) + 4;

			if ( certLen + 4 <= msgSize )
			{
				do
				{
					cert = ( char * ) malloc ( certLen );
					if ( !cert )
						break;

					memcpy ( cert, &msg->ids.id2.msgID, certLen );

					if ( !SavePrivateBinary ( BuildDataStorePath ( ENVIRONS_PUBLIC_CERTNAME ), cert, certLen ) ) {
						CErr ( "HandleCertificateResponse: Failed to privately store RSA public certificate." ); break;
					}

                    free_n ( opt_pubCert );

					opt_pubCert = cert;
					cert		= 0;
					CVerb ( "HandleCertificateResponse: Updated signed cert with help of the Mediator." );
				}
				while ( 0 );

                free_n ( cert );
			}
		}
	}


	bool MediatorClient::IsServiceAvailable ()
	{
		CVerb ( "IsServiceAvailable" );

		return ( (env->useDefaultMediator || env->useCustomMediator) && ( GetAvailableMediator () != NULL ) );
	}


	bool MediatorClient::IsRegistered ()
	{
		CVerb ( "IsRegistered" );

		return ( GetAvailableMediator () != NULL );
    }


#ifdef MEDIATOR_USE_SOCKET_BUFFERS_APPLY_AT_CLIENT

    void ApplySocketBufferSizes ( int sock, MediatorMsg * msg )
    {
        CVerbArg ( "ApplySocketBufferSizes [ %i ]", sock );

        int rc;
        int * pUI = reinterpret_cast<int *>( msg );
        pUI += 2;

        int recSize = *pUI; pUI++;
        int sendSize = *pUI;

        if ( sendSize > 32000 && sendSize < 2000000 )
        {
            rc = setsockopt ( sock, SOL_SOCKET, SO_RCVBUF, ( const char * ) &sendSize, sizeof ( sendSize ) );
            if ( rc < 0 ) {
                CErrArg ( "ApplySocketBufferSizes [ %i ]: Failed to set receive buffer size.", sock ); LogSocketError ();
            }
            else {
                CVerbArg ( "ApplySocketBufferSizes [ %i ]: Receive buffer size set to [ %i ].", sock, sendSize );
            }
        }

        if ( recSize > 32000 && recSize < 2000000 )
        {
            rc = setsockopt ( sock, SOL_SOCKET, SO_SNDBUF, ( const char * ) &recSize, sizeof ( recSize ) );
            if ( rc < 0 ) {
                CErrArg ( "ApplySocketBufferSizes [ %i ]: Failed to set send buffer size.", sock ); LogSocketError ();
            }
            else {
                CVerbArg ( "ApplySocketBufferSizes [ %i ]: Send buffer size set to [ %i ].", sock, recSize );
            }
        }
    }

#endif


    void * MediatorClient::MediatorListenerStarter ( void * arg )
    {
        if ( !arg ) {
            CErr ( "MediatorListenerStarter: Invalid (NULL) argument." );
        }
        else {
			sp ( Instance )		envSP;

            MediatorInstance *	inst	= ( MediatorInstance * ) arg;

			MediatorClient	*	client	= ( MediatorClient * ) inst->mediatorObject;

            if ( !client ) {
                CErr ( "MediatorListenerStarter: No mediator instance available." );
            }
            else
            {
				envSP = client->envSP;
				if ( envSP )
				{
					IncreaseThreadPriority ( "MediatorListenerStarter" );

#ifdef MEDIATOR_USE_TCP_NODELAY
					int value = 1;
					int ret = setsockopt ( inst->connection.instance.socket, IPPROTO_TCP, TCP_NODELAY, ( const char * ) &value, sizeof ( value ) );
					if ( ret < 0 ) {
						CErr ( "MediatorListenerStarter: Failed to set TCP_NODELAY on socket" ); LogSocketError ();
					}
#endif
					// Execute thread
					client->MediatorListener ( arg );
				}
            }

			inst->connection.instance.thread.Notify ( "MediatorListenerStarter" );

            inst->connection.instance.thread.Detach ( "MediatorListenerStarter" );
        }

		CVerbs ( 1, "MediatorListenerStarter: bye bye ..." );

        return 0;
    }
    
    
    /*
     * Note: This thread must not acquire mediatorLock as this may lead to a deadlock
     * when ReleaseMediator waits for this thread to be terminated.
     */
	void * MediatorClient::MediatorListener ( void * arg )
	{
		CVerb ( "MediatorListener started..." );

		pthread_setname_current_envthread ( "MediatorClient::MediatorListener" );

		MediatorInstance	*	med             = ( MediatorInstance * ) arg;
		MediatorConnection	*	con             = &med->connection;
        ThreadInstance      *   client          = &con->instance;

		MediatorMsg			*	msg;
		int						bytesReceived;
		char				*	buffer          = con->buffer;

		unsigned int            msgLength       = 0;
		unsigned int            msgDecLength;
		unsigned int			msgInnerLength;
		char                *   decrypted       = 0;
        int                     bytesInBuffer   = 0;
        char                *   currentPtr      = buffer;
        unsigned int            remainingSize   = MEDIATOR_REC_BUFFER_SIZE_MAX - 1;

        char                *   startOfCurrentMessage   = buffer;

        int                 *   sock;
        int                     socki;

#ifndef USE_MEDIATOR_CLIENT_WINSOCK_SOCKETS
        struct pollfd desc;
        desc.events = POLLIN | POLLERRMASK;
        int rc;
#endif
        
		sock = &client->socket;
		if ( IsInvalidFD ( *sock )) {
			// Remove this mediator instance from list?
			goto Finish;
        }
        CVerbVerbArg ( "MediatorListener started for socket [ %i ]", *sock );

		if ( !RegisterAuth ( med ) ) {
			CWarn ( "MediatorListener: Registration / Authentication failed." );
			goto Finish;
        }
        
		CVerb ( "MediatorListener: listening..." );

        // Send started signal
        med->listening = true;

		/// Set filter mode
        SetFilterMode ( med );

        med->enabled = true;
        
        client->thread.Notify ( "MediatorListener" );

        if ( !InformMediator ( med ) ) {
            CWarn ( "MediatorListener: InformMediator failed." );
            goto Finish;
        }

		if ( !CheckCertificateAndKey ( med ) ) {
			CWarn ( "MediatorListener: Check of our certificate for signature failed." );
			goto Finish;
        }
        
#ifdef USE_MEDIATOR_CLIENT_WINSOCK_SOCKETS
        if ( WSAEventSelect ( *sock, client->receiveEvent, FD_READ | FD_CLOSE ) == SOCKET_ERROR )
        {
            CErrArg ( "MediatorListener: Failed to register receive event [ %d ]!", WSAGetLastError () );
            goto Finish;
        }
#else
        if ( !SetNonBlockSocket ( *sock, true, "MediatorListener" ) )
            goto Finish;
#endif
		DevicesHasChanged ( MEDIATOR_DEVICE_RELOAD );

		API::onEnvironsNotifier1 ( env, NOTIFY_MEDIATOR_SERVER_CONNECTED );
		registerFails = 0;

		while ( isRunning ) 
        {
            bytesReceived = ( int ) recv ( *sock, currentPtr, remainingSize, 0 );
            if ( bytesReceived == 0 ) {
                LogSocketErrorF ( "MediatorClient.MediatorListener" );
                CLogArg ( "MediatorListener: connection/socket [ %i ] closed by someone; Bytes [ %i ]!", *sock, bytesReceived );
                break;
            }
            
            if ( bytesReceived < 0 )
            {
                SOCKET_Check_Val ( check );
                
                if ( SOCKET_Check_Retry ( check ) )
                {
#ifdef USE_MEDIATOR_CLIENT_WINSOCK_SOCKETS
					WSAWaitForMultipleEvents ( 1, &client->receiveEvent, FALSE, WSA_INFINITE, FALSE );

					WSAResetEvent ( client->receiveEvent );

					if ( IsInvalidFD ( *sock ) )
						break;
#else
				Retry:
                    desc.fd         = *sock;
                    desc.revents    = 0;
                    
                    if ( IsInvalidFD ( *sock ) )
                        break;
                    
                    rc = poll ( &desc, 1, 10000 );
                    
                    if ( !isRunning )
                        break;
                    
                    if ( rc == -1 || (desc.revents & POLLERRMASK) ) {
                        CVerb ( "MediatorListener: Socket has been closed" );
                        LogSocketErrorF ( "MediatorClient.MediatorListener" );
                        break;
                    }
                    
                    if ( rc == 0 )
                        goto Retry;
#endif
					continue;
                }
                break;
            }
            
            currentPtr [ bytesReceived ] = 0;

            currentPtr += bytesReceived;

            remainingSize -= bytesReceived;

            CListenerLogArg ( 7, "MediatorListener: <--- [ %8i ] bytes - Free space in buffer [ %8i ] bytes", bytesReceived, remainingSize );

            bytesInBuffer += bytesReceived;
			while ( bytesInBuffer >= 8 )
            {
                CListenerLogArg ( 6, "MediatorListener: <--- [ %8i ] bytesInBuffer", bytesInBuffer );

				msgLength = *( ( unsigned int * ) startOfCurrentMessage );

                if ( ( msgLength & 0xFFFFFFF ) > ( unsigned int ) bytesInBuffer ) {
                    CListenerLogArg ( 6, "MediatorListener: <--- [ %8i ] msgLength > [ %8i ] bytesInBuffer", msgLength & 0xFFFFFFF, bytesInBuffer );
                    con->longReceive = true;
					break;
                }
                con->longReceive = false;

				if ( msgLength & 0x40000000 ) {
					msgLength &= 0xFFFFFFF;

					msgDecLength = msgLength;
					if ( msgDecLength > MEDIATOR_REC_BUFFER_SIZE_MAX || !AESDecrypt ( &client->aes, startOfCurrentMessage, &msgDecLength, &decrypted ) ) {
						break;
					}
					msg = ( MediatorMsg * ) decrypted;
				}
				else {
					msgDecLength = msgLength;
					msg = ( MediatorMsg * ) startOfCurrentMessage;
				}
                
                if ( msgDecLength <= 0 ) {
                    CErrArg ( "MediatorListener: Decoded msg length is 0 : lenght of packet [ %i ], inner msg [ %u ]", msgLength, msg->size );
                    goto Continue;
                }
                
                msgInnerLength = msg->size;
                
                //CLogArg ( "MediatorListener: New decoded --> msgDecLength [ %u ] - msgInnerLength [ %u ].", msgDecLength, msgInnerLength );

			NextInnerMessage:
                //CLogArg ( "MediatorListener: cmd [ %c %c : %i - %i ]", msg->cmd0, msg->cmd1, msgInnerLength, msgDecLength );
                
				if ( msg->cmd0 == MEDIATOR_CMD_STUNT && msg->cmd1 == MEDIATOR_OPT_NULL && msg->opt0 == MEDIATOR_OPT_NULL )
                {
                    CListenerLog ( 7, "MediatorListener: <--- STUNT request. " );

					if ( msgInnerLength >= MEDIATOR_STUNT_RESP_SIZE && env->environsState >= environs::Status::Starting ) {
						CListenerLog ( 6, "MediatorListener: <--- STUNT request" );

						StunTRequest::HandleIncomingRequest ( env, msg );
					}
				}
                else if ( msg->cmd0 == MEDIATOR_CMD_STUN && msg->cmd1 == MEDIATOR_OPT_NULL && msg->opt0 == MEDIATOR_OPT_NULL && msg->opt1 == MEDIATOR_OPT_NULL ) {
                    CListenerLog ( 7, "MediatorListener: <--- STUN request. " );

					if ( msgInnerLength >= sizeof ( STUNReqReqHeader ) && env->environsState >= environs::Status::Starting ) {

						CListenerLog ( 6, "MediatorListener: <--- STUN request" );

						env->asyncWorker.PushMediatorMsg ( ( char * ) msg, ASYNCWORK_TYPE_STUN );
					}
				}
                else if ( msg->cmd1 == MEDIATOR_CMD_SHORT_MESSAGE && msg->opt0 == MEDIATOR_OPT_NULL && msg->opt1 == MEDIATOR_OPT_NULL ) {
                    CListenerLog ( 7, "MediatorListener: <---  Short Msg. " );

					if ( msgInnerLength > 12 ) {

						int deviceID = msg->ids.id2.msgID;
                        CListenerLogArg ( 6, "MediatorListener: <---  Short message by [ 0x%X ] of size [ %u ]", deviceID, msgInnerLength );

						ShortMsgPacketHeader	*	shortMsg = ( ShortMsgPacketHeader * ) msg;

                        if ( shortMsg->version >= '3' && msgInnerLength > sizeof ( ShortMsgPacketHeader ) + 2 )
                        {
							char * appName = ( ( ShortMsgPacket * ) shortMsg )->appArea;
                            char * areaName = appName + shortMsg->sizes[0];

                            if ( shortMsg->sizes [0] < MAX_NAMEPROPERTY && shortMsg->sizes [1] < MAX_NAMEPROPERTY )
                            {
                                char * msgText = (char *) (areaName + shortMsg->sizes [1]);

								int len = ( int ) ( msgInnerLength - ( sizeof ( ShortMsgPacketHeader ) + shortMsg->sizes [ 0 ] + shortMsg->sizes [ 1 ] + 1 ) );

                                if ( len > 0 ) {
                                    msgText [ len ] = 0;
                                    CVerbArg ( "MediatorListener: <---  Short message by [ 0x%X ] of size [ %u ] [ %s ]", deviceID, msgInnerLength, msgText );
                                    CListenerLogArg ( 4, "MediatorListener: <---  Short message by [ 0x%X ] of size [ %u ]", deviceID, msgInnerLength );
                                    
                                    API::onEnvironsMsgNotifier ( env, deviceID, areaName, appName, SOURCE_DEVICE, msgText, len, "im" );                                    
                                }
                            }
						}
					}
				}
                else if ( msg->cmd1 == MEDIATOR_CMD_MEDIATOR_NOTIFY && msg->opt0 == MEDIATOR_OPT_NULL && msg->opt1 == MEDIATOR_OPT_NULL ) {
                    CListenerLog ( 7, "MediatorListener: <---  Notify. " );

                    MediatorNotifyHeader *  notif   = ( MediatorNotifyHeader * ) msg;

                    unsigned int notify = notif->msgID;

                    if ( notify == NOTIFY_MEDIATOR_SRV_STUNT_REG_REQ ) {
                   
                        env->asyncWorker.PushMediatorMsg ( ( char * ) msg, ASYNCWORK_TYPE_REGISTER_STUNT_SOCKETS );
                    }
					else if ( msgInnerLength >= sizeof ( MediatorNotifyHeader ) )
                    {

						CListenerLogArg ( 6, "MediatorListener: <--- [ %s ]", environs::resolveName ( ( int ) notify ) );

                        int notifyDeviceID = notif->notifyDeviceID;

                        while ( notif->size >= sizeof ( MediatorNotifyHeader ) )
                        {
                            size_t appLen = notif->sizes[0];
                            size_t areaLen = notif->sizes[1];

                            if ( appLen >= MAX_NAMEPROPERTY || areaLen >= MAX_NAMEPROPERTY )
                                break;

                            char * areaName = 0;
                            char * appName  = 0;

                            MediatorNotify  *   raw     = ( MediatorNotify * ) msg;

                            if ( raw->appArea [0] && raw->appArea [1] && appLen > 1 && areaLen > 1 ) {
                                appName  = raw->appArea;
                                areaName = appName + appLen;

                                if ( notifyDeviceID == env->deviceID && !strncmp ( areaName, env->areaName, sizeof ( env->areaName ) ) && !strncmp ( appName, env->appName, sizeof ( env->appName ) ) )
                                    break;
                            }
                            else {
                                if ( notifyDeviceID == env->deviceID )
                                    break;
                            }

                            DeviceChangePacket * packet = ( DeviceChangePacket * ) calloc ( 1, sizeof ( DeviceChangePacket ) + areaLen + appLen );
                            if ( !packet )
                                break;

                            packet->notification    = notify;
                            packet->deviceID        = notifyDeviceID;

                            if ( areaName ) {
                                if ( !BuildAppAreaField ( packet->sizes, appName, areaName, false ) ) {
                                    free ( packet );
                                    break;
                                }
                            }
                            else {
                                packet->sizes [ 0 ] = 1;
                                packet->sizes [ 1 ] = 1;
                            }

                            TraceAliveLocker ( "MediatorListener" );
                            
                            if ( !aliveThread.Lock ( "MediatorListener" ) ) {
                                free ( packet );
                                break;
                            }

                            if ( deviceChangePacketsEnd ) {
                                deviceChangePacketsEnd->next    = packet;
                                deviceChangePacketsEnd          = packet;
                            }
                            else {
                                deviceChangePackets = deviceChangePacketsEnd = packet;
                            }

                            deviceChangeIndicator |= MEDIATOR_DEVICE_CHANGE_MEDIATOR;

                            aliveThread.Notify ( "MediatorListener", false );
                            
                            TraceAliveUnlocker ( "MediatorListener" );
                            aliveThread.Unlock ( "MediatorListener" );
                            break;
                        }
					}
				}
                else if ( msg->cmd1 == MEDIATOR_CMD_HELP_TLS_GEN && msg->opt1 == MEDIATOR_OPT_NULL ) {
                    CListenerLog ( 3, "MediatorListener: <--- TLS gen." );

					HandleCertificateResponse ( msg, msgInnerLength );
				}
				else if ( msg->cmd1 == MEDIATOR_CMD_SESSION_ASSIGN && msg->opt1 == MEDIATOR_OPT_NULL )
                {
					CListenerLog ( 3, "MediatorListener: <---  Session id." );
					/// Assigned a session id
					client->sessionID = msg->ids.sessionID;
				}
				else if ( msg->cmd1 == MEDIATOR_CMD_AUTHTOKEN_ASSIGN )
                {
					CListenerLog ( 3, "MediatorListener: <---  AuthToken." );
					/// Assigned an authToken
					MediatorReqHeader * authMsg = ( MediatorReqHeader * ) msg;

                    if ( authMsg->sizes[1] > 1 && authMsg->sizes[1] < 180 )
					{
						char * dest     = env->DefaultMediatorToken;
						char * destUser = env->DefaultMediatorUserName;

						if ( med != &this->mediator ) {
							dest        = env->CustomMediatorToken;
							destUser    = env->CustomMediatorUserName;
						}

						memcpy ( dest, ( ( MediatorReqMsg * ) authMsg)->deviceUID, authMsg->sizes[1] );
						dest [ authMsg->sizes[1] ] = 0;

						size_t userLen = strlen ( env->UserName );

						memcpy ( destUser, env->UserName, userLen );
						destUser [ userLen ] = 0;

                        env->asyncWorker.Push ( 0, ASYNCWORK_TYPE_SAVE_MEDIATOR_TOKENS );

						/// Clear the password now and use the token for subsequent logins
						Zero ( env->UserPassword );
					}
				}
				else if ( msg->cmd0 == MEDIATOR_SRV_CMD_ALIVE_REQUEST && msg->cmd1 == MEDIATOR_OPT_NULL && msg->opt0 == MEDIATOR_OPT_NULL )
				{
					CListenerLog ( 5, "MediatorListener: <--- Alive packet request" );

                    MediatorGetPacket beat;
                    Zero ( beat );

                    beat.size       = 8;
                    beat.version    = MEDIATOR_PROTOCOL_VERSION;
                    beat.cmd        = MEDIATOR_CMD_HEARTBEAT;
                    beat.opt0       = MEDIATOR_OPT_NULL;
                    beat.opt1       = MEDIATOR_OPT_NULL;

                    CVerbs ( 6, "MediatorListener: Sending heartbeat..." );
                    
                    if ( !PushSend ( &med->connection.instance, ( char * ) &beat, 8, 0 ) ) {
                        CErr ( "MediatorListener: Send of heartbeat failed." );
                    }
				}
				else if ( msg->cmd0 == MEDIATOR_SRV_CMD_SESSION_RETRY && msg->cmd1 == MEDIATOR_OPT_NULL && msg->opt0 == MEDIATOR_OPT_NULL )
				{
                    CListenerLog ( 3, "MediatorListener: <--- Registration retry" );
                    CVerb ( "MediatorListener: <--- Registration retry - Closing listener" );
                    goto Finish;
				}
				else if ( msg->cmd0 == MEDIATOR_SRV_CMD_SESSION_LOCKED && msg->cmd1 == MEDIATOR_OPT_NULL && msg->opt0 == MEDIATOR_OPT_NULL )
				{
					CLog ( "MediatorListener: <--- Registration details (area/app/id) locked by another user." );

					// We should consider to request a new device id

                    env->deviceID = 0;
                    //environs::API::SetDeviceID ( env->hEnvirons, 0 );
                    CVerb ( "MediatorListener: <--- Registration details (area/app/id) locked by another user. - Closing listener" );
					goto Finish;
                }
                else if ( msg->cmd1 == MEDIATOR_CMD_NATSTAT )
                {
                    CListenerLogArg ( 3, "MediatorListener: <--- Received natstat [ %c ].", msg->cmd1 );

                    behindNAT = (msg->cmd0 == 1);
                }
                else if ( msg->cmd1 == MEDIATOR_CMD_DEVICE_FLAGS )
                {
                    CListenerLogArg ( 3, "MediatorListener: <--- Received device flags [ %c ].", msg->cmd1 );

					env->asyncWorker.PushMediatorMsg ( ( char * ) msg, ASYNCWORK_TYPE_DEVICE_FLAGS_UPDATE );
                }
#ifdef MEDIATOR_USE_SOCKET_BUFFERS_APPLY_AT_CLIENT
                else if ( msg->cmd1 == MEDIATOR_CMD_SET_SOCKET_BUFFERS )
                {
                    CListenerLogArg ( 3, "MediatorListener: <--- Received socket buffer sizes [ %c ].", msg->cmd1 );

                    ApplySocketBufferSizes ( sock, msg );
                }
#endif
                else if ( msg->cmd1 == MEDIATOR_CMD_QUIT )
                {
                    CListenerLogArg ( 3, "MediatorListener: <--- Received quit [ %c ].", msg->cmd1 );
                    
                    client->SendTcpFin ();
                    break;
                }
                else
                {
#ifdef DEBUG_TIMEOUT_ISSUE1
                    CLogArg ( "MediatorListener: Received seqNr [ %i ]", ((MediatorQueryResponseV6 *) msg)->seqNr );
#endif
					// Send received signal
					pthread_mutex_lock ( &con->receiveLock );
					//CLogArg ( "MediatorListener: Received data [%c%c%c] of size [%u]", msg [5], msg [6], msg [7], msgLength );

                    if ( con->responseBuffers.size () > 0 )
                    {
                        int seqNr = ((MediatorQueryResponseV6 *) msg)->seqNr;
                        
                        std::map < int, MediatorResponseBuffer * >::iterator foundIt = con->responseBuffers.find ( seqNr );
                        
                        if ( foundIt != con->responseBuffers.end () )
                        {
                            MediatorResponseBuffer * req = ( MediatorResponseBuffer * ) foundIt->second;
                            if ( req && req->buffer )
                            {
                                unsigned int size = msgInnerLength;

								if ( req->size < ( int ) size)
									size = req->size;
#ifdef DEBUG_TIMEOUT_ISSUE1
                                CLogArg ( "MediatorListener: <--- [ %i ] Copy buffer [ %i ].", seqNr, size );
#endif
                                CListenerLogArg ( 6, "MediatorListener: <--- Copy buffer [ %i ].", size );
                                memcpy ( req->buffer, msg, size );
                                
                                req->success = true;
                                
                                CListenerLog ( 6, "MediatorListener: <--- Signaling buffer." );
                                
                                if ( pthread_cond_signal ( &con->receiveEvent ) ) {
                                    CErr ( "MediatorListener: Failed to signal event!" );
                                }
                            }
#ifdef DEBUG_TIMEOUT_ISSUE
                            else { CErrArg ( "MediatorListener: Buffer missing for seqNr [ %i ]", seqNr ); }
#endif
                        }
#ifdef DEBUG_TIMEOUT_ISSUE
                        else { CErrArg ( "MediatorListener: No matching seqNr [ %i ]", seqNr ); }
#endif
                    }
#ifdef DEBUG_TIMEOUT_ISSUE
                    else { CErrArg ( "MediatorListener: No contexts for seqNr [ %i : %c : %c ]", ((MediatorQueryResponseV6 *) msg)->seqNr, msg->cmd0, msg->cmd1 ); }
#endif
					pthread_mutex_unlock ( &con->receiveLock );
				}
				//CLogArg ( "MediatorListener: %s", (msg + 4) );
                
                if ( msgDecLength > msgInnerLength )
                {
                    //CLogArg ( "MediatorListener: msgDecLength [ %u ] msgInnerLength [ %u ] -> msgDecLength [ %u ].", msgDecLength, msgInnerLength, (msgDecLength - msgInnerLength) );
                    
                    msgDecLength -= msgInnerLength;
                    if ( msgDecLength >= 8 ) {
                        msg = (MediatorMsg *) ( ( ( char * ) msg ) + msgInnerLength );
                        
                        msgInnerLength = msg->size;
                        //CLogArg ( "MediatorListener: --> msgInnerLength [ %u ].", msgInnerLength );
                        
                        if ( msgInnerLength >= 8 && msgInnerLength <= msgDecLength )
                            goto NextInnerMessage;
#ifndef NDEBUG
						else { CErrArg ( "MediatorListener: ERROR. Remaining inner msg bytes [ %i ]", msgInnerLength ); }
#endif
                    }
#ifndef NDEBUG
					else { CErrArg ( "MediatorListener: ERROR. Remaining msg bytes [ %i ]", msgDecLength ); }
#endif
                }
                
            Continue:
				bytesInBuffer -= msgLength;
				startOfCurrentMessage += msgLength;

                free_m ( decrypted );
            }

            free_m ( decrypted );

			if ( bytesInBuffer > 0 )
            {
                // Refactor if the whole message would not fit into the buffer
                if ( (msgLength & 0xFFFFFFF) > remainingSize )
                {
                    RefactorBuffer ( startOfCurrentMessage, buffer, bytesInBuffer, currentPtr );

                    remainingSize       = ( int ) ( (currentPtr - startOfCurrentMessage) - 1 );
                }
			}
            else {
                currentPtr              = buffer;
                startOfCurrentMessage   = buffer;
                bytesInBuffer           = 0;
                remainingSize           = MEDIATOR_REC_BUFFER_SIZE_MAX - 1;
            }
		}

	Finish:
        free_m ( decrypted );

        client->thread.Lock ( "MediatorListener" );
        
		socki = client->socket;
		if ( IsValidFD ( socki ) ) {
            client->socket = INVALID_FD;
			ShutdownCloseSocket ( socki, false, "MediatorListener instance.socket 1" );

            // Increase probability that SendThread is not using the socket that we're gonna close later.
            // Hence, we trigger SendThread activity
            SendBroadcast ( false, false, false, true );
        }
        
        client->thread.Unlock ( "MediatorListener" );
        
        if ( pthread_mutex_trylock ( &client->stuntSocketLock ) == 0 )
        {
            client->stuntSocketsLog.clear ();
            
            pthread_mutex_unlock ( &client->stuntSocketLock );
        }
        
        StuntThreadsDispose ();

		sendThreadDisposeContexts = true;

		sendThread.Notify ( "MediatorListener" );

		client->encrypt       = 0;
		client->sessionID     = 0;
		client->authenticated = false;
        med->listening        = false;

        // Signal a potential "sendResponse entity" which waits for responses, that we're not going to serve anymore ...
        //
        pthread_mutex_lock ( &con->receiveLock );
        
        if ( pthread_cond_signal ( &con->receiveEvent ) ) {
            CErr ( "MediatorListener: Failed to signal event!" );
        }
        pthread_mutex_unlock ( &con->receiveLock );


		if ( IsValidFD ( socki ) ) {
			ShutdownCloseSocket ( socki, true, "MediatorListener instance.socket 2" );
		}

		if ( isRunning )
			ReleaseCert ( 0 );
        
        registerFails++;
        if ( registerFails > ENVIRONS_MEDIATOR_MAX_TRYS ) {
            CErr ( "MediatorListener: We seem to be banned by the Mediator..." );
        }
        else {
            DevicesHasChanged ( MEDIATOR_DEVICE_RELOAD );

            API::onEnvironsNotifier1 ( env, NOTIFY_MEDIATOR_SERVER_DISCONNECTED );
        }

		CLog ( "MediatorListener: bye bye ..." );

		return 0;
    }
    
    
    bool MediatorClient::CommitSend ( MediatorInstance * med, char * msgResp, bool withResponse, unsigned int returnMaxSize, int msTimeout )
    {
        CVerbVerb ( "CommitSend" );
        
        if ( !med || !msgResp || env->environsState < environs::Status::Starting ) {
            CVerbVerb ( "CommitSend: Invalid arguments." );
            return false;
        }
        
        int					rc          = ETIMEDOUT, seqNr = 0;
        
        bool				success		= false;
        MediatorConnection * con		= &med->connection;
        
        MediatorResponseBuffer  resp;
#ifndef NDEBUG
		resp.success = false;
#endif        
        unsigned int * psize            = reinterpret_cast < unsigned int * > ( msgResp );
        
        // update message size to buffer
        unsigned int sendSize           = *psize;
        
        if ( sendSize > MEDIATOR_CLIENT_MAX_BUFFER_SIZE ) {
            CErrArg ( "CommitSend: Size of message [ %u ] is larger than [ %i ] bytes ! We don't support this yet!", sendSize, MEDIATOR_CLIENT_MAX_BUFFER_SIZE );
            return false;
        }
        
        MediatorQueryHeaderV6 * header = ( MediatorQueryHeaderV6 * ) msgResp;
        
        if ( withResponse )
            header->seqNr = ( unsigned int ) __sync_add_and_fetch ( &con->instance.seqNr, 1 );
        
        CVerbsVerbArg ( 5, "CommitSend: Submit packet of size [ %d ]  ...", sendSize );
        
        if ( withResponse )
        {
            seqNr = header->seqNr;
            
            if ( pthread_mutex_lock ( &con->receiveLock ) ) {
                CErr ( "CommitSend: Failed to aquire mutex on receiveLock!" ); return false;
            }
            
            resp.size    = ( int ) returnMaxSize;
            resp.buffer  = msgResp;
            resp.success = false;
            
            con->responseBuffers [ seqNr ] = &resp;
            
            if ( pthread_mutex_unlock ( &con->receiveLock ) ) {
                CErr ( "CommitSend: Failed to release receiveLock!" ); return false;
            }
            
#ifdef DEBUG_TIMEOUT_ISSUE1
            CLogArg ( "CommitSend: Send seqNr [ %i ]", seqNr );
#endif
        }
        
        success = SendBufferOrEnqueue ( &con->instance, msgResp, sendSize, seqNr );
        if ( withResponse )
        {
            do
            {
                int maxWaits = 3;
                
                if ( pthread_mutex_lock ( &con->receiveLock ) ) {
                    CErr ( "CommitSend: Failed to aquire mutex on receiveLock!" ); break;
                }
                
                if ( !success )
                {
                    CLogArg ( "CommitSend: Send failed [ %i ]", sendSize ); LogSocketError ();
                    
                    if ( IsValidFD ( con->instance.socket ) ) { LogSocketErrorF ( "MediatorClient.CommitSend" ); }
                    
                    *psize = 0;
                }
                else
                {
                    success = false;
                    
                    CVerbsVerbArg ( 5, "CommitSend: Buffer size [ %u ], Going into wait state ...", returnMaxSize );
                    
                    unsigned int timeout = msTimeout ? msTimeout : ((returnMaxSize >= 512) ? WAIT_TIME_FOR_RECEIVING_TCP_MAX : WAIT_TIME_FOR_RECEIVING_TCP_ACK );
                    
                Retry:
                    if ( !resp.success ) {
                        // Win32: reset the receive signal to prepare the sendi
                        pthread_cond_preparev ( &con->receiveEvent );
                        
                        rc = pthread_cond_timedwait_msec ( &con->receiveEvent, &con->receiveLock, timeout );
                    }
                    
                    if ( !resp.success && rc ) {
                        if ( rc == ETIMEDOUT ) {
                            maxWaits--;
                            if ( maxWaits > 0 && con->longReceive ) {
                                CWarnsArg ( 1, "CommitSend: TIMEOUT [ %i ]. Retry. [ %i ]", seqNr, maxWaits );
                                goto Retry;
                            }
                            CErrArg ( "CommitSend: Wait for response [ %i ] failed due to TIMEOUT [ %u - %u ]. Buffer size [ %u ]", seqNr, timeout, msTimeout, returnMaxSize );
                        }
                        else if ( rc == EPERM ) {
                            CErr ( "CommitSend: Wait for response failed due to Mutex not locked by caller" );
                        }
                        else if ( rc == EINVAL ) {
                            CErr ( "CommitSend: Wait for response failed due to INVALID input parameters" );
                        }
                    }
                    else {
                        success = true;
                    }
                }
                
                std::map < int, MediatorResponseBuffer * >::iterator foundIt = con->responseBuffers.find ( seqNr );
                
                if ( foundIt != con->responseBuffers.end () )
                {
                    con->responseBuffers.erase ( foundIt );
                }
                
                if ( pthread_mutex_unlock ( &con->receiveLock ) ) {
                    CErr ( "CommitSend: Failed to release receiveLock!" );
                    success = false;
                }
            }
            while ( false );
        }
        else {
            if ( success )
                *psize = sendSize;
        }
        
        return success;
    }
    

	bool MediatorClient::SendMessageToMediator ( void * msgResp, bool withResponse, unsigned int returnMaxSize, int msTimeout )
	{
		bool ret = true;

		if ( native.networkStatus < NETWORK_CONNECTION_NO_INTERNET ) {
			CVerb ( "SendMessageToMediator: No network connection available." );
			return false;
		}

		if ( !env->useCustomMediator && !env->useDefaultMediator ) {
			CVerb ( "SendMessageToMediator: Mediator usage is disabled by settings." );
			return false;
		}

		// Send to all of the mediators
		MediatorInstance * med = &mediator;

		while ( med ) {
			if ( !med->ip || !med->port ) {
				goto NextMediator;
			}

			if ( !med->listening || IsInvalidFD ( med->connection.instance.socket ) ) {
				if ( !RegisterAtMediators ( false ) )
					goto NextMediator;
				/// We have started a registration. Tell the app that it should try again in some seconds...
				goto EndWithFalse;
			}

			CVerbsArg ( 5, "SendMessageToMediator: [ %d ] bytes to [ %s : %i ]", *( ( unsigned int * ) msgResp ), inet_ntoa ( *( ( struct in_addr * ) &med->ip ) ), med->port );

			if ( CommitSend ( med, ( char * ) msgResp, withResponse, returnMaxSize, msTimeout ) )
			{
				if ( withResponse ) {
					// Do we have received anything?
					if ( *( ( unsigned int * ) msgResp ) > 0 ) {
						goto EndWithTrue;
					}
				}
				else //if ( *( ( unsigned int * ) msgResp ) == sendSize ) {
					goto EndWithTrue;
				//}
			}
		NextMediator:
			med = med->next;
		}

	EndWithFalse:
		ret = false;

    EndWithTrue:
		return ret;
	}


	bool MediatorClient::SendMessageToDevice ( int deviceID, const char * areaName, const char * appName, void * sendBuffer, int messageSize )
	{
		CVerbsArgID ( 5, "SendMessageToDevice: [ %i ] bytes to not connected device [0x%X -> 0x%X]", messageSize, env->deviceID, deviceID );
		CVerbsVerbArgID ( 6, "SendMessageToDevice: [ %s ]", (char *) sendBuffer );

		bool ret = false;

		if ( messageSize <= 0 ) {
			return false;
		}

		DeviceBase::SaveToStorageMessages ( env, "om", deviceID, areaName, appName, ( const char * ) sendBuffer, messageSize );


		char * buffer = ( char * ) calloc ( 1, messageSize + sizeof ( ShortMsgPacket ) + 1 );
		if ( !buffer ) {
			CErrID ( "SendMessageToDevice: Failed to allocate temporary message buffer!" );
			return false;
		}

		ShortMsgPacketHeader * msg = ( ShortMsgPacketHeader * ) buffer;
        msg->version = MEDIATOR_PROTOCOL_VERSION;
        msg->ident [ 0 ] = MEDIATOR_CMD_SHORT_MESSAGE;
        msg->ident [ 1 ] = MEDIATOR_OPT_NULL;
        msg->ident [ 2 ] = MEDIATOR_OPT_NULL;

		msg->deviceID = deviceID;
		msg->sizes [ 0 ] = 1;
		msg->sizes [ 1 ] = 1;

		if ( areaName && appName ) {
            if ( !BuildAppAreaField ( msg->sizes, appName, areaName, false ) ) {
                free_n ( buffer );
                return false;
            }
		}

		size_t offset = msg->sizes [ 0 ] + msg->sizes [ 1 ];

		char * text = ( char * ) ( msg->sizes + 2 + offset );

		// copy the message to the send buffer
        memcpy ( text, sendBuffer, messageSize );

		text [ messageSize ] = 0;

		msg->size = ( unsigned int ) ( messageSize + offset + sizeof ( ShortMsgPacketHeader ) + 1 );
        
        ret = ( PushSend ( buffer, msg->size ) == ( int ) msg->size );
		if ( !ret ) {
			CErrID ( "SendMessageToDevice: Failed to send message through mediator." );
		}
		else {
			CVerbsID ( 6, "SendMessageToDevice: Message successfully sent!" );
		}

        free_n ( buffer );

		return  ret;
	}


	bool MediatorClient::IsIPInSameNetwork ( unsigned int ip )
	{
		CVerb ( "IsIPInSameNetwork" );

		if ( !LockAcquireA ( localNetsLock, "IsIPInSameNetwork" ) )
			return false;

		bool ret = false;

		NetPack * net = &localNets;

		while ( net ) {
			if ( ( net->ip & net->mask ) == ( ip & net->mask ) ) {
				ret = true;
				break;
			}
			net = net->next;
		}

		LockReleaseVA ( localNetsLock, "IsIPInSameNetwork" );

		return ret;
	}


	bool MediatorClient::IsDeviceInSameNetwork ( int deviceID, const char * areaName, const char * appName )
	{
		//CLog ( "IsDeviceInSubnet" );

		sp ( DeviceInstanceNode ) device = GetDeviceNearbySP ( deviceID, areaName, appName );
		if ( device ) {
			bool bc = ( device->info.broadcastFound != 0 );
			if ( bc )
				return true;
		}

		/// If we haven't found the target in broadcast range, this might be due to some unexpected reasons:
		/// - Broadcast packets might have been missed
		/// - A firewall has dropped them before we could get the chance to allow them
		/// - and so on ...
		/// -> We'll try to find a match of the subnets using both external IPs
		/// --> The subnet may be wrong. However, how to query the correct one?

		DevicePack deviceInfo;
        Zero ( deviceInfo );

        DeviceInfo * info = (DeviceInfo *) ( ( ( char * ) &deviceInfo ) + DEVICES_HEADER_SIZE );

		int deviceCount = GetDeviceFromMediatorCached ( ( char * ) &deviceInfo, sizeof ( deviceInfo ), deviceID, areaName, appName );

		if ( deviceCount > 0 && info->ip ) {
			unsigned int subnet = GetLocalSN ();

			unsigned int ourNetwork = GetLocalIPe () & subnet;
			unsigned int theirNetwork = info->ipe & subnet;

			if ( ourNetwork == theirNetwork ) {
				/// If we are in the same network, then verify the special case that one of us is behind a firewall that is also in the same subnet
				// happens in our lab
				unsigned int theirInternalNetork = info->ip & subnet;
				unsigned int ourInternalNetork = GetLocalIP () & subnet;

				if ( theirInternalNetork == theirNetwork )
					return ( ourInternalNetork == ourNetwork );
				//return (ourInternalNetork != ourNetwork);
			}
		}

		return false;
	}


	bool MediatorClient::GetNATStat ()
	{
        return behindNAT;
    }


    bool MediatorClient::InformMediator ( MediatorInstance * med )
    {
        if ( !isRunning )
            return false;

        CVerbs ( 6, "InformMediator" );

        bool success = true;

        char buffer [ 20 ];

#ifdef MEDIATOR_USE_SOCKET_BUFFERS_APPLY_AT_SERVER
        /*
        int recSize = 0;
        int sendSize = 0;

        socklen_t retSize = sizeof ( recSize );

        // - Load send buffer size
        int rc = getsockopt ( med->connection.instance.socket, SOL_SOCKET, SO_RCVBUF, ( char * ) &recSize, &retSize );
        if ( rc < 0 ) {
            CErr ( "InformMediator: Failed to query receive buffer size!" ); LogSocketError ();
        }
        else {
            CVerbsArg ( 2, "InformMediator: receive buffer size [%i]", recSize );
        }

        // - Load send buffer size
        retSize = sizeof ( sendSize );

        rc = getsockopt ( med->connection.instance.socket, SOL_SOCKET, SO_SNDBUF, ( char * ) &sendSize, &retSize );
        if ( rc < 0 ) {
            CErr ( "InformMediator: Failed to query send buffer size!" ); LogSocketError ();
        }
        else {
            CVerbsArg ( 2, "InformMediator: send buffer size [%i]", sendSize );
        }
        */
#endif

        unsigned int * pUI = reinterpret_cast<unsigned int *>( buffer );
        *pUI = MEDIATOR_NAT_REQ_SIZE;

        buffer [ 4 ] = MEDIATOR_PROTOCOL_VERSION;
        buffer [ 5 ] = MEDIATOR_CMD_NATSTAT;
        buffer [ 6 ] = ';';
        buffer [ 7 ] = ';';

        pUI += 2;
        *pUI = GetLocalIP ();
        
        if ( !SendBufferNoQueue ( &med->connection.instance, buffer, MEDIATOR_NAT_REQ_SIZE ) ) {
            CErr ( "InformMediator: Failed to query NAT status!" );
            success = false;
        }

#ifdef MEDIATOR_USE_SOCKET_BUFFERS_APPLY_AT_SERVER
        /*
        if ( sendSize && recSize )
        {
            pUI = reinterpret_cast<unsigned int *>( buffer );
            *pUI = MEDIATOR_MSG_SOCKET_BUFFERS_SIZE;

            buffer [ 4 ] = MEDIATOR_PROTOCOL_VERSION;
            buffer [ 5 ] = MEDIATOR_CMD_SET_SOCKET_BUFFERS;
            buffer [ 6 ] = ';';
            buffer [ 7 ] = ';';

            pUI += 2;

            *pUI = recSize; pUI++;
            *pUI = sendSize;

            if ( !CommitSend ( med, ( char * ) buffer, false, sizeof(buffer) ) ) {
                CErr ( "InformMediator: Failed to inform about socket buffer sizes!" );
                success = false;
            }
        }*/
#endif

        return success;
    }


	bool MediatorClient::GetMediatorServiceVersion ( unsigned int &version, unsigned int &revision )
	{
		if ( !isRunning )
			return false;

		CVerbs ( 8,"GetMediatorServiceVersion" );

		char buffer [ 20 ];

		unsigned int * pUI = reinterpret_cast<unsigned int *>( buffer );
		*pUI = 8; // 12 bytes
        
        buffer [ 4 ] = MEDIATOR_PROTOCOL_VERSION;
        buffer [ 5 ] = MEDIATOR_CMD_GET_VERSION;
        buffer [ 6 ] = MEDIATOR_OPT_NULL;
        buffer [ 7 ] = MEDIATOR_OPT_NULL;
        
		pUI = reinterpret_cast<unsigned int *>( buffer );

		if ( SendMessageToMediator ( buffer, true, 20 ) && *pUI >= MEDIATOR_MSG_VERSION_SIZE ) {
			pUI++;
			version = *pUI;
            
			pUI += 2;
			revision = *pUI;
			return true;
		}
		else {
			CErr ( "GetMediatorServiceVersion: Query failed!" );
		}

		return false;
    }


	bool MediatorClient::IsConnectAllowed ( int deviceID, const char * appName, const char * areaName )
	{
		sp ( DeviceInstanceNode ) device = GetDeviceSP ( deviceID, areaName, appName );

        if ( !device ) {
            // Device has not been registered by mediator layer, so let's assume the default value
            return env->allowConnectDefault;
        }
        
		return device->allowConnect;
	}


    sp ( DeviceInstanceNode ) MediatorClient::GetDeviceSP ( int deviceID, const char * areaName, const char * appName, int * success )//, bool useLock )
    {
        if ( !isRunning )
            return 0;

#ifdef USE_MEDIATOR_OPT_KEY_MAPS_COMP
        AppAreaKey deviceKey;
        AppAreaKey * key = &deviceKey;
#else
        char    key [ MAX_DEVICE_INSTANCE_KEY_LENGTH ];
#endif
		if ( !areaName || !*areaName )
			areaName = env->areaName;

		if ( !appName || !*appName )
			appName = env->appName;

        int length = BuildMapKey ( key, deviceID, areaName, appName );
        if ( length <= 0 ) {
            if ( success )
                *success = -1;

            CErrID ( "GetDeviceSP: Failed to build key!" );
            return 0;
        }

        sp ( DeviceInstanceNode ) deviceSP;

        if ( /*useLock &&*/ pthread_mutex_lock ( &devicesMapLock ) ) {
            if ( success )
                *success = -1;
            CErr ( "GetDeviceSP: Failed to aquire mutex!" );
            return 0;
        }

        std::map<APPAREATYPE *, DeviceInstanceNode *, compare_char_key>::iterator foundIt = devicesMapAvailable->find ( key );
        if ( foundIt != devicesMapAvailable->end () )
        {
#ifdef USE_MEDIATOR_OPT_KEY_MAPS_COMP
            CVerbArg ( "GetDeviceSP: Found [ %i : %s ]", key->deviceID, key->appArea );
#else
            CVerbArg ( "GetDeviceSP: Found [ %s ]", key ENVIRONS_DEVICE_KEY_EXT );
#endif
            DeviceInstanceNode * device = foundIt->second;

            if ( device && device->mapSP )
                deviceSP = device->mapSP;
        }

        if ( /*useLock &&*/ pthread_mutex_unlock ( &devicesMapLock ) ) {
            if ( success )
                *success = -1;
            CErr ( "GetDeviceSP: Failed to unlock mutex!" );
        }

        return deviceSP;
    }


    sp ( DeviceInstanceNode ) MediatorClient::GetDeviceSP ( int hInst, int objID )
    {
#ifdef ENABLE_INSTANCE_WEAK_REFERENCE
        sp ( Instance ) envSP = native.instancesSP [ hInst ].lock ();
#else
        sp ( Instance ) envSP = native.instancesSP [ hInst] ;
#endif
        if ( !envSP )
            return 0;
        
        sp ( MediatorClient ) mediator = envSP->mediator MED_WP;
        if ( !mediator )
            return 0;

        return mediator->GetDeviceSP ( objID );
    }


	sp ( DeviceInstanceNode ) MediatorClient::GetDeviceSP ( int objID )
	{
		if ( !isRunning )
			return 0;

        sp ( DeviceInstanceNode ) deviceSP;

        if ( pthread_mutex_lock ( &devicesMapLock ) ) {
			CErr ( "GetDeviceSP: Failed to aquire mutex!" );
			return 0;
		}

                std::map<APPAREATYPE *, DeviceInstanceNode *, compare_char_key>::iterator it     = devicesMapAvailable->begin ();
        //const   std::map<APPAREATYPE *, DeviceInstanceNode *, compare_char_key>::iterator &end   = devicesMapAvailable->end ();

        //while ( it != end ) {
        while ( it != devicesMapAvailable->end () ) {
            if ( it->second->info.objID == objID ) {
                deviceSP = it->second->mapSP;
                break;
            }
            ++it; // Crash here (Issue #194)
        }

        if ( pthread_mutex_unlock ( &devicesMapLock ) ) {
			CErr ( "GetDeviceSP: Failed to unlock mutex!" );
		}

		return deviceSP;
	}


    sp ( DeviceInstanceNode ) MediatorClient::GetDeviceNearbySP ( int deviceID, const char * areaName, const char * appName ) //, bool useLock )
	{
        sp ( DeviceInstanceNode ) device = GetDeviceSP ( deviceID, areaName, appName, 0 ); //, useLock );

		if ( device ) {
			if ( device->info.broadcastFound == DEVICEINFO_DEVICE_BROADCAST && ( lastGreetUpdate - device->info.updates ) > 120000 ) {
				// If it has not updated within 2 minutes, then we will assume that the device has vanished
				CWarnArgID ( "GetDeviceNearbySP: device [%i] found, but seems to be inactive!", device->info.deviceID );
				device = 0;
			}

			else if ( device->info.broadcastFound == DEVICEINFO_DEVICE_MEDIATOR ) {
				device = 0;
			}
		}

		return device;
	}


	bool MediatorClient::GetMediatorLocalEndpoint ( MediatorInstance * med, struct sockaddr_in * addr )
	{
		if ( !med ) {
			med = GetAvailableMediator ();
			if ( !med )
				return false;
		}

        int sock = med->connection.instance.socket;

        if ( IsInvalidFD ( sock ) )
            return false;

        if ( addr ) {
            socklen_t length = sizeof ( sockaddr );
            
            memset ( addr, 0, length );
            
            int ret = getsockname ( sock, ( struct sockaddr * )addr, &length );
            if ( ret )
                return false;
            
            CVerbsArg ( 3, "GetMediatorLocalEndpoint: [ %s : %d ]", inet_ntoa ( addr->sin_addr ), ntohs ( addr->sin_port ) );
        }

		return true;
	}


	MediatorInstance *	MediatorClient::GetAvailableMediator ()
	{
		CVerbVerb ( "GetAvailableMediator" );

		/// Find the first active mediator connection
		MediatorInstance * med = &mediator;

		while ( med ) {
			if ( med->enabled && med->listening ) {
				return med;
			}
			med = med->next;
		}

		CVerbVerb ( "GetAvailableMediator: No active mediator found" );
		return nill;
	}


    /*
     * Note: On success, this method returns with devicesMapLock locked.
     *       Otherwise, no lock is held.
     */
	bool MediatorClient::DevicesMediatorReload ()
	{
		CVerbs ( 6, "DevicesMediatorReload" );

		bool    success         = true;
		char *  bufferDevices   = 0;
        int     updateCounter;

#ifdef USE_MEDIATOR_OPT_KEY_MAPS_COMP
        AppAreaKey      deviceKey;
        AppAreaKey *    key         = &deviceKey;
#else
		char	key [ MAX_DEVICE_INSTANCE_KEY_LENGTH ];
#endif
		int		countDevices	= 0;

		if ( mediator.listening )
		{
			countDevices			 = GetDevicesFromMediatorCount ();
            if ( countDevices < 0 || countDevices > 5000 ) {
                CWarn ( "DevicesMediatorReload: Failed to query count of mediator devices!" );
                return false;
            }
            
            // Set mediator count to query in the next call
			deviceMediatorQueryCount = countDevices;

			countDevices = GetDevicesFromMediator ( bufferDevices, 0 );
			if ( countDevices < 0 || countDevices > 5000  || !bufferDevices ) {
				free_n ( bufferDevices );
				CWarnArg ( "DevicesMediatorReload: Failed to load [ %d ] mediator devices!", countDevices );
				return false;
            }
            
            // Update query count with the device count that we acutally have received
            deviceMediatorQueryCount = (countDevices + 1);
		}
		else {
			// Empty cache, nothing to do ...
			//
            if ( deviceMediatorCachedCount <= 0 ) {
                if ( !LockAcquireA ( devicesMapLock, "DevicesMediatorReload" ) )
                    return false;
				return true;
            }

			// Simulate a query with 0 device infos
			//
            void * t = calloc ( 1, sizeof ( DevicePack ) );
            if ( !t )
                return false;
            
            bufferDevices = static_cast < char * > ( t );

			deviceMediatorQueryCount	= 0;
			countDevices				= 0;

			DeviceHeader * deviceHead = ( DeviceHeader * ) bufferDevices;
			deviceHead->deviceCountAvailable    = 0;
			deviceHead->startIndex              = 0;
			deviceHead->deviceCount             = 0;
		}

		do
		{
			if ( !LockAcquireA ( devicesMapLock, "DevicesMediatorReload" ) ) {
				free ( bufferDevices );
				return false;
			}

			if ( devicesMapUpdates > 120 )
				devicesMapUpdates = 0;

			devicesMapUpdates++;
			updateCounter = devicesMapUpdates;

			if ( countDevices <= 0 ) {
				CVerbsArg ( 5, "DevicesMediatorReload: Loaded [ %d ]  mediator devices!", countDevices );
				break;
			}
            
			std::map<APPAREATYPE *, DeviceInstanceNode *, compare_char_key>::iterator foundIt;

			DeviceInfo * device = ( DeviceInfo * ) ( bufferDevices + DEVICES_HEADER_SIZE_V6 );

			// Iterate over the returned device list from Mediator server and sync
			// 
			for ( int i = 0; i < countDevices; i++ )
            {
                if ( device->deviceID )
                {
#ifdef USE_MEDIATOR_OPT_KEY_MAPS_COMP
                    *key->appArea = 0;
#else
                    *key = 0;
#endif
                    int keyLen = BuildMapKey ( key, device );
                    if ( keyLen <= 0 ) {
                        success = false; goto Finish;
                    }
                    
                    foundIt = devicesMapAvailable->find ( key );
                    
                    if ( foundIt != devicesMapAvailable->end () )
                    {
                        /// Update the item
#ifdef USE_MEDIATOR_OPT_KEY_MAPS_COMP
                        CListLogArg ( "DevicesMediatorReload: Updating [ %i : %s ]", key->deviceID, key->appArea );
#else
                        CListLogArg ( "DevicesMediatorReload: Updating [ %s ]", key ENVIRONS_DEVICE_KEY_EXT );
#endif
                        foundIt->second->info.internalUpdates = ( char ) updateCounter;
                        
                        DeviceCompareAndTakeOver ( foundIt->second, device, false );
                    }
                    else {
                        /// Add the item
#ifdef USE_MEDIATOR_OPT_KEY_MAPS_COMP
                        CListLogArg ( "DevicesMediatorReload: Adding mediator device [ %i : %s ]", key->deviceID, key->appArea );
#else
                        CListLogArg ( "DevicesMediatorReload: Adding mediator device [ %s ]", key ENVIRONS_DEVICE_KEY_EXT );
#endif
                        sp ( DeviceInstanceNode ) itemSP = sp_make ( DeviceInstanceNode );
                        if ( !itemSP ) {
                            CErr ( "DevicesMediatorReload: Failed to allocate memory for device item!" );
                            success = false; goto Finish;
                        }
                        else {
                            DeviceInstanceNode  * item = itemSP.get ();
                            item->hEnvirons = env->hEnvirons;

#ifdef USE_MEDIATOR_OPT_KEY_MAPS_COMP
                            memcpy ( &item->key, key, sizeof(item->key) );
#else
                            memcpy ( item->key, key, sizeof(item->key) );
#endif
                            item->mapSP = itemSP;
                            
                            memcpy ( &item->info, device, DEVICE_MEDIATOR_PACKET_SIZE );
                            
                            item->info.broadcastFound   = DEVICEINFO_DEVICE_MEDIATOR;
                            item->info.internalUpdates  = ( char ) updateCounter;
                            //item->info.hasAppEnv		= 1;
                            item->info.flags            = DeviceFlagsInternal::NativeReady;
                            item->allowConnect          = env->allowConnectDefault;

#ifdef USE_MEDIATOR_OPT_KEY_MAPS_COMP
                            (*devicesMapAvailable) [ &item->key ] = item;
#else
                            (*devicesMapAvailable) [ ( APPAREATYPE  * ) item->key ] = item;
#endif
                            env->asyncWorker.Push ( item->info.objID, ASYNCWORK_TYPE_DEVICE_FLAGS_SYNC );
                        }
                    }
                }
#ifndef NDEBUG
                else {
                    CErrArg ( "DevicesMediatorReload: Invalid deviceID 0 found at index [ %i ] !!!", i );
                }
#endif
				device++;
			}
		}
		while ( false );

	Finish:
		PrintDevicesMap ( devicesMapAvailable, false );

		if ( success )
		{
            if ( LockAcquireA ( devicesCacheLock, "DevicesMediatorReload" ) ) {
                free_n ( deviceMediatorCached );

				if ( countDevices > 0 && bufferDevices ) {
					deviceMediatorCached		= bufferDevices;
					deviceMediatorCachedCount	= countDevices;

					( ( DeviceHeader * ) bufferDevices )->deviceCount = countDevices;

					bufferDevices				= 0;
				}
                else {
					deviceMediatorCached		= 0;
					deviceMediatorCachedCount	= 0;
				}
				LockReleaseVA ( devicesCacheLock, "DevicesMediatorReload" );
			}
		}
		else {
			LockReleaseVA ( devicesMapLock, "DevicesMediatorReload" );
		}

        free_n ( bufferDevices );

		CListLog ( "DevicesMediatorReload: done" );
		return success;
	}


	bool MediatorClient::DevicesMediatorClear ()
	{
		CListLog ( "DevicesMediatorClear" );

		if ( !LockAcquireA ( devicesMapLock, "DevicesMediatorClear" ) )
			return false;

		std::map<APPAREATYPE *, DeviceInstanceNode *, compare_char_key>::iterator it = devicesMapAvailable->begin ();

		while ( it != devicesMapAvailable->end () )
        {
            it->second->mapSP = 0;
			++it;
		}

		devicesMapAvailable->clear ();

		LockReleaseVA ( devicesMapLock, "DevicesMediatorClear" );

		CListLog ( "DevicesMediatorClear: done" );
		return true;
	}


	bool MediatorClient::DevicesAvailableReload ()
	{
		CListLog ( "DevicesAvailableReload" );

		bool	notify				= false;
		bool    success             = true;
        int     updateCounter       = 0;

		DeviceInstanceNode * device;
		std::map<APPAREATYPE *, DeviceInstanceNode *, compare_char_key>::iterator foundIt;

        if ( mediator.ip ) {
            if ( !DevicesMediatorReload () ) {
                CListLog ( "DevicesAvailableReload: done" );
            
                //DevicesHasChanged ( MEDIATOR_DEVICE_RELOAD );
                return false;
            }
        }
        else {
            if ( !LockAcquireA ( devicesMapLock, "DevicesAvailableReload" ) )
                return false;
        }

		PrintDevicesMap ( devicesMapAvailable, false );

		updateCounter = devicesMapUpdates;

		// Attention: We will hold the devicesMapLock and the devicesLock
		if ( !LockAcquireA ( devicesLock, "DevicesAvailableReload" ) ) {
			success = false;
			goto Finish;
		}
		device = devices;

		while ( device )
        {
#ifdef USE_MEDIATOR_OPT_KEY_MAPS_COMP
            foundIt = devicesMapAvailable->find ( &device->key );
#else
			foundIt = devicesMapAvailable->find ( ( APPAREATYPE  * ) device->key );
#endif
			if ( foundIt != devicesMapAvailable->end () )
			{
				DeviceInfo * found = &foundIt->second->info;

				if ( foundIt->second != device ) {
                    /// Update the item
#ifdef USE_MEDIATOR_OPT_KEY_MAPS_COMP
                    CListLogArg ( "DevicesAvailableReload: Update nearby+mediator [ %i : %s ]", device->key.deviceID, device->key.appArea );
#else
					CListLogArg ( "DevicesAvailableReload: Update nearby+mediator [ %s ]", device->key ENVIRONS_DEVICE_KEY_EXT );
#endif
					// We need to switch (device is part of the nodelist, found is the one from the map)

                    device->info.flags          |= found->flags;
					//device->info.unused			= found->unused;
					device->info.ipe            = found->ipe;
					device->info.isConnected    = found->isConnected;

                    if ( found->nativeID > 0 )
                        device->info.nativeID   = found->nativeID;

					DeviceInstanceNode * toDelete = foundIt->second;

					devicesMapAvailable->erase ( foundIt );

                    device->mapSP = device->baseSP;

                    if ( device->mapSP ) {
#ifdef USE_MEDIATOR_OPT_KEY_MAPS_COMP
                        (*devicesMapAvailable) [ &device->key ] = device;
#else
                        (*devicesMapAvailable) [ ( APPAREATYPE  * ) device->key ] = device;
#endif
                        found = &device->info;

                        found->broadcastFound   = DEVICEINFO_DEVICE_BROADCAST_AND_MEDIATOR;
                    }

                    toDelete->mapSP = 0;
				}

				if ( found->broadcastFound != DEVICEINFO_DEVICE_MEDIATOR )
					found->internalUpdates  = ( char ) updateCounter;
			}
			else {
                /// Add the item
#ifdef USE_MEDIATOR_OPT_KEY_MAPS_COMP
                CListLogArg ( "DevicesAvailableReload: Add nearby reference [ %i : %s ]", device->key.deviceID, device->key.appArea );
#else
				CListLogArg ( "DevicesAvailableReload: Add nearby reference [ %s ]", device->key ENVIRONS_DEVICE_KEY_EXT );
#endif
                device->mapSP = device->baseSP;

                if ( device->mapSP ) {
#ifdef USE_MEDIATOR_OPT_KEY_MAPS_COMP
                    (*devicesMapAvailable) [ &device->key ] = device;
#else
                    (*devicesMapAvailable) [ ( APPAREATYPE  * ) device->key ] = device;
#endif
                    device->info.internalUpdates = ( char ) updateCounter;

                    env->asyncWorker.Push ( device->info.objID, ASYNCWORK_TYPE_DEVICE_FLAGS_SYNC );
                }
			}

			device = device->next;
		}

		PrintDevicesMap ( devicesMapAvailable, false );


		LockReleaseVA ( devicesLock, "DevicesAvailableReload" );

        if ( success ) {
            if ( devicesMapAvailable->size () > 0 )
                GC_DevicesAvailable ();

			UpdateDirtyFlags ( true, true );

			notify = true;
		}

	Finish:
		LockReleaseVA ( devicesMapLock, "DevicesAvailableReload" );

		if ( notify )
			API::onEnvironsNotifier1 ( env, 0, NOTIFY_MEDIATOR_DEVICELISTS_UPDATE_AVAILABLE, SOURCE_NATIVE );

		CListLog ( "DevicesAvailableReload: done" );
		return success;
	}


    /* Note: If something goes wrong, we must not touch the current cache.
     */
	bool MediatorClient::DevicesCacheRebuild ( bool withMediator, bool useLock )
	{
		CListLogArg ( "DevicesCacheRebuild: useLock [ %d ] withMediator: [ %d ]", useLock, withMediator );
        
		if ( useLock && !LockAcquireA ( devicesMapLock, "DevicesCacheRebuild" ) )
			return false;

		bool    success         = true;
		char *  bufferDevices   = 0;

		char *  bufferMediatorDevices   = 0;
		int     countMediator			= 0;
		int     countMediatorAlive		= 0;

		DeviceInfo * item				= 0;
        DeviceInfo * itemMediator       = 0;

		int countDevices = ( int ) devicesMapAvailable->size ();

		if ( countDevices > 0 && countDevices < 5000 )
		{
			while ( success ) {
				bufferDevices = ( char * ) malloc ( ( countDevices * DEVICE_PACKET_SIZE ) + ( 2 * DEVICES_HEADER_SIZE ) );

				if ( !bufferDevices ) {
					CWarnArg ( "DevicesCacheRebuild: Failed to allocate buffer for [ %d ] all available devices!", countDevices );
					success = false; countDevices = 0; break;
				}

				item = ( DeviceInfo * ) ( bufferDevices + DEVICES_HEADER_SIZE );

				if ( withMediator ) {
					countMediator = deviceMediatorQueryCount;
					if ( countMediator > 0 )
					{
						if ( countMediator > countDevices ) {
#ifndef NDEBUG                             
							if ( countMediator > ( countDevices + 1 ) ) {
								CWarnArg ( "DevicesCacheRebuild: Inconsistent status! There are more mediator devices reported by query [ %i ] than we have in the available map [ %i ]!", countMediator, countDevices );
							}
#endif
							countMediator = countDevices;
						}

						bufferMediatorDevices = ( char * ) malloc ( ( countMediator * DEVICE_PACKET_SIZE ) + ( 2 * DEVICES_HEADER_SIZE ) );

						if ( !bufferMediatorDevices ) {
							CWarnArg ( "DevicesCacheRebuild: Failed to allocate buffer for [ %d ] mediator devices!", countMediator );
							withMediator = false;
						}
						else {
							itemMediator = ( DeviceInfo * ) ( bufferMediatorDevices + DEVICES_HEADER_SIZE );
						}
					}
				}

				std::map<APPAREATYPE *, DeviceInstanceNode *, compare_char_key>::iterator it = devicesMapAvailable->begin ();

				while ( it != devicesMapAvailable->end () )
				{
                    DeviceInfo * src = &it->second->info;
                    
                    if ( src->deviceID )
                    {
                        // Check is not required as the GC will remove the item and leave a valid list to us
                        //if ( src->internalUpdates >= updateCounter ) {
#ifdef USE_MEDIATOR_OPT_KEY_MAPS_COMP
                        CListLogArg ( "DevicesCacheRebuild: Copying device to available [ %i : %s ]", it->second->key.deviceID, it->second->key.appArea );
#else
                        CListLogArg ( "DevicesCacheRebuild: Copying device to available [ %s ]", it->second->key ENVIRONS_DEVICE_KEY_EXT );
#endif
                        
                        memcpy ( item, src, DEVICE_PACKET_SIZE );
                        item++;
                        
                        if ( withMediator && src->broadcastFound != DEVICEINFO_DEVICE_BROADCAST ) {
                            if ( countMediatorAlive < countMediator ) {
                                memcpy ( itemMediator, src, DEVICE_PACKET_SIZE );
                                itemMediator++;
                                countMediatorAlive++;
                            }
#ifndef NDEBUG
                            else {
                                CErrArg ( "DevicesCacheRebuild: Inconsistent list status! There are more mediator devices in the available list [ %i ] than the counter states [ %i : %i ]!", countDevices, countMediatorAlive, countMediator );
                            }
#endif
                        }
                    }
#ifndef NDEBUG
                    else {
                        CErr ( "DeviceListUpdaterDo: Invalid deviceID 0 found." );
                    }
#endif
					++it;
				}

				break;
			}
		}
		else
			countDevices = 0;

		if ( useLock && !LockReleaseA ( devicesMapLock, "DevicesCacheRebuild" ) )
			success = false;

		if ( success ) {
            if ( LockAcquireA ( devicesCacheLock, "DevicesCacheRebuild" ) ) {
                free_n ( deviceAvailableCached );

				if ( countDevices && bufferDevices ) {
					deviceAvailableCached		= bufferDevices;
					deviceAvailableCachedCount	= countDevices;

					( ( DeviceHeader * ) bufferDevices )->deviceCount = countDevices;

					bufferDevices = 0;
				}
				else {
					deviceAvailableCached		= 0;
					deviceAvailableCachedCount	= 0;
				}
				deviceAvailableCacheDirty = false;

                if ( withMediator ) {
                    free_n ( deviceMediatorCached );

					if ( countMediatorAlive && bufferMediatorDevices ) {
						deviceMediatorCached		= bufferMediatorDevices;
						deviceMediatorCachedCount	= countMediatorAlive;

						( ( DeviceHeader * ) bufferMediatorDevices )->deviceCount = countMediatorAlive;

						bufferMediatorDevices		= 0;
					}
					else {
						deviceMediatorCached		= 0;
						deviceMediatorCachedCount	= 0;
					}
					deviceMediatorCacheDirty = false;
				}

				LockReleaseVA ( devicesCacheLock, "DevicesCacheRebuild" );
			}
		}

        free_n ( bufferDevices );
        free_n ( bufferMediatorDevices );

		CListLog ( "DevicesCacheRebuild: done" );
		return true;
	}


	bool MediatorClient::GC_DevicesAvailable ()
	{
		CListLog ( "GC.Devices.Available" );

		int updateCounter = devicesMapUpdates;

		std::map < APPAREATYPE *, DeviceInstanceNode *, compare_char_key >::iterator it = devicesMapAvailable->begin ();

		while ( it != devicesMapAvailable->end () )
        {
            DeviceInstanceNode * item = it->second;
            
			if ( !item || item->info.internalUpdates < updateCounter )
            {
                bool erase = false;
                sp ( DeviceController ) deviceSP;
                
                if ( item )
                    deviceSP = item->deviceSP.lock ();

				if ( !deviceSP )
					erase = true;
				else if ( deviceSP->deviceStatus != DeviceStatus::Connected )
					erase = true;
				else if ( item )  {
					int diff = updateCounter - ( int ) item->info.internalUpdates;

					if ( diff > 3 ) {
						deviceSP->SendPing ();

						item->info.internalUpdates = ( char ) updateCounter;
					}
				}

                if ( erase ) {
#ifdef USE_MEDIATOR_OPT_KEY_MAPS_COMP
                    CListLogArg ( "GC.Devices.Available: Disposing device from available [ %i : %s ]", item->key.deviceID, item->key.appArea );
#else
                    CListLogArg ( "GC.Devices.Available: Disposing device from available [ %s ]", item->key ENVIRONS_DEVICE_KEY_EXT );
#endif
                    devicesMapAvailable->erase ( it++ );

                    if ( item )
                        item->mapSP = 0;
                    continue;
                }
			}
			++it;
		}

		CListLog ( "GC.Devices.Available: done" );
		return true;
	}


#if defined(PRINT_DEVICE_MAP1)

	void PrintDevicesMap ( std::map<const char *, DeviceInstanceItem *, compare_char_key> &devMap, bool useLock )
	{

#ifdef DEBUGVERBList
		if ( useLock && !LockAcquireA ( &devicesMapLock, "PrintDevicesMap" ) )
			return;

		CListLog ( "DEVICES: Start" );

		int c = 1;

		std::map<const char *, DeviceInstanceItem *, compare_char_key>::iterator it = devMap.begin ();

		while ( it != devMap.end () )
		{
			CListLog ( "DEVICES: --------------------------------------------------" );

			DeviceInfo * device = &it->second->info;

			CListLogArg ( "DEVICE: [%d] deviceID  [0x%X / %s / T:%i]", c, device->deviceID, GET_DEVICE_SOURCE_STRING ( device->broadcastFound ), device->flags );

			CListLogArg ( "DEVICE: Area/App    = %s - %s", device->areaName, device->appName );
			CListLogArg ( "DEVICE: Device IPe     = %s (from socket)", inet_ntoa ( *( ( struct in_addr * ) &device->ipe ) ) );
			CListLogArg ( "DEVICE: Device IP      = %s", inet_ntoa ( *( ( struct in_addr * ) &device->ip ) ) );
			CListLogArg ( "DEVICE: Device name    = %s", device->deviceName );
			++it; c++;
		}

		CListLog ( "DEVICES: --------------------------------------------------" );
		CListLog ( "DEVICES: END" );

		if ( useLock && !LockReleaseA ( &devicesMapLock, "PrintDevicesMap" ) )
			return;
#endif
	}
#else
#endif


	void MediatorClient::DevicesHasChanged ( int type )
	{
		CVerb ( "DevicesHasChanged" );
        
        TraceAliveLocker ( "DevicesHasChanged" );
        if ( !aliveThread.Lock ( "DevicesHasChanged" ) )
			return;

		deviceChangeIndicator |= type;

        aliveThread.Notify ( "DevicesHasChanged", false );
        
        TraceAliveUnlocker ( "DevicesHasChanged" );
        aliveThread.Unlock ( "DevicesHasChanged" );
	}


	bool MediatorClient::DevicePacketUpdater ()
	{
		DeviceChangePacket * packet = 0;

		CVerb ( "DevicePacketUpdater: Clearing device change packets..." );

		do
		{
			if ( packet ) {
				if ( !DeviceListsUpdate ( packet ) )
                {
                    char * areaName = 0;
                    char * appName  = 0;

                    if ( packet->sizes [0] > 1 && packet->sizes [1] > 1 ) {
                        appName  = &packet->appArea;
                        areaName = appName + packet->sizes [0];
                    }

					CErrsArg ( 3, "DevicePacketUpdater: Failed to process [ %s ] packet for [ %i / %s / %s ]!",
						( packet->notification == NOTIFY_MEDIATOR_SRV_DEVICE_REMOVED ? "remove" : ( packet->notification == NOTIFY_MEDIATOR_SRV_DEVICE_CHANGED ? "change" : "add" ) ),
						packet->deviceID, appName ? appName : env->appName, areaName ? areaName : env->areaName );

					if ( !isRunning || env->environsState  < environs::Status::Starting ) {
						free ( packet );
						break;
					}
                }
                free ( packet );
			}
            
            TraceAliveLocker ( "DevicePacketUpdater" );
            if ( !aliveThread.Lock ( "DevicePacketUpdater" ) )
				break;

			packet = deviceChangePackets;
            
			if ( packet && packet->next ) {
				deviceChangePackets = packet->next;
			}
			else {
				deviceChangePackets = 0;
				deviceChangePacketsEnd = 0;
			}
            
            TraceAliveUnlocker ( "DevicePacketUpdater" );
            aliveThread.Unlock ( "DevicePacketUpdater" );
		}
		while ( packet );

		return true;
	}


	bool MediatorClient::DeviceListsUpdate ( DeviceChangePacket * packet )
    {
		CVerbVerb ( "DeviceListsUpdate" );

        char *  buffer  = 0;
        bool    locked  = false;
        int     success = 1;

		do
		{
			if ( packet->notification == NOTIFY_MEDIATOR_SRV_DEVICE_REMOVED )
			{
				if ( !LockAcquireA ( devicesMapLock, "DeviceListsUpdate" ) )
					return false;

                locked  = true;
				success = ( DeviceRemove ( packet ) >= 0 );
				break;
			}

            success = 0;

            char * areaName = 0;
            char * appName  = 0;

			if ( packet->sizes [ 0 ] > 1 && packet->sizes [ 1 ] > 1 ) {
				appName  = &packet->appArea;
				areaName = appName + packet->sizes [ 0 ];
			}

			int deviceCount = GetDevicesFromMediator ( buffer, packet->deviceID, areaName, appName );
			if ( deviceCount <= 0 || !buffer ) {
                CLogsArg ( 2, "DeviceListsUpdate: Device info [ %i / %s / %s ] not found [ %i ] at mediator!", packet->deviceID, appName ? appName : env->appName, areaName ? areaName : env->areaName, deviceCount );
				break;
            }

            if ( !LockAcquireA ( devicesMapLock, "DeviceListsUpdate" ) )
                break;
            locked  = true;

			DeviceInfo * headerDevice = ( DeviceInfo * ) ( buffer + DEVICES_HEADER_SIZE_V6 );

			if ( packet->notification == NOTIFY_MEDIATOR_SRV_DEVICE_CHANGED )
				success = DeviceChange ( 0, headerDevice );

			if ( success <= 0 )
				success = DeviceAdd ( 0, headerDevice );
		}
		while ( false );

        if ( locked && !LockReleaseA ( devicesMapLock, "DeviceListsUpdate" ) )
            success = 0;

        free_n ( buffer );

		return ( success > 0 );
	}


	sp ( DeviceInstanceNode ) MediatorClient::BuildDeviceListItem ( DeviceInfo * device, bool copy )
	{
		CListLog ( "BuildDeviceListItem" );

		sp ( DeviceInstanceNode ) itemSP = sp_make ( DeviceInstanceNode );
		if ( !itemSP ) {
			CErr ( "BuildDeviceListItem: Failed to allocate memory for device item!" );
			return 0;
		}

        DeviceInstanceNode * item = itemSP.get();

#ifdef USE_MEDIATOR_OPT_KEY_MAPS_COMP
        *item->key.appArea = 0;
        int keyLen = BuildMapKey ( &item->key, device );
#else
        *item->key = 0;
        int keyLen = BuildMapKey ( item->key, device );
#endif
		if ( keyLen <= 0 )
			return 0;

		if ( copy )
			memcpy ( &item->info, device, DEVICE_MEDIATOR_PACKET_SIZE );

        //item->info.unused = 0;
        item->info.flags        = DeviceFlagsInternal::NativeReady;
        item->allowConnect      = env->allowConnectDefault;

		return itemSP;
	}


	int MediatorClient::DeviceRemove ( DeviceChangePacket * packet )
	{
		CListLog ( "DeviceRemove: Mediator packet." );

        int     length;

#ifdef USE_MEDIATOR_OPT_KEY_MAPS_COMP
        AppAreaKey deviceKey;
        AppAreaKey * key = &deviceKey;
#else
		char    key [ MAX_DEVICE_INSTANCE_KEY_LENGTH ];
#endif
		char * appName   = &packet->appArea;
		char * areaName  = appName + packet->sizes [ 0 ];

		if ( packet->sizes [ 0 ] <= 1 || packet->sizes [ 1 ] <= 1 || !*appName || !*areaName ) {
			appName = env->appName;
			areaName = env->areaName;
		}

		length = BuildMapKey ( key, packet->deviceID, areaName, appName );
		if ( length <= 0 ) {
			return false;
		}

		return DeviceRemove ( key, true );
	}


	int MediatorClient::DeviceRemove ( APPAREATYPE * key, bool mediatorRequest, bool notify )
    {
#ifdef USE_MEDIATOR_OPT_KEY_MAPS_COMP
        CListLogArg ( "DeviceRemove: [ %s ] - [ %i : %s ]", mediatorRequest ? "Mediator" : "Nearby", key->deviceID, key->appArea );
#else
        CListLogArg ( "DeviceRemove: [ %s ] - [ %s ]", mediatorRequest ? "Mediator" : "Nearby", key ENVIRONS_DEVICE_KEY_EXT );
#endif

		int success = false;

		std::map<APPAREATYPE *, DeviceInstanceNode *, compare_char_key>::iterator foundIt = devicesMapAvailable->find ( key );

        if ( foundIt != devicesMapAvailable->end () ) {
#ifdef USE_MEDIATOR_OPT_KEY_MAPS_COMP
            CListLogArg ( "DeviceRemove: Remove request for device [ %s ] [ %i : %s ]", GET_DEVICE_SOURCE_STRING ( foundIt->second->info.broadcastFound ), foundIt->second->key.deviceID, foundIt->second->key.appArea );
#else
			CListLogArg ( "DeviceRemove: Remove request for device [ %s ] [ %s ]", GET_DEVICE_SOURCE_STRING ( foundIt->second->info.broadcastFound ), foundIt->second->key ENVIRONS_DEVICE_KEY_EXT );
#endif
			DeviceInstanceNode * item = foundIt->second;
			bool erase = false;

			if ( mediatorRequest ) {
				if ( item->info.broadcastFound == DEVICEINFO_DEVICE_MEDIATOR )
					erase = true;
				else if ( item->info.broadcastFound == DEVICEINFO_DEVICE_BROADCAST_AND_MEDIATOR ) {
					item->info.broadcastFound = DEVICEINFO_DEVICE_BROADCAST;

                    if ( notify && subscribedToNotifications )
                        API::onEnvironsNotifierContext1 ( env, item->info.objID, NOTIFY_MEDIATOR_SRV_DEVICE_REMOVED, SOURCE_NATIVE, &item->info, DEVICE_PACKET_SIZE );
				}

				UpdateDirtyFlags ( true, true );
			}
			else {
				if ( item->info.broadcastFound == DEVICEINFO_DEVICE_BROADCAST )
					erase = true;
				else if ( item->info.broadcastFound == DEVICEINFO_DEVICE_BROADCAST_AND_MEDIATOR )
				{
                    // We need to switch
                    item->info.broadcastFound = DEVICEINFO_DEVICE_MEDIATOR;

                    if ( notify && subscribedToNotifications )
                        API::onEnvironsNotifierContext1 ( env, item->info.objID, NOTIFY_MEDIATOR_DEVICE_REMOVED, SOURCE_NATIVE, &item->info, DEVICE_PACKET_SIZE );
				}
			}

            if ( erase ) {
#ifdef USE_MEDIATOR_OPT_KEY_MAPS_COMP
                CListLogArg ( "DeviceRemove: Removing device [ %s ] [ %i : %s ]", GET_DEVICE_SOURCE_STRING ( foundIt->second->info.broadcastFound ), foundIt->second->key.deviceID, foundIt->second->key.appArea );
#else
				CListLogArg ( "DeviceRemove: Removing device [ %s ] [ %s ]", GET_DEVICE_SOURCE_STRING ( foundIt->second->info.broadcastFound ), foundIt->second->key ENVIRONS_DEVICE_KEY_EXT );
#endif
                devicesMapAvailable->erase ( foundIt );

                if ( notify && subscribedToNotifications )
                    API::onEnvironsNotifier1 ( env, item->info.objID, mediatorRequest ? NOTIFY_MEDIATOR_SRV_DEVICE_REMOVED : NOTIFY_MEDIATOR_DEVICE_REMOVED, SOURCE_NATIVE );

                sp ( DeviceController ) deviceSP = item->deviceSP.lock ();
                if ( deviceSP ) {
                    if ( !((DeviceBase *) deviceSP.get ())->SendPing () ) {
                        ((DeviceBase *) deviceSP.get ())->deviceStatus = DeviceStatus::Deleteable;

                        TriggerCleanUpDevices ();
                    }

                    deviceSP.reset ();
                }

                item->mapSP = 0;
			}

			UpdateDirtyFlags ( true, false );
			success = true;
		}

		CVerb ( "DeviceRemove: done" );

		PrintDevicesMap ( devicesMapAvailable, false );

		return success;
	}


	void MediatorClient::UpdateDirtyFlags ( bool available, bool isMediator )
	{
		if ( !LockAcquireA ( devicesCacheLock, "UpdateDirtyFlags" ) )
			return;

		if ( available ) {
			//deviceAvailableCachedCount  = ( int ) devicesMapAvailable->size ();
			deviceAvailableCacheDirty   = true;
		}

		if ( isMediator ) {
			deviceMediatorCachedCount   = deviceMediatorQueryCount;
			deviceMediatorCacheDirty    = true;
		}

		LockReleaseVA ( devicesCacheLock, "UpdateDirtyFlags" );
	}


	bool MediatorClient::UpdateDirtyCaches ( bool available, bool isMediator )
	{
		CListLog ( "UpdateDirtyCaches" );

		bool success = true;

		if ( !LockAcquireA ( devicesCacheLock, "UpdateDirtyCaches" ) )
			return false;

		bool doUpdate = false;

		if ( isMediator && deviceMediatorCacheDirty ) {
			doUpdate = true;
		}
		else {
			if ( available && deviceAvailableCacheDirty )
				doUpdate = true;
		}

		if ( doUpdate ) {
			if ( !LockReleaseA ( devicesCacheLock, "UpdateDirtyCaches" ) )
				return false;

			success = DevicesCacheRebuild ( deviceMediatorCacheDirty, true );
			goto Finish;
		}

		LockReleaseVA ( devicesCacheLock, "UpdateDirtyCaches" );

		CListLog ( "UpdateDirtyCaches: done" );

	Finish:
		return success;
	}


	int MediatorClient::DeviceCompareAndTakeOver ( DeviceInstanceNode * listDevice, DeviceInfo * device, bool notify )
	{
		CVerb ( "DeviceCompareAndTakeOver" );

		int             success         = 0;
		int             notification    = 0;

		DeviceInfo *    info            = &listDevice->info;

		/** isConnected is true if the device is currently in the connected state. */
		bool			isConnected		= info->isConnected; // 1

		char			internalUpdates = info->internalUpdates; // 1

		unsigned int 	ipe				= info->ipe; // 4 The external IP or the IP resolved from the socket address

        unsigned int 	updates			= info->updates; // 4 The broadcast update counter

		/** Used internally by native layer. */
        //char            unused			= info->unused; // 1

        /** Used internally by native layer. */
        unsigned short	flags           = info->flags; // 1

		/** BroadcastFound is a value of DEVICEINFO_DEVICE_* and determines whether the device has been seen on the broadcast channel of the current network and/or from a Mediator service. */
        char			broadcastFound	= info->broadcastFound; // 1

        int				nativeID		= info->nativeID; // 1
        OBJIDType       objID           = info->objID;


		if ( broadcastFound == DEVICEINFO_DEVICE_BROADCAST && device->broadcastFound == DEVICEINFO_DEVICE_MEDIATOR )
		{
			success         = 1;
			ipe             = device->ipe;
            device->objID   = objID;
			notification    = NOTIFY_MEDIATOR_SRV_DEVICE_ADDED;

			// Sync the broadcast flags over the mediator to the other device
			env->asyncWorker.Push ( objID, ASYNCWORK_TYPE_DEVICE_FLAGS_SYNC );
		}
		else if ( broadcastFound == DEVICEINFO_DEVICE_MEDIATOR && device->broadcastFound == DEVICEINFO_DEVICE_BROADCAST )
		{
			success         = -1; // Switch of items in list is required
			//unused   = 0;

            if ( device->objID != 0 && objID < device->objID )
                device->objID   = objID;

            // If we have a mediator device in the map and take over a broadcast device, then take over the update counter from broadcast device
            updates			= device->updates;
			notification    = NOTIFY_MEDIATOR_DEVICE_ADDED;
		}

		if ( success == 0 ) {
			info->broadcastFound    = device->broadcastFound;
            //info->updates			= device->updates; // We must not copy the updateCounter from mediator
            info->flags             = device->flags;
			info->isConnected       = device->isConnected;
			info->internalUpdates   = device->internalUpdates;
            info->ipe               = device->ipe;
            device->objID           = objID;

            device->nativeID        = info->nativeID; // Otherwise we reset the nativeID within the native layer

			if ( memcmp ( info, device, DEVICE_PACKET_SIZE ) != 0 ) {
				success = 1; // Change detected
			}
		}
		else {
			broadcastFound = DEVICEINFO_DEVICE_BROADCAST_AND_MEDIATOR;

#ifdef USE_MEDIATOR_OPT_KEY_MAPS_COMP
            CListLogArg ( "DeviceCompareAndTakeOver: Updating to [Mediator+Nearby] device [ %i : %s ]", listDevice->key.deviceID, listDevice->key.appArea );
#else
			CListLogArg ( "DeviceCompareAndTakeOver: Updating to [Mediator+Nearby] device [ %s ]", listDevice->key ENVIRONS_DEVICE_KEY_EXT );
#endif
		}

        if ( success == 1 ) {
#ifdef USE_MEDIATOR_OPT_KEY_MAPS_COMP
            CListLogArg ( "DeviceCompareAndTakeOver: Updating [ %s ] device [ %i : %s ]", GET_DEVICE_SOURCE_STRING ( info->broadcastFound ), listDevice->key.deviceID, listDevice->key.appArea );
#else
            CListLogArg ( "DeviceCompareAndTakeOver: Updating [ %s ] device [ %s ]", GET_DEVICE_SOURCE_STRING ( info->broadcastFound ), listDevice->key ENVIRONS_DEVICE_KEY_EXT );
#endif

			memcpy ( info, device, DEVICE_MEDIATOR_PACKET_SIZE );
		}

		info->broadcastFound    = broadcastFound;
		info->isConnected       = isConnected;
		info->internalUpdates   = internalUpdates;
        info->updates			= updates;
        info->flags             = flags;
        info->ipe               = ipe;
        info->nativeID          = nativeID;
        info->objID             = objID;

        if ( success == -1 ) {
#ifdef USE_MEDIATOR_OPT_KEY_MAPS_COMP
            CListLogArg ( "DeviceCompareAndTakeOver: Updating [ %s ] device [ %i : %s ]", GET_DEVICE_SOURCE_STRING ( info->broadcastFound ), listDevice->key.deviceID, listDevice->key.appArea );
#else
            CListLogArg ( "DeviceCompareAndTakeOver: Updating [ %s ] device [ %s ]", GET_DEVICE_SOURCE_STRING ( info->broadcastFound ), listDevice->key ENVIRONS_DEVICE_KEY_EXT );
#endif

			memcpy ( device, info, DEVICE_MEDIATOR_PACKET_SIZE );
		}

        if ( success != 0 && notify && notification && subscribedToNotifications ) {
            API::onEnvironsNotifierContext1 ( env, info->objID, notification, SOURCE_NATIVE, info, DEVICE_PACKET_SIZE );
			//API::onEnvironsNotifierContext1 ( env, info->deviceID, info->areaName, info->appName, notification, info, DEVICE_PACKET_SIZE );
		}

		return success;
	}


	int MediatorClient::DeviceChange ( std::map<APPAREATYPE *, DeviceInstanceNode *, compare_char_key>::iterator &foundIt, DeviceInstanceNode * nearbyDevice, DeviceInfo * mediatorDevice )
	{
		int             success = 0;
		DeviceInfo  *   device  = ( nearbyDevice ? &nearbyDevice->info : mediatorDevice );

#ifdef USE_MEDIATOR_OPT_KEY_MAPS_COMP
        CListLogArg ( "DeviceChange: Comparing the device found [ %s ] with the incoming device [ %i : %s ]", GET_DEVICE_SOURCE_STRING ( foundIt->second->info.broadcastFound ), nearbyDevice ? nearbyDevice->key.deviceID : 0, nearbyDevice ? nearbyDevice->key.appArea : "Mediator device" );
#else
		CListLogArg ( "DeviceChange: Comparing the device found [ %s ] with the incoming device [ %s ]", GET_DEVICE_SOURCE_STRING ( foundIt->second->info.broadcastFound ), nearbyDevice ? nearbyDevice->key ENVIRONS_DEVICE_KEY_EXT : "Mediator device" );
#endif
		int status = DeviceCompareAndTakeOver ( foundIt->second, device, true );

		if ( status == 1 ) {
			success = true;
		}
		else if ( status == -1 ) {
            if ( nearbyDevice ) {
#ifdef USE_MEDIATOR_OPT_KEY_MAPS_COMP
                CListLogArg ( "DeviceChange: Replacing Mediator device with Nearby instance [ %i : %s ]", nearbyDevice->key.deviceID, nearbyDevice->key.appArea );
#else
                CListLogArg ( "DeviceChange: Replacing Mediator device with Nearby instance [ %s ]", nearbyDevice->key ENVIRONS_DEVICE_KEY_EXT );
#endif
                DeviceInstanceNode  * toDelete = foundIt->second;

                int nativeID = toDelete->info.nativeID;

                //nearbyDevice->info.unused |= toDelete->info.unused;
                nearbyDevice->info.flags |= toDelete->info.flags;

                if ( LockAcquireA ( devicesLock, "DeviceChange" ) )
                {
                    sp ( DeviceController ) deviceSP = toDelete->deviceSP.lock ();
                    if ( deviceSP ) {
                        DeviceBase * deviceBase     = ( DeviceBase * ) deviceSP.get();

                        if ( LockAcquireA ( deviceBase->spLock, "DeviceChange" ) )
                        {
                            deviceBase->deviceNode      = nearbyDevice->baseSP;

                            if ( deviceBase->deviceNode )
                                deviceBase->objID       = deviceBase->deviceNode->info.objID;

                            LockReleaseVA ( deviceBase->spLock, "DeviceChange" );
                        }

                        nearbyDevice->deviceSP      = deviceSP;

                        toDelete->deviceSP.reset ();
                    }

                    devicesMapAvailable->erase ( foundIt );

                    toDelete->mapSP.reset ();

                    nearbyDevice->mapSP = nearbyDevice->baseSP;

                    LockReleaseVA ( devicesLock, "DeviceChange" );

                    if ( nearbyDevice->mapSP ) {
                        if ( nativeID > 0 )
                            nearbyDevice->info.nativeID = nativeID;

#ifdef USE_MEDIATOR_OPT_KEY_MAPS_COMP
                        (*devicesMapAvailable) [ &nearbyDevice->key ] = nearbyDevice;
#else
                        (*devicesMapAvailable) [ ( APPAREATYPE  * ) nearbyDevice->key ] = nearbyDevice;
#endif
                    }
                }
                else {
                    CErr ( "DeviceChange: Failed to acquire devicesLock." );
                }
            }
            else {
                CErr ( "DeviceChange: Need to Replace Mediator device with Nearby instance. However, nearby instance is invalid." );
            }
		}

		CVerb ( "DeviceChange: done" );

		PrintDevicesMap ( devicesMapAvailable, false );

		return success;
	}


	int MediatorClient::DeviceChange ( DeviceInstanceNode * nearbyDevice, DeviceInfo * mediatorDevice )
	{
		CListLog ( "DeviceChange" );

		int         success = 0;

        int         length;

#ifdef USE_MEDIATOR_OPT_KEY_MAPS_COMP
        AppAreaKey  *   key     = 0;
        AppAreaKey      deviceKey;
        AppAreaKey  *   key1    = &deviceKey;

        if ( nearbyDevice )
            key = &nearbyDevice->key;
#else
        char    *   key     = 0;
        char        key1 [ MAX_DEVICE_INSTANCE_KEY_LENGTH ];

        if ( nearbyDevice )
            key = nearbyDevice->key;
#endif
		else
		{
            if ( !mediatorDevice )
                return false;

			length = BuildMapKey ( key1, mediatorDevice );
			if ( length <= 0 ) {
				return false;
			}
			key = key1;
		}
		CListLogArg ( "DeviceChange: [ %s ]", key );

		std::map<APPAREATYPE *, DeviceInstanceNode *, compare_char_key>::iterator foundIt = devicesMapAvailable->find ( key );

		if ( foundIt != devicesMapAvailable->end () )
		{
			success = DeviceChange ( foundIt, nearbyDevice, mediatorDevice );

			UpdateDirtyFlags ( true, true );
		}
		else {
			CListLogArg ( "DeviceChange: Not found [ %s ]", key );
			success = -1;
		}

		CVerb ( "DeviceChange: done" );
		return success;
	}


	int MediatorClient::DeviceAdd ( DeviceInstanceNode * nearbyDevice, DeviceInfo * mediatorDevice )
	{
		CVerb ( "DeviceAdd" );

        int             success		= false;

#ifdef USE_MEDIATOR_OPT_KEY_MAPS_COMP
        AppAreaKey  *   key         = 0;
#else
		char        *   key			= 0;
#endif
        DeviceInstanceNode  *       item    = 0;
        sp ( DeviceInstanceNode )   itemSP  = 0;
        DeviceInfo          *       device  = 0;

#ifdef DEBUGVERB
        int deviceID	= 0;
#endif

		std::map<APPAREATYPE *, DeviceInstanceNode *, compare_char_key>::iterator foundIt;

        if ( nearbyDevice ) {
#ifdef USE_MEDIATOR_OPT_KEY_MAPS_COMP
            key			= &nearbyDevice->key;
#else
			key			= nearbyDevice->key;
#endif
			device		= &nearbyDevice->info;
		}
		else {
			device = mediatorDevice;

			itemSP = BuildDeviceListItem ( device, false );
			if ( !itemSP ) {
				CErr ( "DeviceAdd: Failed to allocate memory for device item!" );
				success = false; goto Finish;
			}

            item = itemSP.get();

#ifdef USE_MEDIATOR_OPT_KEY_MAPS_COMP
            key = &item->key;
#else
			key = item->key;
#endif
			item->hEnvirons = env->hEnvirons;
		}

#ifdef USE_MEDIATOR_OPT_KEY_MAPS_COMP
        CListLogArg ( "DeviceAdd: [ %i : %s ]", key->deviceID, key->appArea );
#else
		CListLogArg ( "DeviceAdd: [ %s ]", key ENVIRONS_DEVICE_KEY_EXT );
#endif

#ifdef DEBUGVERB
        deviceID	= device->deviceID;
#endif
		foundIt = devicesMapAvailable->find ( key );

		if ( foundIt != devicesMapAvailable->end () )
		{
			success = DeviceChange ( foundIt, nearbyDevice, mediatorDevice );
		}
		else {
            /// Add the item
#ifdef USE_MEDIATOR_OPT_KEY_MAPS_COMP
            CListLogArg ( "DeviceAdd: Adding [ %s ] device [ %i : %s ]", GET_DEVICE_SOURCE_STRING ( device->broadcastFound ), key->deviceID, key->appArea );
#else
            CListLogArg ( "DeviceAdd: Adding [ %s ] device [ %s ]", GET_DEVICE_SOURCE_STRING ( device->broadcastFound ), key ENVIRONS_DEVICE_KEY_EXT );
#endif
			if ( item ) {
				memcpy ( &item->info, device, DEVICE_MEDIATOR_PACKET_SIZE );

				item->info.broadcastFound   = DEVICEINFO_DEVICE_MEDIATOR;
				item->info.internalUpdates  = ( char ) devicesMapUpdates;
                //item->info.unused			= 0;
                item->info.flags            = DeviceFlagsInternal::NativeReady;

                item->mapSP                 = itemSP;

                if ( item->mapSP ) {
                    (*devicesMapAvailable) [ key ] = item;

                    if ( subscribedToNotifications )
                        API::onEnvironsNotifierContext1 ( env, item->info.objID, NOTIFY_MEDIATOR_SRV_DEVICE_ADDED, SOURCE_NATIVE, &item->info, DEVICE_PACKET_SIZE );

                    env->asyncWorker.Push ( item->info.objID, ASYNCWORK_TYPE_DEVICE_FLAGS_SYNC );

                    UpdateDirtyFlags ( true, true );
                }
			}
			else {
				device->broadcastFound  = DEVICEINFO_DEVICE_BROADCAST;
				device->internalUpdates = ( char ) devicesMapUpdates;
                //device->unused			= 0;
                device->flags           = DeviceFlagsInternal::NativeReady;

                if ( nearbyDevice ) {
                    if ( LockAcquireA ( devicesLock, "DeviceAdd" ) )
                    {
                        nearbyDevice->mapSP = nearbyDevice->baseSP;

                        LockReleaseVA ( devicesLock, "DeviceAdd" );

                        if ( nearbyDevice->mapSP ) {
                            (*devicesMapAvailable) [ key ] = nearbyDevice;

                            if ( subscribedToNotifications )
                                API::onEnvironsNotifierContext1 ( env, device->objID, NOTIFY_MEDIATOR_DEVICE_ADDED, SOURCE_NATIVE, device, DEVICE_PACKET_SIZE );

                            env->asyncWorker.Push ( nearbyDevice->info.objID, ASYNCWORK_TYPE_DEVICE_FLAGS_SYNC );
                        }
                    }
                    else {
                        CErr ( "DeviceAdd: Failed to acquire devicesLock" );
                    }
				}

				UpdateDirtyFlags ( true, false );
			}
			success = true;
		}

    Finish:

		CVerbID ( "DeviceAdd: done" );

		PrintDevicesMap ( devicesMapAvailable, false );

		return success;
	}


	void MediatorClient::UpdateDeviceState ( DeviceBase * device, int nativeID )
	{
		if ( !device )
			return;

		sp ( Instance ) envObj = env->myself;
		if ( !envObj ) {
			CVerbArg ( "UpdateDeviceState: Ignoring update of nativeID [ %i ]. Missing environs object.", nativeID );
			return;
		}

		if ( envObj->environsState < environs::Status::Starting ) {
			CVerbArg ( "UpdateDeviceState: Ignoring update of nativeID [ %i ]. Environs state < starting", nativeID );
			return;
		}

        int     length;

#ifdef USE_MEDIATOR_OPT_KEY_MAPS_COMP
        AppAreaKey deviceKey;
        AppAreaKey * key = &deviceKey;
#else
        char    key [ MAX_DEVICE_INSTANCE_KEY_LENGTH ];
#endif
		char *	areaName	= device->deviceAreaName;
		char *	appName		= device->deviceAppName;

		if ( !areaName )
			areaName = env->areaName;
		if ( !appName )
			appName = env->appName;

		length = BuildMapKey ( key, device->deviceID, areaName, appName );
		if ( length <= 0 ) {
			CErr ( "UpdateDeviceState: Failed to build the key!" );
			return;
		}

		if ( !LockAcquireA ( devicesMapLock, "UpdateDeviceState" ) )
			return;

		std::map<APPAREATYPE *, DeviceInstanceNode *, compare_char_key>::iterator foundIt = devicesMapAvailable->find ( key );

		if ( foundIt != devicesMapAvailable->end () )
        {
#ifdef USE_MEDIATOR_OPT_KEY_MAPS_COMP
            CVerbVerbArg ( "UpdateDeviceState: Found [ %i : %s ]", key->deviceID, key->appArea );
#else
			CVerbVerbArg ( "UpdateDeviceState: Found [ %i : %s ]", key ENVIRONS_DEVICE_KEY_EXT );
#endif
			DeviceInstanceNode * item = foundIt->second;

			//bool changed = false;
			bool isConnected = ( device->deviceStatus == DeviceStatus::Connected ? true : false );

			//if ( item->info.isConnected != isConnected ) {
				item->info.isConnected = isConnected;
            //    changed = true;
			//}

			//if ( item->info.nativeID != nativeID ) {
                item->info.nativeID = nativeID;
                //changed = true;
            //}

			if ( /*changed &&*/ subscribedToNotifications ) {
				int notify = ( item->info.broadcastFound == DEVICEINFO_DEVICE_BROADCAST ? NOTIFY_MEDIATOR_DEVICE_CHANGED : NOTIFY_MEDIATOR_SRV_DEVICE_CHANGED );

                API::onEnvironsNotifierContext1 ( env, item->info.objID, notify, SOURCE_NATIVE, &item->info, DEVICE_PACKET_SIZE );
				//API::onEnvironsNotifierContext ( env, item->info.deviceID, item->info.areaName, item->info.appName, notify, &item->info, DEVICE_PACKET_SIZE );
			}
		}
#ifndef NDEBUG
        else {
#ifdef USE_MEDIATOR_OPT_KEY_MAPS_COMP
            CWarnsArg ( 4, "UpdateDeviceState: Failed to lookup device [ %i : %s ]!", key->deviceID, key->appArea );
#else
            CWarnsArg ( 4, "UpdateDeviceState: Failed to lookup device [ %s ]!", key ENVIRONS_DEVICE_KEY_EXT );
#endif
		}
#endif

		LockReleaseVA ( devicesMapLock, "UpdateDeviceState" );
    }


#if defined(PRINT_DEVICE_LIST)
	void PrintDeviceList ( const char * listName, void * deviceList )
	{
		if ( !deviceList ) {
			CLogArg ( "%s: 0 devices", listName );
			return;
		}

		DeviceInfoHeaderedPackage * list = ( DeviceInfoHeaderedPackage * ) deviceList;

		DeviceInfo * device = &list->device;
		int count = ( int ) list->header.deviceCount;

		for ( int i = 1; i <= count; i++ ) {
			CLogArg ( "%s [%i/%i] deviceID [%i/%i]    name [ %s ]  area [ %s ]  app [ %s ]", listName, i, count, device->deviceID, device->broadcastFound, device->deviceName, device->areaName, device->appName );
			device++;
		}
	}
#else
#define PrintDeviceList(a,b)
#endif


	int MediatorClient::GetDevicesFromMediatorCountCached ()
	{
		CVerb ( "GetDevicesFromMediatorCountCached" );

		return deviceMediatorCachedCount;
	}


	int MediatorClient::GetDevicesFromMediatorCached ( char * buffer, int bufferSize, int startIndex )
	{
		CVerb ( "GetDevicesFromMediatorCached" );

		if ( !buffer || bufferSize <= 0 )
			return 0;

		int reqSize;
		int deviceCount = 0;

		if ( !UpdateDirtyCaches ( false, true ) ) {
			CErr ( "GetDevicesFromMediatorCached: Failed to update cache!" );
			return 0;
		}

		if ( !LockAcquireA ( devicesCacheLock, "GetDevicesFromMediatorCached" ) )
			return 0;

		if ( deviceMediatorCachedCount && deviceMediatorCached )
		{
			reqSize = ( deviceMediatorCachedCount * DEVICE_PACKET_SIZE ) + ( 2 * DEVICES_HEADER_SIZE );

			if ( reqSize <= bufferSize )
			{
				memcpy ( buffer, deviceMediatorCached, reqSize );

				deviceCount = deviceMediatorCachedCount;
			}
		}

		LockReleaseVA ( devicesCacheLock, "GetDevicesFromMediatorCached" );

		PrintDeviceList ( "MediatorC: ", buffer );

		return deviceCount;
	}


	int MediatorClient::GetDeviceFromMediatorCached ( char * buffer, int bufferSize, int deviceID, const char * areaName, const char * appName )
	{
		CVerb ( "GetDeviceFromMediatorCached" );

		if ( !buffer || bufferSize <= 0 )
			return 0;

		int reqSize = sizeof ( DevicePack );
		if ( reqSize > bufferSize )
			return 0;

        DeviceHeader *	package     = ( DeviceHeader * ) buffer;
        DeviceInfo   *	info        = ( DeviceInfo * ) (buffer + DEVICES_HEADER_SIZE);
		int             deviceCount = 0;
        int             length;

#ifdef USE_MEDIATOR_OPT_KEY_MAPS_COMP
        AppAreaKey      deviceKey;
        AppAreaKey   *  key         = &deviceKey;
#else
        char            key [ MAX_DEVICE_INSTANCE_KEY_LENGTH ];
#endif
		if ( !areaName )
			areaName = env->areaName;
		if ( !appName )
			appName = env->appName;

		length = BuildMapKey ( key, deviceID, areaName, appName );
		if ( length <= 0 ) {
			return 0;
		}

		if ( !LockAcquireA ( devicesMapLock, "GetDeviceFromMediatorCached" ) )
			return 0;

		std::map<APPAREATYPE *, DeviceInstanceNode *, compare_char_key>::iterator foundIt = devicesMapAvailable->find ( key );

		if ( foundIt != devicesMapAvailable->end () )
        {
#ifdef USE_MEDIATOR_OPT_KEY_MAPS_COMP
            CVerbArg ( "GetDeviceFromMediatorCached: Found [ %i : %s ]", key->deviceID, key->appArea );
#else
			CVerbArg ( "GetDeviceFromMediatorCached: Found [ %s ]", key ENVIRONS_DEVICE_KEY_EXT );
#endif
			memcpy ( info, &foundIt->second->info, DEVICE_PACKET_SIZE );

			deviceCount = 1;
			package->deviceCount = 1;
		}

		LockReleaseVA ( devicesMapLock, "GetDeviceFromMediatorCached" );

		PrintDeviceList ( "MediatorCID: ", buffer );

		return deviceCount;
	}


	int MediatorClient::GetDevicesAvailableCachedBestMatch ( char ** buffer, int bufferSize, int deviceID )
	{
		CVerbID ( "GetDevicesAvailableCachedBestMatch" );

		int reqSize = sizeof ( DevicePack );

		if ( bufferSize > 0 && ( reqSize > bufferSize ) )
			return 0;

		if ( bufferSize > 0 && !buffer )
			return 0;

		if ( !UpdateDirtyCaches ( true, false ) ) {
			CErr ( "GetDevicesAvailableCachedBestMatch: Failed to update cache!" );
			return 0;
		}

		if ( !LockAcquireA ( devicesCacheLock, "GetDevicesAvailableCachedBestMatch" ) )
			return 0;

		DeviceInfo * device;

		int deviceCount = 0;
		int cacheCount	= deviceAvailableCachedCount;

		if ( cacheCount <= 0 || !deviceAvailableCached )
			goto Finish;

		device = ( DeviceInfo * ) ( deviceAvailableCached + DEVICES_HEADER_SIZE );

		for ( int i = 0; i < cacheCount; ++i, ++device )
		{
			//CLogArg ( "GetDevicesMediator: Device [%d] ID [%d] of type [%c] and name [%s].", i, device->ID, device->deviceType, device->deviceName );

			if ( device->deviceID == deviceID )
			{
				char * foundPack = ( char * ) *buffer;

				if ( !foundPack ) {
					foundPack = ( char * ) malloc ( reqSize );
					if ( !foundPack )
						goto Finish;
					*buffer = ( char * ) foundPack;
                }

                DeviceInfo * info = ( DeviceInfo * ) (foundPack + DEVICES_HEADER_SIZE);

				memcpy ( info, device, DEVICE_PACKET_SIZE );

                deviceCount = 1;

                DeviceHeader * header = ( DeviceHeader * ) foundPack;

				header->deviceCountAvailable = 1;
				header->startIndex = 0;
				header->deviceCount = 1;

				CLogArgID ( "GetDevicesAvailableCachedBestMatch: found p [ %s ] - a [ %s ]", ( *device->areaName ) ? device->areaName : "",
					( *device->appName ) ? device->appName : "" );
				break;
			}
		}

	Finish:
		LockReleaseVA ( devicesCacheLock, "GetDevicesAvailableCachedBestMatch" );

		PrintDeviceList ( "AvailableBestMatch: ", buffer );

		return deviceCount;
    }


    int MediatorClient::GetDevicesAvailableCached ( char ** buffer, int bufferSize, OBJIDType objID )
    {
        CVerb ( "GetDevicesAvailableCached" );

        int reqSize = sizeof ( DevicePack );

        if ( bufferSize > 0 && ( reqSize > bufferSize ) )
            return 0;

        if ( bufferSize > 0 && !buffer )
            return 0;

        if ( !UpdateDirtyCaches ( true, false ) ) {
            CErr ( "GetDevicesAvailableCached: Failed to update cache!" );
            return 0;
        }

        if ( !LockAcquireA ( devicesCacheLock, "GetDevicesAvailableCached" ) )
            return 0;

        DeviceInfo * device;

        int deviceCount = 0;
		int cacheCount	= deviceAvailableCachedCount;

        if ( cacheCount <= 0 || !deviceAvailableCached )
            goto Finish;

		device = ( DeviceInfo * ) ( deviceAvailableCached + DEVICES_HEADER_SIZE );

        for ( int i = 0; i < cacheCount; ++i, ++device )
        {
            //CLogArg ( "GetDevicesMediator: Device [%d] ID [%d] of type [%c] and name [%s].", i, device->ID, device->deviceType, device->deviceName );

            if ( device->objID == objID )
            {
                char * foundPack = ( char * ) *buffer;

                if ( !foundPack ) {
                    foundPack = ( char * ) malloc ( reqSize );
                    if ( !foundPack )
                        goto Finish;
                    *buffer = ( char * ) foundPack;
                }

                DeviceInfo * info = ( DeviceInfo * ) (foundPack + DEVICES_HEADER_SIZE);

                memcpy ( info, device, DEVICE_PACKET_SIZE );

                deviceCount = 1;

                DeviceHeader * header = ( DeviceHeader * ) foundPack;

                header->deviceCountAvailable = 1;
                header->startIndex = 0;
                header->deviceCount = 1;

                CVerbArg ( "GetDevicesAvailableCached: found p [ %s ] - a [ %s ]", *device->areaName ? device->areaName : "", *device->appName ? device->appName : "" );
                break;
            }
        }

    Finish:
        LockReleaseVA ( devicesCacheLock, "GetDevicesAvailableCached" );

        PrintDeviceList ( "AvailableBestMatch: ", buffer );

        return deviceCount;
    }


	int MediatorClient::GetDevicesAvailableCountCached ()
	{
		CVerb ( "GetDevicesAvailableCountCached" );

		return deviceAvailableCachedCount;
	}


	jobject MediatorClient::GetDevicesAvailableCached ( JNIEnv * jenv )
	{
		CVerb ( "GetDevicesAvailableCached" );

		if ( !UpdateDirtyCaches ( true, false ) ) {
			CErr ( "GetDevicesAvailableCached: Failed to update cache!" );
			return 0;
		}

		if ( !LockAcquireA ( devicesCacheLock, "GetDevicesAvailableCached" ) )
			return 0;

		jobject			byteBuffer  = 0;
		unsigned int    reqSize;

        int cacheCount = deviceAvailableCachedCount;

		if ( cacheCount > 0 && deviceAvailableCached )
		{
			reqSize = ( cacheCount * DEVICE_PACKET_SIZE ) + ( 2 * DEVICES_HEADER_SIZE );

			char * tmp = 0;
#ifdef ANDROID
			byteBuffer = allocJByteBuffer ( jenv, reqSize, tmp );
#else
			byteBuffer = malloc ( reqSize ); tmp = ( char * ) byteBuffer;
#endif
			if ( !tmp ) {
				CErrArg ( "GetDevicesN: Failed to allocate buffer for [ %d ] devices with [ %u bytes ]!", cacheCount, reqSize );
			}
			else {
				memcpy ( tmp, deviceAvailableCached, reqSize );
			}
		}

		LockReleaseA ( devicesCacheLock, "GetDevicesAvailableCached" );

		PrintDeviceList ( "AvailableC: ", ( char * ) byteBuffer );

		return byteBuffer;
	}


	int MediatorClient::GetDevicesFromMediatorCount ()
	{
		CVerb ( "GetDevicesFromMediatorCount" );

		if ( !GetAvailableMediator () || env->environsState < environs::Status::Starting )
			return -1;

		MediatorQueryMsgV6		query;
		MediatorQueryHeaderV6 *	header = &query.header;

		// Buffer must contain size, count of devices, count of devices in this response, and whether there are more devices (in this case, we must request subsequent devices somehow?)
		header->size		= MEDIATOR_CMD_GET_DEVICES_COUNT_LEN_V6;
		header->cmdVersion	= MEDIATOR_PROTOCOL_VERSION;
		header->cmd1		= MEDIATOR_CMD_DEVICE_LIST_QUERY;
		header->opt0		= MEDIATOR_CMD_DEVICE_LIST_QUERY_COUNT;

		//header->opt1	= MEDIATOR_OPT_NULL;
		if ( env->kernel && env->kernel->appStatus == APP_STATUS_SLEEPING )
			header->opt1	= MEDIATOR_FILTER_ALL;
		else
			header->opt1	= ( char ) env->mediatorFilterLevel;
		header->msgID		= env->deviceID;

		if ( !SendMessageToMediator ( &query, true, sizeof ( MediatorQueryMsgV6 ) ) ) {
			CVerb ( "GetDevicesFromMediatorCount: Failed to load devices count." );
			return -1;
		}
		if ( header->size < MEDIATOR_CMD_GET_DEVICES_COUNT_LEN_V6 ) {
			CVerb ( "GetDevicesFromMediatorCount: Invalid response. Size of response is less than 8" );
			return -1;
		}

		return header->msgID;
	}


	int MediatorClient::GetDevicesFromMediator ( char *& retBuffer, int deviceID, const char * areaName, const char * appName )
	{
		CVerb ( "GetDevicesFromMediator" );

		if ( env->environsState < environs::Status::Starting )
            return 0;

        // Get buffer of Mediator
        MediatorInstance * med = GetAvailableMediator ();
        if ( !med )
            return -1;

		int		deviceCount			= 0;
		int		deviceCountInBuffer	= 0;

		// Fake one device at mediator
		int		devicesAtMediator	= 0;

		int     maxDeviceToLoad     = 0;
		
		if ( deviceID )
			maxDeviceToLoad = 1; // If we query for a certain device, then prepare buffer size for one device
		else
			maxDeviceToLoad = deviceMediatorQueryCount; // Otherwise, use the latest query count that we did or prepared by the caller

        if ( maxDeviceToLoad <= 0 )
            return 0;

        if ( maxDeviceToLoad > 5000 ) { /// 126 Bytes * 5000 = ~600kb
            maxDeviceToLoad = 5000;
            CWarn ( "GetDevicesFromMediator: More than 5000 mediator devices available! Limiting request to 5000." );
        }

        int     bufferSize          = ( deviceID ? sizeof(DevicePack) : ( ( maxDeviceToLoad * DEVICE_MEDIATOR_PACKET_SIZE ) + ( 2 * DEVICES_HEADER_SIZE_V6 ) ) );

        char *  buffer              = (char *) calloc ( 1, bufferSize );
        if ( !buffer )
            return -1;

        char *	pCurDevice			= buffer;
        int		devicesLeftInBuffer = ( bufferSize - DEVICES_HEADER_SIZE_V6 ) / DEVICE_MEDIATOR_PACKET_SIZE;
        int		remainBufferSize	= bufferSize;
        int		length;

		MediatorQueryMsgV6      *   query;
		MediatorQueryHeaderV6   *   header;
		MediatorQueryResponseV6 *   resp;
		DeviceHeader            *   deviceHead;

		char headerBuffer [ DEVICES_HEADER_SIZE_V6 ];

		//
		// Do until all devices have been received
		//
	LoadNextDevices:
		if ( devicesLeftInBuffer <= 0 || deviceCountInBuffer > devicesAtMediator )
			goto FinalizeRequest;

		query               = ( MediatorQueryMsgV6 * ) pCurDevice;
		header              = ( MediatorQueryHeaderV6 * ) query;
        
		header->cmdVersion  = MEDIATOR_PROTOCOL_VERSION;
		header->cmd1        = MEDIATOR_CMD_DEVICE_LIST_QUERY;
		header->opt0        = MEDIATOR_OPT_NULL;
        header->resultCount = devicesLeftInBuffer;
		header->opt1        = ( char ) env->mediatorFilterLevel;
		header->msgID		= env->deviceID;

		if ( deviceID ) {
            header->opt0		= MEDIATOR_CMD_DEVICE_LIST_QUERY_SEARCH; // MEDIATOR_OPT_DEVICE_LIST_DEVICE_ID must be given if we want to do a list query that starts with a deviceID
			header->deviceID	= deviceID;

			if ( areaName && appName && *areaName && *appName ) {
				if ( !BuildAppAreaField ( header->sizes, appName, areaName, false ) )
					goto Failed;
			}
			else {
				header->sizes [ 0 ] = 1; header->sizes [ 1 ] = 1;
            }
            
            header->size = sizeof ( MediatorQueryHeaderV6 ) + header->sizes [ 0 ] + header->sizes [ 1 ];
		}
        else
            header->size = sizeof ( MediatorQueryHeaderV6 );

        CListenerLog ( 6, "GetDevicesFromMediator: Sending packet" );

        if ( !SendMessageToMediator ( query, true, remainBufferSize, maxDeviceToLoad <= 2 ? 10000 : 15000 ) ) {
            CErrArg ( "GetDevicesFromMediator: Failed to load [ %i ] devices. bufferSize [ %i ]", devicesLeftInBuffer, bufferSize );
            
            // Something went wrong ... Invoke a reload of the whole list
            DevicesHasChanged ( MEDIATOR_DEVICE_RELOAD );
            goto Failed;
		}

        if ( header->size < DEVICES_HEADER_SIZE_V6 ) {
            if ( env->environsState >= environs::Status::Starting ) {
                CErrArg ( "GetDevicesFromMediator: Invalid response. Header is missing. Size [ %u ]", header->size );
            }
            goto Failed;
        }

        if ( header->cmd1 != MEDIATOR_CMD_DEVICE_LIST_QUERY_RESPONSE ) {
            if ( env->environsState >= environs::Status::Starting ) {
                CErrArg ( "GetDevicesFromMediator: Error. (Not a query result) [ %c : %c : %u bytes ]", header->cmd1, header->opt0, header->size );
            }
            goto Failed;
        }

		resp                = ( MediatorQueryResponseV6 * ) pCurDevice;

		//
		devicesAtMediator	= resp->deviceHead.deviceCountAvailable;

		deviceCount			= resp->deviceHead.deviceCount;

		// Restore devices in the temporary buffer if required
		if ( pCurDevice != buffer )
			memcpy ( pCurDevice, headerBuffer, DEVICES_HEADER_SIZE_V6 );

		CVerbArg ( "GetDevicesFromMediator: Received [ %d ] devices starting at [ %d ] of [ %d ] available devices.", deviceCount, resp->deviceHead.startIndex, devicesAtMediator );

		if ( deviceCount <= 0 )
			goto FinalizeRequest;

		if ( deviceCount > 5000 ) {
			CErr ( "GetDevicesFromMediator: Invalid response (deviceCount > 5000 or < 0) occured." );
			goto Failed;
		}

		if ( deviceCount > devicesLeftInBuffer )
			deviceCount = devicesLeftInBuffer;

		deviceCountInBuffer += deviceCount;
		devicesLeftInBuffer -= deviceCount;

		length = deviceCount * DEVICE_MEDIATOR_PACKET_SIZE;

		//memcpy ( pCurDevice, medBuffer + DEVICES_HEADER_SIZE_V6, length );

		pCurDevice += length;
		remainBufferSize -= length;

		// backup header in the temporary header
		memcpy ( headerBuffer, pCurDevice, DEVICES_HEADER_SIZE_V6 );

		goto LoadNextDevices;

	FinalizeRequest:
        if ( deviceCountInBuffer > 0 )
        {
            if ( deviceCountInBuffer > 5000 ) {
                CErr ( "GetDevicesFromMediator: Invalid response (deviceCount > 5000)." );
                goto Failed;
            }
            
            pCurDevice			= buffer + DEVICES_HEADER_SIZE_V6;

            if ( deviceCountInBuffer == 1 )
            {
                DeviceInfo * device = ( DeviceInfo * ) pCurDevice;

                // Make sure that strings are 0 terminated
                device->appName		[ MAX_LENGTH_APP_NAME - 1 ]		= 0;
                device->areaName	[ MAX_LENGTH_AREA_NAME - 1 ]	= 0;
                device->deviceName	[ MAX_LENGTH_DEVICE_NAME - 1 ]	= 0;

				// Check whether the device represents ourself
				//CVerbArg ( "GetDevicesFromMediator: Received deviceID [ 0x%X ]  [ %s / %s ]", device->deviceID, device->appName, device->areaName );
				//CVerbArg ( "GetDevicesFromMediator: We are   deviceID [ 0x%X ]  [ %s / %s ]", env->deviceID, env->appName, env->areaName );

                if ( !device->deviceID || 
					(	device->deviceID == env->deviceID
						&& 
						( !strncmp ( device->areaName, env->areaName, sizeof ( device->areaName ) )
							&& !strncmp ( device->appName, env->appName, sizeof ( device->appName ) ) 
						)
					)
				   )
                {
                    // Save the external IP determined by the Mediator
                    if ( device->deviceID  ) {
                        IPe = device->ipe;
#ifdef ENABLE_EXT_BIND_IN_STUNT
                        MatchExtIPWithInterfaces ();
#endif
                    }

                    // We skip ourself in the list. Update counters accordingly.
                    deviceCountInBuffer--;
                    devicesAtMediator--;
                }
                else {
                    device->objID = 0;

					if ( device->hasAppEnv ) {
						char *  app         = device->appName;
						char *  area        = device->areaName;

						// Make sure that strings are 0 terminated
						app  [ MAX_LENGTH_APP_NAME - 1 ]	= 0;
						area [ MAX_LENGTH_AREA_NAME - 1 ]	= 0;

						device->hasAppEnv = !( !strncmp ( area, env->areaName, sizeof ( env->areaName ) ) && !strncmp ( app, env->appName, sizeof ( env->appName ) ) );
					}
                }
            }
            else
            {
				//CVerbArg ( "GetDevicesMediator: Parsing [ %d ] devices.", deviceCountInBuffer );

                bufferSize          = ( ( deviceCountInBuffer * DEVICE_PACKET_SIZE ) + ( 2 * DEVICES_HEADER_SIZE_V6 ) );
#ifdef NDEBUG
                char * medDevices   = ( char * ) malloc ( bufferSize );
#else
				char * medDevices   = ( char * ) malloc ( bufferSize );
				//char * medDevices   = ( char * ) calloc ( 1, bufferSize );
#endif
                if ( medDevices )
                {
					DeviceInfo * medDevice = ( DeviceInfo * ) ( medDevices + DEVICES_HEADER_SIZE_V6 );
                    
                    bool    sameAppArea = false;
                    char *  app         = 0;
                    char *  area        = 0;

                    size_t  appLen      = 0;
                    size_t  areaLen     = 0;
                    int     skips       = 0;

                    for ( int i = 0; i < deviceCountInBuffer; i++ )
                    {
                        DeviceInfoShort * device = ( DeviceInfoShort * ) pCurDevice;
                        
                        if ( device->hasAppEnv ) {
                            DeviceInfo * d = ( DeviceInfo * ) pCurDevice;
                            app  = d->appName;
                            area = d->areaName;

                            // Make sure that strings are 0 terminated
                            app  [ MAX_LENGTH_APP_NAME - 1 ]	= 0;
                            area [ MAX_LENGTH_AREA_NAME - 1 ]	= 0;

                            appLen		= strlen ( app ) + 1;
                            areaLen		= strlen ( area ) + 1;
							sameAppArea = ( !strncmp ( area, env->areaName, sizeof ( env->areaName ) ) && !strncmp ( app, env->appName, sizeof ( env->appName ) ) );

							//CVerbArg ( "GetDevicesMediator: Has AppEnv, isSameAppArea [ %d ].", ( int ) sameAppArea );
                        }

                        device->deviceName [ MAX_LENGTH_DEVICE_NAME - 1 ]   = 0;

						//CVerbArg ( "GetDevicesMediator: Device [ %d ] ID [ 0x%X ] of area [ %s ] app [ %s ] name [ %s ].", i, device->deviceID, area, app, device->deviceName );
                        if ( sameAppArea && device->deviceID == env->deviceID )
                        {
							// We skip ourself in the list. Update counters accordingly.
                            // Save the external IP determined by the Mediator
                            IPe = device->ipe;
#ifdef ENABLE_EXT_BIND_IN_STUNT
                            MatchExtIPWithInterfaces ();
#endif
							skips++;
                        }
                        else {
                            if ( device->deviceID ) {
                                if ( device->hasAppEnv )
                                    memcpy ( medDevice, pCurDevice, DEVICE_INFO_CLIENT_SIZE );
                                else {
                                    memcpy ( medDevice, pCurDevice, sizeof ( DeviceInfoShort ) );
                                    
									memcpy ( medDevice->areaName, area, areaLen ); medDevice->areaName [ areaLen ] = 0;
                                    memcpy ( medDevice->appName, app, appLen ); medDevice->appName [ appLen ] = 0;
                                }
                                
                                medDevice->hasAppEnv = !sameAppArea;
                                medDevice->objID     = 0;
                                medDevice++;
                            }
                            else {
                                skips++;
#ifndef NDEBUG
                                CErrsArg ( 3, "GetDevicesFromMediator: Invalid deviceID 0 found at index [ %i ] !!!", i );
#endif
                            }
                        }

                        if ( device->hasAppEnv )
                            pCurDevice += DEVICE_INFO_CLIENT_SIZE;
                        else
                            pCurDevice += sizeof ( DeviceInfoShort );
                    }

                    deviceCountInBuffer -= skips;
                    devicesAtMediator   -= skips;
                    
                    free ( buffer );
                    buffer = medDevices;
                }
                else {
                    deviceCountInBuffer = -1;
                    free ( buffer ); buffer = 0;
                }
            }
        }
        
        if ( deviceCountInBuffer < 0 ) {
            CErr ( "GetDevicesFromMediator: Invalid buffer state (deviceCountInBuffer < 0)." );
            goto Failed;
        }

        if ( buffer )
        {
            deviceHead                          = ( DeviceHeader * ) buffer;
            deviceHead->deviceCountAvailable    = devicesAtMediator;
            deviceHead->deviceCount             = deviceCountInBuffer;

            retBuffer = buffer;
        }

		return deviceCountInBuffer;

    Failed:
        free_n ( buffer );
        retBuffer = 0;
        return -1;
	}


	int MediatorClient::GetDevicesAvailableCount ()
	{
		int countMediator = GetDevicesFromMediatorCount ();

        if ( countMediator >= 0 )
            return ( devicesAvailable + countMediator );
        
        return ( devicesAvailable + deviceMediatorCachedCount );
	}


	int MediatorClient::GetDevicesNearbyCount ()
	{
#ifdef FAKE42
		return 42;
#else
		return devicesAvailable;
#endif
	}


	int MediatorClient::GetDevicesNearby ( char * buffer, int bufferSize, int startIndex, int deviceID, const char * areaName, const char * appName )
	{
		int deviceCount = GetDevicesBroadcast ( buffer + DEVICES_HEADER_SIZE, bufferSize - DEVICES_HEADER_SIZE, startIndex, deviceID, areaName, appName );
		if ( deviceCount < 0 )
			return 0;

		if ( startIndex > ( devicesAvailable - 1 ) )
			startIndex = ( devicesAvailable - 1 );

		DeviceHeader * deviceHead = ( DeviceHeader * ) buffer;
		deviceHead->deviceCountAvailable = devicesAvailable;
		deviceHead->startIndex = startIndex;
		deviceHead->deviceCount = deviceCount;

		return deviceCount;
	}


	int MediatorClient::GetDevicesBroadcast ( char * buffer, int bufferSize, int startIndex, int deviceID, const char * areaName, const char * appName )
	{
		CVerbArg ( "GetDevicesBroadcast: buffsize [%d]", bufferSize );

		if ( !LockAcquireA ( devicesLock, "GetDevicesBroadcast" ) )
			return 0;

		if ( startIndex > ( devicesAvailable - 1 ) )
			startIndex = ( devicesAvailable - 1 );

		int					deviceCount	= 0;
		int					deviceStart = 0;
		char *				pCurDevice	= buffer;
		DeviceInstanceNode * device		= devices;

#ifdef FAKE42
		unsigned int max = 42;
		if ( deviceID )
			max = 1;
		while ( max > 0 ) {
			if ( bufferSize < DEVICE_PACKET_SIZE )
				break;

			memcpy ( pCurDevice, device, DEVICE_PACKET_SIZE );
			bufferSize -= DEVICE_PACKET_SIZE;

			CVerbArg ( "GetDevicesBroadcast: Device ID [%d] of type [%c] and name [%s].", device->ID, device->deviceType, device->deviceName );

			deviceCount++;
			max--;
			pCurDevice += DEVICE_PACKET_SIZE;

			//device = device->next;
		}
#else
		if ( !areaName || !appName ) {
			areaName = env->areaName;
			appName = env->appName;
		}

		while ( device ) {
			if ( deviceStart >= startIndex )
				break;

			deviceStart++;
			device = device->next;
		}

		while ( device ) {
			if ( bufferSize < ( signed ) DEVICE_PACKET_SIZE )
				break;

			if ( deviceID ) {
				if ( device->info.deviceID == deviceID
					&& !strncmp ( device->info.areaName, areaName, sizeof ( device->info.areaName ) ) && !strncmp ( device->info.appName, appName, sizeof ( device->info.appName ) )
					)
				{
					memcpy ( pCurDevice, device, DEVICE_PACKET_SIZE );
					deviceCount = 1;
					break;
				}

				device = device->next;
				continue;
			}

			memcpy ( pCurDevice, device, DEVICE_PACKET_SIZE );
			bufferSize -= DEVICE_PACKET_SIZE;

			deviceCount++;
			if ( deviceCount > ( devicesAvailable - startIndex ) )
				break;

			pCurDevice += DEVICE_PACKET_SIZE;

			device = device->next;
		}
#endif

		LockReleaseA ( devicesLock, "GetDevicesBroadcast" );

		return deviceCount;
	}


    bool MediatorClient::GetConnectionDetails ( int deviceID, const char * areaName, const char * appName, volatile DeviceStatus_t * deviceStatus, unsigned int &ip, int &portTcp, unsigned int &ipe, int &portUdp )
    {
        int retry = 1;

        sp ( DeviceInstanceNode ) device;

        while ( retry > 0 ) {
            int success = 0;

            device = GetDeviceSP ( deviceID, areaName, appName, &success );
            if ( device ) {
                ip      = device->info.ip;
                ipe     = device->info.ipe;
                portTcp = device->info.tcpPort;
                portUdp = device->info.udpPort;
                return true;
            }

            if ( success < 0 )
                break;

            if ( deviceStatus && *deviceStatus == DeviceStatus::Deleteable )
                break;

            Sleep ( 800 );
            retry--;
        }

        return false;
    }


	bool MediatorClient::GetPortTCP ( int deviceID, const char * areaName, const char * appName, int &value )
	{
		CVerbsID ( 6, "GetPortTCP" );

		sp ( DeviceInstanceNode ) device = GetDeviceNearbySP ( deviceID, areaName, appName );

		if ( device ) {
			value = device->info.tcpPort;
			return true;
        }

        device = GetDeviceSP ( deviceID, areaName, appName );

        if ( device ) {
            value = device->info.tcpPort;
            return true;
        }
		return true;
	}


	bool MediatorClient::GetPortUDP ( int deviceID, const char * areaName, const char * appName, int &value )
	{
		sp ( DeviceInstanceNode ) device = GetDeviceNearbySP ( deviceID, areaName, appName );

		if ( device ) {
			value = device->info.udpPort;
			return true;
        }

        device = GetDeviceSP ( deviceID, areaName, appName );

        if ( device ) {
            value = device->info.udpPort;
            return true;
        }
        return false;
	}


	bool MediatorClient::GetIP ( int deviceID, const char * areaName, const char * appName, unsigned int &value )
	{
		sp ( DeviceInstanceNode ) device = GetDeviceNearbySP ( deviceID, areaName, appName );

		if ( device ) {
			value = device->info.ip;

			CVerbArg ( "GetIP: IP [%s]", inet_ntoa ( *( ( struct in_addr * ) &value ) ) );
			return true;
		}

        device = GetDeviceSP ( deviceID, areaName, appName );

        if ( device ) {
            value = device->info.ip;

            CVerbArg ( "GetIP: IP [%s]", inet_ntoa ( *( ( struct in_addr * ) &value ) ) );
            return true;
        }
		return false;
	}


	bool MediatorClient::GetIPe ( int deviceID, const char * areaName, const char * appName, unsigned int &value )
	{

		sp ( DeviceInstanceNode ) device = GetDeviceNearbySP ( deviceID, areaName, appName );

		if ( device ) {
			value = device->info.ipe;

			CVerbArg ( "GetIPe: IPe [%s]", inet_ntoa ( *( ( struct in_addr * ) &value ) ) );
			return true;
        }

        device = GetDeviceSP ( deviceID, areaName, appName );

        if ( device ) {
            value = device->info.ipe;

            CVerbArg ( "GetIPe: IPe [%s]", inet_ntoa ( *( ( struct in_addr * ) &value ) ) );
            return true;
        }
		return false;
	}


	bool MediatorClient::GetIntParam ( int deviceID, const char * areaName, const char * appName, const char * key, int &value )
	{
		const unsigned int minBuffSize = 32 + ( 2 * ( MAX_NAMEPROPERTY + 10 ) );

		char buffer [ minBuffSize ];

		if ( !GetParam ( deviceID, areaName, appName, key, buffer, minBuffSize ) ) {
			CErrArg ( "GetIntParam: Failed to retrieve int Value [%s]  Area [%s]  App [%s]  Device [0x%X]!", key, areaName ? areaName : env->areaName, appName ? appName : env->appName, deviceID );
			return false;
		}

		unsigned int * pUI = reinterpret_cast<unsigned int *>( buffer );

		int len = *pUI;
		if ( len > 18 ) { // int-max = 10, size = 4, zero-term = 1
			buffer [ 18 ] = 0;
			CErrArg ( "GetIntParam: Response for int value [%s] is invalid. Length [%i] is > [18]. [%s]", key, len, buffer + 4 );
			return false;
		}
		buffer [ len + 4 ] = 0;

		if ( sscanf_s ( buffer + 4, "%i", &value ) == 1 )
			return true;

		return false;
	}


	bool MediatorClient::GetStringParam ( int deviceID, const char * areaName, const char * appName, const char * key, char * buffer, unsigned int bufferSize )
	{
		const unsigned int minBuffSize = 32 + ( 2 * ( MAX_NAMEPROPERTY + 10 ) );

		if ( bufferSize < minBuffSize )
			return false;

		if ( !GetParam ( deviceID, areaName, appName, key, buffer, bufferSize ) ) {
			CErrArg ( "GetStringParam: Failed to retrieve string Value [%s]  Area [%s]  App [%s]  Device [0x%X]!", key, areaName ? areaName : env->areaName, appName ? appName : env->appName, deviceID );
			return false;
		}

		unsigned int len = *( ( unsigned int * ) buffer );
		if ( len > bufferSize - 5 ) { // int-max = 10, size = 4, zero-term = 1
			buffer [ 18 ] = 0;
			CErrArg ( "GetStringParam: Response for string value [%s] is invalid. Length [%i] is > [%i]. [%s]", key, len, bufferSize - 5, buffer + 4 );
			return false;
		}
		buffer [ len + 4 ] = 0;

		if ( buffer [ 4 ] == '-' && buffer [ 5 ] == '-' && buffer [ 6 ] == '-' )
			return false;

		return true;
	}


	bool MediatorClient::SetParam ( int deviceID, const char * areaName, const char * appName, const char * key, const char * value )
	{
		if ( !isRunning || env->environsState < environs::Status::Starting )
			return false;

		if ( !areaName )
			areaName = " ";

		if ( !appName )
			appName = " ";

		CVerbArg ( "SetParam: area=%s, key=%s, value=%s", areaName, key, value );

		// Build the message
		char buffer [ MESSAGE_BUFFER_SIZE ];

		MediatorGetPacket * set = ( MediatorGetPacket * ) buffer;

		set->version = MEDIATOR_PROTOCOL_VERSION;
		set->cmd = MEDIATOR_CMD_SET;
		set->opt0 = MEDIATOR_OPT_NULL;
		set->opt1 = MEDIATOR_OPT_NULL;

		int length = snprintf ( buffer + sizeof ( MediatorGetPacket ), MESSAGE_BUFFER_SIZE - sizeof ( MediatorGetPacket ), "%s;%s;%i_%s;%s",
			areaName, appName, deviceID, key, value );
		if ( length <= 6 ) {
			CErrArg ( "SetParam: Failed due to short length of message of size [ %i ]", length );
			return false;
		}

		// Set length of message
		set->size = ( unsigned int ) ( length + sizeof ( MediatorGetPacket ) );
		//*((unsigned int *) buffer) = (unsigned int) length;
        
        return ( PushSend ( set, set->size ) == ( int ) set->size );
	}


	bool MediatorClient::GetParam ( const char * areaName, const char * appName, const char * key, char * buffer, unsigned int bufferSize )
	{
		if ( !isRunning )
			return false;

		CVerbArg ( "GetParam: area [%s], app [%s], key [%s]", areaName ? areaName : env->areaName, appName ? appName : env->appName, key );

		MediatorGetPacketV6 * get = ( MediatorGetPacketV6 * ) buffer;

		get->version    = MEDIATOR_PROTOCOL_VERSION;
		get->cmd        = MEDIATOR_CMD_GET;
		get->opt0       = MEDIATOR_OPT_NULL;
        get->opt1       = MEDIATOR_OPT_NULL;

		int length = snprintf ( buffer + sizeof ( MediatorGetPacket ), bufferSize - sizeof ( MediatorGetPacket ), "%s;%s;%s;",
			areaName ? areaName : " ", appName ? appName : " ", key );
		if ( length <= 6 ) {
			CErrArg ( "GetParam: Failed due to short length of message of size [%i]", length );
            return false;
		}

		// Set length of message
		get->size = ( unsigned int ) ( length + sizeof ( MediatorGetPacket ) );

		if ( isRunning && env->environsState >= environs::Status::Starting && SendMessageToMediator ( buffer, true, bufferSize ) ) {
			CVerbArg ( "GetParam: Received value [%s]", buffer + 4 );
			//ret = true;
            return true;
		}
		else {
			CVerbArg ( "GetParam: Failed to receive value for area [%s], app [%s], key [%s]", areaName ? areaName : env->areaName, appName ? appName : env->appName, key );
		}

        return false;
	}


	bool MediatorClient::GetParam ( int deviceID, const char * areaName, const char * appName, const char * key, char * buffer, unsigned int bufferSize )
	{
		char keyValue [ 256 ];

		if ( snprintf ( keyValue, 256, "%i_%s", deviceID, key ) < 0 )
			return false;

		return GetParam ( areaName, appName, keyValue, buffer, bufferSize );
	}

    
    bool MediatorClient::RequestSTUNT ( volatile DeviceStatus_t * deviceStatus, int deviceID, const char * areaName, const char * appName, unsigned short &porti, unsigned int &ip, unsigned short &porte, unsigned int &ipe, struct sockaddr_in * addr, char channel, unsigned int token )
    {
        CVerbsArgID ( 4, "[ %s ].RequestSTUNT", getChannel () );
        
        MediatorInstance * med = GetAvailableMediator ();
        if ( !med )
            return false;
        
        bool		ret		= false;
        int			retry	= 20;
        
        STUNTReqPacketV8	stuntPacket;
        STUNTReqPacketV8	toSend;
        
        porti	= 0;
        porte	= 0;
        ip		= 0;
        ipe		= 0;
        
        Zero ( stuntPacket );
        
        STUNTReqHeaderV8	* header = &stuntPacket.header;
        
        header->version     = MEDIATOR_PROTOCOL_VERSION;
        
        header->ident [ 0 ] = MEDIATOR_CMD_STUNT;
        header->ident [ 1 ] = MEDIATOR_OPT_NULL;
        
        header->channel		= channel;
        header->deviceID	= deviceID;
        
        if ( areaName && *areaName && appName && *appName ) {
            if ( !BuildAppAreaField ( header->sizes, appName, areaName, false ) )
                return false;
        }
        else {
            header->sizes [ 0 ] = 1;
            header->sizes [ 1 ] = 1;
        }
        
        header->size = sizeof ( STUNTReqHeaderV8 ) + header->sizes [ 0 ] + header->sizes [ 1 ];
        
        if ( !RegisterStuntSocket ( false, med, deviceID, appName, areaName, addr, channel, token, false ) )
            return false;
        
        header->token = token;
        
        while ( isRunning && retry > 0 && ( !deviceStatus || *deviceStatus != DeviceStatus::Deleteable ) && env->environsState >= environs::Status::Starting )
        {
            memcpy ( &toSend, &stuntPacket, header->size );
            
            STUNTRespPacketV6 * resp = ( STUNTRespPacketV6 * ) &toSend;
            
            if ( !SendMessageToMediator ( &toSend, true, sizeof ( STUNTReqPacketV8 ), 8000 ) || resp->size < MEDIATOR_STUNT_ACK_SIZE ) {
                CErrArgID ( "[ %s ].RequestSTUNT: Failed to send/receive STUNT details!", getChannel () );
                break;
            }
            
            if ( resp->respCode == 'r' || resp->respCode == MEDIATOR_PROTOCOL_VERSION ) { // '5' means that we have not received something into the buffer
                if ( ( retry % 4 ) == 0 ) {
                    CVerbArgID ( "[ %s ].RequestSTUNT: Received retry message / timeout. Try again in 600 ms...", getChannel () );
                }
                retry--;
                Sleep ( 600 );
                continue;
            }
            
            if ( resp->respCode == 'e' ) {
                CErrArgID ( "[ %s ].RequestSTUNT: Mediator failed to determine STUNT details!", getChannel () );
                break;
            }
            
            if ( resp->respCode != 'p' ) {
                CErrArgID ( "[ %s ].RequestSTUNT: Mediator failed to determine STUNT details!", getChannel () );
                break;
            }
            
            porti = resp->porti;
            porte = resp->porte;
            
            CVerbsArgID ( 4, "[ %s ].RequestSTUNT: Received port ext [ %d ]", getChannel (), porte );
            CVerbsArgID ( 4, "[ %s ].RequestSTUNT: Received port int [ %d ]", getChannel (), porti );
            
            if ( resp->channel != MEDIATOR_STUNT_CHANNEL_VERSATILE && resp->size >= MEDIATOR_STUNT_ACK_EXT_SIZE ) {
                ip = resp->ip;
                CVerbsArgID ( 4, "[ %s ].RequestSTUNT: Received IP [ %s ]!", getChannel (), inet_ntoa ( *( ( struct in_addr * ) &ip ) ) );
                
                ipe = resp->ipe;
                CVerbsArgID ( 4, "[ %s ].RequestSTUNT: Received IPe [ %s ]!", getChannel (), inet_ntoa ( *( ( struct in_addr * ) &ipe ) ) );
                
                if ( !ip || !ipe ) {
                    CErrArgID ( "[ %s ].RequestSTUNT: Received invalid ip(s) 0!", getChannel () );
                    break;
                }
            }
            
            ret = true;
            break;
        }
        
        return ret;
    }
    

	bool MediatorClient::RequestSTUN ( int deviceID, const char * areaName, const char * appName, int sock )
	{
		if ( !isRunning || env->environsState < environs::Status::Starting )
			return false;

		CVerbsID ( 4, "RequestSTUN" );

		bool				ret			= true;
		struct sockaddr *	addr;
		char			*	cipher		= 0;
		char			*	cipherPack	= 0;

		STUNReqPacket		packet;
		Zero ( packet );

        STUNReqHeader   *   stun = &packet.header;

		char			*	toSend		= ( char * ) stun;
		unsigned int		toSendSize	= sizeof ( packet );

		stun->ident [ 0 ]     = MEDIATOR_STUN_REQUEST;
		stun->ident [ 1 ]     = MEDIATOR_OPT_NULL;
		stun->ident [ 2 ]     = MEDIATOR_OPT_NULL;
		stun->ident [ 3 ]     = MEDIATOR_OPT_NULL;

		// SourceID -> DestID
		stun->sourceID	= env->deviceID;
		stun->destID	= deviceID;

		if ( areaName && *areaName && appName && *appName && env->mediatorFilterLevel > MediatorFilter::None && !IsSameAppEnv ( env, appName, areaName ) ) {
			if ( !BuildAppAreaField ( stun->sizes, appName, areaName, false ) )
				return false;
		}
		else {
			stun->sizes [ 0 ] = 1;
			stun->sizes [ 1 ] = 1;
		}

        toSendSize = sizeof ( STUNReqHeader ) + stun->sizes [0] + stun->sizes [1];        

		// Send to all of the mediators
		MediatorInstance * med = &mediator;

		if ( !med->ip || !med->port ) {
			CWarnID ( "RequestSTUN: No mediators available." );
			ret = false;
		}
		else
		{
            while ( med ) {
				if ( med->listening && med->ip && med->port )
				{
					addr = ( struct sockaddr * ) &med->connection.instance.addr;

					if ( med->connection.instance.encrypt && !cipher ) {
						if ( !AESEncrypt ( &med->connection.instance.aes, toSend, &toSendSize, &cipher ) ) {
							CErr ( "RequestSTUN: AES encrypt failed!" );
							ret = false; break;
						}
#ifdef NDEBUG
						cipherPack = ( char * ) malloc ( toSendSize + sizeof ( UdpEncHelloPacket ) );
#else
                        cipherPack = ( char * ) calloc ( 1, toSendSize + sizeof ( UdpEncHelloPacket ) );
#endif
						if ( !cipherPack )
							break;

						UdpEncHelloPacket * hp = ( UdpEncHelloPacket * ) cipherPack;

						memcpy ( &hp->aes, cipher, toSendSize );

						toSendSize		+= sizeof ( UdpEncHelloPacket );
						hp->size		= toSendSize | 0x80000000;

                        hp->sessionID	= med->connection.instance.sessionID;
						toSend			= cipherPack;
					}

					CVerbsID ( 4, "RequestSTUN: sending 3 x udp requests to mediator" );
					int repeats = 3;
                    while ( repeats > 0 ) {
                        CVerbsVerbArg ( 6, "RequestSTUN: Send packet of size [%d]  ...", toSendSize );

						size_t bytes = sendto ( sock, toSend, toSendSize, 0, addr, sizeof ( struct sockaddr ) );
						if ( bytes != toSendSize ) {
							CWarnsArgID ( 3, "RequestSTUN: Send udp STUN to mediator failed %i != %u", ( int ) bytes, toSendSize ); LogSocketErrorNOK ();

							if ( errno == EISCONN )
								break;
						}
						else
							break;
						repeats--;
					}
				}

				med = med->next;
			}
		}

        free_n ( cipher );
        free_n ( cipherPack );
		return ret;
    }


}





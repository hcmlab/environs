/**
 * Stunt / Stun establisher
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

#include "Environs.Native.h" // Access to native function exports

#include "Stunt.Request.h"
#include "Device/Device.Controller.h"
#include <errno.h>
#include <string>
#include "Environs.Obj.h"
#include "Environs.Lib.h"
#include "Environs.Utils.h"
#include "Tracer.h"
using namespace environs;


//#define ENABLE_DONTROUTE_FOR_STUNT

/* Namespace: environs -> */
namespace environs
{
#define	CLASS_NAME 	"STUNT.Request. . . . . ."
	
    extern void CreateCopyString ( const char * src, char ** dest );
    
    
#define MAX_STUNTS_IN_PARALLEL  8

	LONGSYNC    stuntCount	= 0;
	LONGSYNC    stunCount	= 0;
    
    StunTRequest::StunTRequest ( int deviceID )
    {
        this->deviceID	= deviceID;

		__sync_add_and_fetch ( &stuntCount, 1 );
        
        CVerbID ( "Construct" );
        
		allocated		= false;
        env				= 0;
        isInitiator     = true;
        channel			= ';';
        IP				= false;
        IPe				= false;
        Porte			= false;
        Zero ( stuntAddr );
		
		deviceStatus	= 0;
        disposeDevice   = true;
        disposing		= false;
        detach			= true;
        doHandshake		= false;
        
        repeats         = 0;
        socketConnect	= INVALID_FD;
        socketAccept	= INVALID_FD;
        
		internalStunt.isInternal	= true;
        externalStunt.isInternal	= false;
        
        internalStunt.state = 0;
        externalStunt.state = 0;
        acceptStunt.state	= 0;

        internalStunt.sock	= INVALID_FD;
        externalStunt.sock	= INVALID_FD;
        acceptStunt.sock	= INVALID_FD;
    }
    
    
    StunTRequest::~StunTRequest ()
    {
        CVerbsArgID ( 4, "[ %s ].Destruct", getChannel ( ) );
        
        disposing = true;
        
        CloseThreads ( );
        
        int sock = acceptStunt.sock;
        
        if ( IsValidFD ( sock ) ) {
            acceptStunt.sock = INVALID_FD;
            CSocketTraceRemove ( sock, "Destruct: Closing acceptStunt.", 0 );
            closesocket ( sock );
        }
        
        sock = externalStunt.sock;
        if ( IsValidFD ( sock ) ) {
            externalStunt.sock = INVALID_FD;
            CSocketTraceRemove ( sock, "Destruct: Closing externalStunt.", 0 );
            closesocket ( sock );
        }
        
        sock = internalStunt.sock;
        if ( IsValidFD ( sock ) ) {
            internalStunt.sock = INVALID_FD;
            CSocketTraceRemove ( sock, "Destruct: Closing internalStunt.", 0 );
            closesocket ( sock );
        }
        
        sock = socketAccept;
        if ( IsValidFD ( sock ) ) {
            socketAccept = INVALID_FD;
            CSocketTraceRemove ( sock, "Destruct: Closing socketAccept.", 0 );
            closesocket ( sock );
        }
        
        sock = socketConnect;
        if ( IsValidFD ( sock ) ) {
            socketConnect = INVALID_FD;
            CSocketTraceRemove ( sock, "Destruct: Closing socketConnect.", 0 );
            closesocket ( sock );
        }
        
        __sync_sub_and_fetch ( &stuntCount, 1 );
    }
    
    
    void StunTRequest::CloseThreads ( bool waitWorkingThread )
    {
        CVerbsArgID ( 4, "[ %s ].CloseThreads", getChannel () );
        
        disposing	= true;
        doHandshake = false;
        
        int sock = acceptStunt.sock;
        if ( IsValidFD ( sock ) ) {
            CVerbsArgID ( 5, "[ %s ].CloseThreads: Shutdown STUNT accept socket...", getChannel () );
            SetNonBlockSocket ( sock, true, "CloseThreads" );
            
			CSocketTraceVerbUpdate ( sock, "StunTRequest.CloseThreads shutdown acceptStunt.sock" );
            shutdown ( sock, 2 );
        }
        
        sock = externalStunt.sock;
        if ( IsValidFD ( sock ) ) {
            CVerbsArgID ( 5, "[ %s ].CloseThreads: Shutdown STUNT external connector socket...", getChannel () );
            SetNonBlockSocket ( sock, true, "CloseThreads" );
            
			CSocketTraceVerbUpdate ( sock, "StunTRequest.CloseThreads shutdown externalStunt.sock" );
            shutdown ( sock, 2 );
        }
        
        sock = internalStunt.sock;
        if ( IsValidFD ( sock ) ) {
            CVerbsArgID ( 5, "[ %s ].CloseThreads: Shutdown STUNT internal connector socket...", getChannel () );
            SetNonBlockSocket ( sock, true, "CloseThreads" );
            
			CSocketTraceVerbUpdate ( sock, "StunTRequest.CloseThreads shutdown internalStunt.sock" );
            shutdown ( sock, 2 );
        }
        
        sock = socketAccept;
        if ( IsValidFD ( sock ) ) {
            CVerbsArgID ( 5, "[ %s ].CloseThreads: Shutdown STUNT accepted socket...", getChannel () );
            SetNonBlockSocket ( sock, true, "CloseThreads" );
            
			CSocketTraceVerbUpdate ( sock, "StunTRequest.CloseThreads shutdown socketAccept" );
            shutdown ( sock, 2 );
        }
        
        sock = socketConnect;
        if ( IsValidFD ( sock ) ) {
            CVerbsArgID ( 5, "[ %s ].CloseThreads: Shutdown STUNT connected socket...", getChannel () );
            SetNonBlockSocket ( sock, true, "CloseThreads" );
            
			CSocketTraceVerbUpdate ( sock, "StunTRequest.CloseThreads shutdown socketConnect" );
            shutdown ( sock, 2 );
        }
        
        if ( waitWorkingThread && thread.isRunning () )
        {
            CVerbsArgID ( 5, "[ %s ].CloseThreads: Waiting for STUNT Responder  (Mediator.. run.. HandleRequest.. StunTRequest::HandleRequest) to be terminated...", getChannel () );
            //CLogArgID ( "[ %s ].CloseThreads: Waiting for STUNT Responder ...", getChannel () );
            
            thread.Join ( "STUNT Responder" );
        }
    }
    
    
    sp ( StunTRequest ) StunTRequest::CreateRequest ( Instance * env, volatile DeviceStatus_t * deviceStatus, int deviceID, const char * areaName, const char * appName, char channel, unsigned int token )
    {
        CVerbArgID ( "[ %s ].CreateRequest", getChannel ( ) );
        
        sp ( StunTRequest ) request = std::make_shared < StunTRequest > ( deviceID );
        if ( !request ) {
            CErrArgID ( "[ %s ].CreateRequest: Failed to allocate a new StunTRequest instance.", getChannel ( ) );
            return 0;
        }
        
        if ( !request->Init () ) {
            CErrArgID ( "[ %s ].CreateRequest: Failed to init a new StunTRequest instance.", getChannel ( ) );
            goto Finish;
        }
        else
        {
            request->env		= env;
            request->deviceID	= deviceID;
            request->channel	= channel;
            request->deviceStatus = deviceStatus;
            
            sp ( MediatorClient ) mediator = env->mediator MED_WP;
            
            if ( !mediator || !mediator->IsServiceAvailable () ||
                !mediator->RequestSTUNT ( deviceStatus, deviceID, areaName, appName, request->Porti, request->IP, request->Porte, request->IPe, &request->stuntAddr, channel, token ) )
            {
                if ( env->environsState >= environs::Status::Starting ) {
                    CErrArgID ( "[ %s ].CreateRequest: Failed to determine STUNT details (Port/IP/IPe)", getChannel () );
                }
                goto Finish;
            }
        }
        
        request->myself = request;
        return request;
        
    Finish:
        return 0;
    }
    

	bool StunTRequest::Init ()
	{
		if ( !allocated )
        {
            if ( !thread.Init () )
                return false;
            
            allocated = true;
        }
        
        return true;
	}
    
    
    void StunTRequest::HandleIncomingRequest ( Instance * env, void * msg )
    {
        CVerb ( "HandleIncomingRequest" );
        
        STUNTRespReqPacketV8 * req		= ( STUNTRespReqPacketV8 * ) msg;
        STUNTRespReqHeaderV8 * header	= ( STUNTRespReqHeaderV8 * ) msg;
        char			channel		= header->channel;
        
        unsigned int	deviceID	= header->deviceID;
        unsigned int	IP			= header->ip;
        unsigned int	IPe			= header->ipe;
        unsigned short	Porti		= header->porti;
        unsigned short	Porte		= header->porte;
        
        if ( env->environsState < environs::Status::Starting ) {
            CVerbArgID ( "[ %s ].HandleIncomingRequest: Environs is not started.", getChannel ( ) );
            return;
        }
        
        if ( !deviceID || !IP || !IPe || !Porte || !Porti ) {
            CErrArgID ( "[ %s ].HandleIncomingRequest: Invalid request arguments. IPe [ %s ] IP [ 0x%X ] Porte [ %d ] Porti [ %d ]", getChannel ( ), inet_ntoa ( *((struct in_addr *) &IPe) ), IP, Porte, Porti );
            return;
        }
        
        sp ( StunTRequest )	request = std::make_shared < StunTRequest > ( deviceID );
        if ( !request || !request->Init () ) {
            CErrArgID ( "[ %s ].HandleIncomingRequest: Failed to create object for StunTRequest", getChannel ( ) );
            return;
        }
        
        request->mediator   = env->mediator MED_WP;
        if ( !request->mediator ) {
            CErrArgID ( "[ %s ].HandleIncomingRequest: Invalid mediator layer.", getChannel ( ) );
            return;
        }
        
        MediatorClient * mediator = request->mediator.get ();
        
        char * appName = 0;
        char * areaName = 0;
        
        if ( header->sizes [ 0 ] > 1 && header->sizes [ 1 ] > 1 && header->sizes [ 0 ] < MAX_NAMEPROPERTY && header->sizes [ 1 ] < MAX_NAMEPROPERTY )
        {
            appName = req->appArea;
            areaName = req->appArea + header->sizes [ 0 ];
        }
        
        if ( !mediator->IsConnectAllowed ( deviceID, appName, areaName ) ) {
            CWarnArgID ( "[ %s ].HandleIncomingRequest: Connect with [ 0x%X - %s : %s ] not allowed.", getChannel (), deviceID, appName ? appName : "", areaName ? areaName : "" );
            return;
        }        
        
        request->env		= env;
        request->channel	= channel;
        request->IP			= IP;
        request->IPe		= IPe;
        request->Porti		= Porti;
        request->Porte		= Porte;
        request->isInitiator = false;
        
        *request->appName	= 0;
        *request->areaName	= 0;
        
        if ( appName && areaName ) {
            strlcpy ( request->appName, appName, sizeof ( request->appName ) );
            strlcpy ( request->areaName, areaName, sizeof ( request->areaName ) );
        }
        
        bool                doStunt     = false;
        MediatorInstance *  med         = &mediator->mediator;
		ThreadInstance *	inst = &med->connection.instance;

		char				keyID [ 36 + sizeof ( AppAreaBuffer ) ];
		char			*	key			= keyID + 4;

		if ( !BuildAppAreaID ( ( char * ) keyID, deviceID, appName, areaName, channel, header->token ) ) {
			CErrArg ( "[ %s ].HandleIncomingRequest: Failed to build app area id!", getChannel () );
			return;
		}

        bool isOnoing = false;
        
		if ( !LockAcquireA ( inst->stuntSocketLock, "HandleIncomingRequest" ) )
			return;

		const std::map < std::string, sp ( StuntRegisterContext ) >::iterator &foundIt = inst->stuntSocketsLog.find ( key );
		
		if ( foundIt != inst->stuntSocketsLog.end () )
		{
			StuntRegisterContext * ctx = foundIt->second.get ();
			if ( ctx )
			{
				CVerbsArg ( CONCURRENT_STUNT_LOGLEVEL, "[ %s ].HandleIncomingRequest: Found context for [ %s ]!", getChannel (), key );

				struct sockaddr_in * addr = ( channel == MEDIATOR_STUNT_CHANNEL_MAIN ? &ctx->addrI : &ctx->addrC );
				if ( addr->sin_port ) {
					CVerbsArg ( CONCURRENT_STUNT_LOGLEVEL, "[ %s ].HandleIncomingRequest: Doing stunt for [ %s ]!", getChannel (), key );

                    request->stuntAddr = *addr;
                    addr->sin_port = 0;
					doStunt = true;
				}
                else {
                    int s = ( channel == MEDIATOR_STUNT_CHANNEL_MAIN ? ctx->sockI : ctx->sockC );
                    if ( IsValidFD ( s ) ) {
                        isOnoing = true;
                    }
                }
			}
		}

		LockReleaseA ( inst->stuntSocketLock, "HandleIncomingRequest" );
        
        if ( !doStunt ) {
            if ( !isOnoing ) {
                CErrArg ( "[ %s ].HandleIncomingRequest: Failed to query STUNT registration for [ %s ]", getChannel ( ), key );
            }
            else {
                CVerbsArg ( CONCURRENT_STUNT_LOGLEVEL, "[ %s ].HandleIncomingRequest: STUNT is ongoing for [ %s ]", getChannel ( ), key );
            }
            return;
        }
        
        request->myself     = request;
        
        if ( !request->thread.Run ( pthread_make_routine ( &StunTRequest::ProcessIncomingRequest ), request.get (), "StunTRequest.HandleIncomingRequest" ) ) {
            request->myself	= 0;
            CErrArgID ( "[ %s ].HandleIncomingRequest: Failed to create thread for handling STUNT TCP request.", getChannel () );
        }      
    }
    
    
    void * StunTRequest::ProcessIncomingRequest ( void * arg )
    {
        CVerb ( "ProcessIncomingRequest started..." );
        
        // STUNT request
        pthread_setname_current_envthread ( "StunTRequest.ProcessIncomingRequest" );

		StunTRequest * request = (StunTRequest *) arg;
        if ( !request ) {
            CErr ( "ProcessIncomingRequest: Invalid argument." );
            return 0;
        }
        
        sp ( StunTRequest ) requestSP ( request->myself );
        if ( !requestSP ) {
            CErr ( "ProcessIncomingRequest: Invalid argument." );
            return 0;
        }

        int             deviceID    = request->deviceID;
        Instance	*	env			= request->env;
		sp ( Instance ) envSP;

        
        char			channel     = request->channel;
        
        sp ( DeviceController ) deviceSP;
        DeviceBase  *           device;
        
        const sp ( MediatorClient ) &mediator = request->mediator;
        if ( !mediator )
            goto FinishNoUnlock;
        
        if ( env->environsState < environs::Status::Starting ) {
            CVerbArgID ( "[ %s ].ProcessIncomingRequest: Environs is not started.", getChannel ( ) );
            goto FinishNoUnlock;
        }
        
        if ( !WaitForDeviceDeletion ( env, deviceID, request->areaName, request->appName, true ) ) {
			CErrArgID ( "[ %s ].ProcessIncomingRequest: Wait for device deletion failed.", getChannel () );
			goto FinishNoUnlock;
        }

        device = GetDevice ( env, deviceID, request->areaName, request->appName, false );
        if ( device ) {
#ifdef ENABLE_DEVICEBASE_WEAK_REFERENCE
            deviceSP = device->myself.lock ();
#else
			LockAcquireVA ( device->spLock, "ProcessIncomingRequest" );

			deviceSP = device->myself;

			LockReleaseVA ( device->spLock, "ProcessIncomingRequest" );
#endif
		}
		else
		{
            //
            // No device available, create a new one and let the device handle the request
            //
			CVerbArgID ( "[ %s ].ProcessIncomingRequest: Creating new DeviceController.", getChannel () );
            
			deviceSP = std::make_shared < DeviceController > ( deviceID );
            if ( !deviceSP ) {
				CErrArgID ( "[ %s ].ProcessIncomingRequest: Allocation of a device controller object failed.", getChannel () );
                goto Finish;
            }
            
            device                  = deviceSP.get ();
            
#ifdef ENABLE_INSTANCE_WEAK_REFERENCE
            envSP = native.instancesSP [ env->hEnvirons ].lock ();
#else
			LockAcquireVA ( native.instancesSPLock, "ProcessIncomingRequest" );

			envSP = native.instancesSP [ env->hEnvirons ];

			LockReleaseVA ( native.instancesSPLock, "ProcessIncomingRequest" );
#endif
            if ( !envSP || !device->Init ( envSP, request->areaName, request->appName ) ) {
                CErrsArgID ( 2, "[ %s ].ProcessIncomingRequest: Failed to create new DeviceController", getChannel ( ) );
                
                device = 0;
                goto Finish;
            }
            
            device->activityStatus  |= DEVICE_ACTIVITY_RESPONDER;
            device->deviceID		= deviceID;
            device->behindNAT		= true;
            device->deviceStatus	= DeviceStatus::ConnectInProgress;
            
            int nativeID = AddDevice ( deviceSP, false );
            if ( nativeID <= 0 )
            {
                //deviceNode->deviceSP.reset ();
                //device->deviceNode.reset ();
				goto Finish;
            }
            
            device->deviceNode->deviceSP    = deviceSP;
            
            // Get a lock on the device for this thread
            IncLockDevice ( device );
        }
        
        if ( !LockReleaseA ( devicesAccessMutex, "ProcessIncomingRequest" ) )
        {
            if ( device ) {
                device->deviceStatus = DeviceStatus::Deleteable;
                UnlockDevice ( device );

				TriggerCleanUpDevices ();
            }
            goto FinishNoUnlock;
        }
		        
        if ( device && deviceSP ) {
            /// Make sure that the device is actually running a stunt connection
            if ( (device->activityStatus & DEVICE_ACTIVITY_REQUESTOR) == 0 ) 
            {
                request->deviceStatus   = &device->deviceStatus;
                
				//
				// Let the device handle the request.
				//
                bool handleRequest = false;
                
                if ( channel == MEDIATOR_STUNT_CHANNEL_BULK ) {
                    if ( IsInvalidFD ( device->comDatSocket ) )
                        handleRequest = true;
                }
                else {
                    if ( IsInvalidFD ( device->interactSocket ) )
                        handleRequest = true;
                }
                
                if ( handleRequest )
                {
                    request->deviceSP       = deviceSP;
                    
                    request->HandleRequest ();
                }
            }
			deviceSP = 0;

			/// else -> A regular connection seems to be ongoing.. so cancel our requestor            
            
            UnlockDevice ( device );
        }
		goto FinishNoUnlock;
        
	Finish:
        LockReleaseVA ( devicesAccessMutex, "ProcessIncomingRequest" );

	FinishNoUnlock:
		/// We do not join this thread, because WE ARE THE executing thread ...
		//request->thread.Detach ( "STUNT" );

		requestSP->myself.reset ();
        return 0;
    }
    
    
    int StunTRequest::MakeSocket ( bool isInternal )
    {
        int sock = INVALID_FD;
        int value = 1, ret;

		sock = ( int ) socket ( PF_INET, SOCK_STREAM, 0 ); // ( AF_INET, SOCK_STREAM, IPPROTO_TCP );
        if ( IsInvalidFD ( sock ) ) {
            CErrArgID ( "[ %s ].MakeSocket: Failed to create socket.", getChannel ( ) ); LogSocketError ();
            goto Finish;
        }
		CSocketTraceAdd ( sock, "StunTRequest MakeSocket" );
        DisableSIGPIPE ( sock );
        
#ifdef ENABLE_TCL_NODELAY_FOR_NAT
        value = 1;
		ret = setsockopt ( sock, IPPROTO_IP, TCP_NODELAY, ( const char * ) &value, sizeof ( value ) );
        if ( ret < 0 ) {
            CErrArgID ( "[ %s ].MakeSocket: Failed to set TCP_NODELAY on socket", getChannel ( ) ); LogSocketError ();
            goto Finish;
        }
#endif
        value = 1;
		ret = setsockopt ( sock, SOL_SOCKET, SO_REUSEADDR, ( const char * ) &value, sizeof ( value ) );
        if ( ret < 0 ) {
            CErrArgID ( "[ %s ].MakeSocket: Failed to set reuseAddr on socket.", getChannel ( ) ); LogSocketError ();
            goto Finish;
        }
        
#if defined(SO_REUSEPORT)
        value = 1;
		ret = setsockopt ( sock, SOL_SOCKET, SO_REUSEPORT, ( const char * ) &value, sizeof ( value ) );
        if ( ret < 0 ) {
            CErrArgID ( "[ %s ].MakeSocket: Failed to set reusePort on socket.", getChannel ( ) ); LogSocketError ();
            //goto Finish;
        }
#endif
        
#ifdef ENABLE_DONTROUTE_FOR_STUNT
        if ( !isInternal ) {
            value = 1;
            ret = setsockopt ( sock, SOL_SOCKET, SO_DONTROUTE, ( const char * ) &value, sizeof ( value ) );
            if ( ret < 0 ) {
                CErrArgID ( "[ %s ].MakeSocket: Failed to set dont route on socket.", getChannel ( ) ); LogSocketError ();
                goto Finish;
            }
        }
#endif
        CVerbArgID ( "[ %s ].MakeSocket: Local mediator endpoint [ %s : %d ], Socket [ %i ]", getChannel ( ), inet_ntoa ( stuntAddr.sin_addr ), ntohs ( stuntAddr.sin_port ), sock );
        
#ifdef ENABLE_EXT_BIND_IN_STUNT
		if ( MediatorClient::primaryInterface )
			stuntAddr.sin_addr.s_addr	= MediatorClient::primaryInterface;
//		stuntAddr.sin_addr.s_addr	= htonl ( INADDR_ANY );
#endif
        if ( !isInternal )
        {
            ret = ::bind ( sock, ( const sockaddr * ) &stuntAddr, sizeof ( struct sockaddr_in ) );
            if ( ret < 0 ) {
                CErrArgID ( "[ %s ].MakeSocket: Failed to bind socket to local mediator endpoint IP [ %s : %d ]", getChannel ( ), inet_ntoa ( stuntAddr.sin_addr ), ntohs ( stuntAddr.sin_port ) );
                LogSocketError ();
                goto Finish;
            }
        }
        
        if ( SetNonBlockSocket ( sock, true, "StunTRequest.MakeSocket" ) )
			return sock;
        
    Finish:        
        if ( IsValidFD ( sock ) ) {
            CVerbArgID ( "[ %s ].MakeSocket: Closing socket [ %i ] due to creation error.", getChannel ( ), sock );
			ShutdownCloseSocket ( sock, true, "StunTRequest.MakeSocket" );
        }
        return INVALID_FD;
    }

    
#ifndef NDEBUG
#   define LOGPOLLCHECK
#endif
    
    int StunTRequest::Establish ()
    {
#ifndef NDEBUG
        try {
#endif
        int connectSocketT	= INVALID_FD;
        int acceptSocketT	= INVALID_FD;
        int localNetSocketT	= INVALID_FD;
        
        int value = 1,
            ret = 0,
            max = native.stuntMaxTry;
        
        if ( isInitiator )
            max = max / 3;
        
        unsigned int start;
        unsigned int msPassed   = 0, msLog = 0;
        unsigned int msMax      = max * 1000;
        
        struct pollfd desc [ 3 ];

        desc [ 0 ].events = POLLIN;
        desc [ 1 ].events = POLLOUT;
        desc [ 2 ].events = POLLOUT;
        
        StunTHandler * handlers [ 3 ];
        
        int rc;
        int hasSocket	= 0;
        int socketNew	= INVALID_FD;
        
#ifdef LOGPOLLCHECK
        unsigned int lastCheck   = 0;
        int maxFails = 0;
#endif
        
        DeviceController * device = deviceSP.get ();
        
        CLogsArgID ( 3, "[ %s ].Establish: Trying STUNT connect to device with IP [ %s ] on port [ %d ]", getChannel ( ), inet_ntoa ( *((struct in_addr *) &IPe) ), Porte );
        
        if ( !Porte ) {
            CErrArgID ( "[ %s ].Establish: Invalid port 0 given!", getChannel ( ) );
            goto Failed;
        }
        
        hasSocket = INVALID_FD;
        
        do
        {
            connectSocketT = MakeSocket ();
            if ( IsInvalidFD ( connectSocketT ) ) {
                CErrArgID ( "[ %s ].Establish: Failed to create connectSocket.", getChannel ( ) ); LogSocketError ();
                break;
            }
            CSocketTraceAdd ( connectSocketT, "Establish connectSocketT" );
            
            if ( isInitiator && IP != IPe ) {
                localNetSocketT = MakeSocket ( true );
                if ( IsInvalidFD ( localNetSocketT ) ) {
                    CErrArgID ( "[ %s ].Establish: Failed to create localNetSocketT.", getChannel ( ) ); LogSocketError ();
                    break;
                }
                CSocketTraceAdd ( localNetSocketT, "Establish localNetSocketT" );
            }
            
            acceptSocketT = MakeSocket ();
            if ( IsInvalidFD ( acceptSocketT ) ) {
                CErrArgID ( "[ %s ].Establish: Failed to create acceptSocketT.", getChannel ( ) ); LogSocketError ();
                break;
            }
            CSocketTraceAdd ( acceptSocketT, "Establish acceptSocketT" );
            
            ret = listen ( acceptSocketT, 10 );
            if ( ret < 0 ) {
                CErrArgID ( "[ %s ].Establish: Failed to listen on acceptSocket", getChannel ( ) ); LogSocketError ();
                break;
            }
            
            hasSocket = 0; // Success
        }
        while ( false );
        
        if ( hasSocket == INVALID_FD ) {
            if ( IsValidFD ( acceptSocketT ) ) {
                CSocketTraceRemove ( acceptSocketT, "Establish: Closing acceptSocketT", 0 );
                closesocket ( acceptSocketT );
            }
            
            if ( IsValidFD ( connectSocketT ) ) {
                CSocketTraceRemove ( connectSocketT, "Establish: Closing connectSocketT", 0 );
                closesocket ( connectSocketT );
            }
            
            if ( IsValidFD ( localNetSocketT ) ) {
                CSocketTraceRemove ( localNetSocketT, "Establish: Closing localNetSocketT", 0 );
                closesocket ( localNetSocketT );
            }
            goto Failed;
        }
        
        if ( device ) {
            LockAcquireVA ( device->spLock, "Stunt.Establish" );
        }

		hasSocket = socketConnect;
		if ( IsValidFD ( hasSocket ) ) {
			socketConnect = INVALID_FD;
			CSocketTraceRemove ( hasSocket, "Establish: Closing socketConnect", 0 );
            closesocket ( hasSocket );
		}

        hasSocket = socketAccept;
        if ( IsValidFD ( hasSocket ) ) {
			socketAccept = INVALID_FD;
			CSocketTraceRemove ( hasSocket, "Establish: Closing socketAccept", 0 );
            closesocket ( hasSocket );
		}
        
        hasSocket = externalStunt.sock;
        if ( IsValidFD ( hasSocket ) ) {
			externalStunt.sock = INVALID_FD;
			CSocketTraceRemove ( hasSocket, "Establish: Closing externalStunt", 0 );
            closesocket ( hasSocket );
        }
		CSocketTraceVerbUpdate ( connectSocketT, "Establish assigned to externalStunt" );
        externalStunt.sock	= connectSocketT;

        hasSocket = acceptStunt.sock;
        if ( IsValidFD ( hasSocket ) ) {
			acceptStunt.sock = INVALID_FD;
			CSocketTraceRemove ( hasSocket, "Establish: Closing acceptStunt", 0 );
            closesocket ( hasSocket );
        }
		CSocketTraceVerbUpdate ( acceptSocketT, "Establish assigned to acceptStunt" );
        acceptStunt.sock	= acceptSocketT;
        
        if ( localNetSocketT > 0 ) {
			hasSocket = internalStunt.sock;
            
            if ( IsValidFD ( hasSocket ) ) {
				internalStunt.sock = INVALID_FD;
				CSocketTraceRemove ( hasSocket, "Establish: Closing internalStunt", 0 );
                closesocket ( hasSocket );
            }
			CSocketTraceVerbUpdate ( localNetSocketT, "Establish assigned to internalStunt" );
            internalStunt.sock = localNetSocketT;
        }

		hasSocket = 0;
        
        if ( device ) {
            LockReleaseVA ( device->spLock, "Stunt.Establish" );
        }
        
        if ( disposing || ( deviceStatus && ( *deviceStatus == DeviceStatus::Deleteable ) ) )
            goto Failed;
        
        start = GetEnvironsTickCount32 ();
        
#ifdef LOGPOLLCHECK
        lastCheck   = (GetEnvironsTickCount32 () - start);
#endif
        if ( localNetSocketT >= 0 && !internalStunt.Init ( this, true ) )
            goto Failed;
        
        if ( !externalStunt.Init ( this, true ) )
            goto Failed;
        
        if ( !acceptStunt.Init ( this, false ) )
            goto Failed;
        
        if ( localNetSocketT >= 0 && internalStunt.Connect () ) {
            hasSocket = 1;
            goto Done;
        }
        
        if ( externalStunt.Connect () ) {
            hasSocket = 1;
            goto Done;
        }

        int rc1;

        desc [ 0 ].fd = acceptSocketT;
        
        while ( IsInvalidFD ( socketConnect ) && IsInvalidFD ( socketAccept ) )
        {
            int size = 1;
            
            desc [ 0 ].revents = 0;
            
            connectSocketT = externalStunt.sock;
            if ( IsValidFD ( connectSocketT ) )
            {
                handlers [ size ]       = &externalStunt;
                desc [ size ].revents   = 0;
                desc [ size ].fd        = connectSocketT; ++size;
            }
            
            localNetSocketT = internalStunt.sock;
            if ( IsValidFD ( localNetSocketT ) )
            {
                handlers [ size ]       = &internalStunt;
                desc [ size ].revents   = 0;
                desc [ size ].fd        = localNetSocketT; ++size;
            }
            
            rc1 = rc = poll ( desc, size, 1000 );
            
            if ( rc == INVALID_FD ) {
                CWarnsArgID ( 2, "[ %s ].Establish: Closing STUNT due to error!", getChannel () );
                break;
            }
            
            // Detect whether environs is about to be disposed
            if ( disposing || ( deviceStatus && ( *deviceStatus == DeviceStatus::Deleteable || *deviceStatus == DeviceStatus::Connected ) ) || env->environsState < environs::Status_Starting ) {
                disposing = true;
                CWarnsArgID ( 2, "[ %s ].Establish: Closing STUNT due to disposal request!", getChannel () );
                goto Failed;
            }
            
            if ( rc != 0 ) {
                hasSocket = 0;
                short revents;
                
                if ( size == 3 ) {
                    revents = desc [ 2 ].revents;

                    if ( revents & POLLERRMASK )
                        break;

                    if ( revents & POLLOUT ) {
                        if ( handlers [ 2 ]->Connect () )
                            hasSocket++;
                        rc--;
                    }
                }
                
                if ( rc > 0 && size > 1 ) {
                    revents = desc [ 1 ].revents;

                    if ( revents & POLLERRMASK )
                        break;

                    if ( revents & POLLOUT ) {
                        if ( handlers [ 1 ]->Connect () )
                            hasSocket++;
                        rc--;
                    }
                }
                
                if ( rc > 0 && ( desc [ 0 ].revents & POLLIN ) ) {
                    revents = desc [ 0 ].revents;

                    if ( revents & POLLERRMASK )
                        break;

                    if ( revents & POLLIN ) {
                        if ( acceptStunt.Accept () )
                            hasSocket++;
                        rc--;
                    }
                }
                
                if ( hasSocket ) {
                    CVerbsArgID ( 2, "[ %s ].Establish: Succeeded with [ %i ] sockets!", getChannel (), hasSocket );
                    break;
                }

                if ( rc > 0 ) {
                    CVerbsArgID ( 2, "[ %s ].Establish: Poll error [ %i ] sockets!", getChannel (), rc );
                    goto Failed;
                }
            }
            else {
                CVerbsArgID ( 3, "[ %s ].Establish: Timeout [ %d ]", getChannel (), msPassed );
            }
            
            msPassed = (GetEnvironsTickCount32 () - start);
            
#ifdef LOGPOLLCHECK
            if ( (msPassed - lastCheck) < 30 ) {
                maxFails++;

                CVerbArgID ( "[ %s ].Establish: STUNT CHECK [ rc1 %d : rc %d : size %d : msPassed %d : socketConnect %d : socketAccept %d ]", getChannel (), rc1, rc, size, msPassed, socketConnect, socketAccept );
                CVerbArgID ( "[ %s ].Establish: STUNT CHECK [ sock %d : %d : %d : sock %d : state %d ]", getChannel (), desc[0].fd, desc[0].events, desc[0].revents, acceptStunt.sock, acceptStunt.state );
                CVerbArgID ( "[ %s ].Establish: STUNT CHECK [ sock %d : %d : %d : sock %d : state %d ]", getChannel (), desc[1].fd, desc[1].events, desc[1].revents, handlers [ 1 ] ? handlers [ 1 ]->sock : -1, handlers [ 1 ] ? handlers [ 1 ]->state : -1 );

#ifdef DEBUGVERB
                if (size > 2)
					CVerbArgID ( "[ %s ].Establish: STUNT CHECK [ sock %d : %d : %d : sock %d : state %d ]", getChannel (), desc[2].fd, desc[2].events, desc[2].revents, handlers [ 2 ] ? handlers [ 2 ]->sock : -1, handlers [ 2 ] ? handlers [ 2 ]->state : -1 );
#endif                
                if ( (maxFails % 10) == 0 ) {
                    _EnvDebugBreak ( "Stunt.Request::Establish" );
                }
            }

            lastCheck  = msPassed;
#endif
            if ( msPassed > ( msLog + 2000 ) )
            {
                msLog = msPassed;
                CLogArgID ( "[ %s ].Establish: STUNT waiting [ %d : %d ]", getChannel (), msPassed, msLog );
                if ( deviceSP )
                    deviceSP->UpdateConnectStatus ( 1, false );
            }
            
            if ( msPassed > msMax ) {
                CWarnsArgID ( 2, "[ %s ].Establish: Giving up STUNT due to exceeded max wait time!", getChannel () );
                goto Failed;
            }
        }
        
    Done:
        // It's critical here when the according DeviceController shuts down...
        if ( disposing || ( deviceStatus && *deviceStatus == DeviceStatus::Deleteable ) ) {
            disposing = true;
            CWarnsArgID ( 2, "[ %s ].Establish: Closing STUNT due to disposal request (after listening)!", getChannel ( ) );
            goto Failed;
        }
        
        if ( !hasSocket ) {
            CErrArgID ( "[ %s ].Establish: Failed!", getChannel () );
            goto Failed;
        }
        
        // If we are the initiator, then we prefer the acceptSocket
        if ( isInitiator )
        {
            CVerbsArgID ( 3, "[ %s ].Establish: Collecting sockets as initiator.", getChannel ( ) );
            
            if ( IsValidFD ( socketAccept ) ) {
                socketNew = socketAccept; socketAccept = INVALID_FD;
				CSocketTraceVerbUpdate ( socketNew, "Establish taken over socketAccept to socketNew" );
                
                memcpy ( &addr, &acceptStunt.address, sizeof ( struct sockaddr_in ) );
                
                CVerbsArgID ( 3, "[ %s ].Establish: Collected accept socket.", getChannel ( ) );
            }
        }
        
        if ( IsValidFD ( socketConnect ) )
        {
            CVerbsArgID ( 3, "[ %s ].Establish: Collecting connect socket.", getChannel ( ) );
            
            if ( IsInvalidFD ( socketNew ) ) {
                socketNew = socketConnect; socketConnect = INVALID_FD;
				CSocketTraceVerbUpdate ( socketNew, "Establish taken over socketConnect to socketNew" );
                
                CVerbsArgID ( 3, "[ %s ].Establish: Collected connect socket.", getChannel () );
            }
        }
        
        if ( !isInitiator )
        {
            CVerbsArgID ( 3, "[ %s ].Establish: Collecting accept socket as responder.", getChannel ( ) );
            
            if ( IsValidFD ( socketAccept ) ) {
                CVerbsArgID ( 0, "[ %s ].Establish: Accept socket available.", getChannel ( ) );
                
                if ( IsInvalidFD ( socketNew ) ) {
                    socketNew = socketAccept; socketAccept = INVALID_FD;
					CSocketTraceVerbUpdate ( socketNew, "Establish taken over socketAccept to socketNew" );

                    memcpy ( &addr, &acceptStunt.address, sizeof ( struct sockaddr_in ) );
                    
                    CVerbsArgID ( 3, "[ %s ].Establish: Collected accept socket.", getChannel () );
                }
            }
        }
        
        if ( !SetNonBlockSocket ( socketNew, false, "StunTHandler.Establish" ) )
            goto Failed;
        
        if ( !Handshake ( socketNew ) )
            goto Failed;
        
        CLogArgID ( "[ %s ].Establish: Connected [ %s : %d ], Socket [ %i ]", getChannel (), inet_ntoa ( addr.sin_addr ), ntohs ( addr.sin_port ), socketNew );
        
#ifdef USE_ADDR_REUSE_ONLY_FOR_CONNECT
        value = 0;
        rc = setsockopt ( socketNew, SOL_SOCKET, SO_REUSEADDR, ( const char * ) &value, sizeof ( value ) );
        if ( rc < 0 ) {
            CErrArgID ( "[ %s ].Establish: Failed to reset reuse on new socket.", getChannel () ); LogSocketError ();
            goto Failed;
        }
#else
        value = 1;
        ret = setsockopt ( socketNew, SOL_SOCKET, SO_REUSEADDR, ( const char * ) &value, sizeof ( value ) );
        if ( ret < 0 ) {
            CErrArgID ( "[ %s ].Establish: Failed to set reuse on new socket.", getChannel () ); LogSocketError ();
            goto Failed;
        }
#endif
        CloseThreads ( false );
        
        CVerbsArgID ( 2, "[ %s ].Establish: STUNT success!", getChannel ( ) );
        
		CSocketTraceVerbUpdate ( socketNew, "Establish: returned socketNew to caller." );
        return socketNew;
        
    Failed:
        
        if ( IsValidFD ( socketNew ) ) {
            
            CVerbArgID ( "[ %s ].Establish: Closing successful STUNT socket due to DeviceController disposal.", getChannel ( ) );
            ShutdownCloseSocket ( socketNew, true, "Establish stunt" );
        }
        
            CloseThreads ( false );

#ifndef NDEBUG
        }
        catch ( char * )
        {
            printf ( "StunTHandler::Establish: Exception !!!\n" );
            _EnvDebugBreak ( "StunTHandler::Establish" );
        }
#endif
        
        return INVALID_FD;
    }
    

	bool StunTHandler::Init ( StunTRequest * request, bool isConnect )
	{
		state   = 0;
		req     = request;
        fails   = 0;
		port	= 0;
		channel = request->channel;

		Zero ( address );
		address.sin_family = PF_INET;

		if ( isConnect )
		{
			if ( isInternal ) {
				port			= request->Porti;
				address.sin_addr.s_addr = request->IP;
			}
			else {
				CVerbArg ( "[ %s ].Init on external IP and port", getChannel () );

				port					= request->Porte;
				address.sin_addr.s_addr = request->IPe;
			}
			address.sin_port	= htons ( ( unsigned short ) port );

			CLogsArg ( 2, "[ %s ].Init: Connecting to [ %s ] [ %s : %d ], Socket [ %i ].", getChannel (), isInternal ? "internal" : "external", inet_ntoa ( address.sin_addr ), port, sock );
		}
		else
		{
			sockaddr_in addrAcc;
			Zero ( addrAcc );

			socklen_t length = sizeof ( addrAcc );

			int rc = getsockname ( sock, ( struct sockaddr * ) &addrAcc, &length );
			if ( rc ) {
				CErrArg ( "[ %s ].Init: Failed to identify local acceptor IP/Port on socket [ %d ]!", getChannel (), sock );
			}

			CLogsArg ( 2, "[ %s ].Init:  Try accepting on [ %s : %d ], Socket [ %i ] ...", getChannel (), inet_ntoa ( addrAcc.sin_addr ), ntohs ( addrAcc.sin_port ), sock );
		}

		return true;
	}


	bool StunTHandler::Connect ()
    {
        CVerbsArg ( 2, "[ %s ].Connect: [ %s ] state [ %i ] repeats [ %i ]", getChannel (), isInternal ? "internal" : "external", state, req->repeats );
        
        if ( state == 1 ) {
            CVerbsArg ( 2, "[ %s ].Connect: [ %s ] State 1!", getChannel (), isInternal ? "internal" : "external" );
#if defined (_WIN32)
			state = 2;
#else
			int check;
			socklen_t len = sizeof ( check );

			if ( getsockopt ( sock, SOL_SOCKET, SO_ERROR, ( char * ) &check, &len ) != 0 )
			{
				CVerbsArg ( 2, "[ %s ].Connect: [ %s ] Get socket opt failed", getChannel (), isInternal ? "internal" : "external" );
                VerbLogSocketError ();
				state = 3;
			}
			else if ( check )
            {
                if ( SOCK_IN_PROGRESS_Check ( check ) )
                {
                    CVerbsArg ( 3, "[ %s ].Connect: [ %s ] In progress ....!", getChannel (), isInternal ? "internal" : "external" );
                    return false;
                }
                
				CVerbsArg ( 2, "[ %s ].Connect: [ %s ] Failed with error [ %i ]!", getChannel (), isInternal ? "internal" : "external", check );
				VerbLogSocketErrorF ( "StunTHandler.Connect" );
                fails++;
				state = 3;
			}
            else {
                state = 2;
                CVerbsArg ( 2, "[ %s ].Connect: [ %s ] Succeeded!", getChannel (), isInternal ? "internal" : "external" );
            }
#endif
		}
		
        if ( state == 3 ) {
            CVerbsArg ( 2, "[ %s ].Connect: [ %s ] Closing socket and recreating.", getChannel (), isInternal ? "internal" : "external" );
            
            int s = sock; sock = INVALID_FD;

            if ( IsValidFD ( s ) ) {
                SetNonBlockSocket ( s, true, "StunTHandler.Connect" );
                
                CSocketTraceUpdate ( s, "StunTRequest shutdown 2 in Connect" );
				shutdown ( s, 2 );

				CSocketTraceRemove ( s, "Connect: Closing", 0 );
                closesocket ( s );
			}
            
            if ( fails < 20 ) {
                s = req->MakeSocket ( isInternal );
                if ( IsInvalidFD ( s ) ) {
                    CVerbsArg ( 2, "[ %s ].Connect: Failed to create socket.", getChannel ( ) );
                    return false;
                }
                CSocketTraceAdd ( s, "StunTHandler.Connect localNetSocketT" );
                sock = s;
                state = 0;
            }
        }
        
		if ( state == 0 )
        {
            CVerbsArg ( 2, "[ %s ].Connect: [ %s ] Connect to [ %s : %d ] !", getChannel (), isInternal ? "internal" : "external", inet_ntoa ( address.sin_addr ), ntohs ( address.sin_port ) );
            
            int rc = ::connect ( sock, ( struct sockaddr * ) &address, sizeof ( address ) );
            if ( rc < 0 )
            {
				SOCKET_Check_Val ( check );
                VerbLogSocketError_Check ( check );
                //LogSocketError ();
                
                CVerbsArg ( 3, "[ %s ].Connect: [ %s ] Status [ %i ]!", getChannel (), isInternal ? "internal" : "external", check );
                
                if ( SOCK_IN_PROGRESS_Check ( check ) )
                {
                    state = 1;
                    CVerbsArg ( 3, "[ %s ].Connect: [ %s ] In progress ...!", getChannel (), isInternal ? "internal" : "external" );
                    return false;
                }
                
                // Retry
                CVerbsArg ( 2, "[ %s ].Connect: [ %s ] Call failed [ %i ]!", getChannel (), isInternal ? "internal" : "external", check );
                VerbLogSocketError_Check ( check );
                return false;
            }
            else {
                // We seem to be connected immediately after the connect call
                CVerbsArg ( 2, "[ %s ].Connect: [ %s ] Succeeded with first call!", getChannel (), isInternal ? "internal" : "external" );
                state = 2;
            }
        }
        
        if ( state == 2 ) 
        {
			CSocketTraceVerbUpdate ( sock, "StunTRequest assigned to socketConnect in Connect" );
            
            if ( IsInvalidFD ( req->socketConnect ) )
            {
                req->socketConnect = sock; sock = INVALID_FD;
                
                CVerbsArg ( 2, "[ %s ].Connect: Successfully connected to [ %s : %s : %d ]", getChannel (), isInternal ? "internal" : "external", inet_ntoa ( address.sin_addr ), ntohs ( address.sin_port ) );
                
                memcpy ( &req->addr, &address, sizeof ( struct sockaddr_in ) );
                return true;
            }
#ifndef NDEBUG
            else {
                CVerbsArg ( 2, "[ %s ].Connect: Ignoring successful connection to [ %s : %s : %d ]. The other connect has already been accepted.", getChannel (), isInternal ? "internal" : "external", inet_ntoa ( address.sin_addr ), ntohs ( address.sin_port ) );
            }
#endif
		}
		return false;
	}


	bool StunTHandler::Accept ()
    {
        CVerbsArg ( 6, "[ %s ].Accept: repeats [ %i ]!", getChannel (), req->repeats );
        
		socklen_t			addrLen = sizeof ( address );

		int sockRC = ( int ) ::accept ( sock, ( struct sockaddr * )&address, &addrLen );
		if ( sockRC < 0 )
		{
			SOCKET_Check_Val ( check );
			CVerbsArg ( 2, "[ %s ].Accept: [ %i ]!", getChannel (), check );

			if ( SOCKET_Check_Retry ( check ) )
            {
                CVerbsArg ( 3, "[ %s ].Accept: In progress ...!", getChannel () );
				return false;
			}

			// Error
            CErrArg ( "[ %s ].Accept: Call failed [ %i ]!", getChannel (), check );
            
            if ( IsValidFD ( sock ) ) { LogSocketErrorF ( "StunTHandler.Accept" ); }
            req->disposing = true;
			return false;
		}

		CSocketTraceAdd ( sockRC, "StunTRequest Accept" );
		req->socketAccept = sockRC;

		CVerbsArg ( 2, "[ %s ].Accept: Successfully connected from [ %s : %d ]", getChannel (), inet_ntoa ( address.sin_addr ), ntohs ( address.sin_port ) );
		return true;
	}

    
    bool StunTRequest::Handshake ( int sock )
    {
		// Use STUNTHandshakePacket to exchange the application environment

        //CVerbsArgID ( 2, "[ %s ].Handshake [ %s ]", getChannel ( ), pthread_is_self_thread ( internalStunt.thread.threadID ) ? "internal" : "external" );
        
        if ( !SocketTimeout ( sock, 30, 30 ) )
            return false;
        
        bool	success = false;

		char	buffer [ STUNT_HANDSHAKE_IDENT_SIZE ];
        
		buffer [ 0 ] = 'c';
		buffer [ 1 ] = 't';
		buffer [ 2 ] = 'd';
		buffer [ 3 ] = ';';
        
        int *	pID		= (int *)(buffer + 4);        
			*	pID		= env->deviceID;

		int length = ( int ) send ( sock, buffer, STUNT_HANDSHAKE_IDENT_SIZE, MSG_NOSIGNAL );

		if ( length != ( int ) STUNT_HANDSHAKE_IDENT_SIZE )
		{
			CWarnArgID ( "[ %s ].Handshake: Send failed sizes %i != %i", getChannel (), length, STUNT_HANDSHAKE_IDENT_SIZE ); LogSocketError ();
			goto Finish;
		}

        *buffer = 0;
        
		length = ( int ) recv ( sock, buffer, STUNT_HANDSHAKE_IDENT_SIZE, 0 );

		if ( length < STUNT_HANDSHAKE_IDENT_SIZE || buffer [ 0 ] != 'c' || buffer [ 1 ] != 't' || buffer [ 2 ] != 'd' || buffer [ 3 ] != ';' )
		{
			CWarnArgID ( "[ %s ].Handshake: Invalid STUNT response", getChannel () );
			return false;
		}

		if ( *pID == deviceID ) {
			CVerbsArgID ( 2, "[ %s ].Handshake: Success", getChannel () );
			success = true;
		}
		else {
			CWarnArgID ( "[ %s ].Handshake: Wrong destination ID [%d]", getChannel (), env->deviceID );
		}

	Finish:
		if ( !SocketTimeout ( sock, 0, 0 ) )
			return false;

		return success;
    }
    

    /*
	* Called by DeviceBase
	*/
    int StunTRequest::Handshake ( Instance * env, int sock, char * buffer )
    {
		// Use STUNTHandshakePacket to exchange the application environment

        if ( buffer [ 0 ] != 'c' || buffer [ 1 ] != 't' || buffer [ 2 ] != 'd' || buffer [ 3 ] != ';' )
		{
			CWarn ( "Handshake: No STUNT request" );
			return 0;
		}

		int * pID = ( int * ) ( buffer + 4 );

		int deviceID = *pID;

		CVerbID ( "Handshake: STUNT request by [ 0x%X ]" );

		*pID = env->deviceID;

        int length = ( int ) send ( sock, buffer, STUNT_HANDSHAKE_IDENT_SIZE, MSG_NOSIGNAL );
		if ( length != ( int ) STUNT_HANDSHAKE_IDENT_SIZE )
		{
			CWarnArgID ( "Handshake: Send failed sizes [ %i != %i ]", length, STUNT_HANDSHAKE_IDENT_SIZE ); LogSocketError ();
			return -1;
		}

		CLogID ( "Handshake: STUNT ok. Start device handshake." );
		return 1;
    }
    
    
    void StunTRequest::HandleRequest ( )
    {
        CVerbArgID ( "[ %s ].HandleRequest", getChannel ( ) );
                
        int						sock		= INVALID_FD;
        
		if ( !deviceSP ) {
			CErrArgID ( "[ %s ].HandleRequest: Invalid device controller.", getChannel () );
			return;
        }

#ifndef NDEBUG
        try {
#endif

		DeviceBase			*	device		= deviceSP.get ();

        int					*	destSocket          = 0;
        LONGSYNC			*	destState           = 0;
        
        if ( channel == MEDIATOR_STUNT_CHANNEL_BULK ) {
            CVerbArgID ( "[ %s ].HandleRequest: Request for comDat channel.", getChannel ( ) );
            
            destSocket  = &device->comDatSocket;
            destState   = &device->stuntComDatState;
        }
        else //if ( channel == MEDIATOR_STUNT_CHANNEL_MAIN ) // For every other channel type, we assume a temporary devciebase and make use of the main channel's resources
        {
            CVerbArgID ( "[ %s ].HandleRequest: Request for interact channel.", getChannel ( ) );
            
            destSocket  = &device->interactSocket;
            destState   = &device->stuntInteractState;
        }
        
        if ( ___sync_val_compare_and_swap ( destState, ENVIRONS_THREAD_NO_THREAD, ENVIRONS_THREAD_DETACHEABLE ) != ENVIRONS_THREAD_NO_THREAD )
            return;
        
        if ( device->deviceStatus == DeviceStatus::Deleteable )
            goto Finish;
        
        if ( IsValidFD ( *destSocket ) ) {
            CWarnArgID ( "[ %s ].HandleRequest: Socket for channel has already been established.", getChannel ( ) );
            goto Finish;
        }
        
#ifdef ENABLE_DEVICEBASE_WP_STUNT
        if ( channel == MEDIATOR_STUNT_CHANNEL_MAIN )
            device->stuntInteract = myself;
        else
            device->stuntComDat = myself;
#else
		LockAcquireVA ( device->spLock, "HandleRequest" );

        if ( channel == MEDIATOR_STUNT_CHANNEL_MAIN )
            device->stuntInteract = myself;
        else
            device->stuntComDat = myself;

		LockReleaseVA ( device->spLock, "HandleRequest" );
#endif
        sock = Establish ( );
        
        if ( IsInvalidFD ( sock ) ) {
			if ( !disposing ) {
				CVerbsArgID ( 4, "[ %s ].HandleRequest: STUNT failed.", getChannel () );
			}

            // ** Failed to connect
            // Leave destruction to gc
            // An internal connect may have or will succeed soon.
            // - Disable device if not otherwise instructed
            //
            /*if ( disposeDevice && IsInvalidFD ( *destSocket ) ) {
                CVerbArgID ( "[ %s ].HandleRequest: Setting deviceStatus to Deleteable", getChannel ( ) );
				deviceSP->deviceStatus = DeviceStatus::Deleteable;
                
                TriggerCleanUpDevices ( );
            }
            */
        }
        else {
            CSocketTraceUpdate ( sock, "StunTRequest assigning to device in HandleRequest" );
            
            if ( !DeviceBase::HandshakeAndResponse ( env, sock, &addr ) ) {
                // ** Failed to handshake
                // Leave destruction to gc
                // An internal connect may have or will succeed soon.
                if ( !disposing ) {
                    CVerbsArgID ( 4, "[ %s ].HandleRequest: Hanshake failed.", getChannel () );
                }
                
                //CVerbArgID ( "[ %s ].HandleRequest: Setting deviceStatus to Deleteable", getChannel ( ) );
				//deviceSP->deviceStatus = DeviceStatus::Deleteable;
                
                //TriggerCleanUpDevices ( );
            }
            
            CVerbArgID ( "[ %s ].HandleRequest: Clearing stunt connect thread (caused by a Mediator event).", getChannel ( ) );
        }

#ifndef ENABLE_DEVICEBASE_WP_STUNT
		LockAcquireVA ( device->spLock, "HandleRequest" );

		if ( channel == MEDIATOR_STUNT_CHANNEL_MAIN )
			device->stuntInteract = 0;
		else
			device->stuntComDat = 0;

		LockReleaseVA ( device->spLock, "HandleRequest" );
#endif
        
    Finish:
            *destState	= ENVIRONS_THREAD_NO_THREAD;

#ifndef NDEBUG
        }
        catch ( char * )
        {
            printf ( "StunTRequest::HandleRequest: Exception !!!\n" );
            _EnvDebugBreak ( "StunTRequest::HandleRequest" );
        }
#endif
    }
    
    
    ///
    /// We are executed in the context of a TcpListener or BulkListener
    ///
    bool StunTRequest::EstablishRequest ( Instance * env, DeviceBase * device, char channel, unsigned int token )
    {
		if ( !device ) {
			CErrArg ( "[ %s ].EstablishRequest: Invalid argument.", getChannel () );
			return false;
        }

#ifndef NDEBUG
        try {
#endif
        int						deviceID	= device->deviceID;
        
        CVerbArgID ( "[ %s ].EstablishRequest", getChannel ( ) );

		int		sock;
        bool	success	= false;
        int     retries = 3;
        
#ifdef ENABLE_DEVICEBASE_WEAK_REFERENCE
        sp ( DeviceController ) deviceSP    = device->myself.lock ();
#else
		LockAcquireVA ( device->spLock, "EstablishRequest" );

        sp ( DeviceController ) deviceSP    = device->myself;

		LockReleaseVA ( device->spLock, "EstablishRequest" );
#endif
		if ( !deviceSP ) {
			CErrArgID ( "[ %s ].EstablishRequest: Invalid device controller.", getChannel () );
			return false;
		}

        sp ( StunTRequest )     request;
        struct sockaddr_in	*	sockAddr			= 0;
        int					*	destSocket			= 0;
		int					*	destSocketForClose	= 0;
        ThreadSync          *   destLock            = 0;
        LONGSYNC			*	destState			= 0;
        
        
        if ( channel == MEDIATOR_STUNT_CHANNEL_MAIN ) {
            CLogArgID ( "[ %s ].EstablishRequest: Request for TCP interact channel.", getChannel ( ) );
            
            destState			= &device->stuntInteractState;
            destSocket			= &device->interactSocket;
            destSocketForClose	= &device->interactSocketForClose;
            destLock            = &device->interactThread;
            sockAddr			= &device->interactAddr;
        }
        else if ( channel == MEDIATOR_STUNT_CHANNEL_BULK ) {
            CLogArgID ( "[ %s ].EstablishRequest: Request for TCP comDat channel.", getChannel ( ) );
            
            destState			= &device->stuntComDatState;
            destSocket			= &device->comDatSocket;
            destSocketForClose	= &device->comDatSocketForClose;
            destLock            = &device->comDatThread;
            sockAddr			= &device->comDatAddr;
        }
        
        if ( !destState ) {
            CErrArgID ( "[ %s ].HandleRequest: No channel for socket provided.", getChannel () );
            return false;
        }
        
        if ( ___sync_val_compare_and_swap ( destState, ENVIRONS_THREAD_NO_THREAD, ENVIRONS_THREAD_DETACHEABLE ) != ENVIRONS_THREAD_NO_THREAD )
            return false;
        
        if ( device->deviceStatus == DeviceStatus::Deleteable )
			goto FinishAndReset;
        
        if ( IsValidFD ( *destSocket ) ) {
            CWarnArgID ( "[ %s ].EstablishRequest: Socket for channel has already been established.", getChannel ( ) );
			success = true;
			goto FinishAndReset;
        }
        
        request = StunTRequest::CreateRequest ( env, &device->deviceStatus, deviceID, device->deviceAreaName, device->deviceAppName, channel, token );
        if ( !request ) {
			if ( env->environsState >= environs::Status::Starting ) {
				CWarnArgID ( "[ %s ].EstablishRequest: Failed to create StunTRequest", getChannel () );
			}
            goto FinishAndReset;
        }
       
        
#ifdef ENABLE_DEVICEBASE_WEAK_REFERENCE
        request->deviceSP		= device->myself.lock ();
#else
        LockAcquireVA ( device->spLock, "EstablishRequest" );
        
        request->deviceSP		= device->myself;
#endif
		request->deviceStatus	= &device->deviceStatus;

		if ( channel == MEDIATOR_STUNT_CHANNEL_MAIN )
			device->stuntInteract	= request;
		else
			device->stuntComDat		= request;
        
#ifndef ENABLE_DEVICEBASE_WEAK_REFERENCE
        LockReleaseVA ( device->spLock, "EstablishRequest" );
#endif
    Retry:
        sock = request->Establish ( );
        
        if ( IsInvalidFD ( sock ) )
        {
            retries--;
			if ( /*!request->disposing &&*/ retries > 0 ) {
                if ( (deviceSP->deviceStatus != DeviceStatus::Deleteable) && env->environsState >= environs::Status::Started && (!request->deviceStatus || *request->deviceStatus != DeviceStatus::Deleteable) ) {
					request->disposing = false;
                    goto Retry;
                }
				CWarnArgID ( "[ %s ].EstablishRequest: STUNT failed.", getChannel () );
            }
            
            // Leave destruction to gc
            // An internal connect may have or will succeed soon.
            
            //CVerbArgID ( "[ %s ].EstablishRequest: setting deviceStatus to Deleteable", getChannel ( ) );
			//deviceSP->deviceStatus = DeviceStatus::Deleteable;

			//TriggerCleanUpDevices ();
        }
        else {
            success = true;
            
            destLock->Lock ( "EstablishRequest" );

            if ( IsInvalidFD ( *destSocketForClose ) )
            {
				CSocketTraceUpdate ( sock, "StunTRequest assigned to device in EstablishRequest" );

                *destSocket			= sock;
                *destSocketForClose = sock;
                
                memcpy ( sockAddr, &request->addr, sizeof(struct sockaddr_in) );
                
                CInfoArgID ( "[ %s ].EstablishRequest: STUNT successful.", getChannel ( ) );
            }
            else {
                // This may happen if a direct connect through TcpHandlers have been established meanwhile
                CSocketTraceRemove ( sock, "EstablishRequest: Closing", 0 );
                closesocket ( sock );
                
                //sock = INVALID_FD;
            }
            
            destLock->Unlock ( "EstablishRequest" );
        }

#ifndef ENABLE_DEVICEBASE_WP_STUNT
        LockAcquireVA ( device->spLock, "EstablishRequest" );

        if ( channel == MEDIATOR_STUNT_CHANNEL_MAIN )
			device->stuntInteract	= 0;
		else
			device->stuntComDat		= 0;

		LockReleaseVA ( device->spLock, "EstablishRequest" );
#endif
        request->myself = 0;

	FinishAndReset:
        *destState	= ENVIRONS_THREAD_NO_THREAD;
        
            return success;

#ifndef NDEBUG
        }
        catch ( char * )
        {
            printf ( "StunTRequest: EstablishRequest !!!\n" );
            _EnvDebugBreak ( "StunTRequest: EstablishRequest" );
        }
        return false;
#endif
    }



#undef	CLASS_NAME
#define	CLASS_NAME 	"Stun.Request . . . . . ."
    
    
    StunRequest::StunRequest ( int deviceID )
    {
        this->deviceID	= deviceID;

		__sync_add_and_fetch ( &stunCount, 1 );
        
        CVerbID ( "Construct" );
        
        env				= 0;
        disposing		= false;
        doHandshake		= true;
        isRequestor		= false;
        
        threadState     = ENVIRONS_THREAD_NO_THREAD;
        
        IPi             = 0;
        IPe				= 0;
        Porti			= 0;
        Porte			= 0;
        
		allocated		= 0;
    }
    
    
    StunRequest::~StunRequest ( )
    {
        CVerbID ( "Destruct" );
        
        CloseThreads ( );

		__sync_sub_and_fetch ( &stunCount, 1 );
    }


	bool StunRequest::Init ()
	{
        if ( !allocated )
        {
            if ( !thread.Init() )
                return false;
            thread.autoreset = true;
            
            allocated = true;
        }
		return true;
	}
    
    
    void StunRequest::CloseThreads ( )
    {
        CVerbID ( "CloseThreads" );
        
        disposing	= true;
        doHandshake = false;
        
		thread.Join ( "STUN udp" );
        
        ReleaseSP ();
    }
    
    
    void StunRequest::ReleaseSP ( )
    {
        CVerbID ( "ReleaseSP" );
        
        //
        // Don't release the request as long as a thread is running and relying on this resource
        //
        if ( ___sync_val_compare_and_swap ( &threadState, ENVIRONS_THREAD_NO_THREAD, ENVIRONS_THREAD_NO_THREAD ) != ENVIRONS_THREAD_NO_THREAD )
            return;
        
        thread.Lock ( "StunRequest.ReleaseSP" );
        
        myself = 0;
        
        thread.Unlock ( "StunRequest.ReleaseSP" );
    }
    

#ifndef ENABLE_DEVICEBASE_WP_STUN
    void StunRequest::ReleaseDeviceRequest ( )
    {
        CVerbID ( "ReleaseDeviceRequest" );
        
        if ( !deviceSP )
            return;

		LockAcquireVA ( deviceSP->spLock, "ReleaseDeviceRequest" );

		if ( deviceSP )
			deviceSP->stun = 0;

		LockReleaseVA ( deviceSP->spLock, "ReleaseDeviceRequest" );
    }
#endif
    
    
    bool StunRequest::EstablishRequest ( DeviceBase * device )
    {
        CVerb ( "EstablishRequest" );
        
#ifdef ENABLE_DEVICEBASE_WEAK_REFERENCE
        sp ( DeviceController ) deviceSP = device->myself.lock ();
#else
		LockAcquireVA ( device->spLock, "EstablishRequest" );

        sp ( DeviceController ) deviceSP = device->myself;

		LockReleaseVA ( device->spLock, "EstablishRequest" );
#endif
        if ( !deviceSP )
            return false;
        
        int deviceID = device->deviceID;
        
        CVerbID ( "EstablishRequest" );
        
        if ( ___sync_val_compare_and_swap ( &device->stunState, ENVIRONS_THREAD_NO_THREAD, ENVIRONS_THREAD_DETACHEABLE ) != ENVIRONS_THREAD_NO_THREAD ) {
            CVerbID ( "EstablishRequest: Stun already ongoing." );
            return true;
        }
        
        sp ( StunRequest ) request;
        
        if ( device->udpCoreConnected )
            goto Finish;
        
        request = std::make_shared < StunRequest > ( deviceID );
        if ( !request || !request -> Init () ) {
            CErrID ( "EstablishRequest: Failed to allocate a udp stun object." );
            goto Finish;
        }
        
        request->env			= device->env;
        request->isRequestor	= true;
        request->deviceSP		= deviceSP;
        
        request->myself         = request;
        
        request->threadState    = ENVIRONS_THREAD_DETACHEABLE;
        
        if ( !request->thread.Run ( pthread_make_routine ( &StunRequest::EstablishStarter ), request.get (), "StunRequest.EstablishRequest" ) )
        {
            CErrID ( "EstablishRequest: Failed to create thread!" );
            
            request->myself         = 0;
            goto Finish;
        }

		LockAcquireVA ( device->spLock, "EstablishRequest" );
        
        device->stun = request;

		LockReleaseVA ( device->spLock, "EstablishRequest" );
        return true;
        
    Finish:
        device->stunState = ENVIRONS_THREAD_NO_THREAD;
        return false;
    }
    
    
    /*
     * HandleIncomingRequest:
     * - This method is called by the mediator multiple times for both requestor and responder
     * - The responder has to EstablishSTUN once
     * - The requestor has to update the PortUdp
     */
    void StunRequest::HandleIncomingRequest ( Instance * env, void * msg )
    {
        CVerb ( "HandleIncomingRequest" );

		STUNReqReqHeader *	req = ( STUNReqReqHeader * ) msg;
        
        sp ( StunRequest ) request;
        const char *	areaName	= 0;
        const char *	appName		= 0;
        
        int             deviceID	= req->deviceID;
        unsigned int	IPe			= req->IPe;
        unsigned int	IPi			= req->IPi;
        unsigned short	Porti		= 0;
		unsigned short	Porte		= ( unsigned short ) req->Porte;
        
		DeviceBase *	device		= 0;
        
        if ( !deviceID || !IPe || !Porte || !IPi ) {
            CErrArgID ( "HandleIncomingRequest: Invalid values - ports [ %d : %d ]", Porti, Porte );
            return;
        }

		CVerbArgID ( "HandleIncomingRequest: STUN request to device [ %s ] on ports [ %d : %d ]", inet_ntoa ( *( ( struct in_addr * ) &IPe ) ), Porte, Porti );

		if ( req->sizes [ 0 ] >= 1 && req->sizes [ 1 ] >= 1 && req->sizes [ 0 ] < MAX_NAMEPROPERTY && req->sizes [ 1 ] < MAX_NAMEPROPERTY ) {
			appName  = ( ( STUNReqReqPacket * ) req )->appArea;
			areaName = appName + req->sizes [ 0 ];
		}
        
        // Find the device
		sp ( DeviceController ) deviceSP = GetDeviceSP ( env, deviceID, areaName, appName );
        if ( !deviceSP ) {
            CVerbID ( "HandleIncomingRequest: device not found" );
            return;
        }
        device = deviceSP.get ();
        
        if ( device->udpCoreConnected )
            return;
        
        sp ( MediatorClient ) mediator = env->mediator MED_WP;
        if ( mediator ) {
            sp ( DeviceInstanceNode ) deviceInst = mediator->GetDeviceNearbySP ( deviceID, areaName, appName );
            if ( deviceInst ) {                
                Porti = deviceInst->info.udpPort;
            }            
        }

		if ( device->deviceStatus == DeviceStatus::Deleteable || env->environsState < environs::Status::Starting ) {
			CVerbID ( "HandleIncomingRequest: Device is deleteable or Environs is not started." );
			return;
		}
        
        if ( ___sync_val_compare_and_swap ( &device->stunState, ENVIRONS_THREAD_NO_THREAD, ENVIRONS_THREAD_DETACHEABLE ) != ENVIRONS_THREAD_NO_THREAD ) {
            CVerbID ( "HandleIncomingRequest: Stun already ongoing." );

#ifdef ENABLE_DEVICEBASE_WP_STUN
			request = device->stun.lock ();
#else
			LockAcquireVA ( device->spLock, "HandleIncomingRequest" );

            request = device->stun;

			LockReleaseVA ( device->spLock, "HandleIncomingRequest" );
#endif

            if ( request )
            {
                CVerbID ( "HandleIncomingRequest: A Stun is in progress." );
                
                request->Porti	= Porti;
                request->IPi	= IPi;
                
                if ( Porte ) {
                    CVerbID ( "HandleIncomingRequest: Updating port/IPe." );
                    
                    request->Porte	= Porte;
                    request->IPe	= IPe;
                    
                    device->udpAddr.sin_addr.s_addr	= IPe;
                    device->udpAddr.sin_port		= htons ( Porte );
                }
            }
            return;
        }
        
        if ( !device->AllocateUdpSocket ( ) )
            goto Finish;
        
        CSocketTraceUpdate ( device->udpSocket, "AllocateUdpSocket in StunRequest.HandleIncomingRequest" );        
        
        if ( (device->activityStatus & DEVICE_ACTIVITY_UDP_CONNECTED) != 0 ) {
            CVerbID ( "HandleIncomingRequest: Ignoring STUN request since a udp connection has already been established." );
            goto Finish;
        }

		request = std::make_shared < StunRequest > ( deviceID );
        if ( !request || !request->Init () ) {
            CErrID ( "HandleIncomingRequest: Failed to create new StunRequest object!" );
            goto Finish;
        }
        
        request->env        = env;
        request->IPe        = IPe;
        request->IPi        = IPi;
        request->Porti      = Porti;
        request->Porte      = Porte;
        request->deviceSP   = deviceSP;
        
        request->myself         = request;
        
        request->threadState    = ENVIRONS_THREAD_DETACHEABLE;
        
        CVerbID ( "HandleIncomingRequest: Creating thread" );

		if ( !request->thread.Run ( pthread_make_routine ( &StunRequest::EstablishStarter ), request.get (), "StunRequest.HandleIncomingRequest" ) )
		{
			CErrID ( "HandleIncomingRequest: Failed to create thread!" );
            
			request->myself         = 0;
			goto Finish;
		}

		LockAcquireVA ( device->spLock, "HandleIncomingRequest" );
        
        device->stun = request;

		LockReleaseVA ( device->spLock, "HandleIncomingRequest" );
        return;
        
    Finish:
        device->stunState = ENVIRONS_THREAD_NO_THREAD;
        return;
    }
    
    
    void * StunRequest::EstablishStarter ( void * arg )
    {
#ifndef NDEBUG
        try {
#endif
        StunRequest * request = (StunRequest *) arg;
        if ( !request )
            return 0;

        sp ( StunRequest )      requestSP   = request->myself;
        if ( !requestSP )
            return 0;
        
        sp ( DeviceController ) deviceSP    = request->deviceSP;

#ifdef DEBUGVERB
        int                     deviceID	= request->deviceID;
#endif
        CVerbID ( "EstablishStarter started ..." );
        
		request->Establisher ( );

		request->thread.Detach ( "StunRequest.EstablishStarter" );

#ifndef ENABLE_DEVICEBASE_WP_STUN
        request->ReleaseDeviceRequest ();
#endif
        request->threadState = ENVIRONS_THREAD_NO_THREAD;        
        
        request->thread.Lock ( "StunRequest.EstablishStarter" );
        
        request->myself = 0;
        
            request->thread.Unlock ( "StunRequest.EstablishStarter" );

#ifndef NDEBUG
        }
        catch ( char * )
        {
            printf ( "StunRequest::EstablishStarter: Exception !!!\n" );
            _EnvDebugBreak ( "StunRequest::EstablishStarter" );
        }
#endif
        
        return 0;
    }
    
    
    void StunRequest::Establisher ( )
    {
#ifndef NDEBUG
        try {
#endif
        CVerbID ( "Establisher started ..." );
        
        pthread_setname_current_envthread ( "StunRequest.Establisher" );
        
        sp ( MediatorClient ) mediator = env->mediator MED_WP;
        if ( !mediator ) {
            CErrID ( "Establisher: We need to establish a STUN socket, but no mediator is available!" );
            disposing = true;
            return;
        }
        
        mediator->RegisterAtMediators ( true );
        
            Establish ( );

#ifndef NDEBUG
        }
        catch ( char * )
        {
            printf ( "StunRequest::Establisher: Exception !!!\n" );
            _EnvDebugBreak ( "StunRequest::Establisher" );
        }
#endif
    }
    
    
    bool StunRequest::Establish ( )
    {
        CVerbID ( "Establish" );
        
        bool			success		= false;
        
        DeviceBase	*	device		= (DeviceBase *) deviceSP.get ();
		if ( !device ) {
			CErrID ( "Establish: Invalid device!" );
			return false;
		}
        CLogsArgID ( 3, "Establish: Trying UDP STUN connect to socket [ %d ] on IP [ %s ] port [ %d / %d ]", device->udpSocket, inet_ntoa ( *((struct in_addr *) &IPe) ), Porte, Porti );
        
        sp ( MediatorClient ) mediator = env->mediator MED_WP;
		if ( !mediator )
			return false;

        Zero ( device->udpAddr );
        
        device->udpAddr.sin_family		= PF_INET;
        device->udpAddr.sin_addr.s_addr	= IPe;
        
        if ( Porte )
            device->udpAddr.sin_port = htons ( Porte );
        
        int max = native.stunMaxTry >> 1;

        while ( max > 0 && !disposing )
        {
            if ( device->deviceStatus == DeviceStatus::Deleteable || env->environsState < environs::Status::Starting ) {
                CVerbID ( "Establish: Canceling STUN request due to DeviceController being disposed." );
                goto Finish;
            }
            
            if ( isRequestor )
            {
                CVerbID ( "Establish: (Requestor) STUN request to mediator!" );
                if ( Porte ) {
                    CVerbArgID ( "Establish: (Requestor) received port [%d], try connecting to destination device.", Porte );                    
                    
                    device->udpAddr.sin_addr.s_addr	= IPi;
                    device->udpAddr.sin_port		= htons ( Porti );
                    
                    if ( device->UdpSendHelo ( ) )
                        device->UdpSendHelo ( );
                    
                    device->udpAddr.sin_addr.s_addr	= IPe;
                    device->udpAddr.sin_port		= htons ( Porte );
                    
                    if ( IsValidFD ( device->udpSocket ) ) {
                        if ( !device->udpCoreConnected ) {
                            if ( device->UdpSendHelo () )
                                device->UdpSendHelo ();
                        }
                    }
                }
                else {
                    if ( !mediator->RequestSTUN ( deviceID, device->deviceAreaName, device->deviceAppName, device->udpSocket ) ) {
                        CErrID ( "Establish: (Requestor) STUN request to mediator failed!" );
                        goto Finish;
                    }
                }
            }
            else {
                CVerbID ( "Establish: (Responder) STUN request to mediator!" );
                if ( device->udpCoreConnected )
                    break;
                
                if ( IsValidFD ( device->udpSocket ) ) {
                    if ( !mediator->RequestSTUN ( deviceID, device->deviceAreaName, device->deviceAppName, device->udpSocket ) ) {
                        CErrID ( "Establish: (Responder) STUN request to mediator failed!" );
                        goto Finish;
                    }
                    
                    if ( !device->udpCoreConnected ) {
                        if ( device->UdpSendHelo () )
                            device->UdpSendHelo ();
                    }
                    
                    device->udpAddr.sin_addr.s_addr	= IPi;
                    device->udpAddr.sin_port			= htons ( Porti );
                    
                    if ( device->UdpSendHelo ( ) )
                        device->UdpSendHelo ( );
                }
            }
            
            if ( device->deviceStatus == DeviceStatus::Deleteable || IsInvalidFD ( device->udpSocket ) || env->environsState < environs::Status_Starting ) {
                CVerbID ( "Establish: Canceling STUN request due to DeviceController being disposed." );
                goto Finish;
            }
            max--;
            Sleep ( 400 );
            CVerbArgID ( "Establish: next try... [%i].", max );
            
            if ( !doHandshake )
                break;
        }
        
        if ( (Porte && Porti) && !doHandshake && !disposing ) {
            CInfoID ( "Establish: STUN and UDP-handshake with device succeeded" );
            
            device->OnUdpConnectionEstablished ( );
            success = true;
        }
        else {
            CVerbID ( "Establish: Yet, STUN or UDP-handshake with device failed." );
        }
        
    Finish:
        return success;
    }
}









/**
 * Base functionality common for all devices
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
//#   define DEBUGVERBLocks
#endif

#include "Device.Controller.h"
#include "Portal/Portal.Stream.h"
#include "Portal/Portal.Device.h"
#include "Interop/Stat.h"
#include "Environs.Utils.h"
#include "Environs.Crypt.h"
#include "Environs.Obj.h"
#include "Tracer.h"
#include <errno.h>

#ifndef _WIN32
#	include <fcntl.h>
#endif

#ifndef WINDOWS_PHONE
#	include <stdlib.h>
#endif

using namespace environs;
using namespace environs::lib;
using namespace environs::API;


// The TAG for prefixing log messages
#define CLASS_NAME	"Device.Base. . . . . . ."

//#define DEVICEBASE_COMPATIBLE_CHANNEL

/* Namespace: environs -> */
namespace environs
{
    unsigned int DeviceBase::packetSize = UDP_DEVICEBASE_MAX_SIZE - 48;

    bool IsFINMessage ( char * msg )
    {
        if ( msg [ 0 ] == 'F' && msg [ 1 ] == 'I' && msg [ 2 ] == 'N' && msg [ 3 ] == ';' )
        {
            return true;
        }
        return false;
    }
    
    
	DeviceBase::DeviceBase ()
	{
		CVerb ( "Construct..." );
		
		TraceDeviceBaseAdd ( this );

        dbgAlive					= 2;
        connectToken                = rand ();
		accessLocks					= 1;
		allocated                   = false;
		env							= 0;
		activityStatus				= 0;
		hasPhysicalContact			= 0;

		comPort						= 0;
		dataPort					= 0;

		nativeID                    = 0;
		deviceID					= 0;
		deviceAppName				= 0;
		deviceAreaName				= 0;
		connectStatus               = 1;
        connectTime                 = 0;

		lastSensorFrameSeqNr		= -1;
		lastPortalUpdate            = 0;
		portalInfoOff.centerX		= 100;
		portalInfoOff.centerY		= 100;
		portalInfoOff.orientation	= 90;
		portalInfoOff.width			= 1024;
		portalInfoOff.height		= 768;
		
		ZeroStruct ( streamOptions, PortalStreamOptions );

		display.width               = 1024;
		display.height              = 768;
		display.width_mm            = 150;
		display.height_mm           = 230;
		display.orientation			= DISPLAY_ORIENTATION_PORTRAIT;

		width_coverage				= 250;
		height_coverage				= 800;

		packetSequence				= 0;

        udpBuffer                   = 0;
		interactSocket              = INVALID_FD;
		interactSocketForClose      = INVALID_FD;
        Zero ( interactAddr );

		udpSocket					= INVALID_FD;
		udpSocketForClose			= INVALID_FD;
		Zero ( udpAddr );

		comDatSocket                = INVALID_FD;
		comDatSocketForClose        = INVALID_FD;
        Zero ( comDatAddr );

		stuntRedundant              = false;
		behindNAT					= false;
        
        stuntInteractState			= ENVIRONS_THREAD_NO_THREAD;
        stuntComDatState			= ENVIRONS_THREAD_NO_THREAD;
        stunState                   = ENVIRONS_THREAD_NO_THREAD;
		connectThreadState			= ENVIRONS_THREAD_NO_THREAD;
        
#ifdef ENABLE_DEVICEBASE_SINGLE_COMDAT_THREAD
        comDatByteBuffer            = 0;
        comDatListen                = false;
        comDatBuffer                = 0;
        comDatBufferSize            = 0;
        
        comDat_Start                = 0;
        comDat_BufferEnd            = 0;
        comDat_CurrentEnd           = 0;

#ifdef ENABLE_DEVICEBASE_WINSOCK_SINGLE_COMDAT_THREAD
		comDatEvent					= WSA_INVALID_EVENT;
#endif
#endif
        
        interactThreadState         = 0;
        interactBuffer              = 0;
        interactBufferSize          = 2 * TCP_RECV_CONNECT_BUFFER_SIZE_BEGIN;

		dataStorePath				= 0;
		dataStorePathForRequests	= 0;
		dataStorePathLen			= 0;
		dataStorePathRemainingLen	= 0;

		udpCoreConnected            = false;
        disposed                    = false;        
        
		portalGeneratorsLastAssigned	= 0;
		portalGeneratorsCount		= 0;
		Zero ( portalGenerators );

		memset ( &portalGeneratorsDevice, 0xFF, sizeof ( portalGeneratorsDevice ) );

		portalGeneratorsDeviceInput = -1;

		portalReceiversLastAssigned	= 0;
		portalReceiversCount		= 0;
		Zero ( portalReceivers );

		memset ( &portalReceiversDevice, 0xFF, sizeof ( portalReceiversDevice ) );

		encrypt                     = 0;
		Zero ( aes );
		aesBlob                     = 0;

        sensorSender                = 0;

		deviceStatus				= DeviceStatus::Created;

		msgHandlers [ MSG_TYPE_HELO ]       = &DeviceBase::HandleHeloMessage;
		msgHandlers [ MSG_TYPE_IMAGE ]      = &DeviceBase::HandleStreamMessage;
		msgHandlers [ MSG_TYPE_STREAM ]     = &DeviceBase::HandleStreamMessage;
		msgHandlers [ MSG_TYPE_FILE ]       = &DeviceBase::HandleFileTransfer;
		msgHandlers [ MSG_TYPE_MESSAGE ]    = &DeviceBase::HandleStringMessage;
		msgHandlers [ MSG_TYPE_PORTAL ]     = &DeviceBase::HandlePortalMessage;
		msgHandlers [ MSG_TYPE_OPTIONS ]    = &DeviceBase::HandleOptionsMessage;
		msgHandlers [ MSG_TYPE_OPTIONS_RESPONSE ] = &DeviceBase::HandleOptionsResponse;
        msgHandlers [ MSG_TYPE_ENVIRONS ]   = &DeviceBase::HandleNullMessage;
        msgHandlers [ MSG_TYPE_SENSOR ]     = &DeviceBase::HandleSensorData;

		lastFile                    = 0;
		lastFileID                  = 0;
		lastFilePart                = 0;

		portalHandlers [ MSG_PORTAL_REQUEST_ID ]        = &DeviceBase::HandleNullPortal;
		portalHandlers [ MSG_PORTAL_ASK_FOR_REQUEST_ID ] = &DeviceBase::ProccessPortalAskForRequest;
		portalHandlers [ MSG_PORTAL_PROVIDE_STREAM_ID ] = &DeviceBase::ProccessPortalProvidedStream;
		portalHandlers [ MSG_PORTAL_PROVIDE_IMAGES_ID ] = &DeviceBase::ProccessPortalProvidedImages;
		portalHandlers [ MSG_PORTAL_REQUEST_FAIL_ID ]   = &DeviceBase::ProccessPortalRequestFailed;
		portalHandlers [ MSG_PORTAL_STOP_ID ]			= &DeviceBase::ProccessPortalStop;
		portalHandlers [ MSG_PORTAL_STOP_ACK_ID ]       = &DeviceBase::ProccessPortalStopAckBase;
		portalHandlers [ MSG_PORTAL_STOP_FAIL_ID ]      = &DeviceBase::HandleNullPortal;
		portalHandlers [ MSG_PORTAL_START_ID ]          = &DeviceBase::ProccessPortalStart;
		portalHandlers [ MSG_PORTAL_START_ACK_ID ]      = &DeviceBase::ProccessPortalStartAckBase;
		portalHandlers [ MSG_PORTAL_START_FAIL_ID ]     = &DeviceBase::HandleNullPortal;
		portalHandlers [ MSG_PORTAL_PAUSE_ID ]          = &DeviceBase::ProccessPortalPause;
		portalHandlers [ MSG_PORTAL_PAUSE_ACK_ID ]      = &DeviceBase::ProccessPortalPauseAckBase;
		portalHandlers [ MSG_PORTAL_PAUSE_FAIL_ID ]     = &DeviceBase::HandleNullPortal;

#ifdef ENABLE_PORTAL_STALL_MECHS
		portalHandlers [ MSG_PORTAL_BUFFER_FULL_ID ]    = &DeviceBase::ProccessPortalStall;
		portalHandlers [ MSG_PORTAL_BUFFER_AVAIL_AGAIN_ID ] = &DeviceBase::ProccessPortalUnStall;
#else
		portalHandlers [ MSG_PORTAL_BUFFER_FULL_ID ]    = &DeviceBase::HandleNullPortal;
		portalHandlers [ MSG_PORTAL_BUFFER_AVAIL_AGAIN_ID ] = &DeviceBase::HandleNullPortal;
#endif

		portalHandlers [ MSG_PORTAL_IFRAME_REQUEST_ID ] = &DeviceBase::ProccessPortaliFrameRequest;
	}


	DeviceBase::~DeviceBase ()
	{	
		CVerbsID ( 2, "Destruct..." );

        Dispose ();
        
        if ( IsValidFD ( interactSocketForClose ) ) {
            ShutdownCloseSocket ( interactSocketForClose, true, "DeviceBase Destruct interactSocketForClose" );
		}
        
        if ( IsValidFD ( comDatSocketForClose ) ) {
            ShutdownCloseSocket ( comDatSocketForClose, true, "DeviceBase Destruct comDatSocketForClose" );
		}
        
        if ( IsValidFD ( udpSocketForClose ) ) {
            ShutdownCloseSocket ( udpSocketForClose, true, "DeviceBase Destruct udpSocketForClose" );
		}
        
#ifdef ENABLE_DEVICEBASE_SINGLE_COMDAT_THREAD
        if ( comDatByteBuffer ) {
            disposeBuffer ( comDatByteBuffer );
            comDatByteBuffer = 0;
        }

#ifdef ENABLE_DEVICEBASE_WINSOCK_SINGLE_COMDAT_THREAD
		CloseWSAHandle_n ( comDatEvent );
#endif
#endif        
        dataStorePathLen            = 0;
        dataStorePathRemainingLen   = 0;
        
        free_m ( dataStorePath );
        
        free_m ( dataStorePathForRequests );
        
        free_m ( aesBlob );
        
        free_m ( interactBuffer );

        if ( lastFile ) {
            fclose ( lastFile );
            lastFile = 0;
        }
        
        if ( aes.encCtx CRYPT_AES_LOCK_EXP ( || aes.lockAllocated ) )
            AESDisposeKeyContext ( &aes );

		ReleaseCert ( deviceID );

        if ( allocated ) {
            allocated = false;
            
			LockDisposeA ( interactSocketLock );
			LockDisposeA ( comDatSocketLock );
            
			CVerbID ( "Destruct: Releasing mutex for access to the portal!" );

			LockDisposeA ( portalMutex );

			CVerbID ( "Destruct: Releasing mutex for access to the portal receivers!" );

			LockDisposeA ( portalReceiversMutex );

			CVerbID ( "Destruct: Releasing mutex for accessing object state!" );

			LockDispose ( &spLock );
		}
        
        free_n ( udpBuffer );
        
        if ( deviceAppName != env->appName )
            free_m ( deviceAppName );
        
        if ( deviceAreaName != env->areaName )
            free_m ( deviceAreaName );

		env		= 0;
        
        CVerbID ( "Destructed." );
        
		TraceDeviceBaseRemove ( this );
        
        native.gcThread.Notify ( "DeviceBase.Destruct" );
	}


	bool DeviceBase::Init ( const sp ( Instance ) &envObj, const char * areaName, const char * appName )
	{
        CVerbID ( "Init" );
        
        connectTime = GetEnvironsTickCount32 ();

        envSP   = envObj;
		if ( !envSP )
			return false;

		env     = envSP.get ();

		streamOptions.useStream				= env->useStream;
		streamOptions.usePNG				= env->usePNG;
		streamOptions.useOpenCL				= env->useOCL;
		streamOptions.streamOverCom			= env->streamOverCom;
		streamOptions.useNativeResolution	= env->useNativeResolution;
		streamOptions.limitMaxResolution	= true;
		streamOptions.streamMinFPS          = ( float ) env->streamFPS;

		portalhWnd							= env->appWindowHandle;

		if ( !allocated )
		{
			if ( !LockInitA ( interactSocketLock ) )
				return false;

			if ( !LockInitA ( comDatSocketLock ) )
				return false;

			if ( !LockInitA ( spLock ) )
				return false;

			if ( !LockInitA ( portalMutex ) )
				return false;

			if ( !LockInitA ( portalReceiversMutex ) )
                return false;

			if ( !interactThread.Init () )
				return false;

			if ( !comDatThread.Init () )
				return false;

			if ( !udpThread.Init () )
				return false;

			if ( !connectThread.Init () )
				return false;

			allocated = true;
        }
        
        sp ( MediatorClient ) mediator = env->mediator MED_WP;
        if ( !mediator ) {
            CErrsID ( 2, "Init: Mediator layer unavailable." );
            return false;
        }
        
		deviceNode = mediator->GetDeviceSP ( deviceID, areaName, appName );
        if ( !deviceNode ) {
            CErrsID ( 2, "Init: Device not found at mediator layer." );
            return false;
        }
        
        if ( !deviceNode->allowConnect ) {
            CErrsID ( 2, "Init: Contact with this device is disabled." );
            return false;
        }
        
        if ( !areaName || !*areaName )
            areaName = env->areaName;
        
        if ( !appName || !*appName )
            appName = env->appName;
        
        
        if ( strncmp ( areaName, env->areaName, sizeof ( env->areaName ) - 1 ) || strncmp ( appName, env->appName, sizeof ( env->appName ) - 1 ) )
        {
            CreateCopyString ( areaName, &deviceAreaName );
            
            CreateCopyString ( appName, &deviceAppName );
        }
		return true;
	}


	bool DeviceBase::HandleNullMessage ( ComMessageHeader * header, bool isComDatChannel )
	{
		return true;
	}


	void DeviceBase::HandleNullPortal ( int PortalID )
	{
	}

    
#ifdef ENABLE_DEVICEBASE_WP_STUNT
	void DeviceBase::CloseStunt ( const wp ( StunTRequest ) &request, bool disposeDevice, LONGSYNC * state, const char * channel, bool wait )
	{
		CVerbArgID ( "CloseStunt: %s channel", channel );

		sp ( StunTRequest ) stuntRequest = request.lock ();
		if ( !stuntRequest )
			return;

		StunTRequest	* stunt = stuntRequest.get ();

		if ( disposeDevice )
			stunt->disposeDevice = true;

		if ( ___sync_val_compare_and_swap ( state, ENVIRONS_THREAD_DETACHEABLE, ENVIRONS_THREAD_NO_THREAD ) != ENVIRONS_THREAD_DETACHEABLE )
			return;

		if ( stunt ) {
			LockAcquireVA ( spLock, "CloseStunt" );

			stunt->CloseThreads ( false );

			LockReleaseVA ( spLock, "CloseStunt" );

			if ( stunt->thread.isRunning () ) //&& !stunt->thread.areWeTheThread () )
			{
				if ( wait )
					stunt->detach = false;

				CVerbArgID ( "CloseStunt: Waiting for STUNT %s channel Responder (Mediator.. run.. HandleRequest.. Device::StuntHandleRequest) to be terminated...", channel );
				//CLogArgID ( "CloseStunt: Waiting for STUNT %s channel Responder ...", channel );

				if ( wait )
					stuntRequest->thread.Join ( "STUNT" );
				return;
			}
		}
	}
#else
    void DeviceBase::CloseStunt ( const sp ( StunTRequest ) &request, bool disposeDevice, LONGSYNC * state, const char * channel, bool wait )
    {
        CVerbArgID ( "CloseStunt: %s channel", channel );

		LockAcquireVA ( spLock, "CloseStunt" );

        sp ( StunTRequest ) stuntRequest = request;

		LockReleaseVA ( spLock, "CloseStunt" );

        if ( !stuntRequest )
            return;
        
		StunTRequest	* stunt = stuntRequest.get ();

        if ( disposeDevice )
            stunt->disposeDevice = true;
        
        if ( ___sync_val_compare_and_swap ( state, ENVIRONS_THREAD_DETACHEABLE, ENVIRONS_THREAD_NO_THREAD ) != ENVIRONS_THREAD_DETACHEABLE )
            return;
        
        if ( stunt ) {
            LockAcquireVA ( spLock, "CloseStunt" );
            
            stunt->CloseThreads ( false );
            
            LockReleaseVA ( spLock, "CloseStunt" );
            
            if ( stunt->thread.isRunning () ) //&& !stunt->thread.areWeTheThread () )
            {                
                if ( wait )
					stunt->detach = false;
                
                CVerbArgID ( "CloseStunt: Waiting for STUNT %s channel Responder (Mediator.. run.. HandleRequest.. Device::StuntHandleRequest) to be terminated...", channel );
                //CLogArgID ( "CloseStunt: Waiting for STUNT %s channel Responder ...", channel );
                
				if ( wait )
					stuntRequest->thread.Join ( "STUNT" );
                return;
            }
        }
    }
#endif
    
    void DeviceBase::SendTcpFin ( int sock )
    {
        CVerb ( "SendTcpFin" );
        
        ComMessageHeader msg;
        Zero ( msg );
        
        msg.preamble [ 0 ] = 'F';
        msg.preamble [ 1 ] = 'I';
        msg.preamble [ 2 ] = 'N';
        msg.preamble [ 3 ] = ';';
        
        msg.version = TCP_MSG_PROTOCOL_VERSION;
        msg.length  = sizeof ( msg );
        msg.type    = MSG_TYPE_HELO;
        
        msg.MessageType.payloadType = MSG_HANDSHAKE_PING;
        
        if ( !SetNonBlockSocket ( sock, true, "SendTcpFin" ) )
			return;

		int toSendLen	= sizeof ( msg );
		char * cipher	= 0;
		char * toSend	= 0;
        
		if ( encrypt ) {
			if ( !AESEncrypt ( &aes, ( char * ) &msg, ( unsigned int* ) &toSendLen, &cipher ) )
				return;
			toSend = cipher;
		}
		else
			toSend = ( char * ) &msg;

		CVerbArgID ( "SendTcpFin: Sending [ %d ] bytes", toSend );
        
        send ( sock, ( const char * ) toSend, toSendLen, MSG_NOSIGNAL );
        
		free_n ( cipher );

        CVerb ( "SendTcpFin: done." );
    }


	void CloseListener ( int deviceID, DeviceBase * device, int &sockID, int &sockForClose, ThreadSync * sync, const char * name, pthread_mutex_t * lock, bool wait )
	{
		CVerbVerbID ( "CloseListener" );

		int sock;

		if ( !wait ) {
			sock = sockID;

			if ( IsValidFD ( sock ) )
			{
				sockID = INVALID_FD;

                device->SendTcpFin ( sock );
                
				ShutdownCloseSocket ( sock, false, "DeviceBase CloseListener 1 (nowait)" );
			}
			return;
		}

		bool var42 = false;

		// There's a (even little) chance for a race condition if the handler thread has acquired the socket fd
		// and is paused before going into the syscall
		// We wait for 1s to mitigate that, then close the socket and risk that the thread handler goes into
		// the syscall with a file descriptor that has meanwhile been assigned to something else ...

		if ( sync->isRunning () )
		{
			sock = sockID;
			if ( IsValidFD ( sock ) )
			{
                sockID = INVALID_FD;
                
                device->SendTcpFin ( sock );

				ShutdownCloseSocket ( sock, false, "DeviceBase CloseListener sock (wait/isRunning)" );
			}

			sync->WaitOne ( name, 200 );
		}
		else sockID = INVALID_FD;

		if ( lock && pthread_mutex_trylock ( lock ) == 0 ) {
			// No sender has acquired the lock
			var42 = true;
		}

		sock = sockID;
		if ( IsValidFD ( sock ) )
		{
			if ( sock == sockForClose )
				sockID = INVALID_FD;
			else if ( IsInvalidFD ( sockForClose ) ) {
				sockID = INVALID_FD; sockForClose = sock;
				CSocketTraceUpdate ( sock, "DeviceBase CloseListener invalidate sockID d.t. invalid sockForClose" );
			}
			else {
				ShutdownCloseSocket ( sock, true, "DeviceBase CloseListener 2 (wait/close)" );
			}
		}

		sock = sockForClose;
		if ( IsValidFD ( sock ) )
        {
            device->SendTcpFin ( sock );
            
			ShutdownCloseSocket ( sock, false, "DeviceBase CloseListener sockForClose (wait)" );
			shutdown ( sock, 2 );
		}

		if ( var42 ) {
			pthread_mutex_unlock ( lock );
		}

		// Let's wait max. 10 seconds
		// If the thread has not terminated, then close the socket and take the risk of using a closed socket
		if ( sync->isRunning () )
		{
			CSocketTraceUpdate ( sock, "DeviceBase CloseListener 3 (wait/isRunning)" );

			if ( native.isAppShutdown )
				sync->WaitOne ( name, 500 );
			else
				sync->WaitOne ( name, 8000 );
		}

		if ( sync->isRunning () )
		{
			CSocketTraceUpdate ( sock, "DeviceBase CloseListener 4 (wait/isRunning)" );

			sock = sockForClose;

			sockForClose = INVALID_FD;

			if ( IsValidFD ( sock ) ) {
				device->SendTcpFin ( sock );

				CSocketTraceRemove ( sock, "CloseListener: Closing", 0 );
				closesocket ( sock );
			}
		}

		sync->Join ( name );
	}


	void DeviceBase::CloseListeners ( bool wait )
	{
		CVerbID ( "CloseListeners" );

		if ( ( activityStatus & DEVICE_ACTIVITY_LISTENER_CLOSED ) == DEVICE_ACTIVITY_LISTENER_CLOSED )
			return;

        if ( wait )
        {
            activityStatus |= DEVICE_ACTIVITY_LISTENER_CLOSED;
            
            CloseConnectorThread ();
            
            CloseStunt ( stuntInteract, false, &stuntInteractState, "iact", wait );
            
            CloseStunt ( stuntComDat, false, &stuntComDatState, "comdat", wait );

#ifdef ENABLE_DEVICEBASE_WP_STUN
			sp ( StunRequest ) req = stun.lock ();
#else
			LockAcquireVA ( spLock, "CloseListeners" );

            sp ( StunRequest ) req = stun;
            
            stun.reset ();

			LockReleaseVA ( spLock, "CloseListeners" );
#endif

            if ( req ) {
				//req->ReleaseDeviceRequest ();
				req->CloseThreads ();
				req.reset ();
            }
        }
        
		CloseListener ( deviceID, this, interactSocket, interactSocketForClose, &interactThread, "InteractListener", &interactSocketLock, wait );
        
#ifdef ENABLE_DEVICEBASE_SINGLE_COMDAT_THREAD
		bool waitBefore = wait;

		wait = false;
#endif
		CloseListener ( deviceID, this, comDatSocket, comDatSocketForClose, &comDatThread, "ComDatListener", &comDatSocketLock, wait );

#ifdef ENABLE_DEVICEBASE_SINGLE_COMDAT_THREAD
		//comDatThread.ResetSync ( "CloseComDatListener" );

		ComDatSignal ();
		/*if ( wait && comDatListen ) {
		comDatThread.WaitOne ( "CloseComDatListener", 1000 );
		}*/

		wait = waitBefore;
#endif        
        CloseUdpListener ( wait );
	}
    
    
    void DeviceBase::DisposePlatform ()
    {
        CVerbID ( "DisposePlatform" );
    }
    
    
    void DeviceBase::PreDispose ()
    {
        CVerbID ( "PreDispose" );
        
        deviceStatus	= DeviceStatus::Deleteable;

		if ( allocated )
		{
			connectThread.Notify ( "PreDispose", false );

			CloseListeners ( false );
		}

		activityStatus	|= DEVICE_ACTIVITY_PLATFORM_PREDISPOSED;
    }
    
    
	bool DeviceBase::IsPreDisposed ()
    {
        CVerbID ( "IsPreDisposed" );

		return ( ( activityStatus	& DEVICE_ACTIVITY_PLATFORM_PREDISPOSED ) != 0 );
    }
	
    
	void DeviceBase::Dispose ()
	{
		CVerbID ( "Dispose" );

		deviceStatus = DeviceStatus::Deleteable;
        
        if ( disposed || !allocated )
            return;
		disposed = true;

		if ( ( activityStatus & DEVICE_ACTIVITY_PLATFORM_DISPOSED ) != DEVICE_ACTIVITY_PLATFORM_DISPOSED )
			DisposePlatform ();
        
        connectThread.Notify    ( "Dispose", false );
        
        sp ( MediatorClient ) mediator = env->mediator MED_WP;
        if ( mediator ) {
			mediator->UpdateDeviceState ( this, 0 );

			mediator->RemoveStuntDevice ( this );
            mediator.reset ();
        }
        
		CloseListeners ();

        /*if ( sensorSender ) {
            DisposeSensorSender ( hInst, this );
        }*/
        
		StopAllPortalDevices ( this );

        // Temporary storage for objects to be deleted
        //
        void * objs [ MAX_PORTAL_STREAMS_A_DEVICE ];
                
		LockAcquireVA ( portalMutex, "Dispose" );

		for ( unsigned int i = 0; i < MAX_PORTAL_STREAMS_A_DEVICE; i++ )
		{
			if ( portalGenerators [ i ] )
			{
                objs [ i ] = (void *) portalGenerators [ i ];
                portalGenerators [ i ] = 0;
			}
            else
                objs [ i ]  = 0;
		}

		portalGeneratorsCount			= 0;
		portalGeneratorsLastAssigned	= 0;

		LockReleaseVA ( portalMutex, "Dispose" );
        
        for ( unsigned int i = 0; i < MAX_PORTAL_STREAMS_A_DEVICE; i++ )
        {
            if ( objs [ i ] )
            {
                PortalGenerator * gen = (PortalGenerator *) objs [ i ];

                objs [ i ] = 0;
                delete gen;
            }
        }        

		// Stop waiting portal receivers at first (they hold a reference to the device)
		CVerbLock ( "Dispose: portalReceiversMutex" );

		LockAcquireVA ( portalReceiversMutex, "Dispose" );

		for ( unsigned int i = 0; i < MAX_PORTAL_STREAMS_A_DEVICE; i++ )
        {
			if ( portalReceivers [ i ] )
            {
                objs [ i ] = (void *) portalReceivers [ i ];
                portalReceivers [ i ] = 0;
            }
            else
                objs [ i ]  = 0;
		}

		portalReceiversCount		= 0;
		portalReceiversLastAssigned	= 0;

		CVerbUnLock ( "Dispose: portalReceiversMutex" );

		LockReleaseVA ( portalReceiversMutex, "Dispose" );
        
        for ( unsigned int i = 0; i < MAX_PORTAL_STREAMS_A_DEVICE; i++ ) {
            if ( portalReceivers [ i ] ) {
                
                PortalReceiver * receiver = (PortalReceiver *) objs [ i ];
                objs [ i ] = 0;
                
                onEnvironsNotifier1 ( env, nativeID, NOTIFY_PORTAL_STREAM_STOPPED, receiver->portalID );
                
                delete receiver;
            }
        }
	}

    
    void SecurePathName ( char * path )
    {
        CVerb ( "SecurePathName" );
        
        // Replace ../ with ___
        do
        {
            char * found = strstr ( path, "../" );
            if ( !found )
                break;
            
            *found = '_'; found++;
            *found = '_'; found++;
            *found = '_';
        }
        while ( true );
        
        do
        {
            char * found = strstr ( path, "..\\" ); //dataStoreNew + lenRoot
            if ( !found )
                break;
            
            *found = '_'; found++;
            *found = '_'; found++;
            *found = '_';
        }
        while ( true );
    }
    

	bool DeviceBase::InitDeviceStorage ( Instance * env, int deviceID, const char * areaName, const char * appName, char *&dest,
                                        unsigned short &length, unsigned short &remainLength, char *&destRequests,
                                        bool assingNewBuffers, bool buildReqPath )
	{
		CVerbID ( "InitDeviceStorage" );

		size_t lenRoot = strlen ( native.dataStore );
#ifndef NDEBUG
		if ( lenRoot <= 0 )
			lenRoot = 0;
#endif
		size_t lenData = lenRoot + DATASTORE_PATH_APPEND_LEN;

		char * dataStoreNew = dest;

		char * dataStoreReqNew = 0;
        
        if ( assingNewBuffers )
        {
            dataStoreNew = ( char * ) malloc ( lenData + 2 );
            if ( !dataStoreNew ) {
                CErrArgID ( "InitDeviceStorage: Failed to allocate memory for dataStorePath of size [ %u ].", ( unsigned int ) lenData );
                return false;
            }
            
            if ( buildReqPath ) {
                dataStoreReqNew = ( char * ) malloc ( lenData + 2 );
                if ( !dataStoreReqNew ) {
                    CErrArgID ( "InitDeviceStorage: Failed to allocate memory for dataStorePathRequests of size [ %u ]", ( unsigned int ) lenData );
                    free ( dataStoreNew );
                    return false;
                }
            }
        }
        else {
            lenData = length;
        }

		/// build areaName
		int len = 0, copied = 0;

		do
        {
            if ( !appName || !*appName )
                appName = env->appName;

            if ( !areaName || !*areaName )
                areaName = env->areaName;

            // Check the whole path at first
            copied = snprintf ( dataStoreNew, lenData, "%s%s/%s/%i/", native.dataStore, areaName, appName, deviceID );
            if ( copied <= 0 ) {
                CErrID ( "InitDeviceStorage: Failed to build dataStorePath." ); break;
            }
            
			SecurePathName ( dataStoreNew + lenRoot );
            
            if ( !DirectoryPathExist ( dataStoreNew ) )
            {                
                // Create data store if neccessary
                CreateDataDirectory ( native.dataStore );
                
                copied = snprintf ( dataStoreNew, lenData, "%s%s/", native.dataStore, areaName );
                if ( copied <= 0 ) {
                    CErrID ( "InitDeviceStorage: Failed to build dataStorePath with area name." ); break;
                }
                len += copied;
                
				SecurePathName ( dataStoreNew + lenRoot );
                
                if ( !CreateDataDirectory ( dataStoreNew ) ) {
                    CErrID ( "InitDeviceStorage: Failed to create dataStore with area name." ); break;
                }
                
                copied = snprintf ( dataStoreNew + len, lenData - len, "%s/", appName );
                if ( copied <= 0 ) {
                    CErrID ( "InitDeviceStorage: Failed to build dataStorePath with app name." ); break;
                }
                
				SecurePathName ( dataStoreNew + len );
                
                len += copied;
                
                if ( !CreateDataDirectory ( dataStoreNew ) ) {
                    CErrID ( "InitDeviceStorage: Failed to create dataStore with app name." ); break;
                }
                
                copied = snprintf ( dataStoreNew + len, lenData - len, "%i/", deviceID );
                if ( copied <= 0 ) {
                    CErrID ( "InitDeviceStorage: Failed to build dataStorePath with app name." ); break;
                }
                len += copied;
                
                if ( !CreateDataDirectory ( dataStoreNew ) ) {
                    CErrID ( "InitDeviceStorage: Failed to create dataStore with deviceID." ); break;
                }
            }
            else
                len = copied;

			if ( assingNewBuffers && buildReqPath )
				memcpy ( dataStoreReqNew, dataStoreNew, len + 1 );

			length = ( unsigned short ) len;
			remainLength = ( unsigned short ) ( lenData - ( len + 2 ) );

            if ( assingNewBuffers )
            {
                // Create the data store for this deviceID pref_dataStore
                free_n ( dest );
                dest = dataStoreNew;
                
                if ( buildReqPath ) {
                    free_n ( destRequests );
                    destRequests = dataStoreReqNew;
                }
            }

			return true;
		}
		while ( 0 );
        
        if ( assingNewBuffers )
        {
            free_n ( dataStoreNew );
            
            free_n ( dataStoreReqNew );
        }
		
        return false;
	}


	const char * DeviceBase::GetFilePath ( int fileID )
	{
		if ( !dataStorePathForRequests )
			return 0;

		int len = snprintf ( dataStorePathForRequests + dataStorePathLen, dataStorePathRemainingLen, "%i.bin", fileID );
		if ( len <= 0 ) {
			CErrArgID ( "GetFilePath: Failed to build path for fileID [ %i ]! [ deviceID 0x%X ]", fileID, deviceID );
			return 0;
		}

		return dataStorePathForRequests;
	}


	char * DeviceBase::GetFilePath ( Instance * env, int deviceID, const char * areaName, const char * appName, unsigned short &pathLen )
	{
		char * dataStorePath                     = 0;
		char * dataStorePathForRequests          = 0;
		unsigned short dataStorePathRemainingLen = 0;

		if ( !InitDeviceStorage ( env, deviceID, areaName, appName, dataStorePath, pathLen, dataStorePathRemainingLen, dataStorePathForRequests, true, false ) )
			return 0;

		return dataStorePath;
	}


	bool DeviceBase::GetDirectContactStatus ()
	{
		return ( hasPhysicalContact > 0 );
	}


	/**
	* Get the status, whether the device (id) has established an active portal
	*
	* @param	portalType  0 = Any type, or PORTAL_DIR_INCOMING, PORTAL_DIR_OUTGOING
	* @return	success 	true = yes, false = no
	*/
	bool DeviceBase::GetPortalEnabled ( int portalType )
	{
		CVerbID ( "GetPortalEnabled" );

		if ( portalType == PORTAL_DIR_INCOMING )
			return ( portalReceiversCount > 0 );

		if ( portalType == PORTAL_DIR_OUTGOING )
			return ( portalGeneratorsCount > 0 );

		return ( portalGeneratorsCount || portalReceiversCount );
	}


	/**
    * Get the portalID of the first active portal that matches the given portalType.
	*
	* @param	portalType  0 = Any type, or PORTAL_DIR_INCOMING, PORTAL_DIR_OUTGOING
	* @return	portalID 	The portal ID.
	*/
	int DeviceBase::GetPortalID ( int portalType )
	{
		int portalID = -1;
		CVerbID ( "GetPortalID" );

		if ( portalType == PORTAL_DIR_INCOMING )
		{
			CVerbLock ( "GetPortalID: portalReceiversMutex" );

			if ( !LockAcquireA ( portalReceiversMutex, "GetPortalID" ) )
				return -1;

			for ( int i = 0; i < MAX_PORTAL_STREAMS_A_DEVICE; i++ ) {
				if ( portalReceivers [ i ] ) {
					portalID = portalReceivers [ i ]->portalInfo.portalID;
					break;
				}
			}

			CVerbUnLock ( "GetPortalID: portalReceiversMutex" );

			LockReleaseVA ( portalReceiversMutex, "GetPortalID" );
		}
		else if ( portalType == PORTAL_DIR_OUTGOING )
		{
			CVerbLock ( "GetPortalID: portalMutex" );

			if ( !LockAcquireA ( portalMutex, "GetPortalID" ) )
				return -1;

			for ( int i = 0; i < MAX_PORTAL_STREAMS_A_DEVICE; i++ ) {
				if ( portalGenerators [ i ] ) {
					portalID = portalGenerators [ i ]->portalID;
					break;
				}
			}

			CVerbUnLock ( "GetPortalID: portalMutex" );

			LockReleaseVA ( portalMutex, "GetPortalID" );
		}

		CVerbArgID ( "GetPortalID: [%s] [0x%X]", ( portalType == PORTAL_DIR_INCOMING ? "Incoming" : "Outgoing" ), portalID );

		return portalID;
	}


	int LookForFreeSlot ( unsigned char &start, void ** slots )
	{
		if ( start >= MAX_PORTAL_STREAMS_A_DEVICE )
			start = 0;

		for ( unsigned int i = start; i < MAX_PORTAL_STREAMS_A_DEVICE; i++ ) {
			if ( !slots [ i ] ) {
				start++;
				return i;
			}
		}
		for ( unsigned int i = 0; i < start; i++ ) {
			if ( !slots [ i ] ) {
				start++;
				return i;
			}
		}
		return -1;
	}


	/**
	* Find a free portalID slot for the direction encoded into the given portalDetails.
	*
	* @param	portalDetails	Required PORTAL_DIR_INCOMING or PORTAL_DIR_OUTGOING
	*
	* @return	portalID 		The portal ID with the free id slot encoded in bits 0xFF.
	*/
	int DeviceBase::GetPortalIDFreeSlot ( int portalDetails )
	{
		int portalID = -1;

		CVerbID ( "GetPortalIDFreeSlot" );

		if ( IsPortalReceiver () )
		{
			CVerbLock ( "GetPortalIDFreeSlot: portalReceiversMutex" );

			if ( !LockAcquireA ( portalReceiversMutex, "GetPortalIDFreeSlot" ) )
				return -1;

			portalID = LookForFreeSlot ( portalReceiversLastAssigned, ( void ** ) portalReceivers );

			CVerbUnLock ( "GetPortalIDFreeSlot: portalReceiversMutex" );

			LockReleaseVA ( portalReceiversMutex, "GetPortalIDFreeSlot" );
		}
		else
		{
			CVerbLock ( "GetPortalIDFreeSlot: portalMutex" );

			if ( !LockAcquireA ( portalMutex, "GetPortalIDFreeSlot" ) )
				return -1;

			portalID = LookForFreeSlot ( portalGeneratorsLastAssigned, ( void ** ) portalGenerators );

			CVerbUnLock ( "GetPortalIDFreeSlot: portalMutex" );

			LockReleaseVA ( portalMutex, "GetPortalIDFreeSlot" );
		}

		if ( portalID < 0 || portalID >= MAX_PORTAL_STREAMS_A_DEVICE ) {
			CWarnID ( "GetPortalIDFreeSlot: No free slot found!" );
			portalID = -1 ;
		}
		else {
			portalID |= RemovePortalIndex ( portalDetails );

			CVerbArgID ( "GetPortalIDFreeSlot: [%s] [0x%X]", ( IsPortalReceiver () ? "Incoming" : "Outgoing" ), portalID );
		}

		return portalID;
	}


	void DeviceBase::SetDirectContactStatus ( bool hasContact )
	{
		CVerbVerbArgID ( "SetDirectContactStatus: value [%i] - previous [%i]", hasContact, hasPhysicalContact );

		if ( hasContact ) {
			portalInfoOff.width = width_coverage;
			portalInfoOff.height = height_coverage;

			if ( hasPhysicalContact <= 0 )
				hasPhysicalContact = 1;
			else
				hasPhysicalContact++;
		}
		else {
			if ( hasPhysicalContact > 0 )
				hasPhysicalContact = 0;
			else
				hasPhysicalContact--;
		}

		if ( deviceStatus >= DeviceStatus::Connected )
		{
			if ( abs ( hasPhysicalContact ) < 3 )
			{
				PortalInfoBase info;
				info.portalID = ( PORTAL_TYPE_ANY | PORTAL_DIR_OUTGOING );

				if ( hasContact ) {
					UpdatePortalSize ( 0, width_coverage, height_coverage );

					if ( GetPortalInfo ( &info ) ) {
						onEnvironsNotifierContext1 ( env, nativeID, NOTIFY_DEVICE_ON_SURFACE, info.portalID, &info, sizeof ( PortalInfoBase ) );
					}
					else hasPhysicalContact--;
				}
				else {
					if ( GetPortalInfo ( &info ) ) {
						onEnvironsNotifierContext1 ( env, nativeID, NOTIFY_DEVICE_NOT_ON_SURFACE, info.portalID, &info, sizeof ( PortalInfoBase ) );
					}
					else hasPhysicalContact++;
				}
			}

			unsigned int sendValue = ( unsigned int ) hasContact;

            onEnvironsAsyncSend1 ( env, nativeID, 0, true, MSG_TYPE_OPTIONS, MSG_OPT_CONTACT_DIRECT_SET, &sendValue, sizeof ( unsigned int ) );
            //onEnvironsAsyncSend1 ( env, deviceNode->info.objID, 0, true, MSG_TYPE_OPTIONS, MSG_OPT_CONTACT_DIRECT_SET, &sendValue, sizeof ( unsigned int ) );
		}
	}


	inline void refactorMallocBuffer ( char * &msg_Start, ComMessageHeader * &header, char * buffer,
		int currentLength, char * &msg_CurrentEnd )
	{
		CVerbVerbArg ( "refactorMallocBuffer: Refactoring memory usage, currentLength = %i", currentLength );

		// Refactor memory usage
		unsigned int partSize = ( unsigned int ) ( msg_Start - buffer );
		if ( !partSize ) {
			CVerbVerb ( "refactorMallocBuffer: Refactoring not neccessary since buffer is already at beginning." );
			return;
		}

		if ( currentLength < ( int ) partSize ) { // No overlap
			memcpy ( buffer, msg_Start, currentLength );
		}
        else { // Overlap, so copy multipass
            void * tmp = malloc ( currentLength );
            if ( !tmp ) {
                CErrArg ( "refactorMallocBuffer: Allocation of memory failed [ %i ]", currentLength );
                return;
            }
            
            memcpy ( tmp, msg_Start, currentLength );
            
            memcpy ( buffer, tmp, currentLength );
            
            free ( tmp );
            /*
			char * dest = buffer;
			char * src = msg_Start;

			int remain = currentLength - partSize;
			while ( remain > 0 ) {
				if ( remain < ( int ) partSize )
					partSize = remain;
				memcpy ( dest, src, partSize );

				dest += partSize;
				src += partSize;
				remain -= partSize;
				partSize += partSize;
			}
            */
		}
		msg_Start = buffer;
		header = ( ComMessageHeader * ) msg_Start;
		msg_CurrentEnd = msg_Start + currentLength;
	}


	inline bool relocateMallocBuffer ( char * &msg_Start, char * &msg_BufferEnd, ComMessageHeader * &header,
		char * &buffer, unsigned int &capacity,
		unsigned int msg_Length, int currentLength, char * &msg_CurrentEnd )
	{
		CVerb ( "relocateMallocBuffer" );

		// Increase buffer capacity
		unsigned int aligned = msg_Length + ( 4 - ( msg_Length % 4 ) );

		CVerbArg ( "relocateMallocBuffer: Increasing buffer capacity to %u aligned to %u (+1024) bytes", msg_Length, aligned );

		// Add some spare space after that buffer
		aligned += 1024;

		char * newBuffer = ( char * ) malloc ( aligned );
		if ( !newBuffer ) {
			CErrArg ( "relocateMallocBuffer: Failed to allocate receive buffer to capacity of %i bytes!", aligned );
			return false;
		}

		// Copy message to new buffer
		CVerbArg ( "relocateMallocBuffer: memcpy %i bytes", currentLength );
		memcpy ( newBuffer, msg_Start, currentLength );
		CVerb ( "relocateMallocBuffer: disposing buffer" );
		free ( buffer );

		buffer 		= newBuffer;
		capacity 	= aligned;

		msg_Start 			= buffer;
		header 				= ( ComMessageHeader * ) msg_Start;
		msg_CurrentEnd		= msg_Start + currentLength;

		msg_BufferEnd 		= msg_Start + capacity;

		return true;
	}


	bool DeviceBase::CreatePortalReceiver ( int portalID )
	{
		CVerbArgID ( "CreatePortalReceiver: portalID [0x%X]", portalID );

		int id = PortalIndex ();
		if ( IsInvalidPortalIndex ( id ) )
			return false;

		bool notify  = false;
		bool success = false;

		CVerbLock ( "CreatePortalReceiver: portalReceiversMutex" );

		if ( !LockAcquireA ( portalReceiversMutex, "CreatePortalReceiver" ) )
			return false;

		do
		{
			if ( portalReceivers [ id ] ) {
				CVerbID ( "CreatePortalReceiver: A decoder is already available." );
				success = true;
				break;
			}

			CreatePortalReceiverPlatform ( id );

			if ( !portalReceivers [ id ] )
				portalReceivers [ id ] = new PortalReceiver ();

			if ( portalReceivers [ id ] ) {
				portalReceivers [ id ]->env = env;
				portalReceivers [ id ]->device = ( void * ) this;

				if ( portalReceivers [ id ]->Init ( deviceID, portalID ) )
					if ( portalReceivers [ id ]->Start () ) {

						portalReceiversCount++;
						portalID = portalReceivers [ id ]->portalID;
						notify = success = true;
					}
			}
		}
		while ( 0 );

		CVerbUnLock ( "CreatePortalReceiver: portalReceiversMutex" );

		LockReleaseVA ( portalReceiversMutex, "CreatePortalReceiver" );

		if ( notify )
			onEnvironsNotifier1 ( env, nativeID, NOTIFY_PORTAL_INCOMING_ESTABLISHED, portalID );
		return success;
	}


	void DeviceBase::DisposePortal ( int portalID )
	{
		CVerbID ( "DisposePortal" );

		unsigned int id = PortalIndex ();
		if ( id >= MAX_PORTAL_STREAMS_A_DEVICE )
			return;

		bool notify = false;

		if ( IsPortalGenerator () )
		{
			PortalGenerator * portal = 0;

			if ( !LockAcquireA ( portalMutex, "DisposePortal" ) )
				return;

			if ( portalGenerators [ id ] ) {
				portal = portalGenerators [ id ];
				portalGenerators [ id ] = 0;
				portalGeneratorsCount--;
			}

			LockReleaseVA ( portalMutex, "DisposePortal" );

			if ( portal ) {
				portal->Stop ();
				delete portal;
			}

			if ( !SendPortalMessage ( MSG_PORTAL_STOP_ACK, portalID ) ) {
				CErrID ( "DisposePortal: Failed to send portal stop ack message to device!" );
			}

			return;
		}

		CVerbLock ( "DisposePortal: portalReceiversMutex" );

		PortalReceiver * portal = 0;

		LockAcquireVA ( portalReceiversMutex, "DisposePortal" );

		if ( portalReceivers [ id ] ) {
			portal = portalReceivers [ id ];
			portalReceivers [ id ] = 0;
			portalReceiversCount--;
			notify = true;
		}

		CVerbUnLock ( "DisposePortal: portalReceiversMutex" );

		LockReleaseVA ( portalReceiversMutex, "DisposePortal" );

		if ( portal )
			delete portal;

		if ( notify )
			onEnvironsNotifier1 ( env, nativeID, NOTIFY_PORTAL_STREAM_STOPPED, portalID );
	}


	bool DeviceBase::GetActiveStatus ()
	{
		CVerb ( "GetActiveStatus" );

		return ( deviceID > 0 && IsValidFD ( interactSocket ) && deviceStatus != DeviceStatus::Deleteable );
	}


	void DeviceBase::GetAlignedDimensions ( int &width, int &height )
	{
		int w, h;

		if ( width > 0 && height > 0 ) {
			w = width;
			h = height;
		}
		else {
			if ( streamOptions.useNativeResolution ) {
				w = display.width;
				h = display.height;

				if ( streamOptions.limitMaxResolution ) {
					if ( w > 800 || h > 1280 ) {
						w >>= 1;
						h >>= 1;
					}
				}
			}
			else {
				w = width_coverage;
				h = height_coverage;
			}
		}

		/*if (w % 32) {
		int mod = w % 32;
		if ( mod < 16 && w > 16 )
		w -= mod;
		else
		w += 32 - mod;
		}
		if (h % 32) {
		int mod = h % 32;
		if ( mod < 16 && h > 16 )
		h -= mod;
		else
		h += 32 - mod;
		}*/
		const int align = 32;
		/*int mod = w % 4;
		if ( mod ) {
		w += (4 - mod);
		}
		mod = h % 4;
		if ( mod ) {
		h += (4 - mod);
		}*/
		int mod = w % align;
		if ( mod ) {
			w += align - mod;
		}

		mod = h % align;
		if ( mod ) {
			h += align - mod;
		}

		width = w;
		height = h;
	}


	DeviceBase * DeviceBase::GetDeviceForHandshake ( Instance * env, int deviceID, const char * areaName, const char * appName, bool interactChannel, int & sock, struct sockaddr_in * addr )
	{
        CVerbID ( "GetDeviceForHandshake" );
        
        CSocketTraceUpdate ( sock, "DeviceBase GetDeviceForHandshake" );
        
        sp ( MediatorClient ) mediator = env->mediator MED_WP;
        if ( !mediator )
            return 0;
        
        if ( !mediator->IsConnectAllowed ( deviceID, appName, areaName ) )
            return 0;
        
        int     success         = 0;
        char *  areaNameBuffer  = 0;
        char *  appNameBuffer   = 0;
        
		DeviceBase * device     = 0;
        
	Retry:
		if ( !WaitForDeviceDeletion ( env, deviceID, areaName, appName, true ) ) {
			CErrID ( "GetDeviceForHandshake: Wait for device deletion failed." );
			return 0;
		}

		device = GetDevice ( env, deviceID, areaName, appName, false );
		if ( device )
        {            
			if ( device->activityStatus & DEVICE_ACTIVITY_REQUESTOR )
            {
				/// A regular connection seems to be ongoing.. 
				CWarnID ( "GetDeviceForHandshake: Device is already connecting as the requestor. Deciding on the connect request by (probably the same remote device)." );

				do
				{
					if ( device->deviceStatus >= DeviceStatus::Connected ) {
						CWarnID ( "GetDeviceForHandshake: Canceling request due to active and valid connection." );
						break;
					}

					if ( addr ) {
						unsigned int deviceIP	= interactChannel ? device->interactAddr.sin_addr.s_addr : device->comDatAddr.sin_addr.s_addr;
						unsigned int curIP		= addr->sin_addr.s_addr;

						if ( deviceIP != curIP ) {
							CWarnArgID ( "GetDeviceForHandshake: Canceling request due to different IPs [ %s ] of ongoing connect..", inet_ntoa ( addr->sin_addr ) );
							break;
						}
					}

					/// So. Let's compare the deviceIDs
					/// If we are the "better" one, then our connect has priority. Therefore, we cancel this request path now.
					if ( deviceID < env->deviceID ) {
						CWarnID ( "GetDeviceForHandshake: Canceling request due to priority check." );
						break;
					}

                    /// If we are not the "winner", then dispose our other ongoing connect and make this request path (connect initiated by the other device) the priority path.
                    CVerbsID ( 4, "GetDeviceForHandshake: Disposing device and query again." );
                    
					CVerbID ( "GetDeviceForHandshake: Setting deviceStatus to Deleteable" );
					device->deviceStatus = DeviceStatus::Deleteable;

					success = -1;
				}
				while ( false );

				UnlockDevice ( device );

				if ( success < 0 ) {
					TriggerCleanUpDevices ();

					CVerbID ( "GetDeviceForHandshake: Retry looking for an active device ..." );
					success = 0;
					device = 0;
					LockReleaseA ( devicesAccessMutex, "GetDeviceForHandshake" );
					goto Retry;
				}

				if ( success == 0 )
					goto Finish;
			}

			if ( interactChannel ) {
                device->interactThread.Lock ( "GetDeviceForHandshake" );
                
                int s = device->interactSocketForClose;
                
                if ( IsInvalidFD ( s ) ) {
					CSocketTraceUpdate ( sock, "DeviceBase assigned to interactSocket in GetDeviceForHandshake" );
					device->interactSocket			= sock;
                    device->interactSocketForClose	= sock;
                    
                    if ( addr )
                        memcpy ( &device->interactAddr, addr, sizeof ( struct sockaddr_in ) );
                    else {
                        CWarnID ( "GetDeviceForHandshake: Invalid sockaddr_in argument for interact channel." );
                    }
                }                
                
                device->interactThread.Unlock ( "GetDeviceForHandshake" );
                
				if ( IsValidFD ( s ) ) {
					CSocketTraceUpdate ( sock, "GetDeviceForHandshake: Canceling allocated interactSocket (already established)." );
					//closesocket ( sock );
					//sock = INVALID_FD;

					UnlockDevice ( device );
					goto Finish;
				}
			}
            else {
                device->comDatThread.Lock ( "GetDeviceForHandshake" );
                
                int s = device->comDatSocketForClose;
                
                if ( IsInvalidFD ( s ) ) {
					CSocketTraceUpdate ( sock, "DeviceBase assigned to comDatSocket in GetDeviceForHandshake" );
					device->comDatSocket			= sock;
                    device->comDatSocketForClose	= sock;
                    
                    if ( addr )
                        memcpy ( &device->comDatAddr, addr, sizeof ( struct sockaddr_in ) );
                    else {
                        CWarnID ( "GetDeviceForHandshake: Invalid sockaddr_in argument for comDat channel." );
                    }
                }                
                
                device->comDatThread.Unlock ( "GetDeviceForHandshake" );

				if ( IsValidFD ( s ) ) {
					CSocketTraceUpdate ( sock, "GetDeviceForHandshake: Canceling allocated comDatSocket (already established)." );
					//closesocket ( sock );
					//sock = INVALID_FD;

					UnlockDevice ( device );
					goto Finish;
				}
            }

			// Take ownership of the socket handle from caller. We will close it.
			sock = INVALID_FD;

            success = 1;
		}
		else {
            CVerbID ( "GetDeviceForHandshake: Creating new device." );
            
            CSocketTraceUpdate ( sock, "DeviceBase assigned new device in GetDeviceForHandshake" );
			sp ( DeviceController ) deviceSP = std::make_shared < DeviceController > ( deviceID, interactChannel, sock, addr );
            if ( !deviceSP ) {
                CErrID ( "GetDeviceForHandshake: Allocation of a device controller object failed!" );
                goto Finish;
            }

			// Take ownership of the socket handle from caller. We will close it.
			sock    = -1;
            
            device  = ( DeviceBase * ) deviceSP.get ();
            
			if ( !device->Init ( env->myself, areaName, appName ) ) {
				CWarnsID ( 2, "GetDeviceForHandshake: Initialization of a device controller object failed!" );
				goto Finish;
			}

			device->activityStatus |= DEVICE_ACTIVITY_RESPONDER;

			if ( !device->AllocateUdpSocket () ) {
				CErrID ( "GetDeviceForHandshake: Allocation of udp socket failed.!" );
				goto Finish;
			}

			int nativeID = AddDevice ( deviceSP, false );
			if ( nativeID <= 0 )
				goto Finish;
            
            device->deviceNode->deviceSP    = deviceSP;
            
			CVerbID ( "GetDeviceForHandshake: increasing reference count on new device." );
			IncLockDevice ( device );

			CVerbID ( "GetDeviceForHandshake: setting deviceStatus to ConnectInProgress" );
			device->deviceStatus = DeviceStatus::ConnectInProgress;

			device->UpdateConnectStatus ( 1, true );
            
            success = 1;
		}

	Finish:
		LockReleaseA ( devicesAccessMutex, "GetDeviceForHandshake" );
        
        free_n ( areaNameBuffer );
        
        free_n ( appNameBuffer );

        if ( success >= 1 )
            return device;
        return 0;
	}


	bool DeviceBase::HandshakeShortMessage ( Instance * env, ComMessageHeader * headeredPkg )
	{
		ShortMsg * msg = ( ShortMsg * ) headeredPkg;

		unsigned int pkgLength = headeredPkg->length;

		( ( char * ) headeredPkg ) [ headeredPkg->length ] = 0;

		int deviceID = msg->header.deviceID;
		if ( !deviceID ) {
			CErrID ( "HandshakeShortMessage: Identified with an invalid id == 0!" );
			return false;
		}

		char * appName	= msg->appArea;
		char * areaName = appName + msg->header.sizes [ 0 ];

		if ( msg->header.sizes [ 0 ] > MAX_NAMEPROPERTY || msg->header.sizes [ 1 ] > MAX_NAMEPROPERTY ) {
			CErrArgID ( "HandshakeShortMessage: Invalid sizes of app [ %d ] area [ %d ] names!", msg->header.sizes [ 0 ], msg->header.sizes [ 1 ] );
			return false;
		}

		char * text = areaName + msg->header.sizes [ 1 ];

		int msgLength = ( int ) ( pkgLength - ( text - ( char * ) msg ) );
		if ( msgLength <= 0 ) {
			CErrArgID ( "HandshakeShortMessage: Invalid length [ %i ]", msgLength );
			return false;
		}

		CVerbsVerbArgID ( 3, "HandshakeShortMessage: [ %s / %s ] msg [ %s ]", appName, areaName, text );

		// Check Mediator filter level
		if ( env->mediatorFilterLevel >= MEDIATOR_FILTER_AREA ) {
			if ( *areaName && strncmp ( areaName, env->areaName, sizeof ( env->areaName ) - 1 ) )
				return false;
		}

		if ( env->mediatorFilterLevel >= MEDIATOR_FILTER_AREA_AND_APP ) {
			if ( *appName && strncmp ( appName, env->appName, sizeof ( env->appName ) - 1 ) )
				return false;
		}

		char tmp = 0;
		if ( pkgLength > 32 ) {
			tmp = text [ 32 ]; text [ 32 ] = 0;
		}
        CLogsArgID ( 1, "HandshakeShortMessage: [ %s / %s ] msg [ %s ]", *appName ? appName : env->appName, *areaName ? areaName : env->areaName, text );

		if ( pkgLength > 32 && tmp ) {
			text [ 32 ] = tmp;
		}

		onEnvironsMsgNotifier ( env, deviceID, areaName, appName, SOURCE_DEVICE, text, msgLength, "id" );

		return false;
	}


	void AESTakeOver ( int deviceID, AESContext * aesSrc, AESContext * aesDst, pthread_mutex_t * mutex, const char * channel )
	{
		if ( pthread_mutex_lock ( mutex ) ) {
			CErrArgID ( "AESTakeOver%sChannel: Failed to acquire mutex.", channel );
		}

		if ( aesDst->encCtx )
			AESDisposeKeyContext ( aesSrc );
		else {
			if ( aesSrc->deviceID != deviceID ) {
				AESUpdateKeyContext ( aesSrc, deviceID );
				aesSrc->deviceID = deviceID;
			}

			memcpy ( aesDst, aesSrc, sizeof ( AESContext ) );
            
#ifdef ENABLE_CRYPT_AES_LOCKED_ACCESS
			LockInitA ( aesDst->encLock );
			LockInitA ( aesDst->decLock );
			aesDst->lockAllocated = true;

			LockDisposeA ( aesSrc->encLock );
			LockDisposeA ( aesSrc->decLock );
			aesSrc->lockAllocated = false;
#endif
            aesSrc->encCtx = 0;
		}

		if ( pthread_mutex_unlock ( mutex ) ) {
			CErrArgID ( "AESTakeOver%sChannel: Failed to release mutex.", channel );
		}
	}


	bool DeviceBase::HandshakeComDatChannel ( Instance * env, bool isStunt, int & sock, struct sockaddr_in * addr, char * buffer, int &bytesRead, AESContext * aes )
	{
		CVerb ( "HandshakeComDatChannel: device channel request" );

		char * payload = ( char * ) &( ( ComMessageHeader * ) buffer )->payload;
		bool ret = false;
		int deviceID;

		const int minSize = MSG_HEADER_SIZE + ( sizeof ( unsigned int ) * 2 );
        
		if ( bytesRead < minSize )
		{
			int toRead = minSize - bytesRead;
            
			// We need to receive the complete message
			int bytesRead1 = MediatorClient::Receive ( sock, buffer + bytesRead, toRead, toRead, "HandshakeComDatChannel" );
			if ( bytesRead1 <= 0 ) {
				// -1 means that the socket has closed, so we don't need the listener anymore
				CWarn ( "HandshakeComDatChannel: Socket to device has been closed!!" );
				return false;
			}
            
			if ( bytesRead1 != toRead ) {
				CErr ( "HandshakeComDatChannel: Wrong handshake data! Requestor does not follow handshake convention." );
				return false;
			}
			bytesRead = minSize;
		}

		DeviceBase * device	= HandshakeDevice ( env, payload, false, sock, addr, aes );
		if ( !device )
			goto Finish;

		device->stuntRedundant = isStunt;
		deviceID = *( ( unsigned int * ) payload );

		if ( device->interactAddr.sin_addr.s_addr && ( device->interactAddr.sin_addr.s_addr != addr->sin_addr.s_addr ) ) {
			CErrID ( "HandshakeComDatChannel: The device comDat request's IP does not match with the prior device request's IP" );
			goto Finish;
		}

        if ( !device->env )
            device->env = env;

		if ( device->StartComDatListener () )
			ret = true;

	Finish:
		if ( device ) {
			if ( !ret ) {
				device->deviceStatus = DeviceStatus::Deleteable;

				TriggerCleanUpDevices ();
			}

			UnlockDevice ( device );
		}
		return ret;
	}


	DeviceBase * DeviceBase::HandshakeDevice ( Instance * env, char * handshake, bool mainChannel, int & sock, struct sockaddr_in * addr, AESContext * aes )
    {
        CSocketTraceUpdate ( sock, "DeviceBase HandshakeDevice" );
        
		char			*	areaName		= 0;

		int					deviceID		= *( ( int * ) handshake );
		int					platform		= *( ( int * ) ( handshake + 4 ) );
		DeviceBase		*	device			= 0;

		CVerbID ( "HandshakeDevice: Device identification message" );

		unsigned char * sizes = (unsigned char *) (handshake + 10);

		char * appName = handshake + 12;

		CVerbArgID ( "HandshakeDevice: Device identifiers [ 0x%X / 0x%X ]: [ %s ]!", deviceID, platform, appName );
		if ( !deviceID ) {
			CErrID ( "HandshakeDevice: Identified with an invalid id == 0!" );
			return 0;
		}		

		if ( *sizes >= MAX_NAMEPROPERTY || *( sizes + 1 ) >= MAX_NAMEPROPERTY ) {
			CErr ( "HandshakeDevice: AppName/AreaName length to large." );
			return 0;
		}

		areaName = appName + *sizes;
		*(areaName + *(sizes + 1)) = 0;

		if ( *appName && *areaName ) {
			device = GetDeviceForHandshake ( env, deviceID, areaName, appName, mainChannel, sock, addr );
		}
		else
			device = GetDeviceForHandshake ( env, deviceID, 0, 0, mainChannel, sock, addr );

		if ( !device )
			return 0;

		device->platform = ( Platforms_t ) platform;
		if ( aes->encCtx ) {
			AESTakeOver ( deviceID, aes, &device->aes, &device->portalMutex, mainChannel ? "Interact" : "ComDat" );
			device->encrypt = 1;
		}

		return device;
	}


	bool DeviceBase::HandshakeInteractChannel ( Instance * env, bool isStunt, int & sock, struct sockaddr_in * addr, char * payload, AESContext * aes )
	{
		unsigned int		deviceID		= *( ( unsigned int * ) payload );
        
		DeviceBase		*	device			= HandshakeDevice ( env, payload, true, sock, addr, aes );
		if ( !device )
			goto Failed;

		device->stuntRedundant = isStunt;

		if ( device->comDatAddr.sin_addr.s_addr && ( device->comDatAddr.sin_addr.s_addr != addr->sin_addr.s_addr ) ) {
			CErrID ( "HandshakeInteractChannel: The device interact channel request's IP does not match with the prior device request's IP" );
			goto Failed;
		}

		if ( !device->StartInteractListener () ) {
			CErrID ( "HandshakeInteractChannel: Failed to start interact channel thread!" );
			goto Failed;
		}

		UnlockDevice ( device );
		return true;

	Failed:
		if ( device ) {
			device->deviceStatus = DeviceStatus::Deleteable;

			UnlockDevice ( device );

			TriggerCleanUpDevices ();
		}
		
		return false;
	}


	bool DeviceBase::HandshakeAndResponse ( Instance * env, int sock, struct sockaddr_in * addr )
	{
		CVerb ( "HandshakeAndResponse" );

		bool				success = false;
		unsigned short		payloadType;
		char			*	decrypted = 0;
		bool                isStunt = false;

		char			*	buffer	= 0;
		ComMessageHeader *	header	= 0;
		char			*	msg		= 0;
		int					bytesRead;

		AESContext			aes;
		Zero ( aes );

		up ( char [ ] )		bufferUP = up ( char [ ] ) ( new char [ DEVICE_HANDSHAKE_BUFFER_MAX_SIZE ] );
        
        if ( !bufferUP || env->environsState < environs::Status::Starting )
			goto Failed;

		buffer	= bufferUP.get ();
		header	= ( ComMessageHeader * ) buffer;
		msg		= buffer;
        
		// --- Stunt handshake label ------
	HandshakeStuntRequest:
		// --- Stunt handshake label ------

		bytesRead = MediatorClient::Receive ( sock, buffer, DEVICE_HANDSHAKE_BUFFER_MAX_SIZE - 1, 4, "HandshakeAndResponse" );
		if ( bytesRead <= 0 ) {
			CWarn ( "HandshakeAndResponse: Socket to device has been closed." );
			goto Failed;
		}

		CVerbVerbArg ( "HandshakeAndResponse: Received Handshake of [ %i ] bytes.", bytesRead );

		if ( buffer [ 4 ] == 'H' && buffer [ 5 ] == 'C' && buffer [ 6 ] == 'L' && buffer [ 7 ] == 'S' ) {
			bytesRead = SecureChannelProvide ( env, sock, &aes );
			if ( !bytesRead )
				goto Failed;
            
            bytesRead = MediatorClient::ReceiveOneMessage ( bytesRead > 0, &aes, sock, buffer, DEVICE_HANDSHAKE_BUFFER_MAX_SIZE - 1, decrypted );
            if ( bytesRead <= 0 ) {
                CWarn ( "HandshakeAndResponse: Socket to device has been closed." );
                goto Failed;
            }
            
			if ( decrypted ) {
				msg     = decrypted;
				header  = ( ComMessageHeader * ) msg;
			}
		}
		else {
			int testStunt = 0;
			if ( bytesRead == STUNT_HANDSHAKE_IDENT_SIZE ) {
				testStunt = StunTRequest::Handshake ( env, sock, buffer );
				if ( testStunt < 0 )
					goto Failed;
				else if ( testStunt == 1 ) {
					isStunt = true;
					goto HandshakeStuntRequest;
				}
			}

			if ( bytesRead <= ( int ) MSG_HEADER_SIZE ) {
				bytesRead += MediatorClient::Receive ( sock, buffer + bytesRead, ( DEVICE_HANDSHAKE_BUFFER_MAX_SIZE - 1 ) - bytesRead, 0, "HandshakeAndResponse" );
                
				if ( bytesRead <= ( int ) MSG_HEADER_SIZE )
				{
					CErr ( "HandshakeAndResponse: Failed to receive unencrypted Handshake." );
					goto Failed;
				}
			}

            if ( bytesRead == STUNT_HANDSHAKE_IDENT_SIZE ) {
                CSocketTraceUpdate ( sock, "DeviceBase performed StunTRequest.Handshake in HandshakeAndResponse" );
                
				testStunt = StunTRequest::Handshake ( env, sock, buffer );
				if ( testStunt < 0 )
					goto Failed;
				else if ( testStunt == 1 ) {
					isStunt = true;
					goto HandshakeStuntRequest;
				}
			}
		}

		if ( env->useCLSForDevicesEnforce && !aes.encCtx ) {
			CWarn ( "HandshakeAndResponse: Secured channel is enforced, but a device connects un-encrypted. We're refusing this connection." );
			goto Failed;
		}

		// "MSG;LEN; HID"
		if ( header->type != MSG_TYPE_HELO )
			goto Failed;

		payloadType = header->MessageType.payloadType;

		if ( payloadType == MSG_HANDSHAKE_MAIN_REQ )
		{
			msg [ bytesRead ] = 0;
            
            CSocketTraceUpdate ( sock, "DeviceBase performed HandshakeInteractChannel" );
			if ( HandshakeInteractChannel ( env, isStunt, sock, addr, ( char * ) &header->payload, &aes ) ) {
				success = true;
				goto Success;
			}
		}

		// "MSG;LEN; HIB"
		else if ( payloadType == MSG_HANDSHAKE_COMDAT_REQ )
        {
            CSocketTraceUpdate ( sock, "DeviceBase performed HandshakeComDatChannel" );
			if ( HandshakeComDatChannel ( env, isStunt, sock, addr, msg, bytesRead, &aes ) ) {
				success = true;
				goto Success;
			}
		}

		else if ( payloadType == MSG_HANDSHAKE_SHORT_MESSAGE )
		{
			msg [ bytesRead ] = 0;

			if ( header->length > ( unsigned ) bytesRead )
				header->length = bytesRead;

			HandshakeShortMessage ( env, header );
			success = true;
		}

	Failed:
		if ( IsValidFD ( sock ) )
			ShutdownCloseSocket ( sock, true, "DeviceBase HandshakeAndResponse" );

		AESDisposeKeyContext ( &aes );

		if ( !success )
			TriggerCleanUpDevices ();

    Success:
        free_n ( decrypted );
        
		return success;
	}


	bool DeviceBase::SendDeviceConfig ()
	{
		CVerbID ( "SendDeviceConfig" );

		char deviceConfig [ MSG_BUFFER_SEND_SIZE ];

		int numChars = snprintf ( deviceConfig, MSG_BUFFER_SEND_SIZE,
			"id:%i;rsize:%i;wp:%i;hp:%i;w:%i;h:%i;do:%i;tr:h4%i;pr:%i;an:%s;pn:%s;pl:%i;pt:%i<EOF>",
			env->deviceID, dataRecSize, native.display.width,
			native.display.height, native.display.width_mm, native.display.height_mm, native.display.orientation,
			( int ) env->useStream, ( int ) env->useNativeResolution, env->appName, env->areaName, ( int ) native.platform, ( int ) env->useTcpPortal );

		if ( numChars <= 0  ) {
			CErrID ( "SendDeviceConfig: Failed to build helo message!" );
			return false;
		}

		if ( numChars >= MSG_BUFFER_SEND_SIZE ) {
			CWarnID ( "SendDeviceConfig: Helo message has been truncated!" );
			numChars = MSG_BUFFER_SEND_SIZE - 1;
		}

		deviceConfig [ numChars ] = 0;

		numChars++;

		if ( SendBuffer ( false, MSG_TYPE_HELO, 0, 0, MSG_HANDSHAKE_CONIG_RESP, deviceConfig, numChars ) != numChars ) {
			CErrID ( "SendDeviceConfig: Failed to send device configuration" );
			return false;
		}

		return true;
	}


	bool DeviceBase::HandleComDatChannel ( int sock, struct sockaddr_in * addr )
	{
		CVerbID ( "HandleComDatChannel" );

		comDatSocket			= sock;
		comDatSocketForClose	= sock;

		memcpy ( &comDatAddr, addr, sizeof ( comDatAddr ) );

		return StartComDatListener ();
	}



	bool DeviceBase::HandleHeloMessage ( unsigned short packetType, char * msg, unsigned int msgLen )
	{
		CVerbArgID ( "HandleHeloMessage: %i", packetType );

		if ( packetType == MSG_HANDSHAKE_PORTS )
		{
			char * payload = msg + MSG_HEADER_SIZE;

			comPort = ( unsigned short ) *( ( unsigned int * ) payload );
			dataPort = ( unsigned short ) *( ( unsigned int * ) ( payload + 4 ) );

			CVerbArgID ( "HandleHeloMessage: received port config [ %i : %i ].", comPort, dataPort );

			if ( SendBuffer ( false, MSG_TYPE_HELO, 0, 0, MSG_HANDSHAKE_PORTS_ACK, 0, 0 ) != 0 ) {
				CErrID ( "HandleHeloMessage: Failed to send ports ACK." );
				return false;
			}

			return true;
		}

		if ( packetType == MSG_HANDSHAKE_CONIG_REQ ) {
			CVerbID ( "HandleHeloMessage: Device configuration requested." );

			return SendDeviceConfig ();
		}

		if ( packetType == MSG_HANDSHAKE_CONIG_RESP ) {
			CVerbID ( "HandleHeloMessage: Device configuration received." );

			// Make sure that config string is null terminated. Its okay to truncate last charater.
			msg [ msgLen - 1 ] = 0;

			if ( EvaluateDeviceConfig ( ( char * ) &( ( ComMessageHeader * ) msg )->payload ) ) {

				if ( SendBuffer ( false, MSG_TYPE_HELO, 0, 0, MSG_HANDSHAKE_CONIG_RESP_ACK, 0, 0 ) != 0 ) {
					CErrID ( "HandleHeloMessage: Failed to send ACK for device configuration response." );
					return false;
				}

				return true;
			}

			CErrID ( "HandleHeloMessage: Invalid device configuration received, failed to retrieve device id!!!" );
		}

		if ( packetType == MSG_HANDSHAKE_SUCCESS ) {
			CVerbID ( "HandleHeloMessage: Connection established!" );

			if ( comPort == 0 || dataPort == 0 ) {
				// We haven't received ports during the handshake, so .. bye bye..
				UpdateConnectStatus ( -20 );

				onEnvironsNotifier1 ( env, deviceNode->info.objID, NOTIFY_CONNECTION_MAIN_FAILED );
				return false;
			}

            PrepareStorage ();

			OnPreConnectionEstablished ();

			if ( SendBuffer ( false, MSG_TYPE_HELO, 0, 0, MSG_HANDSHAKE_MAIN_ACK, &env->deviceID, sizeof ( env->deviceID ) ) != sizeof ( env->deviceID ) ) {
				CErrID ( "HandleHeloMessage: Failed to send connection ack message to device!" );
				return false;
			}
            
            bool update = false;
            
            LockAcquireVA ( portalMutex, "HandleHeloMessage" );
            
			if ( !( activityStatus & DEVICE_ACTIVITY_MAIN_CONNECTED ) ) {
				activityStatus |= DEVICE_ACTIVITY_MAIN_CONNECTED;
                update = true;
            }
            
            LockReleaseVA ( portalMutex, "HandleHeloMessage" );

            if ( update ) {
				UpdateConnectStatus ( 20 );

				onEnvironsNotifier1 ( env, deviceNode->info.objID, NOTIFY_CONNECTION_MAIN_ACK );
				OnConnectionEstablished ();
            }
            
            interactThreadState = 2; // Thread shall pause.

			return true;
		}

		if ( packetType == MSG_HANDSHAKE_MAIN_ACK ) {
			CVerbID ( "HandleHeloMessage: Main channel by device acknowledged." );
            
            bool update = false;
            
            LockAcquireVA ( portalMutex, "HandleHeloMessage" );
            
            if ( !( activityStatus & DEVICE_ACTIVITY_MAIN_CONNECTED ) ) {
                activityStatus |= DEVICE_ACTIVITY_MAIN_CONNECTED;
                update = true;
            }
            
            LockReleaseVA ( portalMutex, "HandleHeloMessage" );
            
			if ( update ) {
				UpdateConnectStatus ( 20 );

				onEnvironsNotifier1 ( env, deviceNode->info.objID, NOTIFY_CONNECTION_MAIN_ACK );
				OnConnectionEstablished ();
			}
            
            interactThreadState = 2; // Thread shall pause.
			return true;
		}

		if ( packetType == MSG_HANDSHAKE_CONNECTED ) {
			CVerbID ( "HandleHeloMessage: Connection by device acknowledged." );

			onEnvironsNotifier1 ( env, deviceNode->info.objID, NOTIFY_CONNECTION_ESTABLISHED_ACK );
			return true;
        }
        
        if ( packetType == MSG_HANDSHAKE_PING ) {
            CVerbID ( "HandleHeloMessage: Channel ping ..." );
            return true;
        }

		return false;
	}


	/*
	* startDataChannel:
	* - can be called multimple times due to helo packet drops to surface
	*/
	bool DeviceBase::InitUdpChannel ()
	{
		CVerbID ( "InitUdpChannel" );

		if ( activityStatus & DEVICE_ACTIVITY_UDP_CONNECTED ) {
			CVerbID ( "InitUdpChannel: udp channel already established." );
			return true;
		}

		if ( behindNAT )
			return StunRequest::EstablishRequest ( this );

		udpAddr.sin_addr.s_addr 	= interactAddr.sin_addr.s_addr;
		udpAddr.sin_family 			= PF_INET;
		udpAddr.sin_port 			= htons ( dataPort );

		// Start listener thread for the socket
		if ( deviceStatus == DeviceStatus::Deleteable ) {
			CWarnID ( "InitUdpChannel: stopping due to device disposal." );
			return false;
		}

		if ( UdpSendHelo () )
			UdpSendHelo ();

		return true;
	}


	bool DeviceBase::StartInteractListener ()
	{
        CVerbID ( "StartInteractListener" );
        
        bool success = true;

		if ( !interactThread.isRunning () )
		{
			//
			// Create a new Thread to handle the new connection request
			//
			IncLockDevice ( this );

			if ( deviceStatus == DeviceStatus::Deleteable ||
				!interactThread.Run ( pthread_make_routine ( &DeviceController::InteractListenerStarter ), this, "StartInteractListener" ) ) {

				// The Run call may fail if the thread is already running.
				// Hence, we do fail only if the thread really could not be created
				if ( !interactThread.isRunning () ) {
					CVerb ( "StartInteractListener: Setting deviceStatus to Deleteable" );
					deviceStatus = DeviceStatus::Deleteable;

					success = false;
				}

				UnlockDevice ( this );
			}
		}
		return success;
	}



	bool DeviceBase::StartComDatListener ()
	{
        CVerbID ( "StartComDatListener" );
        
        bool success = true;

		if ( !comDatThread.isRunning () )
		{
            if ( !env )
                success = false;
            else
            {
                //
                // Create a new Thread to handle the new connection request
                //
                IncLockDevice ( this );

                if ( deviceStatus == DeviceStatus::Deleteable ||
                    !comDatThread.Run ( pthread_make_routine ( &DeviceController::ComDatListenerStarter ), this, "StartComDatListener" ) ) {

                    // The Run call may fail if the thread is already running.
                    // Hence, we do fail only if the thread really could not be created
                    if ( !comDatThread.isRunning () ) {
                        CVerb ( "StartComDatListener: setting deviceStatus to Deleteable" );
                        deviceStatus = DeviceStatus::Deleteable;
                        
                        success = false;
                    }
                    
                    UnlockDevice ( this );
                }
            }
        }
		return success;
	}


	bool DeviceBase::StartUdpListener ()
	{
        CVerbID ( "StartUdpListener" );

		if ( deviceStatus == DeviceStatus::Deleteable )
			return false;

		bool success = true;

		if ( !udpThread.isRunning () )
		{
			IncLockDevice ( this );

			if ( deviceStatus == DeviceStatus::Deleteable ||
				!udpThread.Run ( pthread_make_routine ( &DeviceBase::UdpListenerStarter ), this, "StartUdpListener" ) ) {

				// The Run call may fail if the thread is already running.
				// Hence, we do fail only if the thread really could not be created
				if ( !udpThread.isRunning () ) {
					success = false;
				}

				UnlockDevice ( this );
			}
		}
        return success;
	}

	
	void * DeviceBase::ComDatListenerStarter ( void * arg )
    {
        DeviceBase * device = ( DeviceBase * ) arg;

		if ( device->deviceStatus != DeviceStatus::Deleteable )
			device->ComDatListener ();

        device->comDatThread.Notify ( "ComDatListenerStarter" );
        
        device->comDatThread.Detach ( "ComDatListenerStarter" );

		UnlockDevice ( device );
        return 0;
	}


	unsigned int DeviceBase::PrepareHandshakeBuffer ( char * buffer )
	{
		// Prepare the buffer header
		*( ( int * ) buffer ) = env->deviceID;

		*( ( int * ) ( buffer + 4 ) ) = native.platform;

		unsigned char * sizes = ( unsigned char * ) ( buffer + 10 );

		if ( !BuildAppAreaField ( sizes, env->appName, env->areaName, false ) ) {
			CErr ( "PrepareHandshakeBuffer: Failed to build AppName/AreaName ident." );
			return 0;
		}

		if ( *sizes >= MAX_NAMEPROPERTY || *( sizes + 1 ) >= MAX_NAMEPROPERTY ) {
			CErr ( "PrepareHandshakeBuffer: AppName/AreaName length to large." );
			return 0;
		}

		return ( 12 + *sizes + *( sizes + 1 ) );
	}


	bool DeviceBase::EstablishComDatChannel ()
	{
		CVerbID ( "EstablishComDatChannel" );

		if ( comDatSocket == -1 )
		{
            if ( behindNAT ) {
                if ( !StunTRequest::EstablishRequest ( env, this, MEDIATOR_STUNT_CHANNEL_BULK, connectToken ) )
                {
					if ( deviceStatus != DeviceStatus::Deleteable ) {
						CErrID ( "EstablishComDatChannel: Failed to create STUNT comDat socket." );
					}
					return false;
				}
			}
			else {
				if ( !AllocateComDatSocket () )
					return false;

				if ( !LockAcquireA ( portalMutex, "EstablishComDatChannel" ) ) {
					return false;
				}

				// Connect to device
				CVerbID ( "EstablishComDatChannel: Connecting ..." );
				int rc = Mediator::Connect ( deviceID, comDatSocket, ( struct sockaddr * )&comDatAddr, 0, "ComDat channel" );

				if ( !LockReleaseA ( portalMutex, "EstablishComDatChannel" ) )
					return false;

				if ( rc != 0 ) {
                    CErrArgID ( "EstablishComDatChannel: Failed to connect [ %s : %i ]", inet_ntoa ( comDatAddr.sin_addr ), ntohs ( comDatAddr.sin_port ) );
                    
                    if ( IsValidFD ( comDatSocket ) ) { LogSocketErrorF ( "DeviceBase.EstablishComDatChannel" ); }
					return false;
				}
			}
			UpdateConnectStatus ( 10 );

			onEnvironsNotifier1 ( env, deviceNode->info.objID, NOTIFY_CONNECTION_COMDAT_NEW );

			if ( env->useCLSForDevices ) {
				int rc = SecureChannelEstablish ( env, deviceID, comDatSocket, &aes, aesBlob );
				if ( !rc )
					return false;
				if ( rc > 0 )
					encrypt = 1;
				else {
                    if ( aes.encCtx CRYPT_AES_LOCK_EXP ( || aes.lockAllocated ) ) {
						LockAcquireVA ( portalMutex, "EstablishComDatChannel" );

						AESDisposeKeyContext ( &aes );

						LockReleaseVA ( portalMutex, "EstablishComDatChannel" );
					}
				}
			}

			char handshakeBuffer [ MSG_BUFFER_SEND_SIZE ];

			int payloadLength = PrepareHandshakeBuffer ( handshakeBuffer );
			if ( !payloadLength )
				return false;

			if ( SendBuffer ( true, MSG_TYPE_HELO, 0, 0, MSG_HANDSHAKE_COMDAT_REQ, handshakeBuffer, payloadLength ) < 0 ) {
				CErrID ( "EstablishComDatChannel: Failed to send comDat channel initialization to device." );
				return false;
			}

			if ( !InitUdpChannel () )
				return false;
		}
		else {
#ifdef ENABLE_DEVICEBASE_WP_STUNT
			if ( stuntRedundant && !stuntComDat.expired () )
			{
				CVerbID ( "EstablishComDatChannel: Closing ongoing stunt request." );

				CloseStunt ( stuntComDat, true, &stuntComDatState, "comDat" );
			}
#else
            bool closeStunt = false;
            
			LockAcquireVA ( spLock, "EstablishComDatChannel" );

            if ( stuntComDat )
            {
                if ( stuntRedundant ) {
					closeStunt = true;
                }
                else {
                    stuntComDat = 0;
                }
            }

			LockReleaseVA ( spLock, "EstablishComDatChannel" );

			if ( closeStunt )
			{
				CVerbID ( "EstablishComDatChannel: Closing ongoing stunt request." );

				CloseStunt ( stuntComDat, true, &stuntComDatState, "comDat" );
			}
#endif
			UpdateConnectStatus ( 10 );

			onEnvironsNotifier1 ( env, deviceNode->info.objID, NOTIFY_CONNECTION_COMDAT_NEW );

			if ( SendBuffer ( true, MSG_TYPE_HELO, 0, 0, MSG_HANDSHAKE_COMDAT_ACK, 0, 0 ) < 0 ) {
				CErrID ( "EstablishComDatChannel: Failed to send ack message to device!" );
				return false;
			}
		}

		TuneReceiveBuffer ( comDatSocket );

		return true;
	}


	bool DeviceBase::HandleFileTransfer ( ComMessageHeader * header, bool isBulkChannel )
	{
		unsigned int		payloadSize = header->length - MSG_HEADER_SIZE;
		CVerbID ( "HandleFileTransfer" );

		int dataLength;
		MessageHeaderPartitions * cHeader = ( MessageHeaderPartitions * ) header;

		const char * descriptor = "0";
		unsigned int descLen;

		/// Get fileID from first 4 bytes
		int fileID = header->MessagePack.fileID;

		if ( header->MessageType.payloadType == NATIVE_FILE_TYPE_PARTS )
		{
			if ( ( cHeader->parts * PARTITION_PART_SIZE ) > MAX_BULK_SEND_SIZE )
				return true;

			dataLength = header->length - MSG_PARTITIONS_HEADER_SIZE;

			char * payload = ( char * ) &cHeader->payload;

			if ( cHeader->part < 2 || cHeader->part == cHeader->parts ) {
#ifndef NDEBUG
                if ( cHeader->part < 2 ) {
                    CVerbsArgID ( 2, "HandleFileTransfer: Received a part [ %i / %i ] file ID [ %i ] of overall size [ %u ]", cHeader->part, cHeader->parts, fileID, payloadSize );
                }
#endif  
                if ( cHeader->part == cHeader->parts ) {
                    CLogArgID ( "HandleFileTransfer: Received a part [ %i / %i ] file ID [ %i ] of overall size [ %u ]", cHeader->part, cHeader->parts, fileID, payloadSize );
                }

				descLen = cHeader->descriptorLength;
				if ( descLen ) {
					descLen += 1;
					descriptor = payload;

					SaveToStorageDescription ( fileID, descriptor, descLen );

					payload += descLen;
					dataLength -= descLen;
#ifndef NDEBUG
                    CLogsArgID ( 1, "HandleFileTransfer: Descriptor [ %s ]", descriptor );
#else
                    CLogsArgID ( (cHeader->part == cHeader->parts ? 1 : 2), "HandleFileTransfer: descriptor [ %s ]", descriptor );
#endif
				}
			}
			else {
				CVerbsArgID ( 2, "HandleFileTransfer: Received a part [ %i / %i ] file ID [ %i ] of overall size [ %u ]", cHeader->part, cHeader->parts, fileID, payloadSize );

				descLen = cHeader->descriptorLength;
			}

			/// Make sure that device provided size does not overrun our message buffer
			if ( ( dataLength + descLen ) > payloadSize ) {
				CErrArgID ( "HandleFileTransfer: File size provided by the message header [ %u ] overflows the actual payload size [ %u ]. We're truncating the size.", ( dataLength + descLen ), payloadSize );
				dataLength = payloadSize - descLen;
			}

			if ( ( lastFileID != fileID || ( unsigned ) lastFilePart >= cHeader->part ) && lastFile ) {
				fclose ( lastFile );
				lastFile = 0;
			}

			if ( cHeader->part == 1 ) {
				if ( SaveToStorage ( fileID, &lastFile, 0, payload, ( unsigned ) dataLength ) ) {
					lastFileID      = fileID;
					lastFilePart	= cHeader->part;

					/// Push data to application
					onEnvironsDataNotifier1 ( env, deviceNode->info.objID, nativeID, SOURCE_DEVICE, fileID, descriptor, descLen, 0 );
				}
			}
			else {
				if ( SaveToStorage ( fileID, &lastFile, ( cHeader->part - 1 ) * PARTITION_PART_SIZE, payload, ( unsigned ) dataLength ) ) {
					lastFileID      = fileID;
					lastFilePart	= cHeader->part;

					if ( cHeader->parts )
						onEnvironsNotifierContext1 ( env, deviceNode->info.objID, NOTIFY_FILE_RECEIVE_PROGRESS, fileID, 0, ( cHeader->part * 100 / cHeader->parts ) );
				}

				if ( cHeader->part >= cHeader->parts ) {
					if ( lastFile ) {
						fclose ( lastFile );
						lastFile = 0;
					}
					lastFilePart = 0;

					lastFileID = -1;

					SendBuffer ( true, MSG_TYPE_MESSAGE, 0, 0, NATIVE_FILE_TYPE_ACK, payload, 4 );

					/// Push data to application
					int fileSize = ( ( cHeader->part - 1 ) * PARTITION_PART_SIZE ) + dataLength;

					onEnvironsDataNotifier1 ( env, deviceNode->info.objID, nativeID, SOURCE_DEVICE, fileID, descriptor, descLen, fileSize );
				}
			}
		}
		else {
			CInfoArgID ( "HandleFileTransfer: Received a file ID [ %i ] of overall size [ %u ]", fileID, payloadSize );

			char *	payload = ( char * ) &header->payload;

			dataLength = header->length - MSG_HEADER_SIZE ;
			descLen = *( ( unsigned int * ) payload );

			/// Look whether we have a filename
			if ( descLen ) {
				descriptor = payload + 4;
				CInfoArgID ( "HandleFileTransfer: Descriptor [ %s ]", descriptor );

				SaveToStorageDescription ( fileID, descriptor, descLen );

				descLen += 5;
				payload += descLen;
				dataLength -= descLen;
			}
			else {
				payload += 4;
				dataLength -= 4;
			}

			/// Send Ack if the app has taken it over
			SendBuffer ( true, MSG_TYPE_MESSAGE, 0, 0, NATIVE_FILE_TYPE_ACK, payload, 4 );

			/// Make sure that device provided size does not overrun our message buffer
			//descLen += 4;
			if ( ( dataLength + descLen ) > payloadSize ) {
				CErrArgID ( "HandleFileTransfer: File size provided by the message header [ %u ] overflows the actual payload size [ %u ]. We're truncating the size.", ( dataLength + descLen ), payloadSize );
				dataLength = payloadSize - descLen;
			}

			if ( lastFile ) {
				fclose ( lastFile );
				lastFile = 0;
			}
            SaveToStorage ( fileID, 0, 0, payload, ( unsigned ) dataLength );
            
            if ( lastFile ) {
                fclose ( lastFile );
                lastFile = 0;
            }

			/// Push data to application
			onEnvironsDataNotifier1 ( env, deviceNode->info.objID, nativeID, SOURCE_DEVICE, fileID, descriptor, descLen, dataLength );
		}
		return true;
	}
    
    
#ifndef ENABLE_DEVICEBASE_SINGLE_COMDAT_THREAD
	/**
	* Note: The ComDatListener is kept alive by means of the comDatListenerAlive member which holds an SP to the parent object.
    */
	void * DeviceBase::ComDatListener ()
	{
		CVerbID ( "ComDatListener: Working thread started ..." );

        pthread_setname_current_envthread ( "DeviceBase.ComDatListener" );

		ByteBuffer * byteBuffer		= 0;
		int 		bytesRead 		= 0;
		char 	*	buffer 			= 0;
		char 	* 	msg_Start 		= 0;
		char	*	msg_BufferEnd;
		char 	* 	msg_CurrentEnd 	= 0;
		char		*	decrypted	= 0;
		unsigned int	packLen		= 0;
		int			currentLength	= 0;
		bool 		header_available = false;
		bool		doReset			= false;
		int			msg_Length 		= 0;

		unsigned int bufferSize		= 0;
		int 		remainBufferSize;

		ComMessageHeader * header 	= ( ComMessageHeader * ) msg_Start;
     
		byteBuffer = ( ByteBuffer * ) allocBuffer ( TCP_DEVICEBASE_START_SIZE );
		if ( !byteBuffer ) {
			CErrID ( "ComDatListener: Failed to allocate memory for comDat data receiving!" );
			goto EndWithValue;
		}

		buffer 			= BYTEBUFFER_DATA_POINTER_START ( byteBuffer );
		bufferSize		= byteBuffer->capacity;
		msg_Start 		= buffer;
		msg_BufferEnd	= msg_Start + bufferSize;
		msg_CurrentEnd 	= buffer;

		if ( !EstablishComDatChannel () ) {
			UpdateConnectStatus ( -20 );

			onEnvironsNotifier1 ( env, deviceNode->info.objID, NOTIFY_CONNECTION_COMDAT_FAILED );
			goto EndWithValue;
        }

		while ( deviceStatus != DeviceStatus::Deleteable )
		{
			if ( doReset ) {
				CVerbVerbID ( "ComDatListener: doReset" );
				msg_Length      = currentLength         = 0;
				msg_CurrentEnd  = msg_Start             = buffer;
				doReset         = header_available      = false;
				header          = ( ComMessageHeader * ) msg_Start;
			}

			remainBufferSize = ( int ) ( msg_BufferEnd - msg_CurrentEnd );
            
			CVerbVerbArgID ( "ComDatListener: remainBufferSize %i", remainBufferSize );

			if ( ( remainBufferSize < msg_Length ) && ( msg_Start != buffer ) ) {
				refactorMallocBuffer ( msg_Start, header, buffer, currentLength, msg_CurrentEnd );

				remainBufferSize = ( int ) ( msg_BufferEnd - msg_CurrentEnd );
			}

			if ( bufferSize < ( unsigned ) msg_Length ) {
				CVerbVerbArgID ( "ComDatListener: relocating ByteBuffer - message length %u, capacity = %u", msg_Length, bufferSize );

				if ( msg_Length > ( PARTITION_SEND_BUFFER_SIZE * 2 ) ) {
					CErrArgID ( "ComDatListener: Protocol error - message length [ %i bytes ] exceeds twice the max part size [ %i ]! Flushing whole stream!",
						msg_Length, ( PARTITION_SEND_BUFFER_SIZE * 2 ) );
					doReset = true;
					continue;
				}

				ByteBuffer * newBuffer = relocateBuffer ( byteBuffer, false, msg_Length + 1024 );
				if ( !newBuffer ) {
					CErrArgID ( "ComDatListener: Failed to relocate receive buffer to capacity %i bytes!", msg_Length + 1024 );
					break;
				}

				// Copy message to new buffer
				memcpy ( BYTEBUFFER_DATA_POINTER_START ( newBuffer ), msg_Start, currentLength );
				disposeBuffer ( byteBuffer );

				byteBuffer 			= newBuffer;
				bufferSize			= byteBuffer->capacity;
				buffer 				= BYTEBUFFER_DATA_POINTER_START ( newBuffer );
				msg_Start 			= buffer;
				msg_BufferEnd		= msg_Start + bufferSize;
				msg_CurrentEnd		= msg_Start + currentLength;
				header 				= ( ComMessageHeader * ) msg_Start;

				remainBufferSize	= ( int ) ( msg_BufferEnd - msg_CurrentEnd );
			}
            
            if ( IsValidFD ( comDatSocket ) ) {
                bytesRead = ( int ) recv ( comDatSocket, msg_CurrentEnd, remainBufferSize, 0 );
                CVerbVerbArgID ( "ComDatListener: bytesRead %i", bytesRead );
            }
            
            if ( bytesRead <= 0 ) {
                CVerbID ( "ComDatListener: Socket to device has been closed!!" );
                break;
            }
            msg_CurrentEnd += bytesRead;
			currentLength += bytesRead;

		HandleMessage:
			if ( currentLength < 13 ) {
				CVerbVerbArgID ( "ComDatListener: Warning - Message received is incomplete; msg [%i] Bytes [%i]. Continue receiving...", currentLength, bytesRead );
				continue;
			}

			if ( !header_available )
			{
				if ( msg_Start [ 0 ] != 'M' ||  msg_Start [ 1 ] != 'S' || msg_Start [ 2 ] != 'G' || msg_Start [ 3 ] != ';' )
				{
					if ( aes.decCtx ) {
						packLen				= *( ( unsigned int * ) msg_Start );
						unsigned int flags	= 0xF0000000 & packLen;
						msg_Length			= packLen & 0xFFFFFFF;

						if ( flags & 0x40000000 ) { /// Encrypted package
							packLen = msg_Length;

							if ( packLen > ( unsigned ) currentLength ) {
								// We haven't received the whole message! go on receiving...
								CVerbVerbArgID ( "ComDatListener: Encrypted message [%i] is longer than currentLength [%i]!", packLen, currentLength );
								continue;
							}

							if ( AESDecrypt ( &aes, msg_Start, &packLen, &decrypted ) )
							{
								if ( decrypted [ 0 ] == 'M' && decrypted [ 1 ] == 'S' && decrypted [ 2 ] == 'G' && decrypted [ 3 ] == ';' )
								{
									header = ( ComMessageHeader * ) decrypted;
									goto HandleHeader;
								}
								else {
									if ( IsFINMessage ( decrypted ) ) {
										CVerb ( "ComDatListener: FIN!" );
										break;
									}

                                    CErr ( "ComDatListener: MSG preamble is missing in decrypted message!" );
                                    
                                    CVerbVerbArgID ( "ComDatListener: decrypted [%s]", ConvertToHexSpaceString ( decrypted, packLen ) );
                                    
                                    free ( decrypted );
                                    decrypted = 0;
								}
							}
						}
                    }
                    
                    if ( IsFINMessage ( msg_Start ) ) {
                        CVerb ( "ComDatListener: FIN!" );
                        break;
                    }

                    CErrID ( "ComDatListener: Protocol error - message header is missing! Flushing whole stream!" );
                    
                    CVerbVerbArgID ( "ComDatListener: ciphers [%s]", ConvertToHexSpaceString ( msg_Start, msg_Length ) );
                    
					doReset = true;
					continue;
				}
				else {
					if ( IsFINMessage ( msg_Start ) ) {
						CVerb ( "ComDatListener: FIN!" );
						break;
					}

					header		= ( ComMessageHeader * ) msg_Start;
					msg_Length	= header->length;
				}

			HandleHeader:
				// Get size of message
				packLen = header->length;
				CVerbVerbArgID ( "ComDatListener: length of message %i", packLen );

				if ( packLen > 200000000 ) {
					CErrArgID ( "ComDatListener: Invalid message format. Message length %u > 200000000!", packLen );
					break;
				}
				header_available = true;
			}

			CVerbVerbArgID ( "ComDatListener: msgLength %i, currentLength %i", packLen, currentLength );
			if ( packLen > ( unsigned ) currentLength ) {
				// We haven't received the whole message! go on receiving ...
                // Note: This can only happen on unencrypted streams
				CVerbVerbArgID ( "ComDatListener: message [%i] is longer than bytes [%i] received. Continue receiving...", packLen, bytesRead );
				continue;
			}

			unsigned int msgType = ( unsigned int ) header->type;
			if ( msgType < MSG_TYPE_MAX_COUNT ) {
				if ( !( ( *this.*msgHandlers [ msgType ] ) ( header, true ) ) )
					goto EndWithValue;
			}
            
            free_m ( decrypted );

			// Is there a message pending in stream?
			currentLength -= msg_Length;
			if ( currentLength <= 0 ) {
				doReset = true;
				continue;
			}

			// Continue with next message in stream
			header_available = false;
			msg_Start += msg_Length;
			header = ( ComMessageHeader * ) msg_Start;
			goto HandleMessage;
		}

	EndWithValue:
		CVerbID ( "ComDatListener: Setting deviceStatus to Deleteable" );

		// We make this reference in order to make sure that the compiler does not optimize our SP away...
		deviceStatus = DeviceStatus::Deleteable;

		// this call delays due to waiting for the UI-thread to consume this call..
		UpdateConnectStatus ( -20 );

		onEnvironsNotifier1 ( env, deviceNode->info.objID, NOTIFY_CONNECTION_COMDAT_CLOSED );

		if ( byteBuffer ) {
			disposeBuffer ( byteBuffer );
			byteBuffer = 0;
		}
        
        free_m ( decrypted );

		TriggerCleanUpDevices ();

		CVerbID ( "ComDatListener: bye bye..." );

		return NULL;
    }
#endif
    
    
    bool DeviceBase::SendPing ()
    {
        CVerbID ( "SendPing" );
        
        ComMessageHeader msg;
        Zero ( msg );
        
        msg.preamble [ 0 ] = 'M';
        msg.preamble [ 1 ] = 'S';
        msg.preamble [ 2 ] = 'G';
        msg.preamble [ 3 ] = ';';
        
        msg.version    = TCP_MSG_PROTOCOL_VERSION;
        msg.type       = MSG_TYPE_HELO;
        
        msg.MessageType.payloadType = MSG_HANDSHAKE_PING;
        /*
        if ( SendHeaderedBuffer ( &msg, sizeof ( ComMessageHeader ) ) <= 0 ) {
            CVerbID ( "SendPing: Failed." );
            return false;
        }
        */
        if ( SendComDatMessage ( &msg, sizeof ( ComMessageHeader ) ) <= 0 ) {
            CVerbID ( "SendPing: Failed." );
            return false;
        }
        
        
        CVerbID ( "SendPing: ok." );
        return true;
    }

    
    void SendUdpFIN ( int sock )
    {
        if ( sock != -1 ) {
            send ( sock, "FIN;", 4, 0 );
            
            MediatorClient::SendUDPFIN ( sock );
        }
    }


	bool DeviceBase::SendDataPacket ( const char * msg, int length, struct sockaddr * dest )
	{
		CVerbVerbID ( "SendDataPacket" );

		if ( length <= 0 ) {
			CErrArgID ( "SendDataPacket: Length of message is invalid [%i]!", length );
			return false;
		}

		if ( IsInvalidFD ( udpSocket ) ) {
			CErrID ( "SendDataPacket: Invalid udp socket!" );
			return false;
		}

		int sentBytes = ( int ) sendto ( udpSocket, msg, length, 0, dest, sizeof ( struct sockaddr ) );

		if ( sentBytes < length ) {
			CWarnsArgID ( 3, "SendDataPacket: Failed to send packet! Sent bytes and length of packet mismatch (%i != %i);", sentBytes, length );
			VerbLogSocketError ();

			if ( errno == EISCONN ) {
				udpCoreConnected = true;
			}

			return false;
		}

		return true;
	}


	bool DeviceBase::SendDataPacket ( const char * msg, unsigned int length )
	{
		CVerbVerbID ( "SendDataPacket" );

		if ( length == 0 ) {
			CErrID ( "SendDataPacket: Length of message is 0!" );
			return false;
		}

		if ( IsInvalidFD ( udpSocket ) ) {
			CErrID ( "SendDataPacket: Invalid udp socket!" );
			return false;
		}

		int sentBytes = ( int ) send ( udpSocket, msg, length, 0 );

		if ( sentBytes < ( int ) length ) {
            CErrArgID ( "SendDataPacket: Failed to send packet! Bytes sent and length of packet mismatch (%d != %d);", sentBytes, length );
            
            if ( IsValidFD ( udpSocket ) ) { LogSocketErrorF ( "DeviceBase.SendDataPacket" ); }

			return false;
		}

		return true;
	}


	/*
	* Format: HEADER (12) DATA

	* STRUCT: Preamble   Length     Version  Type         SubType
	* HEADER: 0 1 2 3    4 5 6 7    8        9		       10 11
	* Note: Have a look at ComMessageHeader for message type/subtype definitions
	*/

	int DeviceBase::SendHeaderedBuffer ( void * msg, int length )
	{
		CVerbVerbID ( "SendHeaderedBuffer" );

		if ( length <= 0 ) {
			CErr ( "SendHeaderedBuffer: Length of message <= 0!" );
			return -1;
		}

		if ( deviceStatus <= DeviceStatus::ConnectInProgress )
			return -1;

		char * cipher = 0;
		*( ( int * ) ( ( ( char * ) msg ) + 4 ) ) = length;

		if ( encrypt ) {
			if ( !AESEncrypt ( &aes, ( char * ) msg, ( unsigned int* ) &length, &cipher ) )
				return -1;
			msg = cipher;
		}

		CVerbArgID ( "SendHeaderedBuffer: sending [%d] bytes", length );

		if ( pthread_mutex_lock ( &interactSocketLock ) ) {
			CErrID ( "SendHeaderedBuffer: Failed to aquire mutex!" );
			return 0;
		}

		int bytesSent = 0;

		if ( IsValidFD ( interactSocket ) ) {
			bytesSent = ( int ) send ( interactSocket, ( const char * ) msg, length, MSG_NOSIGNAL );
			if ( bytesSent < length ) {
				CWarnArgID ( "SendHeaderedBuffer: Failed to send packet to device! sent bytes and length of packet mismatch [%i != %i].",
					bytesSent, length );
				LogSocketErrorID ();
			}
		}

		if ( pthread_mutex_unlock ( &interactSocketLock ) ) {
			CErrID ( "SendHeaderedBuffer: Failed to release mutex!" );
		}
        
        free_n ( cipher );
		return bytesSent;
	}


	bool DeviceBase::SendPortalMessage ( unsigned short portalMessage, int portalID )
	{
		CVerbArgID ( "SendPortalMessage: PortalID [ 0x%X ]", portalID );

		portalID &= 0xFFFFFF;

        ComMessageHeader msg;
        Zero ( msg );
        
        msg.preamble [ 0 ] = 'M';
        msg.preamble [ 1 ] = 'S';
        msg.preamble [ 2 ] = 'G';
        msg.preamble [ 3 ] = ';';

		msg.version    = TCP_MSG_PROTOCOL_VERSION;
		msg.type       = MSG_TYPE_PORTAL;
        
		msg.MessageType.payloadType    = portalMessage;
		msg.MessagePack.portalID       = portalID;

		*( ( unsigned int * ) &msg.payload ) = portalID;

		CVerbArgID ( "SendPortalMessage: [ %s ]", resolveName ( portalMessage ) );

        /*
		if ( !SendHeaderedBuffer ( &msg, sizeof ( ComMessageHeader ) ) ) {
			CErrID ( "SendPortalMessage: Failed to send portal command" );
			return false;
        }
        */
        if ( SendComDatMessage ( &msg, sizeof ( ComMessageHeader ) ) <= 0 ) {
            CErrID ( "SendPortalMessage: Failed to send portal command" );
            return false;
        }

		CVerbArgID ( "SendPortalMessage: PortalID [ 0x%X ] ok.", portalID );
		return true;
	}


	int DeviceBase::SendComDatMessage ( void * msg, int length )
	{
		if ( length <= 0 ) {
			CErrID ( "SendComDatMessage: Length of message <= 0!" );
			return -1;
		}

		if ( deviceStatus <= DeviceStatus::ConnectInProgress )
			return -1;

		int toSend = length;
		char * cipher = 0;
		*( ( int * ) ( ( ( char * ) msg ) + 4 ) ) = toSend;
        
        //CLogArg ( "SendBulkMessage: ciphers [%s]", ConvertToHexSpaceString ( (char *)msg, length) );
        
		if ( encrypt ) {
			if ( !AESEncrypt ( &aes, ( char * ) msg, ( unsigned int* ) &toSend, &cipher ) )
				return -1;
			msg = cipher;
		}

		CVerbArgID ( "SendComDatMessage: Sending [ %d ] bytes", toSend );
        
        int bytes = 0;
        bool loop = true;
        
    Retry:
		if ( comDatSocket != -1 ) {
			bytes = ( int ) send ( comDatSocket, ( const char * ) msg, toSend, MSG_NOSIGNAL );
		}
        
		if ( bytes < toSend )
        {
            if ( bytes < 0 && loop )
            {
				SOCKET_Check_Val ( check );
                
                if ( SOCKET_Check_Retry ( check ) )
                {
                    struct pollfd desc;
                    desc.fd      = comDatSocket;
                    desc.events  = POLLOUT;
                    desc.revents = 0;
                    
                    if ( comDatSocket != -1 )
                    {
                        bytes = poll ( &desc, 1, 500 );
                        if ( bytes >= 1 )
                        {
                            if ( desc.revents & POLLOUT )
                            {
                                loop = false; goto Retry;
                            }
                        }
                    }
                }
            }
            else if ( loop ) {
                loop = false; goto Retry;
            }
            
			CErrArgID ( "SendComDatMessage: Failed to send packet to device! sent bytes and length of packet mismatch [ %i != %i ].",
                       bytes, toSend );
            
            if ( IsValidFD ( comDatSocket ) ) { LogSocketErrorF ( "DeviceBase.SendComDatMessage" ); }
		}
		else if ( encrypt )
			bytes = length;
        
        free_n ( cipher );
		return bytes;
	}


	int DeviceBase::SendBufferInParts ( bool comDat, char msgType, int fileID, const char * fileDescriptor, unsigned short payloadType, const void * payload, int payloadSize )
	{
		if ( payloadSize <= 0 || payloadSize > MAX_BULK_SEND_SIZE ) {
			CErrArgID ( "SendBufferInParts: Length of message [ %i ] > 200MB! We don't support his yet.", payloadSize ); return -1;
		}

		if ( payloadSize && !payload ) {
			CErrID ( "SendBufferInParts: Length of payload is given, but no payload to send!" ); return -1;
		}

		/// Use a buffer on the thread stack
		/// Aes uses padding of 16 bytes; An aes pack increases payload with 20 bytes (size, iv)
		/// The message header size is 16 bytes
		/// 
		bool			locked		= false;
        int          *  sockP;
		int				bytesSent	= -1,
                        bytes		= 0;
		unsigned int	msgSize;
		char		 *	cipher		= 0;
		const char 	 *	payloadStart = ( const char * ) payload;

		unsigned int	part		= 0,
			parts		= 0,
			partSize	= 0,
			descLen		= 0;
        
        pthread_mutex_t * mutex     = 0;

		if ( fileDescriptor )
			descLen = ( unsigned int ) strlen ( fileDescriptor );

		if ( descLen > ( PARTITION_SEND_BUFFER_SIZE >> 1 ) ) {
			CErrArg ( "SendBufferInParts: Length of descriptor is too long [ %i bytes ]", descLen );
			return -1;
		}
        
        size_t bufferSize = PARTITION_SEND_BUFFER_SIZE + 20 + descLen;
        
        CVerbVerbArg ( "SendBufferInParts: Alloc buffer size [ %i ]", bufferSize );
        
        char * sendBuffer = ( char * ) malloc ( bufferSize );
        if ( !sendBuffer ) {
            CErrID ( "SendBufferInParts: Failed to allocate memory for sending!" ); return -1;
        }
        
        MessageHeaderPartitions	* header	= ( MessageHeaderPartitions * ) sendBuffer;
        char                    * toSend    = sendBuffer;
        
		// Prepare Buffer with preamble and header : MSG_HEADER_LEN
		memcpy ( header, "MSG;", 4 );

		// Version
		header->version = TCP_MSG_PROTOCOL_VERSION;

		header->type = msgType;
		if ( payloadType )
			header->MessageType.payloadType = NATIVE_FILE_TYPE_PARTS;

		header->fileID = fileID;

		if ( payloadSize > PARTITION_SEND_BUFFER_SIZE )
		{
			parts = 0;

			parts += payloadSize / PARTITION_PART_SIZE;
			if ( payloadSize % PARTITION_PART_SIZE )
				parts++;

			part = 1;
            
            CVerbVerbArg ( "SendBufferInParts: Determined parts [ %i ]", parts );
		}

		header->parts	= parts;

		mutex = comDat ? &comDatSocketLock : &interactSocketLock;
        
        sockP = comDat ? &comDatSocket : &interactSocket;
		if ( IsInvalidFD ( *sockP ) )
			goto Finish;

		CVerbArgID ( "SendBufferInParts: sending [ %u ] bytes buffer.", payloadSize );

		bytesSent = 0;

		while ( part <= parts )
        {
            CVerbVerbArg ( "SendBufferInParts: Loop [ %i / %i ]", part, parts );
            
			if ( deviceStatus <= DeviceStatus::ConnectInProgress ) {
				if ( msgType != MSG_TYPE_HELO )
					goto Finish;
			}

			msgSize = MSG_PARTITIONS_HEADER_SIZE;

			partSize = PARTITION_PART_SIZE;

			header->part	= part;
			if ( descLen && ( part < 2 || part == parts ) ) {
				header->descriptorLength	= descLen;

				if ( descLen ) {
                    char * pDesc = ( char * ) &header->payload;
                    
                    CVerbVerbArg ( "SendBufferInParts: Copy descriptor [ %s : %i ]", fileDescriptor, descLen );

					memcpy ( pDesc, fileDescriptor, descLen );
					pDesc [ descLen ] = 0;

                    msgSize += descLen + 1;
                    CVerbVerbArg ( "SendBufferInParts: msgSize [ %i ]", msgSize );
				}
			}
			else
				header->descriptorLength = 0;

			if ( payloadSize < ( int ) partSize )
				partSize = payloadSize;
            
            CVerbVerbArg ( "SendBufferInParts: Copy parts data [ %i ]. Remaining [ %i ]", partSize, bufferSize - msgSize );
            
            ASSERT_ENV ( partSize < bufferSize - msgSize, "SendBufferInParts", "Partssize to be copied is larger than the buffersize!" )
            
            memcpy ( sendBuffer + msgSize, payloadStart, partSize );

			payloadSize -= partSize;
			payloadStart += partSize;

			msgSize += partSize;

			// Length
			header->length = msgSize;

			if ( aes.encCtx ) {
                if ( cipher ) {
                    CVerbVerb ( "SendBufferInParts: Releasing previous cipher" );
                    free ( cipher ); cipher = 0; }
                
                CVerbVerbArg ( "SendBufferInParts: Encrypting msgSize [ %i ]", msgSize );
				if ( !AESEncrypt ( &aes, sendBuffer, &msgSize, &cipher ) ) {
					goto Finish;
				}
				toSend = cipher;
			}

			if ( pthread_mutex_lock ( mutex ) ) {
				CErrID ( "SendBufferInParts: Failed to aquire mutex!" );
				goto Finish;
			}
			locked = true;
            
            CVerbVerbArg ( "SendBufferInParts: Sending payload [ %i ]", msgSize );
            
            if ( *sockP == -1 )
                goto Finish;
                
            bytes = ( int ) send ( *sockP, toSend, msgSize, MSG_NOSIGNAL );
            
			if ( bytes != ( int ) msgSize ) {
                CErrArgID ( "SendBufferInParts: Failed to send buffer with parts to device! Sizes mismatch [ %i != %u ].", bytes, msgSize );
                
                if ( IsValidFD ( *sockP ) ) { LogSocketErrorF ( "DeviceBase.SendBufferInParts" ); }
                goto Finish;
			}
			else bytesSent += partSize;

			locked = false;

			if ( pthread_mutex_unlock ( mutex ) ) {
				CErrID ( "SendBufferInParts: Failed to release mutex!" );
				goto Finish;
			}

			if ( parts )
				onEnvironsNotifierContext1 ( env, deviceNode->info.objID, NOTIFY_FILE_SEND_PROGRESS, fileID, 0, ( part * 100 / parts ) );

			if ( !payloadSize ) break;

			part++;
		}

	Finish:
		if ( locked && pthread_mutex_unlock ( mutex ) ) {
			CErrID ( "SendBufferInParts: Failed to release mutex!" );
		}

		free ( sendBuffer );
        
        free_n ( cipher );

		return bytesSent;
	}


	int DeviceBase::SendBuffer ( bool comDat, char msgType, int fileID, const char * fileDescriptor, unsigned short payloadType, const void * payload, int payloadSize )
	{
		if ( payloadSize < 0 ) {
			CErr ( "SendBuffer: payloadSize < 0!" );
			return -1;
		}

		if ( msgType == MSG_TYPE_FILE && payloadSize > PARTITION_MIN_BUFFER_REQUIREMENT ) {
			return SendBufferInParts ( comDat, msgType, fileID, fileDescriptor, payloadType, payload, payloadSize );
		}

		int				bytesSent	= -1,
						payloadSent = 0;
        int          *  sockP;

		unsigned int	msgSize;
		unsigned int *	pUI;
		char			sendBuffer [ 20 ];
		char		 *	cipher		= 0,
			*tmp			= 0;
		unsigned int	headerSize	= MSG_HEADER_SIZE;

		pthread_mutex_t * mutex = comDat ? &comDatSocketLock : &interactSocketLock;

		ComMessageHeader * header = ( ComMessageHeader * ) sendBuffer;

		unsigned int descLen = 0;
		if ( msgType == MSG_TYPE_FILE )
		{
			if ( fileDescriptor ) {
				descLen = ( unsigned int ) strlen ( fileDescriptor ) ;
				if ( descLen ) {
					header = ( ComMessageHeader * ) malloc ( MSG_HEADER_SIZE * 2 + ( descLen + 5 ) );
					if ( !header ) {
						CErrID ( "SendBuffer: Failed to allocate memory for sending file descriptor!" );
						return -1;
					}
				}
			}
		}

		// Prepare Buffer with preamble and header : MSG_HEADER_LEN
		memcpy ( header, "MSG;", 4 );

		if ( payloadSize && !payload ) {
			CErrID ( "SendBuffer: Length of payload is given, but no payload to send!" );
			goto Finish;
		}
		msgSize = payloadSize + MSG_HEADER_SIZE;

		// Version
		header->version = TCP_MSG_PROTOCOL_VERSION;

		header->type = msgType;
		if ( payloadType )
			header->MessageType.payloadType = payloadType;

		header->MessagePack.fileID = fileID;

		if ( msgType == MSG_TYPE_FILE ) {
			// Add descriptor
			pUI = ( unsigned int * ) ( ( ( char * ) header ) + headerSize );
			*pUI = descLen;
			headerSize += 4;
			msgSize += 4;

			if ( descLen && fileDescriptor ) {
				pUI++;
				char * pDesc = ( char * ) pUI;
				memcpy ( pDesc, fileDescriptor, descLen );
				pDesc [ descLen ] = 0;

				headerSize += descLen + 1;
				msgSize += descLen + 1;
			}
		}

		// Length
		header->length = msgSize;

		if ( deviceStatus <= DeviceStatus::ConnectInProgress ) {
			if ( msgType != MSG_TYPE_HELO )
				goto Finish;
		}

		if ( aes.encCtx ) {
			payloadSent = payloadSize + headerSize;

			tmp = ( char * ) malloc ( payloadSent );
			if ( !tmp )
				goto Finish;

			memcpy ( tmp, header, headerSize );
			if ( payloadSize )
                memcpy ( tmp + headerSize, payload, payloadSize );
            
#ifdef DEBUGVERBVerb
            char tmpBuffer [1024];
#endif
            CVerbVerbArgID ( "SendBuffer: [ %s ] channel, msg [ %s ]", comDat ? "ComDat" : "Interact", ConvertToHexSpaceBuffer ( tmp, payloadSent, tmpBuffer ) );

			if ( !AESEncrypt ( &aes, tmp, ( unsigned int * ) &payloadSent, &cipher ) )
				goto Finish;
		}

        sockP = comDat ? &comDatSocket : &interactSocket;
        
		if ( IsInvalidFD ( *sockP ) ) {
			goto Finish;
		}

		if ( pthread_mutex_lock ( mutex ) ) {
			CErrID ( "SendBuffer: Failed to aquire mutex!" );
			goto Finish;
		}

        CVerbArgID ( "SendBuffer: Sending %s[ %d ] bytes message over [ %s ] channel.", cipher ? "encrypted " : "", msgSize, comDat ? "ComDat" : "Interact" );

        if ( cipher ) {
#ifdef DEBUGVERBVerb
            char tmpBuffer [1024];
#endif
            CVerbVerbArgID ( "SendBuffer: [ %s ] channel, ciphers [ %s ]", comDat ? "ComDat" : "Interact", ConvertToHexSpaceBuffer ( cipher, payloadSent, tmpBuffer ) );
            
            if ( IsValidFD ( *sockP ) )
                bytesSent = ( int ) send ( *sockP, cipher, payloadSent, MSG_NOSIGNAL );
            
			if ( bytesSent != payloadSent ) {
                CErrArgID ( "SendBuffer: Failed to send ecrypted buffer to device! Sizes mismatch [%i != %i].", bytesSent, payloadSent );
                
                if ( IsValidFD ( *sockP ) ) { LogSocketErrorF ( "DeviceBase.SendBuffer" ); }
			}
			else
				bytesSent = payloadSize;
		}
        else {
            if ( IsValidFD ( *sockP ) )
                bytesSent = ( int ) send ( *sockP, ( char * ) header, headerSize, MSG_NOSIGNAL | MSG_MORE );
            
			if ( bytesSent != ( int ) headerSize ) {
                CErrArgID ( "SendBuffer: Failed to send packet header to device! sent bytes and length of packet mismatch [%i != %u].", bytesSent, headerSize );
                
                if ( IsValidFD ( *sockP ) ) { LogSocketErrorF ( "DeviceBase.SendBuffer" ); }
			}
			else {
				if ( payloadSize ) {
					CVerbArgID ( "SendBuffer: sending [ %s ] channel, [ %d ] bytes. Payload", comDat ? "ComDat" : "Interact", payloadSize );
                    
                    if ( IsValidFD ( *sockP ) )
                        payloadSent = ( int ) send ( *sockP, ( const char * ) payload, payloadSize, MSG_NOSIGNAL );
                    
					if ( payloadSent != ( int ) payloadSize ) {
						CErrArgID ( "SendBuffer: Failed to send payload to device! sent bytes and length of packet mismatch [%i != %u].", payloadSent, payloadSize );
                        
                        if ( IsValidFD ( *sockP ) ) { LogSocketErrorF ( "DeviceBase.SendBuffer" ); }
					}
					else
						bytesSent = payloadSent;
				}
				else bytesSent = 0;
			}
		}

		if ( pthread_mutex_unlock ( mutex ) ) {
			CErrID ( "SendBuffer: Failed to release mutex!" );
		}

	Finish:
		if ( ( char * ) header != sendBuffer ) {
			free ( header );
        }
        
        free_n ( tmp );
        
        free_n ( cipher );

		return bytesSent;
	}


	bool DeviceBase::SendComDatBuffer ( char msgType, int fileID, const char * fileDescriptor, unsigned short payloadType, const char * bulkData, size_t size )
	{
		bool ret = false;

		if ( !bulkData || size <= 0 ) {
			CErr ( "SendComDatBuffer: Invalid data or size <= 0!" );
			return false;
		}

		if ( SendBuffer ( true, msgType, fileID, fileDescriptor, payloadType, bulkData, ( int ) size ) <= ( int ) size )
			return false;
		ret = true;

		packetSequence++;
		return ret;
	}


	bool DeviceBase::SendComDatData ( int fileID, const char * buffer, unsigned int length )
	{
		int bytesSent;

		//CLogArg ( "SendBulkData to [%i]", deviceID );

		bytesSent = SendBuffer ( true, MSG_TYPE_FILE, fileID, 0, NATIVE_FILE_TYPE_APP_DEFINED, buffer, length );
		if ( bytesSent != ( signed ) ( length + MSG_HEADER_SIZE + 4 ) )
		{
			CErrArgID ( "SendComDatData: Failed to send comDat data, bytes sent mismatch sent [ %i ] != to send [ %u ]!",
				bytesSent, ( unsigned int ) ( length + MSG_HEADER_SIZE + 4 ) );
			return false;
		}

		return true;
	}


	bool DeviceBase::SendTcpBuffer ( int nativeID, bool comDat, char msgType, unsigned short subType, void * data, int size )
	{
		CVerbIDN ( "SendTcpBuffer" );

		DeviceBase * device = GetDevice ( nativeID );
		if ( !device )
			return false;

        bool success = device->SendTcpBuffer ( comDat, msgType, subType, data, size );

        UnlockDevice ( device );
        return success;
        /*
		int bufferSize = MSG_HEADER_SIZE + size;
		char * buffer = ( char * ) calloc ( 1, bufferSize + 4 );
		if ( !buffer ) {
			UnlockDevice ( device );
			return false;
		}

		ComMessageHeader * msg = ( ComMessageHeader * ) buffer;

		memset ( msg, 0, bufferSize );

		memcpy ( msg, "MSG;", 4 );
		msg->version = TCP_MSG_PROTOCOL_VERSION;
		msg->type = msgType;
		msg->MessageType.payloadType = subType;

        if ( data ) {
            memcpy ( &msg->payload, data, size );
            
            //CLogArg ( "SendBulkMessage: ciphers [%s]", ConvertToHexSpaceString ( (char *)data, size) );
        }

		bool ret = true;
		int sentSize;

		if ( comDat )
			sentSize = device->SendComDatMessage ( buffer, bufferSize );
		else
			sentSize = device->SendHeaderedBuffer ( buffer, bufferSize );

		UnlockDevice ( device );

		if ( sentSize != bufferSize ) {
			CVerbArgIDN ( "SendTcpBuffer: Failed to send portal message to device [ %i != %i ]", sentSize, bufferSize );
			ret = false;
		}

		free ( buffer );
		return ret;
        */
    }


    bool DeviceBase::SendTcpBuffer ( bool comDat, char msgType, unsigned short subType, void * data, int size )
    {
        CVerbIDN ( "SendTcpBuffer" );

        int bufferSize = MSG_HEADER_SIZE + size;
        char * buffer = ( char * ) calloc ( 1, bufferSize + 4 );
        if ( !buffer ) {
            return false;
        }

        ComMessageHeader * msg = ( ComMessageHeader * ) buffer;

        memset ( msg, 0, bufferSize );

        memcpy ( msg, "MSG;", 4 );
        msg->version = TCP_MSG_PROTOCOL_VERSION;
        msg->type = msgType;
        msg->MessageType.payloadType = subType;

        if ( data ) {
            memcpy ( &msg->payload, data, size );

            //CLogArg ( "SendBulkMessage: ciphers [%s]", ConvertToHexSpaceString ( (char *)data, size) );
        }

        bool ret = true;
        int sentSize;

        if ( comDat )
            sentSize = SendComDatMessage ( buffer, bufferSize );
        else
            sentSize = SendHeaderedBuffer ( buffer, bufferSize );

        if ( sentSize != bufferSize ) {
            CVerbArgIDN ( "SendTcpBuffer: Failed to send buffer to device [ %i != %i ]", sentSize, bufferSize );
            ret = false;
        }
        
        free ( buffer );
        return ret;
    }
    
    
    int DeviceBase::DetectNATStatToDevice ( Instance * env, volatile DeviceStatus_t * deviceStatus, int deviceID, const char * areaName, const char * appName, unsigned int &IP, unsigned int &IPe, int &Port )
	{
		CVerb ( "DetectNATStatToDevice" );

		bool isSameNetwork;
        
        sp ( MediatorClient ) mediator = env->mediator MED_WP;
		if ( !mediator )
			return -1;
        
        int Porte = 0;
        
        if ( !mediator->GetConnectionDetails ( deviceID, areaName, appName, deviceStatus, IP, Port, IPe, Porte ) ) 
		{
			if ( env->environsState >= environs::Status::Starting ) {
				CErrsID ( 2, "DetectNATStatToDevice: Failed to retrieve connection details." );
			}
            return -1;
        }
        
        if ( deviceStatus && *deviceStatus == DeviceStatus::Deleteable ) {
            CVerbID ( "DetectNATStatToDevice: Device is deleteable." );
            return -1;
        }

		isSameNetwork = mediator->IsDeviceInSameNetwork ( deviceID, areaName, appName );

		// We do hole punching if
		// - destination is not in same subnet (direct accessible)
		// - the private and public ip of destination is not the same (NAT)
		// - we are behind a NAT
		//
		if ( !isSameNetwork && ( ( IP != IPe ) || ( mediator->GetNATStat () ) ) ) {
			// We need to puch a hole into the destination's NAT
			CVerbArgID ( "DetectNATStatToDevice: Destination is behind a NAT [ %s ]", inet_ntoa ( *( ( struct in_addr * ) &IPe ) ) );
			return 1;
		}

		return 0;
	}


	int DeviceBase::GetConnectionToDevice ( Instance * env, volatile DeviceStatus_t * deviceStatus, int deviceID, const char * areaName, const char * appName, unsigned int IP, unsigned int IPe, int rc )
	{
		CVerbsArgID ( 6, "GetConnectionToDevice: [ 0x%X -> 0x%X ]", env->deviceID, deviceID );

		int sock		= -1;
		int tcpPorti	= 0;

		sockaddr_in	addr;
		Zero ( addr );

		addr.sin_family	= PF_INET;

		// Build handshake message at first to identify us..
		if ( !*env->appName ) {
			CErrID ( "GetConnectionToDevice: No application name set." );
			return -1;
		}
        
        sp ( MediatorClient ) mediator = env->mediator MED_WP;
		if ( !mediator )
			return -1;

		// Get ip and ports from mediator if required
		if ( rc < 0 ) {
			rc = DetectNATStatToDevice ( env, deviceStatus, deviceID, areaName, appName, IP, IPe, tcpPorti );
			if ( rc == -1 ) {
				if ( env->environsState >= environs::Status::Starting ) {
					CErrsID ( 2, "GetConnectionToDevice: Failed to detect NAT status!" );
				}
				goto Finish;
			}
		}

		if ( mediator->IsIPInSameNetwork ( IP ) )
			addr.sin_addr.s_addr	= IP;
		else
			addr.sin_addr.s_addr	= IPe;


		// We do hole punching if
		// - destination is not in same subnet (direct accessible)
		// - the private and public ip of destination is not the same (NAT)
		// - we are behind a NAT
		//
		if ( rc && mediator->IsServiceAvailable () ) {
			// We need to puch a hole into the destination's NAT
			CLogArgID ( "GetConnectionToDevice: Destination is behind a NAT [ %s ]", inet_ntoa ( *( ( struct in_addr * ) &IPe ) ) );
            
            sp ( StunTRequest ) stuntReq = StunTRequest::CreateRequest ( env, deviceStatus, deviceID, areaName, appName, MEDIATOR_STUNT_CHANNEL_VERSATILE, rand () );
			if ( stuntReq )
			{
				stuntReq->IPe   = IPe;
				stuntReq->IP    = IP;
				stuntReq->Porti = ( unsigned short ) tcpPorti;

				CVerbArgID ( "GetConnectionToDevice: Retrieved stunt device [ %s : %d ]", inet_ntoa ( *( ( struct in_addr * ) &IPe ) ), stuntReq->Porte );

				int stuntSock   = stuntReq->Establish ();

				if ( stuntSock == -1 ) {
					CErrArgID ( "GetConnectionToDevice: Failed to create STUNT socket, Port [ %d ]", stuntReq->Porte );
				}
				else {
					sock = stuntSock;
					memcpy ( &addr, &stuntReq->addr, sizeof ( struct sockaddr_in ) );
				}
				stuntReq->myself = 0;
			}
		}
		else {
			CVerbsID ( 6, "GetConnectionToDevice: Destination is in same network." );

			if ( !mediator->GetPortTCP ( deviceID, areaName, appName, tcpPorti ) || tcpPorti <= 0 ) {
				CErrID ( "GetConnectionToDevice: Failed to retrieve com port!" );
				goto Finish;
			}
            addr.sin_port	= htons ( ( unsigned short ) tcpPorti );
            
			sock = ( int ) socket ( PF_INET, SOCK_STREAM, 0 ); // IPPROTO_TCP (using 0 and let the service provider choose the protocol)
			if ( IsInvalidFD ( sock ) ) {
				CErrID ( "GetConnectionToDevice: Failed to create socket for communication!" );
				LogSocketError ();
				goto Finish;
			}

			CSocketTraceAdd ( sock, "DeviceBase GetConnectionToDevice" );
            DisableSIGPIPE ( sock );
            
			rc = Mediator::Connect ( deviceID, sock, ( struct sockaddr * )&addr, native.networkConnectTimeout, "Arbitrary channel" );
			if ( rc != 0 ) {
                CErrArgID ( "GetConnectionToDevice: Failed connecting to device [ %s : %i ]", inet_ntoa ( addr.sin_addr ), tcpPorti );
                
                LogSocketErrorF ( "DeviceBase.GetConnectionToDevice" );
                
                SetNonBlockSocket ( sock, true, "GetConnectionToDevice" );

                shutdown ( sock, 2 );  //SD_SEND

				CSocketTraceRemove ( sock, "GetConnectionToDevice: Closing", 0 );
                closesocket ( sock );
				//ShutdownCloseSocket ( sock, true );
				sock = -1;
			}
		}

	Finish:
		return sock;
	}


	bool DeviceBase::SendMessageToDevice ( Instance * env, volatile DeviceStatus_t * deviceStatus, int deviceID, const char * areaName, const char * appName, void * sendBuffer, unsigned int bufferLength )
	{
		if ( !env || !env->deviceID ) {
			CVerbArgID ( "SendMessageToDevice: Instance or OUR deviceID is invalid [ 0x%X ]!", env->deviceID );
			return false;
		}

		CVerbsArgID ( 3, "SendMessageToDevice: [ 0x%X -> 0x%X ]", env->deviceID, deviceID );

		/// At first try sending through the mediator if we are forced to use stunt
		unsigned int IP = 0, IPe = 0; int Port = 0;

		int natStat = DetectNATStatToDevice ( env, deviceStatus, deviceID, areaName, appName, IP, IPe, Port );
        
        if ( natStat == 1 ) {
            sp ( MediatorClient ) mediator = env->mediator MED_WP;
            
            if ( mediator && mediator->IsServiceAvailable () && mediator->SendMessageToDevice ( deviceID, areaName, appName, sendBuffer, bufferLength ) )
                return true;
            return false;
		}

		DeviceBase::SaveToStorageMessages ( env, "od", deviceID, areaName, appName, ( const char * ) sendBuffer, bufferLength );


		const unsigned int maxMsgSize = ( DEVICE_HANDSHAKE_BUFFER_MAX_SIZE - ( sizeof ( ShortMsg ) + 2 ) );

		if ( bufferLength >= maxMsgSize ) {
			CErrArgID ( "SendMessageToDevice: Message size [ %u ] >= Buffer size [ %u ].", bufferLength, maxMsgSize );
			return false;
		}

		bool			success	= false;
		unsigned int	len;
		char		*	cipher	= 0;
		int				sock	= -1, bytesSent;
		ShortMsgHeader* header;
		ShortMsg	*	msg		= 0;

        AESContext		aes;
        Zero ( aes );

		sock = GetConnectionToDevice ( env, deviceStatus, deviceID, areaName, appName, IP, IPe, 0 );
		if ( sock == -1 )
			return false;

		len = sizeof ( ShortMsg ) + MSG_HEADER_SIZE + 10 + bufferLength;

		msg = ( ShortMsg * ) calloc ( 1, len + 2 );
		if ( !msg ) {
			CErrID ( "SendMessageToDevice: Failed to allocate memory." );
			goto Finish;
		}

		header = &msg->header;

		header->preamble [ 0 ] = 'M';
		header->preamble [ 1 ] = 'S';
		header->preamble [ 2 ] = 'G';
		header->preamble [ 3 ] = ';';

		header->version		= TCP_MSG_PROTOCOL_VERSION;
		header->type		= MSG_TYPE_HELO;
		header->payloadType	= MSG_HANDSHAKE_SHORT_MESSAGE;

		// Prepare the buffer header
		header->deviceID	= env->deviceID;

		header->sizes [ 0 ] = 1;
		header->sizes [ 1 ] = 1;

		// Append appArea only if we are allowed to see other appAreas
		if ( env->mediatorFilterLevel < MEDIATOR_FILTER_AREA_AND_APP && areaName && appName ) 
		{
			// Append appArea only if the destination is located in a different appAreas
			if ( strncmp ( areaName, env->areaName, sizeof ( env->areaName ) - 1 ) || strncmp ( appName, env->appName, sizeof ( env->appName ) - 1 ) ) {
				if ( !BuildAppAreaField ( header->sizes, appName, areaName, false ) )
					return false;
			}
		}

		len = sizeof ( ShortMsgHeader ) + msg->header.sizes [ 0 ] + msg->header.sizes [ 1 ];

		memcpy ( ( ( char * ) msg ) + len, sendBuffer, bufferLength );

		len += bufferLength;
		( ( char * ) msg ) [ len ] = 0;

		header->length = len;

		if ( env->useCLSForDevices ) {
			unsigned int aesLen = AES_SHA256_KEY_LENGTH * 2;

			char aesBlob [ AES_SHA256_KEY_LENGTH * 2 ];

			unsigned int * pUI = ( unsigned int * ) aesBlob;
			for ( unsigned int i=0; i<( aesLen / 4 ); i++ ) {
				*pUI = rand (); pUI++;
			}
            
			aes.deviceID = deviceID;
			if ( !AESDeriveKeyContext ( aesBlob, aesLen, &aes ) ) {
				CErrID ( "SendMessageToDevice: Failed to derive session keys." ); goto Finish;
			}
			int rc = SecureChannelEstablish ( env, deviceID, sock, &aes, aesBlob );
			if ( !rc )
				goto Finish;

			if ( rc > 0 ) {
				if ( !AESEncrypt ( &aes, ( char * ) msg, &len, &cipher ) )
					goto Finish;
				free ( msg );
				msg = ( ShortMsg * ) cipher;
			}
		}

		CVerbVerbArgID ( "SendMessageToDevice: [ %s ]", ( char * ) sendBuffer );

		bytesSent = ( int ) send ( sock, ( const char * ) msg, len, MSG_NOSIGNAL );
		if ( bytesSent != ( int ) len ) {
			CErrArgID ( "SendMessageToDevice: Failed to send message! Sent bytes and length of packet mismatch [ %i != %u ]",
                       bytesSent, len );
            
            LogSocketErrorF ( "DeviceBase.SendMessageToDevice" );
		}
		else {
			success = true;
			//onEnvironsNotifier1 ( env, deviceID, areaName, appName, NOTIFY_SHORT_MESSAGE_ACK, SOURCE_NATIVE );
		}

	Finish:
        if ( sock != -1 ) {
			ShutdownCloseSocket ( sock, true, "DeviceBase SendMessageToDevice" );
		}

        AESDisposeKeyContext ( &aes );
        
        free_n ( msg );

		return success;
	}


	void * DeviceBase::InteractListenerStarter ( void * arg )
	{
        DeviceBase * device = ( DeviceBase * ) arg;

		if ( device->deviceStatus != DeviceStatus::Deleteable )
			device->InteractListener ();

        device->interactThread.Notify ( "InteractListenerStarter" );
        
        device->interactThread.Detach ( "InteractListenerStarter" );

		UnlockDevice ( device );
        return 0;
	}


	/*void DeviceBase::CloseInteractListener ( bool wait )
	{
		CVerbID ( "ClosePortalListener" );

		CloseListener ( deviceID, interactSocket, interactSocketForClose, &interactThread, "InteractListener", &interactSocketLock, wait );
	}*/


	void DeviceBase::CloseUdpListener ( bool wait )
	{
		CVerbID ( "CloseUdpListener" );
        
        int sock = udpSocket;
        
		if ( IsValidFD ( sock ) ) {
            udpSocket = INVALID_FD;
            
            SetNonBlockSocket ( sock, true, "CloseUdpListener" );
            
            SendUdpFIN ( sock );
            
            Mediator::UnConnectUDP ( sock );
            
            MediatorClient::SendUDPFIN ( sock );
            
            CSocketTraceUpdate ( sock, "Shutdown 1 in CloseUdpListener" );
			shutdown ( sock, 1 ); // SHUT_WR
		}

		if ( wait ) {
            sock = udpSocketForClose;
            
            if ( IsValidFD ( sock ) ) {
                Mediator::UnConnectUDP ( sock );
                
                SetNonBlockSocket ( sock, true, "CloseUdpListener" );
                
                MediatorClient::SendUDPFIN ( sock );
                
                CSocketTraceUpdate ( sock, "Shutdown 2 in CloseUdpListener" );
                shutdown ( sock, 2 );
			}

            udpThread.Join ( "UdpListener" );
		}
	}


	bool DeviceBase::EstablishInteractChannel ()
	{
		CVerbID ( "EstablishInteractChannel" );

		if ( IsInvalidFD ( interactSocket ) ) {
			// We are the initiator and need to establish a socket
			if ( behindNAT ) {
				CVerbID ( "EstablishInteractChannel: Connecting stunt ..." );
                
                if ( !StunTRequest::EstablishRequest ( env, this, MEDIATOR_STUNT_CHANNEL_MAIN, connectToken ) )
				{
					if ( deviceStatus != DeviceStatus::Deleteable ) {
						CErrID ( "EstablishInteractChannel: Failed to create STUNT socket" );
					}
					return false;
				}
				comPort = ntohs ( interactAddr.sin_port );
			}
			else {
				if ( !AllocateInteractSocket () )
					return false;

				if ( !LockAcquireA ( portalMutex, "EstablishInteractChannel" ) )
					return false;

				// Connect to device
				CVerbID ( "EstablishInteractChannel: Connecting ..." );
				int rc = Mediator::Connect ( deviceID, interactSocket, ( struct sockaddr * )&interactAddr, 0, "Interact channel" );

				if ( !LockReleaseA ( portalMutex, "EstablishInteractChannel" ) )
					return false;

				if ( rc != 0 ) {
                    CErrArgID ( "EstablishInteractChannel: Failed to connect [ %s : %d ]", inet_ntoa ( interactAddr.sin_addr ), ntohs ( interactAddr.sin_port ) );
                    
                    if ( IsValidFD ( interactSocket ) ) { LogSocketErrorF ( "DeviceBase.EstablishInteractChannel" ); }
					return false;
				}
			}

			UpdateConnectStatus ( 10 );
            
			onEnvironsNotifier1 ( env, deviceNode->info.objID, NOTIFY_CONNECTION_MAIN_NEW );

			if ( !InitiateInteractChannel () ) {
				if ( env->environsState >= environs::Status::Starting ) {
					CErrID ( "EstablishInteractChannel: Failed to handshake and initiate interact channel!" );
				}
				return false;
			}

			if ( SendBuffer ( false, MSG_TYPE_HELO, 0, 0, MSG_HANDSHAKE_SUCCESS, &env->deviceID, sizeof ( env->deviceID ) ) != sizeof ( env->deviceID ) ) {
				CErrID ( "EstablishInteractChannel: Failed to send ack message to device!" );
				return false;
			}

			if ( !InitUdpChannel () )
				return false;
		}
        else {
			if ( stuntRedundant )
            {
#ifdef ENABLE_DEVICEBASE_WP_STUNT
				if ( !stuntInteract.expired () )
					CloseStunt ( stuntInteract, true, &stuntInteractState, "iact" );
#else
				bool closeStunt = false;

				LockAcquireVA ( spLock, "EstablishInteractChannel" );

				closeStunt = ( stuntInteract != 0 );

				LockReleaseVA ( spLock, "EstablishInteractChannel" );

                CVerbID ( "EstablishInteractChannel: Closing ongoing stunt request." );
                
				if ( closeStunt )
					CloseStunt ( stuntInteract, true, &stuntInteractState, "iact" );
#endif
			}

			UpdateConnectStatus ( 10 );

			onEnvironsNotifier1 ( env, deviceNode->info.objID, NOTIFY_CONNECTION_MAIN_NEW );

			// We are the responder and were started by a socket created by a (yet anonymous request)

			//
			// Send device configuration in response to a helo packet to requesting device
			//
			if ( !SendDeviceConfig () ) {
				CErrID ( "EstablishInteractChannel: Failed to send device configuration" );
				return false;
			}
		}
        
		TuneReceiveBuffer ( interactSocket );

		return true;
	}


	__forceinline void DeviceBase::EnqueueToReceiverStream ( void * payload, unsigned int size, unsigned short type, int portalID )
	{
		CVerbVerbID ( "EnqueueToReceiverStream" );

		PortalDevice * portalDevice = GetPortalDeviceAccess ( portalID );
		if ( !portalDevice )
			return;

		PortalReceiver * receiver = portalDevice->receiver;

		if ( receiver && receiver->stream )
		{
#ifndef ENABLE_PORTAL_STALL_MECHS
			receiver->stream->PushStreamPacket ( payload, size, type );
#else
			if ( !receiver->stream->PushStreamPacket ( payload, size, type ) )
			{
				if ( !receiver->stream->stalled )
				{
					// Notify buffer full to sender
					if ( !SendPortalMessage ( MSG_PORTAL_BUFFER_FULL, portalID ) ) {
						CErrID ( "EnqueueToReceiverStream: Failed to send buffer full message!" );
					}
					else receiver->stream->stalled = true;
				}
			}
			else if ( receiver->stream->stalled )
			{
				if ( !SendPortalMessage ( MSG_PORTAL_BUFFER_AVAIL_AGAIN, portalID ) ) {
					CErrID ( "EnqueueToReceiverStream: Failed to send buffer available message!" );
				}
				else receiver->stream->stalled = false;
			}
#endif
		}

		ReleasePortalDevice ( portalDevice );
	}

    
    void * DeviceBase::InteractListener ()
    {
        CVerbID ( "InteractListener: Working thread started ..." );
        
        pthread_setname_current_envthread ( "DeviceBase.InteractListener" );
        
        int     		bytesRead;
        
        ComMessageHeader * header;
        
        char        * 	msg_Start;
        char		*	msg_BufferEnd;
        char        * 	msg_CurrentEnd;
        
        char		*	decrypted	= 0;
        unsigned int	packLen		= 0;
        
        int             currentLength	= 0;
        bool            header_available = false;
        bool            doReset			= false;
        unsigned int    msg_Length 		= 0;
        
        int				remainBufferSize;
        
        if ( !interactBuffer ) {
            interactBuffer = ( char * ) malloc ( interactBufferSize );
            
            if ( !interactBuffer )
                goto EndWithValue;
        }
        
        if ( interactThreadState == 0 )
        {
            if ( !EstablishInteractChannel () )
            {
                UpdateConnectStatus ( -20 );
                
                onEnvironsNotifier1 ( env, deviceNode->info.objID, NOTIFY_CONNECTION_MAIN_FAILED );
                goto EndWithValue;
            }
            
            interactThreadState = 3; // Channel established and authenticated; Thread shall run in active state.
        }
        
        msg_Start 		= interactBuffer;
        msg_BufferEnd	= msg_Start + interactBufferSize;
        msg_CurrentEnd 	= interactBuffer;
        
        header			= ( ComMessageHeader * ) msg_Start;
        
        IncreaseThreadPriority ( "InteractListener" );
        
        while ( deviceStatus != DeviceStatus::Deleteable && interactThreadState == 3 )
        {
            if ( doReset ) {
                CVerbVerbID ( "InteractListener: doReset" );
                msg_Length      = currentLength         = 0;
                msg_CurrentEnd  = msg_Start             = interactBuffer;
                doReset         = header_available      = false;
                header          = ( ComMessageHeader * ) msg_Start;
            }
            
            remainBufferSize = ( int ) ( msg_BufferEnd - msg_CurrentEnd );
            CVerbVerbArgID ( "InteractListener: remain buffer size [%i]", remainBufferSize );
            
            if ( ( ( unsigned ) remainBufferSize < msg_Length ) && ( msg_Start != interactBuffer ) ) {
                refactorMallocBuffer ( msg_Start, header, interactBuffer, currentLength, msg_CurrentEnd );
                
                remainBufferSize = ( int ) ( msg_BufferEnd - msg_CurrentEnd );
            }
            
            if ( interactBufferSize < msg_Length )
            {
                if ( msg_Length > MAX_TCP_SEND_PACKET_SIZE ) {
                    CErrArgID ( "InteractListener: Protocol error - message length [ %i bytes ] exceeds the max allowed size [ %i ]! Flushing whole stream!",
                               msg_Length, MAX_TCP_SEND_PACKET_SIZE );
                    doReset = true;
                    continue;
                }
                
                CVerbArgID ( "InteractListener: message length %u, capacity = %u", msg_Length, interactBufferSize );
                
                if ( !relocateMallocBuffer ( msg_Start, msg_BufferEnd, header, interactBuffer, interactBufferSize,
                                            msg_Length, currentLength, msg_CurrentEnd ) )
                    break;
                
                remainBufferSize = ( int ) ( msg_BufferEnd - msg_CurrentEnd );
            }
            
            bytesRead = ( int ) recv ( interactSocket, msg_CurrentEnd, remainBufferSize, 0 );
            
            CVerbVerbArgID ( "InteractListener: bytes received %i", bytesRead );
            
            if ( bytesRead <= 0 ) {
                // 0 means that the socket has closed, so we don't need the listener anymore
                CVerbID ( "InteractListener: Socket to device has been closed!!" );
                
                interactThreadState = 1;
                break;
            }
            
            msg_CurrentEnd += bytesRead;
            currentLength += bytesRead;
            
        HandleMessage:
            // Check whether the MSG preamble is present
            if ( currentLength < 13 ) {
                // Message incomplete; Continue receiving
                CWarnArgID ( "InteractListener: Message received is incomplete; currentLength [%i] - bytesRead [%d].", currentLength, ( int ) bytesRead );
                continue;
            }
            CVerbVerbArgID ( "InteractListener: currentLength %i", currentLength );
            
            if ( !header_available )
            {
                if ( msg_Start [ 0 ] != 'M' ||  msg_Start [ 1 ] != 'S' || msg_Start [ 2 ] != 'G' || msg_Start [ 3 ] != ';' )
                {
                    if ( aes.decCtx ) {
                        packLen				= *( ( unsigned int * ) msg_Start );
                        unsigned int flags	= 0xF0000000 & packLen;
                        msg_Length			= packLen & 0xFFFFFFF;
                        
                        if ( flags & 0x40000000 ) { /// Encrypted package
                            packLen = msg_Length;
                            
                            if ( packLen > ( unsigned ) currentLength ) {
                                // We haven't received the whole message! go on receiving...
                                CVerbVerbArgID ( "InteractListener: encrypted message [%i] is longer than currentLength [%i]!", packLen, currentLength );
                                continue;
                            }
                            
                            if ( AESDecrypt ( &aes, msg_Start, &packLen, &decrypted ) )
                            {
                                if ( decrypted [ 0 ] == 'M' && decrypted [ 1 ] == 'S' && decrypted [ 2 ] == 'G' && decrypted [ 3 ] == ';' )
                                {
                                    header = ( ComMessageHeader * ) decrypted;
                                    goto HandleHeader;
                                }
                                else {
                                    if ( IsFINMessage ( decrypted ) ) {
                                        CVerb ( "InteractListener: FIN!" );
                                        break;
                                    }
                                    
                                    CErr ( "InteractListener: MSG preamble is missing in decrypted message!" );
                                    
                                    CVerbVerbArg ( "InteractListener: ciphers [%s]", ConvertToHexSpaceString ( decrypted, packLen ) );
                                    
                                    free ( decrypted );
                                    decrypted = 0;
                                }
                            }
                        }
                        
                        if ( IsFINMessage ( msg_Start ) ) {
                            CVerb ( "InteractListener: FIN!" );
                            break;
                        }
                    }
                    
                    CErrArgID ( "InteractListener: Protocol error - message header is missing! Flushing whole stream! packLen [%i], currentLength [%i], bytesRead [%i], remainBufferSize [%i], tcpReceiveCapacity [%i]",
                               packLen, currentLength, ( int ) bytesRead, ( int ) remainBufferSize, interactBufferSize );
                    
                    
                    CVerbVerbArg ( "InteractListener: ciphers [%s]", ConvertToHexSpaceString ( msg_Start, msg_Length ) );
                    
                    doReset = true;
                    continue;
                }
                else {
                    if ( IsFINMessage ( msg_Start ) ) {
                        CVerb ( "InteractListener: FIN!" );
                        break;
                    }
                    
                    header		= ( ComMessageHeader * ) msg_Start;
                    msg_Length	= header->length;
                }
                
            HandleHeader:
                // Get size of message
                packLen = header->length;
                if ( packLen > 2000000 ) {
                    CErrArgID ( "InteractListener: Invalid message format. Message length in header %u > 2000000!", packLen );
                    break;
                }
                
                CVerbVerbArgID ( "InteractListener: length of message %i", packLen );
                header_available = true;
            }
            
            if ( packLen > ( unsigned ) currentLength ) {
                // We haven't received the whole message! go on receiving...
                CVerbVerbArgID ( "InteractListener: message [%i] is longer than currentLength [%i]!", packLen, currentLength );
                continue;
            }
            
            
            unsigned int msgType = ( unsigned int ) header->type;
            
            if ( msgType < MSG_TYPE_MAX_COUNT ) {
                header->length = packLen; // Make sure that the handler works with the actual length
                
                if ( !( ( *this.*msgHandlers [ msgType ] ) ( header, false ) ) )
                    goto EndWithValue;
            }
            
            free_m ( decrypted );
            
            // Is there a message pending in stream?
            currentLength -= msg_Length;
            if ( currentLength == 0 ) {
                doReset = true;
                continue;
            }
            else if ( currentLength < 0 ) {
                CErrArgID ( "InteractListener: remainging length of message is < 0 [%i]!", currentLength );
                doReset = true;
                continue;
            }
            
            // Continue with next message in stream
            header_available = false;
            msg_Start       += msg_Length;
            header          = ( ComMessageHeader * ) msg_Start;
            goto HandleMessage;
        }
        
    EndWithValue:
        if ( interactThreadState <= 1 )
        {
            CVerbID ( "InteractListener: Setting deviceStatus to Deleteable" );
            
            // We make this reference in order to make sure that the compiler does not optimize our SP away...
            deviceStatus = DeviceStatus::Deleteable;
            
            OnInteractListenerClosed ();
            
            UpdateConnectStatus ( -20 );
            
            onEnvironsNotifier1 ( env, deviceNode->info.objID, NOTIFY_CONNECTION_MAIN_CLOSED );
            
            TriggerCleanUpDevices ();
        }
        
        free_n ( decrypted );
        
        CVerbID ( "InteractListener: bye bye..." );
        
        return NULL;
    }
    

	void * DeviceBase::UdpListenerStarter ( void * arg )
    {
        DeviceBase * device = ( DeviceBase * ) arg;

		if ( device->deviceStatus != DeviceStatus::Deleteable )
			device->UdpListener ();

        device->udpThread.Notify ( "UdpListenerStarter" );
        
        device->udpThread.Detach ( "UdpListenerStarter" );

		UnlockDevice ( device );
        return 0;
	}


	void * DeviceBase::UdpListener ()
	{
		CVerbID ( "UdpListener: Working thread started ..." );

        pthread_setname_current_envthread ( "DeviceBase::UdpListener" );
        
		char * buffer = ( char * ) malloc ( UDP_DEVICEBASE_MAX_SIZE );
		if ( !buffer ) {
			CErrID ( "UdpListener: Failed to allocate receive buffer." );
			return 0;
		}

		unsigned int * fragmentBits	= 0;
		int fragmentApartments 		= 1;
		int fragmentsReceived 		= 0;
		int seq 					= -1;
		int rc						= 0;

		ByteBuffer * streamBuffer 	= 0;
		UdpMessageHeader * header 	= reinterpret_cast<UdpMessageHeader *>( buffer );

#ifdef DISPLAYDEVICE
		DeviceController * device	= ( DeviceController * ) this;
#endif

		struct 	sockaddr_in			addr;
		Zero ( addr );

		addr.sin_family 			= PF_INET;
		socklen_t addrSize 			= sizeof ( addr );

		unsigned int * pID			= reinterpret_cast<unsigned int *>( buffer + 4 );

		while ( deviceStatus != DeviceStatus::Deleteable )
		{
            if ( IsInvalidFD ( udpSocket ) )
                break;
            
            int bytesReceived = ( int ) recvfrom ( udpSocket, buffer, UDP_DEVICEBASE_MAX_SIZE, 0, ( struct sockaddr* )&addr, &addrSize );
            if ( bytesReceived <= 0 ) {
				CVerbArgID ( "UdpListener: Socket [ %i ] has been closed.", bytesReceived ); //LogSocketError ( );
				break;
			}
			CVerbVerbArg ( "UdpListener [ %s : %d : %i ]: Received [ %i ] bytes", inet_ntoa ( addr.sin_addr ), ntohs ( addr.sin_port ), deviceNode ? deviceNode->info.objID : 0, bytesReceived );

			switch ( buffer [ 0 ] )
			{
#ifdef DISPLAYDEVICE
			case 't':
				if ( buffer [ 1 ] == 'f' && buffer [ 2 ] == ':' )
				{
					if ( buffer [ 3 ] >= UDP_MSG_PROTOCOL_VERSION )
						device->HandleTouchPacket ( buffer, bytesReceived );
				}
				break;

			case 's':
				if ( buffer [ 1 ] == 'f' && buffer [ 2 ] == ':' )
				{
					if ( buffer [ 3 ] >= UDP_MSG_PROTOCOL_VERSION )
						device->HandleSensorPacket ( buffer, bytesReceived );
				}
				break;
#endif
            case 'c':
                if ( buffer [ 1 ] == 'd' && buffer [ 2 ] == ':' )
                {
                    sp ( DeviceInstanceNode ) node = deviceNode;
                    if ( node ) {
                        onEnvironsSensor ( env, node->info.objID, (environs::lib::SensorFrame *) buffer, bytesReceived );
                    }
                }
                break;

			case 'i':
				if ( buffer [ 1 ] == 'd' )
				{
					if ( buffer [ 2 ] == ':' )
					{
						CVerbID ( "UdpListener: Handshake helo IDENT received" );
						// This msg is sent by Core.UdpAcceptor through the socket of the particular device

						int ID = *pID;
						if ( ID == deviceID )
                        {
							if ( deviceStatus > DeviceStatus::Deleteable )
							{
								if ( udpCoreConnected ) {
									addr.sin_addr = udpAddr.sin_addr;
									addr.sin_port = udpAddr.sin_port;
								}

								if ( !( activityStatus & DEVICE_ACTIVITY_UDP_CONNECTED ) )
								{
									// Connect to requestor
									CVerbArgID ( "UdpListener: Connecting to [ %s : %d ]", inet_ntoa ( addr.sin_addr ), ntohs ( addr.sin_port ) );

									if ( IsInvalidFD ( udpSocket ) )
                                        break;
                                    
                                    CSocketTraceUpdate ( udpSocket, "Connect in UdpListener" );
                                    
									rc = ::connect ( udpSocket, ( struct sockaddr * ) &addr, sizeof ( addr ) );
									if ( rc < 0 ) {
										CWarnArgID ( "UdpListener: Failed connecting to [ %s : %d ]", inet_ntoa ( addr.sin_addr ), ntohs ( addr.sin_port ) );
									}
									else {
										CLogsArgID ( 2, "UdpListener: Connected to [ %s : %d ]", inet_ntoa ( addr.sin_addr ), ntohs ( addr.sin_port ) );
										OnUdpConnectionEstablished ();
									}
								}

#ifdef ENABLE_DEVICEBASE_WP_STUN
								sp ( StunRequest ) req = stun.lock ();
#else
								LockAcquireVA ( spLock, "UdpListener" );

                                sp ( StunRequest ) req = stun;

								LockReleaseVA ( spLock, "UdpListener" );
#endif
								if ( req && !udpCoreConnected )
								{
									if ( req->isRequestor ) {
										req->doHandshake = false;
										CLogArgID ( "UdpListener: Handshake with stun device [ 0x%X ] successfull", ID );
									}
									else {
										memcpy ( &udpAddr, &addr, sizeof ( udpAddr ) );
										CVerbArgID ( "UdpListener: Taken over stun udp [ %s : %d ]", inet_ntoa ( addr.sin_addr ), ntohs ( addr.sin_port ) );

										CVerbArgID ( "UdpListener: Handshake with stun device [ 0x%X ] successfull", ID );
									}
								}
							}

							buffer [ 0 ] = 'y'; buffer [ 1 ] = ';'; buffer [ 2 ] = ':'; buffer [ 3 ] = ';';
							*pID = env->deviceID;

							if ( IsInvalidFD ( udpSocket ) )
								break;
							if ( send ( udpSocket, buffer, 8, 0 ) == 8 )
								send ( udpSocket, buffer, 8, 0 );
						}
						else {
							CWarnArgID ( "UdpListener: Handshake (:) with unknown device [ 0x%X ] ignored", ID );
							//const char * dump = ConvertToHexString ( buffer, 24 );
							//CLogArg ( "UdpListener: dump [%s]", dump );
						}
					}
					else if ( buffer [ 2 ] == '.' )
					{
						CVerbID ( "UdpListener: handshake ACK received" );

						int ID = *pID;
						if ( ID == deviceID )
						{
							memcpy ( &udpAddr, &addr, sizeof ( udpAddr ) );
							CVerbArgID ( "UdpListener: taken over udp [ %s : %d ]", inet_ntoa ( addr.sin_addr ), ntohs ( addr.sin_port ) );

							if ( !( activityStatus & DEVICE_ACTIVITY_UDP_CONNECTED ) ) {
								OnUdpConnectionEstablished ();
							}
						}
						else {
							CWarnArgID ( "UdpListener: Handshake (.) with unknown device [ 0x%X ] ignored", ID );
						}
					}
				}
				break;

			case 'y':
				if ( buffer [ 1 ] == ';' && buffer [ 2 ] == ':' )
				{
					if ( buffer [ 3 ] == ';' )
                    {
						if ( !( activityStatus & DEVICE_ACTIVITY_UDP_CONNECTED ) )
						{
							if ( IsInvalidFD ( udpSocket ) )
                                break;
                            
                            CSocketTraceUpdate ( udpSocket, "Connect in UdpListener" );
                            
							// Connect to recipient (we are the requestor)
							rc = ::connect ( udpSocket, ( struct sockaddr * ) &addr, sizeof ( addr ) );
							if ( rc < 0 ) {
								CWarnArgID ( "UdpListener: Failed connecting to [ %s : %d ]", inet_ntoa ( addr.sin_addr ), ntohs ( addr.sin_port ) );
							}
							else {
								CLogsArgID ( 2, "UdpListener: Connected to [ %s : %d ]", inet_ntoa ( addr.sin_addr ), ntohs ( addr.sin_port ) );
								OnUdpConnectionEstablished ();
							}
                        }

#ifdef ENABLE_DEVICEBASE_WP_STUN
						sp ( StunRequest ) req = stun.lock ();
#else
						LockAcquireVA ( spLock, "UdpListener" );

						sp ( StunRequest ) req = stun;

						LockReleaseVA ( spLock, "UdpListener" );
#endif
						if ( req ) {
							if ( req->doHandshake ) {
								req->doHandshake = false;
								CVerbArgID ( "UdpListener: Handshake with device [ 0x%X ] successfull", *pID );
							}
						}

						buffer [ 3 ] = ':';
						*pID = env->deviceID;
						if ( IsInvalidFD ( udpSocket ) )
							break;
						send ( udpSocket, buffer, 8, 0 );                        
					}
					else if ( buffer [ 3 ] == ':' )
					{
						CVerbArgID ( "UdpListener: Handshake with device [ 0x%X ] success ACKed", *pID );
					}
				}
				else if ( buffer [ 1 ] == ';' && buffer [ 2 ] == ';' )
				{
					if ( buffer [ 3 ] == '-' ) {
						CVerbArgID ( "UdpListener: Received STUN response from mediator [ %s ]", inet_ntoa ( addr.sin_addr ) );
						break;
					}

					// An incoming connection needs to be processed.
					CVerbArgID ( "UdpListener: Received handshake ACK from [ %s ]'s acceptor.", inet_ntoa ( addr.sin_addr ) );
				}
				break;

				//
				// Otherwise threat as stream packet
				//
			default:
			{
				if ( buffer [ 0 ] == 'F' && buffer [ 1 ] == 'I' && buffer [ 2 ] == 'N' && buffer [ 3 ] == ';' )
				{
					CVerb ( "UdpListener: FIN!" );
					break;
				}

				if ( bytesReceived < ( signed ) PKT_HEADER_SIZE ) {
					// Packet does not contain at least the preamble!!!
					continue;
				}

				if ( header->version < 1 )
					continue;

				//unsigned short fragmentSize = *((unsigned short*) (receiveBuffer + 2));

				int sequence = header->sequence;
				if ( sequence < seq )
					continue;
				//CLogArg ( "UdpListener: sequence %i", sequence );

				int fileSize = header->fileSize;
				// Skip sizes over 2 MB
				if ( fileSize > ( DATA_BUFFER_SIZE - 4 ) )
					continue;
				//CLogArg ( "UdpListener: fileSize %i", fileSize );

				int fragments = header->fragments;
				if ( sequence > seq )
				{
					// A newer image was transmitted
					seq = sequence;

					// Initialize buffers and counters
					fragmentsReceived = 0;

					fragmentApartments = fragments / 32;

					int bitsLast = fragments - ( fragmentApartments * 32 );
					fragmentApartments++;
                    
                    free_n ( fragmentBits );

					int bytes = fragmentApartments * 4;

					fragmentBits = ( unsigned int* ) calloc ( bytes, 1 );
                    if ( !fragmentBits )
                        break;

					// prepare the last apartment
					fragmentBits [ fragmentApartments - 1 ] = ( 0xFFFFFFFF >> bitsLast );
				}

				int fragment = header->fragment; // *((int*) (receiveBuffer + 16));
												 //CLogArg ( "UdpListener: fragment %i/%i", fragment, fragments );
				if ( fragment >= fragments )
					continue;

				fragmentsReceived++;

				//CInfoArg ( "UdpListener: packets(%li bytes); seq(%i); frag(%i/%i)", bytesReceived, sequence, fragment, fragments );

				if ( !streamBuffer ) {
					//CLog ( "UdpListener: getNextStreamBuffer" );
					//WARNING ( "DeviceBase::UdpListener: Implementation needs to be tested." )

					PortalDevice * portalDevice = GetPortalDeviceAccess ( header->portalID );

					if ( portalDevice ) {
						if ( portalDevice->receiver )
							streamBuffer = portalDevice->receiver->stream->GetNextStreamBuffer ( ( unsigned int ) fileSize );

						ReleasePortalDevice ( portalDevice );
					}
                }

                if ( streamBuffer ) {
                    //CInfo ( "DataListener: writing filesize" );
                    memcpy ( BYTEBUFFER_DATA_POINTER_START ( streamBuffer ), &header->payload, bytesReceived - PKT_HEADER_SIZE );
                }


                if ( fragmentBits ) {
                    int apartment = fragment / 32;

                    fragmentBits [ apartment ] |= ( 0x80000000 >> ( fragment - ( apartment * 32 ) ) );

                    if ( fragmentsReceived >= fragments ) {
                        int i = 0;
                        for ( ; i<fragmentApartments; i++ ) {
                            if ( fragmentBits [ i ] != 0xFFFFFFFF )
                                break;
                        }

                        if ( i == fragmentApartments )
                        {
                            //WARNING ( "DeviceBase::UdpListener: Implementation needs to be tested." )

                            PortalDevice * portalDevice = GetPortalDeviceAccess ( header->portalID );

                            if ( portalDevice ) {
                                if ( portalDevice->receiver )
                                    portalDevice->receiver->stream->PushStreamBuffer ( streamBuffer, fileSize, header->payloadType );

                                ReleasePortalDevice ( portalDevice );
                            }

                            streamBuffer = 0;

                            CVerbArg ( "UdpListener: image completely received (%i);", ( sequence + 1 ) );
                        }
                    }
                }
                }
                break;
            }
        }

		free ( buffer );

		UpdateConnectStatus ( -10 );

		onEnvironsNotifier1 ( env, deviceNode->info.objID, NOTIFY_CONNECTION_DATA_CLOSED );

		CVerbID ( "UdpListener: bye bye..." );
		return 0;
    }
    
    
    void DeviceBase::HandleSensorPacket ( char * buffer, int length )
    {
        if ( deviceStatus != DeviceStatus::Connected || !buffer )
            return;
        
        environs::lib::SensorFrame * frame = ( environs::lib::SensorFrame *) buffer;
        
        if ( frame->seqNumber && frame->seqNumber <= lastSensorFrameSeqNr )
            return;
        
        lastSensorFrameSeqNr = frame->seqNumber;
        
        CVerbVerbArg ( "HandleSensorPacket: New packet [%i]", frame->type );
        
        sp ( DeviceInstanceNode ) node = deviceNode;
        
        if ( node ) {
            CVerbVerbArg ( "HandleSensorPacket: ObjID [ 0x%X ] New packet [%i] x [%.2f] y [%.2f] z [%.2f]", node->info.objID, frame->type, frame->data.floats.f1, frame->data.floats.f2, frame->data.floats.f3 );
            //			CLogArg ( "HandleSensorPacket: ObjID [ 0x%X ] New packet [%i] x [%.2f] y [%.2f] z [%.2f]", node->info.objID, frame->type, frame->data.floats.f1, frame->data.floats.f2, frame->data.floats.f3 );
            
            onEnvironsSensor ( env, node->info.objID, frame, length );
        }
    }


	void DeviceBase::OnInteractListenerClosed ()
	{
		CVerbID ( "OnInteractListenerClosed" );

		UpdateConnectStatus ( 2, true );

		onEnvironsNotifier1 ( env, deviceNode->info.objID, NOTIFY_CONNECTION_CLOSED );
	}


	void DeviceBase::OnConnectionEstablished ()
	{
		CVerbID ( "OnConnectionEstablished" );

        if ( IsInvalidFD ( interactSocket ) || IsInvalidFD ( comDatSocket ) ) {
            CVerbArgID ( "OnConnectionEstablished: Socket interact [ %i ] comDat [ %i ]", interactSocket, comDatSocket );
			return;
        }

		if ( !LockAcquireA ( portalMutex, "OnConnectionEstablished" ) )
			return;

		unsigned int check = DEVICE_ACTIVITY_MAIN_CONNECTED | DEVICE_ACTIVITY_COMDAT_CONNECTED;
        
        CVerbArgID ( "OnConnectionEstablished: activityStatus [ %X ]", activityStatus );
        
		if ( activityStatus & DEVICE_ACTIVITY_CONNECTED )
			goto Finish;

        if ( ( activityStatus & check ) != check ) {
			goto Finish;
        }

		activityStatus |= DEVICE_ACTIVITY_CONNECTED;

		CVerbID ( "OnConnectionEstablished: Connected." );

		CLogsArgID ( 2, "OnConnectionEstablished: UDP %sconnected", ( activityStatus & DEVICE_ACTIVITY_UDP_CONNECTED ) ? "" : "NOT " );

		if ( deviceStatus >= DeviceStatus::ConnectInProgress ) {
			CVerbID ( "OnConnectionEstablished: setting deviceStatus to Connected" );
			deviceStatus = DeviceStatus::Connected;
            
            sp ( MediatorClient ) mediator = env->mediator MED_WP;
			if ( mediator ) {
				mediator->UpdateDeviceState ( this, nativeID );

				mediator->RemoveStuntDevice ( this );
			}
            
            const char * devName = "Unknown";
            
            sp ( DeviceInstanceNode ) node = deviceNode;
            if ( node ) {
                devName = node->info.deviceName;
            }
            
            CLogArgID ( "OnConnectionEstablished: %s connected to [ %s : %s ] [ %s : %d : %s ]", (encrypt && aes.encCtx ? "Secure" : "Unsecure"), deviceAppName ? deviceAppName : env->appName, deviceAreaName ? deviceAreaName : env->areaName, inet_ntoa ( interactAddr.sin_addr ), ntohs ( interactAddr.sin_port ), devName );

            connectThread.Notify ( "OnConnectionEstablished" );
		}
		else {
			/// Both connected, but we're not in connect in progress anymore
			deviceStatus = DeviceStatus::Deleteable;

			TriggerCleanUpDevices ();
			goto Finish;
		}

		if ( pthread_mutex_unlock ( &portalMutex ) ) {
			CErrID ( "OnConnectionEstablished: Failed to release object state mutex!" );
			return;
		}
        
        DismissOtherDevices ( env, nativeID, deviceID, deviceAreaName, deviceAppName );
        
		UpdateConnectStatus ( 100, true );

		onEnvironsNotifier1 ( env, deviceNode->info.objID, NOTIFY_CONNECTION_ESTABLISHED );
        
		SetEnvironsState ( env, environs::Status::Connected );

#ifdef DEVICEBASE_COMPATIBLE_CHANNEL
		if ( SendBuffer ( false, MSG_TYPE_HELO, 0, 0, MSG_HANDSHAKE_CONNECTED, 0, 0 ) != 0 ) {
			CErrID ( "OnConnectionEstablished: Failed to send connection OK ACK." );
			return;
        }
#endif
        if ( SendBuffer ( true, MSG_TYPE_HELO, 0, 0, MSG_HANDSHAKE_CONNECTED, 0, 0 ) != 0 ) {
            CErrID ( "OnConnectionEstablished: Failed to send connection OK ACK." );
            return;
        }
		return;

	Finish:
		LockReleaseVA ( portalMutex, "OnConnectionEstablished" );
	}


	bool DeviceBase::SaveToStorageMessages ( const char * prefix, const char * path, const char * message, int length )
	{
		CVerb ( "SaveToStorageMessages" );

		if ( !path || !prefix ) {
			CErr ( "SaveToStorageMessages: Invalid (NULL) path/prefix received!" );
			return false;
		}

		bool success = false;
		size_t written = 0;
		FILE *	file = 0;

#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable: 4996 )
#endif
		// Try it up to 1,2sec (messages store is subject to be opened often by multiple threads)
		int tries = 4;
		do {
			file = fopen ( path, "a" );
			if ( file )
				break;
			Sleep ( 400 );
			tries--;
		}
		while ( tries > 0 );

		if ( !file ) {
			CErrArg ( "SaveToStorageMessages: Failed to create file [ %s ] [ %i ]", path, errno );
			return false;
		}
#ifdef _WIN32
#pragma warning( pop )
#endif
		fseek ( file, 0, SEEK_END );

		char prefixStr [ 128 ];

		int toWriteLen = snprintf ( prefixStr, sizeof ( prefixStr ), "%s:%llu ", prefix, GetUnixEpoch () );
		if ( toWriteLen <= 0 || toWriteLen >= (int) sizeof ( prefixStr ) ) {
			CErr ( "SaveToStorageMessages: Failed to build prefix!" );
			goto Finish;
		}

		written = fwrite ( prefixStr, 1, toWriteLen, file );
		if ( ( int ) written != toWriteLen )
			goto Finish;

		toWriteLen = length;
		written = fwrite ( message, 1, length, file );
		if ( ( int ) written != length )
			goto Finish;
		
		toWriteLen = 1;
		written = fwrite ( "\n", 1, 1, file );
		if ( written == 1 )
			success = true;

	Finish:
		if ( !success ) {
			CErrArg ( "SaveToStorageMessages: Failed to write [ %d ] prefix. written [ %d ]", toWriteLen, written );
		}

		fclose ( file );
		return success;
	}
    

	bool DeviceBase::SaveToStorageMessages ( Instance * env, const char * prefix, int deviceID, const char * areaName, const char * appName, const char * message, int length )
	{
		CVerbID ( "SaveToStorageMessages" );

		if ( !message || length <= 0 || !env ) {
			CErrID ( "SaveToStorageMessages: Invalid message received!" );
			return false;
		}

		bool success				= false;

		char dataStore [ 1024 ];
		*dataStore = 0;

        char * pdataStore			= dataStore;
		char * dataStoreRequests    = 0;

		unsigned short dataStoreLen			= sizeof ( dataStore );
		unsigned short dataStoreRemainLen   = 0;

		// Make sure that the data path exist
		//
		if ( !InitDeviceStorage ( env, deviceID, areaName, appName, pdataStore, dataStoreLen, dataStoreRemainLen, dataStoreRequests, false, false ) || !*dataStore )
			return false;

		// Build path using the device identifier
		//
		int len = snprintf ( dataStore + dataStoreLen, dataStoreRemainLen, "receivedMessages.txt" );
		if ( len <= 0 ) {
			CErrID ( "SaveToStorageMessages: Failed to build path!" );
			goto Finish;
		}

		success = SaveToStorageMessages ( prefix, dataStore, message, length );

    Finish:
        //free_n ( dataStore );

		return success;
    }
    
    
    bool DeviceBase::PrepareStorage ()
    {
        if ( !dataStorePath || !dataStorePathRemainingLen )
        {
            if ( !LockAcquireA ( spLock, "PrepareStorage" ) )
                return false;
            
            if ( !dataStorePath || !dataStorePathRemainingLen )
                InitDeviceStorage ( env, deviceID, deviceAreaName, deviceAppName, dataStorePath, dataStorePathLen, dataStorePathRemainingLen, dataStorePathForRequests, true, true );
            
            if ( !LockReleaseA ( spLock, "PrepareStorage" ) )
                return false;
            
            if ( !dataStorePath || !dataStorePathRemainingLen ) {
                CErrID ( "PrepareStorage: Failed!" );
                return false;
            }
        }
        return true;
    }


	bool DeviceBase::SaveToStorageMessages ( const char * prefix, const char * message, int length )
	{
		CVerbID ( "SaveToStorageMessages" );

		if ( !message || length <= 0 ) {
			CErrID ( "SaveToStorageMessages: Invalid message received!" );
			return false;
		}

        if ( !PrepareStorage () )
            return false;

        if ( !LockAcquireA ( spLock, "SaveToStorageMessages" ) )
            return false;

        bool success = false;

		int len = snprintf ( dataStorePath + dataStorePathLen, dataStorePathRemainingLen, "receivedMessages.txt" );
		if ( len <= 0 ) {
			CErrID ( "SaveToStorageMessages: Failed to build path!" );
		}
        else
            success = SaveToStorageMessages ( prefix, dataStorePath, message, length );

        LockReleaseA ( spLock, "SaveToStorageMessages" );

        return success;
	}


	bool DeviceBase::SaveToStorageDescription ( int fileID, const char * descriptor, unsigned int length )
	{
		CVerbID ( "SaveToStorageDescription" );

		if ( !descriptor ) {
			CErrID ( "SaveToStorageDescription: Invalid descriptor received!" );
			return false;
        }

        if ( !LockAcquireA ( spLock, "SaveToStorageDescription" ) )
            return false;

        if ( !SaveToStorageBuildPath ( fileID, false ) ) {
            LockReleaseA ( spLock, "SaveToStorageDescription" );
            return false;
        }

        unsigned int    written = 0;
        FILE        *	file    = 0;

#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable: 4996 )
#endif
        //if ( DirectoryPathExist ( dataStorePath ) )
        unlink ( dataStorePath );

        file = fopen ( dataStorePath, "wb" );

        LockReleaseA ( spLock, "SaveToStorageDescription" );

        if ( !file ) {
            CErrArgID ( "SaveToStorageDescription: Failed to create file [ %s ]", dataStorePath );
            return false;
        }

        bool success = true;

#ifdef _WIN32
#pragma warning( pop )
#endif
        fseek ( file, 0, SEEK_SET );

        written = ( unsigned int ) fwrite ( descriptor, 1, length, file );
        if ( written != length ) {
            CErrArgID ( "SaveToStorageDescription: Failed to write [ %d ] bytes. written [ %d ]", length, written );
            success = false;
        }

        fclose ( file );

        return success;
	}


	bool DeviceBase::SaveToStorageBuildPath ( int fileID, bool isBin )
	{
		CVerbID ( "SaveToStorageBuildPath" );

		if ( !dataStorePath || !dataStorePathRemainingLen ) {
			CErrID ( "SaveToStorageBuildPath: Invalid dataStorePath or remaining path length!" );
			return false;
		}

		int len = snprintf ( dataStorePath + dataStorePathLen, dataStorePathRemainingLen, "%i.%s", fileID, isBin ? "bin" : "txt" );
		if ( len <= 0 ) {
			CErrArgID ( "SaveToStorageBuildPath: Failed to build path! [ fileID %i ]", fileID );
			return false;
		}

		CLogsArgID ( 2, "SaveToStorageBuildPath: [ %s ]", dataStorePath );
		return true;
	}


	bool DeviceBase::SaveToStorage ( int fileID, FILE ** fp, int pos, char * buffer, unsigned int length )
	{
		CVerbID ( "SaveToStorage" );

		if ( !buffer ) {
			CErrID ( "SaveToStorage: Invalid buffer received!" );
			return false;
		}

		unsigned int    written = 0;
		FILE    *       file    = 0;

		if ( !pos ) {
			if ( fp && *fp ) {
				fclose ( *fp );
				*fp = 0;
            }

            if ( !LockAcquireA ( spLock, "SaveToStorage" ) )
                return false;
            
            if ( !SaveToStorageBuildPath ( fileID, true ) ) {
                LockReleaseA ( spLock, "SaveToStorage" );
                return false;
            }
#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable: 4996 )
#endif
            // Delete an existing file if we have just started to put data parts into the file
            //if ( DirectoryPathExist ( dataStorePath ) )
                unlink ( dataStorePath );
            
            file = fopen ( dataStorePath, "wb" );

            LockReleaseA ( spLock, "SaveToStorage" );

            if ( !file ) {
                CErrArgID ( "SaveToStorage: Failed to create file [ %s ]", dataStorePath );

                if ( fp )
                    *fp = 0;
                return false;
            }
#ifdef _WIN32
#pragma warning( pop )
#endif
		}
        else if ( fp  ) {
			file = *fp;
            if ( !file )
                return false;
        }
        else
            return false;

		try
		{
			fseek ( file, pos, SEEK_SET );

			written = ( unsigned int ) fwrite ( buffer, 1, length, file );
			if ( written != length ) {
				CErrArgID ( "SaveToStorage: Failed to write [ %d ] bytes. written [ %d ]", length, written );
                fclose ( file );
                
                if ( fp )
                    *fp = 0;
				return false;
			}
		}
		catch ( ... ) {
            _EnvDebugBreak ( "SaveToStorage" );

			CErrArgID ( "SaveToStorage: Exception while writing at pos [ %d ] [ %d ] bytes. written [ %d ]", pos, length, written );
		}

		if ( fp ) {
			*fp = file;
		}
		else
			fclose ( file );
		return true;
	}


	bool DeviceBase::LoadFromStorage ( int fileID, char * buffer, int * capacity )
    {
        CVerbID ( "LoadFromStorage" );

		unsigned int	bytesRead   = 0;
        bool            success     = false;

		if ( !capacity ) {
			CErrID ( "LoadFromStorage: Invalid parameters given!" );
			return false;
        }

        if ( !LockAcquireA ( spLock, "LoadFromStorage" ) )
            return false;

        do
        {
            if ( !dataStorePathForRequests || !dataStorePathLen ) {
                CErrID ( "LoadFromStorage: Invalid data storage!" );
                break;
            }

            int len = snprintf ( dataStorePathForRequests + dataStorePathLen, dataStorePathRemainingLen, "%i.bin", fileID );
            if ( len <= 0 ) {
                CErrArgID ( "LoadFromStorage: Failed to build path! [ fileID %i ]", fileID );
                break;
            }

            // Get filesize
            STAT_STRUCT ( st );

            if ( stat ( dataStorePathForRequests, &st ) != 0 ) {
                CErrArgID ( "LoadFromStorage: Invalid data storage [ %s ]", dataStorePathForRequests );
                break;
            }

            if ( !buffer ) {
                CVerbArg ( "LoadFromStorage: Required size [ %u ]", st.st_size );
                *capacity = ( unsigned int ) st.st_size;
                success = true;
                break;
            }

            if ( *capacity < st.st_size ) {
                CErrArgID ( "LoadFromStorage: buffer too small [ %d ] for [ %s ]", *capacity, dataStorePathForRequests );
                break;
            }

#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable: 4996 )
#endif
            CVerbArgID ( "LoadFromStorage: Loading ... [ %s ]", dataStorePathForRequests );

            FILE *	file = fopen ( dataStorePathForRequests, "rb" );
            if ( !file ) {
                CErrArgID ( "LoadFromStorage: Failed to open file [ %s ]", dataStorePathForRequests );
                break;
            }
            
#ifdef _WIN32
#pragma warning( pop )
#endif
            LockReleaseA ( spLock, "LoadFromStorage" );
            
            bytesRead = ( unsigned int ) fread ( buffer, 1, ( size_t ) st.st_size, file );
            
            fclose ( file );
            if ( bytesRead != ( unsigned ) st.st_size ) {
                CErrArgID ( "LoadFromStorage: Read failed [ %u != %u ]", ( unsigned int ) bytesRead, ( unsigned int ) st.st_size );
                return false;
            }
            
            return true;
        }
        while ( false );

        LockReleaseA ( spLock, "LoadFromStorage" );
        return success;
    }
    
    
    void DeviceBase::ProccessPortalStartAckBase ( int portalID )
	{
		CVerbID ( "ProccessPortalStartAckBase" );

		ProccessPortalStartAck ( portalID );
	}

	void DeviceBase::ProccessPortalPauseAckBase ( int portalID )
	{
		CVerbID ( "ProccessPortalPauseAckBase" );

		ProccessPortalPauseAck ( portalID );
	}

	void DeviceBase::ProccessPortalStopAckBase ( int portalID )
	{
		CVerbID ( "ProccessPortalStopAckBase" );

		ProccessPortalStopAck ( portalID );
	}


	bool DeviceBase::ProvidePortal ( int portalID )
	{
		CLogArgID ( "ProvidePortal: [0x%X]", portalID );

		bool success = false;

		unsigned int id = PortalIndex ();

		if ( id >= MAX_PORTAL_STREAMS_A_DEVICE ) {
			/// Search for a free portal source slot
			for ( unsigned int i = 0; i < MAX_PORTAL_STREAMS_A_DEVICE; i++ ) {
				if ( !portalGenerators [ i ] ) {
					id = i;

					portalID &= 0xFFFFFF00;
					portalID |= id;
					break;
				}
			}
		}

		ClearPortalDir ();
		portalID |= PORTAL_DIR_OUTGOING;

		if ( id >= MAX_PORTAL_STREAMS_A_DEVICE ) {
			CWarnArgID ( "ProvidePortal: Max outgoing portals [%u] for this device exceeded.", MAX_PORTAL_STREAMS_A_DEVICE );
		}
		else {
			if ( LockAcquireA ( portalMutex, "ProvidePortal" ) )
			{
				// Acknowledge if we are initialized or already streaming the portal
                if ( portalGenerators [ id ] && portalGenerators [ id ]->status >= PortalSourceStatus::Initialized ) {
					if ( !SendPortalMessage ( MSG_PORTAL_PROVIDE_STREAM, portalID ) ) {
						CErrID ( "ProvidePortal: Failed to send portal request ACK message! (Portal is already initialized.)" );
					}
					else
						success = true;
					goto FinishInit;
				}

				if ( !CreatePortalGenerator ( portalID ) ) {
					CErrID ( "ProvidePortal: Failed to create PortalGenerator. Low memory?" );
					goto FinishInit;
				}

				if ( portalGenerators [ id ] )
					portalID = portalGenerators [ id ]->portalID;

				onEnvironsNotifier1 ( env, nativeID, NOTIFY_PORTAL_PROVIDER_READY, portalID );

				success = true;

			FinishInit:
				LockReleaseVA ( portalMutex, "ProvidePortal" );

				if ( success )
					return true;
			}
		}

		if ( !SendPortalMessage ( MSG_PORTAL_REQUEST_FAIL, portalID ) ) {
			CErrID ( "ProvidePortal: Failed to send portal request FAIL message!" );
		}

		onEnvironsNotifier1 ( env, nativeID, NOTIFY_PORTAL_PROVIDE_FAIL, portalID );

		return success;
	}
    
    
    bool DeviceBase::BringUpInteractThread ()
    {
        if ( interactThreadState < 2 )
            return false;
        
        if ( interactThreadState > 2 )
            return true;
            
        interactThreadState = 3;
        
        if ( !StartInteractListener () ) {
            CErrID ( "BringUpInteractThread: Failed to start interact channel thread!" );
            
            interactThreadState = 2;
            return false;
        }
        return true;
    }
    

	void DeviceBase::ProccessPortalProvided ( int portalID, PortalStreamType_t streamType )
	{
        CVerbArgID ( "ProccessPortalProvided: [0x%X]", portalID );
        
        bool success = true;
        
        success = BringUpInteractThread ();
        if ( success )
        {
            if ( CreatePortalReceiver ( portalID ) ) {
				int idx = PortalIndex ();

				if ( idx >= 0 && idx < MAX_PORTAL_STREAMS_A_DEVICE )
					onEnvironsNotifier1 ( env, nativeID, streamType == PortalStreamType::Video ? NOTIFY_PORTAL_STREAM_INCOMING : NOTIFY_PORTAL_IMAGES_INCOMING, portalReceivers [ idx ]->portalID );
            }
            else
                success = false;
        }
        
        if ( !success )
            onEnvironsNotifier1 ( env, nativeID, NOTIFY_PORTAL_REQUEST_FAIL, portalID );
	}

	void DeviceBase::ProccessPortalProvidedStream ( int portalID )
	{
		CVerbID ( "ProccessPortalProvidedStream" );

		ProccessPortalProvided ( portalID, PortalStreamType::Video );
	}

	void DeviceBase::ProccessPortalProvidedImages ( int portalID )
	{
		CVerbID ( "ProccessPortalProvidedImages" );

		ProccessPortalProvided ( portalID, PortalStreamType::Images );
	}

	void DeviceBase::ProccessPortalRequestFailed ( int portalID )
	{
		CVerbID ( "ProccessPortalRequestFailed" );

		onEnvironsNotifier1 ( env, nativeID, NOTIFY_PORTAL_REQUEST_FAIL, portalID );
	}


	void DeviceBase::ProccessPortalStop ( int portalID )
	{
		CVerbID ( "ProccessPortalStop" );

		if ( ReleasePortalDeviceID ( portalID ) )
			DisposePortal ( portalID );
	}


	PortalDevice * DeviceBase::GetPortalDeviceAccess ( int portalID )
	{
		int id = PortalIndex ();

		if ( IsValidPortalIndex ( id ) ) {
			// Query our internal portalID 
			if ( IsPortalGenerator () ) {
				return HoldPortalDeviceID ( portalGeneratorsDevice [ id ] );
			}
			else {
				return HoldPortalDeviceID ( portalReceiversDevice [ id ] );
			}
		}

		return 0;
	}


	bool DeviceBase::HandlePortalMessage ( ComMessageHeader * header, bool isBulkChannel )
	{
		unsigned short	portalMsgID = ( header->MessageType.payloadType & 0xFF );

		if ( portalMsgID < MSG_PORTAL_MAX_COUNT )
		{
			// Prepare portalID
			int portalID = header->MessagePack.portalID & 0xFFFFFF;

			ReversePortalDir ( portalID );

			//CVerbArgID ( "HandlePortalMessage: [%s] %s, portalID [0x%X] by device [%s / %s].", resolveName(header->MessageType.payloadType), MSG_PORTAL_Descriptions [portalMsgID], portalID, deviceAreaName, deviceAppName );

			if ( portalMsgID <= MSG_PORTAL_REQUEST_FAIL_ID )
			{
				if ( portalMsgID == MSG_PORTAL_REQUEST_ID )
					ProccessPortalRequest ( portalID, header );
				else
					( ( *this.*portalHandlers [ portalMsgID ] ) ( portalID ) );
			}
			else
			{
				PortalDevice * portalDevice = GetPortalDeviceAccess ( portalID );

				if ( portalDevice ) {
					( ( *this.*portalHandlers [ portalMsgID ] ) ( portalDevice->portalID ) );

					ReleasePortalDevice ( portalDevice );
				}
				else {
					// Those messages only populate notifications to the platform layer, so its safe to
					// execute without the portaldevice locked.
					if ( portalMsgID == MSG_PORTAL_STOP_ACK_ID || portalMsgID == MSG_PORTAL_PAUSE_ACK_ID )
						( ( *this.*portalHandlers [ portalMsgID ] ) ( portalID ) );
				}
			}
		}
		return true;
	}


	void DeviceBase::ProccessPortalAskForRequest ( int portalID )
	{
		CVerbArgID ( "ProccessPortalAskForRequest: portalID [0x%X]", portalID );

		onEnvironsNotifier1 ( env, nativeID, NOTIFY_PORTAL_ASK_REQUEST, portalID );
	}


	void DeviceBase::ProccessPortalRequest ( int portalID, ComMessageHeader * header )
	{
		CVerbID ( "ProccessPortalRequest" );

		int * pUI = reinterpret_cast<int *>( &header->payload );

		int width = *pUI; pUI++;
		int height = *pUI;

		if ( width > 0 && height > 0 ) {
			streamOptions.streamWidth = width;
			streamOptions.streamHeight = height;

			portalInfoOff.width = width;
			portalInfoOff.height = height;
		}

		onEnvironsNotifier1 ( env, nativeID, NOTIFY_PORTAL_REQUEST, portalID );
	}


	void DeviceBase::ProccessPortaliFrameRequest ( int portalID )
	{
		CVerbArgID ( "ProccessPortaliFrameRequest: portalID [0x%X]", portalID );

		unsigned int id = GetPortalIndex ( portalID );

		if ( env->usePlatformPortalGenerator ) {
			return;
		}

		if ( IsPortalGenerator () && id < MAX_PORTAL_STREAMS_A_DEVICE )
		{
			CVerbID ( "ProccessPortaliFrameRequest: We are the sender." );

			if ( portalGenerators [ id ] )
				portalGenerators [ id ]->workerStages.encoder->iFrameRequest = true;
		}
	}

#ifdef ENABLE_PORTAL_STALL_MECHS
	void DeviceBase::ProccessPortalStall ( int portalID )
	{
		CVerbArgID ( "ProccessPortalStall: portalID [0x%X]", portalID );

		unsigned int id = GetPortalIndex ( portalID );

		if ( IsPortalGenerator () && id < MAX_PORTAL_STREAMS_A_DEVICE )
		{
			CVerbID ( "ProccessPortalStall: We are the sender." );

			if ( LockAcquireA ( portalMutex, "ProccessPortalStall" ) ) {
				if ( portalGenerators [ id ] )
					portalGenerators [ id ]->Stall ();

				LockReleaseVA ( portalMutex, "ProccessPortalStall" );
			}
		}
	}


	void DeviceBase::ProccessPortalUnStall ( int portalID )
	{
		CVerbArgID ( "ProccessPortalUnStall: portalID [0x%X]", portalID );

		unsigned int id = GetPortalIndex ( portalID );

		if ( IsPortalGenerator () && id < MAX_PORTAL_STREAMS_A_DEVICE )
		{
			CVerbID ( "ProccessPortalUnStall: We are the sender." );

			if ( LockAcquireA ( portalMutex, "ProccessPortalUnStall" ) ) {
				if ( portalGenerators [ id ] )
					portalGenerators [ id ]->UnStall ();

				LockReleaseVA ( portalMutex, "ProccessPortalUnStall" );
			}
		}
	}
#endif


	void DeviceBase::ProccessPortalStart ( int portalID )
	{
		CVerbArgID ( "ProccessPortalStart: portalID [0x%X]", portalID );


		unsigned int id = GetPortalIndex ( portalID );
		if ( id >= MAX_PORTAL_STREAMS_A_DEVICE )
			return;

		if ( IsPortalGenerator () )
		{
			CVerbID ( "ProccessPortalStart: We are the sender." );

			if ( env->usePlatformPortalGenerator )
			{
				onEnvironsNotifier1 ( env, nativeID, NOTIFY_PORTAL_STREAM_STARTED, portalGenerators [ id ]->portalID );
			}
			else {
				if ( LockAcquireA ( portalMutex, "ProccessPortalStart" ) )
				{
					if ( !portalGenerators [ id ] ) {
						if ( !CreatePortalGenerator ( portalID ) ) {
							CErrID ( "ProccessPortalStart: Failed to create PortalSource. Low memory?" );
							goto FinishStart;
						}
					}

					CVerbID ( "ProccessPortalStart: Starting the portal source." );
					portalGenerators [ id ]->Start ();
					portalID = portalGenerators [ id ]->portalID;

					PortalInfoBase info;
					info.portalID = portalID;

					if ( GetPortalInfo ( &info, false ) ) {
						onEnvironsNotifierContext1 ( env, nativeID, NOTIFY_PORTAL_LOCATION_CHANGED, portalID, &info, sizeof ( PortalInfoBase ) );
					}

				FinishStart:
					LockReleaseVA ( portalMutex, "ProccessPortalStart" );
				}
			}

			if ( !SendPortalMessage ( MSG_PORTAL_START_ACK, portalID ) ) {
				CWarnID ( "ProccessPortalStart: Failed to send portal start ack message to device!" );
			}
		}
	}


	void DeviceBase::ProccessPortalStartAck ( int portalID )
	{
		CVerbID ( "ProccessPortalStartAck" );

		onEnvironsNotifier1 ( env, nativeID, NOTIFY_PORTAL_STREAM_STARTED, portalID );
	}


	void DeviceBase::ProccessPortalPause ( int portalID )
	{
		CVerbID ( "ProccessPortalPause" );

		unsigned int id = PortalIndex ();

		if ( IsPortalGenerator () && id < MAX_PORTAL_STREAMS_A_DEVICE )
		{
			if ( portalGenerators [ id ] )
				portalGenerators [ id ]->Pause ();

			if ( !SendPortalMessage ( MSG_PORTAL_PAUSE_ACK, portalID ) ) {
				CWarnID ( "ProccessPortalPause: Failed to send portal pause ack message to device!" );
			}

			/*hasPhysicalContact = 0;

			if ( Kernel::active ) {
			onEnvironsNotifier ( env, deviceID, deviceAreaName, deviceAppName, NOTIFY_DEVICE_NOT_ON_SURFACE, portalID );
			}*/
		}
	}


	void DeviceBase::ProccessPortalPauseAck ( int portalID )
	{
		CVerbID ( "ProccessPortalPauseAck" );

		onEnvironsNotifier1 ( env, nativeID, NOTIFY_PORTAL_STREAM_PAUSED, portalID );
	}


	void DeviceBase::ProccessPortalStopAck ( int portalID )
	{
		CVerbID ( "ProccessPortalStopAck" );

		onEnvironsNotifier1 ( env, nativeID, NOTIFY_PORTAL_STREAM_STOPPED, portalID );
	}


	bool DeviceBase::GetPortalInfo ( PortalInfoBase * info, bool useLock )
	{
		CVerbID ( "GetPortalInfo" );

		if ( !info ) {
			return false;
		}

		int portalID = info->portalID;

		CVerbArgID ( "GetPortalInfo: [0x%X]", portalID );

		PortalDevice * portalDevice = GetPortalDeviceAccess ( portalID );

		if ( !portalDevice )
			return false;

		bool success = false;

		if ( portalDevice->generator ) {
			CVerbID ( "GetPortalInfo: Generator" );
			success = portalDevice->generator->GetPortalInfo ( info );
		}
		else if ( portalDevice->receiver ) {
			CVerbID ( "GetPortalInfo: Receiver" );
			success = true;

			if ( !portalDevice->receiver->portalInfo.width || !portalDevice->receiver->portalInfo.height )
			{
				CVerbID ( "GetPortalInfo: Requesting current portal info" );
				int retSize = onEnvironsGetResponse ( env, nativeID, MSG_OPT_PORTAL_INFO_GET, sizeof ( PortalInfoBase ), info );

				ReversePortalDir ( info->portalID );

				if ( retSize == sizeof ( PortalInfoBase ) ) {
					memcpy ( &portalDevice->receiver->portalInfo, info, sizeof ( PortalInfoBase ) - sizeof ( int ) );
				}
				else {
					CErrID ( "GetPortalInfo: Async get response failed." );
					success = false;
				}
			}
			else {
				CVerbID ( "GetPortalInfo: Copy" );
				memcpy ( info, &portalDevice->receiver->portalInfo, sizeof ( PortalInfoBase ) );

				CVerbArgID ( "GetPortalInfo: Copied from receiver cache", info->portalID );
			}
		}

		ReleasePortalDevice ( portalDevice );

		return success;
	}


	bool DeviceBase::SetPortalInfoPosibble ( bool updateCounter )
	{
		INTEROPTIMEVAL currentTicks = GetEnvironsTickCount ();

		if ( ( currentTicks - lastPortalUpdate ) < INTEROPTIMEMS ( 33 ) ) {
			CVerbArgID ( "SetPortalInfoPosibble: Skip [%i].", ( currentTicks - lastPortalUpdate ) );
			return false;
		}
		if ( updateCounter )
			lastPortalUpdate = currentTicks;

		return true;
	}


	bool DeviceBase::SetPortalInfo ( void * portalInfo, bool preventFrameOverflow )
	{
		CVerbID ( "SetPortalInfo" );

		if ( !portalInfo ) {
			CVerbID ( "SetPortalInfo: Invalid portal info argument." );
			return false;
		}

		if ( preventFrameOverflow && !SetPortalInfoPosibble ( true ) )
			return true;

		bool success = false;

		PortalInfoBase * info = ( PortalInfoBase * ) portalInfo;

		int portalID = info->portalID;
		CVerbArgID ( "SetPortalInfo: Generator portalID [0x%X]", portalID );

		if ( IsPortalGenerator () ) {
			UpdatePortalSize ( portalID, info->width, info->height, true, false );
			UpdatePosition ( portalID, info->centerX, info->centerY, info->orientation, false );

			success = true;
		}
		else {
			CVerbArgID ( "SetPortalInfo: Receiver centerX [%i] centerY [%i]", info->centerX, info->centerY );

            //            onEnvironsAsyncSend1 ( env, deviceNode->info.objID, portalID, true, MSG_TYPE_OPTIONS, MSG_OPT_PORTAL_INFO_SET, info, sizeof ( PortalInfoBase ) );
            onEnvironsAsyncSend1 ( env, nativeID, portalID, true, MSG_TYPE_OPTIONS, MSG_OPT_PORTAL_INFO_SET, info, sizeof ( PortalInfoBase ) );
            
			info->portalID = portalID;

			// Update our local info and notify app layer about this change
			/*PortalDevice * portalDevice = GetPortalDeviceAccess ( portalID );
			if ( portalDevice ) {
				if ( portalDevice->receiver )
					portalDevice->receiver->portalInfo 

			}
			else {
				portalInfoOff.width = width_new;
				portalInfoOff.height = height_new;
			}

			if ( info->flags & PORTAL_INFO_FLAG_LOCATION ) {

			}
			if ( info->flags & PORTAL_INFO_FLAG_SIZE ) {
			}
			if ( info->flags & PORTAL_INFO_FLAG_ANGLE ) {
			}

			if ( portalDevice ) {
				ReleasePortalDevice ( portalDevice );
			}
			NOTIFY_PORTAL_LOCATION_CHANGED
				NOTIFY_PORTAL_SIZE_CHANGED*/
			success = true;
		}

		return success;
	}


	void DeviceBase::UpdatePortalSize ( int portalID, int width_new, int height_new, bool updateAll, bool preventFrameOverflow )
	{
		CVerbArgID ( "UpdatePortalSize: width_new [%i] - height_new [%i]", width_new, height_new );

		if ( preventFrameOverflow && !SetPortalInfoPosibble () )
			return;

		PortalDevice * portalDevice = GetPortalDeviceAccess ( portalID );
		if ( portalDevice ) {
			if ( portalDevice->generator )
				portalDevice->generator->UpdatePortalSize ( width_new, height_new, updateAll );

			ReleasePortalDevice ( portalDevice );
		}
		else {
			portalInfoOff.width = width_new;
			portalInfoOff.height = height_new;
		}
		lastPortalUpdate = GetEnvironsTickCount ();
	}


	void DeviceBase::UpdatePosition ( int portalID, int x, int y, float angle, bool preventFrameOverflow )
	{
		CVerbArgID ( "UpdatePosition: x [%i] - y [%i]", x, y );

		if ( preventFrameOverflow && !SetPortalInfoPosibble () )
			return;

		PortalDevice * portalDevice = GetPortalDeviceAccess ( portalID );
		if ( portalDevice ) {
			if ( portalDevice->generator ) {
				if ( angle < 0 )
					angle = portalDevice->generator->renderDimensions.orientation;

				portalDevice->generator->UpdatePosition ( x, y, angle );
			}

			ReleasePortalDevice ( portalDevice );
		}
		else {
			portalInfoOff.centerX = x;
			portalInfoOff.centerY = y;

			if ( angle >= 0 )
				portalInfoOff.orientation = angle;
		}
		lastPortalUpdate = GetEnvironsTickCount ();
	}


	void DeviceBase::UpdateAngle ( int portalID, float angle )
	{
		PortalDevice * portalDevice = GetPortalDeviceAccess ( portalID );
		if ( portalDevice ) {
			PortalGenerator * portal = portalDevice->generator;

			if ( portal ) {
				portal->UpdatePosition ( portal->centerX, portal->centerY, angle );
			}

			ReleasePortalDevice ( portalDevice );
		}
		else {
			portalInfoOff.orientation = angle;
		}
	}


	bool DeviceBase::CreatePortalGenerator ( int portalID )
	{
		CVerbID ( "CreatePortalGenerator" );

		unsigned int id = PortalIndex ();

		if ( id >= MAX_PORTAL_STREAMS_A_DEVICE )
			return false;

		if ( portalGenerators [ id ] )
		{
			CVerbID ( "CreatePortalGenerator: Portal source already created." );
			if ( portalGenerators [ id ]->status == PortalSourceStatus::Deleteable ) {
				delete portalGenerators [ id ];
				portalGenerators [ id ] = 0;
			}
			else {
                if ( portalGenerators [ id ]->status < PortalSourceStatus::Initialized ) {
					GetAlignedDimensions ( streamOptions.streamWidth, streamOptions.streamHeight );

					return portalGenerators [ id ]->Init ( &streamOptions, ( DeviceController * ) this, portalID );
				}
				return true;
			}
		}

		CreatePortalGeneratorPlatform ( id );

		if ( !portalGenerators [ id ] )
			portalGenerators [ id ] = new PortalGenerator ();

		if ( portalGenerators [ id ] ) {
			portalGenerators [ id ]->env = env;
			portalGeneratorsCount++;

			GetAlignedDimensions ( streamOptions.streamWidth, streamOptions.streamHeight );
			return portalGenerators [ id ]->Init ( &streamOptions, ( DeviceController * ) this, portalID );
		}

		return false;
	}


	void DeviceBase::CreatePortalGeneratorPlatform ( int portalIDdent )
	{
		CVerbID ( "CreatePortalGeneratorPlatform" );
	}


	void DeviceBase::SyncPortalInfo ( int portalID )
	{
		CVerbID ( "SyncPortalInfo" );

		int				status	= 0;

		PortalInfoBase portalInfo;

		ZeroStruct ( portalInfo, PortalInfoBase );

		PortalDevice * portalDevice = GetPortalDeviceAccess ( portalID );

		if ( portalDevice && portalDevice->generator )
		{
			// Get the most recent portalInfo
			status = portalDevice->generator->GetPortalInfoIfChanged ( &portalInfo );

			ReleasePortalDevice ( portalDevice );
		}

		/// status 1 == no change or error, just return
		if ( status == 1 )
			return;

		/// status 0 == portalSource not available, take the offline infos
		if ( !status )
			memcpy ( &portalInfo, &portalInfoOff, sizeof ( PortalInfoBase ) );

		if ( SendBuffer ( true, MSG_TYPE_OPTIONS, 0, 0, MSG_OPT_PORTAL_INFO_SET, &portalInfo, sizeof ( portalInfo ) ) <= 0 ) {
			CWarnID ( "SyncPortalInfo: Failed to sync portalinfo with the device." );
		}
	}


	bool DeviceBase::HandleHeloMessage ( ComMessageHeader * header, bool isComDatChannel )
	{
		if ( isComDatChannel ) {
			CVerbID ( "HandleHeloMessage: ComDat Channel" );

			if ( header->MessageType.payloadType == MSG_HANDSHAKE_COMDAT_ACK ) {
				CVerbID ( "HandleHeloMessage: ComDat channel by device acknowledged." );
                
                bool update = false;
                
                LockAcquireVA ( portalMutex, "HandleHeloMessage" );
                
                if ( !( activityStatus & DEVICE_ACTIVITY_COMDAT_CONNECTED ) ) {
                    activityStatus |= DEVICE_ACTIVITY_COMDAT_CONNECTED;
                    update = true;
                }
                
                LockReleaseVA ( portalMutex, "HandleHeloMessage" );
                
				if ( update ) {
					if ( SendBuffer ( true, MSG_TYPE_HELO, 0, 0, MSG_HANDSHAKE_COMDAT_ACK, 0, 0 ) < 0 ) {
						CErrID ( "HandleHeloMessage: Failed to send (comDat) ack message to device!" );
						return false;
					}

					UpdateConnectStatus ( 20 );

					onEnvironsNotifier1 ( env, deviceNode->info.objID, NOTIFY_CONNECTION_COMDAT_ACK );
					OnConnectionEstablished ();
				}
				return true;
            }
            else if ( header->MessageType.payloadType == MSG_HANDSHAKE_PING ) {
                CVerbID ( "HandleHeloMessage: ComDat channel ping ..." );
                return true;
            }
            else if ( header->MessageType.payloadType == MSG_HANDSHAKE_CONNECTED ) {
                CVerbID ( "HandleHeloMessage: Connection by device acknowledged." );
                
                onEnvironsNotifier1 ( env, deviceNode->info.objID, NOTIFY_CONNECTION_ESTABLISHED_ACK );
                return true;
            }
		}
		else
			return HandleHeloMessage ( header->MessageType.payloadType, ( char * ) header, header->length );

		return false;
	}


	bool DeviceBase::HandleStreamMessage ( ComMessageHeader * header, bool isBulkChannel )
	{
		int unitSize = header->length - MSG_HEADER_SIZE;
		if ( unitSize > 0 )
		{
			unsigned short unitType = header->MessageType.payloadType;

			// Is it an init packet?
			if ( unitType & DATA_STREAM_INIT ) {
				CInfoArgID ( "HandleStreamMessage: Received [ %s ] init packet of size [ %d ]!", ( unitType & DATA_STREAM_VIDEO ) ? "video" : "image", unitSize );
			}
#ifdef DEBUGVERB
			else {
				CVerbVerbArgID ( "HandleStreamMessage: Received [%s] unit of size %i!", ( unitType & DATA_STREAM_VIDEO ) ? "video" : "image", unitSize );
			}
#endif
			int portalID = header->MessagePack.portalID;
			ReversePortalDir ( portalID );

			EnqueueToReceiverStream ( &header->payload, unitSize, header->MessageType.payloadType, portalID );
		}
		return true;
	}


	bool DeviceBase::HandleStringMessage ( ComMessageHeader * header, bool isBulkChannel )
	{
		unsigned short		payloadType = header->MessageType.payloadType;

		if ( payloadType == MESSAGE_APP_STRING ) {

			/// Async send to the application
			int length = header->length;
			char * pack = ( char * ) header;

			char o = pack [ length ];
			pack [ length ] = 0;

			length -= MSG_HEADER_SIZE;

			if ( length > 0 ) {
				CVerbArgID ( "HandleStringMessage: <--- [ %s ]", ( char * ) &header->payload );
				SaveToStorageMessages ( "ic", ( const char * ) &header->payload, length );

                onEnvironsMsgNotifier1 ( env, deviceNode->info.objID, SOURCE_DEVICE, ( char * ) &header->payload, length );
			}

			pack [ header->length ] = o;
		}
		else if ( payloadType == NATIVE_FILE_TYPE_ACK )
		{
			/// Asynch send to the application
			onEnvironsNotifier1 ( env, deviceNode->info.objID, NATIVE_FILE_TYPE_ACK );
		}

		return true;
	}


	bool DeviceBase::HandleOptionsResponse ( ComMessageHeader * header, bool isBulkChannel )
	{
		onEnvironsHandleResponse ( env, header->length - MSG_HEADER_SIZE, ( char * ) &header->payload );
		return true;
	}


	bool DeviceBase::HandleOptionsMessage ( ComMessageHeader * header, bool isBulkChannel )
	{
		HandleOptionsMessage ( header->MessageType.payloadType, ( char * ) &header->payload );
		return true;
    }
    
    
    bool DeviceBase::HandleSensorData ( ComMessageHeader * header, bool isBulkChannel )
    {
        //CVerbVerbArg ( "HandleSensorData: New packet [%i] x [%.2f] y [%.2f] z [%.2f]", frame->type, frame->data.floats.f1, frame->data.floats.f2, frame->data.floats.f3 );
        
#ifdef DISPLAYDEVICE
        sp ( DeviceInstanceNode ) node = deviceNode;
        
		if ( node ) {
			/*environs::lib::SensorFrameExt * packExt = ( environs::lib::SensorFrameExt * )&header->payload;

			CLogArg ( "SensorEventSender: d1 [%lf] d2 [%lf] d3 [%lf] f1 [%lf] f2 [%lf] f3 [%lf]", packExt->data.doubles.d1, packExt->data.doubles.d2, packExt->data.doubles.d3, 
				packExt->floats.f1, packExt->floats.f2, packExt->floats.f3 );*/

			onEnvironsSensor ( env, node->info.objID, ( environs::lib::SensorFrame * ) &header->payload, header->length );
		}
#endif
        return true;
    }


	void DeviceBase::HandleOptionsMessage ( unsigned short payloadType, char * payload )
	{
		CVerbID ( "HandleOptionsMessage" );

		unsigned int values [ 12 ];
		PortalInfoBase * info;
		PortalDevice * portalDevice = 0;
		int value1;

		values [ 0 ] = *( ( unsigned int * ) payload );

		int type = payloadType & MSG_OPTION_TYPE;
		switch ( type )
		{
		case MSG_OPTION_SET:
		{
			switch ( payloadType )
			{
			case MSG_OPT_PORTAL_INFO_SET:
				CVerbID ( "HandleOptionsMessage: set portal info" );

				if ( !SetPortalInfoPosibble ( true ) )
					break;

				info = ( PortalInfoBase * ) payload;
				ReversePortalDir ( info->portalID );

				portalDevice = GetPortalDeviceAccess ( info->portalID );
				if ( !portalDevice )
				{
					if ( IsPortalIDGenerator ( info->portalID ) )
					{
						if ( info->flags & PORTAL_INFO_FLAG_LOCATION ) {
							CVerbArgID ( "HandleOptionsMessage: Update location to [%u, %u]", info->centerX, info->centerY );

							portalInfoOff.centerX = info->centerX;
							portalInfoOff.centerY = info->centerY;
						}

						if ( info->flags & PORTAL_INFO_FLAG_ANGLE ) {
							CVerbID ( "HandleOptionsMessage: Update angle" );

							portalInfoOff.orientation = info->orientation;
						}

						if ( info->flags & PORTAL_INFO_FLAG_SIZE ) {
							CVerbID ( "HandleOptionsMessage: Update size" );

							portalInfoOff.width  = info->width;
							portalInfoOff.height = info->height;
						}
					}
				}
				else {
					if ( portalDevice->generator )
					{
						CVerbID ( "HandleOptionsMessage: Try setting outgoing portal info" );

						PortalGenerator * portal = portalDevice->generator;

						CVerbID ( "HandleOptionsMessage: Set outgoing portal info" );

						value1 = 0;

						if ( ( info->flags & PORTAL_INFO_FLAG_LOCATION ) && ( info->flags & PORTAL_INFO_FLAG_ANGLE ) ) {
							CVerbID ( "HandleOptionsMessage: Update location, angle" );
							value1++;
							portal->UpdatePosition ( info->centerX, info->centerY, info->orientation, false );
						}
						else if ( info->flags & PORTAL_INFO_FLAG_LOCATION ) {
							CVerbArgID ( "HandleOptionsMessage: Update location to [%u, %u]", info->centerX, info->centerY );
							value1++;
							portal->UpdatePosition ( info->centerX, info->centerY, portal->renderDimensions.orientation, false );
						}
						else if ( info->flags & PORTAL_INFO_FLAG_ANGLE ) {
							CVerbID ( "HandleOptionsMessage: Update angle" );
							value1++;
							portal->UpdatePosition ( portal->centerX, portal->centerY, info->orientation, false );
						}

						if ( info->flags & PORTAL_INFO_FLAG_SIZE ) {
							CVerbID ( "HandleOptionsMessage: Update size" );
							value1++;
							portal->UpdatePortalSize ( info->width, info->height, false );
						}

						if ( value1 && env->desktopDrawRequested )
							PortalGenerator::UpdateDevicesCoverage ( this );
					}
					else {
						CVerbID ( "HandleOptionsMessage: trying to set incoming portal info" );

						PortalReceiver * portal = portalDevice->receiver;
						if ( portal )
						{
							CVerbID ( "HandleOptionsMessage: set incoming portal info" );

							bool locChanged = ( portal->portalInfo.centerX != info->centerX || portal->portalInfo.centerY != info->centerY || portal->portalInfo.orientation != info->orientation );
							bool sizeChanged = ( portal->portalInfo.width != info->width || portal->portalInfo.height != info->height );

							if ( locChanged || sizeChanged ) {
								memcpy ( &portal->portalInfo, info, sizeof ( PortalInfoBase ) );

								portal->portalInfo.portalID = portal->portalID;
							}

							if ( locChanged ) {
								CVerbID ( "HandleOptionsMessage: Update location" );
								onEnvironsNotifierContext1 ( env, nativeID, NOTIFY_PORTAL_LOCATION_CHANGED, portal->portalID, info, sizeof ( PortalInfoBase ) );
							}
							if ( sizeChanged ) {
								CVerbID ( "HandleOptionsMessage: Update size" );
								onEnvironsNotifierContext1 ( env, nativeID, NOTIFY_PORTAL_SIZE_CHANGED, portal->portalID, info, sizeof ( PortalInfoBase ) );
							}
						}
					}
				}
				break;
			}
			break;
		}

		case MSG_OPTION_GET:
		{
			switch ( payloadType )
			{
			case MSG_OPT_PORTAL_INFO_GET:
			{
				CVerbID ( "HandleOptionsMessage: get portal info" );

				bool success = false;
				PortalInfoBase recent;

				unsigned int portalID = *( ( ( int * ) payload ) + 1 );
				ReversePortalDir ( portalID );

				portalDevice = GetPortalDeviceAccess ( portalID );
				if ( !portalDevice )
					break;

				PortalGenerator * portal = portalDevice->generator;
				if ( portal ) {

					ZeroStruct ( recent, PortalInfoBase );

					success = portal->GetPortalInfo ( &recent );
				}

				if ( success ) {
					recent.portalID = portalID;
					info = &recent;
				}
				else {
					info = &portalInfoOff;
				}
				memcpy ( values + 1, info, sizeof ( PortalInfoBase ) );

				SendBuffer ( true, MSG_TYPE_OPTIONS_RESPONSE, 0, 0, 0, values, sizeof ( PortalInfoBase ) + sizeof ( unsigned int ) );
				break;
			}

			default:
				values [ 1 ] = 'e';
				SendBuffer ( true, MSG_TYPE_OPTIONS_RESPONSE, 0, 0, 0, values, sizeof ( unsigned int ) * 2 );
				break;
			}

			break;
		}
		}

		if ( portalDevice )
			ReleasePortalDevice ( portalDevice );
	}


	void DeviceBase::UpdateConnectStatus ( int value, bool set )
	{
		int newValue = value;
		if ( set )
			connectStatus = value;
		else {
			newValue = connectStatus + value;
			if ( newValue > 100 )
				newValue = 100;
			else if ( newValue < 0 )
				newValue = 2;

			connectStatus = newValue;

			if ( value < 0 ) {
				/// An error occured or something has been canceled
				/// We indicate an error through an offset of 1000
				newValue += 1000;
			}
		}

		onEnvironsNotifier1 ( env, deviceNode->info.objID, NOTIFY_CONNECTION_PROGRESS, newValue );
	}


	/**
	* Connect to device with the given ID.
	*
	* @param deviceID	Destination device ID
	* @return	0 Connection can't be conducted (maybe environs is stopped or the device ID is invalid)
	* @return	1 A connection to the device already exists or a connection task is already in progress)
	* @return	2 A new connection has been triggered and is in progress
	*/
	int DeviceBase::ConnectToDevice ( int hInst, int Environs_CALL_, int deviceID, const char * areaName, const char * appName )
	{
		DeviceStatus_t status = DeviceStatus::Deleteable;

		CLogID ( "ConnectToDevice" );

		Instance * env = instances [ hInst ];
		if ( !env )
			return 0;

		if ( env->environsState < environs::Status::Started ) {
			CErrID ( "ConnectToDevice: Dismiss connect. Environs is not fully started." );
			return 0;
		}
        
        sp ( MediatorClient ) mediator = env->mediator MED_WP;
        if ( !mediator ) {
			CErrID ( "ConnectToDevice: Mediator layer is missing. No way to connect to device." );
            return 0;
        }

        sp ( DeviceController ) deviceSP;
        
		DeviceBase *	device	= GetDevice ( env, deviceID, areaName, appName );
		if ( device ) {
			CWarnID ( "ConnectToDevice: Device is already connected or about to connect." );
			status = device->deviceStatus;
			goto Finish;
        }

		deviceSP = std::make_shared < DeviceController > ( deviceID );
        if ( !deviceSP ) {
            CErrID ( "ConnectToDevice: Allocation of a device controller object failed!" );
            goto Finish;
        }
        
        device = deviceSP.get ();

        IncLockDevice ( device );
        
		if ( !device->Init ( env->myself, areaName, appName ) ) {
			CErrID ( "ConnectToDevice: Failed to allocate a device controller object for establishing a connection!" );
			goto Finish;
		}

		device->deviceID        = deviceID;
		device->activityStatus  |= DEVICE_ACTIVITY_REQUESTOR;

		/// Try adding the new device (wait for successful disposal of the previous one and play against concurrent players)
        if ( !TryAddDevice ( deviceSP ) ) {
			device->deviceStatus = DeviceStatus::Deleteable;

			CErrID ( "ConnectToDevice: Failed to add new device." );
			goto Finish;
        }
        
        device->deviceNode->deviceSP    = deviceSP;

		// Create connector thread (if async is requested)
        if ( device->Connect ( Environs_CALL_ ) ) {
			status = device->deviceStatus;
			goto Finish;
		}

		CErrID ( "ConnectToDevice: Failed to initiate connection to device." );
		device->deviceStatus = DeviceStatus::Deleteable;
        device->CloseConnectorThread ();

    Finish:
        if ( device )
            UnlockDevice ( device );

		if ( status == DeviceStatus::Deleteable ) {
			//onEnvironsNotifier ( env, deviceID, areaName, appName, NOTIFY_CONNECTION_PROGRESS, 5 );

            if ( device ) {
                if ( device->deviceNode )
                    onEnvironsNotifier1 ( env, device->deviceNode->info.objID, NOTIFY_CONNECTION_MAIN_FAILED );
                else
                    onEnvironsNotifier1 ( env, deviceID, areaName, appName, NOTIFY_CONNECTION_MAIN_FAILED, SOURCE_NATIVE );                
            }

			TriggerCleanUpDevices ();
		}

		return status;
	}

    
    bool DeviceBase::IsConnectingValid ()
    {
        if ( deviceStatus == DeviceStatus::ConnectInProgress )
        {
            if ( ( GetEnvironsTickCount32 () - connectTime ) > 20000 )
            {
                // Remove the device as it seems connecting, but no activity is going on (or took longer than 20 seconds)
                deviceStatus = DeviceStatus::Deleteable;
                TriggerCleanUpDevices ();
                return false;
            }
        }
        return true;
    }
    

	bool DeviceBase::Connect ( int Environs_CALL_ )
	{
		CVerbID ( "Connect" );

		if ( deviceStatus == DeviceStatus::Deleteable || env->environsState < environs::Status::Starting ) {
			deviceStatus = DeviceStatus::Deleteable;
            
			TriggerCleanUpDevices ();
			return false;
		}

		if ( !connectThread.isRunning () ) 
		{
			if ( ___sync_val_compare_and_swap ( &connectThreadState, ENVIRONS_THREAD_NO_THREAD, ENVIRONS_THREAD_DETACHEABLE ) == ENVIRONS_THREAD_NO_THREAD )
            {                
				UpdateConnectStatus ( 1, true );

				CVerb ( "Connect: setting deviceStatus to ConnectInProgress" );
                deviceStatus = DeviceStatus::ConnectInProgress;
                
                connectTime = GetEnvironsTickCount32 ();

				if ( Environs_CALL_ == CALL_NOWAIT )
                {
					IncLockDevice ( this );

					if ( !connectThread.Run ( pthread_make_routine ( &DeviceBase::ConnectorThreadStarter ), this, "Connect", false ) )
					{
						CVerb ( "Connect: Setting deviceStatus to Deleteable" );
						deviceStatus = DeviceStatus::Deleteable;

						connectThreadState = ENVIRONS_THREAD_NO_THREAD;

						TriggerCleanUpDevices ();

						UnlockDevice ( this );
						return false;
					}
					return true;
				}

				ConnectorThread ( Environs_CALL_ );

				connectThreadState = ENVIRONS_THREAD_NO_THREAD;
			}
		}

		return ( deviceStatus == DeviceStatus::Connected );
	}


	void DeviceBase::CloseConnectorThread ()
	{
		CVerbID ( "CloseConnectorThread" );		
		
		CVerb ( "CloseConnectorThread: setting deviceStatus to Deleteable" );
		deviceStatus = DeviceStatus::Deleteable;

        connectThread.Join ( "Connector Device" );
	}



	/*
	* UdpSendHelo
	*/
	bool DeviceBase::UdpSendHelo ()
	{
		CVerbID ( "UdpSendHelo" );

		if ( !udpAddr.sin_port ) {
			CVerbArgID ( "UdpSendHelo: cannot send helo, destination port of device [%d] is 0!", deviceID );
			return false;
		}

		CVerbArgID ( "UdpSendHelo: Sending helo packet to [%s] on port [%d]...", inet_ntoa ( udpAddr.sin_addr ), ntohs ( udpAddr.sin_port ) );

		UdpHelloPacket	packet;
		Zero ( packet );

		packet.ident [ 0 ] = 'i';
		packet.ident [ 1 ] = 'd';
		packet.ident [ 2 ] = ':';

		packet.version	= UDP_MSG_PROTOCOL_VERSION; // Version
		packet.deviceID = env->deviceID;			// Our deviceID

		strlcpy ( packet.areaName, env->areaName, sizeof ( packet.areaName ) );
		strlcpy ( packet.appName, env->appName, sizeof ( packet.appName ) );

		if ( !SendDataPacket ( ( const char * ) &packet, sizeof ( packet ), ( struct sockaddr * )&udpAddr ) ) {
			CWarnsID ( 2, "UdpSendHelo: Failed to send UDP helo packet to device!" );
			return false;
		}

		// Start listener thread for the socket
		if ( !udpThread.isRunning () && deviceStatus != DeviceStatus::Deleteable )
			if ( !StartUdpListener () )
				return false;

		return true;
	}


	bool DeviceBase::AllocateInteractSocket ()
	{
		CVerbID ( "AllocateInteractSocket" );

        if ( !interactThread.Lock ( "AllocateInteractSocket" ) )
            return false;
        
        int value, ret, sock;
        bool success = false;

#ifdef INCREASE_TCP_SOCKET_BUFFERS
        int retSize;
#endif
        if ( IsValidFD ( interactSocket ) || IsValidFD ( interactSocketForClose ) ) {
            CWarnID ( "AllocateInteractSocket: socket already allocated." );
            success = true;
            goto Finish;
		}
        
        sock = ( int ) socket ( PF_INET, SOCK_STREAM, 0 );
		if ( IsInvalidFD ( sock ) ) {
            CErrID ( "AllocateInteractSocket: Failed to create socket." ); LogSocketError ();
            goto Finish;
        }
        CSocketTraceAdd ( sock, "DeviceBase AllocateInteractSocket interactSocket" );
        
        interactSocket          = sock;
		interactSocketForClose  = sock;

        
		DisableSIGPIPE ( interactSocket );

		value = 1;
		ret = setsockopt ( interactSocket, IPPROTO_TCP, TCP_NODELAY, ( const char * ) &value, sizeof ( value ) );
		if ( ret < 0 ) {
            CErrID ( "AllocateInteractSocket: Failed to set TCP_NODELAY on socket" );
            
            if ( IsValidFD ( interactSocket ) ) { LogSocketErrorF ( "DeviceBase.AllocateInteractSocket" ); }
            goto Finish;
		}

#ifndef USE_ADDR_REUSE_ONLY_FOR_CONNECT
		value = 1;
		ret = setsockopt ( interactSocket, SOL_SOCKET, SO_REUSEADDR, ( const char * ) &value, sizeof ( value ) );
		if ( ret < 0 ) {
            CErrID ( "AllocateInteractSocket: Failed to set reuse on socket." );
            
            if ( IsValidFD ( interactSocket ) ) { LogSocketErrorF ( "DeviceBase.AllocateInteractSocket" ); }
			goto Finish;
        }
#endif

#ifdef INCREASE_TCP_SOCKET_BUFFERS
		// - Load send buffer size
		int retSize = sizeof ( value );
		value = 0;
		ret = getsockopt ( interactSocket, SOL_SOCKET, SO_RCVBUF, ( char * ) &value, &retSize );
		if ( ret < 0 ) {
            CErrID ( "AllocateInteractSocket: Failed to retrieve tcp receive buffer size!" );
            
            if ( IsValidFD ( interactSocket ) ) { LogSocketErrorF ( "DeviceBase.AllocateInteractSocket" ); }
		}
		else
			CLogArgID ( "AllocateInteractSocket: rec buffer size [%i]", value );

		value *= 4;
		ret = setsockopt ( interactSocket, SOL_SOCKET, SO_RCVBUF, ( const char * ) &value, sizeof ( value ) );
		if ( ret < 0 ) {
            CErrID ( "AllocateInteractSocket: Failed to set receive buffer size on socket." );
            
            if ( IsValidFD ( interactSocket ) ) { LogSocketErrorF ( "DeviceBase.AllocateInteractSocket" ); }
			//return false;
		}

		// - Load send buffer size
		retSize = sizeof ( value );
		value = 0;
		ret = getsockopt ( interactSocket, SOL_SOCKET, SO_SNDBUF, ( char * ) &value, &retSize );
		if ( ret < 0 ) {
            CErrID ( "AllocateInteractSocket: Failed to retrieve tcp send buffer size!" );
            
            if ( IsValidFD ( interactSocket ) ) { LogSocketErrorF ( "DeviceBase.AllocateInteractSocket" ); }
		}
		else
			CLogArgID ( "AllocateInteractSocket: rec buffer size [%i]", value );

		// We should set the buffer size to that of the mobileclient to ensure fill?
		//value = PORTAL_SOCKET_BUFFER_SIZE_NORMAL;
		value *= 4;
		ret = setsockopt ( interactSocket, SOL_SOCKET, SO_SNDBUF, ( const char * ) &value, sizeof ( value ) );
		if ( ret < 0 ) {
            CErrID ( "AllocateInteractSocket: Failed to set send buffer size on socket." );
            
            if ( IsValidFD ( interactSocket ) ) { LogSocketErrorF ( "DeviceBase.AllocateInteractSocket" ); }
			//return false;
		}
#endif
        success = true;
        
    Finish:
        interactThread.Unlock ( "AllocateInteractSocket" );

		return success;
	}
    

	bool DeviceBase::AllocateComDatSocket ()
	{
        CVerbID ( "AllocateComDatSocket" );
        
        if ( !comDatThread.Lock ( "AllocateComDatSocket" ) )
            return false;
        
        int value, ret, sock;
        bool success = false;

		if ( IsValidFD ( comDatSocket ) || IsValidFD ( comDatSocketForClose )  ) {
            CVerbID ( "AllocateComDatSocket: socket already allocated." );
            success = true;
            goto Finish;
		}
        
        sock = ( int ) socket ( PF_INET, SOCK_STREAM, 0 ); // IPPROTO_TCP (using 0 and let the service provider choose the protocol)
		if ( IsInvalidFD ( sock ) ) {
            CErrID ( "AllocateComDatSocket: Failed to create socket." ); LogSocketError ();
            goto Finish;
        }
        CSocketTraceAdd ( sock, "DeviceBase AllocateComDatSocket comDatSocket" );
        
        comDatSocket         = sock;
		comDatSocketForClose = sock;

		DisableSIGPIPE ( comDatSocket );

		value = 1;
		ret = setsockopt ( comDatSocket, IPPROTO_TCP, TCP_NODELAY, ( const char * ) &value, sizeof ( value ) );
		if ( ret < 0 ) {
            CErrID ( "AllocateComDatSocket: Failed to set TCP_NODELAY on socket" );
            
            if ( IsValidFD ( comDatSocket ) ) { LogSocketErrorF ( "DeviceBase.AllocateComDatSocket" ); }
            goto Finish;
		}

#ifndef USE_ADDR_REUSE_ONLY_FOR_CONNECT
		value = 1;
		ret = setsockopt ( comDatSocket, SOL_SOCKET, SO_REUSEADDR, ( const char * ) &value, sizeof ( value ) );
		if ( ret < 0 ) {
            CErrID ( "AllocateComDatSocket: Failed to set reuse on socket." );
            
            if ( IsValidFD ( comDatSocket ) ) { LogSocketErrorF ( "DeviceBase.AllocateComDatSocket" ); }
            goto Finish;
        }
#endif

		// We should set the buffer size to that of the mobileclient to ensure fill?
		/*value = PORTAL_SOCKET_BUFFER_SIZE_NORMAL;
		ret = setsockopt ( comDatSocket, SOL_SOCKET, SO_SNDBUF, (const char *)&value, sizeof(value) );
		if ( ret < 0 ) {
         CErr ( "AllocateComDatSocket: Failed to set send buffer size on socket." ); LogSocketError ();
         goto Finish;
         }*/
        
        success = true;
        
    Finish:
        comDatThread.Unlock ( "AllocateComDatSocket" );

		return success;
	}


	bool DeviceBase::AllocateUdpSocket ()
	{
        CVerbID ( "AllocateUdpSocket" );
        
        if ( !udpThread.Lock ( "AllocateUdpSocket" ) )
            return false;
        
        bool success = false;
        
        int value, rc, sock;
        socklen_t retSize;
        
		if ( IsValidFD ( udpSocket ) || IsValidFD ( udpSocketForClose ) ) {
            CVerbID ( "AllocateUdpSocket: UDP socket already allocated." );
            
            CSocketTraceUpdate ( udpSocketForClose, "Checked in AllocateUdpSocket" );
            success = true;
            goto Finish;
        }
        
        if ( deviceStatus == DeviceStatus::Deleteable ) {
            CErrID ( "AllocateUdpSocket: Device disposal is ongoing!" );
            goto Finish;
        }

		//
		// Create data sockets
        //
        sock = ( int ) socket ( PF_INET, SOCK_DGRAM, IPPROTO_UDP );
		if ( IsInvalidFD ( sock ) ) {
			CErrID ( "AllocateUdpSocket: Failed to create socket!" ); LogSocketError ();
			goto Finish;
        }
        CSocketTraceAdd ( sock, "DeviceBase AllocateUdpSocket udpSocket" );
        
        udpSocketForClose   = sock;
        udpSocket           = sock;
        
		DisableSIGPIPE ( udpSocket );

#ifndef USE_ADDR_REUSE_ONLY_FOR_CONNECT
		value = 1;
		rc = setsockopt ( udpSocket, SOL_SOCKET, SO_REUSEADDR, ( const char * ) &value, sizeof ( value ) );
		if ( rc < 0 ) {
            CErrID ( "AllocateUdpSocket: Failed to set reuse on socket." );
            
            if ( IsValidFD ( udpSocket ) ) { LogSocketErrorF ( "DeviceBase.AllocateUdpSocket" ); }
			goto Finish;
		}
#endif
		// - Get socket options
		retSize = sizeof ( retSize );

		// - Load receive buffer size so as to commit with surface
		value = 0;
		rc = getsockopt ( udpSocket, SOL_SOCKET, SO_RCVBUF, ( char * ) &value, &retSize );
		if ( rc < 0 ) {
            CErrID ( "AllocateUdpSocket: Failed to get udp data socket receive buffer size!" );
            
            if ( IsValidFD ( udpSocket ) ) { LogSocketErrorF ( "DeviceBase.AllocateUdpSocket" ); }
		}
		else
            environs::dataRecSize = value;

		CVerbArgID ( "AllocateUdpSocket: get udp socket option buffer size returned [%i], value [%i] size [%i]", rc, value, retSize );

		// - Set receive buffer size to 2MB - does not work at android - os limitation cannot be changed without rooting
		/*
		value = 2000000;
		ret = setsockopt ( sock, SOL_SOCKET, SO_RCVBUF, &value, sizeof (value));
		if (ret < 0) {
		CWarnArg ( "startUdpListener: WARNING - Failed to set data socket receive buffer size to 2MB! (%s)", strerror(errno) );

		// - Set buffer size to default
		value = g_config.dataRecSize;
		ret = setsockopt ( sock, SOL_SOCKET, SO_RCVBUF, &value, sizeof (value));
		if (ret < 0) {
		CWarnArg ( "startUdpListener: WARNING - Failed to set data socket receive buffer size to default (%i)! (%s)", value, strerror(errno) );
		}
		}
		*/
        success = true;

    Finish:
        udpThread.Unlock ( "AllocateUdpSocket" );
        
        if ( !success ) {
            CVerb ( "AllocateUdpSocket: setting deviceStatus to Deleteable" );
            deviceStatus = DeviceStatus::Deleteable;
        }
        
        return success;
	}


	void DeviceBase::TuneReceiveBuffer ( int sock )
	{
	}


	void DeviceBase::TuneSendBuffer ( int sock )
	{
	}


	void * DeviceBase::ConnectorThreadStarter ( void * arg )
	{
        DeviceBase * device = ( DeviceBase * ) arg;
        
        device->ConnectorThread ( CALL_NOWAIT );
        
        device->connectThread.Detach ( "Connector Device" );

		UnlockDevice ( device );
        return 0;
	}


	void DeviceBase::ConnectorThread ( int Environs_CALL_ )
    {
		CVerbID ( "ConnectorThread: started ..." );

		pthread_setname_current_envthread ( "DeviceBase::ConnectorThread" );
        
        int port;
        int portUdp;
        
#ifdef ENABLE_DEVICEBASE_WEAK_REFERENCE
        if ( myself.expired () )
            goto Failed;
#else
        if ( !myself )
            goto Failed;
#endif

		while ( deviceStatus != DeviceStatus::Deleteable )
		{
			if ( env->environsState < environs::Status::Starting ) {
                CErrID ( "ConnectorThread: Dismiss connect due to disposal of Environs" );
                break;
			}
            
            sp ( MediatorClient ) mediator = env->mediator MED_WP;
			if ( !mediator ) {
				CVerbID ( "ConnectorThread: Mediator is missing." );
				break;
            }
            
            sp ( DeviceInstanceNode ) node = deviceNode;
            if ( node ) {
                unsigned int ip = node->info.broadcastFound != DEVICEINFO_DEVICE_MEDIATOR ? node->info.ip : node->info.ipe;
                
                CLogArgID ( "Connect: to [ %s : %s ] ...", node->info.deviceName, inet_ntoa ( *( ( struct in_addr * ) &ip ) ) );
                node.reset ();
            }
            
            unsigned int ip     = 0;
            unsigned int ipe    = 0;
            
            if ( !mediator->GetConnectionDetails ( deviceID, deviceAreaName, deviceAppName, &deviceStatus, ip, port, ipe, portUdp ) || (deviceStatus == DeviceStatus::Deleteable) ) {
                CErrID ( "ConnectorThread: Failed to retrieve connection details." );
                break;
            }
            
			bool isSameNetwork = mediator->IsDeviceInSameNetwork ( deviceID, deviceAreaName, deviceAppName );

			// We do hole punching if
			// - destination is not in same subnet (direct accessible)
			// - the private and public ip of destination is not the same (NAT)
			// - we are behind a NAT
			//
			if ( ipe && ( !isSameNetwork && ( ( ip != ipe ) || mediator->GetNATStat () ) ) )
			{
				// We need to puch a hole into the destination's NAT
				CVerbArgID ( "ConnectorThread: Destination is behind a NAT IPe [ %s ]", inet_ntoa ( *( ( struct in_addr * ) &ipe ) ) );
				CVerbArgID ( "ConnectorThread: Destination is behind a NAT IP  [ %s ]", inet_ntoa ( *( ( struct in_addr * ) &ip ) ) );

				behindNAT = true;
			}
			else
			{
				behindNAT           = false;
                
				interactAddr.sin_port = htons ( ( unsigned short ) port );
				comDatAddr.sin_port	= htons ( ( unsigned short ) port );
				comPort				= ( unsigned short ) port;

				udpAddr.sin_port	= htons ( ( unsigned short ) portUdp );
				dataPort = ( unsigned short ) portUdp;

				comDatAddr.sin_family   = PF_INET;
				interactAddr.sin_family = PF_INET;

				unsigned int netIP  = ip;
				if ( !ip || mediator->IsIPInSameNetwork ( ipe ) )
					netIP = ipe;

				interactAddr.sin_addr.s_addr	= netIP;
				comDatAddr.sin_addr.s_addr      = netIP;
				udpAddr.sin_addr.s_addr         = interactAddr.sin_addr.s_addr;
			}

			if ( !AllocateUdpSocket () ) {
				CVerbID ( "ConnectorThread: AllocateUdpSocket failed." );
				break;
			}

			if ( env->useCLSForDevices ) {
				unsigned int aesLen = AES_SHA256_KEY_LENGTH * 2;
                
                free_n ( aesBlob );

				aesBlob = ( char * ) malloc ( aesLen );
				if ( !aesBlob ) {
					CErrID ( "ConnectorThread: Memory allocation for aesBlob failed!" ); break;
				}

				unsigned int * pUI = ( unsigned int * ) aesBlob;
				for ( unsigned int i=0; i<( aesLen / 4 ); i++ ) {
					*pUI = rand (); pUI++;
				}
                
                AESDisposeKeyContext ( &aes );
                
				aes.deviceID = deviceID;
				if ( !AESDeriveKeyContext ( aesBlob, aesLen, &aes ) ) {
					CErrID ( "ConnectorThread: Failed to derive session keys." ); break;
				}
			}

            PrepareStorage ();
            
            connectThread.autoreset = false;
            
            if ( Environs_CALL_ == CALL_WAIT && !connectThread.ResetSync ( "ConnectorThread", false ) ) {
				CVerbID ( "ConnectorThread: ResetSync failed." );
				break;
			}

			OnPreConnectionEstablished ();

			if ( !StartInteractListener () ) {
				CErrID ( "ConnectorThread: Failed to create thread for interact channel socket!" );
				break;
			}

			if ( !StartComDatListener () ) {
				CErrID ( "ConnectorThread: Failed to create thread for comDat channel socket!" );
				break;
			}
            
			if ( Environs_CALL_ == CALL_WAIT )
			{
				int maxWaits = 20;

				while ( deviceStatus != DeviceStatus::Connected && deviceStatus != DeviceStatus::Deleteable )
                {
                    if ( connectThread.WaitOne ( "ConnectorThread", 10000 ) )
					{
						CVerbID ( "ConnectorThread: WaitOne signaled." );
						break;
					}

					--maxWaits;

					if ( maxWaits <= 0 )
						goto Failed;
					CLogID ( "ConnectorThread: WaitOne." );
				}

				CVerbID ( "ConnectorThread: Wait loop done." );
				return;
			}

            CVerbID ( "ConnectorThread: Finished." );
            return;
		}
        
    Failed:
		CVerb ( "ConnectorThread: setting deviceStatus to Deleteable" );
		deviceStatus = DeviceStatus::Deleteable;

		CErrID ( "ConnectorThread: Failed." );

		TriggerCleanUpDevices ();
	}


	void DeviceBase::OnPreConnectionEstablished ()
	{
		CVerbID ( "OnPreConnectionEstablished" );

	}


	void DeviceBase::OnUdpConnectionEstablished ()
	{
		CVerbID ( "OnUdpConnectionEstablished" );

		if ( !( activityStatus & DEVICE_ACTIVITY_UDP_CONNECTED ) ) {
			activityStatus |= DEVICE_ACTIVITY_UDP_CONNECTED;
			UpdateConnectStatus ( 10 );

			onEnvironsNotifier1 ( env, deviceNode->info.objID, NOTIFY_CONNECTION_DATA_ACK );
		}

		OnConnectionEstablished ();
	}
    

	int	DeviceBase::ReceiveOneMessage ( char * buffer, unsigned int bufferSize )
	{
		CVerbID ( "ReceiveOneMessage" );

		int				bytesReceived	= 0;
		unsigned int	length			= 0;
		int				bufferRemaining = ( int ) bufferSize;
		char		*	pBuffer			= buffer;
		ComMessageHeader * header		= ( ComMessageHeader * ) buffer;

		unsigned int	recMsgLength	= 0;

		while ( deviceStatus != DeviceStatus::Deleteable )
		{
			pBuffer += bytesReceived;
			bufferRemaining = bufferSize - length;

			if ( bufferRemaining <= 2 ) {
				CErrID ( "ReceiveOneMessage: Preventing potential receive buffer overflow." );
				return 0;
			}

			if ( IsValidFD ( interactSocket ) )
				bytesReceived = MediatorClient::Receive ( interactSocket, pBuffer, bufferRemaining, 0, "ReceiveOneMessage" );

			if ( bytesReceived <= 0 ) {
                CWarnID ( "ReceiveOneMessage: Socket error" );
                
                if ( IsValidFD ( interactSocket ) ) { LogSocketErrorF ( "DeviceBase.ReceiveOneMessage" ); }
				return bytesReceived;
			}
			length += bytesReceived;

			if ( length >= 8 && recMsgLength == 0 ) {
				if ( encrypt )
					recMsgLength = *( reinterpret_cast<unsigned int *>( buffer ) ) & 0xFFFFFF;
				else
					recMsgLength = header->length;

				if ( recMsgLength >= bufferSize ) {
					CErrArgID ( "ReceiveOneMessage: Expected buffer overrun [ %u : %u ] without having <EOF> received [ %u bytes ] !!! Preventing crash now. Goodbye device...", recMsgLength, bufferSize, length );
					return 0;
				}
			}

			if ( length >= recMsgLength )
				break;
		}

		if ( encrypt && recMsgLength > 0 ) {
			if ( recMsgLength <= 4 )
				return -1;
			char * decrypted = 0;

			if ( !AESDecrypt ( &aes, buffer, &recMsgLength, &decrypted ) )
				return -1;
			memcpy ( buffer, decrypted, recMsgLength );
			free ( decrypted );
		}

		return recMsgLength;
	}


	int DeviceBase::SecureChannelProvide ( Instance * env, int &sock, AESContext * aes )
	{
		CVerb ( "SecureChannelProvide" );

		int sentBytes;

		if ( !env->useCLSForDevicesEnforce ) {
			// We do not enforce crypt layer security
			if ( !env->useCLSForDevices ) {
				// We do not use crypt layer security by default, but somebody wants to talk to use
				// Let's ask whether unencrypted messages may work
				char buffer [8];

				buffer [0] = 'N';
				buffer [1] = 'C';
				buffer [2] = 'L';
				buffer [3] = 'S';

				sentBytes = (int) send ( sock, buffer, 4, MSG_NOSIGNAL );
				if ( 4 != sentBytes ) {
					CVerbArg ( "SecureChannelProvide: No CLS request failed [ 4 != %i ].", sentBytes );
					return 0;
				}

				// Wait for answer ...
				sentBytes = MediatorClient::Receive ( sock, buffer, sizeof ( buffer ), 4, "SecureChannelProvide" );
				if ( sentBytes <= 0 ) {
					CVerbArg ( "SecureChannelProvide: Socket [ %i ] closed; Bytes [ %i ]!", sock, sentBytes );
					return 0;
				}

				if ( buffer [0] == 'N' && buffer [1] == 'C' && buffer [2] == 'L' && buffer [3] == 'S' ) {
					CVerb ( "SecureChannelProvide: No CLS ACK received." );
					return -1;
				}
			}
		}

		/// We support certificates up to a size of 2k
		bool			success	= 0;
		char		*	buffer	= 0, *decrypted = 0;
        int				bytes	= 0;
        int				bytesReceived	= 0;
		unsigned int	length	= 0;
		unsigned int *	pUI		= reinterpret_cast<unsigned int *>( opt_pubCert );

		if ( !opt_pubCert || !opt_privKey ) {
			CErr ( "SecureChannelProvide: Certificate/Private key missing!" );
			return 0;
		}

		/// Send certificate (4 bytes size, x bytes cert)
		unsigned int certSize = ( *pUI & 0xFFFF ) + 4; /// We add 4 bytes for the formatSize "header"

		sentBytes = ( int ) send ( sock, opt_pubCert, certSize, MSG_NOSIGNAL );
		if ( ( int ) certSize != sentBytes ) {
			CVerbArg ( "SecureChannelProvide: Sending of certificate failed  [ %u != %i ].", certSize, sentBytes );
			return 0;
		}

		do
		{
			buffer = ( char * ) malloc ( certSize );
			if ( !buffer ) {
				CErrArg ( "SecureChannelProvide: Memory allocation [ %u bytes ] failed.", certSize ); break;
            }
            
            unsigned int    msgSize         = 0;
            char *          recBuffer       = buffer;
            unsigned int    recBufferSize   = certSize;
            int             minRec          = 4;
            
        ReceiveNext:
			/// Receive the encrypted 64 byte Hash of the area name
			bytesReceived = MediatorClient::Receive ( sock, recBuffer, recBufferSize, minRec, "SecureChannelProvide" );
			if ( bytesReceived <= 0 ) {
				CVerbArg ( "SecureChannelProvide: Socket [ %i ] closed; [ %i bytes ]!", sock, bytesReceived ); break;
            }
            CVerbVerbArg ( "SecureChannelProvide: Received bytes [ %i ]!", bytesReceived );

			bytes += bytesReceived;

			if (  !msgSize && bytes >= 4 ) {
                if ( IsFINMessage ( buffer ) ) {
                    CVerbArg ( "SecureChannelProvide: Received FIN [ %i ]!", bytesReceived );
                    break;
                }
                
				msgSize = *( reinterpret_cast<unsigned int *>( buffer ) ) & 0xFFFF;

				if ( msgSize >= certSize ) {
					CErrArg ( "SecureChannelProvide: Message size [ %u ] would overflow receive buffer. Aborting transfer.", msgSize );
					break;
				}
			}

			if ( bytes < ( int ) msgSize ) {
				CVerb ( "SecureChannelProvide: Received message is not complete." );

				recBuffer       += bytesReceived;
				recBufferSize   -= bytesReceived;

				minRec = (msgSize - bytes);
				goto ReceiveNext;
			}
            
			if ( send ( sock, ( char * ) &bytes, sizeof ( int ), MSG_NOSIGNAL ) < ( int ) sizeof ( int ) ) {
				CErr ( "SecureChannelProvide: Failed to send aes receive ack." ); break;
			}

			CVerbVerbArg ( "SecureChannelProvide: ciphers [%s]", ConvertToHexSpaceString ( buffer, bytes ) );

			//CLogArg ( "SecureChannelProvide: opt_privKey [%s]", ConvertToHexSpaceString (opt_privKey, *((unsigned int *)opt_privKey)) );

#ifdef USE_WIN32_CLIENT_CACHED_PRIVKEY
			if ( !DecryptMessageWithKeyHandles ( g_hPrivKey, buffer + 4, bytes - 4, &decrypted, &length ) )
#else
			if ( !DecryptMessage ( opt_privKey, *( reinterpret_cast<unsigned int *>( opt_privKey ) ), buffer + 4, bytes - 4, &decrypted, &length ) ) 
#endif
			{
				CErr ( "SecureChannelProvide: Decrypt session keys failed." );
				break;
			}

			aes->deviceID   = 0xFFFFFFFF;
			aes->size       = AES_SHA256_KEY_LENGTH;
            
			if ( !AESDeriveKeyContext ( decrypted, length, aes ) ) {
				CErr ( "SecureChannelProvide: Failed to derive session keys." ); break;
			}
			success = 1;
		}
		while ( 0 );
        
        free_n ( buffer );
        free_n ( decrypted );
		return success;
	}


	int DeviceBase::SecureChannelEstablish ( Instance * env, int deviceID, int &sock, AESContext * aes, char * aesBlob )
	{
		CVerbID ( "SecureChannelEstablish" );

		if ( !aesBlob || !SocketTimeout ( sock, -1, 30 ) ) {
			CErrID ( "SecureChannelEstablish: No AES blob or invalid socket generated yet." ); return false;
		}

		/// We support certificates up to a size of 2k
		int             success		= 0;
		char			buffer [ ENVIRONS_MAX_KEYBUFFER_SIZE ];
		int				bytes		= 0;
		unsigned int *	pUI			= reinterpret_cast<unsigned int *>( buffer );
		char		*	response	= 0;
		unsigned int	hashLen;
        int             bytesSent;
		bool			revertSocket = false;

		aes->size = AES_SHA256_KEY_LENGTH;

		/// At first, send a cls request
        buffer [ 4 ] = 'H'; buffer [ 5 ] = 'C'; buffer [ 6 ] = 'L'; buffer [ 7 ] = 'S';
		*pUI = 8;

        do
        {
            bytesSent = ( int ) send ( sock, buffer, 8, MSG_NOSIGNAL );
            if ( bytesSent < 8 ) {
                CErrArgID ( "SecureChannelEstablish: Failed to send packet to device! sent bytes and length of packet mismatch [ %i != 8 ].", bytesSent ); LogSocketErrorID ();
                break;
            }

            if ( env->environsState < environs::Status::Started )
                break;

            bytes = MediatorClient::Receive ( sock, buffer, ENVIRONS_MAX_KEYBUFFER_SIZE, 4, "SecureChannelEstablish" );
            if ( bytes < 4 ) {
                CVerbArgID ( "SecureChannelEstablish: Socket [ %i ] closed or incomplete message received; Bytes [ %i ]!", sock, bytes ); break;
            }
            
            if ( bytes == 4 && buffer [ 0 ] == 'N' && buffer [ 1 ] == 'C' && buffer [ 2 ] == 'L' && buffer [ 3 ] == 'S' ) {
                // Device asks for non crypt layer security talks
                
                if ( !env->useCLSForDevicesEnforce ) {
                    // It's okay
                    bytesSent = ( int ) send ( sock, "NCLS", 4, MSG_NOSIGNAL );
                    if ( 4 != bytesSent ) {
                        CVerbArg ( "SecureChannelEstablish: No CLS ACK failed [ 4 != %i ].", bytesSent ); break;
                    }
                    success = -1;
					revertSocket = true;
                    break;
                }
                
                // We are instructed to enforce CLS
                bytesSent = ( int ) send ( sock, "HCLS", 4, MSG_NOSIGNAL );
                if ( 4 != bytesSent ) {
                    CVerbArg ( "SecureChannelEstablish: CLS enforce response failed [ 4 != %i ].", bytesSent ); break;
                }
                
                bytes = MediatorClient::Receive ( sock, buffer, ENVIRONS_MAX_KEYBUFFER_SIZE, 4, "SecureChannelEstablish" );
                if ( bytes <= 0 ) {
                    CVerbArgID ( "SecureChannelEstablish: Socket [ %i ] closed; Bytes [ %i ]!", sock, bytes ); break;
                }
            }
            
            success = 1;
        }
        while ( false );

        if ( success <= 0 ) {
			if ( revertSocket )
				SocketTimeout ( sock, -1, 0 );
            return success;
        }
        
        /// Certificate (4 bytes size, x bytes cert)
        
		unsigned int certSize = ( *pUI & 0xFFFF );

		do
		{
			if ( certSize > 10000 ) {
				CErrArgID ( "SecureChannelEstablish: Certificate length received [ %u bytes ] is larger than 10000 bytes.", certSize ); break;
			}

			int maxReceives = 6;

			while ( certSize > ( unsigned ) bytes ) {
				CVerbArgID ( "SecureChannelEstablish: Certificate length received [ %u bytes ] is larger than bytes received [ %i ].", certSize, bytes ); 
				
				int bytesAdd = MediatorClient::Receive ( sock, buffer + bytes, ENVIRONS_MAX_KEYBUFFER_SIZE, 0, "SecureChannelEstablish" );
				if ( bytesAdd <= 0 ) {
					CVerbArgID ( "SecureChannelEstablish: Socket [ %i ] closed; Bytes [ %i ]!", sock, bytesAdd ); break;
				}

				bytes += bytesAdd;
				maxReceives--;

				if ( maxReceives <= 0 )
					goto Finish;
			}

			hashLen = AES_SHA256_KEY_LENGTH * 2;

			if ( certSize < hashLen ) {
				CErrArgID ( "SecureChannelEstablish: Invalid certificate length received [ %u bytes ].", certSize ); break;
			}

			response = ( char * ) malloc ( certSize + 4 );
			if ( !response ) {
				CErrArgID ( "SecureChannelEstablish: Memory allocation [ %u bytes ] failed.", certSize ); break;
			}

			memcpy ( response + 4, aesBlob, hashLen );

			if ( !EncryptMessage ( deviceID, buffer, response + 4, &hashLen ) ) {
				CErrID ( "SecureChannelEstablish: Failed to encrypt response." ); break;
            }
            
            if ( hashLen < 256 ) {
                CErrArgID ( "SecureChannelEstablish: Encrypted response size is invalid [ %u ].", hashLen ); break;
            }

            *((unsigned int *) response) = hashLen + 4;
            
			bytesSent = ( int ) send ( sock, response, hashLen + 4, MSG_NOSIGNAL );
			if ( bytesSent < ( int ) hashLen ) {
				CErrArgID ( "SecureChannelEstablish: Failed to send aes session keys. Size mismatch [ %i != %i ].", bytesSent, hashLen ); LogSocketErrorID ();
				break;
			}

			/// We expect a 4 byte sync packet before we send our next handshake sequence
			bytes = MediatorClient::Receive ( sock, buffer, 4, 0, "SecureChannelEstablish" );
			if ( bytes <= 0 ) {
				CVerbArgID ( "SecureChannelEstablish: Socket [ %i ] closed; Bytes [ %i ]!", sock, bytes ); break;
			}

			revertSocket = true;
			success = 1;
		}
		while ( 0 );

    Finish:
        free_n ( response );
        
		if ( revertSocket )
			SocketTimeout ( sock, -1, 0 );
		return success;
	}


	bool DeviceBase::InitiateInteractChannel ()
	{
		CVerb ( "InitiateInteractChannel" );

		int bytesReceived;

		if ( !*env->appName )
			return false;

		if ( env->useCLSForDevices ) {
			bytesReceived = SecureChannelEstablish ( env, deviceID, interactSocket, &aes, aesBlob );
			if ( !bytesReceived )
				return false;
			if ( bytesReceived > 0 )
				encrypt = 1;
			else {
                if ( aes.encCtx CRYPT_AES_LOCK_EXP ( || aes.lockAllocated ) ) {
					LockAcquireVA ( portalMutex, "InitiateInteractChannel" );

					AESDisposeKeyContext ( &aes );

					LockReleaseVA ( portalMutex, "InitiateInteractChannel" );
				}
			}
		}

		char handshakeBuffer [ MSG_BUFFER_SEND_SIZE ];

		int payloadLength = PrepareHandshakeBuffer ( handshakeBuffer );
		if ( !payloadLength )
			return false;

		if ( SendBuffer ( false, MSG_TYPE_HELO, 0, 0, MSG_HANDSHAKE_MAIN_REQ, handshakeBuffer, payloadLength ) != payloadLength )
			return false;

		bytesReceived = ReceiveOneMessage ( handshakeBuffer, MSG_BUFFER_SEND_SIZE );
		if ( bytesReceived <= ( int ) MSG_HEADER_SIZE ) {
			CErrID ( "InitiateInteractChannel: Failed to receive device configuration." );
			return false;
		}

		ComMessageHeader * header = ( ComMessageHeader * ) handshakeBuffer;

		CVerbArgID ( "InitiateInteractChannel: Received implicit Handshake ACK on main channel [ %i bytes ]", bytesReceived );

		handshakeBuffer [ bytesReceived ] = 0;

		// Analyse buffer
		if ( header->type != MSG_TYPE_HELO || header->MessageType.payloadType != MSG_HANDSHAKE_CONIG_RESP )
			return false;

		CVerbArgID ( "InitiateInteractChannel: Msg [ %s ]", ( char * ) &header->payload );

		if ( !EvaluateDeviceConfig ( ( char * ) &header->payload ) ) {
			CErrID ( "InitiateInteractChannel: Invalid init message, failed to retrieve device id!!!" );
			return false;
		}

		// Send our device configuration at first, then send our ports
		if ( !SendDeviceConfig () ) {
			CErrID ( "InitiateInteractChannel: Failed to send device configuration." );
			return false;
		}

		bytesReceived = ReceiveOneMessage ( handshakeBuffer, MSG_BUFFER_SEND_SIZE );
		if ( bytesReceived < ( signed ) MSG_HEADER_SIZE ) {
			CErrID ( "InitiateInteractChannel: Failed to receive device configuration ACK." );
			return false;
		}

		// Analyse ACK
		if ( header->type != MSG_TYPE_HELO || header->MessageType.payloadType != MSG_HANDSHAKE_CONIG_RESP_ACK ) {
			CErrID ( "InitiateInteractChannel: Failed to receive device configuration ACK. Message is not an ACK." );
			return false;
		}

		//
		// Send our ports
		// ---------------

		//
		// Prepare the buffer header
		//
		// Fill payload with our ports
        unsigned int * pUI = reinterpret_cast<unsigned int *>( handshakeBuffer );
		*pUI 	 = ( unsigned int ) env->tcpPort;
		*( pUI + 1 ) = ( unsigned int ) env->udpPort;
		handshakeBuffer [ 8 ] = 0;

		if ( SendBuffer ( false, MSG_TYPE_HELO, 0, 0, MSG_HANDSHAKE_PORTS, handshakeBuffer, 9 ) != 9 )
			return false;

		// Wait or ACK
		bytesReceived = ReceiveOneMessage ( handshakeBuffer, MSG_BUFFER_SEND_SIZE );
		if ( bytesReceived < ( signed ) MSG_HEADER_SIZE ) {
			CErrID ( "InitiateInteractChannel: Failed to receive ports ACK." );
			return false;
		}

		// Analyse ACK
		if ( header->type != MSG_TYPE_HELO || header->MessageType.payloadType != MSG_HANDSHAKE_PORTS_ACK ) {
			CErrID ( "InitiateInteractChannel: Failed to receive ports ACK. Message is not an ACK." );
			return false;
		}

		return true;
	}


	bool DeviceBase::SendBuffer ( int nativeID, int fileID, const char * fileDescriptor, const char * buffer, size_t size )
	{
		CVerbVerbIDN ( "SendBuffer" );

		// Get the client
		DeviceBase * device = GetDevice ( nativeID );
		if ( !device ) {
			CErrIDN ( "SendBuffer: Device must be connected for sending buffers!" );
			return false;
		}

		bool ret = device->SendComDatBuffer ( MSG_TYPE_FILE, fileID, fileDescriptor, NATIVE_FILE_TYPE_APP_DEFINED, buffer, size );

		UnlockDevice ( device );
		return ret;
	}


	bool DeviceBase::SendFile ( int nativeID, int fileID, const char * fileDescriptor, const void * filePath )
	{
		CVerbVerbIDN ( "SendFile" );

		bool ret = false;

		// Get the client
		DeviceBase * device = GetDevice ( nativeID );
		if ( !device ) {
			CErrIDN ( "SendFile: Device must be connected for sending files!" );
			return false;
		}

#if (defined(_WIN32) && !defined(WINDOWS_PHONE))
		const char * tFilePath = ( const char * ) filePath;

		DWORD dwAttrib = GetFileAttributesA ( tFilePath );

		if ( dwAttrib == INVALID_FILE_ATTRIBUTES || ( dwAttrib & FILE_ATTRIBUTE_DIRECTORY ) ) {
			CErrArgIDN ( "SendFile: The source either does not exist or is a directory! [ %s ]", tFilePath );
			goto Finish;
		}

		DWORD  dwBytesRead = 0;

		HANDLE hFile = CreateFileA ( tFilePath,
			GENERIC_READ,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL );

		if ( hFile == INVALID_HANDLE_VALUE )
		{
			CErrArgIDN ( "SendFile: Failed to read file [ %s ]", tFilePath );
			goto Finish;
		}

		DWORD size = 0;
		char * buffer = 0;

		size = GetFileSize ( hFile, NULL );
		while ( size != INVALID_FILE_SIZE && size > 0 )
		{
			if ( size > MAX_BULK_SEND_SIZE ) {
				CErrIDN ( "SendFile: File is larger than 200MB! We don't do this yet!" );
				break;
			}

			buffer = ( char * ) malloc ( size + 10 );
			if ( !buffer ) {
				CErrArgIDN ( "SendFile: Failed to allocate memory [ %i ] bytes)!", size );
				break;
			}

			if ( !ReadFile ( hFile, buffer, size, &dwBytesRead, NULL ) ) {
				CErrIDN ( "SendFile: Failed to read file!" );
				break;
			}

			if ( dwBytesRead > 0 && dwBytesRead <= size )
			{
				buffer [ dwBytesRead ] = 0;
				ret = device->SendComDatBuffer ( MSG_TYPE_FILE, fileID, fileDescriptor, NATIVE_FILE_TYPE_EXT_DEFINED, buffer, dwBytesRead );
			}
			else {
				CErrArgIDN ( "SendFile: Failed to read file. Bytes read [ %i ]", dwBytesRead );
			}
			break;
		}
        
        free_n ( buffer );

		CloseHandle ( hFile );
#else

		int fileSize = 0;
		char * buffer = LoadBinary ( ( const char * ) filePath, &fileSize );
		if ( !buffer ) {
			CErrArgIDN ( "SendFile: Failed to read file [ %s ].", ( const char * ) filePath );
			goto Finish;
		}

		ret = device->SendComDatBuffer ( MSG_TYPE_FILE, fileID, fileDescriptor, NATIVE_FILE_TYPE_EXT_DEFINED, buffer, fileSize );
        
        free_m ( buffer );
#endif

	Finish:
		UnlockDevice ( device );
		return ret;
	}


	bool DeviceBase::SendMessage ( int hInst, volatile DeviceStatus_t * deviceStatus, int deviceID, const char * areaName, const char * appName, const char * message, unsigned int length )
	{
        Instance * env = instances [ hInst ];
        if ( !env )
            return false;

		CVerbsArgID ( 3, "SendMessage:  [ %s ]", message );
        
		// Get the client
		sp ( DeviceController ) deviceSP = GetDeviceSP ( env, deviceID, areaName, appName );

		DeviceBase * device = ( DeviceBase * ) deviceSP.get ();

        if ( device && device->deviceStatus >= DeviceStatus::Connected && ( device->activityStatus & DEVICE_ACTIVITY_CONNECTED ) )
		{
			device->SaveToStorageMessages ( "oc", message, length );

			CVerbsArgID ( 4, "SendMessage: Use available connection for message [ %s ]", message );

			return device->SendComDatBuffer ( MSG_TYPE_MESSAGE, 0, 0, MESSAGE_APP_STRING, message, length );
		}
        
        return DeviceBase::SendMessageToDevice ( env, deviceStatus, deviceID, areaName, appName, ( void * ) message, length );
	}


	bool DeviceBase::SendHeaderedBuffer ( Instance * env, int deviceID, const char * areaName, const char * appName, void * message, unsigned int length )
	{
		CVerbID ( "SendHeaderedBuffer" );

		bool ret = false;

		int bytesSent;
		DeviceBase * device = GetDevice ( env, deviceID, areaName, appName );
		if ( device ) {
			bytesSent = device->SendHeaderedBuffer ( message, length );
			UnlockDevice ( device );

			if ( bytesSent != ( signed ) length ) {
				CErrID ( "SendHeaderedBuffer: Failed to send message." );
			}
			else ret = true;
		}

		return ret;
	}


	bool DeviceBase::SendPushNotification ( Instance * env, int deviceID, const char * areaName, const char * appName, const char * message )
    {
        sp ( MediatorClient ) mediator = env->mediator MED_WP;
		// Get the client
		if ( !mediator ) {
			CErrArg ( "SendPushNotification: Mediator not available yet! Notification to device %i failed.", deviceID );
			return false;
		}

		if ( GCMAPIKey ) {
			if ( !mediator->SetParam ( 0, areaName, appName, "gcm", GCMAPIKey ) ) {
				CErr ( "SendPushNotification: Failed to store gcm api key to mediator." );
				return false;
			}

			/*if ( !mediator->SetParam ( areaName, clientID, "push", message ) ) {
			CErrArg ( "SendPushNotification: Failed to send push message via mediator." );
			return false;
			}*/
		}


		return mediator->SetParam ( deviceID, areaName, appName, "push", message );
	}


	void DeviceBase::SendUdpPortal ( unsigned short payloadType, int portalID, int frameCounter, char * prefix, int prefixSize, char * payload, int payloadSize )
	{
		// Divide data into parts of 64kb
		// Format of each part: sequence(4b) size(4b) fragmentcount(4b) fragment(4b) data...

		unsigned int remaining = payloadSize;
		unsigned int fragments = ( remaining / packetSize ) + 1;

		// Prepare Buffer with preamble
        if ( !udpBuffer ) {
            udpBuffer = (char *) malloc ( UDP_DEVICEBASE_MAX_SIZE );
            if ( !udpBuffer )
                return;
        }
		UdpMessageHeader * header = ( UdpMessageHeader * ) udpBuffer;

		// Version *((short*)udpBuffer) = UDP_MSG_PROTOCOL_VERSION;
		header->version = UDP_MSG_PROTOCOL_VERSION;
		// fragment-size *((unsigned short*)udpBuffer + 1) = (unsigned short)packetSize;
		header->payloadSize = ( unsigned short ) packetSize;
		header->payloadType = payloadType;
		// Sequence number *((unsigned int*)udpBuffer + 1) = packetSequence;
		header->sequence = packetSequence;
		// overall size *((int*)udpBuffer + 2) = remaining;
		header->fileSize = remaining;

		//*((int*)udpBuffer + 3) = fragments;
		header->fragments = ( unsigned short ) fragments;
		header->portalID = portalID;

		int max_send = receiveBufferSize / packetSize;

		int sent_count = 0;

		for ( unsigned int fragment = 0; fragment < fragments; fragment++ )
		{
			//*((int*)udpBuffer + 4) = fragment;
			header->fragment = ( unsigned short ) fragment;

			unsigned int size = packetSize;
			if ( remaining < packetSize )
				size = remaining;
			remaining -= size;

			memcpy ( &header->payload, payload + ( fragment * packetSize ), size );

			size_t send_size = 20 + size;
			if ( udpAddr.sin_addr.s_addr )
			{
				int sent_size = ( int ) send ( udpSocket, ( const char * ) header, ( int ) send_size, 0 );
				if ( sent_size != ( int ) send_size )
				{
                    CLogArgID ( "SendUdpPortal: Send failed [ %i != %u ]", sent_size, ( unsigned int ) send_size );
                    
                    if ( IsValidFD ( udpSocket ) ) { LogSocketErrorF ( "DeviceBase.SendUdpPortal" ); }
				}
				sent_count++;
			}

			if ( sent_count >= max_send )
			{
				sent_count = 0;
			}
		}
		packetSequence++;
	}


	bool DeviceBase::SendTcpPortal ( unsigned short payloadType, int portalID, int frameCounter, char * prefix, int prefixSize, char * payload, int payloadSize )
	{
		CVerbVerbID ( "SendTcpPortal" );

		if ( !prefix )
			prefixSize = 0;

		if ( payloadSize >= MAX_TCP_SEND_PACKET_SIZE ) {
			CErrArgID ( "SendTcpPortal: portal packet size [ %i bytes ] exceeds [ %i ]. Send portal in parts needs implementation!", payloadSize, MAX_TCP_SEND_PACKET_SIZE );
			return true;
		}

		bool result = false;

		char sendBuffer [ 32 ];
		ComMessageHeader * header = ( ComMessageHeader * ) sendBuffer;

		packetSequence++;

		// Prepare Buffer with preamble and header : MSG_HEADER_LEN
		memcpy ( header, "MSG;", 4 );

		unsigned int msgSize = payloadSize + prefixSize + MSG_HEADER_SIZE + 4; // 4 bytes for the frameCounter
																			   // Length
		header->length = msgSize;

		// Version
		header->version = TCP_MSG_PROTOCOL_VERSION;

		header->type = ( char ) ( ( payloadType >> 4 ) & 0x3 );
		header->MessageType.payloadType = payloadType;

		// Reverse portal direction
		header->MessagePack.portalID = portalID;

		*( ( int * ) &header->payload ) = frameCounter;

		if ( !interactAddr.sin_addr.s_addr )
			return false;

		/*if ( environs::opt_useCLSForAllTraffic ) {
		if ( !aes.encCtx ) {
		CVerbVerbID ( "SendTcpPortal: Securing all traffic is enforced, but no session keys has been handshaked." );
		return false;
		}
		/// Encrypt: TODO
		}*/

		CVerbVerbArg ( "SendTcpPortal: sending [%d] bytes", msgSize );

		if ( pthread_mutex_lock ( &interactSocketLock ) ) {
			CErrID ( "SendTcpPortal: Failed to aquire mutex!" );
			return false;
		}

		do
		{
			int bytesSent = ( int ) send ( interactSocket, ( char * ) header, MSG_HEADER_SIZE + 4, MSG_NOSIGNAL | MSG_MORE );
			if ( bytesSent != ( MSG_HEADER_SIZE + 4 ) ) {
                CWarnArgID ( "SendTcpPortal: Failed to send packet header to device! sent bytes and length of packet mismatch [%i != %i].", bytesSent, ( MSG_HEADER_SIZE + 4 ) );
                
                if ( IsValidFD ( interactSocket ) ) { LogSocketErrorF ( "DeviceBase.SendTcpPortal" ); }
				break;
			}

			if ( prefixSize ) {
				CVerbVerbArg ( "SendTcpPortal: sending [%d] prefix bytes. Payload", prefixSize );

				bytesSent = ( int ) send ( interactSocket, prefix, prefixSize, MSG_NOSIGNAL | ( payloadSize ? MSG_MORE : 0 ) );
				if ( bytesSent != ( signed ) prefixSize ) {
                    CWarnArgID ( "SendTcpPortal: Failed to send prefix to device! sent bytes and length of packet mismatch [%i != %u].", bytesSent, prefixSize );
                    
                    if ( IsValidFD ( interactSocket ) ) { LogSocketErrorF ( "DeviceBase.SendTcpPortal" ); }
					break;
				}
			}

			if ( payloadSize ) {
				CVerbVerbArg ( "SendTcpPortal: sending [%d] bytes. Payload", payloadSize );

				bytesSent = ( int ) send ( interactSocket, payload, payloadSize, MSG_NOSIGNAL );
				if ( bytesSent != ( signed ) payloadSize ) {
                    CWarnArgID ( "SendTcpPortal: Failed to send payload to device! sent bytes and length of packet mismatch [%i != %u].", bytesSent, payloadSize );
                    
                    if ( IsValidFD ( interactSocket ) ) { LogSocketErrorF ( "DeviceBase.SendTcpPortal" ); }
					break;
				}
			}
			result = true;
		}
		while ( false );

		if ( pthread_mutex_unlock ( &interactSocketLock ) ) {
			CErrID ( "SendTcpPortal: Failed to release mutex!" );
		}

		return result;
	}
    
    
#ifdef ENABLE_DEVICEBASE_SINGLE_COMDAT_THREAD
    ThreadSync  * staticComDatThread    = 0;
    
    bool        staticComDatThreadRun   = false;
    bool        staticComDatChanged     = false;

#ifdef ENABLE_DEVICEBASE_WINSOCK_SINGLE_COMDAT_THREAD
	HANDLE		staticSignalSocket      = WSA_INVALID_EVENT;
#else
    int         staticSignalSocket      = INVALID_FD;
#endif

    struct sockaddr_in	signalAddr;
    
    extern int                             devicesMapLast;
    extern DeviceBase *                    devicesMap [ MAX_CONNECTED_DEVICES ];
    
    
    bool DeviceBase::StartComDat ()
    {
        CVerb ( "StartComDat" );
        
        LockAcquireVA ( native.appEnvLock, "StartComDat" );

        bool success = false;
        
        if ( !staticComDatThread )
        {
            staticComDatThread = new ThreadSync ();
            if ( !staticComDatThread )
                goto Finish;
            
            if ( !staticComDatThread->Init () )
                goto Finish;
            staticComDatThread->autoreset = true;
        }

#ifdef ENABLE_DEVICEBASE_WINSOCK_SINGLE_COMDAT_THREAD
		if ( staticSignalSocket == WSA_INVALID_EVENT ) 
		{
			staticSignalSocket = WSACreateEvent ();

			if ( staticSignalSocket == WSA_INVALID_EVENT )
				goto Finish;
		}
#endif        
        if ( !staticComDatThread->isRunning () )
        {
            staticComDatThreadRun = true;
            
            if ( !staticComDatThread->Run ( pthread_make_routine ( &DeviceBase::ComDat ), 0, "StartComDat" ) )
                goto Finish;
        }
        success = true;
        
        LockReleaseVA ( native.appEnvLock, "StartComDat" );
        
    Finish:
        return success;
    }
    
    
    void DeviceBase::StopComDat ()
    {
		CVerb ( "StopComDat" );

        staticComDatThreadRun = false;
        
        LockAcquireVA ( native.appEnvLock, "StopComDat" );
        
        if ( staticComDatThread )
        {
            ComDatSignal ();
            
            int repeats = 4;
        Retry:
            if ( repeats > 0 && staticComDatThread->isRunning () )
            {
                staticComDatThread->WaitOne ( "StopComDat", 500 );
                
                if ( staticComDatThread->isRunning () ) {
                    repeats--;
                    ComDatSignal ();
                    goto Retry;
                }
            }
            
            staticComDatThread->Join ( "StopComDat" );
            
            delete staticComDatThread;
            
            staticComDatThread = 0;            
        }

		int sock = native.udpSignalSender;

		if ( IsValidFD ( sock ) )
		{
			native.udpSignalSender = INVALID_FD;
			CSocketTraceRemove ( sock, "StopComDat: Closing", 0 );
            closesocket ( sock );
		}
        
#ifdef ENABLE_DEVICEBASE_WINSOCK_SINGLE_COMDAT_THREAD
		CloseWSAHandle_m ( staticSignalSocket );
#else
        sock = staticSignalSocket;
        
        if ( IsValidFD ( sock ) )
        {
            staticSignalSocket = INVALID_FD;
			CSocketTraceRemove ( sock, "StopComDat: Closing", 0 );
            closesocket ( sock );
        }        
#endif        
        LockReleaseVA ( native.appEnvLock, "StopComDat" );
    }
    
    
    void * DeviceBase::ComDat ( void * arg )
    {
        staticComDatThread->ResetSync ( "ComDat" );
        
		ComDatLoop ();
        
        if ( staticComDatThread ) {
            staticComDatThread->Notify ( "ComDat" );
            
            staticComDatThread->Detach ( "ComDat" );
        }
        return 0;
    }
    
    
    void ReleaseDevices ( DeviceBase ** devs, int size )
    {
        size--;
        
        for ( int i = 0; i < size; i++ ) {
			if ( devs [ i ] ) {
				UnlockDevice ( devs [ i ] );
				devs [ i ] = 0;
			}
        }
    }
    
    
    void DeviceBase::ComDatSignal ()
    {
        staticComDatChanged = true;

#ifdef ENABLE_DEVICEBASE_WINSOCK_SINGLE_COMDAT_THREAD

		if ( staticSignalSocket != WSA_INVALID_EVENT )
			WSASetEvent ( staticSignalSocket );

#else
        if ( IsInvalidFD ( native.udpSignalSender ) && !native.InitSignalSender () )
            return;
        
		int repeats = 3;
		while ( repeats > 0 ) {
			int rc = (int) sendto ( native.udpSignalSender, "s", 1, 0, ( const sockaddr * ) &signalAddr, sizeof ( sockaddr ) );
			if ( rc == 1 )
				break;
			repeats--;
			LogSocketError ();
		}
#endif
    }
    
    
	bool DeviceBase::BuildDevices ( DeviceBase ** &fdDevices, FDTYPE * &fds, int &size, int &capacity )
    {
        //CLog ( "BuildDevices" );
        
		staticComDatChanged = false;
        
		if ( !LockAcquireA ( devicesAccessMutex, "BuildDevices" ) )
			return false;

		if ( size > 1 )
			ReleaseDevices ( fdDevices, size );

		size = 0;

		int deviceCount = devicesMapLast;

		if ( deviceCount >= capacity )
		{
			capacity = deviceCount + 5;

			free_n ( fds );
			free_n ( fdDevices );

			fdDevices = ( DeviceBase ** ) malloc ( sizeof ( DeviceBase * ) * capacity );
			if ( !fdDevices )
				return false;

			fds = ( FDTYPE * ) malloc ( sizeof ( FDTYPE ) * capacity );
			if ( !fds )
				return false;
		}

		for ( int i = 1; i <= deviceCount; i++ )
		{
			DeviceBase * device = devicesMap [ i ];

			if ( device && device->deviceStatus != DeviceStatus::Deleteable )
			{
				int sock = device->comDatSocket;
				if ( IsInvalidFD ( sock ) )
					continue;

#ifdef ENABLE_DEVICEBASE_WINSOCK_SINGLE_COMDAT_THREAD
				if ( device->comDatEvent == WSA_INVALID_EVENT ) {
					HANDLE ev = WSACreateEvent ();

					if ( ev == WSA_INVALID_EVENT )
						continue;

					if ( WSAEventSelect ( sock, ev, FD_READ | FD_CLOSE ) == SOCKET_ERROR )
					{
						CErrArg ( "BuildDevices: Failed to register event [ %d ]!", WSAGetLastError () );

						WSACloseEvent ( ev );
						continue;
					}

					device->comDatEvent = ev;
				}
#endif
				IncLockDevice ( device );

				fdDevices [ size ]	= device;

#ifdef ENABLE_DEVICEBASE_WINSOCK_SINGLE_COMDAT_THREAD
				fds [ size ]		= device->comDatEvent;
#else
				( fds + size )->fd	= sock;
#endif
				size++;
			}
		}

		LockReleaseA ( devicesAccessMutex, "BuildDevices" );

		if ( !AddSignalSocket ( fds, size ) )
			return false;

		return true;
    }
    
    
    void ResetFDs ( struct pollfd * fds, int size )
    {
        for ( int i = 0; i < size; i++ ) {
            (fds + i)->events = POLLIN;
            (fds + i)->revents = 0;
        }
    }

#define POLLERRMASK	(POLLERR | POLLHUP | POLLNVAL)
    
    bool DeviceBase::AddSignalSocket ( FDTYPE * fds, int &size )
    {
#ifdef ENABLE_DEVICEBASE_WINSOCK_SINGLE_COMDAT_THREAD
		
		if ( staticSignalSocket == WSA_INVALID_EVENT )
			return false;

		fds [ size ] = staticSignalSocket;
#else
        int sigSock = staticSignalSocket;
        if ( IsInvalidFD ( sigSock ) )
        {
            sigSock = ( int ) socket ( PF_INET, SOCK_DGRAM, 0 );
            if ( IsInvalidFD ( sigSock ) )
                return false;

			CSocketTraceAdd ( sigSock, "DeviceBase AddSignalSocket sigSock" );
                       
            Zero ( signalAddr );
            
            signalAddr.sin_family       = AF_INET;
            signalAddr.sin_addr.s_addr	= htonl ( INADDR_ANY );
            signalAddr.sin_port	= htons ( 12345 );

			int short curPort		= 12345;

			int maxPort		= ( curPort + ENVIRONS_DYNAMIC_PORTS_UPSTEPS ) & 0xFFFF;

			while ( curPort < maxPort ) {
				signalAddr.sin_port	= htons ( curPort );

				int rc = ::bind ( sigSock, ( const sockaddr * ) &signalAddr, sizeof ( struct sockaddr ) );
				if ( rc == 0 )
					break;
				curPort++;
			}

			if ( curPort >= maxPort ) {
				CSocketTraceRemove ( sigSock, "AddSignalSocket: Closing sigSock.", 0 );
                closesocket ( sigSock );
				return false;
			}
            
            if ( !SetNonBlockSocket( sigSock, true, "AddSignalSocket" ) ) {
				CSocketTraceRemove ( sigSock, "AddSignalSocket: Closing sigSock.", 0 );
                closesocket ( sigSock );
                return false;
            }

			signalAddr.sin_addr.s_addr	= htonl ( INADDR_LOOPBACK );
            
            staticSignalSocket = sigSock;
        }
        
        (fds + size)->fd = sigSock;
#endif
        size++;
        return true;
    }
    
    
    void DeviceBase::ComDatLoop ( )
    {
        CVerb ( "ComDatLoop: Working thread started ..." );
        
        pthread_setname_current_envthread ( "DeviceBase.ComDatLoop" );        

#ifdef ENABLE_DEVICEBASE_WINSOCK_SINGLE_COMDAT_THREAD
		WSANETWORKEVENTS	netEvents;
#else
		char                signalBuffer [ 8 ];
#endif
        int                 capacity    = 0;
        int                 size        = 0;
		int					last		= 0;
        DeviceBase      **  fdDevices   = 0;
		FDTYPE			*   fds         = 0;

		staticComDatChanged = true;
        
        while ( staticComDatThreadRun )
        {
            if ( staticComDatChanged )
            {
				if ( !BuildDevices ( fdDevices, fds, size, capacity ) )
					break;
				last = size - 1;
            }
            else {
                if ( size <= 0 ) {
                    if ( !AddSignalSocket ( fds, size ) )
                        break;
					last = 0;
                }
            }

#ifdef ENABLE_DEVICEBASE_WINSOCK_SINGLE_COMDAT_THREAD
			WSAResetEvent ( staticSignalSocket );

			DWORD rc = WSA_WAIT_FAILED;

			if ( ( rc = WSAWaitForMultipleEvents ( size, fds, FALSE, WSA_INFINITE, FALSE ) ) == WSA_WAIT_FAILED ) {
				CVerb ( "ComDatLoop: Sockets have been closed" );
				break;
			}

			if ( !staticComDatThreadRun || rc > ( DWORD ) last )
				break;

			if ( rc == ( DWORD ) last )
				continue;

			DeviceBase * device = fdDevices [ rc ];

			if ( WSAEnumNetworkEvents ( device->comDatSocket, fds [ rc ], &netEvents ) != SOCKET_ERROR ) {
				device->ComDatHandler ();
			}
			else  {
				CVerb ( "ComDatLoop: Setting deviceStatus to Deleteable" );

				device->comDatListen = false;

				device->deviceStatus = DeviceStatus::Deleteable;
				TriggerCleanUpDevices ();

				device->comDatThread.Notify ( "ComDatLoop" );

				staticComDatChanged = true;
			}
#else
            ResetFDs ( fds, size );

			int rc = poll ( fds, size, -1 );
			if ( !staticComDatThreadRun )
				break;

			if ( rc == -1 ) {
				CVerb ( "ComDatLoop: Sockets have been closed" );
				LogSocketErrorF ( "ComDat" );
				break;
			}

			//CLogArg ( "ComDatLoop: rc [ %i ]", rc );

			if ( rc ) {
				int handled = 0;

				short ev;

				for ( int i = 0; i < last; i++ ) {
					ev = ( fds + i )->revents;

					DeviceBase * device = fdDevices [ i ];

					if ( ev & POLLIN ) {
						device->ComDatHandler ();

						handled++;
					}
					if ( ev & POLLERRMASK ) {
						CVerb ( "ComDatLoop: Setting deviceStatus to Deleteable" );

						device->comDatListen = false;

						device->deviceStatus = DeviceStatus::Deleteable;
						TriggerCleanUpDevices ();

						device->comDatThread.Notify ( "ComDatLoop" );

						staticComDatChanged = true;
						handled++;
					}

					if ( handled >= rc )
						break;
				}

				if ( handled < rc ) {
					ev = ( fds + last )->revents;
					if ( ev & POLLIN )
					{
						// drain signal socket
						rc = 1;
						while ( rc > 0 )
						{
							rc = ( int ) recv ( ( int ) staticSignalSocket, signalBuffer, sizeof ( signalBuffer ), 0 );
						}
						//CLog ( "ComDatLoop: rc signal" );
					}
				}
			}
#endif            
        }
        
        free_n ( fds );
        
        if ( fdDevices && *fdDevices && size > 1 )
            ReleaseDevices ( fdDevices, size );
        
        free_n ( fdDevices );
        
        TriggerCleanUpDevices ();
        
        CLog ( "ComDatLoop: bye bye..." );
    }
    
    
    /**
     * Note: The ComDatListener is kept alive by means of the comDatListenerAlive member which holds an SP to the parent object.
     */
    void * DeviceBase::ComDatListener ()
    {
        CVerbID ( "ComDatListener: Establish thread started ..." );
        
        pthread_setname_current_envthread ( "DeviceBase.ComDatListener" );
        
        if ( !comDatByteBuffer )
        {
            comDatByteBuffer = ( ByteBuffer * ) allocBuffer ( TCP_DEVICEBASE_START_SIZE );
            if ( !comDatByteBuffer ) {
                CErrID ( "ComDatListener: Failed to allocate memory for comDat data receiving!" );
                return NULL;
            }
        }
        
        comDatBuffer 		= BYTEBUFFER_DATA_POINTER_START ( comDatByteBuffer );
        comDatBufferSize	= comDatByteBuffer->capacity;
        comDat_Start 		= comDatBuffer;
        comDat_BufferEnd	= comDat_Start + comDatBufferSize;
        comDat_CurrentEnd 	= comDatBuffer;
        
        if ( !EstablishComDatChannel () ) {
            UpdateConnectStatus ( -20 );
            
            onEnvironsNotifier1 ( env, deviceNode->info.objID, NOTIFY_CONNECTION_COMDAT_FAILED );
            return NULL;
        }
        
        if ( !SetNonBlockSocket ( comDatSocket, true, "ComDatListener" ) )
            return 0;
        
        comDatListen        = true;

		ComDatSignal ();
        return NULL;
    }
    
    
    /**
     * Note: The ComDatHandler is kept alive by means of the comDatListenerAlive member which holds an SP to the parent object.
     */
    void DeviceBase::ComDatHandler ()
    {
        //CLogID ( "ComDatHandler" );
        
        bool        rebuild         = true;
        int 		bytesRead 		= 0;
        char		*	decrypted	= 0;
        unsigned int	packLen		= 0;
        int			currentLength	= 0;
        bool 		header_available = false;
        bool		doReset			= false;
        int			msg_Length 		= 0;
        
        int 		remainBufferSize;
        
        ComMessageHeader * header 	= ( ComMessageHeader * ) comDat_Start;
        
        while ( deviceStatus != DeviceStatus::Deleteable )
        {
            if ( doReset ) {
                CVerbVerbID ( "ComDatHandler: doReset" );
                msg_Length      = currentLength         = 0;
                comDat_CurrentEnd  = comDat_Start       = comDatBuffer;
                doReset         = header_available      = false;
                header          = ( ComMessageHeader * ) comDat_Start;
            }
            
            remainBufferSize = ( int ) ( comDat_BufferEnd - comDat_CurrentEnd );
            
            CVerbVerbArgID ( "ComDatHandler: remainBufferSize %i", remainBufferSize );
            
            if ( ( remainBufferSize < msg_Length ) && ( comDat_Start != comDatBuffer ) ) {
                refactorMallocBuffer ( comDat_Start, header, comDatBuffer, currentLength, comDat_CurrentEnd );
                
                remainBufferSize = ( int ) ( comDat_BufferEnd - comDat_CurrentEnd );
            }
            
            if ( comDatBufferSize < ( unsigned ) msg_Length ) {
                CVerbVerbArgID ( "ComDatHandler: relocating ByteBuffer - message length %u, capacity = %u", msg_Length, comDatBufferSize );
                
                if ( msg_Length > ( PARTITION_SEND_BUFFER_SIZE * 2 ) ) {
                    CErrArgID ( "ComDatHandler: Protocol error - message length [ %i bytes ] exceeds twice the max part size [ %i ]! Flushing whole stream!",
                               msg_Length, ( PARTITION_SEND_BUFFER_SIZE * 2 ) );
                    doReset = true;
                    continue;
                }
                
                ByteBuffer * newBuffer = relocateBuffer ( comDatByteBuffer, false, msg_Length + 1024 );
                if ( !newBuffer ) {
                    CErrArgID ( "ComDatHandler: Failed to relocate receive buffer to capacity %i bytes!", msg_Length + 1024 );
                    break;
                }
                
                // Copy message to new buffer
                memcpy ( BYTEBUFFER_DATA_POINTER_START ( newBuffer ), comDat_Start, currentLength );
                disposeBuffer ( comDatByteBuffer );
                
                comDatByteBuffer	= newBuffer;
                comDatBufferSize	= comDatByteBuffer->capacity;
                comDatBuffer 		= BYTEBUFFER_DATA_POINTER_START ( newBuffer );
                comDat_Start 		= comDatBuffer;
                comDat_BufferEnd	= comDat_Start + comDatBufferSize;
                comDat_CurrentEnd	= comDat_Start + currentLength;
                header 				= ( ComMessageHeader * ) comDat_Start;
                
                remainBufferSize	= ( int ) ( comDat_BufferEnd - comDat_CurrentEnd );
            }
            
            if ( IsInvalidFD ( comDatSocket ) )
                break;
            
            bytesRead = ( int ) recv ( comDatSocket, comDat_CurrentEnd, remainBufferSize, 0 );
            //CLogArgID ( "ComDatHandler: bytesRead [ %i ]", bytesRead );
            rebuild = false;
            
            if ( bytesRead < 0 ) {
                VerbLogSocketError();
                
				SOCKET_Check_Val ( check );
                
                if ( SOCKET_Check_Retry ( check ) )
                    break;
            }
            
            if ( bytesRead <= 0 )
            {
                VerbLogSocketError();
                CVerbID ( "ComDatHandler: Socket to device has been closed!!" );
                
                CVerbID ( "ComDatHandler: Setting deviceStatus to Deleteable" );
                
                deviceStatus = DeviceStatus::Deleteable;
                
                // this call delays due to waiting for the UI-thread to consume this call..
                UpdateConnectStatus ( -20 );
                
                onEnvironsNotifier1 ( env, deviceNode->info.objID, NOTIFY_CONNECTION_COMDAT_CLOSED );

				comDatListen = false;

				comDatThread.Notify ( "ComDatHandler" );
                
				ComDatSignal ();
                break;
            }
            
            comDat_CurrentEnd += bytesRead;
            currentLength += bytesRead;
            
        HandleMessage:
            if ( currentLength < 13 ) {
                CVerbVerbArgID ( "ComDatHandler: Warning - Message received is incomplete; msg [%i] Bytes [%i]. Continue receiving...", currentLength, bytesRead );
                continue;
            }
            
            if ( !header_available )
            {
                if ( comDat_Start [ 0 ] != 'M' ||  comDat_Start [ 1 ] != 'S' || comDat_Start [ 2 ] != 'G' || comDat_Start [ 3 ] != ';' )
                {
                    if ( aes.decCtx ) {
                        packLen				= *( ( unsigned int * ) comDat_Start );
                        unsigned int flags	= 0xF0000000 & packLen;
                        msg_Length			= packLen & 0xFFFFFFF;
                        
                        if ( flags & 0x40000000 ) { /// Encrypted package
                            packLen = msg_Length;
                            
                            if ( packLen > ( unsigned ) currentLength ) {
                                // We haven't received the whole message! go on receiving...
                                CVerbVerbArgID ( "ComDatHandler: Encrypted message [%i] is longer than currentLength [%i]!", packLen, currentLength );
                                continue;
                            }
                            
                            if ( AESDecrypt ( &aes, comDat_Start, &packLen, &decrypted ) )
                            {
                                if ( decrypted [ 0 ] == 'M' && decrypted [ 1 ] == 'S' && decrypted [ 2 ] == 'G' && decrypted [ 3 ] == ';' )
                                {
                                    header = ( ComMessageHeader * ) decrypted;
                                    goto HandleHeader;
                                }
                                else {
                                    if ( decrypted [ 0 ] == 'F' && decrypted [ 1 ] == 'I' && decrypted [ 2 ] == 'N' && decrypted [ 3 ] == ';' )
                                    {
                                        CVerb ( "ComDatHandler: FIN!" );
                                        break;
                                    }
                                    CErr ( "ComDatHandler: MSG preamble is missing in decrypted message!" );
                                    
                                    CVerbVerbArgID ( "ComDatHandler: decrypted [%s]", ConvertToHexSpaceString ( decrypted, packLen ) );
                                    
                                    free ( decrypted );
                                    decrypted = 0;
                                }
                            }
                        }
                    }
                    
                    CErrID ( "ComDatHandler: Protocol error - message header is missing! Flushing whole stream!" );
                    
                    CVerbVerbArgID ( "ComDatHandler: ciphers [%s]", ConvertToHexSpaceString ( msg_Start, msg_Length ) );
                    
                    doReset = true;
                    continue;
                }
                else {
                    if ( comDat_Start [ 0 ] == 'F' && comDat_Start [ 1 ] == 'I' && comDat_Start [ 2 ] == 'N' && comDat_Start [ 3 ] == ';' )
                    {
                        CVerb ( "ComDatHandler: FIN!" );
                        break;
                    }
                    header		= ( ComMessageHeader * ) comDat_Start;
                    msg_Length	= header->length;
                }
                
            HandleHeader:
                // Get size of message
                packLen = header->length;
                CVerbVerbArgID ( "ComDatHandler: length of message %i", packLen );
                
                if ( packLen > 200000000 ) {
                    CErrArgID ( "ComDatHandler: Invalid message format. Message length %u > 200000000!", packLen );
                    break;
                }
                header_available = true;
            }
            
            CVerbVerbArgID ( "ComDatHandler: msgLength %i, currentLength %i", packLen, currentLength );
            if ( packLen > ( unsigned ) currentLength ) {
                // We haven't received the whole message! go on receiving ...
                // Note: This can only happen on unencrypted streams
                CVerbVerbArgID ( "ComDatHandler: message [%i] is longer than bytes [%i] received. Continue receiving...", packLen, bytesRead );
                continue;
            }
            
            unsigned int msgType = ( unsigned int ) header->type;
            if ( msgType < MSG_TYPE_MAX_COUNT ) {
                if ( !( ( *this.*msgHandlers [ msgType ] ) ( header, true ) ) )
                    goto EndWithValue;
            }
            
            free_m ( decrypted );
            
            // Is there a message pending in stream?
            currentLength -= msg_Length;
            if ( currentLength <= 0 ) {
                doReset = true;
                continue;
            }
            
            // Continue with next message in stream
            header_available = false;
            comDat_Start += msg_Length;
            header = ( ComMessageHeader * ) comDat_Start;
            goto HandleMessage;
        }
        
    EndWithValue:
        free_n ( decrypted );
        
        if ( rebuild && !staticComDatChanged ) {
            staticComDatChanged = true;
        }
    }
#endif

} /* namespace environs */


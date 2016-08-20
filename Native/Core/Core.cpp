/**
 * Environs Core
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

#include "Core.h"
#include "Environs.Obj.h"
#include "Environs.Lib.h"
#include "Environs.Crypt.h"
#include "Environs.Utils.h"
#include "Callbacks.h"
#include "Notifications.h"
#include "Async.Worker.h"
#include "Device/Devices.h"
#include "Interop/Sock.h"
#include "Tracer.h"
#include <errno.h>

#ifndef WINDOWS_PHONE
#	include <stdlib.h>
#endif

using namespace environs::API;

// The TAG for prepending to log messages
#define CLASS_NAME	"Core . . . . . . . . . ."


namespace environs
{
	//
	// Externals

	//
	// Static class members
	volatile bool	Core::active 			= false;


	Core::Core ()
    {
        TraceCoreAdd ();

        CVerb ( "Construct..." );

		active				= false;
		allocated           = false;
		stopInProgress      = 0;
		hEnvirons			= 0;
		env					= nill;

		tcpAcceptSocket		= INVALID_FD;
		udpAcceptSocket		= INVALID_FD;
		appStatus			= APP_STATUS_ACTIVE;
		appStatusNext		= INVALID_FD;
		tcpHandlers			= 0;
        
        udpBuffer           = 0;

#ifdef ENABLE_WINSOCK_THREADED_CORE
		Zero ( events );
#endif
	}


	Core::~Core ()
	{
		CLog ( "Destruct..." );

		Release ();
        
		if ( IsValidFD ( tcpAcceptSocket ) ) {
			CSocketTraceRemove ( tcpAcceptSocket, "Destruct: Closing tcpAcceptSocket.", 0 );
            closesocket ( tcpAcceptSocket );
		}
        
        if ( IsValidFD ( udpAcceptSocket ) ) {
			CSocketTraceRemove ( udpAcceptSocket, "Destruct: Closing udpAcceptSocket.", 0 );
            closesocket ( udpAcceptSocket );
		}
        
        if ( allocated ) {
            allocated = false;
            
			LockDisposeA ( appStatusMutex );
		}
        
        free_m ( udpBuffer );

#ifdef ENABLE_WINSOCK_THREADED_CORE
		CloseWSAHandle_n ( events [ 0 ] );
		CloseWSAHandle_n ( events [ 1 ] );
		CloseWSAHandle_n ( events [ 2 ] );
#endif        
        CLog ( "Destruct done." );

        TraceCoreRemove ();
	}


	void Core::MediatorEvent ( void * eventArg )
	{
		CLog ( "MediatorEvent" );
	}


	int Core::onPreInit ()
	{
		return true;
	}


	int Core::onInitialized ()
	{
		return true;
	}

    
    void Core::GenerateRandomDeviceID ( Instance * env )
    {
        CVerb ( "GenerateRandomDeviceID" );
        
        srand ( (unsigned) GetEnvironsTickCount () );
        
        int randID = 0;
        while ( randID == 0 ) {
            randID = rand () % 0xFFFFFF;
        }
        
        unsigned int ip = Mediator::GetLocalIP ();
        
        randID <<= 8;
        randID |= (ip & 0xFF);
        
        environs::API::SetDeviceID ( env->hEnvirons, randID );
    }
    

	bool Core::EstablishMediator ( Instance * env )
    {
		CVerb ( "EstablishMediator" );

        sp ( MediatorClient ) mediator = env->mediator MED_WP;
		if ( !mediator ) {
			mediator = sp_make ( MediatorClient );

			if ( !mediator || !mediator->Init ( env->myself ) ) {
				CErr ( "EstablishMediator: Failed to initialize Mediator object." );
				return false;
            }

            env->mediatorSP = mediator;
            env->mediator = env->mediatorSP;
        }
        
        if ( env->useDefaultMediator ) {
            if ( !mediator->AddMediator ( inet_addr ( env->DefaultMediatorIP ), env->DefaultMediatorPort ) ) {
                CErr ( "EstablishMediator: Failed to add default Mediator!" );
                return false;
            }
        }
        
        if ( env->useCustomMediator ) {
            if ( !mediator->AddMediator ( env->CustomMediatorIP, env->CustomMediatorPort ) ) {
                CErr ( "EstablishMediator: Failed to add custom Mediator!" );
                return false;
            }
        }
        
        mediator->BuildBroadcastMessage ();
		return true;
    }
    
    
    bool Core::InitCrypt ()
    {
        if ( opt_pubCert )
            return true;
        
        opt_pubCert = LoadPrivateBinary ( BuildDataStorePath ( ENVIRONS_PUBLIC_CERTNAME ), 0 );
        
		if ( opt_privKey ) DisposePrivateKey ( ( void ** ) &opt_privKey );

#ifdef USE_WIN32_CLIENT_CACHED_PRIVKEY
		if ( g_hPrivKey ) { CryptDestroyKey ( g_hPrivKey ); g_hPrivKey = 0; }
		if ( g_hPrivKeyCSP ) { CryptReleaseContext ( g_hPrivKeyCSP, 0 ); g_hPrivKeyCSP = 0; }
#endif
        opt_privKey = LoadPrivateBinary ( BuildDataStorePath ( ENVIRONS_PRIVATE_KEYNAME ), 0 );
        
        //CLogArg ( "Init: opt_privKey [%s]", ConvertToHexSpaceString (opt_privKey, *((unsigned int *)opt_privKey)) );
        
        /// Make sure that we have private key / public certificate
        if ( !opt_pubCert || !opt_privKey )
        {
            CVerbVerb ( "InitCrypt: Generating RSA keys ..." );
            
            free_m ( opt_pubCert );
            free_m ( opt_privKey );
            
            onEnvironsMsgNotifier1 ( env, 0, SOURCE_NATIVE, "Generating RSA security, this may take a while..." );
            
            if ( !GenerateCertificate ( &opt_privKey, &opt_pubCert ) ) {
                CErr ( "InitCrypt: Failed to create RSA keys." ); return false;
            }
            
            //CLogArg ( "Init: opt_privKey [%s]", ConvertToHexSpaceString (opt_privKey, *((unsigned int *)opt_privKey)) );
            if ( !SavePrivateBinary ( BuildDataStorePath ( ENVIRONS_PRIVATE_KEYNAME ), opt_privKey, ( *( ( unsigned int* ) opt_privKey ) & 0xFFFF ) + 4 ) ) {
                CErr ( "InitCrypt: Failed to privately store RSA private key." );
#ifdef USE_OPENSSL
                opt_privKey = 0;
#endif
                return false;
            }
            
            if ( !SavePrivateBinary ( BuildDataStorePath ( ENVIRONS_PUBLIC_CERTNAME ), opt_pubCert, ( *( ( unsigned int* ) opt_pubCert ) & 0xFFFF ) + 4 ) ) {
                CErr ( "InitCrypt: Failed to privately store RSA public certificate." ); return false;
            }
        }
        
        if ( !PreparePrivateKey ( &opt_privKey ) ) {
            CErr ( "InitCrypt: Preparation of private key failed." ); return false;
        }

#ifdef USE_WIN32_CLIENT_CACHED_PRIVKEY
		if ( !CryptAcquireContext ( &g_hPrivKeyCSP, NULL, 0, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT ) ) {
			CErr ( "InitCrypt: CryptAcquireContext failed." );
			return false;
		}

		if ( !CryptImportKey ( g_hPrivKeyCSP, ( BYTE * ) (opt_privKey + 4), *( reinterpret_cast<unsigned int *>( opt_privKey ) ), NULL, 0, &g_hPrivKey ) ) {
			CErr ( "InitCrypt: CryptImportKey failed." );
			return false;
		}
#endif        
        if ( opt_pubCert ) {
            /// Add padding id
            *( ( unsigned int * ) opt_pubCert ) |= env->CLSPadding; // 'o' << 24;
        }
        
        return true;
    }
    
    
    int Core::Init ( Instance * obj )
	{
		CVerb ( "Init" );

		if ( !obj ) return 0;
		env = obj;
		hEnvirons = env->hEnvirons;

#ifdef ENABLE_WINSOCK_THREADED_CORE
		CreateWSAHandle ( events [ 0 ], false );
		CreateWSAHandle ( events [ 1 ], false );
		CreateWSAHandle ( events [ 2 ], false );
#endif
		if ( !allocated ) {
			if ( !LockInitA ( appStatusMutex ) )
				return false;

			if ( !stopSync.Init () )
                return false;
            
            if ( !appStatusThread.Init () )
                return false;
            
			if ( !tcpHandlerSync.Init () )
				return false;
            
            if ( !thread.Init () )
                return false;

            allocated = true;
		}

		if ( !env->notificationQueue.Start () ) {
			CErr ( "Init: Failed to start Notification Queue." );
			return 0;
		}

		if ( !env->asyncWorker.Start () ) {
			CErr ( "Init: Failed to start async worker." );
			return 0;
		}

		if ( !LockAcquireA ( native.transitionLock, "Init" ) )
			return false;

		bool ret = false;

		onEnvironsNotifier1 ( env, NOTIFY_START_INIT );

		do
        {
			if ( !onPreInit () ) {
				CErr ( "Init: PreInit Failed." ); break;
			}
            
            free_n ( opt_pubCert );
            
            if ( !InitCrypt () )
                break;

			if ( !EstablishMediator ( env ) )
                break;

			if ( !onInitialized () ) {
				CErr ( "Init: PostInit Failed." ); break;
			}

			if ( !SetEnvironsState ( env, environs::Status::Initialized ) ) {
				CErr ( "Init: Failed to enter initialized state." ); break;
			}

			if ( !SetEnvironsState ( env, environs::Status::Stopped ) ) {
				CErr ( "Init: Failed to enter stopped state." ); break;
			}
			ret = true;
		}
		while ( 0 );

		if ( !ret )
			env->environsState = environs::Status::Uninitialized;

		LockReleaseVA ( native.transitionLock, "Init" );

		onEnvironsNotifier1 ( env, ret ? NOTIFY_START_INIT_SUCCESS : NOTIFY_START_INIT_FAILED );

		return ret;
	}


	void Core::Release ()
	{
		CVerb ( "Release" );

        if ( env->environsState >= environs::Status::Started )
            Stop ();

		LockAcquireVA ( native.transitionLock, "Release" );
        
#ifdef MEDIATOR_WP
        if ( env->mediatorSP )
            env->mediatorSP.reset ();
#else
        if ( env->mediator )
            env->mediator.reset ();
#endif
        free_m ( opt_pubCert );
        
		if ( opt_privKey ) {
			DisposePrivateKey ( ( void ** ) &opt_privKey );
		}

#ifdef USE_WIN32_CLIENT_CACHED_PRIVKEY
		if ( g_hPrivKey ) { CryptDestroyKey ( g_hPrivKey ); g_hPrivKey = 0; }
		if ( g_hPrivKeyCSP ) { CryptReleaseContext ( g_hPrivKeyCSP, 0 ); g_hPrivKeyCSP = 0; }
#endif

		environs::Status_t status = environs::Status::Uninitialized;
		env->environsState = status;

		LockReleaseVA ( native.transitionLock, "Release" );

		onEnvironsNotifier1 ( env, 0, status, SOURCE_NATIVE, NOTIFY_TYPE_STATUS );
	}


	/// This method is potentially called by the UI thread, hence we spawn a worker thread to process this request
	void Core::SetAppStatus ( int status )
	{
		CVerb ( "SetAppStatus" );

		if ( env->environsState < environs::Status::Started )
			return;

		if ( !LockAcquireA ( appStatusMutex, "SetAppStatus" ) )
			return;

		int nextStatus = 0;

		if ( status == appStatus ) {
			appStatusNext = -1;
			goto Finish;
		}

		nextStatus = appStatusNext;
		if ( nextStatus == status )
			goto Finish;

		appStatusNext = status;

		if ( !appStatusThread.Run ( pthread_make_routine ( &Core::StartAppStatusUpdater ), this, "SetAppStatus" ) )
		{
			CErr ( "SetAppStatus: Failed to create updating thread!" );
		}

	Finish:
		LockReleaseA ( appStatusMutex, "SetAppStatus" );
	}


	void * Core::StartAppStatusUpdater ( void * object )
	{
		( ( Core* ) object )->AppStatusUpdater ();
		return 0;
	}


	void Core::AppStatusUpdater ()
	{
		bool unlock = true;
		sp ( MediatorClient ) mediator;

		Sleep ( 1000 );

		if ( !LockAcquireA ( appStatusMutex, "AppStatusUpdater" ) ) {
			unlock = false; goto Finish;
		}

		if ( appStatusNext == -1 || appStatus == appStatusNext ) {
			appStatusNext = -1; goto Finish;
		}

		appStatus = appStatusNext;
		appStatusNext = -1;

		unlock = false;
		if ( !LockReleaseA ( appStatusMutex, "AppStatusUpdater" ) )
			goto Finish;
        
        mediator = env->mediator MED_WP;
        
		if ( mediator && mediator->IsRegistered () )
			mediator->GetDevicesFromMediatorCount ();

		switch ( appStatus )
		{
		case APP_STATUS_ACTIVE:
			break;

		case APP_STATUS_SLEEPING:
			break;
		}

		/*if ( !LockAcquireA ( appStatusMutex, "AppStatusUpdater" ) ) {
			unlock = false; goto Finish;
		}*/

	Finish:
		appStatusThread.Detach ( "App status" );

		if ( unlock ) {
			LockReleaseVA ( appStatusMutex, "AppStatusUpdater" );
		}
	}


	int Core::onPreStart ()
	{
		return true;
	}


	int Core::onStarted ()
	{
		return true;
	}


	void Core::DetermineDefaultDisplay ()
	{
		native.display.width        = 1920;
		native.display.height       = 1080;
		native.display.width_mm     = 1920;
		native.display.height_mm    = 1080;
		native.display.dpi          = 96;
	}


	int Core::Start ()
	{
		CVerb ( "Start" );
		
		if ( native.useLogFile )
			CheckLog ();

		EnvironsAndroidNullEnv ();

		/// Make sure that we have a device name and uid
		AllocNativePlatform ();

		if ( native.display.width == ENVIRONS_DISPLAY_UNINITIALIZED_VALUE )
			DetermineDefaultDisplay ();
        
#ifdef ENABLE_DEVICEBASE_SINGLE_COMDAT_THREAD
        
        if ( !DeviceBase::StartComDat () )
            return false;
#endif        
		if ( !LockAcquireA ( native.transitionLock, "Start" ) )
			return false;

		unsigned short	curPort;
		unsigned int	maxPort;
		bool			success = false;
		sp ( MediatorClient ) mediator;

		int rc, value, sock;
		struct sockaddr_in	addr;
		bool doStop = true;

		if ( !SetEnvironsState ( env, environs::Status::Starting ) ) {
			CWarn ( "Start: Environs is already started/starting." );
			doStop = false;
			goto Finish;
		}

        active = true;

		onEnvironsNotifier1 ( env, NOTIFY_START_IN_PROGRESS );

		UpdateNetworkStatus ();

#ifdef INCREASE_TCP_SOCKET_BUFFERS
		int retSize;
#endif
		if ( !onPreStart () )
            goto Finish;
        
        if ( !InitCrypt () )
            goto Finish;
        
        if ( !udpBuffer ) {
            udpBuffer = ( char * ) malloc ( UDP_MAX_SIZE + 2 );
            if ( !udpBuffer ) {
                CErr ( "Start: Failed to allocate memory for udp buffer!" );
                goto Finish;
            }
        }
        
        if ( IsValidFD ( tcpAcceptSocket ) || IsValidFD ( udpAcceptSocket ) ) {
            StopListener ( true );
        }
        
        // Create tcp accept socket
        sock = ( int ) socket ( PF_INET, SOCK_STREAM, 0 );

		if ( IsInvalidFD ( sock ) ) {
            CErr ( "Start: Failed to create tcp socket!!!" ); LogSocketError ();
			goto Finish;
		}
		CSocketTraceAdd ( sock, "Core.Start tcpAcceptSocket" );
        tcpAcceptSocket = sock;
        
		DisableSIGPIPE ( tcpAcceptSocket );

		value = 1;
		rc = setsockopt ( tcpAcceptSocket, IPPROTO_TCP, TCP_NODELAY, ( const char * ) &value, sizeof ( value ) );
		if ( rc < 0 ) {
            CErr ( "Start: Failed to set TCP_NODELAY on communication socket" );
            
            if ( IsValidFD ( tcpAcceptSocket ) ) { LogSocketErrorF ( "Core.Start" ); }
		}

#ifdef INCREASE_TCP_SOCKET_BUFFERS
		socklen_t retSize = sizeof ( value );

		// - Load send buffer size
		rc = getsockopt ( tcpAcceptSocket, SOL_SOCKET, SO_RCVBUF, ( char * ) &value, &retSize );
		if ( rc < 0 ) {
			CErr ( "Start: Failed to retrieve tcp receive buffer size!" ); LogSocketError ();
		}
		else
			CLogArg ( "Start: receive buffer size [%i]", value );

		value *= 4;
		rc = setsockopt ( tcpAcceptSocket, SOL_SOCKET, SO_RCVBUF, ( const char * ) &value, sizeof ( value ) );
		if ( rc < 0 ) {
			CErr ( "Start: Failed to set receive buffer size on communication socket." ); LogSocketError ();
			//goto Finish;
		}

		// - Load send buffer size
		retSize = sizeof ( value );
		value = 0;
		rc = getsockopt ( tcpAcceptSocket, SOL_SOCKET, SO_SNDBUF, ( char * ) &value, &retSize );
		if ( rc < 0 ) {
			CErr ( "Start: Failed to retrieve tcp send buffer size!" ); LogSocketError ();
		}
		else
			CLogArg ( "Start: send buffer size [%i]", value );

		// We should set the buffer size to that of the mobileclient to ensure fill?
		//value = PORTAL_SOCKET_BUFFER_SIZE_NORMAL;
		value *= 4;
		rc = setsockopt ( tcpAcceptSocket, SOL_SOCKET, SO_SNDBUF, ( const char * ) &value, sizeof ( value ) );
		if ( rc < 0 ) {
			CErr ( "Start: Failed to set send buffer size on communication socket." ); LogSocketError ();
			//return false;
		}
#endif
        if ( !SetNonBlockSocket ( tcpAcceptSocket, true, "Core.Start" ) )
            goto Finish;

#ifdef ENABLE_WINSOCK_THREADED_CORE
		if ( WSAEventSelect ( tcpAcceptSocket, events [ 0 ], FD_ACCEPT | FD_CLOSE ) == SOCKET_ERROR )
		{
			CErrArg ( "Start: Failed to register tcpSocket event [ %d ]!", WSAGetLastError () );
			goto Finish;
		}
#endif
		Zero ( addr );
		addr.sin_family			= AF_INET;
		addr.sin_addr.s_addr	= htonl ( INADDR_ANY );

		curPort		= Instance::tcpPortLast ? Instance::tcpPortLast : env->tcpPort;

		maxPort		= ( curPort + ENVIRONS_DYNAMIC_PORTS_UPSTEPS ) & 0xFFFF;

		while ( curPort < maxPort ) {
			addr.sin_port	= htons ( curPort );

			rc = ::bind ( tcpAcceptSocket, ( const sockaddr * ) &addr, sizeof ( struct sockaddr ) );
			if ( rc == 0 )
				break;
			curPort++;
		}

		if ( rc < 0 ) {
            CErrArg ( "Start: Failed to bind communication socket on port [ %u ... %u + %u ]", env->tcpPort, env->tcpPort, ENVIRONS_DYNAMIC_PORTS_UPSTEPS );
            
            if ( IsValidFD ( tcpAcceptSocket ) ) { LogSocketErrorF ( "Core.Start" ); }
            
			onEnvironsNotifier1 ( env, NOTIFY_SOCKET_BIND_FAILED );
			goto Finish;
		}

		env->tcpPort            = curPort;
        Instance::tcpPortLast   = curPort;

		CVerbsArg ( 2, "Start: Tcp acceptor bound to port [ %i ]", env->tcpPort );

		rc = listen ( tcpAcceptSocket, SOMAXCONN );
		if ( rc < 0 ) {
            CErrArg ( "Start: Failed to listen on communication socket on port [ %i ]", env->tcpPort );
            
            if ( IsValidFD ( tcpAcceptSocket ) ) { LogSocketErrorF ( "Core.Start" ); }
            
			onEnvironsNotifier1 ( env, NOTIFY_SOCKET_LISTEN_FAILED );
			goto Finish;
        }
        
        // Create data acceptor socket
		sock = ( int ) socket ( PF_INET, SOCK_DGRAM, 0 );

		if ( IsInvalidFD ( sock ) ) {
			CErr ( "Start: Failed to create data acceptor socket!!!" ); LogSocketError ();
			goto Finish;
		}
        CSocketTraceAdd ( sock, "Core.Start udpAcceptSocket" );
        udpAcceptSocket = sock;
        
        DisableSIGPIPE ( udpAcceptSocket );
        
        if ( !SetNonBlockSocket ( udpAcceptSocket, true, "Core.Start" ) )
            goto Finish;

#ifdef ENABLE_WINSOCK_THREADED_CORE
		if ( WSAEventSelect ( udpAcceptSocket, events [ 1 ], FD_READ | FD_CLOSE ) == SOCKET_ERROR )
		{
			CErrArg ( "Start: Failed to register udpSocket event [ %d ]!", WSAGetLastError () );
			goto Finish;
		}

        // Reset dedicated thread signal event
		WSAResetEvent ( events [ 2 ] );
#endif
		addr.sin_family			= AF_INET;
		addr.sin_addr.s_addr	= htonl ( INADDR_ANY );;

		curPort		= Instance::udpPortLast ? Instance::udpPortLast : env->udpPort;

		maxPort		= ( curPort + ENVIRONS_DYNAMIC_PORTS_UPSTEPS ) & 0xFFFF;

		while ( curPort < maxPort ) {
			addr.sin_port	= htons ( curPort );

			rc = ::bind ( udpAcceptSocket, ( const sockaddr * ) &addr, sizeof ( struct sockaddr ) );
			if ( rc == 0 )
				break;
			curPort++;
		}

		if ( rc < 0 ) {
            CErrArg ( "Start: Failed to bind udp socket to port [ %u ... %u + %u ]", env->udpPort, env->udpPort, ENVIRONS_DYNAMIC_PORTS_UPSTEPS );
            
            if ( IsValidFD ( udpAcceptSocket ) ) { LogSocketErrorF ( "Core.Start" ); }
            
			onEnvironsNotifier1 ( env, NOTIFY_SOCKET_BIND_FAILED );
			goto Finish;
		}

        env->udpPort            = curPort;
        Instance::udpPortLast   = curPort;

        CVerbsArg ( 2, "Start: Udp acceptor bound to port [ %u ]", env->udpPort );
        
        thread.ResetSync ( "Start" );
        
        threadAlive = env->myself;
        
        if ( !threadAlive || ( !thread.isRunning () && !thread.Run ( pthread_make_routine ( &Core::StartListener ), this, "Core: Start" ) ) )
        {
            CErr ( "Start: Failed to create thread!" );
            
            threadAlive = 0;
            goto Finish;
        }
        
        // Start everything else ...
		if ( !EstablishMediator ( env ) )
            goto Finish;
        
        if ( !env->deviceID && !env->useDefaultMediator && !env->useCustomMediator ) {
            GenerateRandomDeviceID ( env );
        }

		env->asyncWorker.StartSend ( 0 );
        
        mediator = env->mediator MED_WP;
        if ( !mediator )
            goto Finish;
        
		if ( mediator->Start () ) {
			mediator->RegisterAtMediators ( false );

			if ( !onStarted () )
				goto Finish;

            if ( SetEnvironsState ( env, environs::Status::Started ) ) {
                __sync_add_and_fetch ( &native.coresStarted, 1 );
				success = true;
            }
		}
        
	Finish:
		LockReleaseA ( native.transitionLock, "Start" );

        // This release is required as we may call Stop later which (fails and) waits for stopSync
        // In this case, another Stop is waiting for all mediator SP refs are released.
        // If we don't release here, then there will be a deadlock
        if ( mediator )
            mediator.reset ();

		if ( success ) {
			CInfoArg ( "Start: Successful. Tcp [ %i ], Udp [ %i ]", env->udpPort, env->udpPort );
			onEnvironsNotifier1 ( env, NOTIFY_START_SUCCESS );
		}
		else {
			if ( doStop ) {
				CErr ( "Start: Failed" );

				onEnvironsNotifier1 ( env, NOTIFY_START_FAILED );

				// Create a clean and defined core state
				Stop ();
			}
		}

		stopSync.ResetSync ( "Start" );

		return success;
	}


	int Core::onPreStop ()
    {
        CVerb ( "onPreStop" );
		return true;
	}


	int Core::onStopped ()
    {
        CVerb ( "onStopped" );
		return true;
    }
    
    
    void Core::StopListener ( bool wait )
    {
        CVerb ( "StopListener" );        
        
        if ( wait )
        {
#ifdef ENABLE_WINSOCK_THREADED_CORE
			WSASetEvent ( events [ 2 ] );

			thread.Join ( "Core Listener" );

			int sockS = tcpAcceptSocket;
			tcpAcceptSocket = INVALID_FD;

			int sockD = udpAcceptSocket;
			udpAcceptSocket = INVALID_FD;

			if ( IsValidFD ( sockS ) ) {
				if ( WSAEventSelect ( sockS, NULL, 0 ) ) {
					CErr ( "StopListener: Failed to deassoc event from tcpAcceptSocket." );
				}

				ShutdownCloseSocket ( sockS, true, "Core.StopListener tcpAcceptSocket" );
			}

			if ( IsValidFD ( sockD ) ) {
				if ( WSAEventSelect ( sockD, NULL, 0 ) ) {
					CErr ( "StopListener: Failed to deassoc event from udpAcceptSocket." );
				}

				ShutdownCloseSocket ( sockD, true, "Core.StopListener udpAcceptSocket" );
			}
#else
            int sockS = tcpAcceptSocket;
			tcpAcceptSocket = INVALID_FD;

            int sockD = udpAcceptSocket;            
            udpAcceptSocket = INVALID_FD;

			// There's a (even little) chance for a race condition if the handler thread has acquired the socket fd
			// and is paused before going into the syscall
			// We wait for 1s to mitigate that, then close the socket and risk that the thread handler goes into
			// the syscall with a file descriptor that has meanwhile been assigned to something else ...

            if ( thread.isRunning () )
            {
                if ( IsValidFD ( sockS ) ) {
                    FakeConnect ( sockS, env->tcpPort );
                    
                    ShutdownCloseSocket ( sockS, false, "Core.StopListener tcpAcceptSocket" );
                }
                if ( IsValidFD ( sockD ) ) {
                    MediatorClient::SendUDPFIN ( sockD );
                    
                    ShutdownCloseSocket ( sockD, false, "Core.StopListener udpAcceptSocket" );
                }

                thread.WaitOne ( "StopListener", 1000 );
            }

            ShutdownCloseSocket ( sockS, true, "Core.StopListener tcpAcceptSocket" );
            ShutdownCloseSocket ( sockD, true, "Core.StopListener udpAcceptSocket" );

			thread.Join ( "Core Listener" );
#endif
        }
        else
        {
#ifdef ENABLE_WINSOCK_THREADED_CORE
			WSASetEvent ( events [ 2 ] );
#else
            if ( thread.isRunning () )
            {
                int sockS = tcpAcceptSocket;
                if ( IsValidFD ( sockS ) )
                    FakeConnect ( sockS, env->tcpPort );
                
                int sockD = udpAcceptSocket;
                if ( IsValidFD ( sockD ) ) {
                    MediatorClient::SendUDPFIN ( sockD );
                }
            }
#endif
        }
    }
    
    
    void Core::StopNetLayer ()
    {
        CVerb ( "StopNetLayer" );
        
        if ( !SetEnvironsState ( env, environs::Status::Stopping ) )
            return;

		active = false;
        
        onEnvironsNotifier1 ( env, NOTIFY_STOP_BEGIN );

		PrepareRemovalOfAllDevices ( env );
        
        sp ( MediatorClient ) mediator = env->mediator MED_WP;
        if ( mediator )
            mediator->Stop ( false );
        
        // Disable sending by AsyncWorker
        // Wait will follow later as he socket through which the worker is sending
        // will be closed by a call to remove all devices
        env->asyncWorker.SignalStopSend ();
        
        StopListener ( false );
    }


	int Core::Stop ()
	{
		CVerb ( "Stop" );

		active = false;

		int						waits = 0;
		int						success = 0;
		sp ( MediatorClient )	mediator;
        
		if ( ___sync_val_compare_and_swap ( &stopInProgress, ENVIRONS_THREAD_NO_THREAD, ENVIRONS_THREAD_DETACHEABLE ) != ENVIRONS_THREAD_NO_THREAD )
		{
			CVerb ( "Stop: Another thread is already doing the stop." );
			goto WaitAndFinish;
		}

		stopSync.ResetSync ( "Stop" );

        StopNetLayer ();

		if ( !SetEnvironsState ( env, environs::Status::StopInProgress ) )
            goto Finish;

        __sync_sub_and_fetch ( &native.coresStarted, 1 );

		onEnvironsNotifier1 ( env, NOTIFY_STOP_IN_PROGRESS );

		onPreStop ();
        
        StopListener ( true );
        
        mediator = env->mediator MED_WP;
        if ( mediator ) 
		{
            mediator->Stop ( true );
            
			if ( !LockAcquireA ( native.transitionLock, "Stop" ) )
				return 0;
            
#ifdef MEDIATOR_WP
            env->mediatorSP.reset ();
#else
            env->mediator.reset ();
#endif
			LockReleaseVA ( native.transitionLock, "Stop" );

			mediator->BroadcastByeBye ();
		}

		// Wait for AsyncWorker to stop and empty the send queue
		env->asyncWorker.StopSend ();

		tcpHandlerSync.ResetSync ( "Core.Stop" );

		while ( tcpHandlers > 0 ) {
			CLog ( "Stop: Waiting for tcp acceptors ..." );

			tcpHandlerSync.WaitOne ( "tcpHandlerSync", 1000 );
		}

		RemoveAllDevices ( env );

#ifdef ENABLE_DEVICEBASE_SINGLE_COMDAT_THREAD
        if ( coresAlive == 1 )
            DeviceBase::StopComDat ();
#endif
		while ( stuntCount > 0 )
		{
			if ( !( waits % 50 ) ) {
				CWarnArg ( "Stop: [ %i stunts ] still alive ...", stuntCount );
			}
			waits++;
			Sleep ( 600 );
		}

		while ( stunCount > 0 )
		{
			if ( !( waits % 50 ) ) {
				CWarnArg ( "Stop: [ %i stuns ] still alive ...", stunCount );
			}
			waits++;
			Sleep ( 600 );
        }

        // Wait for all mediator references to be released
        if ( mediator )
        {
            while ( mediator.use_count() > 1 )
            {
                if ( !( waits % 50 ) ) {
                    CWarnArg ( "Stop: [ %i mediator ] still alive ...", mediator.use_count() );
                }
                waits++;

                if ( mediator->isRunning )
                    mediator->Stop ( true );

                // Another start could have failed, invoked Stop (also failed as we are doing the stop already)
                // and is now waiting for the stopSync with a mediator SP ...
                Sleep ( 600 );
            }

            // This is the last reference to the MediatorClient. We will call delete here.
            mediator.reset ();
        }
        
        onStopped ();

		if ( !SetEnvironsState ( env, environs::Status::Stopped ) )
			goto Finish;

		onEnvironsNotifier1 ( env, NOTIFY_STOP_SUCCESS );

		success = 1;

	Finish:
		stopSync.Notify ( "Stop" );
		stopInProgress = ENVIRONS_THREAD_NO_THREAD;
		return success;

	WaitAndFinish:
		// Wait here
		stopSync.WaitOne ( "Stop" );
		return success;
    }


	class TcpAcceptorContext
	{
	public:
#ifdef TRACE_TCP_ACCEPTOR_CONTEXTS
		TcpAcceptorContext () {
			TraceTcpAcceptorAdd ( this );
		}

		~TcpAcceptorContext () {
			TraceTcpAcceptorRemove ( this );
		}
#endif
		sp ( Instance )		env;
		int					sock;
		struct sockaddr_in	addr;

		EnvThread           thread;

		sp ( TcpAcceptorContext ) myself;
	};


	void * Core::TcpHandler ( void * arg )
	{
#ifndef NDEBUG
		try {
#endif
		TcpAcceptorContext * context = ( TcpAcceptorContext * ) arg;

		CVerb ( "TcpHandler started..." );

#ifdef ENABLE_WINSOCK_THREADED_CORE
		if ( WSAEventSelect ( context->sock, NULL, 0 ) ) {
			CErr ( "TcpHandler: Failed to deassoc event from socket." );
		}
#endif        
        SetNonBlockSocket ( context->sock, false, "Core.TcpHandler" );
        
		Instance * env = context->env.get ();

		DeviceBase::HandshakeAndResponse ( env, context->sock, &context->addr );

		CVerb ( "TcpHandler: done..." );

		if ( __sync_sub_and_fetch ( &env->kernel->tcpHandlers, 1 ) == 0 )
		{
			// If we're the last one, then notify any waiting instance ...

			env->kernel->tcpHandlerSync.Notify ( "TcpHandler" );
		}

		context->myself = 0;
		
#ifndef NDEBUG
		}
		catch ( char * )
		{
            printf ( "TcpHandler: Exception !!!\n" );
			_EnvDebugBreak ( "TcpHandler" );
		}
#endif
		return 0;
	}
    
    
    void * Core::StartListener ( void * object )
    {
        Core * core = ( Core * ) object;
        
        core->Listener ();
        
        core->thread.Notify ( "StartListener" );
        
        core->thread.Detach ( "StartListener" );
        
        core->threadAlive.reset ();
        
        return 0;
    }
    
    
    void Core::Listener ()
    {
        CVerb ( "Listener: created." );
        
        pthread_setname_current_envthread ( "Core.Listener" );
        
        onEnvironsNotifier1 ( env, NOTIFY_START_LISTEN_SUCCESS );

#ifdef ENABLE_WINSOCK_THREADED_CORE
		DWORD eventNr;
		WSANETWORKEVENTS netEvents;

		while ( active )
		{
			if ( ( eventNr = WSAWaitForMultipleEvents ( 3, events, FALSE, WSA_INFINITE, FALSE ) ) == WSA_WAIT_FAILED )
			{
				CVerb ( "Listener: Wait error!" );
				break;
			}

			if ( !active )
				break;

			if ( eventNr == WAIT_OBJECT_0 ) 
			{
				if ( WSAEnumNetworkEvents ( tcpAcceptSocket, events [ 0 ], &netEvents ) != SOCKET_ERROR )
				{
					while ( TcpAcceptor () );
				}
				//WSAResetEvent ( events [ 0 ] );
				//while ( TcpAcceptor () );
			}
			else if ( eventNr == WAIT_OBJECT_0 + 1 ) 
			{
				if ( WSAEnumNetworkEvents ( udpAcceptSocket, events [ 1 ], &netEvents ) != SOCKET_ERROR )
				{
					while ( UdpAcceptor () );
				}
				//WSAResetEvent ( events [ 1 ] );
				//while ( UdpAcceptor () );
			}
			else
				break;
		}
#else
        struct pollfd desc [ 2 ];
        
        desc[0].events = POLLIN;
        desc[1].events = POLLIN;

		while ( active )
		{
			desc [ 0 ].fd = tcpAcceptSocket;
			desc [ 0 ].revents = 0;

			desc [ 1 ].fd = udpAcceptSocket;
			desc [ 1 ].revents = 0;

			int rc = poll ( desc, 2, -1 );

			if ( !active )
				break;

			if ( rc == -1 ) {
				CVerb ( "Listener: Socket has been closed" );
				LogSocketErrorF ( "Core.Listener" );
				break;
			}

			if ( rc ) {
				if ( desc [ 0 ].revents & POLLIN ) {

					if ( !TcpAcceptor () )
						break;
                    rc--;
				}

				if ( desc [ 1 ].revents & POLLIN ) {

					if ( !UdpAcceptor () )
                        break;
                    rc--;
				}

                if ( rc > 0 )
                    break;
			}
		}
#endif        
        CLog ( "Listener: bye bye ..." );
    }

#ifdef DISPLAYDEVICE
#   define MAX_TCP_HANDLERS    200
#else
#   define MAX_TCP_HANDLERS    50
#endif
    
    bool Core::TcpAcceptor ()
    {
        CVerb ( "TcpAcceptor: created." );
        
        pthread_setname_current_envthread ( "Core::TcpAcceptor" );
        
        struct sockaddr_in	addr;
        socklen_t			addrLen = sizeof ( struct sockaddr_in );        
        
        Zero ( addr );
        
        int sock = ( int ) accept ( tcpAcceptSocket, ( struct sockaddr * ) &addr, &addrLen );
        
        if ( IsInvalidFD ( sock ) ) {
            CVerb ( "TcpAcceptor: accept failed." ); //LogSocketErrorF ( "Core.TcpAcceptor" );
            return false;
        }
        
        CSocketTraceAdd ( sock, "Core TcpAcceptor" );
        
        CLogsArg ( 2, "TcpAcceptor: New connection [ %s ]", inet_ntoa ( addr.sin_addr ) );
        
        if ( env->environsState >= environs::Status::Started )
        {
			sp ( TcpAcceptorContext ) context = sp_make ( TcpAcceptorContext );
            
			if ( context && context->thread.Init () )
            {
				memcpy ( &context->addr, &addr, sizeof ( struct sockaddr_in ) );

				context->env        = env->myself;
				if ( context->env ) {
					context->sock	= sock;
					context->myself = context;

					LONGSYNC count = __sync_add_and_fetch ( &tcpHandlers, 1 );

					// Create the connect thread
					if ( count < MAX_TCP_HANDLERS && context->thread.Run ( pthread_make_routine ( &TcpHandler ), ( void * ) context.get (), "TcpAcceptor" ) ) {
						return true;
					}
					else {
						CErr ( "TcpAcceptor: Error creating thread for handling the connect." );
						__sync_sub_and_fetch ( &tcpHandlers, 1 );
						context->myself = 0;
					}
				}
			}
			else {
				CErr ( "TcpAcceptor: Failed to create a handler context!" );
			}
        }

#ifdef ENABLE_WINSOCK_THREADED_CORE
		if ( WSAEventSelect ( sock, NULL, 0 ) ) {
			CErr ( "TcpAcceptor: Failed to deassoc event from socket." );
		}
#endif
        ShutdownCloseSocket ( sock, true, "Core.TcpAcceptor sock" );
        return true;
    }
    
    
    
    bool Core::UdpAcceptor ()
    {
        CVerb ( "UdpAcceptor: created." );
        
        pthread_setname_current_envthread ( "Core::UdpAcceptor" );
        
        char * buffer = udpBuffer;
        if ( !buffer ) {
            CErr ( "UdpAcceptor: Receive buffer is missing!" );
            return false;
        }
        
        struct sockaddr_in	addr;
        Zero ( addr );
        
        socklen_t			addrLen = sizeof ( struct sockaddr_in );
        
        int bytesReceived = ( int ) recvfrom ( udpAcceptSocket, buffer, UDP_MAX_SIZE, 0, ( struct sockaddr * ) &addr, &addrLen );
        
        if ( bytesReceived <= 0 || bytesReceived > UDP_MAX_SIZE ) 
		{
#ifndef ENABLE_WINSOCK_THREADED_CORE
			CVerb ( "UdpAcceptor: Socket has been closed" );
#endif
            return false;
        }
        
        //CLogArg ( "UdpListener: received %i bytes", bytesReceived );
        buffer [ bytesReceived ] = 0;
        
        if ( buffer [ 4 ] =='E' && buffer [ 5 ] == MEDIATOR_OPT_BLANK && buffer [ 6 ] == 'D' && buffer [ 7 ] == MEDIATOR_OPT_BLANK ) {
            
            sp ( MediatorClient ) mediator = env->mediator MED_WP;
            if ( !mediator )
                return false;
            
            mediator->HandleDeviceUpdateMessage ( buffer );
            return true;
        }
        
        switch ( buffer [ 0 ] )
        {
            case 'i':
                if ( buffer [ 1 ] == 'd' && buffer [ 2 ] == ':' )
                {
                    UdpHelloPacket * packet = ( UdpHelloPacket * ) buffer;
                    
                    int deviceID = packet->deviceID;
                    
                    const char * areaName = 0;
                    const char * appName = 0;
                    if ( bytesReceived >= ( signed ) sizeof ( UdpHelloPacket ) ) {
                        if ( *packet->areaName && *packet->appName ) {
							packet->areaName[MAX_NAMEPROPERTY] = 0;
							packet->appName[MAX_NAMEPROPERTY] = 0;
							areaName = packet->areaName;
                            appName = packet->appName;
                        }
                    }
                    
                    // An incoming connection needs to be processed.
                    CVerbArgID ( "UdpAcceptor: Received connection request [ %s : %i ]", inet_ntoa ( addr.sin_addr ), ntohs ( addr.sin_port ) );
                    
                    if ( env->environsState >= environs::Status::Started )
                    {
                        DeviceBase * device = GetDevice ( env, deviceID, areaName, appName );
                        if ( device ) {
                            memcpy ( &device->udpAddr, &addr, sizeof ( struct sockaddr ) );
                            CLogsArgID ( 2, "UdpAcceptor: Taken over udp IP [ %s : %d ]", inet_ntoa ( addr.sin_addr ), ntohs ( addr.sin_port ) );

#ifdef ENABLE_DEVICEBASE_WP_STUN
							if ( !device->stun.expired () ) {
								device->udpCoreConnected = true;
							}
#else
							LockAcquireVA ( device->spLock, "UdpAcceptor" );

                            if ( device->stun ) {
                                device->udpCoreConnected = true;
                            } 

                            LockReleaseVA ( device->spLock, "UdpAcceptor" );
#endif                           
                            
                            // Send two ACKs
                            sendto ( udpAcceptSocket, "y;;;", 4, 0, ( const sockaddr * ) &addr, sizeof ( sockaddr ) );
                            sendto ( udpAcceptSocket, "y;;;", 4, 0, ( const sockaddr * ) &addr, sizeof ( sockaddr ) );
                            
                            if ( device->UdpSendHelo () )
                                device->UdpSendHelo ();
                            
                            UnlockDevice ( device );
                        }
						else {
							CWarnsArgID ( 3, "UdpAcceptor: Device not found [%s] [%s]", areaName, appName );
						}
                    }
                }
                break;
                
            case 'y':
                if ( buffer [ 3 ] == '-' ) {
                    CVerbArg ( "UdpAcceptor: Received STUN server response [%s]", inet_ntoa ( addr.sin_addr ) );
                }
                break;
        }
        return true;
    }
    

} // -> namespace


/**
 * Mediator base functionality
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
#endif

#include "Mediator.h"
#include "Environs.Native.h"
#include "Environs.Utils.h"
#include "Tracer.h"

#if !defined(MEDIATORDAEMON)
#   include "Environs.Obj.h"
#   include "Environs.Lib.h"
#   include "Device/Device.Controller.h"
#else
#   include <cstdio>

int g_Debug = 0;
#endif

#include <errno.h>
#include <fcntl.h>

#ifndef WINDOWS_PHONE
#	include <stdlib.h>
#endif

#ifndef _WIN32
#   include <signal.h>
#   include <unistd.h>

#   ifndef ANDROID
#       include <ifaddrs.h>
#   endif

#   ifdef __APPLE__
#       include <net/if_dl.h>
#   endif
#else
#	ifndef WINDOWS_PHONE
#		include <iphlpapi.h>

#		pragma comment(lib, "iphlpapi.lib")
#	endif
#endif

#include <map>
#include <string>

#define	CLASS_NAME 	"Mediator . . . . . . . ."

//#define ENABLE_LINGER_OPTIONS

#ifndef MEDIATORDAEMON
#   define ENABLE_SOCKET_CLOSE_DRAIN
#endif

#ifdef MEDIATORDAEMON
bool CloseThreadSocket ( int * psock );
#endif


/* Namespace: environs -> */
namespace environs
{
    bool                    Mediator::allocatedClass = false;
    NetPack                 Mediator::localNets;
    int                     Mediator::localNetsSize = 0;
	pthread_mutex_t         Mediator::localNetsLock;

	unsigned long long		 Mediator::wifiMAC		= 0;
	unsigned long long		 Mediator::btMAC		= 0;


    CLIENTEXP ( OBJIDTypeV  deviceInstanceObjIDs    = 1; )
    
    DeviceInstanceNode::DeviceInstanceNode ( ) : next ( 0 ), prev ( 0 )
    {
        TraceDeviceInstanceNodesAdd ( this );

        Zero ( info );

        TraceDeviceInstanceNodeAdd ();

		CLIENTEXP ( info.objID = __sync_add_and_fetch ( &deviceInstanceObjIDs, 1 ); )

		CLIENTEXP ( hEnvirons = 0; )

		CLIENTEXP ( allowConnect = false; )

#ifdef USE_MEDIATOR_OPT_KEY_MAPS_COMP
		//CLIENTEXP ( ZeroStruct ( key, AppAreaKey );  *key.appArea = 0; )
		CLIENTEXP ( *key.appArea = 0; )
#else
        CLIENTEXP ( *key = 0; )
#endif
    }
    

	ILock::ILock ()
	{
		init = false;
	}

	bool ILock::Init ()
	{
		if ( !init ) {
			Zero ( lock );
			init = LockInitA ( lock );
		}
		return init;
	}

	bool ILock::Lock ( const char * func )
	{
		return LockAcquireA ( lock, func );
	}

	bool ILock::Unlock ( const char * func )
	{
		return LockReleaseA ( lock, func );
	}

	ILock::~ILock ()
    {
        //CVerbVerb ( "~ILock" );
        
		if ( init )
			LockDisposeA ( lock );
    }


    INoRWLock::INoRWLock ()
    {
        init1 = false;
    }

    bool INoRWLock::Init ()
    {
        if ( !init1 ) {
            Zero ( lock1 );
            init1 = LockInitA ( lock1 );
        }
        return init1;
    }

    bool INoRWLock::LockRead ( const char * func )
    {
        return LockAcquireA ( lock1, func );
    }

    bool INoRWLock::UnlockRead ( const char * func )
    {
        return LockReleaseA ( lock1, func );
    }

    bool INoRWLock::LockWrite ( const char * func )
    {
        return LockAcquireA ( lock1, func );
    }

    bool INoRWLock::UnlockWrite ( const char * func )
    {
        return LockReleaseA ( lock1, func );
    }

    INoRWLock::~INoRWLock ()
    {
        //CVerbVerb ( "~ILock" );
        
        if ( init1 )
            LockDisposeA ( lock1 );
    }


    bool SetNonBlockSocket ( int &sock, bool set, const char * name )
	{
#ifdef _WIN32
		u_long flags;
#else
		long flags;
#endif

		if ( set )
		{
#ifdef _WIN32
			flags = 1;

			if ( ioctlsocket ( sock, FIONBIO, &flags ) != NO_ERROR )
#else
			flags = fcntl ( sock, F_GETFL, 0 );

            if ( flags & O_NONBLOCK )
                return true;

			if ( fcntl ( sock, F_SETFL, flags | O_NONBLOCK ) < 0 )
#endif
			{
				goto Failed;
			}
			return true;
		}

#ifdef _WIN32
		flags = 0;

		if ( ioctlsocket ( sock, FIONBIO, &flags ) != NO_ERROR )
#else
		flags = fcntl ( sock, F_GETFL, NULL );

        if ( ( flags & O_NONBLOCK ) == 0 )
            return true;

		flags &= ( ~O_NONBLOCK );

		if ( fcntl ( sock, F_SETFL, flags ) < 0 )
#endif
		{
			goto Failed;
		}

		return true;

	Failed:
		CErrArg ( "[ %s ].SetNonBlockSocket: Set socket to %sblocking mode failed!", name, set ? "non-" : "" );
		LogSocketErrorF ( "SetNonBlockSocket" );
		return false;
    }
    
    
    bool SocketTimeout ( int &sock, int recvSec, int sendSec, bool isMS )
    {
        if ( IsInvalidFD ( sock ) )
            return false;
        
        int rc = INVALID_FD;
        
#ifdef _WIN32
        DWORD tv;
#else
        struct timeval tv;
#endif
        
        if ( recvSec >= 0 )
        {
#ifdef _WIN32
            if ( isMS )
                tv = recvSec;
            else
                tv = recvSec * 1000;
#else
            if ( isMS ) {
				tv.tv_sec	= recvSec / 1000;
				tv.tv_usec	= ( recvSec % 1000 ) * 1000;
            }
            else {
                tv.tv_sec	= recvSec;
                tv.tv_usec	= 0;
            }
#endif
			rc = setsockopt ( sock, SOL_SOCKET, SO_RCVTIMEO, ( const char * ) &tv, sizeof ( tv ) );
            if ( rc < 0 ) {
#ifdef MEDIATORDAEMON
				CVerb ( "SocketTimeout: Failed to set SO_RCVTIMEO." ); VerbLogSocketError ();
#else
                CVerbs ( 2, "SocketTimeout: Failed to set SO_RCVTIMEO." ); VerbLogSocketError ();
#endif
                return false;
            }
        }
        
        if ( sendSec >= 0 )
        {
#ifdef _WIN32
            if ( isMS )
                tv = sendSec;
            else
                tv = sendSec * 1000;
#else
            if ( isMS ) {
				tv.tv_sec	= sendSec / 1000;
				tv.tv_usec	= ( sendSec % 1000 ) * 1000;
            }
            else {
                tv.tv_sec	= sendSec;
                tv.tv_usec	= 0;
            }
#endif
			rc = setsockopt ( sock, SOL_SOCKET, SO_SNDTIMEO, ( const char * ) &tv, sizeof ( tv ) );
            if ( rc < 0 ) {
#ifdef MEDIATORDAEMON
				CVerb ( "SocketTimeout: Failed to set SO_SNDTIMEO." ); VerbLogSocketError ();
#else
                CVerbs ( 2, "SocketTimeout: Failed to set SO_SNDTIMEO." ); VerbLogSocketError ();
#endif
                return false;
            }
        }
        
        return true;
    }


    void ShutdownCloseSocketI ( int sock, bool doClose CLOSE_SOCKET_ARG (const char * msg) )
    {
        if ( IsInvalidFD ( sock ) )
            return;

#ifdef ENABLE_LINGER_OPTIONS
#   ifndef MEDIATORDAEMON
		linger ling;
		//ling.l_onoff	= 1;
        //ling.l_linger   = 1;
		ling.l_onoff	= 0;
		ling.l_linger   = 0;

		if ( setsockopt ( sock, SOL_SOCKET, SO_LINGER, ( char * ) &ling, sizeof ( ling ) ) < 0 ) {
			CVerbArg ( "ShutdownCloseSocket: Failed to set SO_LINGER on socket [ %i ]", sock );
			VerbLogSocketError ();
		}
#   endif
#endif

#ifdef ENABLE_SOCKET_CLOSE_DRAIN
		int rc;
		static char tmp [ 1024 ];

		CSocketTraceUpdate ( sock, "ShutdownCloseSocket: Shutdown 11" );
		shutdown ( sock, 1 );  //SD_SEND
#endif
        // Make subsequent calls to this socket return immediately
        SetNonBlockSocket ( sock, true, "ShutdownCloseSocket" );

#ifdef ENABLE_SOCKET_CLOSE_DRAIN
        do
        {
            rc = ( int ) recv ( sock, tmp, sizeof ( tmp ), 0 );
            if ( rc <= 0 )
                break;
        }
        while ( true );
#endif
        if ( !doClose ) {
            CSocketTraceUpdateCheck ( sock, msg );

#ifndef ENABLE_SOCKET_CLOSE_DRAIN
            CSocketTraceUpdate ( sock, "ShutdownCloseSocket: Shutdown 1" );
            shutdown ( sock, 1 );  //SD_SEND
#endif
            return;
        }        

        CSocketTraceUpdateCheck ( sock, msg );

		CSocketTraceUpdate ( sock, "ShutdownCloseSocket: Shutdown 2" );
        shutdown ( sock, 2 );  //SD_SEND

		CSocketTraceRemove ( sock, "ShutdownCloseSocket: Closing", msg );
        closesocket ( sock );
    }
    
    
    bool FakeConnect ( int sock, unsigned int port )
    {
        int closer = ( int ) socket ( PF_INET, SOCK_STREAM, 0 );
        if ( closer < 0 )
            return false;

		CSocketTraceAdd ( closer, "Mediator FakeConnect closer" );
        
        struct sockaddr_in  addr;
        Zero ( addr );
        
        addr.sin_family		 = PF_INET;
        addr.sin_port		 = htons ( (unsigned short) port );
        addr.sin_addr.s_addr = htonl ( INADDR_LOOPBACK );
        
        Mediator::Connect ( 0, closer, ( struct sockaddr * ) &addr, 2 );

		CSocketTraceRemove ( closer, "FakeConnect: Closing closer", 0 );
        closesocket ( closer );
        return true;
    }
    

	bool BuildAppAreaField ( unsigned char * sizesDst, const char * app, const char * area, bool ext )
	{
		if ( !sizesDst )
			return false;

		char * dst = ( char * ) ( sizesDst + 2 );

		size_t max = ( ext ? MAX_NAMEPROPERTY + 1 : MAX_NAMEPROPERTY );

		size_t len = strlcpy ( dst, app, max );
		if ( len >= max )
			return false;
		*sizesDst = ( unsigned char ) ( len + 1 );
		sizesDst++;

		dst [ len ] = 0;
		dst += len + 1;

		max = ( ext ? 180 : MAX_NAMEPROPERTY );

		len = strlcpy ( dst, area, max );
		if ( len >= max )
			return false;
		*sizesDst = ( unsigned char ) ( len + 1 );

		dst [ len ] = 0;
		return true;
    }
    
    
    bool BuildAppAreaID ( char * sizesDst, int deviceID, const char * app, const char * area, char channel, unsigned int token )
    {
        if ( !sizesDst )
            return false;
        
		int size = -1;
        
        char * dst = sizesDst + 4;

        if ( channel == MEDIATOR_STUNT_CHANNEL_VERSATILE )
        {
            if ( app ) {
                if ( area )
                    size = snprintf ( dst, MAX_DEVICE_INSTANCE_KEY_LENGTH, "%X %X %s %s", deviceID, token, app, area );
            }
            else {
                if ( area )
                    size = snprintf ( dst, MAX_DEVICE_INSTANCE_KEY_LENGTH, "%X %X %s", deviceID, token, area );
                else
                    size = snprintf ( dst, MAX_DEVICE_INSTANCE_KEY_LENGTH, "%X %X", deviceID, token );
            }
        }
        else {
            if ( app ) {
                if ( area )
                    size = snprintf ( dst, MAX_DEVICE_INSTANCE_KEY_LENGTH, "%X %s %s", deviceID, app, area );
            }
            else {
                if ( area )
                    size = snprintf ( dst, MAX_DEVICE_INSTANCE_KEY_LENGTH, "%X %s", deviceID, area );
                else
                    size = snprintf ( dst, MAX_DEVICE_INSTANCE_KEY_LENGTH, "%X", deviceID );
            }
        }

        if ( size <= 0 )
            return false;

        dst [ size ] = 0;
        * ( ( unsigned int * ) sizesDst ) = size + 1;
        return true;
    }
    
    
    void DisposeSendContexts ( ThreadInstance * client )
    {
		if ( !client )
			return;

        LockAcquireA ( client->sendQueueLock, "DisposeSendContexts" );
        
        while ( client->sendQueue.size_ > 0 ) {
            SendContext * ctx = ( SendContext * ) client->sendQueue.pop ();
            
            if ( ctx )
                delete ( ctx );
        }
        
        while ( client->sendQueuePrior.size_ > 0 ) {
            SendContext * ctx = ( SendContext * ) client->sendQueuePrior.pop ();
            
            if ( ctx )
                delete ( ctx );
        }
        
        LockReleaseA ( client->sendQueueLock, "DisposeSendContexts" );
    }
    
    
    ThreadInstance::ThreadInstance ()
    {
        TraceThreadInstanceAdd ( this );

        allocated       = false;

		ZeroStruct ( aes, AESContext );

#ifdef MEDIATORDAEMON
#   ifdef USE_NOTIFY_TARGET_INDEX
        inNotifierList  = -1;
#   endif
        inAcceptorList  = false;

		ZeroStruct ( ctx, ClientProcContext );

#ifndef USE_MEDIATOR_CLIENT_INSTANCE_BUFFER_UP
		bufferPtr		= 0;
#endif
#endif
        
#ifdef USE_MEDIATOR_CLIENT_WINSOCK_SOCKETS
		receiveEvent	= 0;
#endif        
        Reset ();
    }
    
    
    void ThreadInstance::Reset ()
    {
        daemon          = 0;
        
        deviceID        = 0;
        version         = 0;
        sessionID       = 0;
        aliveLast       = 0;
        socket          = INVALID_FD;
        port            = 0;
        portUdp         = 0;
        connectTime     = 0;
        filterMode      = 0;
        
        encrypt         = 0;
        authenticated   = 0;
        authLevel       = 0;
        createAuthToken = false;
        seqNr           = 1;
        
        *uid            = 0;
        *ips            = 0;
                
#ifdef MEDIATORDAEMON
        subscribedToNotifications   = true;
        subscribedToMessages        = true;
        
        sendFails           = 0;
        socketToClose       = INVALID_FD;
        sendBusy1            = 0;
#endif
    }
    
    
    ThreadInstance::~ThreadInstance ()
    {
        Dispose ();

        if ( aes.encCtx CRYPT_AES_LOCK_EXP ( || aes.lockAllocated ) ) {
			CVerbVerb ( "Destruct: Disposing AES key context." );
			AESDisposeKeyContext ( &aes );
		}

		DisposeSendContexts ( this );

        if ( allocated ) {
            allocated = false;
            
            LockDisposeA ( stuntSocketLock );
            
			LockDisposeA ( sendQueueLock );
        }

#ifdef USE_MEDIATOR_CLIENT_WINSOCK_SOCKETS
		CloseWSAHandle_n ( receiveEvent );
#endif

#if defined(MEDIATORDAEMON) && !defined(USE_MEDIATOR_CLIENT_INSTANCE_BUFFER_UP)
		if ( bufferPtr )
			delete ( bufferPtr );
#endif
        TraceThreadInstanceRemove ( this );
    }
    
    
    bool ThreadInstance::Init ()
    {
        if ( !allocated )
        {
#ifdef MEDIATORDAEMON
            if ( !ILock::Init () )
                return false;
#endif
            if ( !LockInitA ( sendQueueLock ) )
                return false;

#ifdef USE_MEDIATOR_CLIENT_WINSOCK_SOCKETS
			CreateWSAHandle ( receiveEvent, false );
#endif
            if ( !LockInitA ( stuntSocketLock ) )
                return false;

            Zero ( aes );
            Zero ( addr );
            
            allocated = true;
        }
        
        thread.autoreset = true;
        
        return thread.Init ();
    }
    
    
    void ThreadInstance::SendTcpFin ( )
    {
        CVerb ( "SendTcpFin" );

        int sock = ( int ) socket;

        if ( IsInvalidFD ( sock ) ) {
#ifdef MEDIATORDAEMON
            sock = ( int ) socketToClose;

            if ( IsInvalidFD ( sock ) )
#endif
                return;
        }
        
        char            buffer [ 8 ];
        char *          cipher      = 0;
        char *			toSend		= buffer;
        unsigned int    toSendLen   = 8;
        
        unsigned int *  pUI         = reinterpret_cast<unsigned int *>( buffer );
        *pUI = 8;
        
        buffer [ 4 ] = MEDIATOR_PROTOCOL_VERSION;
        buffer [ 5 ] = MEDIATOR_CMD_QUIT;
        buffer [ 6 ] = ';';
        buffer [ 7 ] = ';';
        
        if ( encrypt ) {
            if ( !AESEncrypt ( &aes, ( char * ) buffer, &toSendLen, &cipher ) || !cipher ) {
                CErrArg ( "SendTcpFin [ %i ]: Failed to encrypt AES message.", socket );
                return;
            }
            toSend = cipher;
        }
        
        send ( ( int ) socket, toSend, toSendLen, MSG_NOSIGNAL );
        
        free_n ( cipher );
        
        CVerb ( "SendTcpFin: done." );
    }
    
    
    void ThreadInstance::Dispose ()
    {
#ifdef MEDIATORDAEMON
        int sock = ( int ) socket;
		socket = INVALID_FD;

		if ( IsInvalidFD ( sock ) )
			sock = ( int ) socketToClose;

        if ( IsValidFD ( sock ) )
        {
            CVerb ( "Dispose: Shutdown socket" );
            ShutdownCloseSocket ( sock, false, "ThreadInstance.Dispose socket" );
            
            thread.WaitOne ( "Dispose", 100 );
        }
        
        if ( IsValidFD ( sock ) ) {
            CVerb ( "Dispose: Closing socket" );
            ShutdownCloseSocket ( sock, true, "ThreadInstance.Dispose socket" );
        }
        
        int sockC = ( int ) socketToClose;
		socketToClose = INVALID_FD;

        if ( IsValidFD ( sockC ) )
        {
            if ( sockC != sock )
            {                
                CVerb ( "Dispose: Shutdown socketToClose" );
                ShutdownCloseSocket ( sockC, true, "ThreadInstance.Dispose stuntSocket" );
            }
        }

		//DisposeSendContexts ( this );
        
        // Wait for the listening thread to shut down
        thread.Join ( "Dispose" );
        thread.Detach ( "Dispose" );
#else
        thread.Lock ( "ThreadInstance.Dispose" );
        
        int sock = ( int ) socket;
        
        socket = INVALID_FD;
        
		thread.ResetSync ( "ThreadInstance.Dispose", false );

        thread.Unlock ( "ThreadInstance.Dispose" );
        
    ShutdownThread:
#ifdef USE_MEDIATOR_CLIENT_WINSOCK_SOCKETS
        WSASetEvent ( receiveEvent );
        
        if ( IsValidFD ( sock )  )
        {
            CVerb ( "Dispose: Shutdown socket" );
            ShutdownCloseSocket ( sock, false, "ThreadInstance.Dispose" );
        }
#else
        if ( IsValidFD ( sock )  )
        {
            // Note: shutdown 2 seems to stop poll as well
            CSocketTraceUpdate ( sock, "ThreadInstance.Dispose: Shutdown 2" );
            shutdown ( sock, 2 );
            
            //CVerb ( "Dispose: Shutdown socket" );
            //ShutdownCloseSocket ( sock, false, "ThreadInstance.Dispose" );
        }
#endif

        if ( thread.isRunning () ) {
			thread.WaitOne ( "ThreadInstance.Dispose", 500 );
        }
        
        if ( IsValidFD ( sock ) ) 
		{
#ifdef USE_MEDIATOR_CLIENT_WINSOCK_SOCKETS
			if ( WSAEventSelect ( sock, NULL, 0 ) ) {
				CErr ( "Dispose: Failed to deassoc event from socket." );
			}
#endif
            CVerb ( "Dispose: Closing socket" );
            ShutdownCloseSocket ( sock, true, "ThreadInstance.Dispose" );
        }

		if ( thread.isRunning () ) {
			// Wait for the listening thread to get terminated
			CLog ( "Dispose: Waiting for Mediator thread ..." );

			thread.Join ( "Dispose" );
		}
		else {
			// Make sure the thread resources are detached.
			thread.Detach ( "Dispose" );
		}
        
        sock = ( int ) socket;
        
        // It could happen, that a register thread was missed during disposal
        // If that is the case, let's shutdown the newly created thead
        if ( IsValidFD ( sock )  )
            goto ShutdownThread;
#endif

#ifndef MEDIATORDAEMON
        LockAcquireA ( stuntSocketLock, "Dispose" );
        
		//thread.Lock ( "ThreadInstance.Dispose" );
#endif
        
        stuntSocketsLog.clear ();
        
#ifndef MEDIATORDAEMON
        LockReleaseA ( stuntSocketLock, "Dispose" );
        
        //thread.Unlock ( "ThreadInstance.Dispose" );
#endif
        Reset ();
    }
    
    
    MediatorConnection::MediatorConnection ()
    {
        allocated       = false;
        buffer          = 0;
        
        responseBuffers.clear ();
        
        Reset ();
    }
    
    
    void MediatorConnection::Reset ()
    {
        longReceive     = false;
        
        env             = 0;
    }
    
    
    MediatorConnection::~MediatorConnection () {
        Dispose ();
        
        if ( allocated ) {
            allocated = false;
            
            if ( LockAcquireA ( receiveLock, "Reset" ) )
            {
                responseBuffers.clear ();
                
                LockReleaseA ( receiveLock, "Reset" );
            }
            
            LockDisposeA ( receiveLock );
            
            CondDisposeA ( receiveEvent );         
        }
        
        free_m ( buffer );
    }
    
    
    bool MediatorConnection::Init ()
    {
        if ( !allocated )
        {
            Zero ( receiveLock );
            
            if ( !LockInitA ( receiveLock ) )
                return false;
            
            Zero ( receiveEvent );
            
            if ( pthread_cond_manual_init ( &receiveEvent, NULL ) ) {
                CErr ( "Init: Failed to init receiveEvent!" );
                return false;
            }
            
            allocated = true;
        }
        
        if ( !buffer ) {
            buffer = ( char * ) malloc ( MEDIATOR_REC_BUFFER_SIZE_MAX );
            if ( !buffer ) {
                CErr ( "Init: Failed to allocate memory!" );
                return false;
            }
        }
        
        return instance.Init ();
    }
    
    
    void MediatorConnection::Dispose ()
    {
        instance.Dispose ();
        
        Reset ();
    }
    
    
    MediatorInstance::MediatorInstance () {
        Reset ();
    }
    
    
    MediatorInstance::~MediatorInstance ()
    {
        if ( !ip )
            return;
        Dispose ();
    }
    
    
    bool MediatorInstance::Init ()
    {
        return connection.Init ();
    }
    
    
    void MediatorInstance::Dispose ()
    {
        connection.Dispose ();
        
        Reset ();
    }
    
    
    void MediatorInstance::Reset ()
    {
        ip          = 0;
        port        = 0;
        enabled     = false;
        listening   = false;
        
        mediatorObject  = 0;
        next            = 0;
    }
    

	ApplicationDevices::ApplicationDevices ()
	{
		id					= 0;
		areaId				= 0;

		count				= 0;
		latestAssignedID	= 0;
		devices				= 0;

		devicesCache		= 0;
		deviceCacheCount	= 0;
        deviceCacheCapacity = 0;
		deviceCacheDirty	= true;

		access              = 1;
	}


	ApplicationDevices::~ApplicationDevices ()
    {
        free_m ( devicesCache );
	}


	Mediator::Mediator ()
	{
        CVerb ( "Construct" );
        
#ifdef DEBUG_TRACK_SOCKET
        __sync_add_and_fetch ( &debugMediatorCount, 1 );
#endif
		allocated               = false;

		isRunning 				= false;
		aliveRunning			= false;

		broadcastRunning		= false;
		broadcastSocket 		= INVALID_FD;

#ifdef USE_MEDIATOR_NON_BLOCK_BROADCAST_WINSOCK
		broadcastEvent			= 0;
#endif

		broadcastMessageLen		= 0;
		broadcastMessageLenExt	= 0;
		lastGreetUpdate			= 0;
        broadcastReceives       = 0;
        broadcastThreadRestarts = 0;

		udpStatusMessageOffset	= 0;
		udpStatusMessageLen		= 0;

#if (!defined(MEDIATORDAEMON) && defined(__cplusplus))
		env						= nill;
#endif  

        certificate				= 0;

		srand ( GetEnvironsTickCount32 () );

		BroadcastGenerateToken ();
        
#ifdef USE_THREADSYNC_OWNER_NAME
        broadcastThread.owner = "broadcastThread";
#endif
	}


	void Mediator::BroadcastGenerateToken ()
	{
		CVerb ( "BroadcastGenerateID" );

		randBroadcastToken      = (int) (GetEnvironsTickCount () + rand ());
	}


	Mediator::~Mediator ()
	{
		CVerb ( "Destructor" );

		//Dispose ();        
		free_m ( certificate );

		if ( allocated )
        {
            allocated = false;

#ifdef USE_MEDIATOR_NON_BLOCK_BROADCAST_WINSOCK
			CloseWSAHandle_n ( broadcastEvent );
#endif
            
#ifdef ENABLE_MEDIATOR_LOCK
			LockAcquireVA ( mediatorLock, "Destructor" );

            LockDisposeA ( devicesLock );

			LockReleaseVA ( mediatorLock, "Destructor" );

			LockDisposeA ( mediatorLock );
#else
            LockDisposeA ( devicesLock );
#endif
		}
        
#ifdef DEBUG_TRACK_SOCKET
        __sync_sub_and_fetch ( &debugMediatorCount, 1 );
#endif
		CVerb ( "Destructor: Done." );
	}


	bool Mediator::Init ()
	{
		CVerb ( "Init" );

		if ( !InitClass () )
			return false;

		if ( !allocated )
        {
            if ( !mediator.Init () )
                return false;

#ifdef USE_MEDIATOR_NON_BLOCK_BROADCAST_WINSOCK
			CreateWSAHandle ( broadcastEvent, false );
#endif
            
#ifdef ENABLE_MEDIATOR_LOCK
			if ( !LockInitA ( mediatorLock ) )
				return false;
#endif
			if ( !LockInitA ( devicesLock ) )
                return false;

            if ( !broadcastThread.Init () )
                return false;
            
			allocated = true;
		}

		if ( !LoadNetworks () ) {
			CErr ( "Init: Failed to load local ip addresses!" );
			return false;
		}

        lastGreetUpdate = GetEnvironsTickCount32 ();

		return true;
	}


	bool Mediator::InitClass ()
	{
		CVerb ( "InitClass" );

		if ( allocatedClass )
			return true;

		Zero ( localNets );

		if ( !LockInitA ( localNetsLock ) )
            return false;

		allocatedClass = true;

		return true;
	}


	void Mediator::DisposeClass ()
	{
		CVerb ( "DisposeClass" );

		ReleaseNetworks ();

		if ( !allocatedClass )
            return;
        allocatedClass = false;

		LockDisposeA ( localNetsLock );
	}


	void Mediator::BroadcastByeBye ( int sock )
	{
		strlcpy ( broadcastMessage + 4, MEDIATOR_BROADCAST_BYEBYE, sizeof ( MEDIATOR_BROADCAST_BYEBYE ) );

		SendBroadcastWithSocket ( true, false, true, sock );
	}


	void Mediator::Dispose ()
	{
		CVerb ( "Dispose" );

		isRunning		= false;
		aliveRunning	= false;

		// Wait for each thread to terminate
		ReleaseThreads ( true );

		ReleaseMediators ();
        
        free_m ( certificate );
	}

    
    bool Mediator::IsSocketAlive ( int &sock )
	{
		CVerb ( "IsSocketAlive" );

		if ( IsInvalidFD ( sock ) ) {
			CVerb ( "IsSocketAlive: socket argument is invalid" );
			return false;
		}

		int value;
		socklen_t size = sizeof ( value );

		int ret = getsockopt ( ( int ) sock, SOL_SOCKET, SO_REUSEADDR, ( char * ) &value, &size );
		if ( ret < 0 ) {
			CInfo ( "IsSocketAlive: disposing invalid socket!" );
            //LogSocketError ();
            int s = ( int ) sock;
            
            sock = INVALID_FD;
            
            ShutdownCloseSocket ( s, true, "Mediator IsSocketAlive" );
			return false;
		}

		CVerb ( "IsSocketAlive: socket is alive" );
		return true;
	}


	void Mediator::VerifySockets ( ThreadInstance * inst, bool waitThread )
	{
		CVerb ( "VerifySockets" );
		 
		if ( !IsSocketAlive ( inst->socket ) )
        {
			if ( !waitThread )
				return;

			// Wait for the listening thread to be terminated
#ifdef MEDIATORDAEMON
            inst->thread.Join ( "VerifySockets" );
#endif
		}
	}


	bool Mediator::ReleaseThreads ( bool wait )
	{
		CVerb ( "ReleaseThreads" );

		int sock;
		bool ret		= true;

		// Signal stop status to threads
		isRunning		= false;
		aliveRunning	= false;

#ifdef USE_MEDIATOR_NON_BLOCK_BROADCAST_WINSOCK
		WSASetEvent ( broadcastEvent );
#endif
		if ( !wait )
		{
			sock = broadcastSocket;

			if ( IsValidFD ( sock ) ) {
				SetNonBlockSocket ( sock, true, "ReleaseThreads" );

				BroadcastByeBye ( sock );

				CVerb ( "ReleaseThreads: Shutdown broadcast socket." );
                ShutdownCloseSocket ( sock, false, "Mediator ReleaseThreads broadcastSocket" );
			}
			return true;
		}

		// Close broadcast listener socket
		LockAcquireVA ( localNetsLock, "ReleaseThreads" );

		sock = broadcastSocket;

		if ( IsValidFD ( sock ) )
			broadcastSocket = INVALID_FD;

		LockReleaseVA ( localNetsLock, "ReleaseThreads" );

		int repeats = 200;
        
#ifndef MEDIATORDAEMON
        if ( native.isAppShutdown )
            repeats = 6;
#endif
        int ms = 300;

		if ( IsValidFD ( sock ) ) {
			BroadcastByeBye ( sock );

			CVerb ( "ReleaseThreads: Shutdown broadcast socket." );
            ShutdownCloseSocket ( sock, false, "Mediator ReleaseThreads broadcastSocket" );
            
            CSocketTraceUpdate ( sock, "Mediator ReleaseThreads: Shutdown 2" );
            shutdown ( sock, 2 );

		Retry:
			if ( broadcastThread.isRunning () )
			{
				broadcastThread.WaitOne ( "Mediator.ReleaseThreads", ms );
				if ( repeats > 0 )
				{
					repeats--;
                    if ( ms < 1000 )
                        ms += 300;
					goto Retry;
				}
			}
		}

		if ( repeats <= 0 && IsValidFD ( sock ) ) {
			ShutdownCloseSocket ( sock, true, "Mediator ReleaseThreads broadcastSocket" );
			sock = INVALID_FD;
		}

		if ( broadcastThread.isRunning () ) {
			CLog ( "ReleaseThreads: Waiting for broadcast thread ..." );

			// Waiting for broadcast thread
			broadcastThread.Join ( "Mediator.ReleaseThreads" );
		}
		else {
			broadcastThread.Detach ( "Mediator.ReleaseThreads" );
		}
        
		if ( IsValidFD ( sock ) )
			ShutdownCloseSocket ( sock, true, "Mediator ReleaseThreads broadcastSocket" );

		return ret;
	}


	void Mediator::ReleaseNetworks ()
	{
		CVerb ( "ReleaseNetworks" );

        localNetsSize = 0;

        if ( !LockAcquireA ( localNetsLock, "ReleaseNetworks" ) )
			return;

		NetPack * net = localNets.next;

		while ( net ) {
			NetPack * toDelete = net;
			net = net->next;
			free ( toDelete );
		}

		Zero ( localNets );

		LockReleaseA ( localNetsLock, "ReleaseNetworks" );
	}


	void Mediator::StopMediators ()
	{
		CVerb ( "StopMediators" );

		MediatorInstance * inst = &mediator;

		while ( inst )
		{
			inst->connection.instance.thread.Lock ( "StopMediators" );

			int sock = ( int ) inst->connection.instance.socket;
            
            if ( IsValidFD ( sock ) && inst->ip )
            {
                inst->connection.instance.SendTcpFin ();
                
				CVerbArg ( "StopMediators: Shutdown socket [ %i ]", sock );
				ShutdownCloseSocket ( sock, false, "Mediator StopMediators" );
			}

			inst->connection.instance.thread.Unlock ( "StopMediators" );

			inst = inst->next;
		}
	}


	void Mediator::ReleaseMediator ( MediatorInstance * med )
	{
		if ( !med ) return;
        
		if ( med != &mediator )
            delete med;
        else if ( med->ip )
            med->Dispose ();
	}


	void Mediator::ReleaseMediators ()
	{
        CVerb ( "ReleaseMediators" );

		// Conduct a "Prerelease" by closing all sockets before actually releasing them
		StopMediators ();

		MediatorInstance * inst;
        
#ifdef ENABLE_MEDIATOR_LOCK
		LockAcquireV ( &mediatorLock, "ReleaseMediators" );
#endif
		inst = &mediator;

		while ( inst ) {
			MediatorInstance * toDelete = inst;
			inst = inst->next;

            toDelete->next = 0;

#ifndef MEDIATORDAEMON
            toDelete->connection.instance.SendTcpFin ();
#endif
			ReleaseMediator ( toDelete );
		}
        
#ifdef ENABLE_MEDIATOR_LOCK
		LockReleaseV ( &mediatorLock, "ReleaseMediators" );
#endif
	}

    
#if defined(ANDROID)
    unsigned int GetDefaultGateway ( char * gateway, size_t size )
    {
        FILE    *       fp;
        char            buffer [ 1024 ];
        
        char command [] = "ip route show";
        
        fp = popen ( command, "r" );
        if ( !fp ) {
            CErrArg ( "GetDefaultGateway: Failed to open [ %s ]", command );
            return 0;
        }
        
        while ( !feof ( fp ) ) {
            *buffer = 0;
            if ( fgets ( buffer, sizeof(buffer), fp ) )
            {
                CVerbArg ( "GetDefaultGateway: [ %s ]", buffer );
                
                // 192.168.16.0/24 dev wlan0
                // search for dev
                char * search = strstr ( buffer, "dev" );
                if ( !search )
                    continue;
                
                search += 3;
                
                int max = sizeof(buffer) - (((int) (search - buffer)) + 1);
                while ( max > 0 ) {
                    if ( *search != 32 && *search != '\t' ) break;
                    search++; max--;
                }
                
                if ( max <= 0 )
                    continue;
                
                char * end = search;
                
                max = sizeof(buffer) - (((int) (search - buffer)) + 1);
                while ( max > 0 ) {
                    if ( *end == 32 || *end == '\t' ) break;
                    end++; max--;
                }
                
                if ( max <= 0 )
                    continue;
                
                *end = 0;
                
                strncpy ( gateway, search, size );
                CVerbArg ( "GetDefaultGateway: Gateway device [ %s ]", gateway );
                break;
            }
        }
        
        pclose ( fp );
        
        return 0;
    }
    
#elif defined(__APPLE__)
    
    unsigned int GetDefaultGateway ( char * gateway, size_t size )
    {
        FILE    *       fp;
        char            buffer [ 1024 ];
        char    *       search;
        
        char command [] = "route -n get default | grep interface";
        
        fp = popen ( command, "r" );
        if ( !fp ) {
            CErrArg ( "GetDefaultGateway: Failed to open [ %s ]", command );
            return 0;
        }
        
        while ( !feof ( fp ) ) {
            *buffer = 0;
            if ( fgets ( buffer, sizeof(buffer), fp ) )
            {
                CVerbArg ( "GetDefaultGateway: [ %s ]", buffer );
                
                search = strstr ( buffer, ": " );
                if ( !search ) {
                    CErr ( "GetDefaultGateway: Failed to find : " );
                    return 0;
                }
                
                search += 2;
                size_t len = strlen ( search );
                
                if ( len > 0 ) {
                    if ( search [ len - 1 ] == '\n' || search [ len - 1 ] == '\r' )
                        search [ len - 1 ] = 0;
                    
                    strncpy ( gateway, search, size );
                    CVerbArg ( "GetDefaultGateway: Gateway device [ %s ]", gateway );
                }
                break;
            }
        }
        
        pclose ( fp );
        
        return 0;
    }
    
#else
    
#   ifndef _WIN32
    unsigned int GetDefaultGateway ( char * gateway, size_t size )
    {
        FILE    *       fp;
        char            buffer [ 1024 ];
        size_t          bytesRead;
        char    *       search;
        unsigned int    gw  = 0;
        
        fp = fopen ( "/proc/net/route", "r" );
        if ( !fp ) {
            CErr ( "GetDefaultGateway: Failed to open /proc/net/route" );
            return 0;
        }
        
        bytesRead = fread ( buffer, 1, sizeof (buffer), fp );
        fclose ( fp );
        
        if ( bytesRead == 0 || bytesRead >= sizeof (buffer) ) {
            CErrArg ( "GetDefaultGateway: Invalid bytes read [ %u ]", bytesRead );
            return 0;
        }
        
        buffer [ bytesRead ] = 0;
        CLogArg ( "GetDefaultGateway: [ %s ]", buffer );
        
        search = strstr ( buffer, "dev" );
        if ( !search ) {
            CErr ( "GetDefaultGateway: Failed to find dev" );
            return 0;
        }
        return gw;
    }
#   endif
#endif


#define USE_WIN_GETADAPTERSINFO


	bool Mediator::LoadNetworks ()
	{
		CVerb ( "LoadNetworks" );

		unsigned int	ip;
		unsigned int	bcast;

		LockAcquireVA ( localNetsLock, "LoadNetworks" );

#ifdef _WIN32

#ifndef WINDOWS_PHONE
		int adaptersFound = 0;

		IP_ADAPTER_INFO tmpAdapter;
		Zero ( tmpAdapter );

		struct sockaddr_in addr;

		PIP_ADAPTER_INFO adapters = &tmpAdapter;
		PIP_ADAPTER_INFO adapter = nill;
		DWORD success = 0;
		int i = 0;

		ULONG bufSize = sizeof ( tmpAdapter );

		if ( GetAdaptersInfo ( adapters, &bufSize ) != ERROR_BUFFER_OVERFLOW ) 
		{
			success = NO_ERROR;
		}
		else {
			adapters = ( PIP_ADAPTER_INFO ) malloc ( bufSize );

			if ( adapters == nill ) {
				CErr ( "LoadNetworks: Failed to allocate memory." );
				goto EndOfMethod;
			}

			success = GetAdaptersInfo ( adapters, &bufSize );
		}

		if ( success == NO_ERROR ) 
		{
			adapter = adapters;
			while ( adapter )
			{
				i++;
				if ( adapter->AddressLength == 6 )
				{
					if ( adapter->Type == IF_TYPE_IEEE80211 ) { // 71
						wifiMAC = 0;

						for ( int m = 0; m < 6; ++m ) {
							wifiMAC <<= 8;
							wifiMAC |= adapter->Address [ m ];
						}
					}
					else if ( adapter->Type == 6 ) {
						btMAC = 0;

						for ( int m = 0; m < 6; ++m ) {
							btMAC <<= 8;
							btMAC |= adapter->Address [ m ];
						}
					}
				}

				inet_pton ( AF_INET, adapter->IpAddressList.IpAddress.String, &( addr.sin_addr ) );
				ip = ( unsigned int ) addr.sin_addr.s_addr;

				inet_pton ( AF_INET, adapter->IpAddressList.IpMask.String, &( addr.sin_addr ) );
				unsigned int mask = ( unsigned int ) addr.sin_addr.s_addr;

				if ( ip && mask ) {
					CLogArg ( "Local IP %i: \t[ %s ]", i, inet_ntoa ( *( ( struct in_addr * ) &ip ) ) );
					CVerbArg ( "Local SN %i: \t[ %s ]", i, inet_ntoa ( *( ( struct in_addr * ) &mask ) ) );

					bcast = GetBroadcast ( ip, mask );
					if ( !bcast ) {
						CErrArg ( "LoadNetworks: Failed to calculate broadcast address for interface [ %i ]!", i );
						continue;
					}

					inet_pton ( AF_INET, adapter->GatewayList.IpAddress.String, &( addr.sin_addr ) );
					unsigned int gw = ( unsigned int ) addr.sin_addr.s_addr;
					if ( gw ) {
						CLogArg ( "Local GW %i: \t[ %s ]", i, inet_ntoa ( *( ( struct in_addr * ) &gw ) ) );
					}

					CVerbArg ( "LoadNetworks: Local BC %i: [ %s ]", i, inet_ntoa ( *( ( struct in_addr * ) &bcast ) ) );

					AddNetwork ( ip, bcast, mask, gw );
					adaptersFound++;
				}

				adapter = adapter->Next;
			}
		}

		if ( adapters != &tmpAdapter )
			free ( adapters );

		if ( !adaptersFound )
		{
			char ac [ 256 ];
			if ( gethostname ( ac, sizeof ( ac ) ) == SOCKET_ERROR ) {
				CErrArg ( "LoadNetworks: Error %i when getting local host name.", WSAGetLastError () );
				goto EndOfMethod;
			}
			CLogArg ( "LoadNetworks: Host name [%s]", ac );

			SOCKET sock = WSASocket ( PF_INET, SOCK_DGRAM, 0, 0, 0, 0 );
			if ( sock == SOCKET_ERROR ) {
				CErrArg ( "LoadNetworks: Failed to get a socket. Error %i", WSAGetLastError () );
				goto EndOfMethod;
			}
			CSocketTraceAdd ( ( int ) sock, "Mediator LoadNetworks sock" );
			DisableSIGPIPE ( sock );

			INTERFACE_INFO InterfaceList [ 20 ];
			unsigned long nBytesReturned;

			if ( WSAIoctl ( sock, SIO_GET_INTERFACE_LIST, 0, 0, &InterfaceList,
				sizeof ( InterfaceList ), &nBytesReturned, 0, 0 ) == SOCKET_ERROR )
			{
                CErrArg ( "LoadNetworks: Failed calling WSAIoctl: error %i", WSAGetLastError () );
				shutdown ( sock, 2 );

				CSocketTraceRemove ( (int) sock, "LoadNetworks: Closing", 0 );
                closesocket ( sock );
				goto EndOfMethod;
			}

			int nNumInterfaces = nBytesReturned / sizeof ( INTERFACE_INFO );
			CVerbArg ( "LoadNetworks: There are [%i] interfaces:", nNumInterfaces );

			// Skip loopback interfaces
			for ( i = 0; i < nNumInterfaces; ++i ) {
				u_long nFlags = InterfaceList [ i ].iiFlags;
				if ( nFlags & IFF_LOOPBACK ) {
					CVerb ( "LoadNetworks: Omiting loopback interface." );
					continue;
				}

				sockaddr_in * pAddress;
				//int max_length = 20;

				// Retrieve ip address
				pAddress = ( sockaddr_in * ) & ( InterfaceList [ i ].iiAddress );

				ip = pAddress->sin_addr.s_addr;

				// Retrieve netmask	
				pAddress = ( sockaddr_in * ) & ( InterfaceList [ i ].iiNetmask );
				unsigned int mask = pAddress->sin_addr.s_addr;

				CLogArg ( "Local IP %i: \t[ %s ]", i, inet_ntoa ( *( ( struct in_addr * ) &ip ) ) );
				CVerbArg ( "Local SN %i: \t[ %s ]", i, inet_ntoa ( *( ( struct in_addr * ) &mask ) ) );

				bcast = GetBroadcast ( ip, mask );
				if ( !bcast ) {
					CErrArg ( "LoadNetworks: Failed to calculate broadcast address for interface [%i]!", i );
					continue;
				}

				CVerbArg ( "LoadNetworks: Local BC %i: [ %s ]", i, inet_ntoa ( *( ( struct in_addr * ) &bcast ) ) );

				AddNetwork ( ip, bcast, mask, 0 );
            }

			shutdown ( sock, 2 );

			CSocketTraceRemove ( ( int ) sock, "LoadNetworks: Closing", 0 );
            closesocket ( sock );
		}
#endif

#elif ANDROID

		int sock, rval;
		struct ifreq ifreqs [ 20 ];
		struct ifreq ifr_mask;
		struct ifconf ifconf;
        int  nifaces, i;
        char   gw [ 1024 ];
        *gw = 0;
        
        GetDefaultGateway ( gw, sizeof(gw) );

		Zero ( ifconf );
		ifconf.ifc_buf = ( char * ) ifreqs;
		ifconf.ifc_len = sizeof ( ifreqs );

		sock = socket ( PF_INET, SOCK_STREAM, 0 );
		if ( sock < 0 ) {
			CErr ( "LoadNetworks: Failed to create socket for ioctl!" );
			goto EndOfMethod;
		}
		CSocketTraceAdd ( sock, "Mediator LoadNetworks sock" );
        DisableSIGPIPE ( sock );
        
		rval = ioctl ( sock, SIOCGIFCONF, ( char* ) &ifconf );
		if ( rval < 0 ) {
            CErr ( "LoadNetworks: ioctl SIOCGIFCONF failed!" );
            
			shutdown ( sock, 2 );

			CSocketTraceRemove ( sock, "LoadNetworks: Closing", 0 );
            closesocket ( sock );
			goto EndOfMethod;
		}

		nifaces =  ifconf.ifc_len / sizeof ( struct ifreq );

		CVerbArg ( "LoadNetworks: Interfaces (count = %d)", nifaces );

		for ( i = 0; i < nifaces; i++ )
		{
			ip = ( ( struct sockaddr_in * )&ifreqs [ i ].ifr_ifru.ifru_addr )->sin_addr.s_addr;

			CLogArg ( "LoadNetworks: Interface name: '%s'\tip: [ %s ]", ifreqs [ i ].ifr_name, inet_ntoa ( *( ( struct in_addr * ) &ip ) ) );

			if ( strlen ( ifreqs [ i ].ifr_name ) < 1 ) {
				CVerb ( "LoadNetworks: Omiting invalid interface name." );
				continue;
			}

			if ( !ip ) {
				CVerb ( "LoadNetworks: Omiting invalid interface." );
				continue;
			}

			// Skip loopback
			if ( ip == 0x0100007F ) {
				CVerb ( "LoadNetworks: Omiting loopback interface." );
				continue;
			}

            unsigned int isGW = 0;
            
            
            if ( *gw ) {
                if ( !strncmp ( gw, ifreqs [ i ].ifr_name, sizeof(gw) ) ) {
                    CLogArg ( "LoadNetworks: Default interface found [%s]", gw );
                    isGW = 1;
                }
            }
            else {
                if ( strstr ( ifreqs [ i ].ifr_name, "wlan" ) )
                    isGW = 1;
            }
            
			// Retrieve netmask	
			ifr_mask.ifr_addr.sa_family = PF_INET;

			strlcpy ( ifr_mask.ifr_name, ifreqs [ i ].ifr_name, IFNAMSIZ - 1 );

			rval = ioctl ( sock, SIOCGIFNETMASK, &ifr_mask );
			if ( rval < 0 ) {
				CErr ( "LoadNetworks: ioctl SIOCGIFNETMASK failed!" );
				continue;
			}

			unsigned int mask = ( ( struct sockaddr_in * )&ifr_mask.ifr_addr )->sin_addr.s_addr;

			bcast = GetBroadcast ( ip, mask );
			if ( !bcast ) {
				CErr ( "LoadNetworks: Failed to calculate broadcast address!" );
				continue;
			}
			CVerbArg ( "LoadNetworks: Netmask:   [ %s ]'", inet_ntoa ( *( ( struct in_addr * ) &mask ) ) );
			CVerbArg ( "LoadNetworks: Broadcast: [ %s ]", inet_ntoa ( *( ( struct in_addr * ) &bcast ) ) );

			AddNetwork ( ip, bcast, mask, isGW );
        }
        
		shutdown ( sock, 2 );

		CSocketTraceRemove ( sock, "LoadNetworks: Closing", 0 );
        closesocket ( sock );

#else // LINUX or MAC or IOS

		struct ifaddrs  *   ifList  = 0;
		struct ifaddrs  *   ifa     = 0;
        void            *   tmp     = 0;
        char                gw [ 1024 ]; *gw = 0;
        
#	ifndef LINUX
        GetDefaultGateway ( gw, sizeof(gw) );
#	endif

		if ( getifaddrs ( &ifList ) != 0 ) {
			CErr ( "LoadNetworks: getifaddrs failed!" );
			goto EndOfMethod;
		}

		for ( ifa = ifList; ifa != NULL; ifa = ifa->ifa_next )
        {
            if ( ! ( ifa->ifa_flags & IFF_UP ) ) {
                CVerbArg ( "LoadNetworks: Interface not up [ %s ].", ifa->ifa_name );
                continue;
            }

            if ( ifa->ifa_flags & IFF_LOOPBACK ) {
                CVerb ( "LoadNetworks: Omiting loopback interface." );
                continue;
            }

#if ( defined(ENVIRONS_IOS) || defined(ENVIRONS_OSX) )
            // Skip any connection but ethernet en...
            if ( !strstr ( ifa->ifa_name, "en") ) {
                CVerbArg ( "LoadNetworks: Not an ethernet interface. Ommiting [ %s ]", ifa->ifa_name ? ifa->ifa_name : "---" );
                continue;
            }

            if ( ifa->ifa_addr->sa_family == AF_LINK ) {
                unsigned char * ptr = (unsigned char *) LLADDR ( (struct sockaddr_dl *) ifa->ifa_addr );

                for ( int m = 0; m < 6; ++m ) {
                    wifiMAC <<= 8;
                    wifiMAC |= *ptr; ptr++;
                }

                CLogArg ( "LoadNetworks: [ %s ] => \tMAC: [ %llX ]", ifa->ifa_name, wifiMAC );
            }
            else
#endif
            if ( ifa->ifa_addr->sa_family == PF_INET ) {
				// IPv4
				ip = ( ( struct sockaddr_in * ) ifa->ifa_addr )->sin_addr.s_addr;

				CLogArg ( "LoadNetworks: [ %s ] => \tIP: [ %s ]", ifa->ifa_name, inet_ntoa ( *( ( struct in_addr * ) &ip ) ) );

				if ( !ip ) {
					CVerb ( "LoadNetworks: Omiting invalid interface." );
					continue;
				}

				// Skip loopback
				/*if ( ip == 0x0100007F ) {
					CVerb ( "LoadNetworks: Omiting loopback interface." );
					continue;
				}*/

                unsigned int isGW = 0;
                
                if ( *gw && !strncmp ( gw, ifa->ifa_name, sizeof(gw) ) ) {
                    CLogArg ( "LoadNetworks: Default interface found [%s]", gw );
                    isGW = 1;
                }
                
				unsigned int mask = ( ( struct sockaddr_in * )ifa->ifa_netmask )->sin_addr.s_addr;

				bcast = GetBroadcast ( ip, mask );
				if ( !bcast ) {
					CErr ( "LoadNetworks: ERROR - Failed to calculate broadcast address!" );
					continue;
				}
				CVerbArg ( "LoadNetworks: Netmask:   [ %s ]", inet_ntoa ( *( ( struct in_addr * ) &mask ) ) );
				CVerbArg ( "LoadNetworks: Broadcast: [ %s ]", inet_ntoa ( *( ( struct in_addr * ) &bcast ) ) );

				AddNetwork ( ip, bcast, mask, isGW );
			}
			else if ( ifa->ifa_addr->sa_family == PF_INET6 ) {
				// We do not support IPv6 yet
				tmp=&( ( struct sockaddr_in6 * )ifa->ifa_addr )->sin6_addr;
				char addressBuffer [ INET6_ADDRSTRLEN ];
				inet_ntop ( PF_INET6, tmp, addressBuffer, INET6_ADDRSTRLEN );
				CVerbArg ( "LoadNetworks: IPv6 not supported yet: [%s] IP Address [ %s ]", ifa->ifa_name, addressBuffer );
			}
		}

		if ( ifList )
			freeifaddrs ( ifList );
#endif

	EndOfMethod:
		LockReleaseVA ( localNetsLock, "LoadNetworks" );

		CVerbVerb ( "LoadNetworks: Done." );
		return true;
	}


	unsigned int Mediator::GetBroadcast ( unsigned int ip, unsigned int netmask )
	{
		u_long host_ip = ip;
		u_long net_mask = netmask;
		u_long net_addr = host_ip & net_mask;
		u_long bcast_addr = net_addr | ( ~net_mask );

		return ( unsigned int ) bcast_addr;
	}


	void Mediator::AddNetwork ( unsigned int ip, unsigned int bcast, unsigned int netmask, unsigned int gw )
	{
		NetPack * net = &localNets;
		NetPack * pack = net;

		while ( net ) {
			if ( net->ip == ip )
				return;

			pack = net;
			net = net->next;
		}

		if ( localNets.ip ) {
			// This NetPack has already been completed, create and attach a new one
			NetPack * newPack = ( NetPack * ) calloc ( 1, sizeof ( NetPack ) );
			if ( !newPack ) {
				CErr ( "AddNetwork: Failed to allocate memory!" );
				return;
			}
			pack->next = newPack;
			pack = newPack;
		}

		if ( gw && pack != &localNets ) 
		{
			// Swap current network with the first one
			memcpy ( pack, &localNets, sizeof ( NetPack ) );						
			
			pack->next = 0;
			pack = &localNets;
		}

		pack->ip	= ip;
		pack->bcast = bcast;
		pack->mask	= netmask;
		pack->gw	= gw;
        
        localNetsSize++;
	}


	bool Mediator::IsStarted ()
	{
		return isRunning;
	}


	bool Mediator::Start ()
	{
		CVerb ( "Start" );

		isRunning = true;

		// Continue even if broadcast fails
		PrepareAndStartBroadcastThread ();

		OnStarted ();

		//SendBroadcast ();

		CVerb ( "Start: success" );
		return true;
	}
    
    
    bool Mediator::PrepareAndStartBroadcastThread ( bool runThread )
    {
        CVerb ( "PrepareAndStartBroadcastThread" );
        
        // Create socket for broadcast thread
        if ( IsInvalidFD ( broadcastSocket ) ) 
		{
			CVerb ( "PrepareAndStartBroadcastThread: Creating socket." );

            int sock = ( int ) socket ( PF_INET, SOCK_DGRAM, IPPROTO_UDP );
            if ( IsInvalidFD ( sock ) ) 
			{
                CErr ( "PrepareAndStartBroadcastThread: Failed to create broadcast socket!" );

				LogSocketErrorF ( "Mediator.PrepareAndStartBroadcastThread" );
                return false;
            }
			CSocketTraceAdd ( sock, "Mediator PrepareAndStartBroadcastThread broadcastSocket" );
            DisableSIGPIPE ( sock );
            
            broadcastSocket = sock;
            
            int value = 1;
            if ( setsockopt ( broadcastSocket, SOL_SOCKET, SO_BROADCAST, ( const char * ) &value, sizeof ( value ) ) == -1 ) {
                CErr ( "PrepareAndStartBroadcastThread: Failed to set broadcast option on socket!" );

				if ( IsValidFD ( broadcastSocket ) ) { LogSocketErrorF ( "Mediator.PrepareAndStartBroadcastThread" ); }
                return false;
            }
            
            value = 1;
            if ( setsockopt ( broadcastSocket, SOL_SOCKET, SO_REUSEADDR, ( const char * ) &value, sizeof ( value ) ) != 0 ) {
                CErr ( "PrepareAndStartBroadcastThread: Failed to set reuseaddr option on socket!" );

				if ( IsValidFD ( broadcastSocket ) ) { LogSocketErrorF ( "Mediator.PrepareAndStartBroadcastThread" ); }
                //return false;
            }
            
#ifdef SO_REUSEPORT
            value = 1;
            if ( setsockopt ( broadcastSocket, SOL_SOCKET, SO_REUSEPORT, ( const char * ) &value, sizeof ( value ) ) != 0 ) {
                CErr ( "PrepareAndStartBroadcastThread: Failed to set reuseaddr option on socket!" );

				if ( IsValidFD ( broadcastSocket ) ) { LogSocketErrorF ( "Mediator.PrepareAndStartBroadcastThread" ); }
                //return false;
            }
#endif
            /// Create broadcast thread
			if ( runThread ) {
				CVerb ( "PrepareAndStartBroadcastThread: Creating thread." );

				broadcastThread.ResetSync ( "BroadcastThreadStarter" );

				if ( !broadcastThread.Run ( pthread_make_routine ( BroadcastThreadStarter ), ( void * )this, "Mediator.Start", true ) ) 
				{
					CErr ( "PrepareAndStartBroadcastThread: Failed to create/wait for thread start!" );

					if ( !broadcastThread.isRunning () ) {
						CErr ( "PrepareAndStartBroadcastThread: Failed to create thread!" );
						return false;
					}
				}		
			}
        }
        return true;
    }


	void Mediator::OnStarted ()
	{
	}

    
#ifdef MEDIATORDAEMON
    bool Mediator::SendBroadcast ( bool enforce, bool sendStatus, bool sendToAny, bool enforceEnqueue )
    {
        CVerbVerb ( "SendBroadcast" );
        
        if ( IsInvalidFD ( broadcastSocket ) || ( !enforce && !broadcastRunning ) )
            return false;
        
        bool success    = true;
        int  sentBytes  = 0;
        char * msg;
        int sendLen;
        
        if ( sendStatus ) {
            msg		= udpStatusMessage;
            sendLen = ( int ) udpStatusMessageLen;
        }
        else {
            msg		= broadcastMessage;
            sendLen = ( int ) broadcastMessageLen;
        }
        
        CVerbArg ( "SendBroadcast: Broadcasting %smessage [ %s ] ( %d )...", sendStatus ? "device status " : "", msg + 4, sendLen );
        
        struct 	sockaddr_in		broadcastAddr;
        Zero ( broadcastAddr );
        
        broadcastAddr.sin_family = PF_INET;
        broadcastAddr.sin_port = htons ( GET_MEDIATOR_BASE_PORT );
        
        if ( sendToAny ) {
            broadcastAddr.sin_addr.s_addr = htonl ( INADDR_BROADCAST ); // 0xFFFFFFFF;
            
            sentBytes = ( int ) sendto ( broadcastSocket, msg, sendLen, 0, ( struct sockaddr * ) &broadcastAddr, sizeof ( struct sockaddr ) );
            if ( sentBytes != sendLen )
            {
                CErr ( "SendBroadcast: Broadcast to any failed!" );
            }
        }
        
        /// We need to broadcast to each interface, because (at least with win32): the default 255.255.... broadcasts only on one (most likely the main or internet) network interface
        NetPack * net = 0;
        
#ifdef NDEBUG
        if ( localNetsSize <= 1 )
#else
            if ( localNetsSize <= 0 )
#endif
                return true;
        
        if ( !LockAcquireA ( localNetsLock, "SendBroadcast" ) ) {
            return false;
        }
        
        net = &localNets;
        while ( net ) {
            broadcastAddr.sin_addr.s_addr = htonl ( net->bcast );
            
            sentBytes = ( int ) sendto ( broadcastSocket, msg, sendLen, 0, ( struct sockaddr * ) &broadcastAddr, sizeof ( struct sockaddr ) );
            if ( sentBytes != sendLen ) {
                CErr ( "SendBroadcast: Broadcast failed!" );
                success = false;
            }
            net = net->next;
        }
        
        LockReleaseVA ( localNetsLock, "SendBroadcast" );
        
        return success;
    }
    
    bool Mediator::SendBroadcastWithSocket ( bool enforce, bool sendStatus, bool sendToAny, int sock )
    {
        CVerbVerb ( "SendBroadcastWithSocket" );
        
        if ( IsInvalidFD ( sock ) )
            sock = broadcastSocket;
        
        if ( IsInvalidFD ( sock ) || ( !enforce && !broadcastRunning ) )
            return false;
        
#ifndef MEDIATORDAEMON
        if ( native.networkStatus < NETWORK_CONNECTION_NO_INTERNET )
            return false;
#endif
        
        bool success    = true;
        int  sentBytes  = 0;
        
        char * msg;
        int sendLen;
        
        if ( sendStatus ) {
            msg		= udpStatusMessage;
            sendLen = ( int ) udpStatusMessageLen;
        }
        else {
            msg		= broadcastMessage;
            sendLen = ( int ) broadcastMessageLen;
        }
        
        CVerbArg ( "SendBroadcastWithSocket: Broadcasting %smessage [ %s ] ( %d )...", sendStatus ? "device status " : "", msg + 4, sendLen );
        
        struct 	sockaddr_in		broadcastAddr;
        Zero ( broadcastAddr );
        
        broadcastAddr.sin_family = PF_INET;
        broadcastAddr.sin_port = htons ( GET_MEDIATOR_BASE_PORT );
        
        if ( sendToAny ) {
            broadcastAddr.sin_addr.s_addr = htonl ( INADDR_BROADCAST ); // 0xFFFFFFFF;
            
            sentBytes = ( int ) sendto ( sock, msg, sendLen, 0, ( struct sockaddr * ) &broadcastAddr, sizeof ( struct sockaddr ) );
            if ( sentBytes != sendLen )
            {
#ifndef MEDIATORDAEMON
                if ( env->environsState >= environs::Status::Starting )
#endif
                    CErr ( "SendBroadcastWithSocket: Broadcast to any failed!" );
            }
        }
        
        /// We need to broadcast to each interface, because (at least with win32): the default 255.255.... broadcasts only on one (most likely the main or internet) network interface
        NetPack * net = 0;
        
#ifdef NDEBUG
        if ( localNetsSize <= 1 )
#else
            if ( localNetsSize <= 0 )
#endif
                return true;
        
        if ( !LockAcquireA ( localNetsLock, "SendBroadcast" ) ) {
            return false;
        }
        
        net = &localNets;
        while ( net ) {
            broadcastAddr.sin_addr.s_addr = net->bcast;
            
            sentBytes = ( int ) sendto ( sock, msg, sendLen, 0, ( struct sockaddr * ) &broadcastAddr, sizeof ( struct sockaddr ) );
            if ( sentBytes != sendLen ) {
#ifndef MEDIATORDAEMON
                if ( env->environsState >= environs::Status::Starting )
#endif
                    CErr ( "SendBroadcast: Broadcast failed!" );
                success = false;
            }
            
            net = net->next;
        }
        
        LockReleaseVA ( localNetsLock, "SendBroadcast" );
        
        return success;
    }
    
#else
    bool Mediator::SendBroadcast ( bool enforce, bool sendStatus, bool sendToAny, bool enforceEnqueue )
    {
        CVerbVerb ( "SendBroadcast" );
        
        if ( IsInvalidFD ( broadcastSocket ) || ( !enforce && !broadcastRunning ) )
            return false;
        
#ifndef MEDIATORDAEMON
        if ( native.networkStatus < NETWORK_CONNECTION_NO_INTERNET )
            return false;
#endif
        
        bool success    = true;
        char * msg;
        int sendLen;
        
        if ( sendStatus ) {
            msg		= udpStatusMessage;
            sendLen = ( int ) udpStatusMessageLen;
        }
        else {
            msg		= broadcastMessage;
            sendLen = ( int ) broadcastMessageLen;
        }
        
        CVerbArg ( "SendBroadcast: Broadcasting %smessage [ %s ] ( %d )...", sendStatus ? "device status " : "", msg + 4, sendLen );

        if ( enforceEnqueue )
            success = PushSendBC ( true, msg, sendLen, INADDR_BROADCAST, GET_MEDIATOR_BASE_PORT );
        else
            SendBufferOrEnqueueBC ( true, msg, sendLen, INADDR_BROADCAST, GET_MEDIATOR_BASE_PORT );
        
        /// We need to broadcast to each interface, because (at least with win32): the default 255.255.... broadcasts only to one (most likely the main) network interface
        NetPack * net = 0;
        
#ifdef NDEBUG
        if ( localNetsSize <= 1 )
#else
            if ( localNetsSize <= 0 )
#endif
                return true;
        
        if ( !LockAcquireA ( localNetsLock, "SendBroadcast" ) ) {
            return false;
        }
        
        net = &localNets;
        while ( net ) {
            if ( enforceEnqueue )
                PushSendBC ( true, msg, sendLen, net->bcast, GET_MEDIATOR_BASE_PORT );
            else
                SendBufferOrEnqueueBC ( true, msg, sendLen, net->bcast, GET_MEDIATOR_BASE_PORT );
            net = net->next;
        }
        
        LockReleaseVA ( localNetsLock, "SendBroadcast" );
        
        return success;
    }
    
    
    bool Mediator::SendBroadcastWithSocket ( bool enforce, bool sendStatus, bool sendToAny, int sock )
    {
        CVerbVerb ( "SendBroadcastWithSocket" );
        
        if ( IsInvalidFD ( sock ) )
            sock = broadcastSocket;
        
        if ( IsInvalidFD ( sock ) || ( !enforce && !broadcastRunning ) )
            return false;
        
#ifndef MEDIATORDAEMON
        if ( native.networkStatus < NETWORK_CONNECTION_NO_INTERNET )
            return false;
#endif
        bool success    = true;
        int  sentBytes  = 0;
        char * msg;
        int sendLen;
        
        if ( sendStatus ) {
            msg		= udpStatusMessage;
            sendLen = ( int ) udpStatusMessageLen;
        }
        else {
            msg		= broadcastMessage;
            sendLen = ( int ) broadcastMessageLen;
        }
        
        CVerbArg ( "SendBroadcastWithSocket: Broadcasting %smessage [ %s ] ( %d )...", sendStatus ? "device status " : "", msg + 4, sendLen );
        
        struct 	sockaddr_in		broadcastAddr;
        
        if ( sock == broadcastSocket ) {
            if ( sendToAny )
                SendBufferOrEnqueueBC ( true, msg, sendLen, INADDR_BROADCAST, GET_MEDIATOR_BASE_PORT );
        }
        else {
            Zero ( broadcastAddr );
            
            broadcastAddr.sin_family = PF_INET;
            broadcastAddr.sin_port = htons ( GET_MEDIATOR_BASE_PORT );
            
            if ( sendToAny ) {
                broadcastAddr.sin_addr.s_addr = htonl ( INADDR_BROADCAST ); // 0xFFFFFFFF;
                
                sentBytes = ( int ) sendto ( sock, msg, sendLen, 0, ( struct sockaddr * ) &broadcastAddr, sizeof ( struct sockaddr ) );
                if ( sentBytes != sendLen )
                {
#ifndef MEDIATORDAEMON
                    if ( env->environsState >= environs::Status::Starting )
#endif
                        CErr ( "SendBroadcastWithSocket: Broadcast to any failed!" );
                }
            }
        }
        
        /// We need to broadcast to each interface, because (at least with win32): the default 255.255.... broadcasts only on one (most likely the main or internet) network interface
        NetPack * net = 0;
        
#ifdef NDEBUG
        if ( localNetsSize <= 1 )
#else
            if ( localNetsSize <= 0 )
#endif
                return true;
        
        if ( !LockAcquireA ( localNetsLock, "SendBroadcast" ) ) {
            return false;
        }
        
        net = &localNets;
        while ( net ) {
            if ( sock == broadcastSocket ) {
                SendBufferOrEnqueueBC ( true, msg, sendLen, net->bcast, GET_MEDIATOR_BASE_PORT );
            }
            else {
                broadcastAddr.sin_addr.s_addr = htonl ( net->bcast );
                
                sentBytes = ( int ) sendto ( sock, msg, sendLen, 0, ( struct sockaddr * ) &broadcastAddr, sizeof ( struct sockaddr ) );
                if ( sentBytes != sendLen ) {
#ifndef MEDIATORDAEMON
                    if ( env->environsState >= environs::Status::Starting )
#endif
                        CErr ( "SendBroadcast: Broadcast failed!" );
                    success = false;
                }
            }
            net = net->next;
        }
        
        LockReleaseVA ( localNetsLock, "SendBroadcast" );
        
        return success;
    }
    
#endif
    

	bool Mediator::IsLocalIP ( unsigned int ip )
	{
		bool ret = false;

		CVerb ( "IsLocalIP" );

		if ( !LockAcquireA ( localNetsLock, "IsLocalIP" ) )
			return false;

		NetPack * net = &localNets;

		while ( net ) {
			if ( ip == net->ip ) {
				ret = true;
				break;
			}
			net = net->next;
		}

		if ( !LockReleaseA ( localNetsLock, "IsLocalIP" ) )
			return false;
		return ret;
	}


	unsigned int Mediator::GetLocalIP ()
	{
		CVerb ( "GetLocalIP" );

		if ( !localNets.ip )
			if ( !LoadNetworks () )
				return 0;

		return localNets.ip;
	}


	unsigned int Mediator::GetLocalSN ()
	{
		CVerb ( "GetLocalSN" );

		return localNets.mask;
	}


	bool Mediator::IsAnonymousUser ( const char * user )
	{
		if ( !user || !strlen ( user ) )
			return true;

		int maxLen = ( int ) sizeof ( MEDIATOR_ANONYMOUS_USER );

		const char * anonUser = MEDIATOR_ANONYMOUS_USER;

		while ( *user && maxLen >= 0 )
		{
			if ( *user++ != *anonUser++ )
				break;
			maxLen--;
		}
		if ( maxLen <= 1 )
			return true;
		return false;
	}

	
	void Mediator::VanishedDeviceWatcher ()
	{
		DeviceInstanceNode	**	listRoot	= 0;
		DeviceInstanceNode	*	device		= 0;
		pthread_mutex_t		*	mutex		= 0;

#ifdef USE_WRITE_LOCKS1
        sp ( ApplicationDevices ) appDevices = GetDeviceList ( nill, nill, &mutex, nill, listRoot );

        if ( !listRoot || !appDevices )
            return;

        bool isListening = IsServiceAvailable ();

        if ( !appDevices->LockWrite ( "VanishedDeviceWatcher" ) )
            return;
#else
        GetDeviceList ( nill, nill, &mutex, nill, listRoot );

		if ( !listRoot )
			return;

		bool isListening = IsServiceAvailable ();

		if ( !LockAcquire ( mutex, "VanishedDeviceWatcher" ) )
            return;
#endif
        device = *listRoot;

		while ( device )
		{
			CVerbVerbArg ( "VanishedDeviceWatcher: Checking [ 0x%X ] Area [ %s ] App [ %s ]", device->info.deviceID, device->info.areaName, device->info.appName );

			if ( device->info.broadcastFound == DEVICEINFO_DEVICE_BROADCAST_AND_MEDIATOR && !isListening ) {
				device->info.broadcastFound = DEVICEINFO_DEVICE_BROADCAST;
			}

			if ( device->info.broadcastFound != DEVICEINFO_DEVICE_MEDIATOR && ( lastGreetUpdate - device->info.updates ) > 120000 )
			{
				// Device has vanished
				DeviceInstanceNode	*	vanished = device;

#ifndef MEDIATORDAEMON
                // Is it connected?
                sp ( DeviceController) deviceSP = device->deviceSP.lock ();
                if ( deviceSP && deviceSP->deviceStatus == DeviceStatus::Connected )
                {
                    device = device->next;
                    continue;
                }
#endif
				//device = vanished->next;

				//if ( !vanished->prev ) {
				//	// The root device vanished
				//	if ( device )
				//		device->prev = 0;

				//	*listRoot = device;
				//}
				//else
				//	vanished->prev->next = device;

				//vanished->next = 0;

                // Remove device and relink the list
                CLogArg ( "VanishedDeviceWatcher: Removing [ 0x%X ] Area [ %s ] App [ %s ]", device->info.deviceID, device->info.areaName, device->info.appName );
				RemoveDevice ( vanished, false );

				device = *listRoot;
				continue;
			}
            
			device = device->next;
		}

#ifdef USE_WRITE_LOCKS1
        appDevices->UnlockWrite ( "VanishedDeviceWatcher" );
#else
		LockReleaseV ( mutex, "VanishedDeviceWatcher" );
#endif
	}


#ifdef MEDIATORDAEMON

	sp ( DeviceInstanceNode ) Mediator::UpdateDevicesV4 ( unsigned int ip, char * msg, char ** uid, bool * created, char broadcastFound )
	{
		CVerbVerb ( "UpdateDevicesV4" );

		if ( !msg )
			return 0;

		int				value;
		//unsigned int	flags		= 0;
		bool			found		= false;
		bool			changed		= false;
		char		*	deviceName	= 0;
		char		*	areaName	= 0;
		char		*	appName		= 0;
		//char		*	userName	= 0;

		DeviceInstanceNode * device		= 0;
		DeviceInstanceNode * devicePrev	= 0;
		DeviceInstanceNode ** listRoot	= 0;

		sp ( DeviceInstanceNode ) deviceSP = 0;

		pthread_mutex_t * mutex		= 0;

		int *						pDevicesAvailable	= 0;

		sp ( ApplicationDevices ) appDevices			= 0;

		// Get the id at first (>0)	
		int * pInt = ( int * ) ( msg + MEDIATOR_BROADCAST_DEVICEID_START );
		value = *pInt;
		if ( !value )
			return 0;

		// Get the areaName, appname, etc..
		char * context = NULL;
		char * psem = strtok_s ( msg + MEDIATOR_BROADCAST_DESC_START, ";", &context );
		if ( !psem )
			return 0;
		deviceName = psem;

		psem = strtok_s ( NULL, ";", &context );
		if ( !psem )
			return 0;
		areaName = psem;

		psem = strtok_s ( NULL, ";", &context );
		if ( !psem )
			return 0;
		appName = psem;

		if ( uid ) {
			psem = strtok_s ( NULL, ";", &context );
			if ( psem )
				*uid = psem;
		}
		/*psem = strtok_s ( NULL, ";", &context );
		if ( psem ) {
		userName = psem;
		}*/

#if !defined(MEDIATORDAEMON)
		if ( value == env->deviceID )
		{
			if ( !strncmp ( areaName, env->areaName, sizeof ( env->areaName ) - 1 ) && !strncmp ( appName, env->appName, sizeof ( env->appName ) - 1 ) )
			{
				// Another device has broadcasted with our app area identifiers.
				// If we have just started, then generate a random deviceID and start again. Otherwise, let's ignore this.
				CWarnArg ( "UpdateDevicesV4: Another device with our identifiers found [ %i ].", broadcastReceives );

				if ( broadcastReceives < 100 ) {
					BroadcastByeBye ();
					env->deviceID = 0;

					environs::API::SetDeviceID ( env->hEnvirons, 0 );

					SendBroadcast ();

					StopMediators ();
					return 0;
				}

				// Broadcast ourself to indicate that we are gonna take this "slot"
				return 1;
			}
		}

		if ( env->mediatorFilterLevel > MEDIATOR_FILTER_NONE ) {
			if ( strncmp ( areaName, env->areaName, sizeof ( env->areaName ) - 1 ) )
				return 1; // We ignore the device, but broadcast ourself to indicate that we are there ...

			if ( env->mediatorFilterLevel > MEDIATOR_FILTER_AREA ) {
				if ( strncmp ( appName, env->appName, sizeof ( env->appName ) - 1 ) )
					return 1; // We ignore the device, but broadcast ourself to indicate that we are there ...
			}
		}
#endif
		appDevices = GetDeviceList ( areaName, appName, &mutex, &pDevicesAvailable, listRoot );
		if ( !listRoot )
			return 0;

#ifdef USE_WRITE_LOCKS1
        if ( !appDevices->LockWrite ( "UpdateDevicesV4" ) )
#else
		if ( !LockAcquire ( mutex, "UpdateDevicesV4" ) )
#endif
			goto Finish;

		device = *listRoot;

		while ( device )
		{
			CVerbVerbArg ( "UpdateDevicesV4: Comparing [0x%X / 0x%X] Area [%s / %s] App [%s / %s]", device->info.deviceID, value, device->info.areaName, areaName, device->info.appName, appName );

			if ( device->info.deviceID == value
#ifndef MEDIATORDAEMON
				&& !strncmp ( device->info.areaName, areaName, sizeof ( device->info.areaName ) ) && !strncmp ( device->info.appName, appName, sizeof ( device->info.appName ) )
#endif
				)
			{
#ifndef MEDIATORDAEMON
				if ( strncmp ( device->info.deviceName, deviceName, sizeof ( device->info.deviceName ) ) )
				{
					// Another device with the same identifiers has already been registered. Let's ignore this.
					LockReleaseV ( mutex, "UpdateDevicesV4" );
					goto Finish;
				}
#endif
				found = true; device->info.unavailable = true;
				break;
			}

			if ( !device->next || device->info.deviceID > value )
				break;

			devicePrev = device;
			device = device->next;
		}

#ifdef MEDIATORDAEMON
		if ( found ) {
			deviceSP = device->baseSP;
			LockReleaseV ( mutex, "UpdateDevicesV4" );
			return deviceSP;
		}
		if ( created )
			*created = true;
#endif

		while ( !found ) {
			// Create a new list node
			DeviceInstanceNode	*		dev		= 0;
			deviceSP	= sp_make ( DeviceInstanceNode );

			if ( !deviceSP || ( dev = deviceSP.get () ) == 0 ) {
				CErr ( "UpdateDevicesV4: Failed to allocate memory for new device!" );
				device = 0;
				break;
			}
			dev->baseSP = deviceSP;

			dev->info.deviceID = value;
			( *pDevicesAvailable )++;

#ifdef MEDIATORDAEMON
			dev->rootSP = appDevices;
#else
			dev->hEnvirons = env->hEnvirons;
			dev->info.broadcastFound = broadcastFound;

			// Build the key
#ifdef USE_MEDIATOR_OPT_KEY_MAPS_COMP
            dev->key.deviceID = value;

            int copied = snprintf ( dev->key.appArea, sizeof ( dev->key ) - 4, "%s %s", areaName, appName );
#else
			int copied = snprintf ( dev->key, sizeof ( dev->key ), MEDIATOR_APP_AREA_KEY_FORMAT, value, areaName, appName );
#endif
			if ( copied <= 0 ) {
				CErr ( "UpdateDevicesV4: Failed to build the key for new device!" );
			}
#endif
			if ( devicePrev )
			{
				dev->prev = devicePrev;

				if ( devicePrev->next ) {
					// Glue next links together
					dev->next = devicePrev->next;
					devicePrev->next = dev;

					// Glue previous links together
					dev->next->prev = dev;
				}
				else
					devicePrev->next = dev;
			}
			else {
				if ( device ) {
					if ( device->info.deviceID > value ) {
						/// Device must be the listRoot (because there is no previous device)
						dev->next = device;
						device->prev = dev;
						*listRoot = dev;
					}
					else {
						dev->prev = device;

						if ( device->next ) {
							// Glue next links together
							dev->next = device->next;
							device->next = dev;

							// Glue previous links together
							dev->next->prev = dev;
						}
						else
							device->next = dev;
					}
				}
				else
					*listRoot = dev;
			}
			device = dev;

			changed = true; //flags |= DEVICE_INFO_ATTR_AREA_NAME | DEVICE_INFO_ATTR_DEVICE_NAME;
			strlcpy ( device->info.areaName, areaName, sizeof ( device->info.areaName ) );
			strlcpy ( device->info.appName, appName, sizeof ( device->info.appName ) );
			//if ( userName ) {
			//	strlcpy ( device->userName, sizeof(device->userName) - 1, userName );
			//	//flags |= DEVICE_INFO_ATTR_USER_NAME;
			//}
			break;
		}

		if ( device ) {
			device->info.updates = lastGreetUpdate;

#ifndef MEDIATORDAEMON
			if ( !device->info.ipe && device->info.broadcastFound == DEVICEINFO_DEVICE_BROADCAST )
#endif
				if ( ip != device->info.ipe ) {
					device->info.ipe = ip; changed = true; //flags |= DEVICE_INFO_ATTR_IPE;
				}

			int platform = *( ( int * ) ( msg + MEDIATOR_BROADCAST_PLATFORM_START ) );

			if ( device->info.platform != platform ) {
				device->info.platform = platform; changed = true; //flags |= DEVICE_INFO_ATTR_DEVICE_TYPE;
			}

			pInt += 1;

			// Get the ip
			value = *pInt;
			if ( device->info.ip != ( unsigned ) value ) {
				device->info.ip = value; changed = true; //flags |= DEVICE_INFO_ATTR_IP;
			}
			// Get ports
			unsigned short * pUShort = ( unsigned short * ) ( msg + MEDIATOR_BROADCAST_PORTS_START );
			unsigned short svalue = *pUShort++;
			if ( svalue != device->info.tcpPort ) {
				device->info.tcpPort = svalue; changed = true; //flags |= DEVICE_INFO_ATTR_PORT_TCP;
			}

			svalue = *pUShort;
			if ( svalue != device->info.udpPort ) {
				device->info.udpPort = svalue; changed = true; //flags |= DEVICE_INFO_ATTR_PORT_UDP;
			}

			if ( strncmp ( device->info.deviceName, deviceName, sizeof ( device->info.deviceName ) - 1 ) ) {
				strlcpy ( device->info.deviceName, deviceName, sizeof ( device->info.deviceName ) ); changed = true; //flags |= DEVICE_INFO_ATTR_DEVICE_NAME;
			}

#ifdef MEDIATORDAEMON
			if ( device->info.broadcastFound != broadcastFound ) {
				device->info.broadcastFound = broadcastFound; changed = true; //flags |= DEVICE_INFO_ATTR_BROADCAST_FOUND;
			}
#endif
			//if ( userName && *userName && strncmp ( device->userName, userName, sizeof ( device->userName ) - 1 ) ) {
			//	strlcpy ( device->userName, userName, sizeof ( device->userName ) ); changed = true; //flags |= DEVICE_INFO_ATTR_USER_NAME;
			//}

			if ( changed ) {
				CVerbArg ( "UpdateDevicesV4: Device      = [0x%X / %s / %s]", device->info.deviceID, device->info.deviceName, device->info.broadcastFound ? "on same network" : "by mediator" );
				if ( device->info.ip != device->info.ipe ) {
					CVerbArg ( "UpdateDevicesV4: Device IPe != IP  [%s]", inet_ntoa ( *( ( struct in_addr * ) &device->info.ip ) ) );
					CVerbArg ( "UpdateDevicesV4: Device IP  != IPe [%s]", inet_ntoa ( *( ( struct in_addr * ) &device->info.ipe ) ) );
				}
				else {
					CListLogArg ( "UpdateDevicesV4: Device IPe [%s]", inet_ntoa ( *( ( struct in_addr * ) &device->info.ipe ) ) );
				}

				CListLogArg ( "UpdateDevicesV4: Area/App = [%s / %s]", device->info.areaName, device->info.appName );
				CListLogArg ( "UpdateDevicesV4: Device  IPe = [%s (from socket), tcp [%d], udp [%d]]", inet_ntoa ( *( ( struct in_addr * ) &ip ) ), device->info.tcpPort, device->info.udpPort );
			}
		}

#ifdef USE_WRITE_LOCKS1
        if ( !appDevices->UnlockWrite ( "UpdateDevicesV4" ) )
#else
		if ( !LockRelease ( mutex, "UpdateDevicesV4" ) )
#endif
        {
			if ( device ) {
				device->baseSP = 0;
				device = 0;
			}
		}
		else {
			if ( deviceSP )
				UpdateDeviceInstance ( deviceSP, !found, changed );
		}

	Finish:
		if ( appDevices )
			__sync_sub_and_fetch ( &appDevices->access, 1 );

		return deviceSP;
	}
#endif

    
#ifdef MEDIATORDAEMON
    sp ( DeviceInstanceNode )
#else
    bool
#endif
	Mediator::UpdateDevices ( unsigned int ip, char * msg, char ** uid, bool * created, char broadcastFound )
	{
		CVerbVerb ( "UpdateDevices" );

		if ( !msg )
			return 0;

		int				value;
		bool			found		= false;
		bool			changed		= false;
		char		*	deviceName	= 0;
		char		*	areaName	= 0;
		char		*	appName		= 0;

		DeviceInstanceNode * device		= 0;
		DeviceInstanceNode * devicePrev	= 0;
		DeviceInstanceNode ** listRoot	= 0;

		sp ( DeviceInstanceNode ) deviceSP = 0;

		pthread_mutex_t * mutex		= 0;

		int *						pDevicesAvailable	= 0;

        sp ( ApplicationDevices ) appDevices			= 0;

		// Get the id at first (>0)	
		int * pInt = ( int * ) ( msg + MEDIATOR_BROADCAST_DEVICEID_START );
		value = *pInt;
		if ( !value )
			return 0;

		// Get the areaName, appname, etc..

		unsigned char * sizesDst = ( unsigned char * ) ( msg + MEDIATOR_BROADCAST_DESC_START );

		// Get the areaName, appname, etc..
		appName  = ( char * ) ( sizesDst + 2 );
		areaName = appName + *sizesDst;

		if ( !*appName || *sizesDst >= MAX_NAMEPROPERTY || !*areaName )
			return 0;

		sizesDst++;

		deviceName = areaName + *sizesDst + 2;

		if ( *sizesDst >= MAX_NAMEPROPERTY || !*deviceName || *( deviceName - 2 ) >= ( MAX_NAMEPROPERTY + 1 ) )
			return 0;

		if ( uid ) {
			char * uuid = deviceName + *( deviceName - 2 );

			if ( *( (unsigned char *) deviceName - 1 ) >= 180 || !*uuid )
				return 0;

			*uid = uuid;
		}

#if !defined(MEDIATORDAEMON)
        bool sameApp = ( strncmp ( appName, env->appName, sizeof ( env->appName ) - 1 ) == 0);
        bool sameArea = ( strncmp ( areaName, env->areaName, sizeof ( env->areaName ) - 1 ) == 0);
        
		if ( value == env->deviceID )
        {
            if ( sameApp && sameArea )
            {
				// Another device has broadcasted with our app area identifiers.
				// If we have just started, then generate a random deviceID and start again. Otherwise, let's ignore this.
				CWarnArg ( "UpdateDevices: Another device with our identifiers found [ %i ].", broadcastReceives );

				if ( broadcastReceives < 100 ) {
					BroadcastByeBye ();
					env->deviceID = 0;

					environs::API::SetDeviceID ( env->hEnvirons, 0 );

					SendBroadcast ();

					StopMediators ();
					return 0;
				}

				// Broadcast ourself to indicate that we are gonna take this "slot"
				return 1;
            }
        }

        if ( env->mediatorFilterLevel > MEDIATOR_FILTER_NONE ) {
            if ( !sameArea )
                return 1; // We ignore the device, but broadcast ourself to indicate that we are there ...

            if ( env->mediatorFilterLevel > MEDIATOR_FILTER_AREA ) {
                if ( !sameApp )
                    return 1; // We ignore the device, but broadcast ourself to indicate that we are there ...
			}
		}
#endif
		appDevices = GetDeviceList ( areaName, appName, &mutex, &pDevicesAvailable, listRoot );
		if ( !listRoot )
			return 0;

#ifdef USE_WRITE_LOCKS1
        if ( !appDevices->LockWrite ( "UpdateDevices" ) )
#else
		if ( !LockAcquire ( mutex, "UpdateDevices" ) )
#endif
			goto Finish;

		device = *listRoot;

		while ( device )
		{
			CVerbVerbArg ( "UpdateDevices: Comparing [0x%X / 0x%X] Area [%s / %s] App [%s / %s]", device->info.deviceID, value, device->info.areaName, areaName, device->info.appName, appName );

			if ( device->info.deviceID == value
#ifndef MEDIATORDAEMON
				&& !strncmp ( device->info.areaName, areaName, sizeof ( device->info.areaName ) ) && !strncmp ( device->info.appName, appName, sizeof ( device->info.appName ) )
#endif
                )
            {
#ifndef MEDIATORDAEMON
				if ( strncmp ( device->info.deviceName, deviceName, sizeof ( device->info.deviceName ) ) )
				{
					// Another device with the same identifiers has already been registered. Let's ignore this.
					LockReleaseV ( mutex, "UpdateDevices" );
					goto Finish;
				}
#endif
				found = true; device->info.unavailable = true;
				break;
			}

			if ( !device->next || device->info.deviceID > value )
				break;

			devicePrev = device;
			device = device->next;
		}

#ifdef MEDIATORDAEMON
		if ( found ) {
			deviceSP = device->baseSP;
			LockReleaseV ( mutex, "UpdateDevices" );
			return deviceSP;
		}
		if ( created )
			*created = true;
#endif

		while ( !found ) {
			// Create a new list node
			DeviceInstanceNode	*		dev		= 0;
			deviceSP	= sp_make ( DeviceInstanceNode );

			if ( !deviceSP || ( dev = deviceSP.get () ) == 0 ) {
				CErr ( "UpdateDevices: Failed to allocate memory for new device!" );
				device = 0;
				break;
			}
            dev->baseSP = deviceSP;

			dev->info.deviceID = value;
			( *pDevicesAvailable )++;

#ifdef MEDIATORDAEMON
            dev->rootSP = appDevices;
            appDevices->deviceCacheDirty = true;
#else
			dev->hEnvirons           = env->hEnvirons;
            dev->info.broadcastFound = broadcastFound;
            dev->info.flags          = DeviceFlagsInternal::NativeReady;
            
            dev->allowConnect        = env->allowConnectDefault;

			// Build the key
#ifdef USE_MEDIATOR_OPT_KEY_MAPS_COMP
            dev->key.deviceID = value;

			int copied = snprintf ( dev->key.appArea, sizeof ( dev->key ) - 4, "%s %s", areaName, appName );
#else
            int copied = snprintf ( dev->key, sizeof ( dev->key ), MEDIATOR_APP_AREA_KEY_FORMAT, value, areaName, appName );
#endif
			if ( copied <= 0 ) {
				CErr ( "UpdateDevices: Failed to build the key for new device!" );
			}
#endif
			if ( devicePrev )
			{
				dev->prev = devicePrev;

				if ( devicePrev->next ) {
					// Glue next links together
					dev->next = devicePrev->next;
					devicePrev->next = dev;

					// Glue previous links together
					dev->next->prev = dev;
				}
				else
					devicePrev->next = dev;
			}
			else {
				if ( device ) {
					if ( device->info.deviceID > value ) {
						/// Device must be the listRoot (because there is no previous device)
						dev->next = device;
						device->prev = dev;
						*listRoot = dev;
					}
					else {
						dev->prev = device;

						if ( device->next ) {
							// Glue next links together
							dev->next = device->next;
							device->next = dev;

							// Glue previous links together
							dev->next->prev = dev;
						}
						else
							device->next = dev;
					}
				}
				else
					*listRoot = dev;
			}
			device = dev;

			changed = true; //flags |= DEVICE_INFO_ATTR_AREA_NAME | DEVICE_INFO_ATTR_DEVICE_NAME;
            
			strlcpy ( device->info.areaName, areaName, sizeof ( device->info.areaName ) );
			strlcpy ( device->info.appName, appName, sizeof ( device->info.appName ) );
			break;
		}

		if ( device ) {
			device->info.updates = lastGreetUpdate;

#ifndef MEDIATORDAEMON
            device->info.hasAppEnv = (sameApp && sameArea) ? 0 : 1;
            
			if ( !device->info.ipe && device->info.broadcastFound == DEVICEINFO_DEVICE_BROADCAST )
#endif
            if ( ip != device->info.ipe ) {
					device->info.ipe = ip; changed = true; //flags |= DEVICE_INFO_ATTR_IPE;
			}

                int platform = *( ( int * ) ( msg + MEDIATOR_BROADCAST_PLATFORM_START ) );

                if ( device->info.platform != platform ) {
				device->info.platform = platform; changed = true; //flags |= DEVICE_INFO_ATTR_DEVICE_TYPE;
			}

			pInt += 1;

			// Get the ip
			value = *pInt;
			if ( device->info.ip != ( unsigned ) value ) {
				device->info.ip = value; changed = true; //flags |= DEVICE_INFO_ATTR_IP;
			}
			// Get ports
			unsigned short * pUShort = ( unsigned short * ) ( msg + MEDIATOR_BROADCAST_PORTS_START );
			unsigned short svalue = *pUShort++;
			if ( svalue != device->info.tcpPort ) {
				device->info.tcpPort = svalue; changed = true; //flags |= DEVICE_INFO_ATTR_PORT_TCP;
			}

			svalue = *pUShort;
			if ( svalue != device->info.udpPort ) {
				device->info.udpPort = svalue; changed = true; //flags |= DEVICE_INFO_ATTR_PORT_UDP;
			}

			if ( strncmp ( device->info.deviceName, deviceName, sizeof ( device->info.deviceName ) - 1 ) ) {
				strlcpy ( device->info.deviceName, deviceName, sizeof ( device->info.deviceName ) ); changed = true; //flags |= DEVICE_INFO_ATTR_DEVICE_NAME;
			}

#ifdef MEDIATORDAEMON
			if ( device->info.broadcastFound != broadcastFound ) {
				device->info.broadcastFound = broadcastFound; changed = true; //flags |= DEVICE_INFO_ATTR_BROADCAST_FOUND;
            }
#endif
			//if ( userName && *userName && strncmp ( device->userName, userName, sizeof ( device->userName ) - 1 ) ) {
			//	strlcpy ( device->userName, userName, sizeof ( device->userName ) ); changed = true; //flags |= DEVICE_INFO_ATTR_USER_NAME;
			//}

			if ( changed ) {
				CVerbArg ( "UpdateDevices: Device      = [0x%X / %s / %s]", device->info.deviceID, device->info.deviceName, device->info.broadcastFound ? "on same network" : "by mediator" );
				if ( device->info.ip != device->info.ipe ) {
					CVerbArg ( "UpdateDevices: Device IPe != IP  [%s]", inet_ntoa ( *( ( struct in_addr * ) &device->info.ip ) ) );
					CVerbArg ( "UpdateDevices: Device IP  != IPe [%s]", inet_ntoa ( *( ( struct in_addr * ) &device->info.ipe ) ) );
				}
				else {
					CListLogArg ( "UpdateDevices: Device IPe [%s]", inet_ntoa ( *( ( struct in_addr * ) &device->info.ipe ) ) );
				}

				CListLogArg ( "UpdateDevices: Area/App = [%s / %s]", device->info.areaName, device->info.appName );
				CListLogArg ( "UpdateDevices: Device  IPe = [%s (from socket), tcp [%d], udp [%d]]", inet_ntoa ( *( ( struct in_addr * ) &ip ) ), device->info.tcpPort, device->info.udpPort );
			}
		}

#ifdef MEDIATORDAEMON
        if ( !appDevices->UnlockWrite ( "UpdateDevices" ) )
#else
		if ( !LockRelease ( mutex, "UpdateDevices" ) )
#endif
        {
			if ( device ) {
				device->baseSP = 0;
				device = 0;
			}
		}
		else {
            if ( deviceSP )
                UpdateDeviceInstance ( deviceSP, !found, changed );
		}

	Finish:
#ifdef MEDIATORDAEMON
		if ( appDevices )
            __sync_sub_and_fetch ( &appDevices->access, 1 );
        
        return deviceSP;
#else
		return 1; // ( device != 0 );
#endif
}


	void Mediator::DevicesHasChanged ( int type )
	{
		CVerb ( "DevicesHasChanged" );
	}


	void printDevices ( DeviceInstanceNode * device )
	{
		while ( device )
		{
			CLogArg ( "printDevices: Device id      = 0x%X", device->info.deviceID );
			CLogArg ( "printDevices: Area name      = %s", device->info.areaName );
			CLogArg ( "printDevices: App name       = %s", device->info.appName );
			CLogArg ( "printDevices: Platform       = %i", device->info.platform );
			CLogArg ( "printDevices: Device IPe     = %s (from socket)", inet_ntoa ( *( ( struct in_addr * ) &device->info.ip ) ) );
			CLogArg ( "printDevices: Device IPe != IP (%s)", inet_ntoa ( *( ( struct in_addr * ) &device->info.ipe ) ) );
			CLogArg ( "printDevices: Device tcpPort = %d", device->info.tcpPort );
			CLogArg ( "printDevices: Device udpPort = %d", device->info.udpPort );
			CLogArg ( "printDevices: Device name    = %s", device->info.deviceName );

			device = device->next;
		}
	}


	MediatorInstance * Mediator::IsKnownMediator ( unsigned int ip, unsigned short port )
	{
		CVerb ( "IsKnownMediator" );

		MediatorInstance * ret = 0;

		MediatorInstance * med = &mediator;

		while ( med ) {
			if ( ip == med->ip && med->port == port ) {
				ret = med;
				//med->available = true;

				// Register again
				CVerb ( "IsKnownMediator: already know." );
				/*
				#ifndef MEDIATORDAEMON
				if ( (opt_useDefaultMediator || opt_useCustomMediator) && environs::ID ) {
				#endif
				if ( pthread_mutex_trylock ( &mediatorLock ) ) {
				CVerb ( "IsKnownMediator: Failed to aquire mutex on mediator!" );
				return false;
				}

				RegisterAtMediator ( med );

				if ( pthread_mutex_unlock ( &mediatorLock ) ) {
				CErr ( "IsKnownMediator: Failed to release mutex on mediator!" );
				}
				#ifndef MEDIATORDAEMON
				}
				#endif
				*/
				break;
			}
			med = med->next;
		}
		return ret;
	}


	bool Mediator::RegisterAtMediator ( MediatorInstance * med )
	{
		return true;
	}


	void * Mediator::BroadcastThreadStarter ( void *arg )
	{
		if ( !arg ) {
			CErr ( "BroadcastThreadStarter: Called with invalid (NULL) argument.\n" );
			return 0;
		}

		Mediator * mediator = ( Mediator * ) arg;

    Retry:
        mediator->broadcastThread.ResetSync ( "BroadcastThreadStarter" );
        
		if ( mediator->isRunning && IsValidFD ( mediator->broadcastSocket ) )
		{
			// Execute thread
			mediator->BroadcastThread ();
		}
        
        if ( mediator->isRunning && IsValidFD ( mediator->broadcastSocket )
#ifndef MEDIATORDAEMON
            && mediator->env->environsState >= environs::Status::Starting
#endif
			)
        {
			int waits = 0;

			// Let's restart the thread. Something has shut us down unexpectedly
			// while all indicators expect us to be running for a working mediator layer
			if ( mediator->broadcastThreadRestarts > 15 ) {
				CErr ( "BroadcastThreadStarter: Failed to restart thread." );
				goto Finish;
			}
			
			mediator->broadcastThreadRestarts++;

            if ( mediator->broadcastThreadRestarts > 3 )
            {
				waits = mediator->broadcastThreadRestarts - 3;
            }

			if ( waits > 0 ) {
				CWarnArg ( "BroadcastThreadStarter: Wait state before restart [ %i ] ...", waits );

#ifndef MEDIATORDAEMON
				mediator->ReleaseDevices ();
#endif
				// Let's wait a while and retry
				Sleep ( waits * 10000 );
			}

			// Close broadcast listener socket
			int sock = mediator->broadcastSocket;
			if ( IsValidFD ( sock ) ) {
				CVerb ( "BroadcastThreadStarter: Closing socket." );

				mediator->broadcastSocket = INVALID_FD;
				ShutdownCloseSocket ( sock, true, "BroadcastThreadStarter broadcastSocket" );
			}

			if ( mediator->PrepareAndStartBroadcastThread ( false ) )
			{
				CWarn ( "BroadcastThreadStarter: Restarted thread." );
				goto Retry;
			}

            // Restart has failed several times and we are not able to reinitiate resources.
            // We're giving up this thread context here
			CErr ( "BroadcastThreadStarter: Failed to restart thread." );                      
        }

	Finish:
		mediator->broadcastThread.Notify ( "BroadcastThreadStarter" );

		mediator->broadcastThread.Detach ( "BroadcastThreadStarter" );

#ifndef MEDIATORDAEMON
		CVerbs ( 1, "BroadcastThreadStarter: bye bye ..." );
#endif
        return 0;
	}


	bool Mediator::AddMediator ( unsigned int ip, unsigned short port )
	{
		CVerbArg ( "AddMediator: IP [ %s : %d ]", inet_ntoa ( *( ( struct in_addr * ) &ip ) ), port );

		if ( !port ) {
			CWarn ( "AddMediator: port [ 0 ] is invalid." );
			return false;
		}

		if ( IsKnownMediator ( ip, port ) ) {
			CVerbArg ( "AddMediator: Mediator [ %s ] already available.", inet_ntoa ( *( ( struct in_addr * ) &ip ) ) );
			return true;
		}

        MediatorInstance * med = new MediatorInstance ();
		if ( !med ) {
			CErr ( "AddMediator: Failed to allocate instance!" );
			return false;
        }
        
        if ( !med->Init () )
            goto Failed;
        
#ifndef MEDIATORDAEMON
        med->connection.env = env;
		med->connection.instance.thread.autoreset = false;
#endif
        med->ip             = ip;
        med->port           = port;
                
		AddMediator ( med );

		return true;
        
    Failed:
		delete med;
		return false;
	}


	MediatorInstance * Mediator::AddMediator ( MediatorInstance * med )
	{
		CVerb ( "AddMediator" );

		if ( !med ) {
			CErr ( "AddMediator: Invalid argument!" );
			return 0;
		}
		MediatorInstance * added = med;
        
#ifdef ENABLE_MEDIATOR_LOCK
		if ( !LockAcquire ( &mediatorLock, "AddMediator" ) ) {
			CErr ( "AddMediator: Failed to aquire mutex on mediator!" );
			added = 0;
			goto Failed;
		}
#endif

		if ( mediator.ip ) {
			// Find the last and make sure that we do not have this mediator already in the list
			MediatorInstance * t = &mediator;
			while ( t->next ) {
				t = t->next;
				if ( t->ip == med->ip && t->port == med->port ) {
					CVerbArg ( "AddMediator: Mediator [ %s : %d ] is already in our list.", inet_ntoa ( *((struct in_addr *) &t->ip) ), t->port );
					added = 0;
					goto FinishUnlock;
				}
			}

			// Attach the new one
			t->next = med;
			med = 0;
		}
		else {
#ifndef MEDIATORDAEMON
            mediator.connection.env = med->connection.env;
#endif
            mediator.ip             = med->ip;
            mediator.port           = med->port;
            
            added = &mediator;
		}

    FinishUnlock:

#ifdef ENABLE_MEDIATOR_LOCK
		LockReleaseV ( &mediatorLock, "AddMediator" );
        
    Failed:
#endif
		if ( med ) {
            delete med;
//            ReleaseMediator ( med );
		}
		return added;
	}


	bool Mediator::RemoveMediator ( unsigned int ip )
	{
		CVerb ( "RemoveMediator" );

        if ( isRunning )
            return false;
        
#ifdef ENABLE_MEDIATOR_LOCK
		if ( !LockAcquire ( &mediatorLock, "RemoveMediator" ) )
			return false;
#endif
		// Find the mediator
		MediatorInstance * s = 0;
		MediatorInstance * t = &mediator;
		do {
			if ( ip == t->ip )
				break;

			s = t;
			t = t->next;
		}
		while ( t );

		if ( t ) {
			MediatorInstance * src = t->next;
			if ( t == &mediator ) {
				if ( src ) {
					// There is a next instance
					*t = *src;
					ReleaseMediator ( src );
				}
				else {
					// No more instances
					ReleaseMediator ( src );

                    ZeroStruct ( *t, MediatorInstance );

					/*MediatorInstance *tmp = new MediatorInstance ( );
					if ( tmp ) {
						*t = *tmp;
						delete tmp;
					}
                    */
				}
			}
			else {
				s->next = t->next;
				ReleaseMediator ( t );
			}
		}
        
#ifdef ENABLE_MEDIATOR_LOCK
		if ( !LockRelease ( &mediatorLock, "RemoveMediator" ) )
			return false;
#endif
		return true;
	}
    
    
#define USE_CONNECT_WITH_POLL
    
#ifdef USE_CONNECT_WITH_POLL
    int Mediator::Connect ( int deviceID, int &sock, struct sockaddr * addr, int timeoutSeconds, const char * name )
    {
        int rc = INVALID_FD;
        
        if ( IsInvalidFD ( sock ) ) {
            CVerbsArgID ( 2, "[ %s ].Connect: Invalid socket!", name );
            return INVALID_FD;
        }
        
        CVerbsArgID ( 6, "[ %s ].Connect: Set to non-blocking mode ...", name );
        
        if ( !SetNonBlockSocket ( sock, true, name ) )
            goto EndWithStatus;
        
        CVerbsArgID ( 5, "[ %s ].Connect: Connect ...", name );
        
        rc = ::connect ( sock, addr, sizeof ( struct sockaddr ) );
        if ( rc < 0 ) {
            if ( SOCK_IN_PROGRESS )
            {
                struct pollfd desc;
                
                int timeout = timeoutSeconds ? timeoutSeconds : 4;
                timeout *= 1000;
                
                do
                {
                    CVerbsArgID ( 5, "[ %s ].Connect: Waiting ...", name );
                    
                    int s = sock;
                    if ( IsInvalidFD ( s ) )
                        break;

					desc.events		= POLLOUT;
                    desc.fd         = sock;
                    desc.revents    = 0;
                    
                    rc = poll ( &desc, 1, timeout );
                    if ( rc > 0 )
                    {
#if !defined (_WIN32)
                        int check;
						socklen_t len = sizeof ( check );
                        
                        if ( getsockopt ( sock, SOL_SOCKET, SO_ERROR, ( char * ) &check, &len ) != 0 )
                        {
                            CVerbsArgID ( 4, "[ %s ].Connect: Get socket opt failed.", name );
                            rc = -1;
                        }
                        else if ( check )
                        {
                            CErrArgID ( "[ %s ].Connect: Failed!", name );
                            
                            if ( IsValidFD ( sock ) ) { LogSocketErrorF ( "MediatorClient.Connect" ); }
                            
                            rc = -1;
                        }
                        else
#endif
                            rc = 0;
                    }
                    else {
                        CVerbsArgID ( 5, "[ %s ].Connect: %s!", name, rc == 0 ? "Timeout" : "Failed" );
                        LogSocketErrorF ( "MediatorClient.Connect" );
                        rc = -1;
                    }
                    
                    CVerbsArgID ( 4, "[ %s ].Connect: %s [ rc = %i ]", name, rc > 0 ? "Failed" : "Success", rc );
                }
                while ( false );
            }
            else {
                CVerbsArgID ( 6, "[ %s ].Connect: Connect seems successful.", name );
            }
        }
        else {
            CVerbsArgID ( 4, "[ %s ].Connect: Connect failed.", name );
        }
        
        if ( rc == 0 ) {
            CVerbsArgID ( 6, "[ %s ].Connect: Set to blocking mode ...", name );
            
            SetNonBlockSocket ( sock, false, name );
        }
        
    EndWithStatus:
        return rc;
    }
#else
    int Mediator::Connect ( int deviceID, int &sock, struct sockaddr * addr, int timeoutSeconds, const char * name )
	{
		int rc = INVALID_FD;

		if ( IsInvalidFD ( sock ) ) {
			CVerbsArgID ( 2, "[ %s ].Connect: Invalid socket!", name );
			return INVALID_FD;
		}
        
        CVerbsArgID ( 6, "[ %s ].Connect: Set to non-blocking mode ...", name );
        
        if ( !SetNonBlockSocket ( sock, true, name ) )
            goto EndWithStatus;
        
        CVerbsArgID ( 5, "[ %s ].Connect: Connect ...", name );
        
        rc = connect ( sock, addr, sizeof ( struct sockaddr ) );
        if ( rc < 0 ) {
            if ( SOCK_IN_PROGRESS )
            {
                do
                {
                    CVerbsArgID ( 5, "[ %s ].Connect: Waiting ...", name );
                    
                    fd_set fdw;
                    //fd_set fde;
                    struct timeval timeout;
                    Zero ( timeout );
                    
                    FD_ZERO ( &fdw );
                    
                    int s = sock;
                    if ( IsInvalidFD ( s ) )
                        break;
                    
                    FD_SET ( ( unsigned ) s, &fdw );
                    
                    /*FD_ZERO ( &fde );
                     FD_SET ( ( unsigned ) sock, &fde );*/
                    
                    timeout.tv_sec = timeoutSeconds ? timeoutSeconds : 4;
                    //					timeout.tv_usec = WAIT_TIME_FOR_CONNECTIONS * 1000;
                    
                    rc = select ( s + 1, NULL, &fdw, NULL, &timeout );
                    //rc = select ( sock + 1, NULL, &fdw, &fde, &timeout );
                    if ( rc > 0 ) {
#if defined (_WIN32)
                        if ( FD_ISSET ( sock, &fdw ) ) {
                            rc = 0;
                        }
                        
                        /*if ( FD_ISSET ( sock, &fde ) ) {
                         rc = -1;
                         
                         CErrArgID ( "[%s].Connect: Failed!", name );
                         LogSocketErrorF ( "MediatorClient.Connect" );
                         }*/
#else
                        int sockErr;
                        socklen_t len = sizeof sockErr;
                        
                        if ( getsockopt ( sock, SOL_SOCKET, SO_ERROR, ( char * ) &sockErr, &len ) != 0 )
                        {
                            CVerbsArgID ( 4, "[ %s ].Connect: Get socket opt failed.", name );
                            rc = -1;
                        }
                        else if ( sockErr )
                        {
                            CErrArgID ( "[ %s ].Connect: Failed!", name );
                            
                            if ( IsValidFD ( sock ) ) { LogSocketErrorF ( "MediatorClient.Connect" ); }
                            
                            rc = -1;
                        }
                        else
                            rc = 0;
#endif
                    }
                    else {
                        CVerbsArgID ( 5, "[ %s ].Connect: Select failed!", name );
                        VerbLogSocketErrorF ( "MediatorClient.Connect" );
                        rc = -1;
                    }
                    
                    CVerbsArgID ( 4, "[ %s ].Connect: %s [ rc = %i ]", name, rc != 0 ? "Failed" : "Success", rc );
                }
                while ( false );
            }
            else {
                CVerbsArgID ( 6, "[ %s ].Connect: Connect seems successful.", name );
            }
        }
        else {
            CVerbsArgID ( 4, "[ %s ].Connect: Connect failed.", name );
        }
        
        CVerbsArgID ( 6, "[ %s ].Connect: Set to blocking mode ...", name );
        
        if ( rc == 0 )
            SetNonBlockSocket ( sock, false, name );
        
    EndWithStatus:
        return rc;
    }
#endif
    
    void Mediator::UnConnectUDP ( int sock )
    {
        if ( IsInvalidFD ( sock ) )
            return;
        
        struct 	sockaddr_in			addr;
        Zero ( addr );
        
        addr.sin_family = AF_UNSPEC;
        
        ::connect ( sock, ( struct sockaddr * ) &addr, sizeof ( addr ) );
    }   
    
    
} /* namepace environs */









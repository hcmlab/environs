/**
 * Mediator Server main class
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
//#	define DEBUGVERB
//#	define DEBUGVERBVerb
#endif

#include "Mediator.Daemon.h"
#include "Environs.Release.h"
#include "Environs.Native.h"
#include "Environs.Utils.h"
#include "Tracer.h"
using namespace environs;

#ifndef _WIN32
#	if defined(OutputDebugStringA)
#		undef OutputDebugStringA
#	endif
#	define OutputDebugStringA(msg)		fputs(msg,stdout)
#endif

//#define MEDIATOR_USE_TCP_NODELAY

#define MAX_SEQNR_DELAY			10

#define SEND_THREADS_DEFAULT	2
#define CLIENT_THREADS_DEFAULT	3

int debugID = 0x14;
//#define DEBUG_DEVICE_ID
#define ENABLE_SEND_THREAD_TRYLOCK
//#define DEBUG_TIMEOUT_ISSUE

#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <queue>
using namespace std;

#ifdef WIN32
#	include <conio.h>
#	include "windows.h"
#	include <ws2tcpip.h>
#	include <Winhttp.h>
#	include "Environs.Build.Lnk.h"

// Link to winsock library
#	pragma comment ( lib, "Ws2_32.lib" )

// Link to winsock library
#	pragma comment ( lib, "Winhttp.lib" )
#else
#	include <string.h>

#	include "stdio.h"
#	include "errno.h"
#	include <cerrno>

#	include <termios.h>
#	include <curl/curl.h>
#	include <unistd.h>
#	include <fcntl.h>

int _getch ( )
{

   static int inchar = -1, fd = 0;
   struct termios cur, old;

   fd = fileno ( stdin );
   tcgetattr ( fd, &old );

   cur = old;
   cur.c_lflag &= ~(ICANON|ECHO);

   tcsetattr ( fd, TCSANOW, &cur );

   inchar = getchar ( );
   tcsetattr ( fd, TCSANOW, &old );

   return inchar;
}

#endif

#include <stdarg.h>

#ifndef CLASS_NAME
#	define CLASS_NAME		"Daemon"
#endif

#define USE_VERIFYSOCKETS

#ifndef NDEBUG
#   define DEBUG_CHECK_CLIENT_CALL_MS
#endif

#ifdef DEBUG_CHECK_CLIENT_CALL_MS
unsigned int maxMeasureDiff = 60;

#   define DEBUG_CHECK_START()          unsigned int start_ms_32 = GetEnvironsTickCount32 ()
#   define DEBUG_CHECK_START_1()        start_ms_32 = GetEnvironsTickCount32 ()
#   define DEBUG_CHECK_MEASURE(f)       unsigned int end_ms_32 = GetEnvironsTickCount32 (); unsigned int diff_ms_32 = end_ms_32 - start_ms_32; \
                                        if ( diff_ms_32 > maxMeasureDiff ) { char timeString [ 256 ]; \
                                            GetTimeString ( timeString, sizeof ( timeString ) ); \
                                            printf ( "%s" f ": [ %u ms ]\n", timeString, diff_ms_32 ); }
#   define DEBUG_CHECK_MEASURE_1(f)     end_ms_32 = GetEnvironsTickCount32 (); diff_ms_32 = end_ms_32 - start_ms_32; \
                                        if ( diff_ms_32 > maxMeasureDiff ) { char timeString [ 256 ]; \
                                            GetTimeString ( timeString, sizeof ( timeString ) ); \
                                            printf ( "%s" f ": [ %u ms ]\n", timeString, diff_ms_32 ); }
#else
#   define DEBUG_CHECK_START()
#   define DEBUG_CHECK_START_1()
#   define DEBUG_CHECK_MEASURE(f)
#   define DEBUG_CHECK_MEASURE_1(f)
#endif


namespace environs
{
	// Forward declarations
	bool				stdLog          = true;
	bool				logToFile;
	bool				fileLog			= true;
	ofstream			logfile;
	pthread_mutex_t     logMutex;

	bool                printStdOut     = true;


    size_t GetTimeString ( char * timeBuffer, unsigned int bufferSize );

    /*
	INLINEFUNC SOCKETSYNC InvalidateThreadSocket ( SOCKETSYNC * psock )
	{
		SOCKETSYNC sock = *psock;
		if ( sock != -1 ) {
			sock = ___sync_val_compare_and_swap ( psock, sock, -1 );
		}
		return sock;
	}


	INLINEFUNC SOCKETSYNC ReplaceThreadSocket ( SOCKETSYNC * psock, int replace )
	{
		SOCKETSYNC sock = *psock;
		sock = ___sync_val_compare_and_swap ( psock, sock, replace );
		return sock;
	}


	INLINEFUNC SOCKETSYNC CloseReplaceThreadSocket ( SOCKETSYNC * psock, int replace )
	{
		SOCKETSYNC sock = ReplaceThreadSocket ( psock, replace );
		if ( sock != -1 )
		{
			CSocketTraceRemove ( (int) sock, "CloseReplaceThreadSocket: Closing", 0 );

			shutdown ( ( int ) sock, 2 );
			closesocket ( ( int ) sock );
		}
		return sock;
	}*/


	bool CloseThreadSocket ( int * psock )
	{
        int sock = *psock; *psock = INVALID_FD; //ReplaceThreadSocket ( psock, -1 );
		if ( sock != INVALID_FD ) {
			CSocketTraceRemove ( (int) sock, "CloseThreadSocket: Closing", 0 );

			shutdown ( ( int ) sock, 2 );
			closesocket ( ( int ) sock );
			return true;
		}
		return false;
	}


	unsigned int GetBitMask ( unsigned int network )
	{
		unsigned int test = network;
		unsigned int mask = 0;

		for ( unsigned int i = 0; i < sizeof ( unsigned int ) * 8; i++ ) {
			if ( 0x80000000 & test ) {
				break;
			}
			test <<= 1;

			mask >>= 1;
			mask |= 0x80000000;
		}

		return ~mask;
	}

    
    const char * GetCommandString ( char c )
    {
        switch ( c ) {
            case MEDIATOR_CMD_SET:
                return "SET";
                
            case MEDIATOR_CMD_GET:
                return "GET";
                
            case MEDIATOR_CMD_GET_DEVICES:
                return "GET_DEVICES";
                
            case MEDIATOR_CMD_HEARTBEAT:
                return "HEARTBEAT";
                
            case MEDIATOR_CMD_GETALL:
                return "GETALL";
                
            case MEDIATOR_CMD_SET_FILTERMODE:
                return "SET_FILTERMODE";
                
            case MEDIATOR_CMD_SET_SOCKET_BUFFERS:
                return "SET_SOCKET_BUFFERS";
                
            case MEDIATOR_CMD_NATSTAT:
                return "NATSTAT";
                
            case MEDIATOR_CMD_NOTIFICATION_SUBSCRIBE:
                return "NOTIFICATION_SUBSCRIBE";
                
            case MEDIATOR_CMD_SHORT_MESSAGE:
                return "SHORT_MESSAGE";
                
            case MEDIATOR_CMD_MESSAGE_SUBSCRIBE:
                return "MESSAGE_SUBSCRIBE";
                
            case MEDIATOR_CMD_REQ_STUNT_ID:
                return "REQ_STUNT_ID";
                
            case MEDIATOR_SRV_CMD_ALIVE_REQUEST:
                return "ALIVE_REQUEST";
                
            case MEDIATOR_CMD_STUNT:
                return "STUNT";
                
            case MEDIATOR_CMD_STUN:
                return "STUN";
                
            case MEDIATOR_CMD_HELP_TLS_GEN:
                return "HELP_TLS_GEN";

			case MEDIATOR_CMD_STUNT_CLEAR:
				return "STUNT_CLEAR";
                
            case MEDIATOR_CMD_DEVICE_FLAGS:
                return "DEVICE_FLAGS";
                
            case MEDIATOR_CMD_DEVICE_LIST_QUERY_COUNT:
                return "DEVICE_LIST_QUERY_COUNT";

			case MEDIATOR_OPT_BLANK:
				return "LOGON";
                
            default:
                break;
        }
        return "Unknown";
    }
    
    
    ILock1::ILock1 ()
    {
        init1 =  LockInitA ( lock1 );
	}

	bool ILock1::Init1 ()
	{
		if ( !init1 )
			init1 =  LockInitA ( lock1 );
		return init1;
	}

	bool ILock1::Lock1 ( const char * func )
	{
		return LockAcquireA ( lock1, func );
	}

	bool ILock1::Unlock1 ( const char * func )
	{
		return LockReleaseA ( lock1, func );
	}

	ILock1::~ILock1 ()
	{
		if ( init1 )
			LockDisposeA ( lock1 );
    }


    ILock2::ILock2 ()
    {
        init2 =  LockInitA ( lock2 );
    }

    bool ILock2::Init2 ()
    {
        if ( !init2 )
            init2 =  LockInitA ( lock2 );
        return init2;
    }

    bool ILock2::Lock2 ( const char * func )
    {
        return LockAcquireA ( lock2, func );
    }

    bool ILock2::Unlock2 ( const char * func )
    {
        return LockReleaseA ( lock2, func );
    }

    ILock2::~ILock2 ()
    {
        if ( init2 )
            LockDisposeA ( lock2 );
    }
    
    
    MediatorThreadInstance::MediatorThreadInstance ()
	{
		Reset ();
	}

	void MediatorThreadInstance::Reset ()
	{
		allocated   = false;
		socketUdp   = -1;
	}


	bool MediatorThreadInstance::Init ()
	{
		if ( !allocated )
		{
			if ( !instance.Init () )
				return false;

			if ( !udp.Init () )
                return false;
            udp.autoreset = true;

			if ( !watchdog.Init () )
				return false;
            watchdog.autoreset = true;

			allocated = true;
		}

		return true;
	}


	MediatorThreadInstance::~MediatorThreadInstance ()
	{
		Dispose ();
	}


	void MediatorThreadInstance::Dispose ()
	{
		udp.Notify ( "MediatorDaemon.Dispose" );

		int sock = socketUdp;
		if ( sock != -1 ) {
			socketUdp = -1;
			CSocketTraceRemove ( sock, "MediatorThreadInstance Dispose: socketUdp", 0 );

			shutdown ( sock, 2 );
			closesocket ( sock );
		}

		if ( watchdog.isRunning () )
		{
			MediatorDaemon * daemon = ( MediatorDaemon * ) instance.daemon;

			if ( pthread_cond_signal ( &daemon->hWatchdogEvent ) ) {
				CErr ( "MediatorDaemon.Dispose: Error to signal watchdog event" );
			}

			watchdog.Join ( "MediatorDaemon.Dispose" );
		}

		// We need to close the socket as the listener is in accept mode
		sock = ( int ) instance.socket;
		if ( sock != -1 ) {
			CVerb ( "Dispose: Closing socket" );
			ShutdownCloseSocket ( sock, false, "MediatorThreadInstance.Dispose" );

			FakeConnect ( sock, instance.port );

			instance.thread.WaitOne ( "MediatorDaemon.Dispose", 500 );
		}

		udp.Join ( "MediatorDaemon.Dispose" );

		sock = ( int ) instance.socket;
		if ( sock != -1 ) {
			instance.socket = -1;

			CVerb ( "Dispose: Closing socket" );
			ShutdownCloseSocket ( sock, true, "MediatorThreadInstance.Dispose" );
		}

		instance.Dispose ();

		Reset ();
	}


	DeviceInstanceNode::~DeviceInstanceNode ()
	{
		clientSP = 0; rootSP = 0;

        TraceDeviceInstanceNodesRemove ( this );
	}


	MediatorDaemon::MediatorDaemon ()
	{
		CVerb ( "Construct" );

		allocated				= false;
		acceptEnabled           = true;
		anonymousLogon          = true;
		*anonymousUser          = 0;
		*anonymousPassword      = 0;

		input					= inputBuffer;

		bannAfterTries      = 3;

		areasCounter        = 0;

		appsCounter         = 0;

		networkOK			= 0xFFFFFFFF;
		networkMask			= 0xFFFFFFFF;
		checkLast			= 0;
		bannedIPLast		= 0;

		usersDBDirty		= false;
		configDirty			= false;
		deviceMappingDirty  = false;

		Zero ( aesKey );
		Zero ( aesCtx );

		privKey				= 0;
		privKeySize			= 0;

#ifdef _WIN32
		hPrivKeyCSP			= 0;
		hPrivKey			= 0;
#endif
		Zero ( notifyThreadID );

		srand ( getRandomValue ( ( void * ) &checkLast ) );

		/// Android -> win daemon
		/// o = OAEP SHA1 MGF1: +openssl, -win32
		/// r = OAEP SHA256 MGF1: -openssl, -win32
		/// p = PKCS1 - openssl: +openssl, -win32
		/// n = no padding: -openssl, -win32

		///
		/// Win -> openssl daemon
		/// o = OAEP SHA1 MGF1: +android, -win32 (data too large for modulus)
		/// r = OAEP SHA256 MGF1: -openssl, -win32
		/// p = PKCS1 - openssl: +openssl, -win32
		/// n = no padding: -openssl, -win32
		encPadding              = ENVIRONS_DEFAULT_CRYPT_PAD;

		stuntID					= 0;

		sessionCounter			= 1;

		reqAuth					= true;
		logToFile					= false;
        
		sendThreadCount			= SEND_THREADS_DEFAULT;
        sendThreadsAlive        = false;
		sendThreads				= 0;
        sessionsChanged         = 0;

#ifdef ENABLE_WINSOCK_SEND_THREADS
		sendEvent				= 0;
#endif
        clientThreadCount       = CLIENT_THREADS_DEFAULT;
        clientThreadsAlive      = false;
        clientThreads           = 0;
        clientContexts          = 0;
        watchdogSessionsChanged = 1;

        notifyTargetsSize       = 0;
        InitEnvironsCrypt ();
	}


	MediatorDaemon::~MediatorDaemon ()
	{
		CLog ( "Destructor" );

		Dispose ();

		notifyTargets.clear ();
		areaIDs.clear ();
		appIDs.clear ();
        
        bannedIPs.clear ();
        bannedIPConnects.clear ();
        connectedIPs.clear ();
        
        while ( !notifyQueue.empty () )
        {
            NotifyQueueContext * ctx = notifyQueue.front ();
            notifyQueue.pop ();
            if ( ctx )
                delete ctx;
        }
        
        if ( clientThreads ) {
            for ( unsigned int i = 0; i < clientThreadCount; ++i )
            {
                if ( clientThreads [ i ] )
                    delete clientThreads [ i ];
                
                if ( clientContexts [ i ] )
                    delete clientContexts [ i ];
            }
            
            free ( clientThreads );
            clientThreads = 0;
            
            free ( clientContexts );
            clientContexts = 0;
        }
        
		if ( sendThreads ) {
			for ( unsigned int i = 0; i < sendThreadCount; ++i ) 
			{
				if ( sendThreads [ i ] )
					delete sendThreads [ i ];
			}
            
            free ( sendThreads );
			sendThreads = 0;
		}
        
        if ( sessionsChanged ) {
            free ( sessionsChanged );
            sessionsChanged = 0;
        }
        
        CloseLog ();

		AESDisposeKeyContext ( &aesCtx );

		ReleaseEnvironsCrypt ();

#ifdef ENABLE_WINSOCK_SEND_THREADS
		CloseWSAHandle_n ( sendEvent );
#endif
		if ( allocated ) {
			LockDisposeA ( usersDBLock );
			LockDisposeA ( bannedIPsLock );
			LockDisposeA ( thread_lock );
			CondDisposeA ( thread_condition );
			LockDisposeA ( notifyLock );
			LockDisposeA ( notifyTargetsLock );
		}
    }
    
    
    void MediatorDaemon::DisposeClass ()
    {
        CVerb ( "DisposeClass" );
        
        if ( logfile.is_open () ) {
            logfile.close ();
        }
        
        LockDisposeA ( logMutex );
    }
    
    
    bool MediatorDaemon::InitMediator ()
	{
		CVerb ( "InitMediator" );

		if ( !allocated ) {
			if ( !LockInitA ( thread_lock ) )
				return false;

			if ( pthread_cond_init ( &thread_condition, NULL ) ) {
				CErr ( "InitMediator: Failed to init thread_condition." );
				return false;
			}

			if ( pthread_cond_init ( &notifyEvent, NULL ) ) {
				CErr ( "InitMediator: Failed to init notifyEvent." );
				return false;
			}

			if ( pthread_cond_init ( &hWatchdogEvent, NULL ) ) {
				CErr ( "InitMediator: Failed to init hWatchdogEvent." );
				return false;
			}

#	ifdef ENABLE_WINSOCK_SEND_THREADS
			CreateWSAHandle ( sendEvent, false );
#	else
			if ( !sendEvent.Init () )
				return false;
			sendEvent.autoreset = false;
#	endif
            
            if ( !clientEvent.Init () )
                return false;
            clientEvent.autoreset = false;

            if ( !areas.Init () )
				return false;
			areas.list.clear ();

			if ( !deviceMappings.Init () )
				return false;
			deviceMappings.list.clear ();

			if ( !stuntClients.Init () )
				return false;
			stuntClients.list.clear ();

			if ( !areasMap.Init () )
				return false;
			areasMap.list.clear ();

			if ( !sessions.Init () )
				return false;
			sessions.list.clear ();

			if ( !acceptClients.Init () )
				return false;
			acceptClients.list.clear ();

			if ( !LockInitA ( bannedIPsLock ) )
				return false;

			if ( !LockInitA ( usersDBLock ) )
				return false;

			if ( !LockInitA ( notifyLock ) )
				return false;

			if ( !LockInitA ( notifyTargetsLock ) )
				return false;

			if ( !LockInitA ( logMutex ) )
				return false;

			allocated = true;

			strlcpy ( anonymousUser, MEDIATOR_ANONYMOUS_USER, sizeof ( anonymousUser ) );

			strlcpy ( anonymousPassword, MEDIATOR_ANONYMOUS_PASSWORD, sizeof ( anonymousPassword ) );
		}

		return true;
	}


	void MediatorDaemon::InitDefaultConfig ()
	{
		CVerb ( "InitDefaultConfig" );

		ports.clear ();
		ports.push_back ( DEFAULT_MEDIATOR_PORT );
	}


	void MediatorDaemon::ReleaseKeys ()
	{
		CVerb ( "ReleaseKeys" );

#ifdef USE_WIN32_CACHED_PRIVKEY
		if ( hPrivKey ) {
			CryptDestroyKey ( hPrivKey );
			hPrivKey = 0;
		}

		if ( hPrivKeyCSP ) {
			CryptReleaseContext ( hPrivKeyCSP, 0 );
			hPrivKeyCSP = 0;
		}
#endif
		if ( privKey ) {
			DisposePrivateKey ( ( void ** ) &privKey );
		}
		privKeySize				= 0;
	}


	bool MediatorDaemon::LoadKeys ()
	{
		CVerb ( "LoadKeys" );

		ReleaseKeys ();

		if ( !environs::LoadPublicCertificate ( "./ssl/mediator.crt", &certificate ) ) {
			CWarn ( "LoadKeys: Failed to load mediator certificate." );
			return false;
		}

		if ( !environs::LoadPrivateKey ( "./ssl/mediator.priv.key", &privKey, &privKeySize ) ) {
			CWarn ( "LoadKeys: Failed to load mediator private key." );
			return false;
		}

#ifdef USE_WIN32_CACHED_PRIVKEY
		if ( !CryptAcquireContext ( &hPrivKeyCSP, NULL, 0, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT ) ) {
			CErr ( "LoadKeys: CryptAcquireContext failed." );
			return false;
		}

		if ( !CryptImportKey ( hPrivKeyCSP, ( BYTE * ) privKey, privKeySize, NULL, 0, &hPrivKey ) ) {
			CErr ( "LoadKeys: CryptImportKey failed." );
			return false;
		}
#endif
		return true;
	}


	bool MediatorDaemon::LoadConfig ()
	{
		CVerb ( "LoadConfig" );

		char * aesCryptKey = 0;

		string line;
		string prefix;
		string svalue;
		ifstream conffile;
		unsigned int value;

		conffile.open ( CONFFILE );

		if ( !conffile.good () )
			goto NoConfFile;

		// Adapt default configuration

		while ( getline ( conffile, line ) )
		{
			const char * str = line.c_str ();
			if ( strlen ( str ) < 3 )
				continue;

			std::istringstream iss ( line );

			if ( str [ 0 ] == 'M' && str [ 1 ] == ':' ) {
				unsigned short port;

				if ( !( iss >> prefix >> value >> port ) ) {
					CLogArg ( "LoadConfig: Invalid config line: %s", str );
				}
				else {
					AddMediator ( value, port );
				}
			}
			else if ( str [ 0 ] == 'N' && str [ 1 ] == ':' ) {
				if ( !( iss >> prefix >> svalue ) ) {
					CLogArg ( "LoadConfig: Invalid config line: %s", str );
				}
				else {
					struct sockaddr_in addr;
					inet_pton ( AF_INET, svalue.c_str (), &( addr.sin_addr ) );
					networkOK = ( unsigned int ) addr.sin_addr.s_addr;

					networkMask = GetBitMask ( networkOK );
				}
			}
			else if ( str [ 0 ] == 'L' && str [ 1 ] == 'S' && str [ 2 ] == ':' ) {
				if ( !( iss >> prefix >> value ) ) {
					CLogArg ( "LoadConfig: Invalid config line: %s", str );
				}
				else {
					stdLog = ( value != 0 );
				}
			}
			else if ( str [ 0 ] == 'L' && str [ 1 ] == 'F' && str [ 2 ] == ':' ) {
				if ( !( iss >> prefix >> value ) ) {
					CLogArg ( "LoadConfig: Invalid config line: %s", str );
				}
				else {
					fileLog = ( value != 0 );
				}
			}
			else if ( str [ 0 ] == 'B' && str [ 1 ] == ':' ) {
				std::time_t dateTime;

				if ( !( iss >> prefix >> value >> dateTime ) ) {
					CLogArg ( "LoadConfig: Invalid config line: %s", str );
				}
				else {
					bannedIPs [ value ] = dateTime;
				}
			}
			else if ( str [ 0 ] == 'C' && str [ 1 ] == 'V' && str [ 2 ] == ':' ) {
				if ( !( iss >> prefix >> value ) ) {
					CLogArg ( "LoadConfig: Invalid config line: %s", str );
				}
				else {
					if ( value < 100 ) {
						bannAfterTries = value;
					}
					else {
						CVerbArg ( "LoadConfig: Invalid try value for bann: %u", value );
					}
				}
			}
			else if ( str [ 0 ] == 'A' && str [ 1 ] == 'N' && str [ 2 ] == ':' ) {
				if ( !( iss >> prefix >> value ) ) {
					CLogArg ( "LoadConfig: Invalid config line: %s", str );
				}
				else {
					anonymousLogon = value ? true : false;
				}
			}
			else if ( str [ 0 ] == 'A' && str [ 1 ] == 'U' && str [ 2 ] == ':' ) {
				if ( !( iss >> prefix >> svalue ) ) {
					CLogArg ( "LoadConfig: Invalid config line: %s", str );
				}
				else {
					strlcpy ( anonymousUser, svalue.c_str (), sizeof ( anonymousUser ) );
				}
			}
			else if ( str [ 0 ] == 'A' && str [ 1 ] == 'P' && str [ 2 ] == ':' ) {
				if ( !( iss >> prefix >> svalue ) ) {
					CLogArg ( "LoadConfig: Invalid config line: %s", str );
				}
				else {
					strlcpy ( anonymousPassword, svalue.c_str (), sizeof ( anonymousPassword ) );
				}
			}
			else if ( str [ 0 ] == 'C' && str [ 1 ] == ':' ) {
				unsigned int tries;

				if ( !( iss >> prefix >> value >> tries ) ) {
					CLogArg ( "LoadConfig: Invalid config line: %s", str );
				}
				else {
					bannedIPConnects [ value ] = tries;
				}
            }
			else if ( str [ 0 ] == 'S' && str [ 1 ] == 'T' && str [ 2 ] == ':' ) {
				if ( !( iss >> prefix >> value ) ) {
					CLogArg ( "LoadConfig: Invalid config line: %s", str );
				}
				else {
					if ( value < 128 && value > 0 ) {
						sendThreadCount = value;
					}
					else {
						CVerbArg ( "LoadConfig: Invalid send thread: %u", value );
					}
				}
			}
            else if ( str [ 0 ] == 'C' && str [ 1 ] == 'T' && str [ 2 ] == ':' ) {
                if ( !( iss >> prefix >> value ) ) {
                    CLogArg ( "LoadConfig: Invalid config line: %s", str );
                }
                else {
                    if ( value < 128 && value > 0 ) {
                        clientThreadCount = value;
                    }
                    else {
                        CVerbArg ( "LoadConfig: Invalid send thread: %u", value );
                    }
                }
            }
            else if ( str [ 0 ] == 'A' && str [ 1 ] == 'E' && str [ 2 ] == 'S' && str [ 3 ] == ':' ) {
				string aesString;

				if ( !( iss >> prefix >> aesString ) ) {
					CLogArg ( "LoadConfig: Invalid config line: %s", str );
				}
				else {
					/// Decrypt the aes key
					unsigned int decLength = ( ( unsigned int ) aesString.length () ) >> 1;

					char * aesBuffer = ( char * ) malloc ( decLength + 4 );
					if ( aesBuffer )
					{
						char * converted = ConvertToByteBuffer ( aesString.c_str (), ( unsigned int ) aesString.length (), aesBuffer );
						if ( converted )
						{
							if ( !DecryptMessage ( privKey, privKeySize, converted, decLength, &aesCryptKey, &decLength ) ) {
								CWarn ( "LoadConfig: Decrypt with privKey failed!" );
							}
							else if ( aesCryptKey ) {
								if ( decLength <= sizeof ( aesKey ) )
									memcpy ( aesKey, aesCryptKey, sizeof ( aesKey ) );

								free ( aesCryptKey );
							}
						}

						free ( aesBuffer );
					}
				}
			}
			else if ( str [ 0 ] == 'P' && str [ 1 ] == 'o' && str [ 2 ] == 'r' && str [ 3 ] == 't' && str [ 4 ] == 's' && str [ 5 ] == ':' ) {
				unsigned short port = DEFAULT_MEDIATOR_PORT;

				if ( !( iss >> prefix >> port ) ) {
					CLogArg ( "LoadConfig: Invalid config line: %s", str );
				}
				else {
					ports.clear ();
					ports.push_back ( port );
				}
			}

		}

		conffile.close ();

	NoConfFile:
		if ( !aesCryptKey ) {
			BUILD_IV_128 ( aesKey );
			BUILD_IV_128 ( aesKey + 16 );

			configDirty = true;
		}

		AESDisposeKeyContext ( &aesCtx );

		if ( !AESDeriveKeyContext ( aesKey, sizeof ( aesKey ), &aesCtx ) ) {
			CWarn ( "LoadConfig: Derive AESContext failed!" );
		}


		BuildBroadcastMessage ();

		return true;
	}


	bool MediatorDaemon::SaveConfig ()
	{
		CVerb ( "SaveConfig" );

		ofstream conffile;
		conffile.open ( CONFFILE );
		if ( !conffile.good () )
			return false;

		stringstream configs;

		configs << "Ports: ";

		// Save 
		if ( ports.size () > 0 )
		{

			for ( unsigned int pos = 0; pos < ports.size (); pos++ ) {
				int port = ports [ pos ];
				configs << port << " ";
			}
		}
		else
			configs << "5899";
		configs << endl;

#ifdef ENABLE_MEDIATOR_LOCK
		if ( LockAcquireA ( mediatorLock, "SaveConfig" ) )
		{
#endif
			MediatorInstance * net = &mediator;

			while ( net && net->ip ) {
				configs << "M: " << net->ip << " " << net->port ;

				configs << endl;
				net = net->next;
			}

#ifdef ENABLE_MEDIATOR_LOCK
			LockReleaseVA ( mediatorLock, "SaveConfig" );
		}
#endif

		if ( networkOK != 0xFFFFFFFF )
		{
			struct sockaddr_in addr;
			char ipStr [ INET_ADDRSTRLEN ];

			addr.sin_addr.s_addr = networkOK;
			inet_ntop ( AF_INET, &( addr.sin_addr ), ipStr, INET_ADDRSTRLEN );

			configs << "N: " << ipStr << endl;
		}

		configs << "CV: " << bannAfterTries << endl;
		configs << "AN: " << ( int ) anonymousLogon << endl;

		if ( !stdLog )
			configs << "LS: 0" << endl;
		if ( !fileLog )
			configs << "LF: 0" << endl;
        
		if ( sendThreadCount != SEND_THREADS_DEFAULT )
			configs << "ST: " << sendThreadCount << endl;
        
        if ( clientThreadCount != CLIENT_THREADS_DEFAULT )
            configs << "CT: " << clientThreadCount << endl;

        if ( strncmp ( anonymousUser, MEDIATOR_ANONYMOUS_USER, MAX_NAMEPROPERTY ) ) {
			configs << "AU: " << anonymousUser << endl;
			configs << "AP: " << anonymousPassword << endl;
		}

		char aesCryptKey [ 514 ];
		memcpy ( aesCryptKey, aesKey, sizeof ( aesKey ) );

		unsigned int length = sizeof ( aesKey );

		if ( !EncryptMessage ( 0, certificate, aesCryptKey, &length ) ) {
			CWarn ( "SaveConfig: Encrypt with certificate failed!" );
		}
		else if ( length >= 256 ) {

			const char * hexStr = ConvertToHexString ( aesCryptKey, length );
			if ( hexStr ) {
				configs << "AES: " << hexStr << " ";

				configs << endl;
			}
		}

		if ( LockAcquireA ( bannedIPsLock, "SaveConfig" ) )
		{
			std::map<unsigned int, std::time_t>::iterator iter;

			for ( iter = bannedIPs.begin (); iter != bannedIPs.end (); ++iter ) {
				configs << "B: " << iter->first << " " << iter->second << endl;
			}

			std::map<unsigned int, unsigned int>::iterator itert;

			for ( itert = bannedIPConnects.begin (); itert != bannedIPConnects.end (); ++itert ) {
				configs << "C: " << itert->first << " " << itert->second << endl;
			}

			LockReleaseVA ( bannedIPsLock, "SaveConfig" );
		}

		conffile << configs.str () << endl;

		conffile.close ();

		configs.clear ();
		return true;
	}


	bool MediatorDaemon::LoadProjectValues ()
	{
		CVerb ( "LoadProjectValues" );

		ifstream conffile;
		conffile.open ( DATAFILE );
		if ( !conffile.good () )
			return false;

		areasMap.Lock ( "LoadProjectValues" );

		sp ( AppsList )		apps;
		sp ( ListValues )   listValues		= 0;

		string line;
		while ( getline ( conffile, line ) )
		{
			const char * str = line.c_str ();
			if ( strlen ( str ) < 3 )
				continue;

			if ( str [ 0 ] == 'P' && str [ 1 ] == ':' ) {
				CVerbArg ( "Loading area [%s]", str );

				apps = make_shared < AppsList > ();
                if ( !apps || !apps->Init () ) {
					CErrArg ( "LoadProjectValues: Failed to create new area [%s].", str + 2 );
					break;
				}
				areasMap.list [ string ( str + 2 ) ] = apps;
				continue;
			}

			if ( str [ 0 ] == 'A' && str [ 1 ] == ':' ) {
				if ( !apps ) {
					CErrArg ( "LoadProjectValues: Missing project for app [%s]!", str + 2 );
					break;
				}
				CVerbArg ( "Loading application [%s]", str );

				listValues = make_shared < ListValues > ();
				if ( !listValues || !listValues->Init () ) {
					CErrArg ( "LoadProjectValues: Failed to create new application [%s].", str + 2 );
					break;
				}
				apps->Lock ( "LoadProjectValues" );

				apps->apps [ string ( str + 2 ) ] = listValues;

				apps->Unlock ( "LoadProjectValues" );
				continue;
			}

			if ( !listValues ) {
				CErr ( "LoadProjectValues: Invalid data file format. Project/App definition for values missing!" );
				break;
			}

			std::istringstream iss ( line );
			string key, value;
			std::time_t timestamp;
			unsigned int size = 0;

			if ( !( iss >> key >> size >> value >> timestamp ) ) {
				CErrArg ( "LoadProjectValues: Failed to read key/value (%s)!", line.c_str () );

				std::istringstream iss1 ( line );
				if ( !( iss1 >> key >> size >> value ) ) {
					CErrArg ( "LoadProjectValues: Failed to read key/value (%s)!", line.c_str () );
					break;
				}

				timestamp = 0;
			}

			sp ( ValuePack ) pack = make_shared < ValuePack > ();
			if ( !pack ) {
				CErrArg ( "LoadProjectValues: Failed to create new value object for %s/%u/%s! Memory low problem!", key.c_str (), size, value.c_str () );
				break;
			}

			pack->timestamp = timestamp;
			pack->value = value;
			pack->size = size;

			listValues->Lock ( "LoadProjectValues" );

			listValues->values [ key ] = pack;

			listValues->Unlock ( "LoadProjectValues" );
		}

		/// Sanitinize database
		msp ( string, AppsList )::iterator it = areasMap.list.begin ();

		while ( it != areasMap.list.end () )
		{
			sp ( AppsList ) appList = it->second;

			if ( !appList ) {
				areasMap.list.erase ( it++ );
				continue;
			}

			msp ( string, ListValues )::iterator ita = appList->apps.begin ();
			while ( ita != appList->apps.end () )
			{
				if ( !ita->second ) {
					appList->apps.erase ( ita++ );
					continue;
				}

				if ( !ita->second->values.size () ) {
					appList->apps.erase ( ita++ );
					continue;
				}
				++ita;
			}

			if ( !appList->apps.size () ) {
				areasMap.list.erase ( it++ );
				continue;
			}

			++it;
		}

		areasMap.Unlock ( "LoadProjectValues" );

		conffile.close ();

		return true;
	}


	bool MediatorDaemon::SaveProjectValues ()
	{
		bool success = true;

		CLog ( "SaveProjectValues" );

		ofstream conffile;
		conffile.open ( DATAFILE );
		if ( !conffile.good () )
            return false;
        
        DEBUG_CHECK_START ();

		if ( !areasMap.Lock ( "SaveProjectValues" ) ) {
			success = false;
		}
		else {
			// Collect areas
			msp ( string, AppsList ) tmpAreas;
			for ( msp ( string, AppsList )::iterator it = areasMap.list.begin (); it != areasMap.list.end (); ++it )
			{
				sp ( AppsList ) appsList = it->second;
				if ( !appsList )
					continue;

				tmpAreas [ it->first ] = appsList;
			}

			if ( !areasMap.Unlock ( "SaveProjectValues" ) ) {
				success = false;
			}

			// Save areas
			for ( msp ( string, AppsList )::iterator it = tmpAreas.begin (); it != tmpAreas.end (); ++it )
			{
				sp ( AppsList ) appsList = it->second;
				if ( !appsList )
					continue;

				sp ( AreaApps ) areaApps;

                areas.LockRead ( "SaveProjectValues" );

				const msp ( string, AreaApps )::iterator areaIt = areas.list.find ( it->first );
				if ( areaIt != areas.list.end () )
					areaApps = areaIt->second;

                areas.UnlockRead ( "SaveProjectValues" );

				conffile << "P:" << it->first << endl;

				if ( !appsList->Lock ( "SaveProjectValues" ) )
					continue;

				msp ( string, ListValues ) tmpListValues;

				for ( msp ( string, ListValues )::iterator ita = appsList->apps.begin (); ita != appsList->apps.end (); ita++ )
				{
					sp ( ListValues ) listValues = ita->second;
					if ( !listValues )
						continue;

					tmpListValues [ ita->first ] = listValues;
				}

				appsList->Unlock ( "SaveProjectValues" );


				for ( msp ( string, ListValues )::iterator ita = tmpListValues.begin (); ita != tmpListValues.end (); ita++ )
				{
					sp ( ListValues ) listValues = ita->second;
					if ( !listValues || !listValues->Lock ( "SaveProjectValues" ) )
						continue;

					unsigned int maxID = 0;

                    /// Find the appList
                    if ( areaApps && areaApps->LockRead ( "SaveProjectValues" ) )
					{
						const msp ( string, ApplicationDevices )::iterator appsIt = areaApps->apps.find ( ita->first );
						if ( appsIt != areaApps->apps.end () )
						{
							const sp ( ApplicationDevices ) & appDevices = appsIt->second;
							if ( appDevices )
								maxID = ( unsigned int ) appDevices->latestAssignedID;
                        }
                        areaApps->UnlockRead ( "SaveProjectValues" );
					}

					conffile << "A:" << ita->first << endl;

					if ( maxID > 0 )
						conffile << "0_maxID " << maxID << " ids" << endl;

					for ( msp ( string, ValuePack )::iterator itv = listValues->values.begin (); itv != listValues->values.end (); ++itv )
					{
						sp ( ValuePack ) value = itv->second;
						if ( value ) {
							if ( maxID > 0 && itv->first.c_str () [ 0 ] == '0' )
								continue;
							conffile << itv->first << " " << value->size << " " << value->value << " " << value->timestamp << endl;
						}
					}

					listValues->Unlock ( "SaveProjectValues" );
				}
			}
        }
        
        DEBUG_CHECK_MEASURE ( "SaveProjectValues" );
        
		conffile.close ();
		return success;
	}


	bool MediatorDaemon::LoadDeviceMappingsEnc ()
	{
		CVerb ( "LoadDeviceMappingsEnc" );

		ifstream conffile;
		conffile.open ( DEVMAPFILE );
		if ( !conffile.good () )
			return false;

		LoadDeviceMappings ( conffile );

		conffile.close ();

		return true;
	}


	bool MediatorDaemon::LoadDeviceMappings ( istream& instream )
	{
		if ( !LockAcquireA ( usersDBLock, "LoadDeviceMappings" ) )
			return false;

		string line;
		while ( getline ( instream, line ) )
		{
			std::istringstream iss ( line );
			string deviceUID;
			int deviceID;
			int authLevel = 0;
			string authToken;

			if ( !( iss >> deviceUID >> deviceID >> authLevel >> authToken ) ) {
				CErrArg ( "LoadDeviceMappings: Failed to read device mapping (%s)!", line.c_str () );
				continue;
			}

			sp ( DeviceMapping ) mapping;
			mapping.reset ( new DeviceMapping );
			//= make_shared < DeviceMapping > ; // sp ( DeviceMapping ) ( new DeviceMapping ); // calloc ( 1, sizeof(DeviceMapping) );
			if ( !mapping )
				break;
			memset ( mapping.get (), 0, sizeof ( DeviceMapping ) );

			mapping->deviceID = deviceID;
			mapping->authLevel = authLevel;
			if ( authToken.length () > 0 )
				memcpy ( mapping->authToken, authToken.c_str (), authToken.length () );

			if ( deviceMappings.Lock ( "LoadDeviceMappings" ) ) {
				deviceMappings.list [ deviceUID ] = mapping;

				deviceMappings.Unlock ( "LoadDeviceMappings" );
			}
		}

		if ( !LockReleaseA ( usersDBLock, "LoadDeviceMappings" ) )
			return false;

		return true;
	}


	bool MediatorDaemon::LoadDeviceMappings ()
	{
		CVerb ( "LoadDeviceMappings" );

		unsigned int size = 0;
		char * decrypted = 0;
		char * ciphers = LoadBinary ( DEVMAPFILE, ( int * ) &size );

		if ( !ciphers )
			return false;

		while ( size ) {
			if ( !AESDecrypt ( &aesCtx, ciphers, &size, &decrypted ) || !size || !decrypted )
				break;

			istringstream text ( ( string ( decrypted ) ) );

			LoadDeviceMappings ( text );

			break;
		}

		if ( decrypted )
			free ( decrypted );
		free ( ciphers );
		return true;
	}


	bool MediatorDaemon::SaveDeviceMappings ()
	{
		bool success = true;

		CVerb ( "SaveDeviceMappings" );

        stringstream plainstream;
        
        DEBUG_CHECK_START ();
        
        msp ( string, DeviceMapping ) cache;
        
		if ( !deviceMappings.Lock ( "SaveDeviceMappings" ) ) {
			success = false;
		}
		else {
            cache = deviceMappings.list;

			if ( !deviceMappings.Unlock ( "SaveDeviceMappings" ) ) {
				success = false;
			}
        }
        
        // Build devcie mappings
        msp ( string, DeviceMapping )::iterator end = cache.end ();
        
        for ( msp ( string, DeviceMapping )::iterator it = cache.begin (); it != end; ++it )
        {
            DeviceMapping * item = it->second.get ();
            if ( item ) {
                plainstream << it->first << " " << item->deviceID << " " << item->authLevel << " " << item->authToken << endl;
            }
        }
        
        DEBUG_CHECK_MEASURE ( "SaveDeviceMappings" );
        
		if ( success ) {
			/// Encrypt plaintext
			string text = plainstream.str ();

			char * cipher = 0;
			unsigned int bufferLen = ( unsigned int ) text.length ();

			if ( bufferLen ) {
				success = false;

				if ( AESEncrypt ( &aesCtx, ( char * ) text.c_str (), &bufferLen, &cipher ) && cipher ) {
					if ( SavePrivateBinary ( DEVMAPFILE, cipher, bufferLen ) ) {
						success = true;
					}
					else CErr ( "SaveDeviceMappings: Failed to save into file." );

					free ( cipher );
				}
				else CErr ( "SaveDeviceMappings: AESEncrypt failed." );
			}
			else {
				unlink ( DEVMAPFILE );
			}
		}

		return success;
	}


	bool MediatorDaemon::AddUser ( int authLevel, const char * userName, const char * pass )
	{
		CLog ( "AddUser" );

		bool success = false;

		char        *   hash    = 0;
		unsigned int    len     = 0;
		UserItem    *   item    = 0;
        size_t          passLen = strlen ( pass );

		do
		{
            if ( passLen ) {
                item = new UserItem ();
                if ( !item )
                    break;

                success = SHAHashPassword ( pass, &hash, &len );
                if ( !success ) {
                    CErrArg ( "AddUser: Failed to create pass hash for [ %s ]", userName ); break;
                }
                success = false;

                if ( !hash ) {
                    CErrArg ( "AddUser: Invalid null pass hash for [ %s ]", userName ); break;
                }
            }

			string user = userName;
			std::transform ( user.begin (), user.end (), user.begin (), ::tolower );

            if ( !LockAcquireA ( usersDBLock, "AddUser" ) )
                break;

			const map<string, UserItem *>::iterator iter = usersDB.find ( user );
			if ( iter != usersDB.end () ) {
                if ( passLen )
                {
                    CLogArg ( "AddUser: Updating password of user [ %s ]", userName );
                    printf ( "\nAddUser: Updating password of user [ %s ]\n", userName );
                }
                else {
                    CLogArg ( "AddUser: Removing user [ %s ]", userName );
                    printf ( "\nAddUser: Removing user [ %s ]\n", userName );

                    usersDB.erase ( iter );
                }
			}

            if ( passLen ) {
                item->authLevel = authLevel;
                item->pass      = string ( hash );

                usersDB [ user ] = item;
            }

            if ( !LockReleaseA ( usersDBLock, "AddUser" ) )
                break;

            item    = 0;
            success = true;
		}
		while ( 0 );

        free_n ( hash );

		if ( item )
			delete item;

        if ( success )
            usersDBDirty = true;
		return success;
	}


	/// For now we don't encrypt the users.db. For productive use, this should be encrypted before storing
	bool MediatorDaemon::LoadUserDBEnc ()
	{
		CVerb ( "LoadUserDB" );

		ifstream usersfile;
		usersfile.open ( USERSDBFILE );
		if ( !usersfile.good () )
			return false;

		LoadUserDB ( usersfile );

		usersfile.close ();

		return true;
	}


	bool MediatorDaemon::LoadUserDB ( istream& instream )
	{
		string userName;
		string userPassword;
		string line;
		string pre1, pre2;
		int authLevel = 0;

		while ( getline ( instream, line ) )
		{
			std::istringstream iss ( line );

			if ( !( iss >> userName >> pre1 >> authLevel >> pre2 >> userPassword ) ) {
				CErrArg ( "LoadUserDB: Failed to read user entry (%s)!", line.c_str () );
				continue;
			}

			if ( userName.find ( '@', 0 ) == string::npos )
				continue;

			if ( userPassword.length () < 64 ) {
				continue;
			}

			if ( authLevel < 0 || authLevel > 10 )
				authLevel = 3;

			const char * pwStr = ConvertToBytes ( userPassword.c_str (), ( unsigned int ) userPassword.length () );
			if ( pwStr ) {

				UserItem * item = new UserItem ();
				if ( item ) {
					string pw = pwStr;
					item->pass = pw;
					item->authLevel = authLevel;

					usersDB [ userName ] = item;
				}
			}
		}
		return true;
	}


	bool MediatorDaemon::LoadUserDB ()
	{
		CVerb ( "LoadUserDB" );

		unsigned int size = 0;
		char * decrypted = 0;
		char * ciphers = LoadBinary ( USERSDBFILE, ( int * ) &size );

		if ( !ciphers )
			return false;

		while ( size ) {
			if ( !AESDecrypt ( &aesCtx, ciphers, &size, &decrypted ) || !size || !decrypted )
				break;

			istringstream text ( ( string ( decrypted ) ) );

			LoadUserDB ( text );
			break;
		}

		if ( decrypted )
			free ( decrypted );
		free ( ciphers );
		return true;
	}


	bool MediatorDaemon::SaveUserDB ()
	{
		bool ret = true;

		CLog ( "SaveUserDB" );

        stringstream plainstream;
        
        DEBUG_CHECK_START ();
        
		if ( !LockAcquireA ( usersDBLock, "SaveUserDB" ) )
			ret = false;
		else {
			// Save areas
			for ( map<string, UserItem *>::iterator it = usersDB.begin (); it != usersDB.end (); ++it )
			{
				//CVerbArg ( "SaveUserDB: try saving [%s]", it->first.c_str() );

				if ( it->second )
				{
					UserItem * item = it->second;

					const char * pwStr = ConvertToHexString ( item->pass.c_str (), ENVIRONS_USER_PASSWORD_LENGTH );
					if ( pwStr ) {
						//CVerbArg ( "SaveUserDB: Saving pass [%s]", pwStr );

						plainstream << it->first << " :: " << item->authLevel << " :: " << pwStr << endl;
					}
				}
			}

			if ( !LockReleaseA ( usersDBLock, "SaveUserDB" ) )
                ret = false;
            
            DEBUG_CHECK_MEASURE ( "SaveUserDB" );
        }

		if ( ret ) {
			/// Encrypt plaintext
			string text = plainstream.str ();

			char * cipher = 0;
			unsigned int bufferLen = ( unsigned int ) text.length ();

			if ( bufferLen ) {
				ret = false;

				if ( AESEncrypt ( &aesCtx, ( char * ) text.c_str (), &bufferLen, &cipher ) && cipher ) {
					if ( SavePrivateBinary ( USERSDBFILE, cipher, bufferLen ) ) {
						ret = true;
					}
					else CErr ( "SaveUserDB: Failed to save into file." );

					free ( cipher );
				}
				else CErr ( "SaveUserDB: AESEncrypt failed." );
			}
		}

		return ret;
	}


	bool MediatorDaemon::IsIpBanned ( unsigned int ip )
	{
		CVerb ( "IsIpBanned" );

		bool banned = false;

		if ( networkOK == 0xFFFFFFFF )
			return false;

		if ( ( ip & networkMask ) == networkOK ) {
			CVerb ( "IsIpBanned: IP is OK!" );
			return false;
		}

		LockAcquireVA ( bannedIPsLock, "IsIpBanned" );

		const std::map<unsigned int, std::time_t>::iterator iter = bannedIPs.find ( ip );
		if ( iter != bannedIPs.end () )
		{
			CLogArg ( "IsIpBanned: A banned IP [%s] tries to connect again.!", inet_ntoa ( *( ( struct in_addr * ) &ip ) ) );

			std::time_t bannedDate = iter->second;
			std::time_t now = std::time ( 0 );

			if ( now - bannedDate < 259200 ) {
				CVerb ( "IsIpBanned: The bann has been set within the last 3 days. Refreshing and refusing connect!" );
				bannedIPs [ ip ] = now;

				banned = true;
				goto Finish;
			}

			CLogArg ( "IsIpBanned: The bann was more than %d seconds ago. Let the request try again!", now - bannedDate );
			bannedIPs.erase ( iter );

			const std::map<unsigned int, unsigned int>::iterator itert = bannedIPConnects.find ( ip );
			if ( itert != bannedIPConnects.end () )
				bannedIPConnects.erase ( itert );
		}

	Finish:
		LockReleaseVA ( bannedIPsLock, "IsIpBanned" );

		return banned;
	}


	bool MediatorDaemon::BannIP ( unsigned int ip )
	{
		CVerbArg ( "BannIP: [ %s ]", inet_ntoa ( *( ( struct in_addr * ) &ip ) ) );

		if ( networkOK == 0xFFFFFFFF )
			return true;

		if ( ( ip & networkMask ) == networkOK || ip == bannedIPLast ) {
			CVerb ( "IsIpBanned: IP is OK!" );
			return false;
		}

        bool banned = false;
        
		if ( !LockAcquireA ( bannedIPsLock, "BannIP" ) )
			return false;

        std::map<unsigned int, unsigned int>::iterator it = connectedIPs.find ( ip );
        if ( it == connectedIPs.end () )
        {
            unsigned int tries = bannedIPConnects [ ip ];
            if ( tries < bannAfterTries ) {
                bannedIPConnects [ ip ] = ++tries;
                CVerbVerbArg ( "BannIP: Retain bann due to allowed tries [ %s ] [ %u : %u ]", inet_ntoa ( *( ( struct in_addr * ) &ip ) ), tries, bannAfterTries );
            }
            else {
				bannedIPLast = ip;

                std::time_t t = std::time ( 0 );
                
                bannedIPs [ ip ] = t;
                
                if ( printStdOut ) {
                    printf ( "Banned ip : [ %s ]\n", inet_ntoa ( *( ( struct in_addr * ) &ip ) ) );
                }
                banned = true;
            }
            configDirty = true;
        }

		LockReleaseVA ( bannedIPsLock, "IsIpBanned" );

		return banned;
	}


	void MediatorDaemon::BannIPRemove ( unsigned int ip )
	{
		CVerb ( "BannIPRemove" );

		bool ret = false;

		if ( !LockAcquireA ( bannedIPsLock, "BannIPRemove" ) )
			return;

		const std::map<unsigned int, std::time_t>::iterator iter = bannedIPs.find ( ip );
		if ( iter != bannedIPs.end () )
		{
			CVerbArg ( "BannIPRemove: The IP [ %s ] has been banned before", inet_ntoa ( *( ( struct in_addr * ) &ip ) ) );
			ret = true;

			bannedIPs.erase ( iter );
        }
        
        const std::map<unsigned int, unsigned int>::iterator itert = bannedIPConnects.find ( ip );
        if ( itert != bannedIPConnects.end () ) {
            bannedIPConnects.erase ( itert );
            
            if ( printStdOut ) {
                printf ( "Removed from banned list ip : [ %s ]\n", inet_ntoa ( *( ( struct in_addr * ) &ip ) ) );
            }
        }
        
        LockReleaseVA ( bannedIPsLock, "IsIpBanned" );

		if ( ret ) {
			configDirty = true;
		}
	}


	bool MediatorDaemon::CreateThreads ()
	{
		CVerb ( "CreateThreads" );

		int s, value, ret;

		ReleaseThreads ();

		isRunning = true;
        
        StopSendThreads ();
        
        StartSendThreads ();
        
        StopClientThreads ();
        
        StartClientThreads ();
        
		for ( unsigned int pos = 0; pos < ports.size (); pos++ )
		{
			sp ( MediatorThreadInstance ) listenerSP = make_shared < MediatorThreadInstance > (); //sp ( MediatorThreadInstance ) ( new MediatorThreadInstance ); // make_shared < MediatorThreadInstance > ();

			if ( !listenerSP ) {
				CErrArg ( "CreateThreads: Error - Failed to allocate memory for new Listener [ Nr. %i ]", pos );
				return false;
			}
			MediatorThreadInstance * listener = listenerSP.get ();

			//memset ( listener, 0, sizeof(MediatorThreadInstance) );

			if ( !listenerSP->Init () ) {
				CErrArg ( "CreateThreads: Error - Failed to init new Listener [ Nr. %i ]", pos );
				return false;
			}

			listener->instance.daemon = this;

			ThreadInstance * inst = &listener->instance;

			inst->port = ports [ pos ];

#ifdef _WIN32__
			int sock = ( int ) WSASocket ( AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED );
#else
			int sock = ( int ) socket ( AF_INET, SOCK_STREAM, IPPROTO_TCP );
#endif
			if ( sock < 0 ) {
				CErrArg ( "CreateThreads: Failed to create socket for listener port [  %i ].", inst->port );
				return false;
            }
            CSocketTraceAdd ( sock, "CreateThreads socket" );
			inst->socket = sock;

			struct linger so_linger;
			Zero ( so_linger );

			so_linger.l_onoff   = 1;
			so_linger.l_linger  = 4;

			ret = setsockopt ( sock, SOL_SOCKET, SO_LINGER, ( const char * ) &so_linger, sizeof ( so_linger ) );
			if ( ret < 0 ) {
				CErr ( "CreateThreads: Failed to set SO_LINGER on listener socket." ); LogSocketError ();
				return false;
			}

#ifdef _WIN32
			value = 1;
			ret = setsockopt ( sock, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, ( const char * ) &value, sizeof ( value ) );
			if ( ret < 0 ) {
				CErr ( "CreateThreads: Failed to set reuseAddr on listener socket." ); LogSocketError ();
				return false;
			}
#else
			value = 0;
			ret = setsockopt ( sock, SOL_SOCKET, SO_REUSEADDR, ( const char * ) &value, sizeof ( value ) );
			if ( ret < 0 ) {
				CErr ( "CreateThreads: Failed to set reuseAddr on listener socket." ); LogSocketError ();
				return false;
			}

#ifdef SO_REUSEPORT
			value = 0;
			ret = setsockopt ( sock, SOL_SOCKET, SO_REUSEPORT, ( const char * ) &value, sizeof ( value ) );
			if ( ret < 0 ) {
				CErr ( "CreateThreads: Failed to set reuseAddr on listener socket." ); LogSocketError ();
				return false;
			}
#endif
#endif
			/*value = 1;
			ret = setsockopt ( sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&value, sizeof(value) );
			if ( ret < 0 ) {
			CErr ( "CreateThreads: Failed to set reuseAddr on listener socket." ); LogSocketError ();
			return false;
			}*/

			sock = ( int ) socket ( PF_INET, SOCK_DGRAM, 0 );
			if ( sock < 0 ) {
				CErrArg ( "CreateThreads: Failed to create udp socket for listener port (%i)!", inst->port );
				return false;
            }
            CSocketTraceAdd ( sock, "CreateThreads socketUdp" );
			listener->socketUdp = sock;

			value = 1;
			ret = setsockopt ( sock, SOL_SOCKET, SO_REUSEADDR, ( const char * ) &value, sizeof ( value ) );
			if ( ret < 0 ) {
				CErr ( "CreateThreads: Failed to set reuseAddr on listener socket." ); LogSocketError ();
				return false;
			}

#ifdef SO_REUSEPORT // Fix for bind error on iOS simulator due to crash of app, leaving the port still bound
			value = 1;
			if ( setsockopt ( sock, SOL_SOCKET, SO_REUSEPORT, ( const char * ) &value, sizeof ( value ) ) < 0 ) {
				CErr ( "CreateThreads: Failed to set reuseport option on udp listener socket!" );
				return false;
			}
#endif
			acceptEnabled = true;

			if ( !inst->thread.Run ( pthread_make_routine ( &MediatorDaemon::AcceptorStarter ), ( void * ) inst, "Mediator.CreateThreads", true ) )
			{
				CErrArg ( "CreateThreads: Error creating acceptor thread for port [ %i ]", inst->port );
				return false;
			}


			if ( !listener->udp.Run ( pthread_make_routine ( &MediatorDaemon::MediatorUdpThreadStarter ), ( void * ) listener, "Mediator.CreateThreads", true ) )
			{
				CErrArg ( "CreateThreads: Error creating udp thread for port [ %i ]", inst->port );
				return false;
			}


			if ( !listener->watchdog.Run ( pthread_make_routine ( &MediatorDaemon::WatchdogThreadStarter ), ( void * ) listener, "Mediator.CreateThreads", true ) )
			{
				CErrArg ( "CreateThreads: Error creating watchdog thread for port [ %i ]", inst->port );
				return false;
			}

			listeners.push_back ( listenerSP );
		}

		s = pthread_create ( &notifyThreadID, 0, &MediatorDaemon::NotifyClientsStarter, this );
		if ( s != 0 ) {
			CErrArg ( "CreateThreads: Error creating notifier thread (pthread_create:%i)", s );
		}

		Start ();

		return true;
	}


	void MediatorDaemon::Dispose ()
	{
		CLog ( "Dispose" );

		isRunning = false;

		// Wait for each thread to terminate
        ReleaseThreads ();        

		vsp ( ThreadInstance ) tmpClients;
		vsp ( ThreadInstance ) tmpClients1;

		// There should be no active sessions here.
		// However, in failure cases (e.g. query/lock errors), this may happen.
		// So, we check that and release lingering client sessions
		sessions.Lock ( "Dispose" );
		sessions.Lock1 ( "Dispose" );

		tmpClients = sessions.cache;
		tmpClients1 = sessions.toRemove;
		
		sessions.list.clear ();
		sessions.toRemove.clear ();
		sessions.cache.clear ();

		sessions.Unlock1 ( "Dispose" );
		sessions.Unlock ( "Dispose" );

		vsp ( ThreadInstance )::iterator itv = tmpClients.begin ();

		while ( itv != tmpClients.end () )
		{
			const sp ( ThreadInstance )	&clientSP = *itv;

			if ( clientSP ) {
				ReleaseClient ( clientSP.get () );
			}

			++itv;
		}

		tmpClients.clear ();

		itv = tmpClients1.begin ();

		while ( itv != tmpClients1.end () )
		{
			const sp ( ThreadInstance )	&clientSP = *itv;

			if ( clientSP ) {
				ClientRemove ( clientSP.get () );
			}

			++itv;
		}

		tmpClients.clear ();


		ReleaseDevices ();

		areasMap.Lock ( "Dispose" );

		for ( msp ( string, AppsList )::iterator it = areasMap.list.begin (); it != areasMap.list.end (); ++it )
		{
			sp ( AppsList ) appsList = it->second;
			if ( !appsList )
				continue;

			appsList->Lock ( "Dispose" );

			for ( msp ( string, ListValues )::iterator ita = appsList->apps.begin (); ita != appsList->apps.end (); ++ita )
			{
				sp ( ListValues ) listValues = ita->second;
				if ( !listValues )
					continue;

				listValues->Lock ( "Dispose" );

				listValues->values.clear ();

				listValues->Unlock ( "Dispose" );

				/*for ( msp ( string, ValuePack )::iterator itv = listValues->values.begin(); itv != listValues->values.end(); ++itv )
				{
				if ( itv->second ) {
				itv->second = 0;
				}
				}*/
				ita->second = 0;
			}

			appsList->apps.clear ();
			it->second = 0;

			appsList->Unlock ( "Dispose" );
		}
		areasMap.list.clear ();

		areasMap.Unlock ( "Dispose" );

		ReleaseDeviceMappings ();


		for ( map<string, UserItem *>::iterator it = usersDB.begin (); it != usersDB.end (); ++it )
		{
			if ( !it->second ) {
				continue;
			}
			delete ( it->second );
		}
		usersDB.clear ();
	}


	void MediatorDaemon::ReleaseClient ( ThreadInstance * client )
	{
		CVerb ( "ReleaseClient" );

		if ( !client )
			return;

        client->SendTcpFin ();

        client->Lock ( "ReleaseClient" );

        CloseThreadSocket ( &client->socketToClose );

        client->Unlock ( "ReleaseClient" );
        
        //DisposeSendContexts ( client );

		if ( !isRunning )
		{
			client->deviceSP = 0;

			client->thread.Join ( "MediatorDaemon.ReleaseClient" );

			client->daemon = 0;
		}
	}


	bool MediatorDaemon::ReleaseThreads ()
	{
		CVerb ( "ReleaseThreads" );

		int s;
		bool ret = true;

#if !defined(_WIN32) || defined(USE_PTHREADS_FOR_WINDOWS)
		void * res;
#endif

		// Signal stop status to threads
		isRunning       = false;
		acceptEnabled   = false;
        
        StopSendThreads ();
        
        StopClientThreads ();

        Mediator::ReleaseThreads ( true );

		// Release notifier thread
		pthread_t notifThreadID = notifyThreadID;

		if ( pthread_valid ( notifThreadID ) ) {
			Zero ( notifyThreadID );

			if ( pthread_cond_signal ( &notifyEvent ) ) {
				CErr ( "ReleaseThreads: Error to signal notifier event" );
			}

			CVerb ( "ReleaseThreads: Waiting for notifier thread" );

			s = pthread_join ( notifThreadID, &res );
			if ( s != 0 ) {
				CErrArg ( "ReleaseThreads: Error waiting for notifier thread (pthread_join:%i)", s );
				ret = false;
			}
			pthread_close ( notifThreadID );
		}

		while ( !notifyQueue.empty () )
		{
			NotifyQueueContext * ctx = notifyQueue.front ();
			notifyQueue.pop ();
			if ( ctx )
				delete ctx;
		}

		acceptClients.Lock ( "ReleaseThreads" );

		msp ( void *, ThreadInstance )::iterator it = acceptClients.list.begin ();

		while ( it != acceptClients.list.end () )
		{
			const sp ( ThreadInstance ) & constSP = it->second;

			if ( constSP ) {
				ShutdownCloseSocket ( ( int ) constSP->socketToClose, false, "ReleaseThreads" );
			}
			++it;
		}

		while ( acceptClients.list.size () > 0 )
		{
            sp ( ThreadInstance ) first = acceptClients.list.begin ()->second;

			acceptClients.list.erase ( acceptClients.list.begin () );

            first->inAcceptorList = false;
            
			acceptClients.Unlock ( "ReleaseThreads" );

			ReleaseClient ( first.get () );

			acceptClients.Lock ( "ReleaseThreads" );
		}

		acceptClients.Unlock ( "ReleaseThreads" );

		listeners.clear ();

		return ret;
	}


	void MediatorDaemon::ReleaseDevices ()
	{
		CLog ( "ReleaseDevices" );

		vsp ( AreaApps ) tmpAreas;

        areas.LockWrite ( "ReleaseDevices" );

		for ( msp ( string, AreaApps )::iterator it = areas.list.begin (); it != areas.list.end (); ++it )
		{
			if ( it->second )
				tmpAreas.push_back ( it->second );
		}

		areas.list.clear ();

        areas.UnlockWrite ( "ReleaseDevices" );

		for ( vsp ( AreaApps )::iterator it = tmpAreas.begin (); it != tmpAreas.end (); ++it )
		{
			sp ( AreaApps ) areaApps = *it;
			if ( !areaApps )
				continue;

            areaApps->LockWrite ( "ReleaseDevices" );

			vsp ( ApplicationDevices ) tmpAppDevices;

			for ( msp ( string, ApplicationDevices )::iterator ita = areaApps->apps.begin (); ita != areaApps->apps.end (); ++ita )
			{
				sp ( ApplicationDevices ) appDevices = ita->second;
				if ( ita->second )
					tmpAppDevices.push_back ( appDevices );
			}

            areaApps->apps.clear ();

            areaApps->UnlockWrite ( "ReleaseDevices" );

			for ( vsp ( ApplicationDevices )::iterator ita = tmpAppDevices.begin (); ita != tmpAppDevices.end (); ++ita )
			{
				sp ( ApplicationDevices ) appDevices = *ita;
				if ( appDevices )
                {
                    appDevices->LockWrite ( "ReleaseDevices" );

					vsp ( DeviceInstanceNode ) devSPs;

					DeviceInstanceNode * device = appDevices->devices;

					while ( device ) {
						if ( device->baseSP )
							devSPs.push_back ( device->baseSP );

						//DeviceInstanceNode * toDispose = device;
						device = device->next;
					}

					appDevices->devices = 0;

                    appDevices->UnlockWrite ( "ReleaseDevices" );

					for ( vsp ( DeviceInstanceNode )::iterator itd = devSPs.begin (); itd != devSPs.end (); ++itd )
					{
						sp ( DeviceInstanceNode ) devSP = *itd;

						CVerbArg ( "[ 0x%X ].ReleaseDevices: deleting memory occupied by client", devSP->info.deviceID );

						ReleaseClient ( devSP->clientSP.get () );

						devSP->baseSP = 0;
					}

					devSPs.clear ();
				}
			}

			tmpAppDevices.clear ();
		}

		tmpAreas.clear ();
	}


	void MediatorDaemon::ReleaseDeviceMappings ()
	{
		CVerb ( "ReleaseDeviceMappings" );

		deviceMappings.Lock ( "ReleaseDeviceMappings" );

		for ( msp ( string, DeviceMapping )::iterator it = deviceMappings.list.begin (); it != deviceMappings.list.end (); ++it )
		{
			if ( !it->second ) {
				continue;
			}
			it->second = 0;
		}
		deviceMappings.list.clear ();

		deviceMappings.Unlock ( "ReleaseDeviceMappings" );
	}


	void MediatorDaemon::RemoveDevice ( int deviceID, const char * areaName, const char * appName )
	{
		CVerbID ( "RemoveDevice from maps" );

		if ( !areaName || !appName ) {
			CErrID ( "RemoveDevice: Called with NULL argument for areaName or appName?!" );
			return;
		}

		bool					found	= false;
		DeviceInstanceNode	*	device	= 0;

		string appsName ( appName );
		sp ( ApplicationDevices )   appDevices;

		string pareaName ( areaName );
		sp ( AreaApps )             areaApps;

        if ( !areas.LockRead ( "RemoveDevice" ) )
			return;

		const msp ( string, AreaApps )::iterator areaIt = areas.list.find ( pareaName );
		if ( areaIt != areas.list.end () )
			areaApps = areaIt->second;

        if ( !areas.UnlockRead ( "RemoveDevice" ) )
			return;

        if ( !areaApps || !areaApps->LockRead ( "RemoveDevice" ) )
        {
			CWarnArgID ( "RemoveDevice: areaName [%s] not found.", areaName );
			return;
		}

		const msp ( string, ApplicationDevices )::iterator appsIt = areaApps->apps.find ( appsName );
		if ( appsIt != areaApps->apps.end () )
			appDevices = appsIt->second;

        areaApps->UnlockRead ( "RemoveDevice" );
        
        ApplicationDevices * appDevs = appDevices.get ();

        if ( !appDevs || !appDevs->LockWrite ( "RemoveDevice" ) )
        {
			CErrArg ( "RemoveDevice: appName [%s] not found.", appName );
			return;
		}

		device = appDevs->devices;

		while ( device )
		{
			if ( device->info.deviceID == deviceID ) {
				found = true;
				break;
			}

			if ( !device->next )
				break;
			device = device->next;
		}

        if ( found && device ) {
            RemoveDeviceUnlock ( appDevs, device );
            return;
        }

        appDevs->UnlockWrite ( "RemoveDevice" );
    }
    
    
    void MediatorDaemon::RemoveDeviceUnlock ( ApplicationDevices * appDevs, DeviceInstanceNode * device )
    {
        CVerbVerb ( "RemoveDeviceUnlock" );
        
        if ( device == appDevs->devices ) {
            if ( device->next ) {
                CVerbArg ( "RemoveDeviceUnlock: relocating client [0x%X] to root of list", device->next->info.deviceID );
                
                device->prev = 0;
                appDevs->devices = device->next;
                
                device->next = 0;
                appDevs->count--;
            }
            else {
                appDevs->devices = 0;
                appDevs->count = 0;
            }
        }
        else {
            if ( !device->prev ) {
                CErrArg ( "RemoveDeviceUnlock: Serious inconsistency error!!! Failed to lookup device list. Missing previous device for ID [0x%X]", device->info.deviceID );
                
                /// Best thing we can do is to assume that we are the root node of the list
                if ( device->next ) {
                    CVerbArg ( "RemoveDeviceUnlock: Relocating client [0x%X] to root of list", device->next->info.deviceID );
                    
                    device->next->prev = 0;
                    appDevs->devices = device->next;
                    appDevs->count--;
                }
                else {
                    appDevs->devices = 0;
                    appDevs->count = 0;
                }
            }
            else {
                if ( device->next ) {
                    CVerbArg ( "RemoveDeviceUnlock: Relinking client [0x%X] to previous client [0x%X]", device->next->info.deviceID, device->prev->info.deviceID );
                    device->prev->next = device->next;
                    device->next->prev = device->prev;
                }
                else {
                    CVerbArg ( "RemoveDeviceUnlock: Finish list as the client [0x%X] was the last one", device->prev->info.deviceID );
                    device->prev->next = 0;
                }
                
                device->next = 0;
                
                appDevs->count--;
            }
        }
        appDevs->deviceCacheDirty = true;

        appDevs->UnlockWrite ( "RemoveDeviceUnlock" );

        NotifyClients ( NOTIFY_MEDIATOR_SRV_DEVICE_REMOVED, device->baseSP );
        
        CVerbArg ( "RemoveDeviceUnlock: Disposing device [0x%X]", device->info.deviceID );
        device->baseSP = 0;
    }
    
    
    void MediatorDaemon::RemoveDevice ( DeviceInstanceNode * device, bool useLock )
	{
		CVerbVerb ( "RemoveDevice" );

		if ( !device )
			return;

		sp ( ApplicationDevices ) appDevices = device->rootSP;
		if ( !appDevices )
			return;

		ApplicationDevices * appDevs = appDevices.get ();
		if ( !appDevs )
			return;

        if ( useLock )
            appDevs->LockWrite ( "RemoveDevice" );

		if ( device == appDevs->devices ) {
			if ( device->next ) {
				CVerbArg ( "RemoveDevice: relocating client [0x%X] to root of list", device->next->info.deviceID );

				device->prev = 0;
				appDevs->devices = device->next;

				device->next = 0;
				appDevs->count--;
			}
			else {
				appDevs->devices = 0;
				appDevs->count = 0;
			}
		}
		else {
			if ( !device->prev ) {
				CErrArg ( "RemoveDevice: Serious inconsistency error!!! Failed to lookup device list. Missing previous device for ID [0x%X]", device->info.deviceID );

				/// Best thing we can do is to assume that we are the root node of the list
				if ( device->next ) {
					CVerbArg ( "RemoveDevice: Relocating client [0x%X] to root of list", device->next->info.deviceID );

					device->next->prev = 0;
					appDevs->devices = device->next;
					appDevs->count--;
				}
				else {
					appDevs->devices = 0;
					appDevs->count = 0;
				}
			}
			else {
				if ( device->next ) {
					CVerbArg ( "RemoveDevice: Relinking client [0x%X] to previous client [0x%X]", device->next->info.deviceID, device->prev->info.deviceID );
					device->prev->next = device->next;
					device->next->prev = device->prev;
				}
				else {
					CVerbArg ( "RemoveDevice: Finish list as the client [0x%X] was the last one", device->prev->info.deviceID );
					device->prev->next = 0;
				}

				device->next = 0;

				appDevs->count--;
			}
		}
		appDevs->deviceCacheDirty = true;

        if ( useLock )
            appDevs->UnlockWrite ( "RemoveDevice" );

		NotifyClients ( NOTIFY_MEDIATOR_SRV_DEVICE_REMOVED, device->baseSP );

		CVerbArg ( "RemoveDevice: Disposing device [0x%X]", device->info.deviceID );
		device->baseSP = 0;
	}


	void MediatorDaemon::RemoveDevice ( unsigned int ip, char * msg )
	{
		CVerb ( "RemoveDevice from broadcast" );

		if ( !isRunning || !msg )
			return;

        int deviceID;
		char * areaName = 0;
		char * appName = 0;

		// Get the id at first (>0)	
        int * pUIntBuffer = ( int * ) ( msg + 12 );
		deviceID = *pUIntBuffer;
		if ( !deviceID )
			return;

		// Get the areaName, appname, etc..
		char * context = NULL;
		char * psem = strtok_s ( msg + MEDIATOR_BROADCAST_DESC_START, ";", &context );
		if ( !psem )
			return;

		psem = strtok_s ( NULL, ";", &context );
		if ( !psem )
			return;
		areaName = psem;

		psem = strtok_s ( NULL, ";", &context );
		if ( !psem )
			return;
		appName = psem;

		CVerbID ( "RemoveDevice BC:" );

		RemoveDevice ( deviceID, areaName, appName );
	}


	void MediatorDaemon::UpdateDeviceInstance ( const sp ( DeviceInstanceNode ) & device, bool added, bool changed )
	{
		if ( added )
			NotifyClients ( NOTIFY_MEDIATOR_SRV_DEVICE_ADDED, device );
		else if ( changed )
			NotifyClients ( NOTIFY_MEDIATOR_SRV_DEVICE_CHANGED, device );
	}


	/// 0 no filter
	/// 1 show all within the same area
	/// 2 show only within same area and apps

	void MediatorDaemon::UpdateNotifyTargets ( const sp ( ThreadInstance ) &clientSP, int filterMode )
	{
		if ( !clientSP )
			return;

		DeviceInstanceNode * device;
		ThreadInstance * client = clientSP.get ();

#ifdef USE_NOTIFY_TARGET_INDEX
        if ( client->inNotifierList == filterMode )
            return;
#endif

		sp ( DeviceInstanceNode ) deviceSP = client->deviceSP;
		if ( !deviceSP )
			return;

		device = deviceSP.get ();

		/// Check whether we need to remove the client from a targetlist
		if ( client->filterMode != filterMode )
		{
			if ( client->filterMode == MEDIATOR_FILTER_NONE )
			{
				/// Remove from NoRestrict notifiers
				CVerbArg ( "UpdateNotifyTargets: Looking for deviceID [ 0x%X : %s : %s ] in NONE-filter", client->deviceID, device ? device->info.areaName : "", device ? device->info.appName : "" );

				if ( LockAcquireA ( notifyTargetsLock, "UpdateNotifyTargets" ) )
				{
					const msp ( long long, ThreadInstance )::iterator notifyIt = notifyTargets.find ( client->sessionID );

					if ( notifyIt != notifyTargets.end () )
					{
						CVerbArg ( "UpdateNotifyTargets: Removing deviceID [ 0x%X : %s : %s ] from NONE-filter", client->deviceID, device ? device->info.areaName : "", device ? device->info.appName : "" );

						notifyTargets.erase ( notifyIt );

                        notifyTargetsSize = notifyTargets.size ();

#ifdef USE_NOTIFY_TARGET_INDEX
                        client->inNotifierList = -1;
#endif
					}

					LockReleaseVA ( notifyTargetsLock, "UpdateNotifyTargets" );
				}
			}
			else if ( client->filterMode == MEDIATOR_FILTER_AREA )
			{
				while ( device && device->rootSP )
				{
					/// Search for area
					const map<unsigned int, string>::iterator areaIdsIt = areaIDs.find ( device->rootSP->areaId );

					if ( areaIdsIt == areaIDs.end () ) {
						CWarnArg ( "UpdateNotifyTargets: Failed to find area ID [ %u ]", device->rootSP->areaId );
						break;
					}

					sp ( AreaApps ) areaApps;

                    areas.LockRead ( "UpdateNotifyTargets" );

					const msp ( string, AreaApps )::iterator areaIt = areas.list.find ( areaIdsIt->second );

					if ( areaIt == areas.list.end () ) {
						CWarnArg ( "UpdateNotifyTargets: Failed to find area [ %s ]", areaIdsIt->second.c_str () );
					}
					else
						areaApps = areaIt->second;

                    areas.UnlockRead ( "UpdateNotifyTargets" );

					if ( areaApps && areaApps->Lock2 ( "UpdateNotifyTargets" ) ) {
						CVerbArg ( "UpdateNotifyTargets: Looking for deviceID [ 0x%X : %s : %s ] in Area-filter", client->deviceID, device->info.areaName, device->info.appName );

						const msp ( long long, ThreadInstance )::iterator notifyIt = areaApps->notifyTargets.find ( client->sessionID );

						if ( notifyIt != areaApps->notifyTargets.end () )
						{
							CVerbArg ( "UpdateNotifyTargets: Removing deviceID [ 0x%X : %s : %s ] from Area-filter", client->deviceID, device->info.areaName, device->info.appName );

							areaApps->notifyTargets.erase ( notifyIt );

                            areaApps->notifyTargetsSize = areaApps->notifyTargets.size ();

#ifdef USE_NOTIFY_TARGET_INDEX
                            client->inNotifierList = -1;
#endif
						}

						areaApps->Unlock2 ( "UpdateNotifyTargets" );
					}
					break;
				}
			}
		}

		client->filterMode = ( short ) filterMode;

		if ( filterMode < MEDIATOR_FILTER_NONE )
			return;

		if ( filterMode == MEDIATOR_FILTER_ALL ) {
			return;
		}

		if ( filterMode == MEDIATOR_FILTER_NONE )
		{
			CVerbArg ( "UpdateNotifyTargets: Adding deviceID [ 0x%X : %s : %s ] to NoRestrict-targets", client->deviceID, device ? device->info.areaName : "", device ? device->info.appName : "" );

			if ( LockAcquireA ( notifyTargetsLock, "UpdateNotifyTargets" ) )
			{
				notifyTargets [ client->sessionID ] = clientSP;

                notifyTargetsSize = notifyTargets.size ();

#ifdef USE_NOTIFY_TARGET_INDEX
                client->inNotifierList = filterMode;
#endif

				LockReleaseVA ( notifyTargetsLock, "UpdateNotifyTargets" );
			}
		}
		else if ( filterMode == MEDIATOR_FILTER_AREA )
		{
			while ( device && device->rootSP )
			{
				/// Search for area
				const map<unsigned int, string>::iterator areaIDsIt = areaIDs.find ( device->rootSP->areaId );

				if ( areaIDsIt == areaIDs.end () ) {
					CWarnArg ( "UpdateNotifyTargets: Failed to find area ID [ %u ]", device->rootSP->areaId );
					break;
				}

				sp ( AreaApps ) areaApps;

                areas.LockRead ( "UpdateNotifyTargets" );

				const msp ( string, AreaApps )::iterator areaIt = areas.list.find ( areaIDsIt->second );

				if ( areaIt == areas.list.end () ) {
					CWarnArg ( "UpdateNotifyTargets: Failed to find area [ %s ]", areaIDsIt->second.c_str () );
				}
				else
					areaApps = areaIt->second;

                areas.UnlockRead ( "UpdateNotifyTargets" );

                AreaApps * aas = areaApps.get ();

				if ( aas && aas->Lock2 ( "UpdateNotifyTargets" ) ) {
					CVerbArg ( "UpdateNotifyTargets: Adding deviceID [ 0x%X : %s : %s ] to Area-filter", client->deviceID, device->info.areaName, device->info.appName );

                    aas->notifyTargets [ client->sessionID ] = clientSP;

                    aas->notifyTargetsSize = aas->notifyTargets.size ();

#ifdef USE_NOTIFY_TARGET_INDEX
                    client->inNotifierList = filterMode;
#endif
					aas->Unlock2 ( "UpdateNotifyTargets" );
				}
				break;
			}
		}
	}


	sp ( ApplicationDevices ) MediatorDaemon::GetDeviceList ( char * areaName, char * appName, pthread_mutex_t ** lock, int ** pDevicesAvailable, DeviceInstanceNode ** &list )
	{
		sp ( ApplicationDevices )	appDevices;
        ApplicationDevices       *  appDevs     = 0;
		sp ( AreaApps )				areaApps;
		sp ( AppsList )				appsList;

		string appsName ( appName );
		string pareaName ( areaName );

#ifdef USE_WRITE_LOCKS3
        bool unlockReadLock = true;
#endif
        areas.LockRead ( "GetDeviceList" );

		const msp ( string, AreaApps )::iterator areaIt = areas.list.find ( pareaName );

		if ( areaIt == areas.list.end () )
        {
#ifdef USE_WRITE_LOCKS3
            areas.UnlockRead ( "GetDeviceList" );

            unlockReadLock = false;

            areas.LockWrite ( "GetDeviceList" );

            const msp ( string, AreaApps )::iterator areaIt1 = areas.list.find ( pareaName );
            if ( areaIt1 != areas.list.end () )
            {
                areaApps = areaIt1->second;

                areas.UnlockWrite ( "GetDeviceList" );
            }
            else
            {
#endif
                /// Create a new one...
                areaApps = make_shared < AreaApps > ();

                if ( areaApps && areaApps->Init () && areaApps->Init2 () ) {
                    areasCounter++;
                    areaIDs [ areasCounter ] = pareaName;

                    areaApps->name	= pareaName;
                    areaApps->id	= areasCounter;
                    areaApps->apps.clear ();
                    areaApps->notifyTargets.clear ();

                    areaApps->notifyTargetsSize = 0;
                    
                    areas.list [ pareaName ] = areaApps;
                }
				else {
					CErrArg ( "GetDeviceList: Failed to create new area [ %s ]!", areaName ); 
					areaApps.reset (); 
				}
                
#ifdef USE_WRITE_LOCKS3
                areas.UnlockWrite ( "GetDeviceList" );
            }
#endif
        }
        else
            areaApps = areaIt->second;

#ifdef USE_WRITE_LOCKS3
        if ( unlockReadLock )
#endif
            areas.UnlockRead ( "GetDeviceList" );

        AreaApps * aas = areaApps.get ();

        if ( !aas || !aas->LockRead ( "GetDeviceList" ) )
        {
			CLogArg ( "GetDeviceList: App [ %s ] not found.", appName );
			return 0;
		}

#ifdef USE_WRITE_LOCKS2
        bool releaseReadLock = true;
		bool resetAppDevs	 = false;
#endif
		const msp ( string, ApplicationDevices )::iterator appsIt = aas->apps.find ( appsName );

		if ( appsIt == aas->apps.end () )
        {
			/// Create a new one...
			appDevices	= make_shared < ApplicationDevices > ();
            appDevs		= appDevices.get ();

#ifdef USE_WRITE_LOCKS2
			resetAppDevs = true;
#endif
            if ( appDevs && appDevs->Init ()
#ifdef USE_WRITE_LOCKS2
                && LockInitA ( appDevs->deviceCacheLock )
#endif
                ) {
#ifdef USE_WRITE_LOCKS2
                // Release read lock, get write lock, and search again
                aas->UnlockRead ( "GetDeviceList" );

                releaseReadLock = false;

                if ( aas->LockWrite ( "GetDeviceList" ) )
                {
                    msp ( string, ApplicationDevices )::iterator appsIt1 = aas->apps.find ( appsName );
                    if ( appsIt1 != aas->apps.end () )
                    {
                        // Somebody has created and added an item while we were acquiring the write lock
                        appDevices	= appsIt1->second;
                        appDevs		= appDevices.get ();
                        aas->UnlockWrite ( "GetDeviceList" );
                    }
                    else
                    {
#endif
                        appsCounter++;
                        appDevs->id         = appsCounter;
                        appDevs->areaId     = aas->id;
                        appDevs->name       = appsName;
                        appIDs [ appsCounter ] = appsName;

                        // Query maxIDs from value packs
                        if ( areasMap.Lock ( "GetDeviceList" ) )
                        {
                            const msp ( string, AppsList )::iterator areaVPIt = areasMap.list.find ( pareaName );
                            if ( areaVPIt != areasMap.list.end () )
                                appsList = areaVPIt->second;
                            else
                            {
                                // Create a new map for the area
                                appsList = make_shared < AppsList > ();
                                if ( !appsList || !appsList->Init () ) {
                                    CErrArg ( "GetDeviceList: Failed to create new area [ %s ].", pareaName.c_str () );

                                    appsList.reset ();
                                }
                                else { areasMap.list [ pareaName ] = appsList; }
                            }

                            if ( areasMap.Unlock ( "GetDeviceList" ) && appsList && appsList->Lock ( "GetDeviceList" ) )
                            {
                                sp ( ListValues ) listValues;

                                const msp ( string, ListValues )::iterator appsVPIt = appsList->apps.find ( appsName );
                                if ( appsVPIt != appsList->apps.end () )
                                    listValues = appsVPIt->second;
                                else
                                {
                                    listValues = make_shared < ListValues > (); // new map<string, ValuePack*> ();
                                    if ( !listValues || !listValues->Init () ) {
                                        CErrArg ( "GetDeviceList: Failed to create new application [ %s ].", appsName.c_str () );
                                        
                                        listValues.reset ();
                                    }
                                    else { appsList->apps [ appName ] = listValues; }
                                }
                                
                                if ( appsList->Unlock ( "GetDeviceList" ) )
                                {
                                    if ( listValues && listValues->Lock ( "GetDeviceList" ) )
                                    {
                                        const msp ( string, ValuePack )::iterator valueIt = listValues->values.find ( string ( "0_maxID" ) );
                                        
                                        if ( valueIt != listValues->values.end () )
                                        {
                                            sp ( ValuePack ) value = valueIt->second;
                                            if ( value )
                                                appDevs->latestAssignedID = value->size;
                                        }
                                        else {
                                            // Create a new one
                                            string value = "1";
                                            
                                            AddToArea ( listValues, "0_maxID", value.c_str (), ( unsigned int ) value.length () );
                                        }
                                        
                                        listValues->Unlock ( "GetDeviceList" );
                                    }
                                    
                                    aas->apps [ appsName ] = appDevices;
#ifdef USE_WRITE_LOCKS2
									resetAppDevs = false;
#endif
                                }
                            }
                        }
#ifdef USE_WRITE_LOCKS2
                        aas->UnlockWrite ( "GetDeviceList" );                        
                    }
                }
#endif
			}
            else { CErrArg ( "GetDeviceList: Failed to create new application devicelist [ %s ].", appName ); }
        }
        else {
            appDevices	= appsIt->second;
            appDevs		= appDevices.get ();
        }

#ifdef USE_WRITE_LOCKS2
        if ( resetAppDevs ) {
            appDevices.reset (); appDevs = 0;
        }

        if ( releaseReadLock )
#endif
            aas->UnlockRead ( "GetDeviceList" );

		if ( !appDevs || appDevs->access <= 0 )
			goto Finish;

		__sync_add_and_fetch ( &appDevs->access, 1 );

        if ( lock )
            *lock = &appDevs->lock1;
		if ( pDevicesAvailable )
			*pDevicesAvailable = &appDevs->count;

		list = &appDevs->devices;

	Finish:
		return appDevices;
	}


	void printDevice ( DeviceInstanceNode * device )
	{
		CLogArg ( "\nDevice      = [ 0x%X : %s : %s ]", device->info.deviceID, device->info.deviceName, device->info.broadcastFound ? "on same network" : "by mediator" );
		if ( printStdOut )
			printf ( "\nDevice      = [0x%X : %s : %s ]\n", device->info.deviceID, device->info.deviceName, device->info.broadcastFound ? "on same network" : "by mediator" );
		CLogArg ( "UID         = [ %d : %s]", device->clientSP ? device->clientSP->authLevel : -1, device->clientSP ? device->clientSP->uid : ".-." );
		if ( printStdOut )
			printf ( "UID         = [ %d : %s ]\n", device->clientSP ? device->clientSP->authLevel : -1, device->clientSP ? device->clientSP->uid : ".-." );
		if ( device->info.ip != device->info.ipe ) {
			CLogArg ( "Device IPe != IP [ %s ]", inet_ntoa ( *( ( struct in_addr * ) &device->info.ip ) ) );
			if ( printStdOut )
				printf ( "Device IPe != IP [ %s ]\n", inet_ntoa ( *( ( struct in_addr * ) &device->info.ip ) ) );
		}
		CLogArg ( "Area/App    = [ %s : %s ]", device->info.areaName, device->info.appName );
		if ( printStdOut )
			printf ( "Area/App    = [ %s : %s ]\n", device->info.areaName, device->info.appName );
		CLogArg ( "Device  IPe = [ %s (from socket) [ tcp %d ]  [ udp %d ]]", inet_ntoa ( *( ( struct in_addr * ) &device->info.ipe ) ), device->info.tcpPort, device->info.udpPort );
		if ( printStdOut )
			printf ( "Device  IPe = [ %s (from socket) [ tcp %d ]  [ udp %d ]]\n", inet_ntoa ( *( ( struct in_addr * ) &device->info.ipe ) ), device->info.tcpPort, device->info.udpPort );
	}


	void printDeviceList ( DeviceInstanceNode * device )
	{
		while ( device )
		{
			printDevice ( device );

			device = device->next;
		}
	}


	void MediatorDaemon::PrintSmallHelp ()
	{
		printf ( "Press ESC to quit; h for help.\n" );
	}


	void MediatorDaemon::PrintHelp ()
	{
		printf ( "-------------------------------------------------------\n" );
		printf ( "a - add/update user (no pass for delete)\n" );
		printf ( "b - send broadcast packet to clients\n" );
		printf ( "c - print configuration\n" );
		printf ( "d - print database\n" );
		printf ( "e - show userlist\n" );
		printf ( "g - print client resources\n" );
		printf ( "h - print help\n" );
		printf ( "i - print interfaces\n" );
		printf ( "j - toggle Acceptor\n" );
		printf ( "l - toggle logging to file\n" );
		printf ( "m - print mediators\n" );
		printf ( "o - toggle logging to output (std)\n" );
		printf ( "p - print active devices\n" );
		printf ( "r - reload pkcs keys\n" );
		printf ( "s - toggle output to stdout\n" );
        printf ( "t - toggle authentication\n" );
		printf ( "w - revoke last bann\n" );
        printf ( "x - show bann list\n" );
        printf ( "y - show connected IPs\n" );
		printf ( "z - clear bann list\n" );
#ifdef DEBUG_CHECK_CLIENT_CALL_MS
		printf ( "n - measure ms (%i) set\n", maxMeasureDiff );
#endif
		printf ( "-------------------------------------------------------\n" );
	}


	void MediatorDaemon::Run ()
	{
		CVerb ( "Run" );

		isRunning = true;
		//static int keyCount = 0;
		//static int valueCount = 0;

		CLog ( "Service is running..." );

		PrintSmallHelp ();
		printf ( "-------------------------------------------------------\n" );

		char command = 0;
		bool hideInput = false;
		string userName;
		string passPhrase;
		int accessLevel = 0;

		do
		{
#if defined(SLEEPRUNNER)	
			Sleep ( 10000 );
#else
			int c = _getch ();
			//printf ( "-------------------------------------------------------\n" );
			//CLog ( "Mediator exit requested!" );
			if ( c == 27 ) {
				printf ( "-------------------------------------------------------\n" );
				CLog ( "Mediator exit requested!" );

				// Stop the threads
				break;
			}
			//printf ( "c [%i]\n", c );

			if ( c == 8 || c == 127 ) {
				if ( input >= inputBuffer ) {
					if ( input > inputBuffer )
						input--;
					*input = 0;

					CLog ( "" );
					CLogArg ( "%s", inputBuffer );
					if ( printStdOut ) {
						printf ( "\r\n" );
						printf ( "%s", inputBuffer );
					}
				}
				continue;
			}

			if ( input == inputBuffer && !command ) {
				if ( c == 'a' ) {
					CLog ( "Add new user:" );
					CLog ( "----------------------------------------------------------------" );
					if ( printStdOut ) {
						printf ( "Add new user:\n" );
						printf ( "----------------------------------------------------------------\n" );
					}
					CLog ( "Please enter an access level (0 - 10). Default [3]:" );
					if ( printStdOut )
						printf ( "Please enter an access level (0 - 10). Default [3]: " );
					command = 'a';
					accessLevel = -1;
					userName = "a";
					continue;
				}
#ifdef DEBUG_CHECK_CLIENT_CALL_MS
				else if ( c == 'n' ) {
					CLog ( "Change ms for measure:" );
					CLog ( "----------------------------------------------------------------" );
					if ( printStdOut ) {
						printf ( "Change ms for measure:\n" );
						printf ( "----------------------------------------------------------------\n" );
					}
					CLogArg ( "Please enter a value (1...). Current [%u]:", maxMeasureDiff );
					if ( printStdOut )
						printf ( "Please enter a value (1...). Current [%u]: ", maxMeasureDiff );
					command = 'n';
					continue;
				}
#endif
				else if ( c == 'b' ) {
					SendBroadcast ();
					continue;
				}
				else if ( c == 'c' ) {
					CLog ( "Configuration:" );
					CLog ( "----------------------------------------------------------------" );
					CLog ( "Ports: " );
					if ( printStdOut ) {
						printf ( "Configuration:\n" );
						printf ( "----------------------------------------------------------------\n" );
						printf ( "Ports:\n " );
					}
					for ( unsigned int pos = 0; pos < ports.size (); pos++ ) {
						CLogArg ( " %i", ports [ pos ] );
						if ( printStdOut )
							printf ( " %i\n", ports [ pos ] );
					}
					//printf ( "\n" );
					CLog ( "----------------------------------------------------------------" );
					if ( printStdOut )
						printf ( "----------------------------------------------------------------\n" );
					continue;
				}
				else if ( c == 'd' ) {
					CLog ( "Database:" );
					CLog ( "----------------------------------------------------------------" );
					if ( printStdOut ) {
						printf ( "Database:\n" );
						printf ( "----------------------------------------------------------------\n" );
					}
					if ( areasMap.Lock ( "Run" ) )
					{
						for ( msp ( string, AppsList )::iterator it = areasMap.list.begin (); it != areasMap.list.end (); ++it )
						{
							sp ( AppsList ) appsList = it->second;
							if ( !appsList || !appsList->Lock ( "Run" ) )
								continue;

							CLogArg ( "Area: %s", it->first.c_str () );

							for ( msp ( string, ListValues )::iterator ita = appsList->apps.begin (); ita != appsList->apps.end (); ++ita )
							{
								sp ( ListValues ) values = ita->second;
								if ( !values || !values->Lock ( "Run" ) )
									continue;

								CLogArg ( "App: %s", ita->first.c_str () );

								for ( msp ( string, ValuePack )::iterator itv = values->values.begin (); itv != values->values.end (); ++itv )
								{
									sp ( ValuePack ) value = itv->second;
									if ( value ) {
										CLogArg ( "\tKey: %s\tValue: %s", itv->first.c_str (), value->value.c_str () );
									}
								}
								values->Unlock ( "Run" );
							}

							appsList->Unlock ( "Run" );
						}

						areasMap.Unlock ( "Run" );
					}
					printf ( "----------------------------------------------------------------\n" );
					continue;
				}
                else if ( c == 'e' ) {
                    CLog ( "Users:" );
                    CLog ( "----------------------------------------------------------------" );
                    if ( printStdOut ) {
                        printf ( "Users:\n" );
                        printf ( "----------------------------------------------------------------\n" );
                    }
                    if ( LockAcquireA ( usersDBLock, "Run" ) )
                    {
                        for ( map<string, UserItem *>::iterator it = usersDB.begin (); it != usersDB.end (); ++it )
                        {
                            if ( it->second )
                            {
                                UserItem * item = it->second;

                                const char * pwStr = ConvertToHexString ( item->pass.c_str (), ENVIRONS_USER_PASSWORD_LENGTH );
                                if ( pwStr ) {
                                    CLogArg ( "User: %s\t%i\t[ %s ]", it->first.c_str (), item->authLevel, pwStr );
                                    if ( printStdOut ) {
                                        printf ( "User: %s\t%i\t[ %s ]\n", it->first.c_str (), item->authLevel, pwStr );
                                    }
                                }
                            }
                        }
                        
                        LockReleaseVA ( usersDBLock, "Run" );
                    }
                    printf ( "----------------------------------------------------------------\n" );
                    continue;
				}
				else if ( c == 'g' ) {
					CLog ( "Active clients:" );
					CLog ( "----------------------------------------------------------------" );

					if ( printStdOut ) {
						printf ( "Active clients:\n" );
						printf ( "----------------------------------------------------------------\n" );
					}

					if ( acceptClients.Lock ( "Run" ) )
					{
						msp ( void *, ThreadInstance ) &list = acceptClients.list;
						msp ( void *, ThreadInstance )::iterator itm = list.begin ();

						size_t i = 0;

						while ( itm != list.end () )
						{
							i++;

							const sp ( ThreadInstance ) &clientSP = itm->second;
							if ( clientSP )
							{
								sp ( DeviceInstanceNode ) deviceSP = clientSP->deviceSP;
								if ( deviceSP )
								{
									CLogArg ( "[%i]: [%i]", i, clientSP->deviceID );

									if ( printStdOut )
										printf ( "[%zu]: [%i]\n", i, clientSP->deviceID );

									printDevice ( deviceSP.get () );
								}
							}
							else {
								CErrArg ( "[%i]: **** Invalid Client\n\n", i );
								if ( printStdOut )
									printf ( "[%zu]: **** Invalid Client\n\n", i );
							}
							CLog ( "----------------------------------------------------------------" );
							if ( printStdOut )
								printf ( "----------------------------------------------------------------\n" );
						}

						acceptClients.Unlock ( "Run" );
					}
					CLog ( "----------------------------------------------------------------" );
					if ( printStdOut )
						printf ( "----------------------------------------------------------------\n" );
					continue;
				}
				else if ( c == 'h' ) { // Help requested
					PrintHelp ();
					continue;
				}
				else if ( c == 'i' ) {
					if ( !LockAcquireA ( localNetsLock, "Run" ) )
						continue;

					NetPack * net = &localNets;
					while ( net ) {
						CLogArg ( "Interface ip:%s", inet_ntoa ( *( ( struct in_addr * ) &net->ip ) ) );
						CLogArg ( "Interface bcast:%s", inet_ntoa ( *( ( struct in_addr * ) &net->bcast ) ) );

						if ( printStdOut ) {
							printf ( "Interface ip:%s\n", inet_ntoa ( *( ( struct in_addr * ) &net->ip ) ) );
							printf ( "Interface bcast:%s\n", inet_ntoa ( *( ( struct in_addr * ) &net->bcast ) ) );
						}
						net = net->next;
					}

					LockReleaseVA ( localNetsLock, "Run" );
					continue;
				}
				else if ( c == 'j' ) {
					acceptEnabled = !acceptEnabled;
					if ( printStdOut )
						printf ( "Run: Acceptor is now [%s]\n", acceptEnabled ? "enabled" : "disabled" );
					CLogArg ( "Run: Acceptor is now [%s]", acceptEnabled ? "enabled" : "disabled" );
					continue;
				}
				else if ( c == 'l' ) {
					logToFile = !logToFile;
					fileLog = logToFile;

					CloseLog ();
					OpenLog ();
					usersDBDirty = true;

					if ( printStdOut )
						printf ( "Run: File logging is now [%s]\n", logToFile ? "enabled" : "disabled" );
					CLogArg ( "Run: File logging is now [%s]", logToFile ? "enabled" : "disabled" );
					continue;
				}
				else if ( c == 'm' ) {
					if ( !mediator.ip ) {
						CLog ( "No more mediators known yet!" );
						if ( printStdOut )
							printf ( "No more mediators known yet!\n" );
						continue;
					}
					else {
#ifdef ENABLE_MEDIATOR_LOCK
						if ( !LockAcquireA ( mediatorLock, "Run" ) )
							continue;
#endif
						CLog ( "Mediators:" );
						CLog ( "----------------------------------------------------------------" );
						if ( printStdOut ) {
							printf ( "Mediators:\n" );
							printf ( "----------------------------------------------------------------\n" );
						}

						MediatorInstance * net = &mediator;

						while ( net ) {
							CLogArg ( "\tIP: %s\tPorts: ", inet_ntoa ( *( ( struct in_addr * ) &net->ip ) ) );
							if ( printStdOut )
								printf ( "\tIP: %s\tPorts: \n", inet_ntoa ( *( ( struct in_addr * ) &net->ip ) ) );

							for ( int i=0; i<MAX_MEDIATOR_PORTS; i++ ) {
								if ( !net->port )
									break;
								CLogArg ( "%i ", net->port );
								if ( printStdOut )
									printf ( "%i \n", net->port );
							}
							CLog ( "\n" );
							if ( printStdOut )
								printf ( "\n" );
							net = net->next;
						}
						CLog ( "----------------------------------------------------------------" );
						if ( printStdOut )
							printf ( "----------------------------------------------------------------\n" );

#ifdef ENABLE_MEDIATOR_LOCK
						LockReleaseVA ( mediatorLock, "Run" );
#endif
					}
					continue;
				}
				else if ( c == 'o' ) {
					stdLog = !stdLog;
					usersDBDirty = true;

					if ( printStdOut )
						printf ( "Run: Std output logging is now [%s]\n", stdLog ? "enabled" : "disabled" );
					CLogArg ( "Run: Std output logging is now [%s]", stdLog ? "enabled" : "disabled" );
					continue;
				}
				else if ( c == 'p' ) {
					CLog ( "Active devices:" );
					CLog ( "----------------------------------------------------------------" );
					if ( printStdOut ) {
						printf ( "Active devices:\n" );
						printf ( "----------------------------------------------------------------\n" );
					}

                    if ( areas.LockRead ( "Run" ) )
					{
						for ( msp ( string, AreaApps )::iterator it = areas.list.begin (); it != areas.list.end (); ++it )
						{
							if ( it->second ) {
								sp ( AreaApps ) areaApps = it->second;
								CLogArg ( "P: [%s]", it->first.c_str () );
								if ( printStdOut )
									printf ( "P: [%s]\n", it->first.c_str () );

                                if ( !areaApps || !areaApps->LockRead ( "Run" ) )
									continue;

								for ( msp ( string, ApplicationDevices )::iterator ita = areaApps->apps.begin (); ita != areaApps->apps.end (); ++ita )
								{
									sp ( ApplicationDevices ) appDevices = ita->second;
									if ( appDevices )
									{
										if ( appDevices->devices )
										{
											CLog ( "----------------------------------------------------------------" );
											CLogArg ( "A: [%s]", ita->first.c_str () );
											if ( printStdOut ) {
												printf ( "----------------------------------------------------------------\n" );
												printf ( "A: [%s]\n", ita->first.c_str () );
											}

                                            if ( appDevices->LockRead ( "Run" ) )
											{
												printDeviceList ( appDevices->devices );

                                                appDevices->UnlockRead ( "Run" );
											}
											CLog ( "----------------------------------------------------------------" );
											if ( printStdOut )
												printf ( "----------------------------------------------------------------\n" );
										}
										else {
											CLogArg ( "A: No devices in [%s]", ita->first.c_str () );
											if ( printStdOut )
												printf ( "A: No devices in [%s]\n", ita->first.c_str () );
										}
									}
                                }
                                areaApps->UnlockRead ( "Run" );
							}
                        }
                        areas.UnlockRead ( "Run" );
					}
					CLog ( "----------------------------------------------------------------" );
					if ( printStdOut )
						printf ( "----------------------------------------------------------------\n" );
					continue;
				}
				else if ( c == 'q' ) {
					printf ( "-------------------------------------------------------\n" );
					CLog ( "Mediator exit requested!" );

					// Stop the threads
					break;
				}
				else if ( c == 'r' ) {
					if ( printStdOut )
						printf ( "Run: Reloading keys\n" );
					CLog ( "Run: Reloading keys" );
					LoadKeys ();
					continue;
				}
				else if ( c == 's' ) {
					printStdOut = !printStdOut;
					if ( printStdOut )
						printf ( "Run: Stdout is now [%s]\n", printStdOut ? "required" : "disabled" );
					CLogArg ( "Run: Stdout is now [%s]", printStdOut ? "required" : "disabled" );
					continue;
				}
				else if ( c == 't' ) {
					reqAuth = !reqAuth;
					if ( printStdOut )
						printf ( "Run: Authentication is now [%s]\n", reqAuth ? "required" : "disabled" );
					CLogArg ( "Run: Authentication is now [%s]", reqAuth ? "required" : "disabled" );
					continue;
                }
				else if ( c == 'w' ) {
					if ( printStdOut ) {
						printf ( "-------------------------------------------------------\n" );
						printf ( "Revoke last IP ban [ %s ].\n", inet_ntoa ( *( ( struct in_addr * ) &bannedIPLast ) ) );
						printf ( "-------------------------------------------------------\n" );
					}

					if ( LockAcquireA ( bannedIPsLock, "Run" ) ) {
						CLogArg ( "Revoke last IP ban [ %s ].", inet_ntoa ( *( ( struct in_addr * ) &bannedIPLast ) ) );

						std::map<unsigned int, std::time_t>::iterator iter = bannedIPs.find ( bannedIPLast );
						if ( iter != bannedIPs.end () ) {
							bannedIPs.erase ( iter );
						}

						std::map<unsigned int, unsigned int>::iterator iter1 = connectedIPs.find ( bannedIPLast );
						if ( iter1 != connectedIPs.end () ) {
							connectedIPs.erase ( iter1 );
						}

						LockReleaseVA ( bannedIPsLock, "Run" );
					}
					continue;
				}
                else if ( c == 'x' ) {
                    if ( LockAcquireA ( bannedIPsLock, "Run" ) ) {
                        if ( printStdOut ) {
                            printf ( "-------------------------------------------------------\n" );
                            printf ( "Connected IPs with [ %u ] entries.\n", ( unsigned int ) connectedIPs.size () );
                            printf ( "-------------------------------------------------------\n" );
                        }
                        CLogArg ( "Connected IPs with [ %u ] entries..", ( unsigned int ) connectedIPs.size () );
                        
                        std::map<unsigned int, unsigned int>::iterator iter;
                        
                        for ( iter = connectedIPs.begin (); iter != connectedIPs.end (); ++iter ) {
                            if ( printStdOut )
                                printf ( "[ %-15s : %u ]\n", inet_ntoa ( *( ( struct in_addr * ) &iter->first ) ), iter->second );
                            CLogArg ( "[ %-15s : %u ]", inet_ntoa ( *( ( struct in_addr * ) &iter->first ) ), iter->second );
                        }
                        
                        LockReleaseVA ( bannedIPsLock, "Run" );
                    }
                    continue;
                }
                else if ( c == 'y' ) {
                    if ( LockAcquireA ( bannedIPsLock, "Run" ) ) {
                        if ( printStdOut ) {
                            printf ( "-------------------------------------------------------\n" );
                            printf ( "Bannlist with [ %u ] entries.\n", ( unsigned int ) bannedIPs.size () );
                            printf ( "-------------------------------------------------------\n" );
                        }
                        CLogArg ( "Bannlist with [ %u ] entries..", ( unsigned int ) bannedIPs.size () );
                        
                        std::map<unsigned int, std::time_t>::iterator iter;

                        for ( iter = bannedIPs.begin (); iter != bannedIPs.end (); ++iter ) {
                            if ( printStdOut )
                                printf ( "[ %s ]\n", inet_ntoa ( *( ( struct in_addr * ) &iter->first ) ) );
                            CLogArg ( "[ %s ]", inet_ntoa ( *( ( struct in_addr * ) &iter->first ) ) );
                        }
                        
                        if ( printStdOut ) {
                            printf ( "-------------------------------------------------------\n" );
                            printf ( "IPs with tries [ %u ] entries.\n", ( unsigned int ) bannedIPConnects.size () );
                            printf ( "-------------------------------------------------------\n" );
                        }
                        CLogArg ( "IPs with tries [ %u ] entries..", ( unsigned int ) bannedIPConnects.size () );
                        
                        std::map<unsigned int, unsigned int>::iterator itert;
                        
                        for ( itert = bannedIPConnects.begin (); itert != bannedIPConnects.end (); ++itert ) {
                            if ( printStdOut )
                                printf ( "[ %-15s : %u ]\n", inet_ntoa ( *( ( struct in_addr * ) &itert->first ) ), itert->second );
                            CLogArg ( "[ %-15s : %u ]", inet_ntoa ( *( ( struct in_addr * ) &itert->first ) ), itert->second );
                        }
                        
                        LockReleaseVA ( bannedIPsLock, "Run" );
                    }
                    continue;
                }
                else if ( c == 'z' ) {
					if ( LockAcquireA ( bannedIPsLock, "Run" ) ) {
						if ( printStdOut )
							printf ( "Run: Clearing bannlist with [ %u ] entries.\n", ( unsigned int ) bannedIPs.size () );
						CLogArg ( "Run: Clearing bannlist with [ %u ] entries.", ( unsigned int ) bannedIPs.size () );
						bannedIPs.clear ();
						//bannedIPConnects.clear ();

						LockReleaseVA ( bannedIPsLock, "Run" );
					}
					continue;
				}
			}
			if ( c == 13 || c == 10 ) { // Return key
										// Close input string
				*input = 0;

				// Analyse input string
				if ( command == 'a' ) {
					if ( accessLevel < 0 ) {
						if ( sscanf_s ( inputBuffer, "%d", &accessLevel ) <= 0 || accessLevel < 0 || accessLevel > 10 )
							accessLevel = 3;
						input = inputBuffer;

						CLog ( "Please enter the username (email):" );
						if ( printStdOut )
							printf ( "\nPlease enter the username (email): " );
					}
					else if ( userName.c_str () [ 0 ] == 'a' && userName.length () == 1 ) {
						userName = inputBuffer;
						if ( userName.find ( '@', 0 ) == string::npos ) {
							command = 0;
							userName = "";
							CLog ( "The username must be a valid email address!" );
							if ( printStdOut )
								printf ( "The username must be a valid email address!\n" );
						}
						else {
							CLog ( "Please enter the passphrase:" );
							if ( printStdOut )
								printf ( "\nPlease enter the passphrase: " );
							input = inputBuffer;
							hideInput = true;
							continue;
						}
					}
					else {
						hideInput = false;
						if ( AddUser ( accessLevel, userName.c_str (), inputBuffer ) ) {
                            if ( strlen ( inputBuffer ) == 0 ) {
                                CLogArg ( "User: [ %s ] successfully removed.", userName.c_str () );
                                if ( printStdOut )
                                    printf ( "\nUser: [ %s ] successfully removed.\n", userName.c_str () );
                            }
                            else {
                                CLogArg ( "User: [ %s ] successfully added.", userName.c_str () );
                                if ( printStdOut )
                                    printf ( "\nUser: [ %s ] successfully added.\n", userName.c_str () );
                            }
						}
						else {
							CErrArg ( "User: Failed to add user [ %s ]", userName.c_str () );
							if ( printStdOut )
								printf ( "\nUser: Failed to add user [ %s ]\n", userName.c_str () );
						}
						command = 0;
						userName = "";
						printf ( "----------------------------------------------------------------\n" );
					}
				}
#ifdef DEBUG_CHECK_CLIENT_CALL_MS
				else if ( command == 'n' ) {
					unsigned int ms = 30;
					if ( sscanf_s ( inputBuffer, "%u", &ms ) > 0 && ms > 0 )
						maxMeasureDiff = ms;
					command = 0;
					*input = 0;
				}
#endif
				CLog ( "" );
				if ( printStdOut )
					printf ( "\n" );
				input = inputBuffer;
				continue;
			}
			else {
				*input = ( char ) c;
				if ( hideInput )
					printf ( "*" );
				else
					printf ( "%c", c );

				input++;
				if ( input == inputBuffer + MEDIATOR_CLIENT_MAX_BUFFER_SIZE - 1 ) {
					// Buffer overflow now
					input = inputBuffer;

					CLog ( "\nInput too long! Try again!" );
					if ( printStdOut )
						printf ( "\nInput too long! Try again!\n" );
					PrintSmallHelp ();
				}
			}
#endif
		}
		while ( isRunning );
	}


	void * MediatorDaemon::MediatorUdpThreadStarter ( void * arg )
	{
		if ( arg ) {
			MediatorThreadInstance * listeners = ( MediatorThreadInstance * ) arg;
			if ( listeners->instance.daemon ) {
				// Execute thread
				( ( MediatorDaemon * ) listeners->instance.daemon )->MediatorUdpThread ( listeners );
			}
		}
		return 0;
	}


	void * MediatorDaemon::MediatorUdpThread ( void * arg )
	{
		CVerb ( "MediatorUdpThread started." );

		if ( !arg ) {
			CErr ( "MediatorUdpThread: called with invalid (NULL) argument." );
			return 0;
		}

		up ( char [ ] ) bufferUP = up ( char [ ] ) ( new char [ 64000 ] );
		if ( !bufferUP )
			return 0;

		char				*	buffer			= bufferUP.get ();
		char				*	decrypted		= 0;
		char				*	msgDec			= 0;
		unsigned int			msgDecLength;
		unsigned int			msgLength;
		sp ( ThreadInstance )	client;

		struct 	sockaddr_in		listenAddr;
		MediatorThreadInstance * listener = ( MediatorThreadInstance * ) arg;

		unsigned short port = listener->instance.port;
		unsigned int sock = listener->socketUdp;

		CVerbArg ( "MediatorUdpThread for port [ %d ] started.", port );

		// Send started signal
		if ( !listener->udp.Notify ( "Acceptor" ) ) {
			CErr ( "MediatorUdpThread: Error to signal event" );
		}

		memset ( &listenAddr, 0, sizeof ( listenAddr ) );

		listenAddr.sin_family		= AF_INET;
		listenAddr.sin_addr.s_addr	= INADDR_ANY; //htonl ( INADDR_BROADCAST ); // INADDR_ANY );
		listenAddr.sin_port			= htons ( port );

		int ret = ::bind ( sock, ( struct sockaddr * )&listenAddr, sizeof ( listenAddr ) );
		if ( ret < 0 ) {
			CErrArg ( "MediatorUdpThread: Failed to bind listener socket to port [ %i ]", port );
			LogSocketError ();
			return 0;
		}
		else {
			CVerbArg ( "MediatorUdpThread bound socket to port [ %i ]", port );
		}

		struct 	sockaddr_in		addr;
		socklen_t 				addrLen = sizeof ( addr );
		int						length;

		CLogArg ( "MediatorUdpThread listen on port [ %i ]", port );


		while ( isRunning )
		{
			client.reset ();

			memset ( &addr, 0, sizeof ( addr ) );

			int bytesReceived = ( int ) recvfrom ( sock, buffer, 1024, 0, ( sockaddr * ) &addr, &addrLen );

			if ( bytesReceived <= 0 ) {
				CLogArg ( "Udp: Socket on port [ %i ] has been closed.", port );
				break;
			}

			UdpEncHelloPacket * hp = ( UdpEncHelloPacket * ) buffer;

			msgLength = hp->size;
			unsigned flags = 0xF0000000 & msgLength;

			/// Message is encrypted                
			if ( flags & 0x80000000 )
			{
				msgLength &= 0xFFFFFFF;
				if ( msgLength > ( unsigned int ) bytesReceived ) {
					CWarnArg ( "Udp: Message size [ %u ] is larger than bytes received.", msgLength );
					continue;
				}

				client = GetSessionClient ( hp->sessionID );
				if ( !client ) {
					CWarnArg ( "Udp: Requested device session does not exist [ %u ].", ( unsigned int ) hp->sessionID );
					continue;
				}
				msgDecLength = msgLength - sizeof ( UdpEncHelloPacket );

				/// Message is AES encrypted
				if ( !AESDecrypt ( &client->aes, &hp->aes, &msgDecLength, &decrypted ) )
					goto Continue;
				msgDec = decrypted;
			}
			else {
				msgDecLength = bytesReceived;

				// Ignore unencrypted messages

				//msgDec = buffer;

				continue;
			}


			CVerbArg ( "Udp: Received %d bytes from IP [ %s : %d ]", msgDecLength, inet_ntoa ( addr.sin_addr ), ntohs ( addr.sin_port ) );

			if ( msgDec [ 0 ] == MEDIATOR_STUN_REQUEST && msgDec [ 1 ] == MEDIATOR_OPT_NULL && msgDec [ 2 ] == MEDIATOR_OPT_NULL && msgDec [ 3 ] == MEDIATOR_OPT_NULL )
			{
				STUNReqHeader * packet = ( STUNReqHeader * ) msgDec;

				int sourceID = packet->sourceID;
				int destID = packet->destID;

				if ( !destID ) {
					CErr ( "Udp: Invalid target deviceID 0" );
					goto Continue;
				}

				// Find source device and check validity of IP
				sp ( ThreadInstance ) clientSP;
				sp ( DeviceInstanceNode ) sourceDeviceSP;

				if ( bytesReceived >= ( int ) sizeof ( STUNReqHeader ) )
				{
					msgDec [ msgDecLength ] = 0;
					const char * areaName = 0;
					const char * appName = 0;

					if ( client && client->version >= '5' )
					{
						sourceDeviceSP = client->deviceSP;
						if ( !sourceDeviceSP ) {
							CWarn ( "Udp: Source device does not exist." );
							goto Continue;
						}

						if ( packet->sizes [ 0 ] > 1 && packet->sizes [ 1 ] > 1 && packet->sizes [ 0 ] < MAX_NAMEPROPERTY && packet->sizes [ 1 ] < MAX_NAMEPROPERTY )
						{
							appName = ( ( STUNReqPacket * ) packet )->appArea;
							areaName  = appName + packet->sizes [ 0 ];
						}

						if ( !appName || !areaName || !*appName || !*areaName )
						{
							appName = sourceDeviceSP->info.appName;
							areaName = sourceDeviceSP->info.areaName;
						}
					}
					else {
						STUNReqPacketV4 * stun = ( STUNReqPacketV4 * ) packet;
						appName = stun->appName;
						areaName = stun->areaName;
					}

					DeviceInstanceNode * device;
					ApplicationDevices * appDevs;
					sp ( ApplicationDevices ) appDevices	= GetApplicationDevices ( appName, areaName );

                    if ( !appDevices || !appDevices->LockRead ( "Udp" ) )
                    {
						UnlockApplicationDevices ( appDevices.get () );
						goto Continue;
					}

					appDevs = appDevices.get ();

					device = GetDeviceInstance ( destID, appDevs->devices );
					if ( device ) {
						clientSP = device->clientSP;
					}
					else {
						CErrArg ( "Udp: requested STUN device [ %i ] does not exist", destID );
					}

                    appDevs->UnlockRead ( "Udp" );

					UnlockApplicationDevices ( appDevs );

					if ( !clientSP ) {
						CErrArg ( "Udp: requested client of STUN device [ %i ] does not exist", destID );
						goto Continue;
					}

					// Reply with mediator ack message to establish the temporary route entry in requestor's NAT
					length	= ( int ) sendto ( sock, "y;;-    ", 8, 0, ( const sockaddr * ) &addr, sizeof ( struct sockaddr_in ) );
					if ( length != 8 ) {
						CErrArg ( "Udp: Failed to send UDP ACK to device [ %i ]", sourceID );
					}

					if ( client && client->version >= '5' )
					{
						if ( appDevs == sourceDeviceSP->rootSP.get () ) {
							appName = 0;
							areaName = 0;
						}
						else {
							appName = sourceDeviceSP->info.appName;
							areaName = sourceDeviceSP->info.areaName;
						}
					}
					else {
						STUNReqPacketV4 * stun = ( STUNReqPacketV4 * ) packet;

						areaName = stun->areaName;
						if ( *stun->areaNameSrc )
							areaName = stun->areaNameSrc;

						appName = stun->appName;
						if ( *stun->appNameSrc )
							appName = stun->appNameSrc;
					}

					HandleSTUNRequest ( clientSP.get (), sourceID, areaName, appName, ( unsigned int ) addr.sin_addr.s_addr, ntohs ( addr.sin_port ) );
				}
			}

		Continue:
			free_m ( decrypted );
		}

		if ( decrypted ) free ( decrypted );

		CLogArg ( "MediatorUdpThread: Thread for port [ %d ] terminated.", port );

		return 0;
	}


	void * MediatorDaemon::AcceptorStarter ( void * arg )
	{
		if ( arg ) {
			ThreadInstance * listener = ( ThreadInstance * ) arg;
			if ( listener->daemon ) {
				// Execute thread
				( ( MediatorDaemon * ) listener->daemon )->Acceptor ( arg );

				listener->thread.Notify ( "AcceptorStarter" );
			}
		}
		return 0;
	}


	void * MediatorDaemon::Acceptor ( void * arg )
	{
		CVerb ( "Acceptor started." );

		if ( !arg ) {
			CErr ( "Acceptor: Called with (NULL) argument." );
			return 0;
		}

		//pthread_attr_t attr;
		struct 	sockaddr_in		listenAddr;
		ThreadInstance *        listener    = ( ThreadInstance * ) arg;

		unsigned short port = listener->port;

		CVerbArg ( "Acceptor for port %d started.", port );

		// Send started signal
		if ( !listener->thread.Notify ( "Acceptor" ) ) {
			CErr ( "Acceptor: Error to signal event" );
		}

		memset ( &listenAddr, 0, sizeof ( listenAddr ) );

		listenAddr.sin_family		= AF_INET;
		listenAddr.sin_addr.s_addr	= INADDR_ANY;
		listenAddr.sin_port			= htons ( port );

		int ret = ::bind ( ( int ) listener->socket, ( struct sockaddr * )&listenAddr, sizeof ( listenAddr ) );
		if ( ret < 0 ) {
			CErrArg ( "Acceptor: Failed to bind listener socket to port [ %i ]!", port );
			LogSocketError ();
			return 0;
		}
		CVerbArg ( "Acceptor bound to port [ %i ]", port );

		ret = listen ( ( int ) listener->socket, SOMAXCONN );
		if ( ret < 0 ) {
			CErrArg ( "Acceptor: Failed to listen on socket to port [ %i ]!", port );
			LogSocketError ();
			return 0;
		}
		CLogArg ( "Acceptor listen on port [ %i ]", port );

		struct 	sockaddr_in		addr;
		socklen_t 				addrLen = sizeof ( addr );

		while ( isRunning )
		{
			Zero ( addr );

			int sock = ( int ) accept ( ( int ) listener->socket, ( struct sockaddr * )&addr, &addrLen );

			if ( sock < 0 ) {
				CLogArg ( "Acceptor: Socket [ %i ] on port [ %i ] has been closed!", sock, port );
				break;
            }
            CSocketTraceAdd ( sock, "Acceptor" );

			CLog ( "\n" );
			const char * ips = inet_ntoa ( addr.sin_addr );

			CLogArg ( "Acceptor [ %i ]: Connection with IP [ %s : %d ]", sock, ips, ntohs ( addr.sin_port ) );
			CLog ( "\n" );

			if ( acceptEnabled )
			{
				sp ( ThreadInstance ) clientSP = make_shared < ThreadInstance > ();
				if ( !clientSP ) {
					CErr ( "Acceptor: Failed to allocate memory for client request!" );
					goto NextClient;
				}
                
                ThreadInstance * client = clientSP.get ();
				//memset ( client.get (), 0, sizeof ( ThreadInstance ) - ( sizeof ( sp ( DeviceInstanceNode ) ) + sizeof ( sp ( ThreadInstance ) ) ) );

				if ( !client->Init () )
					goto NextClient;
                
                client->socketToClose   = sock;
				client->socket          = sock;
				client->port            = port;
				client->filterMode      = 2;
				client->aliveLast       = checkLast;
				client->daemon          = this;
				client->connectTime     = GetEnvironsTickCount ();

				memcpy ( &client->addr, &addr, sizeof ( addr ) );

				strlcpy ( client->ips, ips, sizeof ( client->ips ) );


				client->thread.autoreset = false;
				client->thread.ResetSync ( "Acceptor", false );

				client->clientSP = clientSP;

				// Create client thread
				if ( !client->thread.Run ( pthread_make_routine ( &MediatorDaemon::ClientThreadStarter ), ( void * ) client, "Acceptor" ) )
				{
					client->socket			= INVALID_FD;
					client->socketToClose	= INVALID_FD;
					client->clientSP		= 0;
				}
				else {
					sock = INVALID_FD;

					if ( acceptClients.Lock ( "Acceptor" ) )
					{
						client->inAcceptorList = true;

						acceptClients.list [ client ] = clientSP;

						acceptClients.Unlock ( "Acceptor" );
					}
				}
			}

		NextClient:
			if ( sock != -1 ) {
				CSocketTraceRemove ( sock, "Acceptor: Closing", 0 );

				shutdown ( sock, 2 ); closesocket ( sock );
			}
		}

		CLogArg ( "Acceptor: Thread for port [ %d ] terminated.", port );

		return 0;
	}


	bool MediatorDaemon::AddToArea ( const char * area, const char * app, const char * pKey, const char * pValue )
	{
		bool ret = false;

		// Search for the area at first
		sp ( ListValues )			values	= 0;
		sp ( AppsList )             appsList;

		if ( !area || !app ) {
			CErr ( "AddToArea: Invalid parameters. Missing area/app name!" );
			return false;
		}

		if ( !areasMap.Lock ( "AddToArea" ) )
			return false;

		string areaName ( area );
		string appName ( app );

		const msp ( string, AppsList )::iterator areaIt = areasMap.list.find ( areaName );

		if ( areaIt == areasMap.list.end () )
		{
			appsList = make_shared < AppsList > ();
			if ( !appsList || !appsList->Init () ) {
				CErrArg ( "AddToArea: Failed to create new area [%s].", area );
				return false;
			}
			areasMap.list [ areaName ] = appsList;
		}
		else
			appsList = areaIt->second;

		if ( !areasMap.Unlock ( "AddToArea" ) )
			return false;

		if ( !appsList || !appsList->Lock ( "AddToArea" ) )
			return false;

		const msp ( string, ListValues )::iterator appsIt = appsList->apps.find ( appName );

		if ( appsIt == appsList->apps.end () )
		{
			values = make_shared < ListValues > (); //new map<string, ValuePack*> ();
			if ( !values || !values->Init () ) {
				CErrArg ( "AddToArea: Failed to create new application [%s].", appName.c_str () );
				return false;
			}
			appsList->apps [ appName ] = values;
		}
		else
			values = appsIt->second;

		appsList->Unlock ( "AddToArea" );

		if ( values && values->Lock ( "AddToArea" ) ) {
			ret = AddToArea ( values, pKey, pValue, ( unsigned int ) strlen ( pValue ) );

			values->Unlock ( "AddToArea" );
		}

		//EndWithStatus:
		return ret;
	}


	bool MediatorDaemon::AddToArea ( sp ( ListValues ) &values, const char * pKey, const char * pValue, unsigned int valueSize )
	{
		bool ret = false;

		if ( !values || !pKey || !pValue || valueSize <= 0 ) {
			CErr ( "AddToArea: Invalid parameters. Missing key/value!" );
			return false;
		}

		sp ( ValuePack ) pack = 0;
		string key = pKey;

		// Look whether we already have a value for the key
		const msp ( string, ValuePack )::iterator valueIt = values->values.find ( key );

		if ( valueIt != values->values.end () )
			pack = valueIt->second;

		// Update the old value
		if ( pack ) {
			pack->timestamp = std::time ( 0 );
			pack->value = string ( pValue );
			pack->size = valueSize;
		}
		else
		{
			pack = make_shared < ValuePack > (); // new ValuePack ();
			if ( !pack ) {
				CErrArg ( "AddToArea: Failed to create new value object for %s! Memory low problem!", pKey );
				goto EndWithStatus;
			}
			pack->timestamp = std::time ( 0 );
			pack->value = string ( pValue );
			pack->size = valueSize;

			values->values [ key ] = pack;
		}

		ret = true;

		//CVerbArg ( "addToArea: added %s -> %s", pKey, pValue );

	EndWithStatus:
		return ret;
	}


	bool MediatorDaemon::sendDatabase ( int sock, struct sockaddr * addr )
	{
		return true;
	}


	bool endsWith ( std::string const &fullString, std::string const &ending )
	{
		if ( fullString.length () >= ending.length () ) {
			return ( 0 == fullString.compare ( fullString.length () - ending.length (), ending.length (), ending ) );
		}
		else {
			return false;
		}
	}


	bool MediatorDaemon::HandleRequest ( ThreadInstance * client, char * buffer )
	{
		bool success = false;

		MediatorGetPacket * query = ( MediatorGetPacket * ) buffer;

		unsigned int msgLength = query->size;

		CLogArg ( "HandleRequest [ %i ]: [ %s ]", client->socket, buffer + 4 );

		// Scan parameters
		char empty [ 2 ] = { 0 };

		char * params [ 4 ];
		memset ( params, 0, sizeof ( params ) );

		if ( !ScanForParameters ( buffer + sizeof ( MediatorGetPacket ), msgLength - sizeof ( MediatorGetPacket ), ";", params, 4 ) ) {
			CErrArg ( "HandleRequest [ %i ]: Invalid parameters.", client->socket );
			return false;
		}

		if ( !params [ 0 ] || !params [ 1 ] || !params [ 2 ] ) {
			CErrArg ( "HandleRequest [ %i ]: Invalid parameters. area and/or key missing!", client->socket );
			return false;
		}

		sp ( DeviceInstanceNode ) deviceSP = client->deviceSP;

		if ( *( params [ 0 ] ) == ' ' ) {
			/// Use area name of the client
			params [ 0 ] = deviceSP ? deviceSP->info.areaName : empty;
		}

		if ( *( params [ 1 ] ) == ' ' ) {
			/// Use app name of the client
			params [ 1 ] = deviceSP ? deviceSP->info.appName : empty;
		}

		// Search for the area at first
		sp ( ListValues )	values	= 0;
		sp ( AppsList )		apps    = 0;

		if ( !areasMap.Lock ( "HandleRequest" ) )
			return false;

		string areaName ( params [ 0 ] );

		const msp ( string, AppsList )::iterator areaIt = areasMap.list.find ( areaName );

		// COMMAND: Set New Value
		if ( query->cmd == MEDIATOR_CMD_SET )
		{
			if ( areaIt == areasMap.list.end () )
			{
				// Create a new map for the area
				apps = make_shared < AppsList > ();
				if ( !apps || !apps->Init () ) {
					CErrArg ( "HandleRequest [ %i ]: Failed to create new area [ %s ].", client->socket, params [ 0 ] );
					return false;
				}
				areasMap.list [ areaName ] = apps;
			}
			else
				apps = areaIt->second;

			areasMap.Unlock ( "HandleRequest" );

			if ( !apps || !apps->Lock ( "HandleRequest" ) )
				return false;

			const msp ( string, ListValues )::iterator appsIt = apps->apps.find ( string ( params [ 1 ] ) );
			if ( appsIt == apps->apps.end () )
			{
				string appName ( params [ 1 ] );

				values = make_shared < ListValues > (); // new map<string, ValuePack*> ();
				if ( !values || !values->Init () ) {
					CErrArg ( "HandleRequest [ %i ]: Failed to create new application [ %s ].", client->socket, params [ 1 ] );
					return false;
				}
				apps->apps [ appName ] = values;
			}
			else
				values = appsIt->second;

			apps->Unlock ( "HandleRequest" );

			if ( !values || !params [ 2 ] || !params [ 3 ] ) {
				CErrArg ( "HandleRequest [ %i ]: Invalid parameters. Missing key for [ %s ]", client->socket, params [ 1 ] );
				return false;
			}

			char * key = params [ 2 ];
			if ( endsWith ( string ( key ), string ( "_push" ) ) ) {
				int len = ( int ) strlen ( key );
				key [ len - 5 ] = 0;
				int clientID = -1;
				sscanf_s ( key, "%d", &clientID );
				if ( clientID > 0 ) {
					CLogArg ( "HandleRequest [ %i ]: sending push notification to client [ %i: %s ]", client->socket, clientID, params [ 3 ] );
					/*
					if ( !SendPushNotification ( values.get(), clientID, params [ 2 ] ) ) {
					CErrArg ( "HandleRequest: sending push notification to client [%i] failed. [%s]", clientID, params [ 3 ] );
					}
					else success = true;
					*/
				}
				return false;
			}

			int valueSize = ( int ) ( msgLength - ( params [ 3 ] - buffer ) );
			if ( valueSize <= 0 ) {
				CErrArg ( "HandleRequest [ %i ]: size of value of [ %i ] invalid!", client->socket, valueSize );
				return false;
			}

			if ( !values->Lock ( "HandleRequest" ) )
				return false;

			if ( !AddToArea ( values, params [ 2 ], params [ 3 ], ( unsigned ) valueSize ) ) {
				CErrArg ( "HandleRequest [ %i ]: Adding key [ %s ] failed!", client->socket, params [ 2 ] );
				success = false;
			}

			if ( !values->Unlock ( "HandleRequest" ) )
				return false;

			if ( success )
				CLogArg ( "HandleRequest [ %i ]: [ %s / %s ] +key [ %s ] value [ %s ]", client->socket, params [ 0 ], params [ 1 ], params [ 2 ], params [ 3 ] );
			return success;
		}

		// Look whether we already have a value for the key
		if ( query->cmd == MEDIATOR_CMD_GET )
		{
			const char * response = "---";
			size_t length	= strlen ( response );

			if ( areaIt != areasMap.list.end () )
				apps = areaIt->second;

			areasMap.Unlock ( "HandleRequest" );

			if ( !apps )
				return false;

			sp ( ValuePack ) value = 0;

			const msp ( string, ListValues )::iterator appsIt = apps->apps.find ( string ( params [ 1 ] ) );
			if ( appsIt != apps->apps.end () ) {
				values = appsIt->second;

				if ( !values || !values->Lock ( "HandleRequest" ) )
					return false;

				const msp ( string, ValuePack )::iterator valueIt = values->values.find ( string ( params [ 2 ] ) );

				if ( valueIt != values->values.end () )
					value = valueIt->second;

				values->Unlock ( "HandleRequest" );

				if ( !value )
					return false;

				response = value->value.c_str ();
				length = value->size;
			}

			if ( length > 0 ) {
				// Add 4 bytes for the message size
				length += 14;

				char * sendBuffer = ( char * ) malloc ( length );
				if ( !sendBuffer ) {
					CErrArg ( "HandleRequest [ %i ]: Failed to allocate buffer of size [%u] for sending the requested value.", client->socket, ( unsigned int ) length );
					return false;
				}
                
                unsigned int seqNr  = 0;
                unsigned int * ptr = reinterpret_cast<unsigned int *>( sendBuffer );
                
				*ptr = ( unsigned int ) length;
                
                if ( client->version < '6' ) {
                    memcpy ( sendBuffer + 4, response, length - 5 );
                    sendBuffer [ length - 1 ] = 0;
                    
                    length -= 9;
                }
                else {
                    
                    MediatorGetPacketV6 * q = ( MediatorGetPacketV6 * ) buffer;
                    
                    seqNr  = q->seqNr;
                    client->seqNr       = seqNr;
                    
                    ptr += 2;
                    *ptr = seqNr;
                    
                    ptr++;
                    memcpy ( ptr, response, length - 14 );
                    sendBuffer [ length - 1 ] = 0;
                }
                
                if ( !SendBufferOrEnqueue ( client, sendBuffer, ( unsigned int ) length, false, seqNr ) ) {
                    CErrArg ( "HandleRequest [ %i ]: Failed to response value [%s] for key [%s]", client->socket, response, params [ 1 ] );
                    free ( sendBuffer );
                }
                else success = true;
            }

			return success;
		}

		areasMap.Unlock ( "HandleRequest" );

		CWarnArg ( "HandleRequest [ %i ]: command [ %c ] not supported anymore", client->socket, query->cmd );
		return false;
	}


	sp ( ThreadInstance ) MediatorDaemon::GetSessionClient ( long long sessionID )
	{
		CVerbVerbArg ( "GetSessionClient [%d]", sessionID );

		sp ( ThreadInstance ) client = 0;

		sessions.Lock ( "GetSessionClient" );

		const msp ( long long, ThreadInstance )::iterator sessionIt = sessions.list.find ( sessionID );
        
		if ( sessionIt != sessions.list.end () )
		{
			client = sessionIt->second;
		}

		sessions.Unlock ( "GetSessionClient" );

		CVerbVerbArg ( "GetSessionClient: [%s]", client ? client->uid : "NOT found" );

		return client;
	}


	int MediatorDaemon::GetNextDeviceID ( char * areaName, char * appName, unsigned int ip )
	{
		CVerb ( "GetNextDeviceID" );

		sp ( ApplicationDevices ) appDevices;
		int nextID = 0;

		DeviceInstanceNode	**	deviceList;

		appDevices = GetDeviceList ( areaName, appName, 0, 0, deviceList );
		if ( appDevices )
        {
            DEBUG_CHECK_START ();

            if ( appDevices->LockRead ( "GetNextDeviceID" ) )
			{
				/// Find the next free deviceID
				DeviceInstanceNode	* device = appDevices->devices;

				if ( !nextID )
					nextID = ( int ) __sync_add_and_fetch ( &appDevices->latestAssignedID, 1 );

				while ( device )
				{
					if ( device->info.deviceID > appDevices->latestAssignedID ) {
						___sync_test_and_set ( &appDevices->latestAssignedID, device->info.deviceID );
					}

					if ( device->info.deviceID == nextID )
						nextID = ( int ) __sync_add_and_fetch ( &appDevices->latestAssignedID, 1 );
					else {
						if ( device->info.deviceID > nextID )
							break;
					}

					device = device->next;
				}

                appDevices->UnlockRead  ( "GetNextDeviceID" );
			}

            UnlockApplicationDevices ( appDevices.get () );
            
            DEBUG_CHECK_MEASURE ( "GetNextDeviceID" );
		}

		if ( !nextID ) {
			srand ( ( unsigned ) GetEnvironsTickCount () );

			int randID = 0;
			while ( randID == 0 ) {
				randID = rand () % 0xFFFFFF;
			}

			randID <<= 8;
			randID |= ( ip & 0xFF );

			nextID = randID;
		}

		return nextID;
	}


	int MediatorDaemon::HandleRegistration ( int &deviceID, const sp ( ThreadInstance ) &client, unsigned int bytesLeft, char * msg, unsigned int msgLen )
	{
		if ( !client->version )
			client->version = msg [ 16 ];

		if ( client->version < '5' )
			return HandleRegistrationV4 ( deviceID, client, bytesLeft, msg, msgLen );

		CVerbArg ( "HandleRegistration [ %s : %i ]", client->ips, client->socket );

		int ret = -1;
		char * decrypted = 0;

		if ( msg [ 7 ] == 'S' ) {
			do
			{
				bool					useSession	= true;
				sp ( ThreadInstance )	relClient	= 0;
				StuntSockDecPack	*	regPack		= ( StuntSockDecPack * ) msg;

				/// Get the size of the registration packet
				unsigned int regLen =  0;

				if ( regPack->sizeReq >= 16 )
				{
					regLen =  regPack->sizePayload;
					if ( regLen + 12 > bytesLeft ) {
						CWarnArg ( "HandleRegistration [ %s : %i ]:\tStunt socket registration packet overflow.", client->ips, client->socket ); break;
					}

					relClient = GetSessionClient ( regPack->sessionID );
				}
				else {
					useSession = false;

					regLen = *( ( unsigned int * ) ( msg + 12 ) );
					if ( regLen + 12 > bytesLeft ) {
						CWarnArg ( "HandleRegistration [ %s : %i ]:\tStunt socket registration packet overflow.", client->ips, client->socket ); break;
					}

					/// Get the stunt id
					unsigned int stuntIDa = *( ( unsigned int * ) ( msg + 8 ) );

					if ( !stuntClients.Lock ( "HandleRegistration" ) )
						break;

					const msp ( long long, ThreadInstance )::iterator iter = stuntClients.list.find ( stuntIDa );
					if ( iter == stuntClients.list.end () ) {
						CLogArg ( "HandleRegistration [ %s : %i ]:\tStunt id [ %u ] not found.", client->ips, client->socket, stuntIDa );

						stuntClients.Unlock ( "HandleRegistration" );
						break;
					}

					relClient = iter->second;
					stuntClients.list.erase ( iter );

					if ( !stuntClients.Unlock ( "HandleRegistration" ) )
						break;
				}

				if ( !relClient ) {
					CWarnArg ( "HandleRegistration [ %s : %i ]:\tStunt socket client does not exist.", client->ips, client->socket ); break;
				}

				//CLogArg ( "HandleRegistration: Encrypted [%s]", ConvertToHexSpaceString ( msg + 16, regLen ) );

				if ( !AESDecrypt ( &relClient->aes, useSession ? &regPack->payload : ( msg + 16 ), &regLen, &decrypted ) )
					break;

				HandleStuntSocketRegistration ( client.get (), relClient, decrypted + 4, regLen );
				CVerbArg ( "HandleRegistration [ %s : %i ]:\tClosing stunt socket reg. thread [ 0x%X ].", client->ips, client->socket, relClient->deviceID );
				
				client->authenticated = 1;
				ret = 0;
			}
			while ( 0 );
		}
		else if ( msg [ 7 ] == MEDIATOR_OPT_DEVICE_LIST_DEVICE_ID ) {
			CLogArg ( "HandleRegistration [ %s : %i ]:\tAssigning next device ID.", client->ips, client->socket );

			msg [ msgLen ] = 0;
			MediatorReqHeader	* req = ( MediatorReqHeader * ) msg;
			MediatorReqMsg      * raw = ( MediatorReqMsg * ) msg;

			char * deviceUID = raw->deviceUID;

			int nextID = 0;
			int mappedID = 0;

			if ( req->sizes [ 1 ] < sizeof ( raw->deviceUID ) && deviceMappings.Lock ( "HandleRegistration" ) )
			{
				sp ( DeviceMapping ) mapping;

				if ( *deviceUID ) {
					CVerbArg ( "HandleRegistration [ %s : %i ]:\tLooking for a mapping to deviceUID [ %s ].", client->ips, client->socket, raw->deviceUID );

					const msp ( string, DeviceMapping )::iterator devIt = deviceMappings.list.find ( string ( raw->deviceUID ) );

					if ( devIt != deviceMappings.list.end () )
					{
						mapping = devIt->second;
						nextID  = mappedID = mapping->deviceID;

						CVerbArg ( "HandleRegistration [ %s : %i ]:\tMapping found deviceID [ 0x%X ] authLevel [ %i ] authToken [...].", client->ips, client->socket, nextID, mapping->authLevel );
					}
				}

				unsigned char * sizes = req->sizes + req->sizes [ 0 ] + req->sizes [ 1 ] + 2;

				char * appName  = ( char * ) ( sizes + 2 );
				char * areaName = appName + sizes [ 0 ];

				if ( !mapping ) {
					CVerbArg ( "HandleRegistration [ %s : %i ]:\tNo mapping found. Creating new.", client->ips, client->socket );

					mapping.reset ( new DeviceMapping );

					if ( !mapping )
						goto PreFailExit;
					memset ( mapping.get (), 0, sizeof ( DeviceMapping ) );
				}

				if ( !*appName || !*areaName || sizes [ 0 ] > MAX_NAMEPROPERTY || sizes [ 1 ] > MAX_NAMEPROPERTY || sizes [ 0 ] <= 1 || sizes [ 1 ] <= 1 )
					goto PreFailExit;

				nextID = GetNextDeviceID ( areaName, appName, ( unsigned int ) client->addr.sin_addr.s_addr );

				if ( nextID && ( nextID != mappedID ) ) {
					mapping->deviceID = nextID;

					CVerbArg ( "HandleRegistration [ %s : %i ]:\tCreated a mapping with deviceID [ 0x%X ].", client->ips, client->socket, nextID );

					deviceMappings.list [ string ( raw->deviceUID ) ] = mapping;
				}

			PreFailExit:
				deviceMappings.Unlock ( "HandleRegistration" );

				if ( !mapping ) {
					CErrArg ( "HandleRegistration [ %s : %i ]:\tFailed to find or create a mapping.", client->ips, client->socket );
					return false;
				}

				if ( nextID )
					deviceMappingDirty = true;
			}

			CLogArg ( "HandleRegistration [ %s : %i ]:\tAssigning device ID  [%u ] to [ %s ].", client->ips, client->socket, nextID, *raw->deviceUID ? raw->deviceUID : "Unknown" );

			SendBuffer ( client.get (), &nextID, sizeof ( nextID ) );
			return 1;
		}
		else {
			/// Message seems to be decrypted already

			deviceID = *( ( int * ) ( msg + 16 ) );

			CLogArgID ( "HandleRegistration [ %s : %i ]:\t[ %s %c %c %c ]", client->ips, client->socket, GetCommandString ( msg [ 5 ] ), msg [ 4 ], msg [ 6 ], msg [ 7 ] );

			/*if ( msgLen == MEDIATOR_BROADCAST_SPARE_ID_LEN ) {
			HandleStuntSocketRegistration ( client, deviceID );
			CVerbID ( "HandleRegistration: Closing stunt socket reg. thread." );
			return 0;
			}
			else */
			if ( msgLen >= ( MEDIATOR_BROADCAST_DESC_START + 3 ) ) {
				client->deviceID = deviceID;

				if ( !HandleDeviceRegistration ( client, ( unsigned int ) client->addr.sin_addr.s_addr, msg + 4 ) ) {
					CWarnArgID ( "HandleRegistration [ %s : %i ]:\tDevice registration failed.", client->ips, client->socket );
				}
				else {
					CVerbArgID ( "HandleRegistration [ %s : %i ]:\tSending socket buffer sizes.", client->ips, client->socket );

#ifdef MEDIATOR_USE_SOCKET_BUFFERS_APPLY_AT_CLIENT
					char buffer [ 20 ];

					int recSize = 0;
					int sendSize = 0;

					socklen_t retSize = sizeof ( recSize );

					// - Load send buffer size
					int rc = getsockopt ( ( int ) client->socket, SOL_SOCKET, SO_RCVBUF, ( char * ) &recSize, &retSize );
					if ( rc < 0 ) {
						CErrArgID ( "HandleRegistration [ %s : %i ]: Failed to query receive buffer size!", client->ips, client->socket ); LogSocketError ();
					}
					else {
						CVerbArgID ( "HandleRegistration [ %s : %i ]: receive buffer size [ %i ]", client->ips, client->socket, recSize );
					}

					// - Load send buffer size
					retSize = sizeof ( sendSize );

					rc = getsockopt ( ( int ) client->socket, SOL_SOCKET, SO_SNDBUF, ( char * ) &sendSize, &retSize );
					if ( rc < 0 ) {
						CErrArgID ( "HandleRegistration [ %s : %i ]: Failed to query send buffer size!", client->ips, client->socket ); LogSocketError ();
					}
					else {
						CVerbArgID ( "HandleRegistration [ %s : %i ]: send buffer size [ %i ]", client->ips, client->socket, sendSize );
					}

					if ( sendSize && recSize )
					{
						int * pUI = reinterpret_cast<int *>( buffer );
						*pUI = MEDIATOR_MSG_SOCKET_BUFFERS_SIZE;

						buffer [ 4 ] = MEDIATOR_PROTOCOL_VERSION;
						buffer [ 5 ] = MEDIATOR_CMD_SET_SOCKET_BUFFERS;
						buffer [ 6 ] = ';';
						buffer [ 7 ] = ';';

						pUI += 2;

						*pUI = recSize; pUI++;
						*pUI = sendSize;

						if ( !SendBuffer ( client.get (), buffer, MEDIATOR_MSG_SOCKET_BUFFERS_SIZE ) ) {
							CErrArgID ( "HandleRegistration [ %s : %i ]: Failed to inform about socket buffer sizes!", client->ips, client->socket );
						}
					}
#endif
                    int s = ( int ) client->socket;
                    SocketTimeout ( s, -1, 0 );

#if !defined(ENABLE_WINSOCK_SEND_THREADS) && !defined(ENABLE_WINSOCK_CLIENT_THREADS)
					if ( !SetNonBlockSocket ( s , true, "HandleRegistration" ) )
					{
						CWarnArgID ( "HandleRegistration [ %s : %i ]:\tDevice registration failed.", client->ips, client->socket );
						return false;
					}
#endif
                    // Look for a client thread to assign to
                    unsigned int clientToMap = 0;
                    int fewest = -1;
                    
                    for ( unsigned int i = 0; i < clientThreadCount; ++i )
                    {
                        clientContexts [ i ]->Lock ( "HandleRegistration" );

						int size = ( int ) clientContexts [ i ]->clients.size ();

						if ( fewest < 0 || size < fewest ) {
							fewest = size;
							clientToMap = i;
						}
                        
                        clientContexts [ i ]->Unlock ( "HandleRegistration" );
                    }
                    
                    if ( !clientContexts [ clientToMap ]->Add ( client ) ) {
                        return 0;
                    }

#ifdef ENABLE_WINSOCK_CLIENT_THREADS
#	ifndef USE_ONE_TO_ONE_SOCKET_EVENTS
					if ( WSAEventSelect ( ( int ) client->socket, clientContexts [ clientToMap ]->revent, FD_READ | FD_CLOSE ) == SOCKET_ERROR ) {
						CWarnArgID ( "HandleRegistration [ %s : %i ]:\tFailed to register receive event [ %d ]", client->ips, client->socket, WSAGetLastError () );
						return 0;
					}
#	endif
#endif                    
                    clientEvent.Notify ( "HandleRegistration" );

					CLogArgID ( "HandleRegistration [ %s : %i ]:\tDevice registration succeeded.", client->ips, client->socket );

					return 1;
				}
			}
		}

		if ( decrypted ) free ( decrypted );

		return ret;
	}


	int MediatorDaemon::HandleRegistrationV4 ( int &deviceID, const sp ( ThreadInstance ) &client, unsigned int bytesLeft, char * msg, unsigned int msgLen )
	{
		CVerbArg ( "HandleRegistration [ %s : %i ]", client->ips, client->socket );

		int ret = -1;
		char * decrypted = 0;

		if ( msg [ 7 ] == 'S' ) {
			do
			{
				bool					useSession	= true;
				sp ( ThreadInstance )	relClient	= 0;
				StuntSockDecPack	*	regPack		= ( StuntSockDecPack * ) msg;

				/// Get the size of the registration packet
				unsigned int regLen =  0;

				if ( regPack->sizeReq >= 16 )
				{
					regLen =  regPack->sizePayload;
					if ( regLen + 12 > bytesLeft ) {
						CWarnArg ( "HandleRegistration [ %s : %i ]:\tStunt socket registration packet overflow.", client->ips, client->socket ); break;
					}

					relClient = GetSessionClient ( regPack->sessionID );
				}
				else {
					useSession = false;

					regLen = *( ( unsigned int * ) ( msg + 12 ) );
					if ( regLen + 12 > bytesLeft ) {
						CWarnArg ( "HandleRegistration [ %s : %i ]:\tStunt socket registration packet overflow.", client->ips, client->socket ); break;
					}

					/// Get the stunt id
					unsigned int stuntIDa = *( ( unsigned int * ) ( msg + 8 ) );

					if ( !stuntClients.Lock ( "HandleRegistration" ) )
						break;

					const msp ( long long, ThreadInstance )::iterator iter = stuntClients.list.find ( stuntIDa );
					if ( iter == stuntClients.list.end () ) {
						CLogArg ( "HandleRegistration [ %s : %i ]:\tStunt id [%u] not found.", client->ips, client->socket, stuntIDa );

						stuntClients.Unlock ( "HandleRegistration" );
						break;
					}

					relClient = iter->second;
					stuntClients.list.erase ( iter );

					if ( !stuntClients.Unlock ( "HandleRegistration" ) )
						break;
				}

				if ( !relClient ) {
					CWarnArg ( "HandleRegistration [ %s : %i ]:\tStunt socket client does not exist.", client->ips, client->socket ); break;
				}

				//CLogArg ( "HandleRegistration: Encrypted [%s]", ConvertToHexSpaceString ( msg + 16, regLen ) );

				if ( !AESDecrypt ( &relClient->aes, useSession ? &regPack->payload : ( msg + 16 ), &regLen, &decrypted ) )
					break;

				if ( regLen != MEDIATOR_BROADCAST_SPARE_ID_LEN ) {
					CWarnArg ( "HandleRegistration [ %s : %i ]:\tStunt socket registration packet is not of correct type.", client->ips, client->socket ); break;
				}

				HandleStuntSocketRegistration ( client.get (), relClient, decrypted + 4, regLen );
				CVerbArg ( "HandleRegistration [ %s : %i ]:\tClosing stunt socket reg. thread [0x%X].", client->ips, client->socket, relClient->deviceID );

				ret = 0;
			}
			while ( 0 );
		}
		else if ( msg [ 7 ] == MEDIATOR_OPT_DEVICE_LIST_DEVICE_ID ) {
			CLogArg ( "HandleRegistration [ %s : %i ]:\tAssigning next device ID.", client->ips, client->socket );

			msg [ msgLen ] = 0;
			MediatorReqMsgV4	* req = ( MediatorReqMsgV4 * ) msg;

			int nextID = 0;
			int mappedID = 0;

			if ( deviceMappings.Lock ( "HandleRegistration" ) )
			{
				sp ( DeviceMapping ) mapping;

				if ( *req->deviceUID ) {
					CVerbArg ( "HandleRegistration [ %s : %i ]:\tLooking for a mapping to deviceUID [%s].", client->ips, client->socket, req->deviceUID );

					const msp ( string, DeviceMapping )::iterator devIt = deviceMappings.list.find ( string ( req->deviceUID ) );

					if ( devIt != deviceMappings.list.end () )
					{
						mapping = devIt->second;
						nextID = mappedID = mapping->deviceID;

						CVerbArg ( "HandleRegistration [ %s : %i ]:\tMapping found deviceID [0x%X] authLevel [%i] authToken [...].", client->ips, client->socket, nextID, mapping->authLevel );
					}
				}

				if ( !mapping ) {
					CVerbArg ( "HandleRegistration [ %s : %i ]:\tNo mapping found. Creating new.", client->ips, client->socket );

					mapping.reset ( new DeviceMapping );
					//mapping = make_shared < DeviceMapping > (); //sp ( DeviceMapping ) ( new DeviceMapping ); // calloc ( 1, sizeof(DeviceMapping) );
					if ( !mapping )
						goto PreFailExit;
					memset ( mapping.get (), 0, sizeof ( DeviceMapping ) );
				}

				nextID = GetNextDeviceID ( req->areaName, req->appName, ( unsigned int ) client->addr.sin_addr.s_addr );

				if ( nextID && ( nextID != mappedID ) ) {
					mapping->deviceID = nextID;

					CVerbArg ( "HandleRegistration [ %s : %i ]:\tCreated a mapping with deviceID [0x%X].", client->ips, client->socket, nextID );

					deviceMappings.list [ string ( req->deviceUID ) ] = mapping;
				}

			PreFailExit:
				deviceMappings.Unlock ( "HandleRegistration" );

				if ( !mapping ) {
					CErrArg ( "HandleRegistration [ %s : %i ]:\tFailed to find or create a mapping.", client->ips, client->socket );
					return false;
				}

				if ( nextID )
					deviceMappingDirty = true;
			}

			CLogArg ( "HandleRegistration [ %s : %i ]:\tAssigning device ID [%u] to [%s].", client->ips, client->socket, nextID, *req->deviceUID ? req->deviceUID : "Unknown" );

			SendBuffer ( client.get (), &nextID, sizeof ( nextID ) );
			return 1;
		}
		else {
			/// Message seems to be decrypted already

			deviceID = *( ( int * ) ( msg + 16 ) );

			CLogArgID ( "HandleRegistration [ %s : %i ]:\t[%c%c%c%c]", client->ips, client->socket, msg [ 4 ], msg [ 5 ], msg [ 6 ], msg [ 7 ] );

			/*if ( msgLen == MEDIATOR_BROADCAST_SPARE_ID_LEN ) {
			HandleStuntSocketRegistration ( client, deviceID );
			CVerbID ( "HandleRegistration: Closing stunt socket reg. thread." );
			return 0;
			}
			else */
			if ( msgLen >= ( MEDIATOR_BROADCAST_DESC_START + 3 ) ) {
				client->deviceID = deviceID;

				if ( !HandleDeviceRegistrationV4 ( client, ( unsigned int ) client->addr.sin_addr.s_addr, msg + 4 ) ) {
					CWarnArgID ( "HandleRegistration [ %s : %i ]:\tDevice registration failed.", client->ips, client->socket );
				}
				else {
					CVerbArgID ( "HandleRegistration [ %s : %i ]:\tSending socket buffer sizes.", client->ips, client->socket );

#ifdef MEDIATOR_USE_SOCKET_BUFFERS_APPLY_AT_CLIENT
					char buffer [ 20 ];

					int recSize = 0;
					int sendSize = 0;

					socklen_t retSize = sizeof ( recSize );

					// - Load send buffer size
					int rc = getsockopt ( ( int ) client->socket, SOL_SOCKET, SO_RCVBUF, ( char * ) &recSize, &retSize );
					if ( rc < 0 ) {
						CErrArgID ( "HandleRegistration [ %s : %i ]: Failed to query receive buffer size!", client->ips, client->socket ); LogSocketError ();
					}
					else {
						CVerbArgID ( "HandleRegistration [ %s : %i ]: receive buffer size [%i]", client->ips, client->socket, recSize );
					}

					// - Load send buffer size
					retSize = sizeof ( sendSize );

					rc = getsockopt ( ( int ) client->socket, SOL_SOCKET, SO_SNDBUF, ( char * ) &sendSize, &retSize );
					if ( rc < 0 ) {
						CErrArgID ( "HandleRegistration [ %s : %i ]: Failed to query send buffer size!", client->ips, client->socket ); LogSocketError ();
					}
					else {
						CVerbArgID ( "HandleRegistration [ %s : %i ]: send buffer size [%i]", client->ips, client->socket, sendSize );
					}

					if ( sendSize && recSize )
					{
						int * pUI = reinterpret_cast<int *>( buffer );
						*pUI = MEDIATOR_MSG_SOCKET_BUFFERS_SIZE;

						buffer [ 4 ] = MEDIATOR_PROTOCOL_VERSION;
						buffer [ 5 ] = MEDIATOR_CMD_SET_SOCKET_BUFFERS;
						buffer [ 6 ] = ';';
						buffer [ 7 ] = ';';

						pUI += 2;

						*pUI = recSize; pUI++;
						*pUI = sendSize;

						if ( !SendBuffer ( client.get (), buffer, MEDIATOR_MSG_SOCKET_BUFFERS_SIZE ) ) {
							CErrArgID ( "HandleRegistration [ %s : %i ]: Failed to inform about socket buffer sizes!", client->ips, client->socket );
						}
					}
#endif

#if !defined(ENABLE_WINSOCK_SEND_THREADS) && !defined(ENABLE_WINSOCK_CLIENT_THREADS)
                    int s = ( int ) client->socket;
					if ( !SetNonBlockSocket ( s, true, "HandleRegistration" ) )
					{
						CWarnArgID ( "HandleRegistration [ %s : %i ]:\tDevice registration failed.", client->ips, client->socket );
						return false;
					}
#endif
					// Look for a client thread to assign to
					unsigned int clientToMap = 0;
					unsigned int fewest = 0;

					for ( unsigned int i = 0; i < clientThreadCount; ++i )
					{
						clientContexts [ i ]->Lock ( "HandleRegistration" );

						if ( clientContexts [ i ]->clients.size () < fewest ) {
							fewest = ( unsigned int ) clientContexts [ i ]->clients.size ();
							clientToMap = i;
						}

						clientContexts [ i ]->Unlock ( "HandleRegistration" );
					}

					if ( !clientContexts [ clientToMap ]->Add ( client ) ) {
						return 0;
					}

#ifdef ENABLE_WINSOCK_CLIENT_THREADS
#	ifndef USE_ONE_TO_ONE_SOCKET_EVENTS
					if ( WSAEventSelect ( ( int ) client->socket, clientContexts [ clientToMap ]->revent, FD_READ | FD_CLOSE ) == SOCKET_ERROR ) {
						CWarnArgID ( "HandleRegistration [ %s : %i ]:\tFailed to register receive event [ %d ]", client->ips, client->socket, WSAGetLastError () );
						return 0;
					}
#	endif
#endif                    
					clientEvent.Notify ( "HandleRegistration" );

					CLogArgID ( "HandleRegistration [ %s : %i ]:\tDevice registration succeeded.", client->ips, client->socket );

					return 1;
				}
			}
		}

		if ( decrypted ) free ( decrypted );

		return ret;
	}
    
    
    void MediatorDaemon::ClientRemove ( ThreadInstance * client )
    {
        CVerbArg ( "ClientRemove [ %s : %i ]", client->ips, client->port );
        
        if ( isRunning && client->daemon )
        {
            RemoveAcceptClient ( client );
            
            CVerbArg ( "ClientRemove [ %s : %i ]:\tDisposing memory for client", client->ips, client->port );
            
            sp ( DeviceInstanceNode ) deviceSP = client->deviceSP;
            
            if ( deviceSP ) {
                CVerbArg ( "ClientRemove [ %s : %i ]:\tReleasing deviceSP", client->ips, client->port );
                
                RemoveDevice ( deviceSP.get () );
                client->deviceSP = 0;
            }
            
            sessions.Lock ( "ClientRemove" );
            
            const msp ( long long, ThreadInstance )::iterator &sessionIt = sessions.list.find ( client->sessionID );
            
            if ( sessionIt != sessions.list.end () ) 
			{
                sessions.list.erase ( sessionIt );

				sessions.BuildCache ();
            }
            
            sessions.Unlock ( "ClientRemove" );
            
            for ( size_t i = 0; i < sendThreadCount; ++i )
            {
				___sync_val_compare_and_swap ( sessionsChanged + i, 0, 1 );
            }
            
            ___sync_val_compare_and_swap ( &watchdogSessionsChanged, 0, 1 );
            
            client->thread.Notify ( "ClientRemove" );

#if defined(ENABLE_WINSOCK_SEND_THREADS) || defined(ENABLE_WINSOCK_CLIENT_THREADS)
			if ( IsValidFD ( ( int ) client->socketToClose ) )
			{
				if ( WSAEventSelect ( ( int ) client->socketToClose, NULL, 0 ) ) {
					CErr ( "ClientRemove: Failed to deassoc event from socket." );
				}
			}
#endif
            client->SendTcpFin ();

            client->Lock ( "ReleaseClient" );

            CloseThreadSocket ( &client->socketToClose );

            client->Unlock ( "ReleaseClient" );
            
            // Updated connected ips
            //
            
            if ( client->authenticated && client->sessionID )
            {
                unsigned int ip = client->addr.sin_addr.s_addr;
                
                if ( LockAcquireA ( bannedIPsLock, "ClientRemove" ) )
                {
                    std::map<unsigned int, unsigned int>::iterator it = connectedIPs.find ( ip );
                    if ( it != connectedIPs.end () )
                    {
                        unsigned int c = it->second;
                        if ( c <= 1 )
                            connectedIPs.erase ( it );
                        else
							connectedIPs [ ip ] = ( c - 1 );
                    }
                    
                    LockReleaseVA ( bannedIPsLock, "ClientRemove" );
                }
            }
            
            client->daemon = 0;
        }
    }
    
    
    void ApplySocketBufferSizes ( ThreadInstance * client, size_t recvSize )
    {
        int deviceID = client->deviceID;
        
        CVerbArgID ( "ApplySocketBufferSizes [ %s : %i ]", client->ips, client->socket );
        
        int rc;
        
        int recSize = 0;
        
        socklen_t retSize = sizeof ( recSize );
        
        // - Load send buffer size
        rc = getsockopt ( ( int ) client->socket, SOL_SOCKET, SO_RCVBUF, ( char * ) &recSize, &retSize );
        if ( rc < 0 ) {
            CErrArgID ( "ApplySocketBufferSizes [ %s : %i ]: Failed to query receive buffer size!", client->ips, client->socket ); LogSocketError ();
        }
        else {
            CVerbArgID ( "ApplySocketBufferSizes [ %s : %i ]: receive buffer size [%i]", client->ips, client->socket, recSize );
            
            recvSize <<= 1;
            
            if ( recSize < recvSize ) {
                rc = setsockopt ( ( int ) client->socket, SOL_SOCKET, SO_RCVBUF, ( const char * ) &recvSize, sizeof ( recvSize ) );
                if ( rc < 0 ) {
                    CErrArgID ( "ApplySocketBufferSizes [ %s : %i ]: Failed to set receive buffer size to [ %u ].", client->ips, client->socket, recvSize ); LogSocketError ();
                }
                else {
                    CVerbArgID ( "ApplySocketBufferSizes [ %s : %i ]: Receive buffer size set to [ %i ].", client->ips, client->socket, recvSize );
                }
            }
        }
        
        /*
        rc = setsockopt ( (int) client->socket, SOL_SOCKET, SO_SNDBUF, ( const char * ) &recSize, sizeof ( recSize ) );
        if ( rc < 0 ) {
            CErrArgID ( "ApplySocketBufferSizes [ %s : %i ]: Failed to set send buffer size.", client->ips, client->socket ); LogSocketError ();
        }
        else {
            CVerbArgID ( "ApplySocketBufferSizes [ %s : %i ]: Send buffer size set to [ %i ].", client->ips, client->socket, recSize );
        }*/
    }
    
    
    void * MediatorDaemon::ClientThreadStarter ( void * arg )
    {
        if ( !arg )
            return 0;
        
        ThreadInstance * client = ( ThreadInstance * ) arg;
        
        CVerbArg ( "ClientThreadStarter: Address of arg [ 0x%p ].", &client );
        
        MediatorDaemon * daemon = ( MediatorDaemon * ) client->daemon;
        if ( !daemon )
            return 0;
        
        int rc = 0;
        
        sp ( ThreadInstance ) clientSP = client->clientSP;
        while ( clientSP )
        {
            client->clientSP.reset ();

			bool isBanned = daemon->IsIpBanned ( client->addr.sin_addr.s_addr );
			if ( isBanned )
				break;

#ifdef USE_MEDIATOR_CLIENT_INSTANCE_BUFFER_UP
			client->bufferUP.bufferUP = up ( char [ ] ) ( new char [ MEDIATOR_CLIENT_MAX_BUFFER_SIZE + 1 ] );
			if ( !client->bufferUP.bufferUP )
				break;
#else
			client->bufferPtr  = new char [ MEDIATOR_CLIENT_MAX_BUFFER_SIZE + 1 ];
			if ( !client->bufferPtr )
				break;
#endif

#ifdef MEDIATOR_USE_TCP_NODELAY
			int value = 1;
			int ret = setsockopt ( ( int ) client->socket, IPPROTO_TCP, TCP_NODELAY, ( const char * ) &value, sizeof ( value ) );
			if ( ret < 0 ) {
				CErr ( "ClientThreadStarter: Failed to set TCP_NODELAY on socket" ); LogSocketError ();
			}
#endif
            ApplySocketBufferSizes ( client, MEDIATOR_CLIENT_MAX_BUFFER_SIZE );
            
            int s = ( int ) client->socket;
			SocketTimeout ( s, -1, 20 );

#ifdef USE_MEDIATOR_CLIENT_INSTANCE_BUFFER_UP
			client->ctx.msg				= client->bufferUP.bufferUP.get ();
#else
			client->ctx.msg				= client->bufferPtr;
#endif
			client->ctx.msgEnd			= client->ctx.msg;
			client->ctx.remainingSize	= MEDIATOR_CLIENT_MAX_BUFFER_SIZE - 1;

			CLogArg ( "ClientThread started for [ %s ] socket [ %i ]", client->ips, client->socket );

			// Execute thread
			rc = daemon->ClientThread ( clientSP );
			break;
        }
        
        if ( rc < 0 && daemon->isRunning )
        {
            daemon->ClientRemove ( client );
        }
        return 0;
    }

    
#ifdef MEDIATOR_USE_SOCKET_BUFFERS_APPLY_AT_SERVER
    void ApplySocketBufferSizes ( ThreadInstance * client, char * msg )
    {
        int deviceID = client->deviceID;
        
        CVerbArgID ( "ApplySocketBufferSizes [ %s : %i ]", client->ips, client->socket );
        
        int rc;
        int * pUI = reinterpret_cast<int *>( msg );
        
        int recSize = *pUI; pUI++;
        int sendSize = *pUI;
        
        if ( sendSize > 32000 && sendSize < 2000000 )
        {
            rc = setsockopt ( (int) client->socket, SOL_SOCKET, SO_RCVBUF, ( const char * ) &sendSize, sizeof ( sendSize ) );
            if ( rc < 0 ) {
                CErrArgID ( "ApplySocketBufferSizes [ %s : %i ]: Failed to set receive buffer size.", client->ips, client->socket ); LogSocketError ();
            }
            else {
                CVerbArgID ( "ApplySocketBufferSizes [ %s : %i ]: Receive buffer size set to [ %i ].", client->ips, client->socket, sendSize );
            }
        }
        
        if ( recSize > 32000 && recSize < 2000000 )
        {
            rc = setsockopt ( (int) client->socket, SOL_SOCKET, SO_SNDBUF, ( const char * ) &recSize, sizeof ( recSize ) );
            if ( rc < 0 ) {
                CErrArgID ( "ApplySocketBufferSizes [ %s : %i ]: Failed to set send buffer size.", client->ips, client->socket ); LogSocketError ();
            }
            else {
                CVerbArgID ( "ApplySocketBufferSizes [ %s : %i ]: Send buffer size set to [ %i ].", client->ips, client->socket, recSize );
            }
        }
    }
#endif
    
    
    int MediatorDaemon::ClientThread ( const sp ( ThreadInstance ) &clientSP )
    {
        CVerb ( "ClientThread started" );
        
        ThreadInstance * client = clientSP.get ();       
       
        CVerbArg ( "ClientThread [ %s : %i ]:\tSource port [ %u ]", client->ips, client->socket, ntohs ( client->addr.sin_port ) );
        
		int             deviceID        = client->deviceID;
		
		bool            handled         = false;
		char		*	msgEnd          = client->ctx.msgEnd;

		int				bytesReceived, sock = ( int ) client->socket;

		unsigned int	remainingSize	= client->ctx.remainingSize;
		char		*	msgDec			= 0;
		unsigned int	msgDecLength;
        char		*	decrypted		= 0;
        
#ifndef NDEBUG
        char lastCommand				= 0;
#endif        
		char		*	msg;

#ifdef USE_MEDIATOR_CLIENT_INSTANCE_BUFFER_UP
		char        *   buffer          = client->bufferUP.bufferUP.get ();
#else
		char        *   buffer          = client->bufferPtr;
#endif
		if ( !buffer )
			goto ShutdownClient;
        
        while ( isRunning && IsValidFD ( client->socket ) )
        {
#ifdef DEBUG_DEVICE_ID
			if ( deviceID == debugID ) {
				printf ( "Client [ %s : %i ]:\tGoing into recv ... buffer free [ %i ]!", client->ips, sock, remainingSize );
			}
#endif
            CVerbVerbArgID ( "ClientThread [ %s : %i ]:\tGoing into recv ... buffer free [ %i ]!", client->ips, sock, remainingSize );           

			bytesReceived = ( int ) recv ( ( int ) client->socket, msgEnd, remainingSize, 0 );

			if ( bytesReceived == 0 ) {
				VerbLogSocketError (); VerbLogSocketError1 ();
				CVerbArgID ( "Client [ %s : %i ]:\tconnection/socket closed by someone; Bytes [ %i ]!", client->ips, sock, bytesReceived );
				break;
			}

			if ( bytesReceived < 0 ) {
				SOCKET_Check_Val ( check );

				if ( !SOCKET_Check_Retry ( check ) )
					break;

				client->ctx.msgEnd          = msgEnd;
				client->ctx.remainingSize   = remainingSize;

				if ( !handled ) return 1;
				return 0;
			}

			handled = true;

			msgEnd [ bytesReceived ] = 0;
			msg = buffer;
            
            unsigned int msgLength;
            
#ifdef DEBUG_CHECK_CLIENT_CALL_MS
            char command = 'u';
#else
			char command;
#endif
            int bytesLeft = ( int ) ( bytesReceived + ( msgEnd - msg ) );
            while ( bytesLeft >= 8 )
            {
                int             unitLengthRemain;
                unsigned int    unitLength;
                
#ifdef DEBUG_CHECK_CLIENT_CALL_MS
				unsigned int    start = GetEnvironsTickCount32 ();
#endif
                msgLength = *( ( unsigned int * ) msg );
                unsigned flags = 0xF0000000 & msgLength;
                
                msgLength &= 0xFFFFFFF;
                
                CVerbVerbArgID ( "ClientThread [ %s : %i ]:\tmsgLength [ %d ] Bytes left [ %i ]!", client->ips, sock, msgLength, bytesLeft );
                
                if ( msgLength > ( unsigned int ) bytesLeft )
                {
                    if ( msgLength >= MEDIATOR_CLIENT_MAX_BUFFER_SIZE ) {
                        CErrArgID ( "ClientThread [ %s : %i ]:\tmsgLength [ %d ] >= [ %i ]!", client->ips, sock, msgLength, MEDIATOR_CLIENT_MAX_BUFFER_SIZE );
                        
                        /// Reset channel until received bytes becomes 0?
                        goto ShutdownClient;
                    }
                    break;
                }
                
                if ( flags )
                {
                    /// Message is encrypted
                    if ( flags & 0x40000000 )
                    {
                        msgDecLength = msgLength;
                        
                        /// Message is AES encrypted
                        if ( !AESDecrypt ( &client->aes, msg, &msgDecLength, &decrypted ) )
                            break;
                        msgDec = decrypted;
                    }
                    else {
                        msgDecLength = msgLength - 4;
                        
                        if ( flags == 0x80000000 ) {
                            /// Message is encrypted with public certificate
#ifdef USE_WIN32_CACHED_PRIVKEY
							if ( !DecryptMessageWithKeyHandles ( hPrivKey, msg + 4, msgDecLength, &decrypted, &msgDecLength ) )
#else
                            if ( !DecryptMessage ( privKey, privKeySize, msg + 4, msgDecLength, &decrypted, &msgDecLength ) ) 
#endif
							{
                                CWarnArgID ( "ClientThread [ %s : %i ]:\tDecrypt with privKey failed!", client->ips, sock ); goto ShutdownClient;
                            }
                            msgDec = decrypted;
                        }
                        else { CErrArgID ( "ClientThread [ %s : %i ]:\tUnknown encryption flags", client->ips, sock ); goto ShutdownClient; }
                    }
                }
                else { msgDecLength = msgLength; msgDec = msg; }

				if ( msgDec [ 4 ] == 'H' ) {
					if ( msgDec [ 5 ] == 'C' && msgDec [ 6 ] == 'L' && msgDec [ 7 ] == 'S' )
					{
						if ( client->encrypt ) {
							CWarnArgID ( "ClientThread [ %s : %i ]:\tTried to establish a safe channel again. Not allowed.", client->ips, sock ); goto ShutdownClient;
						}

						if ( !SecureChannelAuth ( client ) ) {
							CWarnArgID ( "ClientThread [ %s : %i ]:\tEstablishing Security / Authentication failed.", client->ips, sock ); goto ShutdownClient;
						}
						CLogArgID ( "ClientThread [ %s : %i ]:\tEstablishing Security / Authentication succeeded.", client->ips, sock );

                        unitLengthRemain    = 0;
                        unitLength          = 0;
						goto Continue;
					}
				}
                
                else if ( msgDec [ 4 ] == 'E' ) {
                    if ( msgDec [ 5 ] == '.' && msgDec [ 6 ] == 'D' )
                    {
                        int ret = HandleRegistration ( deviceID, clientSP, bytesLeft, msgDec, msgDecLength );
                        if ( ret == 0 ) {
							free_n ( decrypted ); return -1;
                        }
                        if ( ret > 0 ) {
                            BannIPRemove ( client->addr.sin_addr.s_addr );                            
							free_n ( decrypted ); return 1;
                        }
                        CVerbArgID ( "ClientThread [ %s : %i ]:\tHandle registration failed.", client->ips, sock );
                    }
					else {
						CLogArgID ( "ClientThread [ %s : %i ]:\tInvalid message [ %s %c %c %c ]. Refusing client.", client->ips, sock, GetCommandString ( msgDec [ 5 ] ), msgDec [ 4 ], msgDec [ 6 ], msgDec [ 7 ] );
					}                    
                    goto ShutdownClient;
                }
                
                CVerbArgID ( "ClientThread [ %s : %i ]:\t[ %s %c %c %c ]", client->ips, sock, GetCommandString ( msgDec [ 5 ] ), msgDec [ 4 ], msgDec [ 6 ], msgDec [ 7 ] );
                
                if ( !client->deviceID ) {
                    CWarnArgID ( "ClientThread [ %s : %i ]:\tRequest has not been identified. kicking connect.", client->ips, sock );                    
                    if ( !client->authenticated )
                        BannIP ( client->addr.sin_addr.s_addr );
                    goto ShutdownClient;
                }
                
                if ( msgDec [ 4 ] < MEDIATOR_PROTOCOL_VERSION_MIN ) {
                    CWarnArgID ( "ClientThread [ %s : %i ]:\tVersion [ %c ] not supported anymore", client->ips, sock, msgDec [ 4 ] ); goto ShutdownClient;
                }
                
                client->aliveLast	= checkLast;
                
                unitLengthRemain    = ( int ) msgDecLength;
                
                while ( unitLengthRemain > 0 )
                {
                    unitLength      = * ( reinterpret_cast<unsigned int *> ( msgDec ) );
                    if ( unitLength > ( unsigned ) unitLengthRemain )
                        unitLength = ( unsigned ) unitLengthRemain;
                    
					if ( unitLength <= 0 ) {
						char * c = msgDec;
						while ( unitLengthRemain > 0 ) {
							if ( *c == '7' ) {
								// Found possible new packet
								msgDec = c - 4;
								unitLengthRemain += 4;
								unitLength = 0;
								goto Continue;
							}
							c++;
							unitLengthRemain--;
						}
						break;
					}
					else if ( unitLength <= 8 )
							goto Continue;

                    command         = msgDec [ 5 ];
#ifndef NDEBUG
					lastCommand = command;
#endif

#ifdef DEBUG_DEVICE_ID1
                    if ( deviceID == debugID ) {
                        char timeString [ 256 ];
                        GetTimeString ( timeString, sizeof ( timeString ) );
                        printf ( "%s . [ %s %c %c ]\n", timeString, GetCommandString ( command ), msgDec [ 6 ], msgDec [ 7 ] );
                    }
#endif
                    if ( command == MEDIATOR_CMD_NATSTAT )
                    {
                        bool behindNAT = ( *( ( unsigned int * ) ( msgDec + 8 ) ) != ( unsigned int ) client->addr.sin_addr.s_addr );
                        
                        *( ( unsigned int * ) msgDec ) = MEDIATOR_NAT_REQ_SIZE;
                        msgDec [ 4 ] = behindNAT ? 1 : 0;
                        
                        int sentBytes = SendBuffer ( client, msgDec, MEDIATOR_NAT_REQ_SIZE );
                        
                        if ( sentBytes != MEDIATOR_NAT_REQ_SIZE ) {
                            CErrArgID ( "Client [ %s : %i ]:\tFailed to response NAT value [ %d ]", client->ips, sock, behindNAT );
                        }
                    }
                    // COMMAND: Return port of destination and request STUNT for client with deviceID (IP, IPe, Port)
                    else if ( command == MEDIATOR_CMD_STUNT )
                    {
                        HandleSTUNTRequest ( clientSP, ( STUNTReqPacketV8 * ) msgDec );
                    }
                    // COMMAND:
                    else if ( command == MEDIATOR_CMD_GET_DEVICES )
                    {
                        HandleQueryDevices ( clientSP, msgDec );
                    }
                    // COMMAND:
                    else if ( command == MEDIATOR_CMD_SHORT_MESSAGE )
                    {
                        HandleShortMessage ( client, msgDec, unitLength );
                    }
                    // COMMAND:
                    else if ( command == MEDIATOR_CMD_HELP_TLS_GEN )
                    {
                        if ( msgDec [ 6 ] == 's' )
                            HandleCertSign ( client, msgDec );
                        else
                            HandleCLSGenHelp ( client );
                    }
                    // COMMAND:
                    else if ( command == MEDIATOR_CMD_SET_FILTERMODE )
                    {
						if ( client->authLevel >= 3 ) {
							MediatorMsg * medMsg = ( MediatorMsg * ) msgDec;
							UpdateNotifyTargets ( clientSP, medMsg->ids.id2.msgID );
						}
                    }
                    // COMMAND:
                    else if ( command == MEDIATOR_CMD_HEARTBEAT )
                    {
                        goto Continue;
                    }
                    // COMMAND:
                    else if ( command == MEDIATOR_CMD_GET_VERSION )
                    {
                        // 0x00 relCount, 0x000 minor, 0x000 major; 0x00000000 revision
                        unsigned int version = BUILD_MAJOR_VERSION;
                        version |= BUILD_MINOR_VERSION << 12;
                        version |= BUILD_RELEASE_COUNTER << 24;
                        
                        int sendSize;
                        unsigned int versionBuffer [ 5 ];
                        
                        if ( client->version < '6' ) {
                            versionBuffer [ 0 ] = MEDIATOR_MSG_VERSION_SIZE;
                            versionBuffer [ 1 ] = version;
                            versionBuffer [ 2 ] = BUILD_REVISION;
                            sendSize = MEDIATOR_MSG_VERSION_SIZE;
                        }
                        else {
                            versionBuffer [ 0 ] = MEDIATOR_MSG_VERSION_SIZE;
                            versionBuffer [ 1 ] = version;
                            versionBuffer [ 2 ] = *( ( unsigned int * ) ( msgDec + 8 ) );
                            versionBuffer [ 3 ] = BUILD_REVISION;
                            
                            sendSize = MEDIATOR_MSG_VERSION_SIZE;
                        }
                        
                        if ( !SendBufferOrEnqueue ( client, versionBuffer, sendSize, true, 0 ) ) {
                            CErrArgID ( "Client [ %s : %i ]:\tFailed to response Mediator version", client->ips, sock );
                        }
                    }
                    // COMMAND:
                    else if ( command == MEDIATOR_CMD_NOTIFICATION_SUBSCRIBE )
                    {
                        MediatorMsg * medMsg = ( MediatorMsg * ) msgDec;
                        
                        client->subscribedToNotifications = ( medMsg->ids.id2.msgID == 1 ? true : false );
                    }
                    // COMMAND:
                    else if ( command == MEDIATOR_CMD_MESSAGE_SUBSCRIBE )
                    {
                        MediatorMsg * medMsg = ( MediatorMsg * ) msgDec;
                        
                        client->subscribedToMessages = ( medMsg->ids.id2.msgID == 1 ? true : false );
                    }
                    // COMMAND:
                    else if ( command == MEDIATOR_CMD_DEVICE_FLAGS )
                    {
                        HandleDeviceFlagSet ( client, msgDec );
                    }
					// COMMAND:
					else if ( command == MEDIATOR_CMD_STUNT_CLEAR )
					{
						HandleSTUNTClear ( client, ( StuntClearTarget * ) msgDec );
					}
#ifdef MEDIATOR_USE_SOCKET_BUFFERS_APPLY_AT_SERVER
                    /*else if ( command == MEDIATOR_CMD_SET_SOCKET_BUFFERS )
                     {
                     ApplySocketBufferSizes ( client, msgDec + 8 );
                     }
                     */
#endif
                    // COMMAND:
                    else if ( command == MEDIATOR_CMD_QUIT )
                    {
                        client->SendTcpFin (); break;
                    }
                    else {
                        msgDec [ unitLength - 1 ] = 0;
                        
                        HandleRequest ( client, msgDec );
                    }
                    
                Continue:
#ifdef DEBUG_CHECK_CLIENT_CALL_MS
                    if ( deviceID ) {
                        unsigned int end = GetEnvironsTickCount32 ();
                        
                        unsigned int diff = end - start;
                        if ( diff > maxMeasureDiff ) {
                            char timeString [ 256 ];
                            GetTimeString ( timeString, sizeof ( timeString ) );
                            
                            printf ( "%sClient: [ %u ms ]\t[ %c %c %s %i bytes ] [ %c %c ]\n", timeString, diff, client->version, command, GetCommandString ( command ), unitLength, msgDec [ 6 ], msgDec [ 7 ] );
                        }
                    }
#endif
                    unitLengthRemain    -= unitLength;
                    msgDec              += unitLength;
                }
                
                bytesLeft   -= msgLength;
                msg         += msgLength;
                
                free_m ( decrypted );
            }

			remainingSize = MEDIATOR_CLIENT_MAX_BUFFER_SIZE - 1;

			if ( bytesLeft > 0 ) {
				msgEnd += bytesReceived;

				RefactorBuffer ( msg, buffer, bytesLeft, msgEnd );
				remainingSize -= bytesLeft;
			}
			else
				msgEnd = buffer;
        }

	ShutdownClient:
#ifdef DEBUG_DEVICE_ID
		if ( deviceID == debugID ) {
			printf ( "Client [ %s : %i : %i ]:\tDisconnected", client->ips, client->port, sock );
		}
#endif
		client->socket		= INVALID_FD;
		client->aliveLast	= 0;

		free_n ( decrypted );
        
        if ( isRunning )
        {
			bool isBanned = false;

			unsigned int ip = client->addr.sin_addr.s_addr;

            if ( !client->deviceID && ip ) {
                if ( !client->authenticated ) {
                    CWarnArgID ( "ClientThread [ %s : %i : %i ]:\tRequest has not been identified. kicking connect.", client->ips, client->port, sock );
					isBanned = BannIP ( ip );
                }
            }
            
            if ( !isBanned )
                UpdateNotifyTargets ( clientSP, -1 );
        }
        else {
            UpdateNotifyTargets ( clientSP, -1 );
        }
        
        CLogArgID ( "ClientThread [ %s : %i : %i ]:\tDisconnected", client->ips, client->port, sock );
        return -1;
    }


	INLINEFUNC DeviceInstanceNode * GetDeviceInstance ( int deviceID, DeviceInstanceNode * device )
	{
		while ( device ) {
			if ( device->info.deviceID == deviceID ) {
				return device;
			}
			device = device->next;
		}
		return 0;
	}


#ifdef USE_WRITE_LOCKS1
    sp ( ThreadInstance ) MediatorDaemon::GetThreadInstance ( ThreadInstance * sourceClient, int deviceID, const char * areaName, const char * appName )
    {
        DEBUG_CHECK_START ();

        bool                        needsUnlock = false;
        sp ( ApplicationDevices )   appDevices  = 0;
        DeviceInstanceNode  *       destList    = 0;

        sp ( DeviceInstanceNode ) sourceDeviceSP = sourceClient->deviceSP;
        if ( !sourceDeviceSP )
            return 0;

        DeviceInstanceNode * sourceDevice = sourceDeviceSP.get ();

        if ( *areaName && *appName )
        {
            if ( sourceDevice && !strncmp ( areaName, sourceDevice->info.areaName, sizeof ( sourceDevice->info.areaName ) )
                && !strncmp ( appName, sourceDevice->info.appName, sizeof ( sourceDevice->info.appName ) ) )
            {
                appDevices = sourceDevice->rootSP;
            }
            else {
                appDevices = GetApplicationDevices ( appName, areaName );
                if ( !appDevices )
                    return 0;

                needsUnlock = true;
            }
        }
        else {
            appDevices = sourceDevice->rootSP;
        }

        if ( !appDevices->LockRead ( "GetThreadInstance" ) )
            return 0;

        if ( needsUnlock ) {
            destList = appDevices->devices;

            UnlockApplicationDevices ( appDevices.get () );
        }
        else if ( sourceDevice ) {
            destList = sourceDevice->rootSP->devices;
        }

        if ( !destList ) {
            appDevices->UnlockRead ( "GetThreadInstance" );
            return 0;
        }
        
        sp ( ThreadInstance ) destClient;
        
        DeviceInstanceNode	* device = GetDeviceInstance ( deviceID, destList );
        if ( device )
            destClient = device->clientSP;

        appDevices->UnlockRead ( "GetThreadInstance" );

        DEBUG_CHECK_MEASURE ( "GetThreadInstance" );
        
        return destClient;
    }
#else
    sp ( ThreadInstance ) MediatorDaemon::GetThreadInstance ( ThreadInstance * sourceClient, int deviceID, const char * areaName, const char * appName )
    {
        DEBUG_CHECK_START ();
        
		sp ( ApplicationDevices )   appDevices  = 0;
		pthread_mutex_t     *       destLock    = 0;
		DeviceInstanceNode  *       destList    = 0;

		sp ( DeviceInstanceNode ) sourceDeviceSP = sourceClient->deviceSP;
		if ( !sourceDeviceSP )
			return 0;

		DeviceInstanceNode * sourceDevice = sourceDeviceSP.get ();

		if ( *areaName && *appName )
		{
			if ( sourceDevice && !strncmp ( areaName, sourceDevice->info.areaName, sizeof ( sourceDevice->info.areaName ) )
				&& !strncmp ( appName, sourceDevice->info.appName, sizeof ( sourceDevice->info.appName ) ) )
			{
				destLock = &sourceDevice->rootSP->lock1;
			}
			else {
				appDevices = GetApplicationDevices ( appName, areaName );
				if ( !appDevices ) {
					return 0;
				}

				destLock = &appDevices->lock1;
			}
		}
		else {
			destLock = &sourceDevice->rootSP->lock1;
		}

		if ( !LockAcquire ( destLock, "GetThreadInstance" ) ) {
			return 0;
		}

		if ( appDevices ) {
			destList = appDevices->devices;

			UnlockApplicationDevices ( appDevices.get () );
		}
		else if ( sourceDevice ) {
			destList = sourceDevice->rootSP->devices;
		}

        if ( !destList ) {
            LockReleaseV ( destLock, "GetThreadInstance" );
			return 0;
        }

		sp ( ThreadInstance ) destClient;

		DeviceInstanceNode	* device = GetDeviceInstance ( deviceID, destList );
		if ( device )
			destClient = device->clientSP;
		 
        LockReleaseV ( destLock, "GetThreadInstance" );
        
        DEBUG_CHECK_MEASURE ( "GetThreadInstance" );

		return destClient;
	}
#endif

	bool MediatorDaemon::HandleShortMessage ( ThreadInstance * sourceClient, char * msg, unsigned int size )
	{
		CVerbVerbArg ( "HandleShortMessage [ %i ]", sourceClient->socket );

		// size of message (including size prefix)
		// 2m;;
		// 1. destination ID
		// 2. message ( size - 12 )

		ShortMsgPacketHeader*	shortMsg		= ( ShortMsgPacketHeader * ) msg;
		if ( shortMsg->size > size )
			return false;

        ShortMsgPacketV4	*	msg4			= ( ShortMsgPacketV4 * ) msg;

		char			*	sendBuffer			= msg;
		ShortMsgPacketHeader*   msgBuffer       = 0;
		int					length				= shortMsg->size;

		int                 destID				= shortMsg->deviceID;
		int                 deviceID			= sourceClient->deviceID;

		CVerbArgID ( "HandleShortMessage [ %i ]", sourceClient->socket );

		sp ( DeviceInstanceNode ) sourceDeviceSP = sourceClient->deviceSP;
        
		DeviceInstanceNode * sourceDevice       = sourceDeviceSP.get ();

		char * appName  = 0;
		char * areaName = 0;

		if ( sourceClient->version <= '4' ) {
			appName     = msg4->appName;
			areaName    = msg4->areaName;
		}
		else {
			appName     = ( ( ShortMsgPacket* ) shortMsg )->appArea;
			areaName    = appName + shortMsg->sizes [ 0 ];

			if ( shortMsg->sizes [ 0 ] > MAX_NAMEPROPERTY || shortMsg->sizes [ 1 ] > MAX_NAMEPROPERTY )
				return false;
		}

		// Find the destination client
		sp ( ThreadInstance ) destClientSP = GetThreadInstance ( sourceClient, destID, areaName, appName );

        ThreadInstance * destClient = destClientSP.get ();
        
		if ( !destClient || !destClient->subscribedToMessages || destClient->socket == INVALID_FD ) {
            CErrArgID ( "HandleShortMessage [ %i ]: Failed to find device connection for id [ 0x%X ]!", sourceClient->socket, destID );
            goto SendResponse;
		}
        
		if ( destClient == sourceClient ) {
            CVerbArgID ( "HandleShortMessage [ %i ]: Destination and source are the same.", destID );
            goto SendResponse;
		}

		shortMsg->deviceID = deviceID;

		if ( destClient->version >= '5' ) {
			msgBuffer = ( ShortMsgPacketHeader * ) malloc ( shortMsg->size + ( MAX_NAMEPROPERTY * 2 ) );
			if ( !msgBuffer )
				return false;

			memcpy ( msgBuffer, shortMsg, sizeof ( ShortMsgPacketHeader ) );

			msgBuffer->deviceID = deviceID;

			if ( sourceDevice &&
                //sourceDevice->rootSP.get () != destRoot &&
                (shortMsg->sizes [ 0 ] > 1 || shortMsg->sizes [ 1 ] > 1 ) &&
                *sourceDevice->info.appName &&
                *sourceDevice->info.areaName )
            {
				if ( !BuildAppAreaField ( msgBuffer->sizes, sourceDevice->info.appName, sourceDevice->info.areaName, false ) )
					goto SendResponse;
			}
			else {
				msgBuffer->sizes [ 1 ] = 1;
				msgBuffer->sizes [ 1 ] = 1;
				( ( ShortMsgPacket * ) msgBuffer )->appArea [ 0 ] = 0;
				( ( ShortMsgPacket * ) msgBuffer )->appArea [ 1 ] = 0;
			}            
            
            size_t offsetOrg = sizeof ( ShortMsgPacketHeader ) + shortMsg->sizes [ 0 ] + shortMsg->sizes [ 1 ];

            // copy the message
			length = ( int ) ( shortMsg->size - offsetOrg );
			if ( length <= 0 )
				goto SendResponse;

			memcpy ( msgBuffer->sizes + 2 + msgBuffer->sizes [ 0 ] + msgBuffer->sizes [ 1 ], msg + offsetOrg, length );

			msgBuffer->size = sizeof ( ShortMsgPacketHeader ) + msgBuffer->sizes [ 0 ] + msgBuffer->sizes [ 1 ] + length;

			length      = msgBuffer->size;
			sendBuffer  = ( char * ) msgBuffer;

			sendBuffer [ length ] = 0;
		}
		else {
			shortMsg->deviceID = deviceID;

			if ( sourceDevice ) {
				if ( *sourceDevice->info.areaName )
					strlcpy ( msg4->areaName, sourceDevice->info.areaName, sizeof ( msg4->areaName ) );
				if ( *sourceDevice->info.appName )
					strlcpy ( msg4->appName, sourceDevice->info.appName, sizeof ( msg4->appName ) );
			}
		}

		CLogArgID ( "HandleShortMessage [ %i ]: Send message to device [ 0x%X ] IP [ %u bytes -> %s ]", sourceClient->socket, destID, length, inet_ntoa ( destClient->addr.sin_addr ) );
        
        if ( sendBuffer == ( char * ) msgBuffer ) {
            if ( PushSend ( destClient, sendBuffer, length, false, 0 ) ) {
                CLogArgID ( "HandleShortMessage [ %i ]: Successfully sent message.", sourceClient->socket );
                
                msgBuffer = 0;
            }
        }
        else {
            if ( PushSend ( destClient, sendBuffer, length, true, 0 ) ) {
                CLogArgID ( "HandleShortMessage [ %i ]: Successfully sent message.", sourceClient->socket );
            }
        }
    SendResponse:
        free_n ( msgBuffer );
		return true;
	}


	bool BuildDevicesCache ( ApplicationDevices * appDevices )
	{
		bool                    alloc   = false;
		unsigned int listCount          = appDevices->count;

		appDevices->deviceCacheCount    = 0;

		if ( !appDevices->devicesCache || !appDevices->deviceCacheCapacity )
			alloc = true;
		else if ( listCount > ( unsigned ) appDevices->deviceCacheCapacity )
			alloc = true;
		else if ( ( appDevices->deviceCacheCapacity - listCount ) > 10 )
			alloc = true;

		if ( alloc )
		{
			size_t cacheSize = ( sizeof ( environs::lib::DeviceInfoShort ) * listCount ) + ( MAX_NAMEPROPERTY * 3 );

			char * tmp = ( char * ) malloc ( cacheSize );
			if ( !tmp ) {
				CErrArg ( "BuildDevicesCache: Failed to allocate [ %u bytes ] memory for [ %i ] devices.", cacheSize, listCount );
				return false;
			}

			free_n ( appDevices->devicesCache );

			appDevices->devicesCache        = tmp;
			appDevices->deviceCacheCapacity = appDevices->count;
		}

		appDevices->deviceCacheDirty    = false;

		char                *   item    = appDevices->devicesCache;

		DeviceInstanceNode	*	device  = appDevices->devices;
		if ( !device )
			return true;
		device->info.hasAppEnv = 1;

		listCount = 1;

		// Copy first item
		memcpy ( item, device, sizeof ( DeviceInfo ) );

		item   += sizeof ( DeviceInfo );

		device = device->next;

		while ( device )
		{
			device->info.hasAppEnv = 0;

			memcpy ( item, device, sizeof ( DeviceInfoShort ) );
			listCount++;

			item   += sizeof ( DeviceInfoShort );

			device = device->next;
		}

		appDevices->deviceCacheCount = listCount;
		return true;
	}


	bool CollectDevices ( bool search, int reqDeviceID, ApplicationDevices * appDevices, char *& resultList, unsigned int &resultCapacity, unsigned int &resultCount )
	{
		if ( !appDevices || !appDevices->count )
			return true;

        if ( !appDevices->LockRead ( "CollectDevices" ) )
			return false;

		bool                    success     = false;
		DeviceInfoShort *       info;

		unsigned int            listCount	= appDevices->count;
		if ( !listCount )
			goto Finish;

		if ( search )
		{
			DeviceInstanceNode * device = appDevices->devices;

			while ( device )
			{
				if ( reqDeviceID == device->info.deviceID )
				{
					memcpy ( resultList, device, sizeof ( DeviceInfo ) );

					resultCount++;
					resultCapacity--;

					resultList += sizeof ( DeviceInfo ); // This is required for size calculations

					success = true;
					break; /// Stop the query, We have found the device.
				}
				device = device->next;
			}
			goto Finish;
		}

        // Check if cache is valid
#ifdef USE_WRITE_LOCKS2
        if ( !LockAcquireA ( appDevices->deviceCacheLock, "CollectDevices" ) )
            goto Finish;

        if ( appDevices->deviceCacheDirty )
        {
            bool s = BuildDevicesCache ( appDevices );
            if ( !s ) {
                LockReleaseA ( appDevices->deviceCacheLock, "CollectDevices" );
                goto Finish;
            }
        }

        LockReleaseA ( appDevices->deviceCacheLock, "CollectDevices" );
#else
        if ( appDevices->deviceCacheDirty && !BuildDevicesCache ( appDevices ) )
            goto Finish;
#endif
		if ( !reqDeviceID )
		{
			if ( listCount > resultCapacity )
				listCount = resultCapacity;

			if ( listCount <= 0 )
				goto Finish;

			size_t cacheSize = ( ( ( listCount - 1 ) * sizeof ( DeviceInfoShort ) ) + sizeof ( DeviceInfo ) );

			memcpy ( resultList, appDevices->devicesCache, cacheSize );

			resultList      += cacheSize;

			resultCapacity  -= listCount;
			resultCount     += listCount;
			goto Finish;
		}


		// Look for the first item that the client has indicated as the last device
		info = ( DeviceInfoShort * ) appDevices->devicesCache;

		if ( info->deviceID == reqDeviceID )
			success = true;

		listCount--;

		info = ( DeviceInfoShort * ) ( appDevices->devicesCache + ( sizeof ( DeviceInfo ) ) );
		if ( !success )
		{
			while ( listCount > 0 )
			{
				listCount--;
				if ( info->deviceID == reqDeviceID ) {
					success = true;
					break;
				}
				info++;
			}
			info++;
		}

		if ( success )
		{
			size_t cacheSize = ( listCount * sizeof ( DeviceInfoShort ) );

			memcpy ( resultList, info, cacheSize );

			resultList      += cacheSize;

			resultCapacity  -= listCount;
			resultCount     += listCount;
		}

    Finish:
        if ( !appDevices->UnlockRead ( "CollectDevices" ) )
			return false;
		return success;
	}


	bool CollectDevicesV4 ( ThreadInstance * sourceClient, unsigned int &startIndex,
		int reqDeviceID, const char * areaName, const char * appName,
		ApplicationDevices * appDevices, DeviceInfoNodeV4 *& resultList, unsigned int &resultCount )
	{
		if ( !appDevices || !appDevices->count )
			return true;

		bool                    success				= true;
		DeviceInfoNodeV4	*	curDevice			= resultList;
		unsigned int            currentStart;
		DeviceInstanceNode	*	device; //				= appDevices->devices;

        if ( !appDevices->LockRead ( "CollectDevices" ) )
			return false;

		unsigned int listCount	= appDevices->count;

		if ( startIndex > listCount ) {
			startIndex -= listCount;

            appDevices->UnlockRead ( "CollectDevices" );
			return true;
		}

		const unsigned int	maxDeviceCount		= ( MEDIATOR_BUFFER_SIZE_MAX - DEVICES_HEADER_SIZE ) / DEVICE_PACKET_SIZE;
		if ( resultCount >= maxDeviceCount )
			return false;

		device = appDevices->devices;

		if ( startIndex ) {
			currentStart = 0;
			while ( device ) {
				if ( currentStart >= startIndex )
					break;

				currentStart++;
				device = device->next;
			}
		}
		startIndex = 0;

		DeviceInfoNodeV4 * prev = curDevice;

		while ( device )
		{
			if ( reqDeviceID ) {
				if ( reqDeviceID == device->info.deviceID && !strncmp ( device->info.areaName, areaName, MAX_NAMEPROPERTY )
					&& !strncmp ( device->info.appName, appName, MAX_NAMEPROPERTY ) )
				{
					DeviceInfoNodeV4 * newDevice = ( DeviceInfoNodeV4 * ) malloc ( sizeof ( DeviceInfoNodeV4 ) );
					if ( !newDevice ) {
						success = false; goto Finish;
					}

					memcpy ( newDevice, device, sizeof ( DeviceInfo ) );
					//*newDevice->userName = 0;
					newDevice->next = 0;

					resultList = newDevice;
					resultCount = 1;
					success = false; goto Finish; /// Stop the query, We have found the device.
				}

				device = device->next;
				continue;
			}

			/// We have a device to insert into the result list
			DeviceInfoNodeV4 * newDevice = ( DeviceInfoNodeV4 * ) malloc ( sizeof ( DeviceInfoNodeV4 ) );
			//DeviceInstanceNode * newDevice = new DeviceInstanceNode;
			if ( !newDevice ) {
				success = false; goto Finish;
			}

			memcpy ( newDevice, device, sizeof ( DeviceInfo ) );
			//*newDevice->userName = 0;
			newDevice->next = 0;


			/// Find the position in result list
			if ( !curDevice ) {
				/// We have an empty list; Create a new item and set as root
				resultList = newDevice;
				prev = newDevice;
			}
			else {
				/// Find the first id that is larger than the current            
				while ( curDevice ) {
					if ( curDevice->info.deviceID > device->info.deviceID ) {
						break;
					}
					prev = curDevice;
					curDevice = curDevice->next;
				}
				curDevice = prev;

				if ( curDevice->info.deviceID > device->info.deviceID && curDevice == resultList ) {
					resultList = newDevice;
					newDevice->next = curDevice;
				}
				else {
					if ( curDevice->next )
						newDevice->next = curDevice->next;
					curDevice->next = newDevice;
				}
			}

			curDevice = newDevice;

			resultCount++;

			if ( resultCount >= maxDeviceCount ) {
				success = false; goto Finish;
			}

			device = device->next;
		}

    Finish:
        if ( !appDevices->UnlockRead ( "CollectDevices" ) )
			return false;
		return success;
	}


	unsigned int MediatorDaemon::CollectDevicesCount ( DeviceInstanceNode * sourceDevice, int filterMode )
	{
		if ( !sourceDevice || !sourceDevice->rootSP )
			return 0;

		unsigned int				deviceCount = 0;
		sp ( AreaApps )				areaApps	= 0;
		sp ( ApplicationDevices )	appDevices	= 0;

		/// Get number of devices within the same application environment
		deviceCount += sourceDevice->rootSP->count;

		if ( filterMode < 1 ) {
            /// No filtering, get them all
            if ( !areas.LockRead ( "CollectDevicesCount" ) )
				goto Finish;

			/// Iterate over all areas
			for ( msp ( string, AreaApps )::iterator it = areas.list.begin (); it != areas.list.end (); ++it )
			{
                areaApps = it->second;
                if ( !areaApps || !areaApps->LockRead ( "CollectDevicesCount" ) )
					continue;

				for ( msp ( string, ApplicationDevices )::iterator ita = areaApps->apps.begin (); ita != areaApps->apps.end (); ++ita )
				{
					appDevices = ita->second;
					if ( !appDevices || appDevices.get () == sourceDevice->rootSP.get () )
						continue;

					deviceCount += appDevices->count;
				}

                areaApps->UnlockRead ( "CollectDevicesCount" );
			}

            areas.UnlockRead ( "CollectDevicesCount" );
		}
		else if ( filterMode < 2 ) {
			/// Get number of devices of other application environments within the same area
			string areaName ( sourceDevice->info.areaName );

            if ( !areas.LockRead ( "CollectDevicesCount" ) )
				goto Finish;

			const msp ( string, AreaApps )::iterator areaIt = areas.list.find ( areaName );

			if ( areaIt == areas.list.end () ) {
				CLogArg ( "CollectDevicesCount: Area [%s] not found.", sourceDevice->info.areaName );
			}
			else
				areaApps = areaIt->second;

            areas.UnlockRead ( "CollectDevicesCount" );

            if ( !areaApps || !areaApps->LockRead ( "CollectDevicesCount" ) )
            {
				CLog ( "CollectDevicesCount: Invalid area name." );
				goto Finish;
			}

			for ( msp ( string, ApplicationDevices )::iterator ita = areaApps->apps.begin (); ita != areaApps->apps.end (); ++ita )
			{
				appDevices = ita->second;
				if ( !appDevices || appDevices.get () == sourceDevice->rootSP.get () )
					continue;

				deviceCount += appDevices->count;
			}

            areaApps->UnlockRead ( "CollectDevicesCount" );
		}

	Finish:
		return deviceCount;
	}


	char * FlattenDeviceListV4 ( DeviceInfoNodeV4 * devices, unsigned int capacity )
	{
		if ( !devices || !capacity )
			return 0;

		DeviceInfo * dest = 0;
		char * buffer = ( char * ) malloc ( ( capacity * DEVICE_PACKET_SIZE ) + DEVICES_HEADER_SIZE + DEVICES_HEADER_SIZE );
		if ( !buffer ) {
			goto Finish;
		}

		dest = ( DeviceInfo * ) ( buffer + DEVICES_HEADER_SIZE );

		while ( devices ) {
			DeviceInfoNodeV4 * current = devices;
			devices = devices->next;

			memcpy ( dest, current, sizeof ( DeviceInfo ) );
			dest++;

			free ( current );
		}

	Finish:
		if ( devices ) {
			while ( devices ) {
				DeviceInfoNodeV4 * current = devices;

				devices = devices->next;
				free ( current );
			}
		}
		return buffer;
    }
    
    
    bool MediatorDaemon::HandleQueryDevices ( const sp ( ThreadInstance ) &sourceClient, char * msg )
    {
        if ( sourceClient->version < '6' )
            return HandleQueryDevicesV5 ( sourceClient, msg );
        
        MediatorQueryResponseV6 * response		= ( MediatorQueryResponseV6 * ) msg;
        MediatorQueryMsgV6 *	query			= ( MediatorQueryMsgV6 * ) msg;
        MediatorQueryHeaderV6 *	header			= ( MediatorQueryHeaderV6 * ) msg;
        char			*	sendBuffer			= msg;
        int					length				= 8;
        int                 deviceID			= header->msgID;
        int                 deviceIDReq			= 0;
        
        bool				success				= false;
        bool                search;
        int					filterMode			= sourceClient->filterMode;
        char				subCmd;
        char            *   resultBuffer		= 0;
        char            *   resultList			= 0;
        unsigned int        resultCount         = 0;
        unsigned int        resultCapacity      = 0;
        unsigned int        resultBufferSize    = 0;
        
        char			*	areaName			= 0;
        char			*	appName				= 0;
        bool                appEnvGiven;
        bool                appEnvMatch;
        
        const unsigned int	maxDeviceCount		= ( MEDIATOR_REC_BUFFER_SIZE_MAX - DEVICES_HEADER_SIZE ) / DEVICE_PACKET_SIZE;
        
        unsigned int        seqNr               = header->seqNr;
        sourceClient->seqNr                     = seqNr;
        
        msp ( string, ApplicationDevices ) * apps;
        
        DeviceInstanceNode * sourceDevice;
        sp ( ApplicationDevices ) appDevices;
        
        sp ( DeviceInstanceNode ) sourceDeviceSP = sourceClient->deviceSP;
        if ( !sourceDeviceSP )
            goto SendResponse;
        
        sourceDevice    = sourceDeviceSP.get ();
        appDevices      = sourceDevice->rootSP;
  
        /// Take over filterMode
        subCmd          = header->opt1;
        
#if defined(DEBUG_DEVICE_ID) || defined(DEBUG_TIMEOUT_ISSUE)
        char timeString [ 256 ];
        if ( deviceID == debugID ) {
            GetTimeString ( timeString, sizeof ( timeString ) );
            printf ( "%sHandleQueryDevices: Received request [ %i ]\n", timeString, seqNr );
        }
#endif
        if ( subCmd != MEDIATOR_OPT_NULL )
            filterMode = subCmd;
        
        if ( sourceClient->authLevel < 3 )
            filterMode = MEDIATOR_FILTER_AREA_AND_APP;
        
        if ( filterMode != sourceClient->filterMode )
            UpdateNotifyTargets ( sourceClient, filterMode );
        
        // Send number of available devices
        if ( header->opt0 == MEDIATOR_CMD_DEVICE_LIST_QUERY_COUNT ) {
            CVerbArgID ( "HandleQueryDevices [ %i ]: requested count", sourceClient->socket );
            
            length              = MEDIATOR_CMD_GET_DEVICES_COUNT_RESP_LENV6;
            
            header->size        = length;
            header->cmdVersion  = MEDIATOR_PROTOCOL_VERSION;
            header->cmd1        = MEDIATOR_CMD_DEVICE_LIST_QUERY;
            header->opt0        = MEDIATOR_CMD_DEVICE_LIST_QUERY_COUNT;
            
            header->msgID       = CollectDevicesCount ( sourceDevice, filterMode );
            
            CVerbArgID ( "HandleQueryDevices [ %i ]: Number of devices [ %u ]", sourceClient->socket, header->msgID );
			success = true; goto SendResponse;
        }
        
        if ( header->resultCount > maxDeviceCount )
            header->resultCount = maxDeviceCount;
        
        resultBufferSize = ( sizeof ( DeviceInfo ) * header->resultCount ) + sizeof ( MediatorQueryResponse ) * 2;
        
        resultBuffer    = ( char * ) malloc ( resultBufferSize );
        if ( !resultBuffer ) {
            CErrArg ( "HandleQueryDevices: Failed to allocate [ %u bytes ] memory for [ %i ] devices.", resultBufferSize, header->resultCount );
            
#ifdef DEBUG_DEVICE_ID
            if ( deviceID == debugID ) {
                GetTimeString ( timeString, sizeof ( timeString ) );
                printf ( "%sHandleQueryDevices: Failed to allocate [ %u bytes ] memory for [ %i ] devices.\n", timeString, resultBufferSize, header->resultCount );
            }
#endif
            goto Finish;
        }
        
        resultCapacity  = header->resultCount;
        resultList = ( resultBuffer + DEVICES_HEADER_SIZE_V6 );
        
        
        subCmd              = header->opt0;
        
        search              = ( subCmd == MEDIATOR_CMD_DEVICE_LIST_QUERY_SEARCH );
        
        if ( subCmd == MEDIATOR_OPT_DEVICE_LIST_DEVICE_ID || search ) {
            deviceIDReq     = header->deviceID;
            
            appName         = query->appArea;
            areaName        = appName + header->sizes [ 0 ];
            
            if ( header->sizes [ 0 ] > 1 && header->sizes [ 1 ] > 1
                && header->sizes [ 0 ] < MAX_NAMEPROPERTY && header->sizes [ 1 ] < MAX_NAMEPROPERTY &&
                *appName && *areaName )
            {
                areaName [ header->sizes [ 1 ] ] = 0;
                
#ifdef DEBUG_DEVICE_ID
                if ( deviceID == debugID ) {
                    GetTimeString ( timeString, sizeof ( timeString ) );
                    printf ( "%sHandleQueryDevices [ %i ]: [ %i ] Query for deviceID  [ 0x%X ] [ %s | %s ]", timeString, (int) sourceClient->socket, header->seqNr, deviceIDReq, areaName, appName );
                }
#endif
                CVerbArgID ( "HandleQueryDevices [ %i ]: Query for deviceID  [ 0x%X ] [ %s | %s ]", (int) sourceClient->socket, deviceIDReq, areaName, appName );
            }
            else {
                appName     = 0;
                areaName    = 0;
                
#ifdef DEBUG_DEVICE_ID
                if ( deviceID == debugID ) {
                    GetTimeString ( timeString, sizeof ( timeString ) );
                    printf ( "%sHandleQueryDevices [ %i ]: [ %i ] Query for deviceID [ 0x%X  ] of default appEnv [ %s | %s ]\n", timeString, (int)sourceClient->socket, header->seqNr, deviceIDReq, sourceDevice->info.appName, sourceDevice->info.areaName );
                }
#endif
                CVerbArgID ( "HandleQueryDevices [ %i ]: Query for deviceID [ 0x%X ] of default appEnv [ %s | %s ]", (int) sourceClient->socket, deviceIDReq, sourceDevice->info.appName, sourceDevice->info.areaName );
            }
        }
        
        
#ifdef DEBUG_DEVICE_ID
        if ( deviceID == debugID ) {
            GetTimeString ( timeString, sizeof ( timeString ) );
            printf ( "%sHandleQueryDevices [ %i ]: [ %i ] requestCount [ %u ] filterMode [ %i ]\n", timeString, (int) sourceClient->socket, header->seqNr, header->resultCount, filterMode );
        }
		CVerbArgID ( "HandleQueryDevices [ %i ]: [ %i ] requestCount [ %u ] filterMode [ %i ]", (int) sourceClient->socket, header->seqNr, header->resultCount, filterMode );
#endif
        
        
        /// Lets start with the devices within the same appEnv
        appEnvGiven = ( appName && areaName && *appName && *areaName );
        
        if ( appEnvGiven )
            appEnvMatch = ( !strncmp ( appName, sourceDevice->info.appName, sizeof ( sourceDevice->info.appName ) )
                           && !strncmp ( areaName, sourceDevice->info.areaName, sizeof ( sourceDevice->info.areaName ) ) );
        else
            appEnvMatch = false;
        
        if ( !appEnvGiven || appEnvMatch )
        {
            if ( !appDevices->count ) {
                if ( appEnvMatch )
                    goto Finish;
            }
            
            if ( !CollectDevices ( search, deviceIDReq, appDevices.get (), resultList, resultCapacity, resultCount ) )
                goto Finish;
        }
        
        
        if ( filterMode < 1 ) {
            /// No filtering, get them all
            if ( !areas.LockRead ( "HandleQueryDevices" ) )
                goto Finish;
            
            vsp ( AreaApps ) searchAreas;
            
            /// Iterate over all areas
            for ( msp ( string, AreaApps )::iterator it = areas.list.begin (); it != areas.list.end (); ++it )
            {
                if ( !it->second )
                    continue;
                
                if ( appEnvGiven ) {
                    if ( strncmp ( areaName, it->second->name.c_str (), MAX_NAMEPROPERTY ) )
                        continue;
                }
                searchAreas.push_back ( it->second );
            }

            areas.UnlockRead ( "HandleQueryDevices" );
            
            for ( vsp ( AreaApps )::iterator its = searchAreas.begin (); its != searchAreas.end (); ++its )
            {
                AreaApps * areaApps	= its->get ();
                if ( !areaApps->LockRead ( "HandleQueryDevices" ) )
                    continue;
                apps = &areaApps->apps;
                
                for ( msp ( string, ApplicationDevices )::iterator ita = apps->begin (); ita != apps->end (); ++ita )
                {
                    appDevices = ita->second;
                    if ( !appDevices || appDevices.get () == sourceDevice->rootSP.get () )
                        continue;
                    
                    if ( appEnvGiven )
                        appEnvMatch = ( !strncmp ( appName, appDevices->name.c_str (), MAX_NAMEPROPERTY )
                                       && !strncmp ( areaName, areaApps->name.c_str (), MAX_NAMEPROPERTY ) );
                    else
                        appEnvMatch = false;
                    
                    if ( !appDevices->count ) {
                        if ( appEnvMatch ) {
                            areaApps->UnlockRead ( "HandleQueryDevices" );
							goto Finish;
						}
                    }
                    else
                    {
                        if ( appEnvGiven && !appEnvMatch )
                            continue;
                        
                        if ( !CollectDevices ( search, deviceIDReq, appDevices.get (), resultList, resultCapacity, resultCount ) )
                        {
                            areaApps->UnlockRead ( "HandleQueryDevices" );
                            goto Finish;
                        }
                    }
                }
                areaApps->UnlockRead ( "HandleQueryDevices" );
            }
        }
        else if ( filterMode < 2 ) {
            /// Get number of devices of other application environments within the same area
            char * area = ( areaName && *areaName ? areaName : sourceDevice->info.areaName );
            
            string pareaName ( area );
            
            sp ( AreaApps ) areaApps;

            if ( !areas.LockRead ( "HandleQueryDevices" ) )
                goto Finish;
            
            const msp ( string, AreaApps )::iterator areaIt = areas.list.find ( pareaName );
            
            if ( areaIt != areas.list.end () )
                areaApps = areaIt->second;

            areas.UnlockRead ( "HandleQueryDevices" );

            if ( !areaApps || !areaApps->LockRead ( "HandleQueryDevices" ) )
            {
#ifdef DEBUG_DEVICE_ID
                if ( deviceID == debugID ) {
                    GetTimeString ( timeString, sizeof ( timeString ) );
                    printf ( "%sHandleQueryDevices [ %i ]: [ %i ] Area [ %s ] not found.\n", timeString, (int)sourceClient->socket, header->seqNr, areaName );
                }
#endif
                CLogArg ( "HandleQueryDevices [ %i ]: Area [ %s ] not found.", (int)sourceClient->socket, areaName );
                goto Finish;
            }
            
            apps = &areaApps->apps;
            
            for ( msp ( string, ApplicationDevices )::iterator ita = apps->begin (); ita != apps->end (); ++ita )
            {
                appDevices = ita->second;
                if ( !appDevices || appDevices == sourceDevice->rootSP )
                    continue;
                
                if ( appEnvGiven )
                    appEnvMatch = ( !strncmp ( appName, appDevices->name.c_str (), MAX_NAMEPROPERTY )
                                   && !strncmp ( areaName, areaApps->name.c_str (), MAX_NAMEPROPERTY ) );
                else
                    appEnvMatch = false;
                
                if ( !appDevices->count )
                {
                    if ( appEnvMatch ) {
                        areaApps->UnlockRead ( "HandleQueryDevices" );
						goto Finish;
					}
                }
                else {
                    if ( appEnvGiven && !appEnvMatch )
                        continue;
                    
                    if ( !CollectDevices ( search, deviceIDReq, appDevices.get (), resultList, resultCapacity, resultCount ) )
                        break;
                }
            }
            areaApps->UnlockRead ( "HandleQueryDevices" );
        }
        
    Finish:
        if ( !resultBuffer )
            goto SendResponse;
        
		success		= true;
        response    = ( MediatorQueryResponseV6 * ) resultBuffer;
        sendBuffer  = ( char * ) response;
        
        response->cmd1                              = MEDIATOR_CMD_DEVICE_LIST_QUERY_RESPONSE;
        response->cmd0                              = MEDIATOR_CMD_DEVICE_LIST_QUERY;
        response->seqNr                             = seqNr;
        response->deviceHead.deviceCountAvailable   = resultCount;
        response->deviceHead.startIndex             = 0;
        response->deviceHead.deviceCount            = resultCount;
        
        length = ( int ) ( resultList - resultBuffer );
        response->size = length;
        
    SendResponse:
        if ( !success ) {
            sendBuffer      = msg;
            header->cmd1    = MEDIATOR_CMD_DEVICE_LIST_QUERY;
            header->opt0    = MEDIATOR_CMD_DEVICE_LIST_QUERY_ERROR;
            length = header->size = 12;
        }
        
#if defined(DEBUG_DEVICE_ID) || defined(DEBUG_TIMEOUT_ISSUE)
		if ( deviceID == debugID ) {
			GetTimeString ( timeString, sizeof ( timeString ) );
			printf ( "%sHandleQueryDevices [ %i ]: [ %i ] send devicelist%s [ %d bytes -> %s ]\n", timeString, ( int ) sourceClient->socket, seqNr, sendBuffer [ 5 ] == 'c' ? "/count" : "", length, inet_ntoa ( sourceClient->addr.sin_addr ) );
		}
#endif
        CLogArgID ( "HandleQueryDevices [ %i ]: send devicelist%s [ %-15s <- %d bytes ]", (int)sourceClient->socket, sendBuffer [ 5 ] == 'c' ? "/count" : "", inet_ntoa ( sourceClient->addr.sin_addr ), length );
        
        if ( sendBuffer == resultBuffer ) {
            if ( !SendBufferOrEnqueue ( sourceClient.get (), sendBuffer, length, false, seqNr ) ) {
                CErrArgID ( "HandleQueryDevices [ %i ]: Failed to send devicelist response to IP [ %s ]", sourceClient->socket, inet_ntoa ( sourceClient->addr.sin_addr ) );
#ifdef DEBUG_DEVICE_ID
                if ( deviceID == debugID ) {
                    GetTimeString ( timeString, sizeof ( timeString ) );
                    printf ( "%sHandleQueryDevices [ %i ]: [ %i ] Failed to send devicelist response to IP [ %s ]\n", timeString, ( int ) sourceClient->socket, seqNr, inet_ntoa ( sourceClient->addr.sin_addr ) );
                }
#endif
                success = false;
            }
            else
                resultBuffer = 0;
        }
        else {
            if ( !SendBufferOrEnqueue ( sourceClient.get (), sendBuffer, length, true, seqNr  ) ) {
                CErrArgID ( "HandleQueryDevices [ %i ]: Failed to send devicelist response to IP [ %s ]", sourceClient->socket, inet_ntoa ( sourceClient->addr.sin_addr ) );
#if defined(DEBUG_DEVICE_ID) || defined(DEBUG_TIMEOUT_ISSUE)
                if ( deviceID == debugID ) {
                    GetTimeString ( timeString, sizeof ( timeString ) );
                    printf ( "%sHandleQueryDevices [ %i ]: [ %i ] Failed to send devicelist response to IP [ %s ]\n", timeString, (int)sourceClient->socket, seqNr, inet_ntoa ( sourceClient->addr.sin_addr ) );
                }
#endif
                success = false;
            }
        }
        
        free_n ( resultBuffer );
		return success;
    }
    
    
    bool MediatorDaemon::HandleQueryDevicesV5 ( const sp ( ThreadInstance ) &sourceClient, char * msg )
    {
        if ( sourceClient->version < '5' )
            return HandleQueryDevicesV4 ( sourceClient, msg );
        
        sp ( DeviceInstanceNode ) sourceDeviceSP = sourceClient->deviceSP;
        if ( !sourceDeviceSP )
			return false;
		DeviceInstanceNode * sourceDevice = sourceDeviceSP.get ();

		MediatorQueryResponse * response		= ( MediatorQueryResponse * ) msg;
		MediatorQueryMsg *	query				= ( MediatorQueryMsg * ) msg;
		MediatorQueryHeader *	header			= ( MediatorQueryHeader * ) msg;
		char			*	sendBuffer			= msg;
		int					length				= 8;
		int                 deviceID			= header->msgID;
		int                 deviceIDReq			= 0;

		bool				error				= true;
		bool                search;
		int					filterMode			= sourceClient->filterMode;
		char				subCmd;
		char            *   resultBuffer		= 0;
		char            *   resultList			= 0;
		unsigned int        resultCount         = 0;
		unsigned int        resultCapacity      = 0;
		unsigned int        resultBufferSize    = 0;

		char			*	areaName			= 0;
		char			*	appName				= 0;
		bool                appEnvGiven;
		bool                appEnvMatch;

		const unsigned int	maxDeviceCount		= ( MEDIATOR_REC_BUFFER_SIZE_MAX - DEVICES_HEADER_SIZE ) / DEVICE_PACKET_SIZE;

		msp ( string, ApplicationDevices ) * apps;

		sp ( ApplicationDevices ) appDevices = sourceDevice->rootSP;

		/// Take over filterMode	
		subCmd = header->opt1;

		if ( subCmd != MEDIATOR_OPT_NULL )
			filterMode = subCmd;

		if ( sourceClient->authLevel < 3 )
			filterMode = MEDIATOR_FILTER_AREA_AND_APP;

		if ( filterMode != sourceClient->filterMode )
			UpdateNotifyTargets ( sourceClient, filterMode );

		// Send number of available devices
		if ( header->opt0 == MEDIATOR_CMD_DEVICE_LIST_QUERY_COUNT ) {
			CVerbArgID ( "HandleQueryDevices [ %i ]: requested count", sourceClient->socket );

			length              = MEDIATOR_CMD_GET_DEVICES_COUNT_RESP_LEN;

			header->size        = length;
			header->cmdVersion  = MEDIATOR_PROTOCOL_VERSION;
			header->cmd1        = MEDIATOR_CMD_DEVICE_LIST_QUERY;
			header->opt0        = MEDIATOR_CMD_DEVICE_LIST_QUERY_COUNT;

			header->msgID       = CollectDevicesCount ( sourceDevice, filterMode );

			CVerbArgID ( "HandleQueryDevices [ %i ]: Number of devices [%u]", sourceClient->socket, header->msgID );
			error = false; goto SendResponse;
		}

		if ( header->resultCount > maxDeviceCount )
			header->resultCount = maxDeviceCount;

		resultBufferSize = ( sizeof ( DeviceInfo ) * header->resultCount ) + sizeof ( MediatorQueryResponse ) * 2;

		resultBuffer    = ( char * ) malloc ( resultBufferSize );
		if ( !resultBuffer ) {
			CErrArg ( "HandleQueryDevices: Failed to allocate [ %u bytes ] memory for [ %i ] devices.", resultBufferSize, header->resultCount );
			goto Finish;
		}

		resultCapacity  = header->resultCount;
		resultList = ( resultBuffer + DEVICES_HEADER_SIZE );


		subCmd              = header->opt0;

		search              = ( subCmd == MEDIATOR_CMD_DEVICE_LIST_QUERY_SEARCH );

		if ( subCmd == MEDIATOR_OPT_DEVICE_LIST_DEVICE_ID || search ) {
			deviceIDReq     = header->deviceID;

			appName         = query->appArea;
			areaName        = appName + header->sizes [ 0 ];

			if ( header->sizes [ 0 ] > 1 && header->sizes [ 1 ] > 1
				&& header->sizes [ 0 ] < MAX_NAMEPROPERTY && header->sizes [ 1 ] < MAX_NAMEPROPERTY &&
				*appName && *areaName )
			{
				areaName [ header->sizes [ 1 ] ] = 0;

				CVerbArgID ( "HandleQueryDevices [ %i ]: Query for deviceID  [%u ] [ %s / %s ]", sourceClient->socket, deviceIDReq, areaName, appName );
			}
			else {
				appName     = 0;
				areaName    = 0;

				CVerbArgID ( "HandleQueryDevices [ %i ]: Query for deviceID [ %u ] of default appEnv [ %s / %s ]", sourceClient->socket, deviceIDReq, sourceDevice->info.appName, sourceDevice->info.areaName );
			}
		}

		CVerbArgID ( "HandleQueryDevices [ %i ]: requestCount [%u] filterMode [%i]", sourceClient->socket, header->resultCount, filterMode );


		/// Lets start with the devices within the same appEnv	
		appEnvGiven = ( appName && areaName && *appName && *areaName );

		if ( appEnvGiven )
			appEnvMatch = ( !strncmp ( appName, sourceDevice->info.appName, sizeof ( sourceDevice->info.appName ) )
				&& !strncmp ( areaName, sourceDevice->info.areaName, sizeof ( sourceDevice->info.areaName ) ) );
		else
			appEnvMatch = false;

		if ( !appEnvGiven || appEnvMatch )
		{
			if ( !appDevices->count ) {
				if ( appEnvMatch )
					goto Finish;
			}

			if ( !CollectDevices ( search, deviceIDReq, appDevices.get (), resultList, resultCapacity, resultCount ) )
				goto Finish;
		}


		if ( filterMode < 1 ) {
            /// No filtering, get them all
            if ( !areas.LockRead ( "HandleQueryDevices" ) )
				goto Finish;

			vsp ( AreaApps ) searchAreas;

			/// Iterate over all areas
			for ( msp ( string, AreaApps )::iterator it = areas.list.begin (); it != areas.list.end (); ++it )
			{
				if ( !it->second )
					continue;

				if ( appEnvGiven ) {
					if ( strncmp ( areaName, it->second->name.c_str (), MAX_NAMEPROPERTY ) )
						continue;
				}
				searchAreas.push_back ( it->second );
			}

            areas.UnlockRead ( "HandleQueryDevices" );
			for ( vsp ( AreaApps )::iterator its = searchAreas.begin (); its != searchAreas.end (); ++its )
			{
				AreaApps * areaApps	= its->get ();

                if ( !areaApps->LockRead ( "HandleQueryDevices" ) )
					continue;
				apps = &areaApps->apps;

				for ( msp ( string, ApplicationDevices )::iterator ita = apps->begin (); ita != apps->end (); ++ita )
				{
					appDevices = ita->second;
					if ( !appDevices || appDevices.get () == sourceDevice->rootSP.get () )
						continue;

					if ( appEnvGiven )
						appEnvMatch = ( !strncmp ( appName, appDevices->name.c_str (), MAX_NAMEPROPERTY )
							&& !strncmp ( areaName, areaApps->name.c_str (), MAX_NAMEPROPERTY ) );
					else
						appEnvMatch = false;

					if ( !appDevices->count ) {
						if ( appEnvMatch )
                        {
                            areaApps->UnlockRead ( "HandleQueryDevices" );
							goto Finish;
						}
					}
					else
					{
						if ( appEnvGiven && !appEnvMatch )
							continue;

						if ( !CollectDevices ( search, deviceIDReq, appDevices.get (), resultList, resultCapacity, resultCount ) )
                        {
                            areaApps->UnlockRead ( "HandleQueryDevices" );
							goto Finish;
						}
					}
				}

                areaApps->UnlockRead ( "HandleQueryDevices" );
			}
		}
		else if ( filterMode < 2 ) {
			/// Get number of devices of other application environments within the same area
			char * area = ( areaName && *areaName ? areaName : sourceDevice->info.areaName );

			string pareaName ( area );

			sp ( AreaApps ) areaApps;

            if ( !areas.LockRead ( "HandleQueryDevices" ) )
				goto Finish;

			const msp ( string, AreaApps )::iterator areaIt = areas.list.find ( pareaName );

			if ( areaIt != areas.list.end () )
				areaApps = areaIt->second;

            areas.UnlockRead ( "HandleQueryDevices" );

            if ( !areaApps || !areaApps->LockRead ( "HandleQueryDevices" ) )
            {
				CLogArg ( "HandleQueryDevices [ %i ]: Area [%s] not found.", sourceClient->socket, areaName );
				goto Finish;
			}

			apps = &areaApps->apps;

			for ( msp ( string, ApplicationDevices )::iterator ita = apps->begin (); ita != apps->end (); ++ita )
			{
				appDevices = ita->second;
				if ( !appDevices || appDevices == sourceDevice->rootSP )
					continue;

				if ( appEnvGiven )
					appEnvMatch = ( !strncmp ( appName, appDevices->name.c_str (), MAX_NAMEPROPERTY )
						&& !strncmp ( areaName, areaApps->name.c_str (), MAX_NAMEPROPERTY ) );
				else
					appEnvMatch = false;

				if ( !appDevices->count )
				{
                    if ( appEnvMatch ) {
                        areaApps->UnlockRead ( "HandleQueryDevices" );
						goto Finish;
					}
				}
				else {
					if ( appEnvGiven && !appEnvMatch )
						continue;

					if ( !CollectDevices ( search, deviceIDReq, appDevices.get (), resultList, resultCapacity, resultCount ) )
						break;
				}
            }
            areaApps->UnlockRead ( "HandleQueryDevices" );
		}

	Finish:
		if ( !resultBuffer )
			goto SendResponse;

		error       = false;
		response    = ( MediatorQueryResponse * ) resultBuffer;
		sendBuffer  = ( char * ) response;

		response->cmd1                              = MEDIATOR_CMD_DEVICE_LIST_QUERY_RESPONSE;
		response->cmd0                              = MEDIATOR_CMD_DEVICE_LIST_QUERY;
		response->deviceHead.deviceCountAvailable   = resultCount;
		response->deviceHead.startIndex             = 0;
		response->deviceHead.deviceCount            = resultCount;

		length = ( int ) ( resultList - resultBuffer );
		response->size = length;

	SendResponse:
		if ( error ) {
			sendBuffer      = msg;
			header->cmd1    = MEDIATOR_CMD_DEVICE_LIST_QUERY;
			header->opt0    = MEDIATOR_CMD_DEVICE_LIST_QUERY_ERROR;
			length = header->size = 8;
		}

		CLogArgID ( "HandleQueryDevices [ %i ]: send devicelist%s [%d bytes -> %s]", sourceClient->socket, sendBuffer [ 5 ] == 'c' ? "/count" : "", length, inet_ntoa ( sourceClient->addr.sin_addr ) );

		if ( sendBuffer == resultBuffer ) {
			if ( !SendBufferOrEnqueue ( sourceClient.get (), sendBuffer, length, false, 0 ) ) {
				CErrArgID ( "HandleQueryDevices [ %i ]: Failed to send devicelist response to IP [%s]", sourceClient->socket, inet_ntoa ( sourceClient->addr.sin_addr ) );
				free_n ( resultBuffer );
				return false;
			}
		}
		else {
			if ( !SendBufferOrEnqueue ( sourceClient.get (), sendBuffer, length, true, 0 ) ) {
				CErrArgID ( "HandleQueryDevices [ %i ]: Failed to send devicelist response to IP [%s]", sourceClient->socket, inet_ntoa ( sourceClient->addr.sin_addr ) );
				free_n ( resultBuffer );
				return false;
			}
			free_n ( resultBuffer );
		}

		return true;
	}


	bool MediatorDaemon::HandleQueryDevicesV4 ( const sp ( ThreadInstance ) &sourceClient, char * msg )
	{
		sp ( DeviceInstanceNode ) sourceDeviceSP = sourceClient->deviceSP;
		if ( !sourceDeviceSP )
			return false;
		DeviceInstanceNode * sourceDevice = sourceDeviceSP.get ();

		MediatorQueryResponse * response		= ( MediatorQueryResponse * ) msg;
		MediatorQueryMsgV4 *	query				= ( MediatorQueryMsgV4 * ) msg;
		char			*	sendBuffer			= msg;
		int					length				= 8;
		unsigned int		availableDevices	= 0;
		int                 deviceID			= query->msgID;
		int                 deviceIDReq			= 0;

		bool				error				= true;
		unsigned int		startIndex;
		int					filterMode			= sourceClient->filterMode;
		char				subCmd;
		DeviceInfoNodeV4*   resultList			= 0;
		unsigned int        resultCount         = 0;

		const char		*	areaName = sourceDevice->info.areaName;
		const char		*	appName  = sourceDevice->info.appName;

		msp ( string, ApplicationDevices ) * apps;

		sp ( ApplicationDevices ) appDevices = sourceDevice->rootSP;

		/// Take over filterMode	
		subCmd = query->opt1;

		if ( subCmd != MEDIATOR_OPT_NULL ) {
			filterMode = subCmd;
		}

		if ( sourceClient->authLevel < 3 )
			filterMode = MEDIATOR_FILTER_AREA_AND_APP;

		if ( filterMode != sourceClient->filterMode )
			UpdateNotifyTargets ( sourceClient, filterMode );

		// Send number of available devices
		if ( query->opt0 == MEDIATOR_CMD_DEVICE_LIST_QUERY_COUNT ) {
			CVerbArgID ( "HandleQueryDevices [ %i ]: requested count", sourceClient->socket );

			length = MEDIATOR_CMD_GET_DEVICES_COUNT_RESP_LEN;

			query->size = length;
			query->cmdVersion = MEDIATOR_PROTOCOL_VERSION;
			query->cmd1 = MEDIATOR_CMD_DEVICE_LIST_QUERY;
			query->opt0 = MEDIATOR_CMD_DEVICE_LIST_QUERY_COUNT;

			query->msgID = CollectDevicesCount ( sourceDevice, filterMode );

			CVerbArgID ( "HandleQueryDevices [ %i ]: Number of devices [%u]", sourceClient->socket, query->msgID );
			error = false; goto SendResponse;
		}

		availableDevices = appDevices->count;

		startIndex = query->startIndex;


		subCmd = query->opt0;

		if ( subCmd == MEDIATOR_OPT_DEVICE_LIST_DEVICE_ID ) {
			deviceIDReq = query->deviceID;
			if ( *query->areaName && *query->appName ) {
				areaName = query->areaName;
				appName = query->appName;

				CVerbArgID ( "HandleQueryDevices [ %i ]: Query for deviceID [%u] [%s/%s]", sourceClient->socket, deviceIDReq, areaName, appName );
			}
			else {
				areaName = sourceDevice->info.areaName;
				appName = sourceDevice->info.appName;

				CVerbArgID ( "HandleQueryDevices [ %i ]: Query for deviceID [%u] of default appEnv [%s/%s]", sourceClient->socket, deviceIDReq, areaName, appName );
			}
		}

		CVerbArgID ( "HandleQueryDevices [ %i ]: start index [%u] filterMode [%i]", sourceClient->socket, startIndex, filterMode );

		/// Lets start with the devices within the same appEnv	

		if ( appDevices->count && !CollectDevicesV4 ( sourceClient.get (), startIndex, deviceIDReq, areaName, appName,
			appDevices.get (), resultList, resultCount ) )
			goto Finish;

		if ( filterMode < 1 ) {
            /// No filtering, get them all
            if ( !areas.LockRead ( "HandleQueryDevices" ) )
				goto Finish;

			vsp ( AreaApps ) searchAreas;

			/// Iterate over all areas
			for ( msp ( string, AreaApps )::iterator it = areas.list.begin (); it != areas.list.end (); ++it )
			{
				searchAreas.push_back ( it->second );
			}

            areas.UnlockRead ( "HandleQueryDevices" );

			for ( vsp ( AreaApps )::iterator its = searchAreas.begin (); its != searchAreas.end (); ++its )
			{
				AreaApps * areaApps	= its->get ();

                if ( !areaApps || !areaApps->LockRead ( "HandleQueryDevices" ) )
					continue;
				apps = &areaApps->apps;

				for ( msp ( string, ApplicationDevices )::iterator ita = apps->begin (); ita != apps->end (); ++ita )
				{
					appDevices = ita->second;
					if ( !appDevices || appDevices.get () == sourceDevice->rootSP.get () || !appDevices->count )
						continue;

					availableDevices += appDevices->count;

					if ( !CollectDevicesV4 ( sourceClient.get (), startIndex, deviceIDReq, areaName, appName,
						appDevices.get (), resultList, resultCount ) )
                    {
                        areaApps->UnlockRead ( "HandleQueryDevices" );
						goto Finish;
					}
				}

                areaApps->UnlockRead ( "HandleQueryDevices" );
			}
		}
		else if ( filterMode < 2 ) {
			/// Get number of devices of other application environments within the same area
			string pareaName ( areaName );

			sp ( AreaApps ) areaApps;

            if ( !areas.LockRead ( "HandleQueryDevices" ) )
				goto Finish;

			const msp ( string, AreaApps )::iterator areaIt = areas.list.find ( pareaName );

			if ( areaIt != areas.list.end () )
				areaApps = areaIt->second;

            areas.UnlockRead ( "HandleQueryDevices" );

            if ( !areaApps || !areaApps->LockRead ( "HandleQueryDevices" ) )
            {
				CLogArg ( "HandleQueryDevices [ %i ]: Area [%s] not found.", sourceClient->socket, areaName );
				goto Finish;
			}

			apps = &areaApps->apps;

			for ( msp ( string, ApplicationDevices )::iterator ita = apps->begin (); ita != apps->end (); ++ita )
			{
				appDevices = ita->second;
				if ( !appDevices || appDevices == sourceDevice->rootSP || !appDevices->count )
					continue;

				availableDevices += appDevices->count;

				if ( !CollectDevicesV4 ( sourceClient.get (), startIndex, deviceIDReq, areaName, appName,
					appDevices.get (), resultList, resultCount ) ) {
					break;
				}
            }
            areaApps->UnlockRead ( "HandleQueryDevices" );
		}

	Finish:
		error = false;
		response = ( MediatorQueryResponse * ) FlattenDeviceListV4 ( resultList, resultCount );
		if ( !response ) {
			query->size = length;
			query->cmd1 = MEDIATOR_CMD_DEVICE_LIST_QUERY;
			query->opt0 = MEDIATOR_CMD_DEVICE_LIST_QUERY_ERROR;
			goto SendResponse;
		}


		sendBuffer = ( char * ) response;

		//response->cmd0 = MEDIATOR_CMD_DEVICE_LIST_QUERY;
		response->cmd1 = MEDIATOR_CMD_DEVICE_LIST_QUERY_RESPONSE;

		response->cmd0 = MEDIATOR_CMD_DEVICE_LIST_QUERY;
		response->deviceHead.deviceCountAvailable = availableDevices;
		response->deviceHead.startIndex = startIndex;
		response->deviceHead.deviceCount = resultCount;

		length = DEVICES_HEADER_SIZE + ( resultCount * DEVICE_PACKET_SIZE );
		response->size = length;

	SendResponse:
		if ( error ) {
			sendBuffer = msg;
			query->cmd1 = MEDIATOR_CMD_DEVICE_LIST_QUERY;
			query->opt0 = MEDIATOR_CMD_DEVICE_LIST_QUERY_ERROR;
			length = query->size = 8;
		}

		CLogArgID ( "HandleQueryDevices [ %i ]: send devicelist%s [ %d bytes -> %s ]", sourceClient->socket, sendBuffer [ 5 ] == 'c' ? "/count" : "", length, inet_ntoa ( sourceClient->addr.sin_addr ) );
        
        if ( !SendBufferOrEnqueue ( sourceClient.get (), sendBuffer, length, true, 0 ) ) {
            CErrArgID ( "HandleQueryDevices [ %i ]: Failed to send devicelist response to IP [ %s ]", sourceClient->socket, inet_ntoa ( sourceClient->addr.sin_addr ) );
            
            if ( sendBuffer != msg )
                free ( sendBuffer );
            return false;
        }
        
        if ( sendBuffer != msg )
			free ( sendBuffer );

		return true;
	}



	void MediatorDaemon::HandleCLSGenHelp ( ThreadInstance * client )
	{
		char buffer [ ENVIRONS_MAX_KEYSIZE ];

		int deviceID = client->deviceID;
		CLogID ( "HandleCLSGenHelp: CLS cert/key help requested." );

		char * cert = 0, *key = 0;

		if ( !GenerateCertificate ( &key, &cert ) || !cert || !key ) {
			return;
		}

		unsigned int certlen = *( ( unsigned int * ) cert ) & 0xFFFF;
		unsigned int keylen = *( ( unsigned int * ) key ) & 0xFFFF;

		buffer [ 4 ] = MEDIATOR_PROTOCOL_VERSION;
		buffer [ 5 ] = MEDIATOR_CMD_HELP_TLS_GEN;
		buffer [ 6 ] = MEDIATOR_OPT_NULL;
		buffer [ 7 ] = MEDIATOR_OPT_NULL;

		int sentSize = certlen + keylen + 16;
		//*((unsigned int *) buffer) = (unsigned int) sentSize;
		memcpy ( buffer, &sentSize, sizeof ( sentSize ) ); // to prevent strict-alias bugs (due to compiler opts)

		memcpy ( buffer + 8, cert, certlen + 4 );
		memcpy ( buffer + 8 + certlen + 4, key, keylen + 4 );

		int sentBytes = SendBuffer ( client, buffer, sentSize );

		if ( sentBytes != sentSize ) {
			CErrArg ( "[0x%X].HandleCLSGenHelp: Failed to send certificate/key to [%s]!", client->deviceID, inet_ntoa ( client->addr.sin_addr ) );
		}

		free ( cert );
		free ( key );
	}


	void MediatorDaemon::HandleCertSign ( ThreadInstance * client, char * msg )
	{

		char buffer [ ENVIRONS_MAX_KEYSIZE ];

		int deviceID = client->deviceID;
		CLogID ( "HandleCertSign: Cert signature requested." );

		char * cert = 0;

		if ( !SignCertificate ( privKey, privKeySize, msg + 8, &cert ) || !cert ) {
			CLogID ( "HandleCertSign: Signing failed." );

			/// We draw on the assumption that signing may be not implemented yet. Generate valid cert/keys and send that to the client
			HandleCLSGenHelp ( client );
			return;
		}

		unsigned int certlen = *( ( unsigned int * ) cert ) & 0xFFFF;

		buffer [ 4 ] = MEDIATOR_PROTOCOL_VERSION;
		buffer [ 5 ] = MEDIATOR_CMD_HELP_TLS_GEN;
		buffer [ 6 ] = MEDIATOR_OPT_SIGNED_CERT_RESPONSE;
		buffer [ 7 ] = MEDIATOR_OPT_NULL;

		int sentSize = certlen + 8;
		//*((unsigned int *) buffer) = (unsigned int) sentSize;
		memcpy ( buffer, &sentSize, sizeof ( sentSize ) );  // to prevent strict-alias bugs (due to compiler opts)

		memcpy ( buffer + 8, cert, certlen + 4 );

		int sentBytes = SendBuffer ( client, buffer, sentSize );

		if ( sentBytes != sentSize ) {
			CErrArg ( "[0x%X].HandleCertSign: Failed to send signed certificate to [%s]!", client->deviceID, inet_ntoa ( client->addr.sin_addr ) );
		}
		free ( cert );
	}


	void MediatorDaemon::HandleDeviceFlagSet ( ThreadInstance * client, char * msgDec )
	{
		MediatorStatusMsgExt	tmp;
		MediatorStatusMsg    *  msg = ( MediatorStatusMsg * ) msgDec;

		sp ( ApplicationDevices )	appDevices;
		sp ( ThreadInstance )		destClient;

		sp ( DeviceInstanceNode )	srcDevice	= client->deviceSP;
		if ( !srcDevice )
			return;

		DeviceInstanceNode *		device	= 0;
		char * areaName = 0;
		char * appName  = 0;

		bool ext = ( msg->sizes [ 0 ] > 1 && msg->sizes [ 1 ] > 1 && msg->sizes [ 0 ] < MAX_NAMEPROPERTY && msg->sizes [ 1 ] < MAX_NAMEPROPERTY );
		if ( ext ) {
			appName   = ( ( MediatorStatusMsgExt * ) msg )->appArea;
			areaName  = appName + msg->sizes [ 0 ];

			if ( !*appName || !*areaName ) {
				appName  = srcDevice->info.appName;
				areaName = srcDevice->info.areaName;
			}
            
            DeviceInstanceNode * src = srcDevice.get ();
            
            if ( !strncmp ( areaName, src->info.areaName, sizeof ( src->info.areaName ) )
                && !strncmp ( appName, src->info.appName, sizeof ( src->info.appName ) ) )
            {
                ext = false;
                appDevices = srcDevice->rootSP;
            }
            else
                appDevices   = GetApplicationDevices ( appName, areaName );
		}
		else {
			appDevices = srcDevice->rootSP;
		}

		if ( !appDevices || !appDevices->LockRead ( "HandleDeviceFlagSet" ) )
			return;

		device = GetDeviceInstance ( msg->deviceID, appDevices->devices );
		if ( device )
			destClient = device->clientSP;

		appDevices->UnlockRead ( "HandleDeviceFlagSet" );

		if ( !destClient )
			return;

		msg->deviceID = client->deviceID;

		if ( ext ) {
			memcpy ( &tmp, msg, sizeof ( MediatorStatusMsg ) + 4 );

			if ( !BuildAppAreaField ( tmp.header.sizes, srcDevice->info.appName, srcDevice->info.areaName, false ) )
				return;
			msg = ( MediatorStatusMsg * ) &tmp;
		}
		else {
			msg->sizes [ 0 ] = 1;
			msg->sizes [ 1 ] = 1;

			char * m = ( char * ) ( msg->sizes + 2 ); *m = 0; m++; *m = 0;
		}

		msg->size = sizeof ( MediatorStatusMsg ) + msg->sizes [ 0 ] + msg->sizes [ 1 ];
        
        if ( !SendBufferOrEnqueue ( destClient.get (), msg, msg->size, true, 0 ) ) {
            CErrArg ( "[ 0x%X ].HandleDeviceFlagSet [ %i ]: Failed to send device flags to device IP [ %s ]!", client->deviceID, client->socket, inet_ntoa ( destClient->addr.sin_addr ) );
        }
    }


	bool MediatorDaemon::HandleSTUNRequest ( ThreadInstance * sourceClient, char * msg )
	{
		char buffer [ 32 ];

		sp ( DeviceInstanceNode ) sourceDeviceSP = sourceClient->deviceSP;
		if ( !sourceDeviceSP )
			return false;
		DeviceInstanceNode * sourceDevice = sourceDeviceSP.get ();

		// SourceID -> DestID
        unsigned int * pUI;
        int * pI		= ( int * ) ( msg + 8 );
		int SourceID	= *( ( int * ) pI ); pI++;
		int DestID		= *( ( int * ) pI );

		CVerbArg ( "[ 0x%X ].HandleSTUNRequest [ %i ]: TCPrec -> [0x%X]", SourceID, sourceClient->socket, DestID );

		// find the source client
		sp ( ThreadInstance ) destClient;

		sp ( ApplicationDevices ) rootSP = sourceDevice->rootSP;

        if ( !rootSP || !rootSP->LockRead ( "HandleSTUNRequest" ) )
			return false;

		DeviceInstanceNode * device = GetDeviceInstance ( DestID, rootSP->devices );

		if ( device )
			destClient = device->clientSP;

        rootSP->UnlockRead ( "HandleSTUNRequest" );

		if ( !destClient ) {
			CWarnArg ( "[ 0x%X ].HandleSTUNRequest [ %i ] -> Destination device -> [0x%X] not found.", SourceID, sourceClient->socket, DestID );
			return false;
		}

		buffer [ 4 ] = MEDIATOR_CMD_STUN;
		buffer [ 5 ] = MEDIATOR_OPT_NULL;
		buffer [ 6 ] = MEDIATOR_OPT_NULL;
		buffer [ 7 ] = MEDIATOR_OPT_NULL;

		pUI = ( unsigned int * ) buffer;
		*pUI = MEDIATOR_STUN_RESP_SIZE; pUI += 2;

		*( ( int * ) pUI ) = SourceID; pUI++;
		*pUI = ( unsigned int ) sourceClient->addr.sin_addr.s_addr; pUI++;
		*pUI = sourceClient->portUdp;

		CLogArg ( "[ 0x%X ].HandleSTUNRequest [ %i ]: send STUN request to device IP [ %s ]!", DestID, sourceClient->socket, inet_ntoa ( destClient->addr.sin_addr ) );
        
        if ( !SendBufferOrEnqueue ( destClient.get (), buffer, MEDIATOR_STUN_RESP_SIZE, true, 0 ) ) {
            CErrArg ( "[ 0x%X ].HandleSTUNRequest [ %i ]: Failed to send STUN request to device IP [ %s ]!", DestID, sourceClient->socket, inet_ntoa ( destClient->addr.sin_addr ) );
            //return false;
        }
        
		buffer [ 3 ] = '-';
		pUI = ( unsigned int * ) buffer; pUI++;
		*( ( int * ) pUI ) = DestID; pUI++;
		*pUI = ( unsigned int ) destClient->addr.sin_addr.s_addr; pUI++;
		*pUI = destClient->portUdp; pUI++;
		*pUI = 0;

		CLogArg ( "[ 0x%X ].HandleSTUNRequest [ %i ]: send STUN reply to device IP [ %s ]!", DestID, sourceClient->socket, inet_ntoa ( sourceClient->addr.sin_addr ) );
        
        if ( !SendBufferOrEnqueue ( sourceClient, buffer, MEDIATOR_STUN_RESP_SIZE, true, 0 ) ) {
            CErrArg ( "[ 0x%X ].HandleSTUNRequest [ %i ]: Failed to send STUN reply to device IP [ %s ]!", DestID, sourceClient->socket, inet_ntoa ( sourceClient->addr.sin_addr ) );
            return false;
        }
        return true;
	}

    
    bool MediatorDaemon::HandleSTUNRequest ( ThreadInstance * destClient, int sourceID, const char * areaName, const char * appName, unsigned int IPe, unsigned int Porte )
    {
        if ( destClient->version < '5' )
            return HandleSTUNRequest ( destClient, sourceID, areaName, appName, IPe, Porte );
        
        STUNReqReqPacket *	request = (STUNReqReqPacket *) calloc ( 1, sizeof ( STUNReqReqPacket ) );
        if ( !request )
            return false;
        
        STUNReqReqHeader *	header = &request->header;
        
        CVerbArg ( "[0x%X].HandleSTUNRequest: UDPrec -> TCPsend [0x%X]", sourceID, destClient->deviceID );
        
        header->size = sizeof ( STUNReqReqPacket );
        header->ident [ 0 ] = MEDIATOR_CMD_STUN;
        header->ident [ 1 ] = MEDIATOR_OPT_NULL;
        header->ident [ 2 ] = MEDIATOR_OPT_NULL;
        header->ident [ 3 ] = MEDIATOR_OPT_NULL;
        
        header->deviceID = sourceID;
        header->IPe = IPe;
        header->Porte = ( unsigned short ) Porte;
        
        sp ( DeviceInstanceNode ) deviceSP = destClient->deviceSP;
        if ( deviceSP ) {
            header->IPi     = deviceSP->info.ip;
            header->Porti   = deviceSP->info.udpPort;
        }
        header->sizes [ 0 ] = 1;
        header->sizes [ 1 ] = 1;
        
        if ( areaName && *areaName && appName && *areaName ) {
            if ( !BuildAppAreaField ( header->sizes, appName, areaName, false ) ) {
                free ( request );
                return false;
            }
        }
        
        header->size = sizeof ( STUNReqReqHeader ) + header->sizes [ 0 ] + header->sizes [ 1 ];
        
        CLogArg ( "[0x%X].HandleSTUNRequest: Send STUN request to device IP [ %s ] Port [ %u / %u ]!", destClient->deviceID, inet_ntoa ( destClient->addr.sin_addr ), Porte, header->Porti );
        
        if ( !SendBufferOrEnqueue ( destClient, request, header->size, false, 0 ) )
        {
            CErrArg ( "[0x%X].HandleSTUNRequest: Failed to send STUN request to device IP [ %s ]!", destClient->deviceID, inet_ntoa ( destClient->addr.sin_addr ) );
            free ( request );
            return false;
        }
        return true;
    }


	bool MediatorDaemon::HandleSTUNRequestV4 ( ThreadInstance * destClient, int sourceID, const char * areaName, const char * appName, unsigned int IPe, unsigned int Porte )
	{
		STUNReqReqPacketV4	request;
		Zero ( request );

		CVerbArg ( "[ 0x%X ].HandleSTUNRequest: UDPrec -> TCPsend [0x%X]", sourceID, destClient->deviceID );

		request.size = sizeof ( request );
		memcpy ( request.ident, "y;;;", 4 );

		request.deviceID = sourceID;
		request.IPe = IPe;
		request.Porte = ( unsigned short ) Porte;

		sp ( DeviceInstanceNode ) deviceSP = destClient->deviceSP;
		if ( deviceSP ) {
			request.IPi     = deviceSP->info.ip;
			request.Porti   = deviceSP->info.udpPort;
		}

		strlcpy ( request.areaName, areaName, sizeof ( request.areaName ) );
		strlcpy ( request.appName, appName, sizeof ( request.appName ) );

		CLogArg ( "[ 0x%X ].HandleSTUNRequest: Send STUN request to device IP [%s] Port [%u/%u]!", destClient->deviceID, inet_ntoa ( destClient->addr.sin_addr ), Porte, request.Porti );
        
        if ( !SendBufferOrEnqueue ( destClient, &request, sizeof ( request ), true, 0 ) )
        {
            CErrArg ( "[ 0x%X ].HandleSTUNRequest: Failed to send STUN request to device IP [%s]!", destClient->deviceID, inet_ntoa ( destClient->addr.sin_addr ) );
            return false;
        }
        
        return true;
	}
    
	
	bool MediatorDaemon::NotifySTUNTRegRequest ( ThreadInstance * client, int sourceID, const char * appName, const char * areaName, char channelType, unsigned int token )
	{
		int deviceID = client->deviceID;

		CVerbID ( "NotifySTUNTRegRequest" );

		STUNTRegReqPacketV8	* req = ( STUNTRegReqPacketV8 * ) calloc ( 1, sizeof ( STUNTRegReqPacketV8 ) + sizeof ( AppAreaBuffer ) );
		if ( !req )
			return false;

		/// Notify a stunt socket registration request

		req->version = MEDIATOR_PROTOCOL_VERSION;

		req->ident [ 0 ] = MEDIATOR_CMD_MEDIATOR_NOTIFY;
		req->ident [ 1 ] = MEDIATOR_OPT_NULL;
		req->ident [ 2 ] = MEDIATOR_OPT_NULL;

		req->notify = NOTIFY_MEDIATOR_SRV_STUNT_REG_REQ;

		req->channelType	= channelType;
		req->deviceID		= sourceID;
        req->token          = token;

		if ( areaName && appName ) {
			if ( !BuildAppAreaField ( req->sizes, appName, areaName, false ) ) {
				return false;
			}
		}
		else {
			req->sizes [ 0 ] = 1; req->sizes [ 1 ] = 1;
		}

		req->size    = sizeof ( STUNTRegReqPacketV8 ) + req->sizes [ 0 ] + req->sizes [ 1 ];

		CLogArgID ( "NotifySTUNTRegRequest: Send stunt socket register request to device [ %s ]", inet_ntoa ( client->addr.sin_addr ) );
        
        // Push to priority queue (fake a seqNr)
        if ( !SendBufferOrEnqueue ( client, req, req->size, false, client->seqNr - 1 ) ) {
            CErrArgID ( "NotifySTUNTRegRequest: Failed to send stunt socket register request to device IP [ %s ]!", inet_ntoa ( client->addr.sin_addr ) ); LogSocketError ();
            free ( req );
            return false;
        }
        return true;
	}


    bool MediatorDaemon::NotifySTUNTRegRequest ( ThreadInstance * client )
    {
        int deviceID = client->deviceID;
        
        CVerbID ( "NotifySTUNTRegRequest" );
        
        STUNTRegReqPacket	* req = (STUNTRegReqPacket *) calloc ( 1, sizeof ( STUNTRegReqPacket ) );
        if ( !req )
            return false;
        
        /// Notify a stunt socket registration request
        
        req->size    = MEDIATOR_CMD_MEDIATOR_NOTIFY_SIZE;
        req->version = MEDIATOR_PROTOCOL_VERSION;
        
        req->ident [ 0 ] = MEDIATOR_CMD_MEDIATOR_NOTIFY;
        req->ident [ 1 ] = MEDIATOR_OPT_NULL;
        req->ident [ 2 ] = MEDIATOR_OPT_NULL;
        
        req->notify = NOTIFY_MEDIATOR_SRV_STUNT_REG_REQ;
        
        CLogArgID ( "NotifySTUNTRegRequest: Send stunt socket register request to device [ %s ]", inet_ntoa ( client->addr.sin_addr ) );
        
        if ( !SendBufferOrEnqueue ( client, req, MEDIATOR_CMD_MEDIATOR_NOTIFY_SIZE, false, 0 ) ) {
            CErrArgID ( "NotifySTUNTRegRequest: Failed to send stunt socket register request to device IP [ %s ]!", inet_ntoa ( client->addr.sin_addr ) ); LogSocketError ();
            free ( req );
            return false;
        }
        return true;
    }
    

	void MediatorDaemon::HandleSTUNTClear ( ThreadInstance * client, StuntClearTarget * msg )
	{
		char				keyID [ 36 + sizeof ( AppAreaBuffer ) ];
		char			*	key			= keyID + 4;
		char			*	appName		= 0;
		char			*	areaName	= 0;

		if ( msg->sizes [ 0 ] > 1 && msg->sizes [ 0 ] < MAX_NAMEPROPERTY )
			areaName = (char *) (msg->sizes + 2);

		if ( msg->sizes [ 1 ] > 1 && msg->sizes [ 1 ] < MAX_NAMEPROPERTY )
			areaName = ( char * ) areaName + msg->sizes [ 0 ];

		if ( !BuildAppAreaID ( ( char * ) keyID, msg->deviceID, appName, areaName, MEDIATOR_STUNT_CHANNEL_MAIN, 0 ) ) {
			CErr ( "HandleSTUNTClear: Failed to build app area id!" );
			return;
		}

		sp ( StuntRegisterContext ) toDispose;

		if ( !LockAcquireA ( client->stuntSocketLock, "HandleSTUNTClear" ) )
			return;

		const std::map < std::string, sp ( StuntRegisterContext ) >::iterator &foundIt = client->stuntSocketsLog.find ( key );

		if ( foundIt != client->stuntSocketsLog.end () )
		{
			toDispose = foundIt->second;

			client->stuntSocketsLog.erase ( foundIt );
		}

		LockReleaseA ( client->stuntSocketLock, "HandleSTUNTClear" );
	}


	bool MediatorDaemon::HandleSTUNTRequest ( const sp ( ThreadInstance ) &sourceClientSP, STUNTReqPacketV8 * req )
	{
		ThreadInstance * sourceClient = sourceClientSP.get ();

		sp ( DeviceInstanceNode ) sourceDeviceSP = sourceClient->deviceSP;
		if ( !sourceDeviceSP )
			return false;
		DeviceInstanceNode * sourceDevice = sourceDeviceSP.get ();

		STUNTRespReqPacketV8 response;
		Zero ( response );

		STUNTRespReqHeaderV8 * header			= &response.header;

		STUNTRespPacketV6	* reqResponse		= ( STUNTRespPacketV6 * ) &response;

		unsigned int        seqNr           = req->header.seqNr;
		sourceClient->seqNr = seqNr;

		unsigned int		sendSize		= 0;
		unsigned int		IP				= 0, IPe = 0;

		char				channelType		= req->header.channel;
		bool				extResp			= ( channelType != ';' );

		unsigned short		portSource		= 0;
		unsigned short		portDest		= 0;
		int                 deviceID		= sourceDevice->info.deviceID; // *pUI;
		int                 destID          = req->header.deviceID;
        
        bool				alreadyInvoked	= false;
        bool				invalidToken	= false;
		bool				unlockAppDevice	= false;
		int					status			= 0;
		const char  *		areaName        = 0;
		const char  *		appName         = 0;
		DeviceInstanceNode *	destDevice		= 0;
		sp ( DeviceInstanceNode ) destDeviceSP	= 0;

		char				sourceStuntID [ sizeof( AppAreaBuffer ) + 36 ];
		char				destStuntID [ sizeof ( AppAreaBuffer ) + 36 ];

		std::map < std::string, sp ( StuntRegisterContext ) >::iterator foundIt;

		// Look for the destination client
		sp ( ThreadInstance )	destClient		= 0;
		sp ( ApplicationDevices ) appDevices	= 0;

		if ( req->header.sizes [ 0 ] > MAX_NAMEPROPERTY || req->header.sizes [ 1 ] > MAX_NAMEPROPERTY )
			return false;

		appName		= req->appArea;
		areaName	= appName + req->header.sizes [ 0 ];

		if ( !req->header.sizes [ 0 ] || !req->header.sizes [ 1 ] || !*areaName || !*appName ) {
			appName = 0; areaName = 0;
			appDevices = sourceDevice->rootSP;
		}
		else {
			appDevices		= GetApplicationDevices ( appName, areaName );
			unlockAppDevice = true;
		}

		if ( !appDevices )
			goto Quit;

        if ( !appDevices->LockRead ( "HandleSTUNTRequest" ) )
        {
			if ( unlockAppDevice )
				UnlockApplicationDevices ( appDevices.get () );
			goto Quit;
		}

		if ( unlockAppDevice )
			UnlockApplicationDevices ( appDevices.get () );

		destDevice = GetDeviceInstance ( destID, appDevices->devices );
		if ( destDevice ) {
			destDeviceSP	= destDevice->baseSP;
			destClient		= destDevice->clientSP;
		}

        appDevices->UnlockRead ( "HandleSTUNTRequest" );

		if ( !destClient || !destDeviceSP ) {
			CErrArgID ( "HandleSTUNTRequest [ %i ]: Failed to find device connection for id [ 0x%X ]!", sourceClient->socket, destID ); goto Quit;
		}

		// Build source client stunt id
		if ( !BuildAppAreaID ( sourceStuntID, destID, appName, areaName, channelType, req->header.token ) ) {
			CErrArgID ( "HandleSTUNTRequest [ %i ]: Failed to build source stunt id for [ 0x%X ]!", sourceClient->socket, destID ); goto Quit;
		}

        if ( IsInvalidFD ( sourceClient->socket ) ) goto Quit;

		// Acquire the lock on sourceClient stuntmap
		if ( !LockAcquireA ( sourceClient->stuntSocketLock,"HandleSTUNTRequest" ) ) goto Quit;

		///
		/// Check whether the source client is ready for a stunt
		///
		foundIt = sourceClient->stuntSocketsLog.find ( sourceStuntID + 4 );

		if ( foundIt != sourceClient->stuntSocketsLog.end () )
        {
            const sp ( StuntRegisterContext ) & ctxSP = foundIt->second;

            StuntRegisterContext * ctx = ctxSP.get ();
            
            if ( ctx->token == req->header.token ) {
                struct sockaddr_in * a = channelType == MEDIATOR_STUNT_CHANNEL_MAIN ? &ctx->addrI : &ctx->addrC;
                
                portSource = ntohs ( a->sin_port );
            }
            else {
                invalidToken = true;
            }
		}

		LockReleaseA ( sourceClient->stuntSocketLock, "HandleSTUNTRequest" );
		
        if ( invalidToken ) {
            status = 0; goto Quit;
        }
        
		if ( !portSource ) {
			CLogArgID ( "HandleSTUNTRequest [ %i ]: Source client has not registered the stunt socket yet!", sourceClient->socket );

			NotifySTUNTRegRequest ( sourceClient, destID, appName, areaName, channelType, req->header.token ); status = -1; goto Quit;
		}

		IP = sourceDevice->info.ip;

		// Build source client stunt id
		if ( !appName || !areaName ) {
			if ( !BuildAppAreaID ( destStuntID, deviceID, appName, areaName, channelType, req->header.token ) ) {
				CErrArgID ( "HandleSTUNTRequest [ %i ]: Failed to build dest stunt id for [ 0x%X ]!", sourceClient->socket, destID ); goto Quit;
			}
		}
		else {
			if ( !BuildAppAreaID ( destStuntID, deviceID, sourceDevice->info.appName, sourceDevice->info.areaName, channelType, req->header.token ) ) {
				CErrArgID ( "HandleSTUNTRequest [ %i ]: Failed to build dest stunt id for [ 0x%X ]!", sourceClient->socket, destID ); goto Quit;
			}		
		}

        invalidToken = false;

        if ( IsInvalidFD ( destClient->socket ) ) goto Quit;
        
		// Acquire the lock on destClient stuntmap
		if ( !LockAcquireA ( destClient->stuntSocketLock, "HandleSTUNTRequest" ) ) goto Quit;

		///
		/// Check whether the source client is ready for a stunt
		///
		foundIt = destClient->stuntSocketsLog.find ( destStuntID + 4 );

		if ( foundIt != destClient->stuntSocketsLog.end () )
        {
            const sp ( StuntRegisterContext ) & ctxSP = foundIt->second;

			StuntRegisterContext * ctx = ctxSP.get ();
            
            if ( ctx->token == req->header.token ) {
                struct sockaddr_in * a = channelType == MEDIATOR_STUNT_CHANNEL_MAIN ? &ctx->addrI : &ctx->addrC;
                
                portDest = ntohs ( a->sin_port );
                
                if ( portDest ) {
                    if ( a->sin_addr.s_addr ) {
                        a->sin_addr.s_addr = 0;
                    }
                    else alreadyInvoked = true;
                }
            }
            else {
                invalidToken = true;
            }
		}

        LockReleaseA ( destClient->stuntSocketLock, "HandleSTUNTRequest" );
        
        if ( invalidToken ) {
            status = 0; goto Quit;
        }
        
        if ( alreadyInvoked ) {
            reqResponse->ident [ 0 ] = MEDIATOR_OPT_NULL;
            reqResponse->ident [ 1 ] = MEDIATOR_OPT_NULL;            
            reqResponse->channel     = channelType;
            goto SendPorts;
        }
        
		///
		/// Check whether the destination client is ready for a stunt
		///
		if ( !portDest ) {
			CLogArgID ( "HandleSTUNTRequest [ %i ]: Destination client [ 0x%X ] has not renewed the stunt socket yet!", sourceClient->socket, destID );

			if ( appName && areaName )
				NotifySTUNTRegRequest ( destClient.get (), deviceID, sourceDevice->info.appName, sourceDevice->info.areaName, channelType, req->header.token );
			else
				NotifySTUNTRegRequest ( destClient.get (), deviceID, appName, areaName, channelType, req->header.token );  status = -1; goto Quit;
		}

		IPe = sourceClient->addr.sin_addr.s_addr;

		if ( !IP || !IPe ) {
			CErrArgID ( "HandleSTUNTRequest [ %i ]: Invalid IP [ %s ] IPe [ %x : %d ] for sourceDevice or invalid Port [ %d ] for destinationDevice in database!", sourceClient->socket,
				inet_ntoa ( *( ( struct in_addr * ) &IP ) ), IPe, portSource, portDest );
			goto Quit;
		}

		header->ident [ 0 ] = MEDIATOR_CMD_STUNT;
		header->ident [ 1 ] = MEDIATOR_OPT_NULL;
		header->ident [ 2 ] = MEDIATOR_OPT_NULL;

		header->channel    = channelType;
		header->seqNr      = seqNr;
		header->deviceID   = sourceClient->deviceID;
		header->token	   = req->header.token;
		header->ip         = IP;
		header->ipe        = IPe;
		header->porti      = sourceDevice->info.tcpPort;
		header->porte      = portSource;

		if ( areaName && appName ) {
			if ( !BuildAppAreaField ( header->sizes, sourceDevice->info.appName, sourceDevice->info.areaName, false ) ) {
				goto Quit;
			}
		}
		else {
			header->sizes [ 0 ] = 1; header->sizes [ 1 ] = 1;
		}

		header->size = sizeof ( STUNTRespReqHeaderV8 ) + header->sizes [ 0 ] + header->sizes [ 1 ];

		CLogArgID ( "STUNTRequest [ %i ]: Send request to device [ 0x%X ] IP [ %s : %d ]", sourceClient->socket, destID, inet_ntoa ( destClient->addr.sin_addr ), portSource );
        
        if ( !SendBufferOrEnqueue ( destClient.get (), header, header->size, true, destClient.get ()->seqNr - 1 ) ) {
            CErrArgID ( "HandleSTUNTRequest [ %i ]: Failed to send STUNT request to device [ 0x%X ] IP [ %s ]!", sourceClient->socket, destID, inet_ntoa ( destClient->addr.sin_addr ) );
            LogSocketError ();
            goto Quit;
        }
        
    SendPorts:
		sendSize = extResp ? MEDIATOR_STUNT_ACK_EXT_SIZE_V6 : MEDIATOR_STUNT_ACK_SIZE_V6;
		reqResponse->size = sendSize;

		reqResponse->respCode = 'p';

		reqResponse->seqNr = seqNr;
		reqResponse->porte = portDest;
		reqResponse->porti = destDeviceSP ? destDeviceSP->info.tcpPort : 0;

		if ( extResp ) {
			reqResponse->ip  = destDeviceSP->info.ip;
			reqResponse->ipe = destClient->addr.sin_addr.s_addr;
		}

		CLogArgID ( "STUNTRequest [ %i ]: Send response to device [ 0x%X ] IP [ %s : %d ]", sourceClient->socket, destID, inet_ntoa ( sourceClient->addr.sin_addr ), portDest );
        
        if ( !SendBufferOrEnqueue ( sourceClient, reqResponse, sendSize, true, seqNr ) ) {
            CErrArgID ( "HandleSTUNTRequest [ %i ]: Failed to send STUNT response (Port) to sourceClient device IP [ %s ]!", sourceClient->socket, inet_ntoa ( sourceClient->addr.sin_addr ) );
            LogSocketError ();
            goto Quit;
        }
        return true;

	Quit:
		if ( status > 0 )
			return true;

		reqResponse->size   = MEDIATOR_STUNT_ACK_SIZE_V6;
		reqResponse->seqNr  = seqNr;

		if ( status < 0 )
			reqResponse->respCode = 'r';
		else
			reqResponse->respCode = 'e';
        
        if ( !SendBufferOrEnqueue ( sourceClient, reqResponse, MEDIATOR_STUNT_ACK_SIZE_V6, true, seqNr ) ) {
            CErrArgID ( "HandleSTUNTRequest [ %i ]: Failed to send [ %s ] message to sourceClient", sourceClient->socket, reqResponse->respCode == 'e' ? "Failed" : "Retry" );
            LogSocketError ();
        }
        return false;
	}


	void MediatorDaemon::BuildBroadcastMessage ( bool withStatus )
	{
		CVerb ( "buildBroadcastMessage" );

		// Format E.Mediator port1 port2 ...
		strlcpy ( broadcastMessage + 4, MEDIATOR_BROADCAST, MEDIATOR_BROADCAST_DESC_START );

		broadcastMessageLen = 16;

		unsigned short * pShort = ( unsigned short * ) ( broadcastMessage + MEDIATOR_BROADCAST_PORTS_ABS_START );

		*pShort = ( unsigned short ) ports [ 0 ];

		broadcastMessageLen += 4;

		unsigned int * pInt = ( unsigned int * ) ( broadcastMessage + MEDIATOR_BROADCAST_PLATFORM_ABS_START );
		*pInt = 1; // We use the platform 1 for now


		broadcastMessageLen += 4;

		broadcastMessage [ broadcastMessageLen ] = 0;

		pInt  = reinterpret_cast<unsigned int *>( broadcastMessage );
		*pInt = broadcastMessageLen;

		//*((unsigned int *) broadcastMessage) = broadcastMessageLen + 4;
		//memcpy ( broadcastMessage, &bcml, sizeof(bcml) );  // to prevent strict-alias bugs (due to compiler opts)
	}


	int MediatorDaemon::ScanForParameters ( char * buffer, unsigned int maxLen, const char * delim, char ** params, int iParams )
	{
		CVerbVerb ( "ScanForParameters" );

		int i = 0;

		char * context = NULL;
		char * psem = strtok_s ( buffer, delim, &context );

		while ( psem != NULL )
		{
			params [ i ] = psem;

			i++;
			if ( i >= ( iParams - 1 ) ) {
				unsigned int length = ( unsigned int ) ( ( psem + strlen ( psem ) ) - buffer + 1 );
				if ( length < maxLen ) {
					params [ i ] = psem + strlen ( psem ) + 1;
					i++;
				}
				break;
			}

			psem = strtok_s ( NULL, delim, &context );
		}

		return i;
	}


	void * MediatorDaemon::BroadcastThread ()
	{
		int ret, bytesReceived;
		socklen_t addrLen;
		char	*	msg;
		char buffer [ BUFFERSIZE ];

		CVerb ( "BroadcastThread started." );

		broadcastThread.Notify ( "MediatorClient::BroadcastThread" );

		if ( broadcastSocket <= 0 ) {
			CErr ( "BroadcastThread: Invalid broadcast socket!" );
			return 0;
		}

		struct 	sockaddr_in		listenAddr;
		memset ( &listenAddr, 0, sizeof ( listenAddr ) );

		listenAddr.sin_family		= AF_INET;
		listenAddr.sin_addr.s_addr	= INADDR_ANY; //htonl ( INADDR_BROADCAST ); // INADDR_ANY );
		listenAddr.sin_port         = htons ( GET_MEDIATOR_BASE_PORT );

		ret = ::bind ( broadcastSocket, ( struct sockaddr * ) &listenAddr, sizeof ( listenAddr ) );
		if ( ret < 0 ) {
			CErrArg ( "BroadcastThread: Failed to bind broadcast listener socket to port %i!", GET_MEDIATOR_BASE_PORT );
			LogSocketError ();
			//return 0;
		}
		else {
			CVerbArg ( "BroadcastThread bound socket to port %i", DEFAULT_BROADCAST_PORT );
		}

		// Send Broadcast
		SendBroadcast ();

		// Wait for broadcast
		addrLen = sizeof ( listenAddr );

		CLogArg ( "BroadcastThread listen on port %i", GET_MEDIATOR_BASE_PORT );

		while ( isRunning ) {
			bytesReceived = ( int ) recvfrom ( broadcastSocket, buffer, BUFFERSIZE, 0, ( struct sockaddr* ) &listenAddr, &addrLen );

			if ( bytesReceived < 0 ) {
				CLog ( "BroadcastThread: Socket has been closed!" );
				break;
			}
			msg = buffer + 4;

			if ( bytesReceived < 10 || msg [ 0 ] != 'E' || msg [ 1 ] != '.' || bytesReceived >= BUFFERSIZE )
				continue;

			buffer [ bytesReceived ] = 0;

			if ( msg [ 2 ] == 'D' ) {
				// if message contains EHLO, then broadcast our data
				if ( msg [ 4 ] == 'H' ) {
					SendBroadcast ();
					CVerbArg ( "BroadcastThread: Replied to client request from %s;\n\t\tMessage: %s", inet_ntoa ( *( ( struct in_addr * ) &( listenAddr.sin_addr.s_addr ) ) ), buffer );
				}
			}
			else {
				CVerbArg ( "BroadcastThread: read %d bytes <- IP %s;\n\t\tMessage: %s", bytesReceived, inet_ntoa ( *( ( struct in_addr * ) &( listenAddr.sin_addr.s_addr ) ) ), buffer );
			}
		}

		CLog ( "BroadcastThread: Thread terminated." );

		return 0;
	}


	bool MediatorDaemon::UpdateDeviceRegistry ( sp ( DeviceInstanceNode ) device, unsigned int ip, char * msg )
	{
		CVerb ( "UpdateDeviceRegistry" );

		char * keyCat;
		char * value;

		if ( !device.get () )
			return false;

		// Update the values in our database

		char * pareaName = device->info.areaName;

		if ( !pareaName || !areasMap.Lock ( "UpdateDeviceRegistry" ) ) {
			CErr ( "UpdateDeviceRegistry: Failed to aquire mutex on areas/values (alt: areaName invalid)! Skipping client request!" );

			return false;
		}

		// Search for the area at first
		sp ( AppsList )						appsList = 0;

		sp ( ListValues )                  values = 0;


		string areaName ( pareaName );
		string appName ( device->info.appName );

		const msp ( string, AppsList )::iterator areaIt = areasMap.list.find ( areaName );

		if ( areaIt == areasMap.list.end () )
		{
			appsList = make_shared < AppsList > ();
			if ( !appsList || !appsList->Init () ) {
				CErrArg ( "UpdateDeviceRegistry: Failed to create new area [%s].", pareaName );

				areasMap.Unlock ( "UpdateDeviceRegistry" );

				return false;
			}
			areasMap.list [ areaName ] = appsList;
		}
		else
			appsList = areaIt->second;

		areasMap.Unlock ( "UpdateDeviceRegistry" );


		if ( !appsList || !appsList->Lock ( "UpdateDeviceRegistry" ) ) {
			return false;
		}

		const msp ( string, ListValues )::iterator appsIt = appsList->apps.find ( appName );

		if ( appsIt == appsList->apps.end () )
		{
			values = make_shared < ListValues > (); // new map<string, ValuePack*> ();
			if ( !values || !values->Init () ) {
				CErrArg ( "UpdateDeviceRegistry: Failed to create new application [%s].", appName.c_str () );
				goto Continue;
			}
			appsList->apps [ appName ] = values;
		}
		else
			values = appsIt->second;

		appsList->Unlock ( "UpdateDeviceRegistry" );

		if ( !values || !values->Lock ( "UpdateDeviceRegistry" ) )
			return false;


		char keyBuffer [ 128 ];
		char valueBuffer [ 256 ];
		if ( snprintf ( keyBuffer, 128, "%i_", device->info.deviceID ) < 0 )
			goto Failed;

		keyCat = keyBuffer + strlen ( keyBuffer );

		// ip in register message
		strlcat ( keyCat, "ip", 100 );

		value = inet_ntoa ( *( ( struct in_addr * ) &device->info.ip ) );

		if ( !AddToArea ( values, keyBuffer, value, ( unsigned int ) strlen ( value ) ) ) {
			CWarnArg ( "UpdateDeviceRegistry: Adding key %s failed!", keyBuffer );
		}

		// extern ip
		*keyCat = 0;
		strlcat ( keyCat, "ipe", 100 );

		value = inet_ntoa ( *( ( struct in_addr * ) &ip ) );

		if ( !AddToArea ( values, keyBuffer, value, ( unsigned int ) strlen ( value ) ) ) {
			CWarnArg ( "UpdateDeviceRegistry: Adding key %s failed!", keyBuffer );
		}

		// tcp port
		*keyCat = 0;
		strlcat ( keyCat, "cport", 100 );
		if ( snprintf ( valueBuffer, 128, "%u", device->info.tcpPort ) < 0 )
			goto Failed;

		if ( !AddToArea ( values, keyBuffer, valueBuffer, ( unsigned int ) strlen ( valueBuffer ) ) ) {
			CWarnArg ( "UpdateDeviceRegistry: Adding key %s failed!", keyBuffer );
		}

		// udp port
		*keyCat = 0;
		strlcat ( keyCat, "dport", 100 );
		if ( snprintf ( valueBuffer, 128, "%u", device->info.udpPort ) < 0 )
			goto Failed;

		if ( !AddToArea ( values, keyBuffer, valueBuffer, ( unsigned int ) strlen ( valueBuffer ) ) ) {
			CWarnArg ( "UpdateDeviceRegistry: Adding key %s failed!", keyBuffer );
		}

		// device type
		*keyCat = 0;
		strlcat ( keyCat, "type", 100 );
		if ( snprintf ( valueBuffer, 128, "%i", device->info.platform ) < 0 )
			goto Failed;

		if ( !AddToArea ( values, keyBuffer, valueBuffer, ( unsigned int ) strlen ( valueBuffer ) ) ) {
			CWarnArg ( "UpdateDeviceRegistry: Adding key %s failed!", keyBuffer );
		}

		// device name
		*keyCat = 0;
		strlcat ( keyCat, "name", 100 );

		if ( !AddToArea ( values, keyBuffer, device->info.deviceName, ( unsigned int ) strlen ( device->info.deviceName ) ) ) {
			CWarnArg ( "UpdateDeviceRegistry: Adding key %s failed!", keyBuffer );
		}

	Failed:
		values->Unlock ( "UpdateDeviceRegistry" );

	Continue:
		configDirty = true;

		return true;
	}


	bool MediatorDaemon::HandleDeviceRegistration ( const sp ( ThreadInstance ) &clientSP, unsigned int ip, char * msg )
	{
		CVerb ( "HandleDeviceRegistration" );

		MediatorReqMsg          regMsg;
		MediatorReqHeader *     header      = &regMsg.header;
		int                     sentBytes;

		if ( !clientSP ) {
			CErr ( "HandleDeviceRegistration:\tInvalid client!." );
			return false;
		}

		ThreadInstance  * client = clientSP.get ();

		int deviceID = client->deviceID;
		if ( !deviceID ) {
			CErrArg ( "HandleDeviceRegistration [ %s ]:\tInvalid deviceID.", client->ips );
			return false;
		}

		/*if ( !RemoveAcceptClient ( client ) ) {
			CErrArgID ( "HandleDeviceRegistration [ %s ]:\tFailed to lookup the client in acceptClients.", client->ips );
			return false;
		}*/

		char		*	uid		= 0;
		bool			created = false;

		sp ( DeviceInstanceNode ) deviceSP = UpdateDevices ( ip, msg, &uid, &created );

		if ( !deviceSP ) {
			CErrArgID ( "HandleDeviceRegistration [ %s ]:\tFailed to parse registration.", client->ips );
			return false;
		}
		if ( !created ) {
			MediatorMsg resp;
			Zero ( resp );

			resp.size = sizeof ( MediatorMsg );
			resp.cmd1 = MEDIATOR_OPT_NULL;
			resp.opt0 = MEDIATOR_OPT_NULL;
			resp.opt1 = MEDIATOR_OPT_NULL;

			sp ( ThreadInstance ) busySP = deviceSP->clientSP;
			if ( busySP )
			{
				// Check whether deviceName is the same
				char * deviceName = msg + MEDIATOR_BROADCAST_DESC_START;

				if ( strncmp ( deviceSP->info.deviceName, deviceName, sizeof ( deviceSP->info.deviceName ) ) )
				{
					// Another device with the same identifiers has already been registered.
					/// Return slot busy
					resp.cmd0 = MEDIATOR_SRV_CMD_SESSION_LOCKED;
					CVerbArgID ( "HandleDeviceRegistration [ %s ]:\tDevice instance slot locked.", client->ips );
				}
				else {
					// Check socket
					if ( !IsSocketAlive ( busySP->socket ) ) {
						VerifySockets ( busySP.get (), false );

						/// Return try again
						resp.cmd0 = MEDIATOR_SRV_CMD_SESSION_RETRY;
					}
					else {
						if ( ( checkLast - busySP->aliveLast ) > 0 )
						{
							/// Request alive response
							resp.cmd0 = MEDIATOR_SRV_CMD_ALIVE_REQUEST;

							busySP->aliveLast -= 2;
							SendBuffer ( busySP.get (), &resp, resp.size );

							Sleep ( 100 );

							/// Trigger Watchdog
							if ( LockAcquireA ( thread_lock, "Watchdog" ) ) {

								if ( pthread_cond_signal ( &hWatchdogEvent ) ) {
									CVerbArgID ( "HandleDeviceRegistration [ %s ]:\tWatchdog signal failed.", client->ips );
								}

								LockReleaseVA ( thread_lock, "Watchdog" );
							}
							/// Return try again
							resp.cmd0 = MEDIATOR_SRV_CMD_SESSION_RETRY;
						}
						else {
							/// Return slot busy
							resp.cmd0 = MEDIATOR_SRV_CMD_SESSION_LOCKED;
							CVerbArgID ( "HandleDeviceRegistration [ %s ]:\tDevice instance slot locked.", client->ips );
						}
					}
				}
			}
			else {
				RemoveDevice ( deviceSP.get () );

				/// Return try again
				resp.cmd0 = MEDIATOR_SRV_CMD_SESSION_RETRY;
				CVerbArgID ( "HandleDeviceRegistration [ %s ]:\tDevice instance slot lingering. Responded retry advice.", client->ips );
			}

			SendBuffer ( client, &resp, resp.size );

			CErrArgID ( "HandleDeviceRegistration [ %s ]:\tA device instance for this registration already exists.", client->ips );
			return false;
		}

		client->deviceSP = deviceSP->baseSP;

		DeviceInstanceNode * device = deviceSP.get ();
		device->clientSP = clientSP;

		*client->uid = 0;
		if ( uid && *uid && device->info.deviceID )
		{
			// Transform lowercase of uid
			string suid ( uid );
			std::transform ( suid.begin (), suid.end (), suid.begin (), ::tolower );

			CVerbArgID ( "HandleDeviceRegistration [ %s ]:\tUsing lowercase uid [%s].", client->ips, suid.c_str () );

			strlcpy ( client->uid, suid.c_str (), sizeof ( client->uid ) );

			if ( deviceMappings.Lock ( "HandleDeviceRegistration" ) )
			{
				sp ( DeviceMapping ) mapping;

				Zero ( regMsg );

				CVerbArgID ( "HandleDeviceRegistration [ %s ]:\tLooking for mapping of uid [%s].", client->ips, client->uid );

				const msp ( string, DeviceMapping )::iterator devIt = deviceMappings.list.find ( string ( client->uid ) );

				if ( devIt != deviceMappings.list.end () ) {
					CVerbArgID ( "HandleDeviceRegistration [ %s ]:\tFound.", client->ips );
					mapping = devIt->second;
				}
				else {
					mapping.reset ( new DeviceMapping );
					//mapping = make_shared < DeviceMapping > (); //sp ( DeviceMapping ) ( new DeviceMapping ); // calloc ( 1, sizeof(DeviceMapping) );
					if ( !mapping )
						goto PreFailExit;
					memset ( mapping.get (), 0, sizeof ( DeviceMapping ) );

					CVerbArgID ( "HandleDeviceRegistration [ %s ]:\tNot found. Created new.", client->ips );
				}
				mapping->deviceID = device->info.deviceID;

				/// Check for authToken
				if ( !*mapping->authToken || client->createAuthToken )
				{
					CVerbArgID ( "HandleDeviceRegistration [ %s ]:\tCreating new auth token.", client->ips );

					srand ( getRandomValue ( &client ) );

					unsigned int tokCount = sizeof ( mapping->authToken ) - ( rand () % 5 );

					for ( unsigned int i=0; i<tokCount; i++ ) {
						mapping->authToken [ i ] = ( rand () % 93 ) + 33;
					}
					mapping->authToken [ tokCount ] = 0;

					header->size = sizeof ( MediatorReqHeader ) + tokCount + 2;
					header->cmd0 = MEDIATOR_PROTOCOL_VERSION;
					header->cmd1 = MEDIATOR_CMD_AUTHTOKEN_ASSIGN;
					header->opt0 = MEDIATOR_OPT_NULL;
					header->opt1 = MEDIATOR_OPT_NULL;

					memcpy ( regMsg.deviceUID, mapping->authToken, tokCount );
					regMsg.deviceUID [ tokCount ] = 0;

					header->sizes [ 0 ] = 1;
					header->sizes [ 1 ] = ( unsigned char ) tokCount;

					mapping->authLevel = client->authLevel;

					CVerbArgID ( "HandleDeviceRegistration [ %s ]:\tMapping created deviceID [0x%X] authLevel [%i] authToken [...].", client->ips, deviceID, client->authLevel );
				}

				deviceMappings.list [ string ( client->uid ) ] = mapping;

			PreFailExit:
				if ( !deviceMappings.Unlock ( "HandleDeviceRegistration" ) )
					return false;

				if ( !mapping ) {
					CErrArgID ( "HandleDeviceRegistration [ %s ]:\tFailed to find or create a mapping.", client->ips );
					return false;
				}

				if ( header->size ) {
					CErrArgID ( "HandleDeviceRegistration [ %s ]:\tSending new auth token to client.", client->ips );

					/// Send authToken to client
					sentBytes = SendBuffer ( clientSP.get (), &regMsg, header->size );

					if ( sentBytes != ( int ) header->size ) {
						CErrArgID ( "HandleDeviceRegistration [ %s ]:\tFailed to assign authtoken!", client->ips );
						return false;
					}
				}

				deviceMappingDirty = true;
			}
		}

		if ( !client->version )
			client->version = *msg;

		if ( *msg < '4' )
			UpdateDeviceRegistry ( deviceSP, ip, msg );

		if ( !RemoveAcceptClient ( client ) ) {
			CErrArgID ( "HandleDeviceRegistration [ %s ]:\tFailed to lookup the client in acceptClients.", client->ips );
		}
        
        /// Assign a session id
        long long sid = __sync_add_and_fetch ( &sessionCounter, 1 );
        sid |= ( ( long long ) ( ( device->rootSP->id << 16 ) | device->rootSP->areaId ) ) << 32;
        
		if ( !sessions.Lock ( "HandleDeviceRegistration" ) )
			return false;

		sessions.list [ sid ] = clientSP;

		sessions.BuildCache ();

		if ( !sessions.Unlock ( "HandleDeviceRegistration" ) )
            return false;
        
        // Updated connected ips
        //
        if ( LockAcquireA ( bannedIPsLock, "HandleDeviceRegistration" ) )
        {
            unsigned int c = 1;
            
            std::map<unsigned int, unsigned int>::iterator it = connectedIPs.find ( ip );
            
            if ( it != connectedIPs.end () ) {
                c += it->second;
            }
            
            connectedIPs [ ip ] = c;
            
            LockReleaseVA ( bannedIPsLock, "HandleDeviceRegistration" );
        }
        
        for ( size_t i = 0; i < sendThreadCount; ++i )
        {
			___sync_val_compare_and_swap ( sessionsChanged + i, 0, 1 );
        }
        
        ___sync_val_compare_and_swap ( &watchdogSessionsChanged, 0, 1 );
        
        CVerbArgID ( "HandleDeviceRegistration [ %s ]:\tAssinged session id [%i]", client->ips, sid );

        client->sessionID = sid;
        
#ifdef ENABLE_WINSOCK_SEND_THREADS
        WSASetEvent ( sendEvent );
#else
        sendEvent.Notify ( "PushSend" );
#endif
        /// Send session id to client
		MediatorMsg * sessMsg = ( MediatorMsg * ) &regMsg;

		sessMsg->size = sizeof ( MediatorMsg );
		sessMsg->cmd0 = MEDIATOR_PROTOCOL_VERSION;
		sessMsg->cmd1 = MEDIATOR_CMD_SESSION_ASSIGN;
		sessMsg->opt0 = MEDIATOR_OPT_NULL;
		sessMsg->opt1 = MEDIATOR_OPT_NULL;
		sessMsg->ids.sessionID = sid;

		CVerbArgID ( "HandleDeviceRegistration [ %s ]:\tSending session id [%i] to client.", client->ips, sid );

		sentBytes = SendBuffer ( client, sessMsg, sessMsg->size );
		if ( sentBytes != ( int ) sessMsg->size ) {
			CErrID ( "HandleDeviceRegistration: Failed to assign session id!" );
			return false;
		}

		return true;
	}


	bool MediatorDaemon::HandleDeviceRegistrationV4 ( const sp ( ThreadInstance ) &clientSP, unsigned int ip, char * msg )
	{
		CVerb ( "HandleDeviceRegistrationV4" );

		MediatorReqMsgV4  regMsg;
		int             sentBytes;

		if ( !clientSP ) {
			CErr ( "HandleDeviceRegistrationV4:\tInvalid client!." );
			return false;
		}

		ThreadInstance  * client = clientSP.get ();

		int deviceID = client->deviceID;
		if ( !deviceID ) {
			CErrArg ( "HandleDeviceRegistrationV4 [ %s ]:\tInvalid deviceID.", client->ips );
			return false;
		}

		// Clear all lingering devices in the list with the same deviceID

		/*if ( !RemoveAcceptClient ( client ) ) {
			CErrArgID ( "HandleDeviceRegistrationV4 [ %s ]:\tFailed to lookup the client in acceptClients.", client->ips );
			return false;
		}*/

		char		*	uid		= 0;
		bool			created = false;

		sp ( DeviceInstanceNode ) deviceSP;
		if ( client->version <= '4' )
			deviceSP = UpdateDevicesV4 ( ip, msg, &uid, &created );
		else
			deviceSP = UpdateDevices ( ip, msg, &uid, &created );

		if ( !deviceSP ) {
			CErrArgID ( "HandleDeviceRegistrationV4 [ %s ]:\tFailed to parse registration.", client->ips );
			return false;
		}
		if ( !created ) {
			MediatorMsg resp;
			Zero ( resp );

			resp.size = sizeof ( MediatorMsg );
			resp.cmd1 = MEDIATOR_OPT_NULL;
			resp.opt0 = MEDIATOR_OPT_NULL;
			resp.opt1 = MEDIATOR_OPT_NULL;

			sp ( ThreadInstance ) busySP = deviceSP->clientSP;
			if ( busySP )
			{
				// Check whether deviceName is the same
				char * deviceName = msg + MEDIATOR_BROADCAST_DESC_START;

				if ( strncmp ( deviceSP->info.deviceName, deviceName, sizeof ( deviceSP->info.deviceName ) ) )
				{
					// Another device with the same identifiers has already been registered.
					/// Return slot busy
					resp.cmd0 = MEDIATOR_SRV_CMD_SESSION_LOCKED;
					CVerbArgID ( "HandleDeviceRegistrationV4 [ %s ]:\tDevice instance slot locked.", client->ips );
				}
				else {
					// Check socket
					if ( !IsSocketAlive ( busySP->socket ) ) {
						VerifySockets ( busySP.get (), false );

						/// Return try again
						resp.cmd0 = MEDIATOR_SRV_CMD_SESSION_RETRY;
					}
					else {
						if ( ( checkLast - busySP->aliveLast ) > 0 )
						{
							/// Request alive response
							resp.cmd0 = MEDIATOR_SRV_CMD_ALIVE_REQUEST;

							busySP->aliveLast -= 2;
							SendBuffer ( busySP.get (), &resp, resp.size );

							Sleep ( 100 );

							/// Trigger Watchdog
							if ( LockAcquireA ( thread_lock, "Watchdog" ) ) {

								if ( pthread_cond_signal ( &hWatchdogEvent ) ) {
									CVerbArgID ( "HandleDeviceRegistrationV4 [ %s ]:\tWatchdog signal failed.", client->ips );
								}

								LockReleaseVA ( thread_lock, "Watchdog" );
							}
							/// Return try again
							resp.cmd0 = MEDIATOR_SRV_CMD_SESSION_RETRY;
						}
						else {
							/// Return slot busy
							resp.cmd0 = MEDIATOR_SRV_CMD_SESSION_LOCKED;
							CVerbArgID ( "HandleDeviceRegistrationV4 [ %s ]:\tDevice instance slot locked.", client->ips );
						}
					}
				}
			}
			else {
				RemoveDevice ( deviceSP.get () );

				/// Return try again
				resp.cmd0 = MEDIATOR_SRV_CMD_SESSION_RETRY;
				CVerbArgID ( "HandleDeviceRegistrationV4 [ %s ]:\tDevice instance slot lingering. Responded retry advice.", client->ips );
			}

			SendBuffer ( client, &resp, resp.size );

			CErrArgID ( "HandleDeviceRegistrationV4 [ %s ]:\tA device instance for this registration already exists.", client->ips );
			return false;
		}

		client->deviceSP = deviceSP->baseSP;

		DeviceInstanceNode * device = deviceSP.get ();
		device->clientSP = clientSP;

		*client->uid = 0;
		if ( uid && *uid && device->info.deviceID )
		{
			// Transform lowercase of uid
			string suid ( uid );
			std::transform ( suid.begin (), suid.end (), suid.begin (), ::tolower );

			CVerbArgID ( "HandleDeviceRegistrationV4 [ %s ]:\tUsing lowercase uid [%s].", client->ips, suid.c_str () );

			strlcpy ( client->uid, suid.c_str (), sizeof ( client->uid ) );

			if ( deviceMappings.Lock ( "HandleDeviceRegistration" ) )
			{
				sp ( DeviceMapping ) mapping;

				Zero ( regMsg );

				CVerbArgID ( "HandleDeviceRegistrationV4 [ %s ]:\tLooking for mapping of uid [%s].", client->ips, client->uid );

				const msp ( string, DeviceMapping )::iterator devIt = deviceMappings.list.find ( string ( client->uid ) );

				if ( devIt != deviceMappings.list.end () ) {
					CVerbArgID ( "HandleDeviceRegistrationV4 [ %s ]:\tFound.", client->ips );
					mapping = devIt->second;
				}
				else {
					mapping.reset ( new DeviceMapping );
					//mapping = make_shared < DeviceMapping > (); //sp ( DeviceMapping ) ( new DeviceMapping ); // calloc ( 1, sizeof(DeviceMapping) );
					if ( !mapping )
						goto PreFailExit;
					memset ( mapping.get (), 0, sizeof ( DeviceMapping ) );

					CVerbArgID ( "HandleDeviceRegistrationV4 [ %s ]:\tNot found. Created new.", client->ips );
				}
				mapping->deviceID = device->info.deviceID;

				/// Check for authToken
				if ( !*mapping->authToken || client->createAuthToken )
				{
					CVerbArgID ( "HandleDeviceRegistrationV4 [ %s ]:\tCreating new auth token.", client->ips );

					srand ( getRandomValue ( &client ) );

					unsigned int tokCount = sizeof ( mapping->authToken ) - ( rand () % 5 );

					for ( unsigned int i=0; i<tokCount; i++ ) {
						mapping->authToken [ i ] = ( rand () % 93 ) + 33;
					}
					mapping->authToken [ tokCount ] = 0;

					regMsg.size = sizeof ( MediatorMsg ) + tokCount - sizeof ( long long );
					regMsg.cmd0 = MEDIATOR_PROTOCOL_VERSION;
					regMsg.cmd1 = MEDIATOR_CMD_AUTHTOKEN_ASSIGN;
					regMsg.opt0 = MEDIATOR_OPT_NULL;
					regMsg.opt1 = MEDIATOR_OPT_NULL;

					memcpy ( regMsg.areaName, mapping->authToken, tokCount );
					regMsg.areaName [ tokCount ] = 0;

					mapping->authLevel = client->authLevel;

					CVerbArgID ( "HandleDeviceRegistrationV4 [ %s ]:\tMapping created deviceID [0x%X] authLevel [%i] authToken [...].", client->ips, deviceID, client->authLevel );
				}

				deviceMappings.list [ string ( client->uid ) ] = mapping;

			PreFailExit:
				if ( !deviceMappings.Unlock ( "HandleDeviceRegistration" ) )
					return false;

				if ( !mapping ) {
					CErrArgID ( "HandleDeviceRegistrationV4 [ %s ]:\tFailed to find or create a mapping.", client->ips );
					return false;
				}

				if ( regMsg.size ) {
					CErrArgID ( "HandleDeviceRegistrationV4 [ %s ]:\tSending new auth token to client.", client->ips );

					/// Send authToken to client
					sentBytes = SendBuffer ( clientSP.get (), &regMsg, regMsg.size );

					if ( sentBytes != ( int ) regMsg.size ) {
						CErrArgID ( "HandleDeviceRegistrationV4 [ %s ]:\tFailed to assign authtoken!", client->ips );
						return false;
					}
				}

				deviceMappingDirty = true;
			}
		}

		if ( !client->version )
			client->version = *msg;

		if ( *msg < '4' )
			UpdateDeviceRegistry ( deviceSP, ip, msg );

		if ( !RemoveAcceptClient ( client ) ) {
			CErrArgID ( "HandleDeviceRegistration [ %s ]:\tFailed to lookup the client in acceptClients.", client->ips );
		}
        
        /// Assign a session id
        long long sid = __sync_add_and_fetch ( &sessionCounter, 1 );
        sid |= ( ( long long ) ( ( device->rootSP->id << 16 ) | device->rootSP->areaId ) ) << 32;

		if ( !sessions.Lock ( "HandleDeviceRegistration" ) )
			return false;

		sessions.list [ sid ] = clientSP;

		sessions.BuildCache ();

		if ( !sessions.Unlock ( "HandleDeviceRegistration" ) )
            return false;
        
        CVerbArgID ( "HandleDeviceRegistrationV4 [ %s ]:\tAssinged session id [%i]", client->ips, sid );

        client->sessionID = sid;
        
        // Updated connected ips
        //
        if ( LockAcquireA ( bannedIPsLock, "HandleDeviceRegistration" ) )
        {
            unsigned int c = 1;
            
            std::map<unsigned int, unsigned int>::iterator it = connectedIPs.find ( ip );
            
            if ( it != connectedIPs.end () ) {
                c += it->second;
            }
            
            connectedIPs [ ip ] = c;
            
            LockReleaseVA ( bannedIPsLock, "HandleDeviceRegistration" );
        }
        
        for ( size_t i = 0; i < sendThreadCount; ++i )
        {
			___sync_val_compare_and_swap ( sessionsChanged + i, 0, 1 );
        }
        
        ___sync_val_compare_and_swap ( &watchdogSessionsChanged, 0, 1 );
        
#ifdef ENABLE_WINSOCK_SEND_THREADS
        WSASetEvent ( sendEvent );
#else
        sendEvent.Notify ( "PushSend" );
#endif
        
        /// Send session id to client
		MediatorMsg * sessMsg = ( MediatorMsg * ) &regMsg;

		sessMsg->size = sizeof ( MediatorMsg );
		sessMsg->cmd0 = MEDIATOR_PROTOCOL_VERSION;
		sessMsg->cmd1 = MEDIATOR_CMD_SESSION_ASSIGN;
		sessMsg->opt0 = MEDIATOR_OPT_NULL;
		sessMsg->opt1 = MEDIATOR_OPT_NULL;
		sessMsg->ids.sessionID = sid;

		CVerbArgID ( "HandleDeviceRegistrationV4 [ %s ]:\tSending session id [%i] to client.", client->ips, sid );

		sentBytes = SendBuffer ( client, sessMsg, sessMsg->size );
		if ( sentBytes != ( int ) sessMsg->size ) {
			CErrID ( "HandleDeviceRegistrationV4: Failed to assign session id!" );
			return false;
		}

		return true;
	}


	bool MediatorDaemon::SecureChannelAuth ( ThreadInstance * client )
	{
		CVerb ( "SecureChannelAuth" );

		bool			ret			= false;
		char			buffer [ ENVIRONS_MAX_KEYBUFFER_SIZE ]; /// cert + challenge + instruct + size(4)
		unsigned int *	pUI			= ( unsigned int * ) buffer;
		int				length		= 0;
		int				recvLength	= 0;
		int				sentBytes   = -1;
		//int				addrLen		= sizeof ( struct sockaddr );

		if ( !certificate ) {
			CErr ( "SecureChannelAuth: Certificate missing!" );
			return false;
		}

		/// get random number
		unsigned int challenge = rand ();

		/// Send instruction (4 bytes: 1234 means multiply with 42) and random number (4 bytes)
		*pUI = 1234; pUI++;
		*pUI = challenge; pUI++;
		*pUI = MEDIATOR_PROTOCOL_VERSION; pUI++;
		length += 12;

		/// Send certificate (4 bytes size, x bytes cert)
		unsigned int certSize = ( *( ( unsigned int * ) certificate ) & 0xFFFF ) + 4; /// We add 4 bytes for the formatSize "header"
		*pUI = certSize; pUI++;
		memcpy ( pUI, certificate, certSize );
		length += certSize + 4;

		/// Add the padding id
		*pUI |= encPadding;

		int sock = ( int ) client->socket;

		if ( sock != -1 )
			sentBytes = ( int ) send ( sock, buffer, length, MSG_NOSIGNAL );

		if ( ( int ) length != sentBytes ) {
			LogSocketError ();
			CVerbArg ( "SecureChannelAuth [ %i ]: Sending of auth token failed [ %u ] != [ %i ].", client->socket, length, sentBytes );
            
            client->sendFails++;
			return false;
		}
		client->sendFails = 0;

		char *			recBuffer		= buffer;
		unsigned int	recBufferSize	= ENVIRONS_MAX_KEYBUFFER_SIZE - 1;
		int             msgSize			= 0;
		int				bytesReceived	= 0;

	ReceiveNext:
		/// Wait for response
		//bytesReceived = ( int ) recvfrom ( ( int ) client->socket, recBuffer, recBufferSize, 0, ( struct sockaddr* ) &client->addr, ( socklen_t * ) &addrLen );
		bytesReceived = ( int ) recv ( ( int ) client->socket, recBuffer, recBufferSize, 0 );
		if ( bytesReceived <= 0 ) {
			CLogArg ( "SecureChannelAuth [ %i ]: Socket closed; [ %i bytes ]!", client->socket, bytesReceived ); return false;
		}

		length += bytesReceived;

		if ( !msgSize ) {
			msgSize = *( reinterpret_cast<unsigned int *>( buffer ) ) & 0xFFFF;

			if ( msgSize >= ENVIRONS_MAX_KEYBUFFER_SIZE ) {
				CErrArg ( "SecureChannelAuth [ %i ]: Message size [ %u ] would overflow receive buffer [ %u ]. Aborting transfer.", client->socket, msgSize, ENVIRONS_MAX_KEYBUFFER_SIZE );
				return false;;
			}

			if ( msgSize < 260 ) {
				CWarnArg ( "SecureChannelAuth [ %i ]: Expected size of response [ %i ] is less than the required 260 bytes; Bytes [ %i ]!", client->socket, msgSize, length ); return false;
			}
		}

		if ( length < msgSize ) {
			CVerbArg ( "SecureChannelAuth [ %i ]: Received message is not not complete.", client->socket );

			recBuffer		+= bytesReceived;
			recBufferSize	-= bytesReceived;
			goto ReceiveNext;
		}

		recvLength = length;
		buffer [ recvLength ] = 0;


		/// Decrypt the response
		char *	msg = 0;
		char *	hash = 0;

		do
		{
			pUI = ( unsigned int * ) buffer;
			length = *pUI++;

			if ( length < 260 ) {
				CLogArg ( "SecureChannelAuth [ %i ]: Received response is not a correct hash; Bytes [ %i ]!", client->socket, length ); break;
			}
			length = 256; // Hard limit length to prevent buffer overrun

			unsigned int msgLen = 0;

			if ( !DecryptMessage ( privKey, privKeySize, buffer + 4, length, &msg, &msgLen ) ) {
				CWarnArg ( "SecureChannelAuth [ %i ]: Failed to decrypt response!", client->socket ); break;
			}

			/// Compute the challenge
			challenge *= 42;

			pUI = ( unsigned int * ) ( msg + 4 );
			unsigned int	hashLen		= 0;
			unsigned int	padLen		= 0;

			/// Parse the username
			char		*	userName	= 0;
			const char	*	pass		= 0;
			length		= *pUI;

			if ( reqAuth ) {
				if ( !length ) {
					CWarnArg ( "SecureChannelAuth [ %i ]: Authentication required but no username supplied!", client->socket ); break;
				}
				userName = msg + 8;

				client->version = *msg;

				if ( length & MEDIATOR_MESSAGE_LENGTH_FLAG_PAD ) {
					ClearBit ( length, MEDIATOR_MESSAGE_LENGTH_FLAG_PAD );

					padLen = GetPad ( length, MEDIATOR_MESSAGE_UNIT_ALIGN );
				}

				if ( length >= recvLength || length <= 0 ) {
					CErrArg ( "SecureChannelAuth [ %i ]: Invalid length of username [ %i ]!", client->socket, length ); break;
				}

				pUI = ( unsigned int * ) ( userName + length + padLen );

				msgLen = *pUI; /// length of the hash
							   //*pUI = 0; 
				userName [ length ] = 0;
				pUI++;

				/// Look through db for user
				string user = userName;

				if ( anonymousLogon ) {
					if ( !strncmp ( user.c_str (), anonymousUser, MAX_NAMEPROPERTY ) ) {
						CLogArg ( "SecureChannelAuth [ %i ]: Anonymous logon.", client->socket );
						pass = anonymousPassword;
						client->authLevel = 0;
					}
				}

				if ( !pass ) {
					std::transform ( user.begin (), user.end (), user.begin (), ::tolower );

					const map<string, UserItem *>::iterator iter = usersDB.find ( user );
					if ( iter == usersDB.end () ) {
						CLogArg ( "SecureChannelAuth [ %i ]: User [ %s ] not found.", client->socket, user.c_str () );
					}
					else {
						UserItem * item = iter->second;

						pass = item->pass.c_str ();
						client->createAuthToken = true;
						client->authLevel = item->authLevel;
					}

					if ( !pass ) {
						CVerbArg ( "SecureChannelAuth [ %i ]: No password for User [ %s ] available. Treating username as deviceUID and looking for authToken.", client->socket, user.c_str () );

						string deviceUID = userName;
						std::transform ( deviceUID.begin (), deviceUID.end (), deviceUID.begin (), ::tolower );

						if ( deviceMappings.Lock ( "SecureChannelAuth" ) )
						{
							const msp ( string, DeviceMapping )::iterator devIt = deviceMappings.list.find ( deviceUID );

							if ( devIt != deviceMappings.list.end () ) {
								CVerbArg ( "SecureChannelAuth [ %i ]: Found auth token for the deviceUID.", client->socket );
								pass = devIt->second->authToken;
								if ( !*pass )
									pass = 0;
								client->authLevel = devIt->second->authLevel;
							}
							deviceMappings.Unlock ( "SecureChannelAuth" );
						}
					}
				}

				if ( !pass ) {
					CVerbArg ( "SecureChannelAuth [ %i ]: No username and deviceUID [ %s ] found.", client->socket, userName );
					break;
				}
			}

			CVerbVerbArg ( "SecureChannelAuth [ %i ]: Auth user [ %s ] pass [ %s ]", client->socket, userName, ConvertToHexSpaceString ( pass, ( int ) strlen ( pass ) ) );

			if ( !BuildEnvironsResponse ( challenge, userName, pass, &hash, &hashLen ) ) {
				CErrArg ( "SecureChannelAuth [ %i ]: Failed to build response.", client->socket ); break;
			}

			CVerbVerbArg ( "SecureChannelAuth [ %i ]: Response [ %s ]", client->socket, ConvertToHexSpaceString ( hash, hashLen ) );

			if ( hashLen > msgLen ) {
				CWarnArg ( "SecureChannelAuth [ %i ]: Response/Challenge sizes not equal.", client->socket ); break;
			}

			/// Accept or not
			if ( memcmp ( pUI, hash, hashLen ) ) {
				CWarnArg ( "SecureChannelAuth [ %i ]: Response/Challenge invalid.", client->socket ); break;
			}

			/// If Accept, then derive session key from the computed AES response
			if ( !AESDeriveKeyContext ( hash, hashLen, &client->aes ) ) {
				CErrArg ( "SecureChannelAuth [ %i ]: Failed to derive AES session keys.", client->socket ); break;
			}

			client->encrypt = 1;

			msgLen = 4;
			memcpy ( buffer, "ae;;", msgLen );

			sentBytes = SendBuffer ( client, buffer, msgLen );

			if ( sentBytes != ( int ) msgLen ) {
				CWarnArg ( "SecureChannelAuth [ %i ]: Failed to send AES ACK message.", client->socket ); break;
			}

			client->authenticated = 1;
			ret = true;
		}
		while ( 0 );

		free_n ( msg );
		free_n ( hash );

		return ret;
	}
    

	bool MediatorDaemon::RemoveAcceptClient ( ThreadInstance * client )
	{
		if ( !client->inAcceptorList )
			return true;
		bool found = false;

		sp ( ThreadInstance ) clientSP;

		if ( !acceptClients.Lock ( "RemoveAcceptClient" ) )
			return false;

		msp ( void *, ThreadInstance ) &clients = acceptClients.list;

		if ( clients.size () > 0 )
		{
			// find the one in our vector
			msp ( void *, ThreadInstance )::iterator it = clients.find ( client );
			if ( it != clients.end () )
			{
				CVerbArg ( "RemoveAcceptClient: Erasing [ 0x%X ] from client list", clientSP->deviceID );

				client->inAcceptorList = false;
				clients.erase ( it );
				found = true;
			}
		}

		acceptClients.Unlock ( "RemoveAcceptClient" );
		return found;
	}


	void MediatorDaemon::HandleStuntSocketRegistration ( ThreadInstance * stuntClient, sp ( ThreadInstance ) orgClient, char * msg, unsigned int msgLen )
    {
		if ( msgLen < ( MEDIATOR_BROADCAST_SPARE_ID_LEN + sizeof ( StuntSockRegTarget ) ) ) {
			CWarnArg ( "HandleStuntSocketRegistration [ %s : %i ]:\tStunt socket registration packet is not of correct type.", stuntClient->ips, stuntClient->socket );
			return;
		}
        
        StuntSockRegTarget * regTarget   = ( StuntSockRegTarget * ) ( msg + MEDIATOR_BROADCAST_SPARE_ID_LEN - 4 );
        
		int deviceID = *( ( int * ) ( msg + 12 ) );

		CVerbID ( "HandleStuntSocketRegistration" );
        
        
        char                    *   appName     = 0;
        char                    *   areaName    = 0;
		bool						success     = false;
        int							stuntSocket = INVALID_FD;
		int							toClose		= INVALID_FD;
        
        char                        buffer [ 36 + sizeof ( AppAreaBuffer ) ];
		char                    *   key			= buffer + 4;
        
        int                     *   s           = 0;
        struct sockaddr_in      *   addr        = 0;
		StuntRegisterContext	*	ctx			= 0;
        sp ( StuntRegisterContext ) ctxSP;

		DeviceInstanceNode      *   device;

		sp ( DeviceInstanceNode )	deviceSP;

		//RemoveAcceptClient ( stuntClient );

		// We need at first the deviceID
		if ( !deviceID ) {
			CWarnID ( "HandleStuntSocketRegistration: Invalid deviceID [ 0 ] for stunt socket mapping!" ); goto Finish;
		}

		/// Verify that the registration is correct
		deviceSP = orgClient->deviceSP;
		if ( !deviceSP || ( device = deviceSP.get () ) == 0 ) {
			CWarnID ( "HandleStuntSocketRegistration: Related client is missing a device instance!" ); goto Finish;
		}

		if ( deviceID != device->info.deviceID ) {
			CWarnID ( "HandleStuntSocketRegistration: deviceIDs don't match!" ); goto Finish;
		}

		// Check for matching IP
		if ( !orgClient->daemon || stuntClient->addr.sin_addr.s_addr != orgClient->addr.sin_addr.s_addr ) {
			CWarnID ( "HandleStuntSocketRegistration: Requestor has been disposed or IP address of requestor does not match!" ); goto Finish;
		}

        stuntSocket = ( int ) stuntClient->socket;
        if ( IsInvalidFD ( stuntSocket ) )
            goto Finish;
        
        if ( regTarget->sizes [ 0 ] > 1 && regTarget->sizes [ 0 ] < MAX_NAMEPROPERTY + 1 && regTarget->sizes [ 1 ] > 1 && regTarget->sizes [ 1 ] < MAX_NAMEPROPERTY + 1 ) {
            appName = ( char * ) ( regTarget->sizes + 2 );
            appName [ regTarget->sizes [ 0 ] - 1 ] = 0;
            
            areaName = appName + regTarget->sizes [ 0 ];
            areaName [ regTarget->sizes [ 1 ] - 1 ] = 0;
        }

        ctxSP = sp_make ( StuntRegisterContext );
        if ( !ctxSP )
            goto Finish;
        ctx = ctxSP.get ();

        if ( !BuildAppAreaID ( buffer, regTarget->deviceID, appName, areaName, regTarget->channelType, regTarget->token ) )
            goto Finish;
        
        if ( LockAcquireA ( orgClient->stuntSocketLock, "HandleStuntSocketRegistration" ) )
        {
            msp ( std::string, StuntRegisterContext ) &sockets = orgClient->stuntSocketsLog;
            
            const msp ( std::string, StuntRegisterContext )::iterator &foundIt = sockets.find ( key );
            
            unsigned int now = GetEnvironsTickCount32 ();
            
            if ( foundIt == sockets.end () )
            {
                ctx->token          = regTarget->token;
                ctx->registerTime   = now;

                s = ( regTarget->channelType == MEDIATOR_STUNT_CHANNEL_MAIN ? &ctx->sockI : &ctx->sockC );
                *s = stuntSocket;

                addr    = ( regTarget->channelType == MEDIATOR_STUNT_CHANNEL_MAIN ? &ctx->addrI : &ctx->addrC );
                *addr   = stuntClient->addr;

                sockets [ key ] = ctxSP;

                success = true;
            }
            else {
                const sp ( StuntRegisterContext ) & constSP = foundIt->second;

                ctxSP   = constSP;
                ctx     = ctxSP.get ();
                
                if ( regTarget->channelType == MEDIATOR_STUNT_CHANNEL_MAIN ) {
                    s       = &ctx->sockI;
                    addr    = &ctx->addrI;
                }
                else {
                    s       = &ctx->sockC;
                    addr    = &ctx->addrC;
                }
                
                if ( now - ctx->registerTime > 10000 ) {
                    // Reuse the context if the last registration attempt has been done more than 10 seconds before
                    ctx->registerTime   = now;
                    ctx->token          = regTarget->token;
                    
                    toClose = *s; *s = stuntSocket;
                    
                    *addr   = stuntClient->addr;
                    success = true;
                }
                else {
                    if ( ctx->token == regTarget->token )
                    {
                        if ( !addr->sin_port )
                        {
                            toClose = *s; *s = stuntSocket;
                            *addr   = stuntClient->addr;
                            success = true;
                        }
                    }
                }
            }
            
            LockReleaseA ( orgClient->stuntSocketLock, "HandleStuntSocketRegistration" );
        }

	Finish:
		if ( IsValidFD ( toClose ) )
			ShutdownCloseSocket ( toClose, true, "HandleStuntSocketRegistration" );

        if ( success ) {
			send ( stuntSocket, ( char * ) &deviceID, sizeof ( deviceID ), MSG_NOSIGNAL );
            
            stuntClient->socket = stuntClient->socketToClose = INVALID_FD;

            CLogArgID ( "HandleStuntSocketRegistration: Successfully registered stunt socket on port [ %d ].", addr ? ntohs ( addr->sin_port ) : 0 );
		}
        else  {
            stuntSocket = (int) stuntClient->socket;
            
            if ( IsValidFD ( stuntSocket ) ) {
                int i = 0; send ( stuntSocket, ( char * ) &i, sizeof ( i ), MSG_NOSIGNAL );
            }
            
            CLogID ( "HandleStuntSocketRegistration: Failed to register stunt socket" );
        }
        return;
	}


	std::string escapeJsonString ( const std::string& input )
	{
		std::ostringstream ss;
		for ( std::string::const_iterator iter = input.begin (); iter != input.end (); ++iter ) {
			switch ( *iter ) {
			case '\\': ss << "\\\\"; break;
			case '"': ss << "\\\""; break;
			case '/': ss << "\\/"; break;
			case '\b': ss << "\\b"; break;
			case '\f': ss << "\\f"; break;
			case '\n': ss << "\\n"; break;
			case '\r': ss << "\\r"; break;
			case '\t': ss << "\\t"; break;
			default: ss << *iter; break;
			}
		}
		return ss.str ();
	}


	bool MediatorDaemon::SendPushNotification ( map<string, ValuePack*> * values, int clientID, const char * value )
	{
		bool ret = false;

		// check which push notification service is registered for the client

		// look for the area
		string details;
		std::stringstream ss;

		ss << clientID << "_pn";
		CVerbArg ( "Client: looking for details key %s!", ss.str ().c_str () );

		const map<string, ValuePack*>::iterator valueIt = values->find ( ss.str () );

		if ( valueIt == values->end () ) {
			CErrArg ( "Client: push notification details for client %i not found!", clientID );
			goto Finish;
		}

		details = valueIt->second->value;
		if ( details.size () < 3 ) {
			CErrArg ( "Client: push notification details are invalid (<3 char): %s!", details.c_str () );
			goto Finish;
		}

		if ( details [ 0 ] == 'g' && details [ 1 ] == 'c' && details [ 2 ] == 'm' ) {
			string clientRegID ( details.begin () + 3, details.end () );

			// Do we have a GCM api key for this area?
			const map<string, ValuePack*>::iterator notifierIt = values->find ( string ( "0_gcm" ) );

			if ( notifierIt == values->end () ) {
				CErr ( "Client: gcm api key not found!" );
				goto Finish;
			}

			string gcmKey = notifierIt->second->value;

			string gcmURL = "android.googleapis.com/gcm/send";
			/*
			$fields = array(
			"registration_ids" => array ( $regId ),
			"data" => array ( 'msg' => $value ),
			);
			{
			registration_ids: [ "clientRegID" ],
			data: [
			{ msg: "value" }
			],
			}*/
			std::stringstream ssJson;
			ssJson << "{ \"registration_ids\" : [ \"" << escapeJsonString ( clientRegID ) << "\" ],";
			ssJson << "\"data\" : { \"msg\": \"" << escapeJsonString ( string ( value ) ) << "\" }, }";

			ret = HTTPPostRequest ( string ( "android.googleapis.com" ), string ( "gcm/send" ), gcmKey, ssJson.str () );
		}
		else if ( details [ 0 ] == 'a' && details [ 1 ] == 'p' && details [ 2 ] == 's' ) {
			string clientRegID ( details.begin () + 3, details.end () );

			// Do we have a GCM api key for this project?
			const map<string, ValuePack*>::iterator notifierIt = values->find ( string ( "0_aps" ) );

			if ( notifierIt == values->end () ) {
				CErr ( "Client: aps api key not found!" );
				goto Finish;
			}

			string gcmKey = notifierIt->second->value;

			string gcmURL = "android.googleapis.com/gcm/send";
			/*
			$fields = array(
			"registration_ids" => array ( $regId ),
			"data" => array ( 'msg' => $value ),
			);
			{
			registration_ids: [ "clientRegID" ],
			data: [
			{ msg: "value" }
			],
			}*/
			std::stringstream ssJson;
			ssJson << "{ \"aps\" : { ";
			ssJson << "	\"alert\" : { ";
			ssJson << "\"body\" : \"Environs wants to connect. " << escapeJsonString ( string ( value ) ) << "\",";
			ssJson << "\"action-loc-key\" : \"Connect\"";
			ssJson << "}, \"sound\" : \"default\" }, } ";

			ret = HTTPPostRequest ( string ( "android.googleapis.com" ), string ( "gcm/send" ), gcmKey, ssJson.str () );
		}

	Finish:

		return ret;
	}


	void MediatorDaemon::NotifyClients ( unsigned int notifyID, const sp ( DeviceInstanceNode ) &device )
	{
		if ( !device )
			return;

		NotifyQueueContext * ctx = new NotifyQueueContext;
		if ( !ctx )
			return;

		ctx->notify     = notifyID;
		ctx->device     = device;

		if ( !ctx->device || !LockAcquireA ( notifyLock, "NotifyClients" ) ) {
			delete ctx;
			return;
		}

		notifyQueue.push ( ctx );
		CVerb ( "NotifyClients: Enqueue" );

		/// Trigger Notifier
		if ( pthread_cond_signal ( &notifyEvent ) ) {
			CVerb ( "NotifyClients: Watchdog signal failed." );
		}

		LockReleaseVA ( notifyLock, "NotifyClients" );
	}


	void * MediatorDaemon::NotifyClientsStarter ( void * daemon )
	{
		if ( daemon ) {
			MediatorDaemon * meddae = ( MediatorDaemon * ) daemon;
			meddae->NotifyClientsThread ();
		}
		return 0;
	}


	void MediatorDaemon::NotifyClientsThread ()
	{
		CVerbVerb ( "NotifyClientsThread" );

		NotifyQueueContext * ctx = 0;

		while ( isRunning )
		{
			LockAcquireVA ( notifyLock, "NotifyClientsThread" );

		Retry:
			if ( notifyQueue.empty () )
			{
				pthread_cond_wait ( &notifyEvent, &notifyLock );

				if ( !isRunning ) {
					LockReleaseVA ( notifyLock, "NotifyClientsThread" );
					return;
				}
				goto Retry;
			}

			ctx = notifyQueue.front ();
			notifyQueue.pop ();

			LockReleaseVA ( notifyLock, "NotifyClientsThread" );

            if ( ctx )
            {
                DEBUG_CHECK_START ();
                
				NotifyClients ( ctx );
                delete ctx;
                
                DEBUG_CHECK_MEASURE ( "NotifyClientsThread" );
            }
            
			CVerbArg ( "NotifyClientsThread: next [%i]", notifyQueue.size () );
		}

		CVerbVerb ( "NotifyClientsThread: done" );
	}


	sp ( ApplicationDevices ) MediatorDaemon::GetApplicationDevices ( const char * appName, const char * areaName )
	{
		CVerb ( "GetApplicationDevices" );

		if ( !areaName || !appName ) {
			CErr ( "GetApplicationDevices: Called with NULL argument." );
			return 0;
		}

		sp ( AreaApps )				areaApps	= 0;
		sp ( ApplicationDevices )	appDevices	= 0;

        string appsName ( appName );
        string pareaName ( areaName );
        
        DEBUG_CHECK_START ();

        if ( !areas.LockRead ( "GetApplicationDevices" ) )
			return 0;

        const msp ( string, AreaApps )::iterator & areaIt = areas.list.find ( pareaName );

        if ( areaIt != areas.list.end () ) {
			areaApps = areaIt->second;
        }

        if ( !areas.UnlockRead ( "GetApplicationDevices" ) )
			return 0;

        if ( !areaApps || !areaApps->LockRead ( "GetApplicationDevices" ) )
        {
			CLogArg ( "GetApplicationDevices: App [ %s ] not found.", appName );
			return 0;
		}

        const msp ( string, ApplicationDevices )::iterator & appsIt = areaApps->apps.find ( appsName );

		if ( appsIt != areaApps->apps.end () )
			appDevices = appsIt->second;

        areaApps->UnlockRead ( "GetApplicationDevices" );

		if ( !appDevices ) {
			CLogArg ( "GetApplicationDevices: Devicelist of App [ %s ] not found.", appName );
			CErr ( "GetApplicationDevices: Invalid appDevices!" );
			goto Finish;
		}

		if ( appDevices->access <= 0 ) {
			CWarn ( "GetApplicationDevices: appDevices is about to be disposed!" );
			appDevices = 0;
			goto Finish;
		}

		__sync_add_and_fetch ( &appDevices->access, 1 );

    Finish:
        DEBUG_CHECK_MEASURE ( "GetApplicationDevices" );
		return appDevices;
	}


	INLINEFUNC void MediatorDaemon::UnlockApplicationDevices ( ApplicationDevices * appDevices )
	{
		__sync_sub_and_fetch ( &appDevices->access, 1 );
	}

    
    bool IsSendReady ( int &sock )
    {
        struct pollfd desc;
        
        desc.events = POLLOUT | POLLERRMASK;
        desc.fd = (int) sock;
        desc.revents = 0;
        
        if ( sock != -1 )
        {
            int rc = poll ( &desc, 1, 0 );
            if ( rc > 0 ) {
                if ( desc.revents & POLLOUT )
                    return true;
            }
        }
        return false;
    }
    
    
    void MediatorDaemon::NotifyClients ( NotifyQueueContext * nctx )
    {
        const sp ( DeviceInstanceNode ) & devSP = nctx->device;
        if ( !devSP )
            return;
        
        DeviceInstanceNode  * device = devSP.get ();

		int				deviceID	= device->info.deviceID;
        const char *	areaName	= device->info.areaName;
        const char *	appName		= device->info.appName;
        unsigned int	sendSize;
        
        size_t                      size;
        sp ( ThreadInstance )		clientSP;
        sp ( ApplicationDevices )	appDevices;
        
        vsp ( ThreadInstance )      dests;
        vsp ( ThreadInstance )      destsAppArea;
        
        CLogArg ( "NotifyClients: broadcasting notify [%s]", environs::resolveName ( nctx->notify ) );

        DEBUG_CHECK_START ();

        if ( notifyTargetsSize > 0 )
        {
            if ( !LockAcquireA ( notifyTargetsLock, "NotifyClients" ) )
                return;

            /// Get the no filter clients
            do
            {
                const msp ( long long, ThreadInstance )::iterator & end = notifyTargets.end ();

                msp ( long long, ThreadInstance )::iterator clientIt = notifyTargets.begin ();

                while ( clientIt != end )
                {
                    ThreadInstance * inst = clientIt->second.get ();

                    if ( inst->subscribedToNotifications && IsValidFD ( inst->socket ) && inst->aliveLast ) {
                        dests.push_back ( clientIt->second );
                    }
                    ++clientIt;
                }
            }
            while ( 0 );

            LockReleaseVA ( notifyTargetsLock, "NotifyClients" );
        }

        DEBUG_CHECK_MEASURE ( "NotifyClients 1" );

        /// Get the AreaApps
        do
        {
            sp ( AreaApps ) areaApps;
            
            DEBUG_CHECK_START_1 ();

            if ( !areas.LockRead ( "NotifyClients" ) )
                return;
            
            const msp ( string, AreaApps )::iterator areaIt = areas.list.find ( areaName );
            
            if ( areaIt != areas.list.end () )
                areaApps = areaIt->second;

            areas.UnlockRead ( "NotifyClients" );

            AreaApps * aas = areaApps.get ();

            if ( aas && aas->notifyTargetsSize > 0  )
            {
                if ( !aas->Lock2 ( "NotifyClients" ) ) {
                    CLogArg ( "NotifyClients: Area [ %s ] not found.", areaName );
                    break;
                }

                const msp ( long long, ThreadInstance )::iterator & end = aas->notifyTargets.end ();

                msp ( long long, ThreadInstance )::iterator clientIt = aas->notifyTargets.begin ();

                while ( clientIt != end )
                {
                    ThreadInstance * inst = clientIt->second.get ();

                    if ( inst->subscribedToNotifications && IsValidFD ( inst->socket ) && inst->aliveLast ) {
                        dests.push_back ( clientIt->second );
                    }
                    ++clientIt;
                }

                aas->Unlock2 ( "NotifyClients" );
                
                DEBUG_CHECK_MEASURE_1 ( "NotifyClients 2" );
            }
        }
        while ( 0 );
        
        appDevices	= GetApplicationDevices ( appName, areaName );

        ApplicationDevices * ads = appDevices.get ();
        if ( ads )
        {
            DEBUG_CHECK_START_1 ();

            if ( ads->LockRead ( "NotifyClients" ) )
            {
                device = ads->devices;
                while ( device )
                {
                    const sp ( ThreadInstance ) & constSP = device->clientSP;
                    
                    ThreadInstance * inst = constSP.get ();
                    if ( inst )
                    {
                        if ( inst->filterMode == MEDIATOR_FILTER_AREA_AND_APP && IsValidFD ( inst->socket ) && inst->subscribedToNotifications && inst->aliveLast )
                        {
                            destsAppArea.push_back ( constSP );
                        }
                    }
                    
                    device = device->next;
                }
                
                CVerbVerb ( "NotifyClients: unlock." );

                ads->UnlockRead ( "NotifyClients" );
                
                DEBUG_CHECK_MEASURE_1 ( "NotifyClients 3" );
            }
            
            UnlockApplicationDevices ( ads );
        }
        
        size = dests.size ();
        if ( size > 0 )
        {
            DEBUG_CHECK_START_1 ();
            
            sp ( SendLoad ) dataSP = std::make_shared < SendLoad > ();
            if ( dataSP )
            {
                MediatorNotify * msg = ( MediatorNotify * ) calloc ( 1, sizeof ( MediatorNotify ) );
                if ( msg )
                {
                    msg->header.cmd0 = MEDIATOR_PROTOCOL_VERSION;
                    
                    msg->header.cmd1 = MEDIATOR_CMD_MEDIATOR_NOTIFY;
                    msg->header.opt0 = MEDIATOR_OPT_NULL;
                    msg->header.opt1 = MEDIATOR_OPT_NULL;
                    msg->header.msgID = nctx->notify;
                    msg->header.notifyDeviceID = deviceID;
                    
                    if ( *areaName && *appName > 0 ) {
                        if ( !BuildAppAreaField ( msg->header.sizes, appName, areaName, false ) )
                            return;
                    }
					else {
						msg->header.sizes [ 0 ] = 1; msg->header.sizes [ 1 ] = 1;
					}
					msg->header.size = sizeof ( MediatorQueryHeader ) + msg->header.sizes [ 0 ] + msg->header.sizes [ 1 ];
                    
                    dataSP->buffer = msg;
                    
                    sendSize = msg->header.size;
                    
                    for ( size_t i = 0; i < size; i++ )
                    {
                        ThreadInstance * dest = dests [ i ].get ();
                        
                        //CLogArg ( "NotifyClients: checking device [0x%X]", dest->deviceID );
                        
                        if ( !dest->deviceID || !dest->subscribedToNotifications ) {
                            continue;
                        }
#ifndef NDEBUG
                        int sock = dest->socket;
#endif
                        if ( dest->aliveLast && dest->socket != -1 ) {
#ifndef NDEBUG
                            CLogArg ( "NotifyClients: Notify device [ 0x%X : %i ]", dest->deviceID, sock );
#else
                            CLogArg ( "NotifyClients: Notify device [ 0x%X ]", dest->deviceID );
#endif
							//SendBufferOrEnqueue ( dest, dataSP, sendSize );
                            PushSend ( dest, dataSP, sendSize );
                        }
                    }
                }
                
                DEBUG_CHECK_MEASURE_1 ( "NotifyClients 4" );
            }
            dests.clear ();
        }
        
        size = destsAppArea.size ();
        if ( size > 0 )
        {
            DEBUG_CHECK_START_1 ();
            
#ifdef USE_NOTIFY_TMP_VECTORS
            int                         repeats     = 0;
            size_t                   *  sendsDone   = ( size_t * ) calloc ( size, sizeof(size_t) );
            if ( sendsDone )
            {
                sp ( SendLoad ) dataSP = std::make_shared < SendLoad > ();
                if ( dataSP )
                {
                    MediatorNotify * msg = ( MediatorNotify * ) calloc ( 1, sizeof ( MediatorNotify ) );
                    if ( msg )
                    {
                        msg->header.cmd0 = MEDIATOR_PROTOCOL_VERSION;

                        msg->header.cmd1 = MEDIATOR_CMD_MEDIATOR_NOTIFY;
                        msg->header.opt0 = MEDIATOR_OPT_NULL;
                        msg->header.opt1 = MEDIATOR_OPT_NULL;
                        msg->header.msgID = nctx->notify;
                        msg->header.notifyDeviceID = deviceID;

                        msg->header.sizes [ 0 ] = 1;
                        msg->header.sizes [ 1 ] = 1;
                        msg->header.size = sizeof ( MediatorQueryHeader ) + 2;

                        dataSP->buffer = msg;

                        sendSize = msg->header.size;

                        size_t  done = 0;

                    RetryDests:
                        for ( size_t i = 0; i < size; i++ )
                        {
                            if ( sendsDone [ i ] )
                                continue;

                            const sp ( ThreadInstance ) & constSP = destsAppArea [ i ];

                            ThreadInstance * dest = constSP.get ();

                            //CLogArg ( "NotifyClients: checking device [0x%X]", dest->deviceID );

                            if ( !dest->deviceID || !dest->subscribedToNotifications ) {
                                sendsDone [ i ] = 1; done++;
                                continue;
                            }
#ifdef DEBUGVerb
                            SOCKETSYNC sock = dest->socket;
#endif
                            if ( dest->aliveLast && IsValidFD ( dest->socket ) ) {
#ifndef NDEBUG
                                CVerbArg ( "NotifyClients: Notify device [ 0x%X : %i ]", dest->deviceID, dest->socket );
#else
                                CVerbArg ( "NotifyClients: Notify device [ 0x%X ]", dest->deviceID );
#endif
                                if ( size <= 1 || repeats > 1 ) {
                                    PushSend ( dest, dataSP, sendSize );
                                    sendsDone [ i ] = 1; done++;
                                }
                                else if ( PushSendTry ( dest, dataSP, sendSize ) ) {
                                    sendsDone [ i ] = 1; done++;
                                }
                            }
                            else {
                                sendsDone [ i ] = 1; done++;
                            }
                        }

                        if ( done < size ) {
                            repeats++;
                            goto RetryDests;
                        }
                    }
                }

                free ( sendsDone );
            }
#else
            sp ( SendLoad ) dataSP = std::make_shared < SendLoad > ();
            if ( dataSP )
            {
                MediatorNotify * msg = ( MediatorNotify * ) calloc ( 1, sizeof ( MediatorNotify ) );
                if ( msg )
                {
                    msg->header.cmd0 = MEDIATOR_PROTOCOL_VERSION;
                    
                    msg->header.cmd1 = MEDIATOR_CMD_MEDIATOR_NOTIFY;
                    msg->header.opt0 = MEDIATOR_OPT_NULL;
                    msg->header.opt1 = MEDIATOR_OPT_NULL;
                    msg->header.msgID = nctx->notify;
                    msg->header.notifyDeviceID = deviceID;
                    
                    msg->header.sizes [ 0 ] = 1;
                    msg->header.sizes [ 1 ] = 1;
                    msg->header.size = sizeof ( MediatorQueryHeader ) + 2;
                    
                    dataSP->buffer = msg;
                    
                    sendSize = msg->header.size;

                    for ( size_t i = 0; i < size; i++ )
                    {
						const sp ( ThreadInstance ) & constSP = destsAppArea [ i ];

                        ThreadInstance * dest = constSP.get ();
                        
                        //CLogArg ( "NotifyClients: checking device [0x%X]", dest->deviceID );
                        
                        if ( !dest->deviceID || !dest->subscribedToNotifications ) {
                            continue;
                        }
#ifndef NDEBUG
                        SOCKETSYNC sock = dest->socket;
#endif
                        if ( dest->aliveLast && dest->socket != -1 ) {
#ifndef NDEBUG
                            CLogArg ( "NotifyClients: Notify device [ 0x%X : %i ]", dest->deviceID, sock );
#else
                            CLogArg ( "NotifyClients: Notify device [ 0x%X ]", dest->deviceID );
#endif
							//SendBufferOrEnqueue ( dest, dataSP, sendSize );
                            PushSend ( dest, dataSP, sendSize );
                        }
                    }
                }
            }
#endif
            DEBUG_CHECK_MEASURE_1 ( "NotifyClients 5" );
            
            destsAppArea.clear ();
        }
        
        CVerbVerb ( "NotifyClients: done." );
    }
    
    
    class StartThreadInstance
    {
    public:
        StartThreadInstance ( int nr, MediatorDaemon * m ) : threadNr ( nr ), mediator ( m ) {};
        
        int threadNr;
        MediatorDaemon * mediator;
    };
    
    
#define GROW_SIZE   100
    
    bool ClientContext::Add ( const sp ( ThreadInstance ) &clientSP )
    {
        if ( !Lock ( "Add" ) )
            return false;

#ifndef ENABLE_WINSOCK_CLIENT_THREADS
        
        bool grow = true;

        if ( desc ) {
            if ( clients.size () + 2 < descCapacity ) {
                grow = false;
            }
        }
        
        if ( grow ) {
            size_t size = descCapacity + GROW_SIZE;
            
            struct pollfd * tmp = (struct pollfd *) calloc ( size, sizeof ( struct pollfd ) );
            if ( !tmp ) {
                Unlock ( "Add" );
                return false;
            }
            
            if ( desc ) {
                memcpy ( tmp, desc, descCapacity * sizeof ( struct pollfd ) );

				free_n ( desc );
            }
            desc = tmp;
            descCapacity = size;
            
            for ( int i = 0; i < size; ++i ) {
                desc [ i ].events = POLLIN | POLLERRMASK;
				desc [ i ].revents = 0;
            }
        }

		desc [ clientCount ].fd = ( int ) clientSP->socket;
#else
#	ifdef USE_ONE_TO_ONE_SOCKET_EVENTS
		bool grow = true;

		if ( revents ) {
			if ( clients.size () + 2 < descCapacity ) {
				grow = false;
			}
		}

		if ( grow ) {
			size_t size = descCapacity + GROW_SIZE;

			HANDLE * tmp = ( HANDLE * ) calloc ( size, sizeof ( HANDLE ) );
			if ( !tmp ) {
				Unlock ( "Add" );
				return false;
			}

			if ( revents ) {
				memcpy ( tmp, revents, descCapacity * sizeof ( HANDLE ) );
			}

			for ( size_t i = descCapacity; i < size; ++i ) {
				tmp [ i ] = WSACreateEvent ();
				if ( tmp [ i ] == WSA_INVALID_EVENT ) {
					Unlock ( "Add" );
					return false;
				}
			}
			
			free_n ( revents );

			revents		 = tmp;
			descCapacity = size;
		}

		if ( WSAEventSelect ( ( int ) clientSP->socket, revents [ clientCount ], FD_READ | FD_CLOSE ) == SOCKET_ERROR ) {
			CWarnArg ( "Add [ %s : %i ]:\tFailed to register receive event [ %d ]", clientSP->ips, clientSP->socket, WSAGetLastError () );
		}
#	endif
#endif
        clients.push_back ( clientSP );
        clientCount++;

		___sync_val_compare_and_swap ( &clientsChanged, 0, 1 );

        Unlock ( "Add" );
        return true;
    }
    
    
    bool ClientContext::Remove ( const sp ( ThreadInstance ) &clientSP, bool useLock, bool keepOrder )
    {
        ThreadInstance * client = clientSP.get ();
        if ( !client )
            return false;
        
        if ( useLock && !Lock ( "Remove" ) )
            return false;
        
        bool success = false;
        
#ifdef ENABLE_WINSOCK_CLIENT_THREADS

		size_t size = clients.size ();

		for ( size_t i = 0; i < size; ++i )
		{
			const sp ( ThreadInstance ) & constSP = clients [ i ];

			if ( constSP.get () == client )
			{
				clients.erase ( clients.begin () + i );

#	ifdef USE_ONE_TO_ONE_SOCKET_EVENTS
				if ( keepOrder && ( i + 1 < clientCount ) )
				{
					HANDLE tmp = revents [ i ];

					for ( size_t j = i; j < clientCount; ++j )
					{
						revents [ j ] = revents [ j + 1 ];
					}
					revents [ clientCount ] = tmp;
				}
#	endif
				clientCount--;

				___sync_val_compare_and_swap ( &clientsChanged, 0, 1 );
				success = true;
				break;
			}
		}
#else
        for ( size_t i = 0; i < clients.size (); ++i )
        {
            if ( clients [ i ].get () == client )
            {
                clients.erase ( clients.begin() + i );

                for ( size_t j = i; j < clientCount; ++j )
                {
                    desc [ j ].fd = desc [ j + 1 ].fd;
                }
                clientCount --;

				___sync_val_compare_and_swap ( &clientsChanged, 0, 1 );
                success = true;
				break;
            }
        }
#endif
        
		if ( useLock )
			Unlock ( "Remove" );
        return success;
    }
    
    
    bool MediatorDaemon::StartClientThreads ()
    {
        CVerb ( "StartClientThreads" );
        
        bool success = true;
        
        if ( !isRunning || clientThreadCount  <= 0 )
            return false;
        
        if ( !clientThreads ) {
            clientThreads = ( EnvThread ** ) calloc ( clientThreadCount, sizeof ( EnvThread  *) );
            if ( !clientThreads )
                return false;
            
            for ( unsigned int i = 0; i < clientThreadCount; i++ )
            {
                clientThreads [ i ] = new EnvThread ();
                
                if ( !clientThreads [ i ] || !clientThreads [ i ]->Init () )
                    return false;
            }
        }
        
        if ( !clientContexts ) {
            clientContexts = ( ClientContext ** ) calloc ( clientThreadCount, sizeof ( ClientContext  *) );
            if ( !clientContexts )
                return false;
            
            for ( unsigned int i = 0; i < clientThreadCount; i++ )
            {
                clientContexts [ i ] = new ClientContext ();
                if ( !clientContexts [ i ] || !clientContexts [ i ]->Init () )
                    return false;

#ifdef ENABLE_WINSOCK_CLIENT_THREADS
#	ifndef USE_ONE_TO_ONE_SOCKET_EVENTS
				CreateWSAHandle ( clientContexts [ i ]->revent, false );
#	endif
#endif
            }
        }
        
        clientEvent.ResetSync ( "StartClientThreads", false );
        
        clientThreadsAlive = true;
        
        for ( unsigned int i = 0; i < clientThreadCount; ++i )
        {
            StartThreadInstance * ctx = new StartThreadInstance ( i, this );
            
            if ( !ctx ) {
                success = false;
                break;
            }
            
            if ( clientThreads [ i ]->Run ( pthread_make_routine ( &ClientThreadsStarter ), ( void * ) ctx, "StartClientThreads" ) == 0 ) {
                delete ctx;
                success = false;
            }
        }
        
        return success;
    }
    
    
    void MediatorDaemon::StopClientThreads ()
    {
        CVerb ( "StopClientThreads" );
        
        clientThreadsAlive = false;
        
        if ( !clientThreads )
            return;
        
        for ( unsigned int i = 0; i < clientThreadCount; ++i )
        {
            clientEvent.Notify ( "StopClientThreads" );
            
            if ( clientThreads [ i ] )
            {
                ClientContext * ctx = clientContexts [ i ];
                
				/*ctx->Lock ( "StopClientThreads" );

				size_t size = ctx->clients.size ();

                for ( int j = 0; j < size; ++j )
                {
                    const sp ( ThreadInstance ) &client = ctx->clients [ j ];
                    
                    if ( IsValidFD ( (int) client->socket ) ) {
                        shutdown ( (int) client->socket, 2 );
                    }
                }

				ctx->Unlock ( "StopClientThreads" );*/

                clientThreads [ i ]->Join ( "StopClientThreads" );

				ctx->Lock ( "StopClientThreads" );

				vsp ( ThreadInstance )  tmpClients;
				size_t size = ctx->clients.size ();

                for ( int j = 0; j < size; ++j )
                {
					tmpClients.push_back ( ctx->clients [ j ] );
                }
                
                ctx->clients.clear ();

				ctx->Unlock ( "StopClientThreads" );

				/*vsp ( ThreadInstance )::iterator it = tmpClients.begin ();

				while ( it != tmpClients.end () )
				{
					const sp ( ThreadInstance ) &client = *it;

					ClientRemove ( client.get () );

					ctx->Remove ( client, true, false );

					++it;
				}*/

				tmpClients.clear ();
            }
        }
    }
    
    
    void * MediatorDaemon::ClientThreadsStarter ( void * arg )
    {
        StartThreadInstance * ctx = (StartThreadInstance *) arg;
        if ( !ctx ) {
            CErr ( "ClientThreadsStarter: called with (NULL) argument!" );
            return 0;
        }
        
        MediatorDaemon * mediator = ctx->mediator;
        
        mediator->ClientThreads ( ctx->threadNr );
        
        if ( mediator->clientThreads [ ctx->threadNr ] )
            mediator->clientThreads [ ctx->threadNr ]->Detach ( "ClientThreadsStarter" );
        
        delete ctx;
        return 0;
    }
    

#define CLIENT_THREAD_TIMEOUT_START		100
#define CLIENT_THREAD_TIMEOUT_MAX		2000
#define CLIENT_THREAD_TIMEOUT_STEPS		100
    

    void  MediatorDaemon::ClientThreads ( int threadNr )
    {
        CLogArg ( "ClientThreads [ %i ]: started ...", threadNr );
        
        pthread_setname_current_envthread ( "MediatorDaemon::ClientThreads" );
        
        if ( !isRunning )
            return;

        ClientContext * ctx = clientContexts [ threadNr ];
		int timeout		= CLIENT_THREAD_TIMEOUT_START;
		int emptyRound	= 0;

        vsp ( ThreadInstance )  tmpClients;
        vsp ( ThreadInstance )  toRemove;

		size_t					i, size;

#ifdef ENABLE_WINSOCK_CLIENT_THREADS
		int						rcWait;
#endif
        while ( clientThreadsAlive )
        {
#ifdef ENABLE_WINSOCK_CLIENT_THREADS
			rcWait = -1;
#endif
#ifndef ENABLE_WINSOCK_CLIENT_THREADS
            bool polled = false;
#endif
            if ( ctx->clientCount > 0 ) {
#ifdef ENABLE_WINSOCK_CLIENT_THREADS
#	ifdef USE_ONE_TO_ONE_SOCKET_EVENTS
				rcWait = ( int ) WSAWaitForMultipleEvents ( ( DWORD ) ctx->clientCount, ctx->revents, FALSE, timeout, FALSE );

				if ( rcWait == WSA_WAIT_TIMEOUT ) {
					if ( timeout < CLIENT_THREAD_TIMEOUT_MAX )
						timeout += CLIENT_THREAD_TIMEOUT_STEPS;
					rcWait = -1;
				}
				else {
					if ( rcWait < ctx->clientCount )
						WSAResetEvent ( ctx->revents [ rcWait ] );
					else
						rcWait = -1;
					timeout = CLIENT_THREAD_TIMEOUT_START;
				}
#else
				if ( ( rcWait = WSAWaitForMultipleEvents ( 1, &ctx->revents, FALSE, timeout, FALSE ) ) == WSA_WAIT_TIMEOUT ) {
					if ( timeout < CLIENT_THREAD_TIMEOUT_MAX )
						timeout += CLIENT_THREAD_TIMEOUT_STEPS;
				}
				else {
					WSAResetEvent ( ctx->revents );
					timeout = CLIENT_THREAD_TIMEOUT_START;
				}
#endif
#else
                for ( i = 0; i < ctx->clientCount; ++i ) {
                    ctx->desc [ i ].revents = 0;
                }
                
                int rc = poll ( ctx->desc, (unsigned int) ctx->clientCount, timeout );
				if ( rc == 0 ) {
					if ( timeout < CLIENT_THREAD_TIMEOUT_MAX )
						timeout += CLIENT_THREAD_TIMEOUT_STEPS;
				}
				else
					timeout = CLIENT_THREAD_TIMEOUT_START;

				polled = true;
#endif
            }
            else {
                clientEvent.WaitOne ( "ClientThreads", 1000, true, true );
                
                clientEvent.ResetSync ( "ClientThreads", false );
                
                clientEvent.UnlockCond ( "ClientThreads" );
            }
			emptyRound = 0;

        DoNext:
			if ( !isRunning )
				break;
            size_t handled = 0;

			size = tmpClients.size ();

			LONGSYNCNV state = 0;

			if ( ctx->clientsChanged == 1 )
				state = ___sync_val_compare_and_swap ( &ctx->clientsChanged, 1, 0 );			
			if ( state == 1 )
			{
				if ( !ctx->Lock ( "ClientThreads" ) )
					break;

				tmpClients.clear ();

#ifdef ENABLE_WINSOCK_CLIENT_THREADS
				tmpClients = ctx->clients;
#else
				vsp ( ThreadInstance ) & clients = ctx->clients;

				for ( i = 0; i < ctx->clientCount; ++i )
				{
					if ( polled ) {
						if ( ctx->desc [ i ].revents & POLLERRMASK ) {

							toRemove.push_back ( clients [ i ] );
							continue;
						}
					}
					tmpClients.push_back ( clients [ i ] );
				}
#endif            
				if ( !ctx->Unlock ( "ClientThreads" ) )
					break;

				size = tmpClients.size ();
			}
#ifdef USE_ONE_TO_ONE_SOCKET_EVENTS
			else if ( rcWait >= 0 && rcWait < size )
			{
				const sp ( ThreadInstance ) &clientSP = tmpClients [ rcWait ];

				DEBUG_CHECK_START ();

				int rc = ClientThread ( clientSP );

				DEBUG_CHECK_MEASURE ( "ClientThreads" );

				handled++;
				rcWait = -1;

				if ( rc < 0 || !clientSP->aliveLast ) {
					CVerbArg ( "ClientThreads [ %i ]: Removing [ 0x%X ] ...", threadNr, clientSP->deviceID );

					toRemove.push_back ( clientSP );
					goto CheckRemovedClients;
				}
			}
#endif
            i = 0;            
            while ( i < size )
            {
                const sp ( ThreadInstance ) &clientSP = tmpClients [ i ]; ++i;

				DEBUG_CHECK_START ();

				int rc = ClientThread ( clientSP );

				DEBUG_CHECK_MEASURE ( "ClientThreads" );

				if ( rc < 0 || !clientSP->aliveLast ) {
					CVerbArg ( "ClientThreads [ %i ]: Removing [ 0x%X ] ...", threadNr, clientSP->deviceID );

					toRemove.push_back ( clientSP );
                }
                else if ( rc == 0 )
					handled++;
            }

#ifdef USE_ONE_TO_ONE_SOCKET_EVENTS
		CheckRemovedClients:
#endif
            size = toRemove.size ();
			if ( size > 0 )
            {
                i = 0;

				ctx->Lock ( "ClientThreads" );

				while ( i < size )
                {
                    const sp ( ThreadInstance ) & constSP = toRemove [ i ];
                    
					ctx->Remove ( constSP, false );
                    ++i;
				}

				ctx->Unlock ( "ClientThreads" );
                
                i = 0;

				sessions.Lock1 ( "ClientThreads" );				
				
				while ( i < size )
                {
                    const sp ( ThreadInstance ) & constSP = toRemove [ i ];
                    
					sessions.toRemove.push_back ( constSP );
					++i;
				}

				sessions.Unlock1 ( "ClientThreads" );
                
                ___sync_val_compare_and_swap ( &watchdogSessionsChanged, 0, 1 );

				if ( pthread_cond_signal ( &hWatchdogEvent ) ) {
					CVerbArg ( "ClientThreads [ %i ]: Watchdog signal failed.", threadNr );
				}

				toRemove.clear ();
				handled++;
			}
           
			if ( handled > 0 ) {
				emptyRound = 0;
				timeout = CLIENT_THREAD_TIMEOUT_START;
				goto DoNext;
			}

			emptyRound++;
			if ( emptyRound <= 2 ) {
				goto DoNext;
			}
        }
    }
    
    
    bool MediatorDaemon::StartSendThreads ()
    {
        CVerb ( "StartSendThread" );
        
        bool success = true;
        
        if ( !isRunning || sendThreadCount  <= 0 )
            return false;

		if ( !sendThreads ) {
            if ( sessionsChanged ) {
                free ( sessionsChanged );
                sessionsChanged = 0;
            }
            
            sessionsChanged = ( LONGSYNCNV * ) calloc ( sendThreadCount, sizeof ( LONGSYNCNV ) );
            if ( !sessionsChanged )
                return false;

            sendThreads = ( EnvThread ** ) calloc ( sendThreadCount, sizeof ( EnvThread  * ) );
			if ( !sendThreads )
				return false;

			for ( unsigned int i = 0; i < sendThreadCount; i++ )
			{
				sendThreads [ i ] = new EnvThread ();
                
				if ( !sendThreads [ i ] || !sendThreads [ i ]->Init () )
					return false;
                sessionsChanged [ i ] = 1;
			}
		}

#ifdef ENABLE_WINSOCK_SEND_THREADS
		WSAResetEvent ( sendEvent );
#else
		sendEvent.ResetSync ( "SendThread", false );
#endif
        sendThreadsAlive = true;        
        
        for ( unsigned int i = 0; i < sendThreadCount; ++i )
        {
            StartThreadInstance * ctx = new StartThreadInstance ( i, this );
            
            if ( !ctx ) {
                success = false;
                break;
            }
            
            if ( sendThreads [ i ]->Run ( pthread_make_routine ( &SendThreadStarter ), ( void * ) ctx, "StartSendThreads" ) == 0 ) {
                delete ctx;
                success = false;
            }
        }
        
        return success;
    }
    
    
    void MediatorDaemon::StopSendThreads ()
    {
        CVerb ( "StopSendThreads" );
        
        sendThreadsAlive = false;
        
		if ( !sendThreads )
			return;

        for ( unsigned int i = 0; i < sendThreadCount; ++i )
        {
#ifdef ENABLE_WINSOCK_SEND_THREADS
			WSASetEvent ( sendEvent );
#else
			sendEvent.Notify ( "StopSendThreads" );
#endif            
			if ( sendThreads [ i ] )
				sendThreads [ i ]->Join ( "StopSendThreads" );
        }
    }


	void * MediatorDaemon::SendThreadStarter ( void * arg )
	{
		StartThreadInstance * ctx = ( StartThreadInstance * ) arg;
		if ( !ctx ) {
			CErr ( "SendThreadStarter: called with (NULL) argument!" );
			return 0;
		}

		MediatorDaemon * mediator = ctx->mediator;

		mediator->SendThread ( ctx->threadNr );

		if ( mediator->sendThreads [ ctx->threadNr ] )
			mediator->sendThreads [ ctx->threadNr ]->Detach ( "SendThreadStarter" );

		delete ctx;
		return 0;
	}
    
    
	void MediatorDaemon::SendContextsCompress ( ThreadInstance * client )
	{
		lib::QueueVector    *   queue = &client->sendQueue;

		// Look how many contexts we can pack into one send to encrypt
		// Max size is half of client buffer of mobile listeners
		int				contexts = 0, contextsDelete;
		int				remainSize = MEDIATOR_CONTEXT_BUFFER_SIZE_MAX;

		SendContext **	items       = ( SendContext ** ) queue->items;
		int				cur         = ( int ) queue->next;
        int				capacity    = ( int ) queue->capacity;
        unsigned int	lastSize	= 0;

		SendContext *	ctx;

		while ( contexts < ( int ) queue->size_ )
		{
			ctx = items [ cur ];

			if ( ctx->sendBuffer ) {
				cur--;
				break;
			}
            
            lastSize = ctx->size;
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

		int requiredSize = ( MEDIATOR_CONTEXT_BUFFER_SIZE_MAX - remainSize ) + 128;

		char * tmp = ( char * ) malloc ( requiredSize );
		if ( !tmp )
			return;

		// Merge previous contexts to current context
		char *			curPtr  = tmp;
        unsigned int	curSize = 0;
        
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
            if ( !ctx )
                continue;
            
            unsigned int itemSize = ctx->size;
            
            if ( ctx->buffer )
                memcpy ( curPtr, ctx->buffer, itemSize );
            else
                memcpy ( curPtr, ctx->dataSP->buffer, itemSize );
            
            curSize += itemSize;
            curPtr += itemSize;
#ifdef DEBUG_DEVICE_ID
            if ( ctx->seqNr && client->deviceID == debugID ) {
                char timeString [ 256 ];
                GetTimeString ( timeString, sizeof ( timeString ) );
                printf ( "%sSendThread: Prepared response [ %i ].\n", timeString, ctx->seqNr );
            }
#endif
            
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
		else
			ctx->dataSP = 0;

		ctx->buffer = tmp;
		ctx->size   = curSize;
	}

#define SEND_THREAD_TIMEOUT_START		10
#define SEND_THREAD_TIMEOUT_MAX			2000
#define SEND_THREAD_TIMEOUT_STEPS		100

	void MediatorDaemon::SendThread ( int threadNr )
	{
		CLogArg ( "SendThread [ %i ]: started ...", threadNr );

		pthread_setname_current_envthread ( "MediatorDaemon::SendThread" );

		if ( !isRunning )
			return;

		int rc, clientCount = 0, clientBuffersFull = 0, remainSendContexts;

		pthread_mutex_t     *   lock;
        lib::QueueVector    *   queue;
        lib::QueueVector    *   queuePrio;
		int						emptyRound;
		int						clientsSend;
		int						timeout = 100;

        vsp ( ThreadInstance )  tmpClients;
        
        size_t                  clientsCacheCount       = 0;
        size_t                  clientsCacheCapacity    = 0;
        ThreadInstance      **  clientsCache            = 0;
        
        size_t i;

        while ( sendThreadsAlive )
		{
			emptyRound = 0;

#ifdef ENABLE_WINSOCK_SEND_THREADS
			//CLogArg ( "SendThread [ %i ]: Going into wait ...", threadNr );

			if ( ( rc = WSAWaitForMultipleEvents ( 1, &sendEvent, FALSE, timeout, FALSE ) ) == WSA_WAIT_TIMEOUT ) {
				if ( timeout < SEND_THREAD_TIMEOUT_MAX )
					timeout += SEND_THREAD_TIMEOUT_STEPS;
			}
			else {
				WSAResetEvent ( sendEvent );
				timeout = SEND_THREAD_TIMEOUT_START;
			}
#else
			rc = sendEvent.WaitOne ( "SendThread", timeout, true, true );
			if ( rc < 0 ) {
				if ( timeout < SEND_THREAD_TIMEOUT_MAX )
					timeout += SEND_THREAD_TIMEOUT_STEPS;
			}
			else
				timeout = SEND_THREAD_TIMEOUT_START;

			sendEvent.ResetSync ( "SendThread", false );

			sendEvent.UnlockCond ( "SendThread" );
#endif
		DoWork:
			clientBuffersFull = 0; // clients which's send buffers are full
			clientCount = 0; remainSendContexts = 0; clientsSend = 0;

			// Look for an appropriate context to send
			// and put them into the temporary vector
            if ( sessionsChanged [ threadNr ] == 1 && ( ___sync_val_compare_and_swap ( sessionsChanged + threadNr, 1, 0 ) == 1 ) )
            {
				tmpClients.clear ();

                if ( !sessions.Lock1 ( "SendThread sessions" ) )
                    break;
                
                tmpClients = sessions.cache;
                
                if ( !sessions.Unlock1 ( "SendThread sessions" ) )
                    break;
                
                clientsCacheCount = tmpClients.size ();
                
                if ( clientsCacheCount > 0 ) {
                    // Build cache
                    if ( clientsCacheCount >= clientsCacheCapacity )
                    {
                        free_m ( clientsCache );
                        clientsCacheCapacity = 0;
                        
                        size_t capacity = clientsCacheCount + 50;
                        
                        clientsCache = ( ThreadInstance ** ) malloc ( sizeof (ThreadInstance *) * capacity );
                        if ( !clientsCache )
                            break;
                        
                        clientsCacheCapacity    = capacity;
                    }
                    
                    i = 0; size_t c = 0;
                    while ( i < clientsCacheCount )
                    {
                        const sp ( ThreadInstance ) & constSP = tmpClients [ i ];
                        
                        ThreadInstance * client = constSP.get (); ++i;
                        if ( client ) {
                            clientsCache [ c ] = client; ++c;
                        }
                    }
                    clientsCacheCount = c;
                }
            }

			if ( clientsCacheCount == 0 )
				continue;

            i = 0;
			while ( isRunning && i < clientsCacheCount )
			{
                ThreadInstance * client = clientsCache [ i ]; i++;

                int state = 1;
                
				if ( client && IsValidFD ( client->socket ) && client->aliveLast )
				{
					if ( client->sendFails > 20 )
						goto DoNextClient;

					if ( client->sendBusy1 == 0 )
						state = ( int ) ___sync_val_compare_and_swap ( &client->sendBusy1, 0, 1 );

                    if ( state == 1 )
                        goto DoNextClient;
                    
					CVerbArg ( "SendThread: Checking deviceID [ 0x%X : %s ] socket [ %i ]", client->deviceID, client->ips, client->socket );

                    lock        = &client->sendQueueLock;
                    queue       = &client->sendQueue;
                    queuePrio   = &client->sendQueuePrior;

					if ( queuePrio->size_ > 0 )
					{
						if ( !LockAcquire ( lock, "SendThread" ) )
							goto DoNextClient;
					}
					else {
#ifdef ENABLE_SEND_THREAD_TRYLOCK
						if ( pthread_mutex_trylock ( lock ) )
							goto DoNextClient;
#else
						if ( !LockAcquire ( lock, "SendThread" ) )
							goto DoNextClient;
#endif
					}

					if ( queue->size_ <= 0 && queuePrio->size_ <= 0 )
					{
						LockRelease ( lock, "SendThread" );
						goto DoNextClient;
                    }
                    
                    // Delete all contexts that have been sent
                    while ( queue->size_ > 0 )
                    {
                        SendContext * ctx = ( SendContext * ) queue->first ();
                        if ( !ctx || !ctx->done )
                            break;
                        
                        queue->pop ();
                        delete ctx;
                    }
                    
                    while ( queuePrio->size_ > 0 )
                    {
                        SendContext * ctx = ( SendContext * ) queuePrio->first ();
                        if ( !ctx || !ctx->done )
                            break;
                        
                        queuePrio->pop ();
                        delete ctx;
                    }
                    
                    clientCount++;
                    
                    bool changeToPriorityQueue = false;
                    
                ContinueWithPriorityQueue:
                    while ( queuePrio->size_ > 0 )
                    {
                        SendContext * ctx;
                        
                        if ( client->sendQueue.size_ > 0 )
                        {
                            ctx = ( SendContext * ) queue->first ();
                            if ( ctx->sendCurrent ) {
                                changeToPriorityQueue = true;
                                break;
                            }
                        }
                        
                        ctx = ( SendContext * ) queuePrio->first ();
                        
                        LockRelease ( lock, "SendThread" );
                        
                        if ( ctx )
                            rc = SendBuffer ( client, ctx );
                        else
                            goto DoNextClient;
                        
                        if ( rc <= 0 ) {
                            // -1 - Error.
                            //  0 - Only partly send
                            clientBuffersFull++;
                            goto DoNextClient;
                        }
						clientsSend++;
                        
#ifdef ENABLE_SEND_THREAD_TRYLOCK_FOR_PRIO_QUEUE
                        if ( pthread_mutex_trylock ( lock ) )
                            goto DoNextClient;
#else
                        if ( !LockAcquire ( lock, "SendThread" ) )
                            goto DoNextClient;
#endif
                        ctx = ( SendContext * ) queuePrio->pop ();

                        LockRelease ( lock, "SendThread" );

                        if ( ctx )
                            delete ctx;

                        if ( !LockAcquire ( lock, "SendThread" ) )
                            goto DoNextClient;
                    }

					while ( queue->size_ > 0 )
					{
						// Handle the first queue element
						SendContext * ctx = ( SendContext * ) queue->first ();

						if ( changeToPriorityQueue || client->version < '7' || queue->size_ <= 1 || ctx->sendBuffer ) {
							LockRelease ( lock, "SendThread" );
                            
                            rc = SendBuffer ( client, ctx );
						}
						else {
							SendContextsCompress ( client );

							ctx = ( SendContext * ) queue->first ();

							LockRelease ( lock, "SendThread" );

							if ( ctx )
								rc = SendBuffer ( client, ctx );
							else
								goto DoNextClient;
						}

						if ( rc <= 0 ) {
							// -1 - Error.
							//  0 - Only partly send 
							clientBuffersFull++;
							goto DoNextClient;
						}
                        
						if ( changeToPriorityQueue ) 
						{
							if ( !LockAcquire ( lock, "SendThread" ) )
								goto DoNextClient;
						}
						else {
#ifdef ENABLE_SEND_THREAD_TRYLOCK
							if ( pthread_mutex_trylock ( lock ) )
								goto DoNextClient;
#else
							if ( !LockAcquire ( lock, "SendThread" ) )
								goto DoNextClient;
#endif
						}

                        ctx = ( SendContext * ) queue->pop ();

                        LockRelease ( lock, "SendThread" );

                        if ( ctx )
                            delete ctx;

                        if ( !LockAcquire ( lock, "SendThread" ) )
                            goto DoNextClient;
                        
                        if ( changeToPriorityQueue || client->sendQueuePrior.size_ > 0 ) {
                            changeToPriorityQueue = false;
                            goto ContinueWithPriorityQueue;
                        }
					}

					LockRelease ( lock, "SendThread" );
				}

			DoNextClient:
                if ( state == 0 )
                    ___sync_val_compare_and_swap ( &client->sendBusy1, 1, 0 );

                remainSendContexts += ( int ) client->sendQueue.size_;
                remainSendContexts += ( int ) client->sendQueuePrior.size_;
			}

			/// Trigger Watchdog
			//if ( LockAcquireA ( thread_lock, "Watchdog" ) ) {

				/*if ( pthread_cond_signal ( &hWatchdogEvent ) ) {
					CVerbArgID ( "SendThread [ %i ]:\tWatchdog signal failed.", threadNr );
				}*/

				//LockReleaseVA ( thread_lock, "Watchdog" );
			//}

			if ( !isRunning )
				break;

			if ( clientsSend > 0 && ( remainSendContexts > 0 || clientBuffersFull != clientCount
#ifndef ENABLE_WINSOCK_SEND_THREADS
				|| sendEvent.IsSetDoReset ()
#endif    
				) )
			{
				emptyRound = 0; timeout = SEND_THREAD_TIMEOUT_START;
				goto DoWork;
			}

			emptyRound++;
			if ( emptyRound <= 2 ) {
				timeout = SEND_THREAD_TIMEOUT_START;
				goto DoWork;
			}

			i = 0;
			while ( i < clientsCacheCount )
			{
				ThreadInstance * client = clientsCache [ i ]; i++;
				if ( client->sendQueuePrior.size_ > 0 ) 
				{
					// Signal another send thread just to make sure that 
					// priority queue is drained for sure 
					if ( clientsCacheCount < 3000 )
					{
#ifdef ENABLE_WINSOCK_SEND_THREADS
						WSASetEvent ( sendEvent );
						WSAResetEvent ( sendEvent );
#else
						sendEvent.Notify ( "SendThread" );
						sendEvent.ResetSync ( "SendThread", true );
#endif
					}
					goto DoWork;
				}

				if ( client->sendQueue.size_ > 0 ) 
				{
					// Signal another send thread just to make sure that 
					// some other entity tries to drain the queue before we're going to wait some cycles
					if ( clientsCacheCount < 3000 )
					{
#ifdef ENABLE_WINSOCK_SEND_THREADS
						WSASetEvent ( sendEvent );
						WSAResetEvent ( sendEvent );
#else
						sendEvent.Notify ( "SendThread" );
						sendEvent.ResetSync ( "SendThread", true );
#endif
					}
					timeout = 10;
					break;
				}
			}
        }
        
        free_n ( clientsCache );

		CLogArg ( "SendThread [ %i ]: bye bye ...", threadNr );
	}

    
    bool MediatorDaemon::PushSend ( ThreadInstance * client, const sp ( SendLoad ) &dataSP, unsigned int size )
    {
        bool success = false;
        
        SendContext * ctx = new SendContext ();
        while ( ctx )
        {
            ctx->buffer = 0;
            ctx->dataSP = dataSP;
            ctx->size	= size;
            
            DEBUG_CHECK_START ();
            
            if ( !LockAcquireA ( client->sendQueueLock, "PushSend" ) )
                break;

			if ( IsValidFD ( ( int ) client->socket ) )
				success = client->sendQueue.push ( ctx );            
          
            LockReleaseA ( client->sendQueueLock, "PushSend" );
            
            DEBUG_CHECK_MEASURE ( "PushSend" );
            
            if ( !success )
                break;

#ifdef ENABLE_WINSOCK_SEND_THREADS
			WSASetEvent ( sendEvent );
#else
			sendEvent.Notify ( "PushSend" );
#endif              
            return true;
        }
        
        if ( ctx )
            delete ctx;
        return false;
    }
    
    
    bool MediatorDaemon::PushSend ( ThreadInstance * client, void * buffer, unsigned int size, bool copy, unsigned int seqNr )
    {
		bool success = false;

        SendContext * ctx = new SendContext ();
        while ( ctx )
        {
            if ( copy ) {
				ctx->buffer = ( char * ) malloc ( size );
                if ( !ctx->buffer )
                    break;
                
                memcpy ( ctx->buffer, buffer, size );
            }
			else
				ctx->buffer = ( char * ) buffer;

            ctx->size   = size;
            ctx->seqNr  = seqNr;
            
            DEBUG_CHECK_START ();

            if ( !LockAcquireA ( client->sendQueueLock, "PushSend" ) )
                break;
            
            if ( IsValidFD ( ( int ) client->socket ) ) {
                if ( seqNr )
                    success = client->sendQueuePrior.push ( ctx );
                else
                    success = client->sendQueue.push ( ctx );
            }

            LockReleaseA ( client->sendQueueLock, "PushSend" );
            
            DEBUG_CHECK_MEASURE ( "PushSend" );
            
			if ( !success )
				break;

#ifdef ENABLE_WINSOCK_SEND_THREADS
			WSASetEvent ( sendEvent );
#else
			sendEvent.Notify ( "PushSend" );
#endif    
            return true;
        }
        
		if ( ctx ) {
			if ( !copy )
				ctx->buffer = 0;
			delete ctx;
		}

        return false;
    }
    
    
#ifdef PUSH_PARTIAL_SENDS
	bool MediatorDaemon::PushSend ( ThreadInstance * client, char * toSend, unsigned int toSendSize, unsigned int toSendCurrent, bool copy, unsigned int seqNr )
	{
		bool success = false;

		SendContext * ctx = new SendContext ();
		while ( ctx )
        {
#ifdef XCODE_ANALYZER_BUG
            ctx->freeSendBuffer	= false;
#endif
			if ( copy ) {
				ctx->buffer = ( char * ) malloc ( toSendSize );
				if ( !ctx->buffer )
					break;

                memcpy ( ctx->buffer, toSend, toSendSize );
			}
			else {
				ctx->buffer			= 0;
                ctx->size			= 0;
			}

            ctx->seqNr          = seqNr;
			ctx->sendBuffer		= toSend;
			ctx->sendSize		= toSendSize;
            ctx->sendCurrent	= toSendCurrent;
            
            DEBUG_CHECK_START ();

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
            
            DEBUG_CHECK_MEASURE ( "PushSend" );

			if ( !success )
                break;
            
            if ( !copy )
                ctx->freeSendBuffer	= true;
            
#ifdef ENABLE_WINSOCK_SEND_THREADS
			WSASetEvent ( sendEvents [ 1 ] );
#else
			sendEvent.Notify ( "PushSend" );
#endif    
			return true;
		}

		if ( ctx ) {
			delete ctx;
		}
		return false;
	}


	bool MediatorDaemon::PushSend ( ThreadInstance * client, const sp ( SendLoad ) &dataSP, char * toSend, unsigned int toSendSize, unsigned int toSendCurrent )
	{
		bool success = false;

		SendContext * ctx = new SendContext ();
		while ( ctx )
		{
			ctx->buffer			= 0;
			ctx->size			= 0;
			ctx->dataSP			= dataSP;

			ctx->sendBuffer		= toSend;
			ctx->sendSize		= toSendSize;
			ctx->sendCurrent	= toSendCurrent;
            ctx->freeSendBuffer	= ( dataSP->buffer != toSend );
            
            DEBUG_CHECK_START ();

			if ( !LockAcquireA ( client->sendQueueLock, "PushSend" ) )
				break;

			if ( IsValidFD ( ( int ) client->socket ) )
				success = client->sendQueue.push ( ctx );

            LockReleaseA ( client->sendQueueLock, "PushSend" );
            
            DEBUG_CHECK_MEASURE ( "PushSend" );

			if ( !success )
				break;

#ifdef ENABLE_WINSOCK_SEND_THREADS
			WSASetEvent ( sendEvents [ 1 ] );
#else
			sendEvent.Notify ( "PushSend" );
#endif    
			return true;
		}

		if ( ctx ) {
			if ( ctx->freeSendBuffer ) {
				ctx->sendBuffer = 0;
			}
			delete ctx;
		}
		return false;
    }
#endif
    
    
#ifdef USE_NOTIFY_TMP_VECTORS
    bool MediatorDaemon::PushSendTry ( ThreadInstance * client, const sp ( SendLoad ) &dataSP, unsigned int size )
    {
        if ( pthread_mutex_trylock ( &client->sendQueueLock ) )
            return false;

        bool success = false;
        
        SendContext * ctx = new SendContext ();
        if ( ctx )
        {
            ctx->buffer = 0;
            ctx->dataSP = dataSP;
            ctx->size	= size;
            
            if ( IsValidFD ( ( int ) client->socket ) )
                success = client->sendQueue.push ( ctx );
            
            LockReleaseA ( client->sendQueueLock, "PushSendTry" );
            
            if ( !success ) {
                delete ctx;
                return false;
            }
            
#ifdef ENABLE_WINSOCK_SEND_THREADS
            WSASetEvent ( sendEvent );
#else
            sendEvent.Notify ( "PushSendTry" );
#endif
            return true;
        }
        
        LockReleaseA ( client->sendQueueLock, "PushSendTry" );
        
        return false;
    }
#endif

    int MediatorDaemon::SendBuffer ( ThreadInstance * client, SendContext * ctx )
    {
        CVerbVerbArg ( "SendBuffer [ %i ]", client->socket );
        
        int             rc          = -1;
        char *          toSend      = 0;
        unsigned int    toSendLen   = 0;
        int             deviceID    = client->deviceID;
        
        CVerbArgID ( "SendBuffer [ %i ]", client->socket );
        
        if ( !client->aliveLast || IsInvalidFD ( client->socket ) )
            return -1;
        
        if ( !ctx->sendBuffer )
        {
            char * sendLoad = ctx->buffer;
            
            if ( !sendLoad ) {
                SendLoad * data = ctx->dataSP.get ();
                if ( !data )
                    return 1;
                
                sendLoad = (char *) data->buffer;
                if ( !data )
                    return 1;
            }
            
            if ( client->encrypt )
            {
                char * cipher = 0;
                toSendLen = ctx->size;
                
                DEBUG_CHECK_START ();
                
                if ( !AESEncrypt ( &client->aes, sendLoad, &toSendLen, &cipher ) || !cipher ) {
                    CErrArgID ( "SendBuffer [ %i ]: Failed to encrypt AES message.", client->socket );
                    
                    // Failed to encrypt. For now, we just skip this send context and go on to the next one
                    return 1;
                }
                
                DEBUG_CHECK_MEASURE ( "SendBuffer AESEncrypt" );
                
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
        
        CVerbArgID ( "SendBuffer [ %i ]: [ %i ] bytes", client->socket, toSendLen );
        
        if ( client->aliveLast && IsValidFD ( (int) client->socket ) )
        {
            rc = ( int ) send ( ( int ) client->socket, toSend + ctx->sendCurrent, toSendLen - ctx->sendCurrent, MSG_NOSIGNAL );
            if ( rc < 0 )
            {
				SOCKET_Check_Val ( check );
                
                if ( SOCKET_Check_Retry ( check ) ) {
                    return 0;
                }
            }
            
            else if ( rc > 0 )
            {
                ctx->sendCurrent += rc;
                if ( ctx->sendCurrent == toSendLen ) {
                    client->sendFails = 0;
                    rc = 1;
                    ctx->done = true;
                    
#if defined(DEBUG_DEVICE_ID) || defined(DEBUG_TIMEOUT_ISSUE)
                    if ( deviceID == debugID && ctx->seqNr ) {
                        char timeString [ 256 ];
                        GetTimeString ( timeString, sizeof ( timeString ) );
                        printf ( "%sSendBuffer: Send response [ %i ].\n", timeString, ctx->seqNr );
                    }
#endif
                }
                
                else rc = 0;
            }
        }
        else {
            client->sendFails++;
        }
        
        CVerbArgID ( "SendBuffer [ %i ]: [ %i ] bytes. Done [ %i ].", client->socket, toSendLen, rc );
        return rc;
    }


	int MediatorDaemon::SendBuffer ( ThreadInstance * client, void * msg, unsigned int msgLen )
	{
		CVerbVerbArg ( "SendBuffer [ %i ]", client->socket );

		int             rc          = -1;
		char *          cipher      = 0;
		unsigned int    toSendLen   = msgLen;
		unsigned int    bytesSent   = 0;
		int             deviceID    = client->deviceID;

		CVerbArgID ( "SendBuffer [ %i ]", client->socket );

		if ( IsInvalidFD ( ( int ) client->socket ) )
			return -1;

        if ( client->encrypt ) {
            DEBUG_CHECK_START ();
            
			if ( !AESEncrypt ( &client->aes, ( char * ) msg, &toSendLen, &cipher ) || !cipher ) {
				CErrArgID ( "SendBuffer [ %i ]: Failed to encrypt AES message.", client->socket );
				return rc;
            }
            
            DEBUG_CHECK_MEASURE ( "SendBuffer AESEncrypt" );
			msg = cipher;
		}

		CVerbArgID ( "SendBuffer [ %i ]: [ %i ] bytes", client->socket, toSendLen );

	SendNext:
		if ( client->aliveLast && client->socket != -1 )
		{
			rc = ( int ) send ( ( int ) client->socket, ( ( char * ) msg ) + bytesSent, toSendLen - bytesSent, MSG_NOSIGNAL );
			if ( rc > 0 )
			{
				bytesSent += rc;
				if ( bytesSent == toSendLen ) {
					client->sendFails = 0;
					rc = msgLen;
				}
				else
					goto SendNext;
			}
			else
				client->sendFails++;
		}
		else {
			client->sendFails++;
		}

		CVerbArgID ( "SendBuffer [ %i ]: [ %i ] bytes. Done.", client->socket, toSendLen );

		if ( cipher )
			free ( cipher );
		return rc;
	}

    
#ifdef USE_TRY_SEND_BEFORE_THREAD_SEND
	bool MediatorDaemon::SendBufferOrEnqueue ( ThreadInstance * client, void * msg, unsigned int size, bool copy, unsigned int seqNr )
	{
		CVerbVerbArg ( "SendBufferOrEnqueue [ %i ]", client->socket );

		if ( client->sendQueue.size_ > 0 ) {
			return PushSend ( client, msg, size, copy, seqNr );
		}

		bool			success		= false;
		int             rc          = -1;
		char *          cipher      = 0;
		char *			toSend		= ( char * ) msg;
		unsigned int    toSendLen   = size;
		unsigned int    bytesSent   = 0;
		int             deviceID    = client->deviceID;

		CVerbArgID ( "SendBufferOrEnqueue [ %i ]", client->socket );

		if ( IsInvalidFD ( ( int ) client->socket ) )
			return false;

		if ( client->encrypt ) {
            DEBUG_CHECK_START ();
            
			if ( !AESEncrypt ( &client->aes, ( char * ) msg, &toSendLen, &cipher ) || !cipher ) {
				CErrArgID ( "SendBufferOrEnqueue [ %i ]: Failed to encrypt AES message.", client->socket );
				return false;
			}
            
            DEBUG_CHECK_MEASURE ( "SendBufferOrEnqueue AESEncrypt" );
			toSend = cipher;
		}

		CVerbArgID ( "SendBufferOrEnqueue [ %i ]: [ %i ] bytes", client->socket, toSendLen );

		if ( client->aliveLast && IsValidFD ( client->socket ) )
		{
			bool push = false;

			rc = ( int ) send ( ( int ) client->socket, toSend + bytesSent, toSendLen - bytesSent, MSG_NOSIGNAL );
			if ( rc >= 0 )
			{
				bytesSent += rc;
				if ( bytesSent == toSendLen )
				{
					client->sendFails = 0;

					if ( !copy && msg != cipher )
						free ( msg );

					success = true;
				}
				else push = true;
			}
			else {
				SOCKET_Check_Val ( check );

				if ( SOCKET_Check_Retry ( check ) )
					push = true;
			}

			if ( push && PushSend ( client, toSend, toSendLen, bytesSent, ( copy && toSend != cipher ), seqNr ) )
			{
				if ( !copy && toSend != msg ) {
					free ( msg );
				}
				cipher = 0;
				success = true;
			}
		}
		else client->sendFails++;

		CVerbArgID ( "SendBufferOrEnqueue [ %i ]: [ %i ] bytes. Done.", client->socket, toSendLen );

		if ( cipher )
			free ( cipher );
		return success;
	}


	int MediatorDaemon::SendBufferOrEnqueue ( ThreadInstance * client, const sp ( SendLoad ) &dataSP, unsigned int size )
	{
		CVerbVerbArg ( "SendBufferOrEnqueue [ %i ]", client->socket );

		if ( client->sendQueue.size_ > 0 ) {
			PushSend ( client, dataSP, size );
			return size;
		}

		int             rc          = -1;
		char *          cipher      = 0;
		char *			msg			= (char *) dataSP->buffer;
		unsigned int    toSendLen   = size;
		unsigned int    bytesSent   = 0;
		int             deviceID    = client->deviceID;

		CVerbArgID ( "SendBufferOrEnqueue [ %i ]", client->socket );

		if ( IsInvalidFD ( ( int ) client->socket ) )
			return -1;

        if ( client->encrypt ) {
            DEBUG_CHECK_START ();
            
			if ( !AESEncrypt ( &client->aes, ( char * ) msg, &toSendLen, &cipher ) || !cipher ) {
				CErrArgID ( "SendBufferOrEnqueue [ %i ]: Failed to encrypt AES message.", client->socket );
				return rc;
            }
            
            DEBUG_CHECK_MEASURE ( "SendBufferOrEnqueue AESEncrypt" );
			msg = cipher;
		}

		CVerbArgID ( "SendBufferOrEnqueue [ %i ]: [ %i ] bytes", client->socket, toSendLen );

		if ( client->aliveLast && IsValidFD ( client->socket ) )
		{
			bool push = false;

			rc = ( int ) send ( ( int ) client->socket, ( ( char * ) msg ) + bytesSent, toSendLen - bytesSent, MSG_NOSIGNAL );
			if ( rc >= 0 )
			{
				bytesSent += rc;
				if ( bytesSent == toSendLen ) {
					client->sendFails = 0;
					rc = size;
				}
				else push = true;
			}
			else {
				SOCKET_Check_Val ( check );

				if ( SOCKET_Check_Retry ( check ) )
					push = true;
			}

			if ( push ) {
				PushSend ( client, dataSP, ( char * ) msg, toSendLen, bytesSent );
				cipher = 0;
				rc = size;
			}
		}
		else client->sendFails++;

		CVerbArgID ( "SendBufferOrEnqueue [ %i ]: [ %i ] bytes. Done.", client->socket, toSendLen );

		if ( cipher )
			free ( cipher );
		return rc;
	}
#endif
    
    void MediatorDaemon::CheckProjectValues ()
    {
        std::time_t now = std::time ( 0 );
        
        const unsigned long long maxAliveSecs = 60 * 60 * 1;
        
        DEBUG_CHECK_START ();
        
		if ( !areasMap.Lock ( "CheckProjectValues" ) )
            return;
        
		// Collect areas
		msp ( string, AppsList ) tmpAreas;
		for ( msp ( string, AppsList )::iterator it = areasMap.list.begin (); it != areasMap.list.end (); ++it )
		{
			sp ( AppsList ) appsList = it->second;
			if ( !appsList )
				continue;

			tmpAreas [ it->first ] = appsList;
		}

		if ( !areasMap.Unlock ( "CheckProjectValues" ) )
            return;
        
        DEBUG_CHECK_MEASURE ( "CheckProjectValues 1" );
        
		// Save areas
		for ( msp ( string, AppsList )::iterator it = tmpAreas.begin (); it != tmpAreas.end (); ++it )
		{
			sp ( AppsList ) appsList = it->second;
			if ( !appsList )
                continue;
            
			DEBUG_CHECK_START_1 ();
            
			if ( !appsList->Lock ( "CheckProjectValues" ) )
				continue;

			msp ( string, ListValues ) tmpListValues;

			for ( msp ( string, ListValues )::iterator ita = appsList->apps.begin (); ita != appsList->apps.end (); ++ita )
			{
				sp ( ListValues ) listValues = ita->second;
				if ( !listValues )
					continue;

				tmpListValues [ ita->first ] = listValues;
			}

            appsList->Unlock ( "CheckProjectValues" );
            
            DEBUG_CHECK_MEASURE_1 ( "CheckProjectValues 2" );
            
			for ( msp ( string, ListValues )::iterator ita = tmpListValues.begin (); ita != tmpListValues.end (); ++ita )
			{
                sp ( ListValues ) listValues = ita->second;
                
				DEBUG_CHECK_START_1 ();
                
				if ( !listValues || !listValues->Lock ( "CheckProjectValues" ) )
					continue;

				for ( msp ( string, ValuePack )::iterator itv = listValues->values.begin (); itv != listValues->values.end (); )
				{
					bool addIt = true;

					sp ( ValuePack ) value = itv->second;
					if ( value ) {
						if ( ( now - value->timestamp ) > ( signed ) maxAliveSecs ) {
							listValues->values.erase ( itv++ );
							addIt = false;
						}
					}
					if ( addIt )
						++itv;
				}

                listValues->Unlock ( "CheckProjectValues" );
                
                DEBUG_CHECK_MEASURE_1 ( "CheckProjectValues 3" );
			}
		}
	}


	void * MediatorDaemon::WatchdogThreadStarter ( void * arg )
	{
		MediatorThreadInstance * listeners = ( MediatorThreadInstance * ) arg;
		if ( listeners->instance.daemon ) {
			// Execute thread
			( ( MediatorDaemon * ) listeners->instance.daemon )->WatchdogThread ( listeners );
		}
		return 0;
	}


	void MediatorDaemon::WatchdogThread ( MediatorThreadInstance * threads )
	{
		CLog ( "Watchdog started..." );

		ThreadInstance		*	client				= 0;
		const unsigned int		checkDuration		= 1000 * 120; // 2 min. (in ms)
		const unsigned int		checkTimeout		= 1000 * 30; // 30 sec.
		const unsigned int		checkTimeoutMin		= 1000 * 2; // 2 sec.
		const unsigned int      maxTimeout			= checkDuration * 3;
		const unsigned int		logRollDurationMin	= 1000 * 60 * 20; // 20 min. (in ms)
		INTEROPTIMEVAL			timeLogRollLast		= 0;
		const unsigned int		projectValuesDurationMin	= 1000 * 60 * 60; // 1 h. (in ms)
        INTEROPTIMEVAL			timeProjectValues	= 0;
        INTEROPTIMEVAL			timeAcceptCheckLast	= 0;
        const unsigned int		acceptCheckDurationMin	= 1000 * 6; // 2 sec. (in ms)

#if defined(_WIN32) && !defined(USE_PTHREADS_FOR_WINDOWS)
		unsigned int			timeout				= checkTimeout;
#else
		struct timeval          now;
		struct timespec         timeout;
#endif
		// Send started signal
		if ( !threads->watchdog.Notify ( "Acceptor" ) ) {
			CErr ( "Watchdog: Error to signal event" );
		}

		vsp ( ThreadInstance ) toDeleteClients;
        vsp ( ThreadInstance ) sessionClients;
        vsp ( ThreadInstance ) tmpClients;
        vsp ( StuntRegisterContext ) toDisposeSPs;

		while ( isRunning )
		{
			CVerbVerb ( "Watchdog: checking..." );

            bool reDo = false;

			INTEROPTIMEVAL lastCheckTime = checkLast;

			checkLast = GetEnvironsTickCount ();

			vsp ( ThreadInstance ) toDeleteTmp;
			vsp ( ThreadInstance ) toRemoveClients;
            
            if ( ___sync_val_compare_and_swap ( &watchdogSessionsChanged, 1, 0 ) == 1 )
            {
                sessionClients.clear ();
                
                if ( !sessions.Lock1 ( "Watchdog" ) ) break;
                
                sessionClients = sessions.cache;
                
                if ( sessions.toRemove.size () > 0 )
                {
                    toRemoveClients = sessions.toRemove; sessions.toRemove.clear ();
                }
                
                if ( !sessions.Unlock1 ( "Watchdog" ) ) break;
            }

			vsp ( ThreadInstance )::iterator it;

			// Remove the closed clients
            size_t i;
            size_t size = toRemoveClients.size ();
			if ( size > 0 )
			{
                i = 0;
				while ( i < size )
				{
                    const sp ( ThreadInstance ) & constSP  = toRemoveClients [ i ]; ++i;
                    
					ClientRemove ( constSP.get () );

					if ( constSP.use_count () > 1 )
						toDeleteTmp.push_back ( constSP );
				}

				toRemoveClients.clear ();
			}
            
			// Look which one we ca delete directly and which one needs to be deferred
			it = toDeleteClients.begin ();
			
			while ( it != toDeleteClients.end () )
            {
                const sp ( ThreadInstance ) & constSP = *it;
                
				if ( constSP.use_count () <= 1 ) {
					it = toDeleteClients.erase ( it );
				}
				else ++it;
			}


            size = toDeleteTmp.size ();
			if ( size > 0 )
            {
                i = 0;
				while ( i < size )
                {
                    const sp ( ThreadInstance ) & constSP  = toDeleteTmp [ i ]; ++i;
                    
					if ( constSP.use_count () > 1 ) {
						// Check whether the client has already been added to the container (otherwise we risk a memory leak)

						size_t sized	= toDeleteClients.size ();
						size_t j		= 0;

						ThreadInstance * toDelete = constSP.get ();

						while ( j < sized ) {
							const sp ( ThreadInstance ) & constSP1  = toDeleteTmp [ j ]; ++j;

							if ( constSP1.get () == toDelete )
								break;
						}

						if ( j >= sized )
							toDeleteClients.push_back ( constSP );
					}
				}
				toDeleteTmp.clear ();
			}


			INTEROPTIMEVAL compensate = ( checkLast - lastCheckTime );

			if ( compensate > checkDuration )
				compensate -= checkDuration;
			else
				compensate = 0;

			i = 0;
			size = sessionClients.size ();

			while ( i < size )
			{
				const sp ( ThreadInstance ) & constSP = sessionClients [ i ]; ++i;

				if ( constSP ) {
					client = constSP.get ();

					CVerbArg ( "Watchdog: Checking deviceID [ 0x%X : %s ] socket [ %i ]", client->deviceID, client->ips, client->socket );

					if ( client->sendFails > 20 || ( checkLast - client->aliveLast ) > ( maxTimeout + compensate ) )
					{
						if ( IsValidFD ( client->socket ) )
						{
							CLogArg ( "Watchdog: Disconnecting [ 0x%X : %s ]  socket [ %i ] due to expired heartbeat or failed sends [ %i ] ...", client->deviceID, client->ips, client->socket, client->sendFails );

							reDo = true;

							client->SendTcpFin ();
						}
						else {
#ifdef USE_VERIFYSOCKETS
							VerifySockets ( client, false );
#endif
						}

						client->aliveLast = 0;
					}

					if ( IsValidFD ( client->socket ) )
					{
						msp ( string, StuntRegisterContext ) & sockets = client->stuntSocketsLog;

						if ( LockAcquireA ( client->stuntSocketLock, "Watchdog" ) )
						{
							unsigned int now = GetEnvironsTickCount32 ();

							msp ( string, StuntRegisterContext )::iterator itm = sockets.begin ();

							while ( itm != sockets.end () )
							{
								const sp ( StuntRegisterContext ) &ctx = itm->second;

								if ( now - ctx->registerTime > 30000 )
								{
									toDisposeSPs.push_back ( ctx );

									sockets.erase ( itm++ );
								}
								else ++itm;
							}

							LockReleaseA ( client->stuntSocketLock, "Watchdog" );

							toDisposeSPs.clear ();
						}
					}
				}
			}


			//
			// Check acceptClients
            //
            if ( ( checkLast - timeAcceptCheckLast ) > acceptCheckDurationMin )
            {
                i = 0;
                msp ( void *, ThreadInstance )              &list   = acceptClients.list;
                
                if ( !acceptClients.Lock ( "Watchdog" ) ) break;
                
                timeAcceptCheckLast = checkLast;
                
                msp ( void *, ThreadInstance )::iterator    itm     = list.begin ();
                msp ( void *, ThreadInstance )::iterator    end     = list.end ();
                
                while ( itm != end )
                {
                    i++; const sp ( ThreadInstance ) &constSP = itm->second;
                    
                    client = constSP.get ();
                    if ( client )
                    {
                        INTEROPTIMEVAL diff = ( checkLast - client->connectTime );
                        
                        if ( client->sendFails > 20 || ( checkLast > client->connectTime && diff > checkDuration ) )
                        {
                            tmpClients.push_back ( constSP );
                            
                            client->inAcceptorList = false;
                            list.erase ( itm++ );
                            end = list.end ();
                        }
                        else ++itm;
                    }
                    else {
                        CErrArg ( "Watchdog: **** Invalid Client [ %i ]", i );
                        
                        list.erase ( itm++ );
                        end = list.end ();
                    }
                }
                
                if ( !acceptClients.Unlock ( "Watchdog" ) ) break;
                
                size = tmpClients.size ();
                if ( size > 0 )
                {
                    i = 0;
                    while ( i < size )
                    {
                        const sp ( ThreadInstance ) &constSP = tmpClients [ i ]; ++i;
                        if ( constSP )
                        {
                            ReleaseClient ( constSP.get () );
                            
                            reDo = true;
                        }
                        else {
                            CErrArg ( "Watchdog: **** Invalid Client [%i]", i );
                        }
                    }
                    
                    tmpClients.clear ();
                }
            }
            
            if ( reDo )
                continue;

			if ( !LockAcquireA ( thread_lock, "Watchdog" ) ) break;

#if !defined(_WIN32) || defined(USE_PTHREADS_FOR_WINDOWS)
			gettimeofday ( &now, NULL );

			timeout.tv_sec = now.tv_sec + ( checkTimeout / 1000 );
			timeout.tv_nsec = now.tv_usec * 1000;
#endif
			if ( ( checkLast - timeLogRollLast ) > logRollDurationMin ) {
				timeLogRollLast = checkLast;
				CloseLog ();
				OpenLog ();
			}

			if ( ( checkLast - timeProjectValues ) > projectValuesDurationMin ) {
				timeProjectValues = checkLast;
				CheckProjectValues ();
			}

			do
			{
				pthread_cond_timedwait ( &hWatchdogEvent, &thread_lock, &timeout );

				if ( !isRunning )
					break;

				INTEROPTIMEVAL now = GetEnvironsTickCount ();

				if ( ( now - checkLast ) > checkTimeoutMin )
					break;
			}
			while ( isRunning );


			if ( !LockReleaseA ( thread_lock, "Watchdog" ) )
				break;

			if ( usersDBDirty ) {
				usersDBDirty = false;

				SaveUserDB ();
			}

			if ( configDirty ) {
				configDirty = false;

				SaveConfig ();
			}

			if ( deviceMappingDirty ) {
				deviceMappingDirty = false;

				SaveDeviceMappings ();
			}
		}

		CLog ( "Watchdog: bye bye..." );
	}


#ifdef WIN32
	std::wstring * s2ws ( const std::string& s )
	{
		int slength = ( int ) s.length () + 1;

		int len = MultiByteToWideChar ( CP_ACP, 0, s.c_str (), slength, 0, 0 );
		wchar_t* buf = new wchar_t [ len ];
		MultiByteToWideChar ( CP_ACP, 0, s.c_str (), slength, buf, len );
		wstring * wbuf = new wstring ( buf );
		delete [ ] buf;

		return wbuf;
	}

	bool MediatorDaemon::HTTPPostRequest ( string domain, string path, string key, string jsonData )
	{
		BOOL ret = false;

		DWORD dwSize = 0;
		DWORD dwDownloaded = 0;
		LPSTR responseBuffer = 0;
		wstring * wDomain = 0;
		wstring * wPath = 0;
		wstring * wsAK = 0;

		HINTERNET  httpOpen = NULL,
			hConnect = NULL,
			hRequest = NULL;

		httpOpen = WinHttpOpen ( L"Mozilla/4.0 (compatible; MSIE 5.22)", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0 );

		while ( httpOpen ) {
			wDomain = s2ws ( domain );
			hConnect = WinHttpConnect ( httpOpen, wDomain->c_str (), INTERNET_DEFAULT_HTTPS_PORT, 0 );
			if ( !hConnect )
				break;

			wPath = s2ws ( path );
			hRequest = WinHttpOpenRequest ( hConnect, L"POST", wPath->c_str (), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE );
			if ( !hRequest )
				break;

			std::stringstream ssAK;
			ssAK << "Authorization: key=" << key;
			wsAK = s2ws ( ssAK.str () );

			ret = WinHttpAddRequestHeaders ( hRequest, wsAK->c_str (), ( ULONG ) wsAK->size (), WINHTTP_ADDREQ_FLAG_ADD );
			if ( !ret )
				break;

			ret = WinHttpAddRequestHeaders ( hRequest, L"Content-Type: application/json", ( ULONG ) -1L, WINHTTP_ADDREQ_FLAG_ADD );
			if ( !ret )
				break;

			ret = WinHttpSendRequest ( hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, ( LPVOID ) jsonData.c_str (), ( DWORD ) jsonData.size (), ( DWORD ) jsonData.size (), 0 );
			if ( !ret )
				break;

			ret = WinHttpReceiveResponse ( hRequest, NULL );
			if ( !ret )
				break;

			ret = false;

			do
			{
				// Check for available data.
				dwSize = 0;
				if ( !WinHttpQueryDataAvailable ( hRequest, &dwSize ) ) {
					CErrArg ( "HTTPPostRequest: Error %u while query available response data.", GetLastError () );
					break;
				}

				// Allocate space for the buffer.
				responseBuffer = new char [ dwSize + 1 ];
				if ( !responseBuffer )
				{
					CErrArg ( "HTTPPostRequest: Failed to allocate %i bytes for response.", dwSize + 1 );
					dwSize = 0;
				}
				else
				{
					// Read the data.
					ZeroMemory ( responseBuffer, dwSize + 1 );

					if ( !WinHttpReadData ( hRequest, ( LPVOID ) responseBuffer, dwSize, &dwDownloaded ) )
					{
						CErrArg ( "HTTPPostRequest: Failed to read response data with error %u.", GetLastError () );
					}
					else {
						CLogArg ( "HTTPPostRequest: Response - %s", responseBuffer );
						if ( strstr ( responseBuffer, "\"success\":1" ) ) {
							ret = true;
						}
					}

					delete [ ] responseBuffer;
				}
			}
			while ( dwSize > 0 );

			break;
		}
		if ( wDomain )
			delete ( wDomain );
		if ( wsAK )
			delete ( wsAK );

		if ( ret )
			printf ( "HTTPPostRequest: success.\n" );
		else
			printf ( "HTTPPostRequest: Error %u\n", GetLastError () );

		if ( hRequest )
			WinHttpCloseHandle ( hRequest );
		if ( hConnect )
			WinHttpCloseHandle ( hConnect );
		if ( httpOpen )
			WinHttpCloseHandle ( httpOpen );


		return ret ? true : false;
	}

#else

	size_t write_to_string ( void *ptr, size_t size, size_t count, void *stream )
	{
		( ( string* ) stream )->append ( ( char* ) ptr, 0, size*count );
		return size*count;
	}

	bool MediatorDaemon::HTTPPostRequest ( string domain, string path, string key, string jsonData )
	{
		bool ret = false;

		CURL * curl;
		CURLcode res;
		struct curl_slist * headers = NULL;

		curl_global_init ( CURL_GLOBAL_DEFAULT );

		curl = curl_easy_init ();

		while ( curl ) {

			std::stringstream ssUrl;
			ssUrl << "https://" << domain << "/" << path;

			curl_easy_setopt ( curl, CURLOPT_URL, ssUrl.str ().c_str () );
			CVerbArg ( "HTTPPostRequest: curl opt url: %s", ssUrl.str ().c_str () );

			curl_easy_setopt ( curl, CURLOPT_POST, 1L );

			std::stringstream ssKey;
			ssKey << "Authorization: key=" << key;
			headers = curl_slist_append ( headers, ssKey.str ().c_str () );
			headers = curl_slist_append ( headers, "Content-Type: application/json" );

			//res =
			curl_easy_setopt ( curl, CURLOPT_HTTPHEADER, headers );

			CVerbArg ( "HTTPPostRequest: curl post data: %s", jsonData.c_str () );
			curl_easy_setopt ( curl, CURLOPT_POSTFIELDS, jsonData.c_str () );
			curl_easy_setopt ( curl, CURLOPT_POSTFIELDSIZE, ( curl_off_t ) jsonData.size () );

			string response;
			curl_easy_setopt ( curl, CURLOPT_WRITEFUNCTION, write_to_string );
			curl_easy_setopt ( curl, CURLOPT_WRITEDATA, &response );

			res = curl_easy_perform ( curl );
			if ( res != CURLE_OK ) {
				CErrArg ( "HTTPPostRequest: curl_easy_perform() failed: %s", curl_easy_strerror ( res ) );
			}
			else {
				CVerbArg ( "HTTPPostRequest: curl reponse:%s", response.c_str () );

				if ( strstr ( response.c_str (), "\"success\":1" ) ) {
					ret = true;
				}
			}

			break;
		}

		if ( headers )
			curl_slist_free_all ( headers );

		if ( curl )
			curl_easy_cleanup ( curl );

		curl_global_cleanup ();

		return ret;
	}
#endif


	// Logging to file

	size_t GetTimeString ( char * timeBuffer, unsigned int bufferSize )
	{
		time_t now;
		struct tm timeInfo;

		time ( &now );
#ifdef WIN32
		if ( localtime_s ( &timeInfo, &now ) ) {
#else
		if ( !localtime_s ( &timeInfo, &now ) ) {
#endif
			timeBuffer [ 0 ] = 'e';
			timeBuffer [ 1 ] = 0;
		}

		return strftime ( timeBuffer, bufferSize, "%a %b %d %H:%M:%S: ", &timeInfo );
	}


	void MLogArg ( const char * format, ... )
    {
        if ( !stdLog && !logToFile )
            return;
            
		char timeString [ 256 ];
		GetTimeString ( timeString, sizeof ( timeString ) );

		char logBuffer [ 2048 ];
		va_list argList;
		va_start ( argList, format );
#ifdef WIN32
		vsprintf_s ( logBuffer, ( sizeof ( logBuffer ) - 2 ), format, argList );
#else
		vsnprintf ( logBuffer, ( sizeof ( logBuffer ) - 2 ), format, argList );
		//vsprintf ( logBuffer, format, argList );
		/*printf ( "%s", timeString );
		printf ( "%s", logBuffer );*/
#endif
		va_end ( argList );

		if ( stdLog ) {
			OutputDebugStringA ( timeString );
			OutputDebugStringA ( logBuffer );
		}

		if ( !logToFile )
			return;

		stringstream line;

		// Save 
		line << logBuffer;

		if ( pthread_mutex_lock ( &logMutex ) ) {
			OutputDebugStringA ( "MLog: Failed to aquire lock!\n" );
			return;
		}

		logfile << timeString << line.str () << std::flush; // << endl;
		logfile.flush ();

		if ( pthread_mutex_unlock ( &logMutex ) ) {
			OutputDebugStringA ( "MLog: Failed to release lock on logfile!\n" );
		}

		line.clear ();
	}

	void MLog ( const char * msg )
	{
        if ( !stdLog && !logToFile )
            return;
        
		char timeString [ 256 ];
		GetTimeString ( timeString, sizeof ( timeString ) );

		if ( stdLog ) {
#ifdef WIN32
			OutputDebugStringA ( timeString );
			OutputDebugStringA ( msg );
#else
			printf ( "%s", timeString );
			printf ( "%s", msg );
#endif
		}

		if ( !logToFile )
			return;
		stringstream line;

		// Save 
		line << msg;

		if ( pthread_mutex_lock ( &logMutex ) ) {
			OutputDebugStringA ( "MLog: Failed to aquire mutex!\n" );
			return;
		}

		logfile << timeString << line.str (); // << std::flush; // << endl;
											  //logfile.flush ();

		if ( pthread_mutex_unlock ( &logMutex ) ) {
			OutputDebugStringA ( "MLog: Failed to release mutex!\n" );
		}
	}


	int logStatusBefore = 0;

	const int maxLogs = 10;

	bool MediatorDaemon::OpenLog ()
	{
		CVerb ( "OpenLog" );

		if ( !fileLog )
			return false;

		if ( pthread_mutex_lock ( &logMutex ) ) {
			CErr ( "OpenLog: Failed to aquire mutex on logfile!" );
			return false;
		}

		if ( logfile.is_open () ) {
			CWarn ( "OpenLog: logile is already opened!" );
			goto Finish;
		}

		if ( GetSizeOfFile ( LOGFILE ) >= 300 * 1024 * 1024 )
		{
			// Find next available
			int i = 0;
			bool modify = false;
			do {
				stringstream tmp;
				tmp << LOGFILE << "." << i;

				ifstream tmpFile ( tmp.str () );
				if ( !tmpFile.good () ) {
					modify = true;
				}
				tmpFile.close ();

				if ( modify ) {
					std::rename ( LOGFILE, tmp.str ().c_str () );
				}
				i++;
			}
			while ( !modify );

			int toDel = i + 1;

			if ( toDel > maxLogs )
				toDel = 0;

			// Delete number 0
			stringstream tmp1;
			tmp1 << LOGFILE << "." << toDel;

			ifstream delFile ( tmp1.str () );
			if ( delFile.good () ) {
				modify = true;
			}
			delFile.close ();

			if ( modify )
				std::remove ( tmp1.str ().c_str () );
		}

		logfile.open ( LOGFILE, ios_base::app );
		if ( !logfile.good () ) {
			CErr ( "OpenLog: Failed to open logfile!" );
			goto Finish;
		}

		if ( logStatusBefore >= 0 )
			logToFile = true;

	Finish:
		if ( pthread_mutex_unlock ( &logMutex ) ) {
			CErr ( "CloseLog: Failed to release mutex on logfile!" );
			return false;
		}

		return true;
	}


	bool MediatorDaemon::CloseLog ()
	{
		CLog ( "CloseLog" );

		if ( pthread_mutex_lock ( &logMutex ) ) {
			OutputDebugStringA ( "CloseLog: Failed to aquire mutex on logfile!\n" );
			return false;
		}

		if ( !logfile.is_open () ) {
			OutputDebugStringA ( "CloseLog: logfile is not openend!\n" );
			goto Finish;
		}

		OutputDebugStringA ( "CloseLog: Closing log file ...\n" );

		if ( logToFile == true )
			logStatusBefore = 1;
		else
			logStatusBefore = -1;
		logToFile = false;

		logfile.close ();

	Finish:
		if ( pthread_mutex_unlock ( &logMutex ) ) {
			OutputDebugStringA ( "CloseLog: Failed to release mutex on logfile!\n" );
			return false;
		}

		return true;
	}

}

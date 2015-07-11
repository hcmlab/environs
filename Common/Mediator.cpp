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
//#define DEBUGVERB
//#define DEBUGVERBVerb
#define DEBUGVERBList
#endif

#include "Mediator.h"
#include "Environs.Native.h"

#if !defined(MEDIATORDAEMON)
#include "Environs.h"
#else
#include <cstdio>
#endif

#include <errno.h>
#include <fcntl.h>

#ifndef _WIN32
#include <signal.h>
#endif
#include <map>
#include <string>

#define	CLASS_NAME 	"Mediator"


/* Namespace: environs -> */
namespace environs
{
    
    bool                Mediator::allocatedClass = false;
    NetPack             Mediator::localNets;
    pthread_mutex_t     Mediator::localNetsMutex;
    
    
#ifdef USE_INTEGER_PROJECT_APP_MAPS
    std::map<int, std::string>      projectIDToNames;
    std::map<int, std::string>      appIDToNames;
    
    std::map<std::string, int>      nameToProjectID;
    std::map<std::string, int>      nameToAppID;
#endif

Mediator::Mediator ( )
{
	CVerb ( "Construct" );

	Zero ( localNets );
	Zero ( mediator );
    
    allocated               = false;
	
	isRunning 				= false;
	broadcastSocketID 		= -1;

	Zero ( broadcastThreadID );
	
	broadcastMessageLen		= 0;
	greetUpdates			= 0;

	certificate				= 0;
}


Mediator::~Mediator ( )
{
	CVerb ( "Destructor" );

	Dispose ();

    if ( allocated )
    {
        if ( pthread_mutex_destroy	( &devicesMutex ) ) {
            CVerb ( "Init: Failed to destroy devicesMutex!" );
        }
        if ( pthread_mutex_destroy	( &mediatorMutex ) ) {
            CVerb ( "Init: Failed to destroy mediatorMutex!" );
        }
#ifdef USE_INTEGER_PROJECT_APP_MAPS
        if ( pthread_mutex_destroy	( &idMapMutex ) ) {
            CVerb ( "Init: Failed to destroy idMapMutex!" );
        }
#endif
        projectIDToNames.clear();
        appIDToNames.clear();
        nameToProjectID.clear();
        nameToAppID.clear();
    }
}


bool Mediator::Init ()
{
	CVerb ( "Init" );

    if ( !InitClass () )
        return false;
    
    if ( !allocated )
	{
		Zero ( mediatorMutex );
        if ( pthread_mutex_init	( &mediatorMutex, NULL ) ) {
            CErr ( "Init: Failed to init mediatorMutex!" );
            return false;
        }

		Zero ( devicesMutex );
        if ( pthread_mutex_init	( &devicesMutex, NULL ) ) {
            CErr ( "Init: Failed to init mediatorMutex!" );
            return false;
        }
        
#ifdef USE_INTEGER_PROJECT_APP_MAPS
        Zero ( idMapMutex );
        
        projectIDToNames.clear();
        appIDToNames.clear();
        nameToProjectID.clear();
        nameToAppID.clear();
        
        if ( pthread_mutex_init	( &idMapMutex, NULL ) ) {
            CErr ( "Init: Failed to init idMapMutex!" );
            return false;
        }
#endif
        allocated = true;
    }
    
	if ( !LoadNetworks () ) {
		CErr ( "Init: Failed to load local ip addresses!" );
        return false;
    }
	return true;
}


bool Mediator::InitClass ()
{
	CVerb ( "InitClass" );

	if ( allocatedClass )
		return true;

	Zero ( localNets );

	Zero ( localNetsMutex );

	if ( pthread_mutex_init ( &localNetsMutex, 0 ) ) {
		CErr ( "InitClass: Failed to init localNetsMutex!" );
		return false;
	}

	allocatedClass = true;

	return true;
}


void Mediator::DisposeClass ()
{
	CVerb ( "DisposeClass" );

	if ( !allocatedClass )
		return;

	ReleaseNetworks ();

	if ( pthread_mutex_destroy ( &localNetsMutex ) ) {
		CErr ( "DisposeClass: Failed to init localNetsMutex!" );
	}
}


void Mediator::BroadcastByeBye ()
{
	strcpy_s ( broadcastMessage + 4, sizeof ( MEDIATOR_BROADCAST_BYEBYE ), MEDIATOR_BROADCAST_BYEBYE );
	SendBroadcast ();
}

    
void Mediator::Dispose ()
{	
	CVerb ( "Dispose" );

	isRunning = false;

	// Wait for each thread to terminate
	ReleaseThreads ();
	
	ReleaseNetworks ();
	ReleaseMediators ();

	if ( certificate ) 
		free ( certificate );
}


bool Mediator::IsSocketAlive ( int &sock )
{
	CVerb ( "IsSocketAlive" );

	if ( sock == -1 ) {
		CVerb ( "IsSocketAlive: socket argument is invalid" );
		return false;
	}

	int value;
	socklen_t size = sizeof(socklen_t);

	int ret = getsockopt ( sock, SOL_SOCKET, SO_REUSEADDR, (char *)&value, &size );
	if ( ret < 0 ) {
		CInfo ( "IsSocketAlive: disposing invalid socket!" );
		//LogSocketError ();
#ifdef MEDIATORDAEMON
        try {
            shutdown ( sock, 2 );
			closesocket ( sock );
		} catch (...) {
		}
#else
		shutdown ( sock, 2 );
		closesocket ( sock );
#endif
		sock = -1;
		return false;
	}

	CVerb ( "IsSocketAlive: socket is alive" );
	return true;
}


void Mediator::VerifySockets ( ThreadInstance * inst, bool waitThread )
{
	CVerb ( "VerifySockets" );

	if ( !IsSocketAlive ( inst->socket ) ) {
		// Close spare socket as well
		if ( inst->spareSocket != -1 ) {
			CVerb ( "VerifySockets: disposing spare socket!" );

			shutdown ( inst->spareSocket, 2 );
			closesocket ( inst->spareSocket );
			inst->spareSocket = -1;
		}
        
        if ( !waitThread )
            return;
        
        // Wait for the listening thread to be terminated
        pthread_t thrd = inst->threadID;
        
        if ( pthread_valid ( thrd ) ) {
            CLog ( "VerifySockets: waiting for mediator listener thread to be termianted." );
            
            /// EXC_BAD_ACCESS
            int s = pthread_join ( thrd, NULL );
            if ( pthread_wait_fail ( s ) ) {
                CErrArg ( "VerifySockets: Error waiting for mediator listener thread (pthread_join:%i)", s );
            }
            pthread_close ( thrd );
            pthread_reset ( inst->threadID );
        }
	}
}


bool Mediator::ReleaseThreads ()
{
	CVerb ( "ReleaseThreads" );

	int value;
	bool ret = true;

	// Signal stop status to threads
	isRunning = false;

	// Close broadcast listener socket
	value = broadcastSocketID;
	if ( value != -1 ) {
		CVerb ( "ReleaseThreads: Closing broadcast socket." );
		
		broadcastSocketID = -1;
		shutdown ( value, 2 );
		closesocket ( value );
	}

	// Waiting for broadcast thread
	pthread_t thrd = broadcastThreadID;

	if ( pthread_valid ( thrd ) ) {
		pthread_reset ( broadcastThreadID );

		CVerb ( "ReleaseThreads: Waiting for broadcast thread to be termianted." );
				
		value = pthread_join ( thrd, NULL );
		if ( pthread_wait_fail ( value ) ) {
			CErrArg ( "ReleaseThreads: Error waiting for broadcast thread (pthread_join:%i)\n", value );
			ret = false;
		}

		pthread_close ( thrd );
	}

	return ret;
}


void Mediator::ReleaseNetworks ()
{
	CVerb ( "ReleaseNetworks" );
		
	if ( pthread_mutex_lock ( &localNetsMutex ) ) {
		CErr ( "ReleaseNetworks: Failed to aquire mutex on localNets!" );
		return;
	}

	NetPack * net = localNets.next;

	while ( net ) {
		NetPack * toDelete = net;
		net = net->next;
		free ( toDelete );
	}

	Zero ( localNets );
			
	if ( pthread_mutex_unlock ( &localNetsMutex ) ) {
		CErr ( "ReleaseNetworks: Failed to release mutex on localNets!" );
	}
}


void Mediator::ReleaseMediator ( MediatorInstance * med ) 
{
	ThreadInstance * inst = &med->connection.instance;

	int s = inst->socket;
	if ( s != -1 ) {
		inst->socket = -1;

		CVerb ( "ReleaseMediator: closing cocket" );
		shutdown ( s, 2 );
		closesocket ( s );
	}

	// Wait for the listening thread to be terminated
	pthread_t thrd = inst->threadID;
	if ( pthread_valid ( thrd ) ) {
		pthread_reset ( inst->threadID );

		CVerb ( "ReleaseMediator: waiting for mediator listener thread to be termianted." );

        /// EXC_BAD_ACCESS
		s = pthread_join ( thrd, NULL );
		if ( pthread_wait_fail ( s ) ) {
			CErrArg ( "ReleaseMediator: Error waiting for mediator listener thread (pthread_join:%i)", s );
		}
		pthread_close ( thrd );
	}


	AESDisposeKeyContext ( &med->connection.instance.aes );
	med->connection.instance.encrypt = 0;

	if ( pthread_mutex_destroy ( &med->connection.rec_mutex ) ) {
		CErr ( "ReleaseNetworks: Failed to destroy rec_mutex." );
	}
	if ( pthread_mutex_destroy ( &med->connection.send_mutex ) ) {
		CErr ( "ReleaseNetworks: Failed to destroy send_mutex." );
	}
	if ( pthread_cond_destroy ( &med->connection.rec_signal ) ) {
		CErr ( "ReleaseMediator: Failed to destroy signal." );
	}

	if ( med->connection.buffer )
		free ( med->connection.buffer );

	if ( med != &mediator )
		delete med;
}


void Mediator::ReleaseMediators ()
{
	CVerb ( "ReleaseMediators" );

	if ( pthread_mutex_lock ( &mediatorMutex ) ) {
		CErr ( "ReleaseMediators: Failed to aquire mutex on mediator!" );
	}

	MediatorInstance * inst = mediator.next;

	while ( inst ) {
		MediatorInstance * toDelete = inst;
		inst = inst->next;

		ReleaseMediator ( toDelete );
	}

	if ( mediator.port ) {
		ReleaseMediator ( &mediator );
		Zero ( mediator );
	}
	
	if ( pthread_mutex_unlock ( &mediatorMutex ) ) {
		CErr ( "ReleaseMediators: Failed to release mutex on mediator!" );
	}
}


bool Mediator::LoadNetworks ( )
{
	CVerb ( "LoadNetworks" );

	ReleaseNetworks ();
	
	NetPack * pack = &localNets;

	unsigned int	ip;
	unsigned int	bcast;
	
	if ( pthread_mutex_lock ( &localNetsMutex ) ) {
		CErr ( "LoadNetworks: Failed to aquire mutex on localNets!" );
	}

#ifdef _WIN32
	char ac [ 256 ];
    if ( gethostname ( ac, sizeof(ac) ) == SOCKET_ERROR ) {
        CErrArg ( "LoadNetworks: Error %i when getting local host name.", WSAGetLastError () );
        goto EndOfMethod;
    }	
	CLogArg ( "LoadNetworks: Host name [%s]", ac );

	SOCKET sock = WSASocket ( AF_INET, SOCK_DGRAM, 0, 0, 0, 0 );
    if ( sock == SOCKET_ERROR ) {
        CErrArg ( "LoadNetworks: Failed to get a socket. Error %i", WSAGetLastError () );
        goto EndOfMethod;
    }

    INTERFACE_INFO InterfaceList [ 20 ];
    unsigned long nBytesReturned;

    if ( WSAIoctl ( sock, SIO_GET_INTERFACE_LIST, 0, 0, &InterfaceList,
			sizeof(InterfaceList), &nBytesReturned, 0, 0) == SOCKET_ERROR )
	{
        CErrArg ( "LoadNetworks: Failed calling WSAIoctl: error %i", WSAGetLastError() );
		shutdown ( sock, 2 );
		closesocket ( sock );
        goto EndOfMethod;
    }

    int nNumInterfaces = nBytesReturned / sizeof(INTERFACE_INFO);
	CVerbArg ( "LoadNetworks: There are [%i] interfaces:", nNumInterfaces );

	// Skip loopback interfaces
	for ( int i = 0; i < nNumInterfaces; ++i ) {
        u_long nFlags = InterfaceList[i].iiFlags;
		if ( nFlags & IFF_LOOPBACK ) {
			CVerb ( "LoadNetworks: Omiting loopback interface." );
			continue;
		}

        sockaddr_in * pAddress;
		//int max_length = 20;
		
		// Retrieve ip address
        pAddress = (sockaddr_in *) & (InterfaceList[i].iiAddress);	

		ip = pAddress->sin_addr.s_addr;

		// Retrieve netmask	
        pAddress = (sockaddr_in *) & (InterfaceList[i].iiNetmask);	
		unsigned int mask = pAddress->sin_addr.s_addr;

        CLogArg ( "Local IP %i: %s", i, inet_ntoa ( *((struct in_addr *) &ip) ) );
        CVerbArg ( "Local SN %i: %s", i, inet_ntoa ( *((struct in_addr *) &mask) ) );

		bcast = GetBroadcast ( ip, mask );
        if ( !bcast ) {
            CErrArg ( "LoadNetworks: Failed to calculate broadcast address for interface [%i]!", i );
            continue;
        }
            
        CVerbArg ( "LoadNetworks: Local BC %i: [%s]", i, inet_ntoa ( *((struct in_addr *) &bcast) ) );
        
		AddNetwork ( pack, ip, bcast, mask );
    }

	shutdown ( sock, 2 );
	closesocket ( sock );

#else

#ifdef ANDROID
	int sock, rval;
	struct ifreq ifreqs[20];
	struct ifreq ifr_mask;
	struct ifconf ifconf;
	int  nifaces, i;

	Zero ( ifconf );
	ifconf.ifc_buf = (char *) ifreqs;
	ifconf.ifc_len = sizeof(ifreqs);

	sock = socket ( AF_INET, SOCK_STREAM, 0 );
	if ( sock < 0 ) {
		CErr ( "LoadNetworks: Failed to create socket for ioctl!" );
        goto EndOfMethod;
	}

	rval = ioctl ( sock, SIOCGIFCONF , (char*) &ifconf );
	if ( rval < 0 ) {
		CErr ( "LoadNetworks: ioctl SIOCGIFCONF failed!" );
		shutdown ( sock, 2 );
		closesocket ( sock );
        goto EndOfMethod;
	}

	nifaces =  ifconf.ifc_len/sizeof(struct ifreq);

	CVerbArg ( "LoadNetworks: Interfaces (count = %d)", nifaces );
		
	for ( i = 0; i < nifaces; i++ )
	{
		ip = ((struct sockaddr_in *)&ifreqs[i].ifr_ifru.ifru_addr)->sin_addr.s_addr;

		CLogArg ( "LoadNetworks: Interface name: '%s' - ip: '%s'", ifreqs[i].ifr_name, inet_ntoa ( *((struct in_addr *) &ip) ) );

        if ( strlen ( ifreqs[i].ifr_name ) < 1 ) {
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
		
		// Retrieve netmask	
		ifr_mask.ifr_addr.sa_family = AF_INET;

		strncpy ( ifr_mask.ifr_name, ifreqs[i].ifr_name, IFNAMSIZ-1 );

		rval = ioctl ( sock, SIOCGIFNETMASK, &ifr_mask );
		if ( rval < 0 ) {
			CErr ( "LoadNetworks: ioctl SIOCGIFNETMASK failed!" );
			continue;
		}

		unsigned int mask = ((struct sockaddr_in *)&ifr_mask.ifr_addr)->sin_addr.s_addr;
		
		bcast = GetBroadcast ( ip, mask );
        if ( !bcast ) {
            CErr ( "LoadNetworks: Failed to calculate broadcast address!" );
            continue;
        }
        CVerbArg ( "LoadNetworks: Netmask: '%s'", inet_ntoa ( *((struct in_addr *) &mask) ) );
        CVerbArg ( "LoadNetworks: Broadcast: '%s'", inet_ntoa ( *((struct in_addr *) &bcast) ) );
        
		AddNetwork ( pack, ip, bcast, mask );
	}
	shutdown ( sock, 2 );
	closesocket ( sock );
    
#else // LINUX or MAC or IOS

	struct ifaddrs  * ifList  = NULL;
	struct ifaddrs  * ifa           = NULL;
	void            * tmp    = NULL;

	if ( getifaddrs ( &ifList ) != 0 ) {
		CErr ( "LoadNetworks: getifaddrs failed!" );
        goto EndOfMethod;    
    }
    
	for ( ifa = ifList; ifa != NULL; ifa = ifa->ifa_next )
    {
	    if ( ifa ->ifa_addr->sa_family == AF_INET ) {
	        // IPv4
            ip = ( (struct sockaddr_in *) ifa->ifa_addr )->sin_addr.s_addr;
            
            CLogArg ( "LoadNetworks: [%s] => IP: [%s]", ifa->ifa_name, inet_ntoa ( *((struct in_addr *) &ip) )  );
            
            if ( !ip ) {
                CVerb ( "LoadNetworks: Omiting invalid interface." );
                continue;
            }
            
            // Skip loopback
            if ( ip == 0x0100007F ) {
                CVerb ( "LoadNetworks: Omiting loopback interface." );
                continue;
            }
            
#ifdef XCODE
            // Skip any connection but ethernet en...
            if ( !strstr ( ifa->ifa_name, "en") ) {
                CVerbArg ( "LoadNetworks: Not an ethernet interface. Ommiting [%s]", ifa->ifa_name ? ifa->ifa_name : "---" );
                continue;
            }
#endif
            unsigned int mask = ((struct sockaddr_in *)ifa->ifa_netmask)->sin_addr.s_addr;
            
            bcast = GetBroadcast ( ip, mask );
            if ( !bcast ) {
                CErr ( "LoadNetworks: ERROR - Failed to calculate broadcast address!" );
                continue;
            }
            CVerbArg ( "LoadNetworks: Netmask:   [%s]", inet_ntoa ( *((struct in_addr *) &mask) ) );
            CVerbArg ( "LoadNetworks: Broadcast: [%s]", inet_ntoa ( *((struct in_addr *) &bcast) ) );
            
            AddNetwork ( pack, ip, bcast, mask );
	    }
        else if ( ifa->ifa_addr->sa_family == AF_INET6 ) {
	            // We do not support IPv6 yet
	            tmp=&((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
	            char addressBuffer[INET6_ADDRSTRLEN];
	            inet_ntop(AF_INET6, tmp, addressBuffer, INET6_ADDRSTRLEN);
	            CVerbArg ( "LoadNetworks: IPv6 not supported yet: [%s] IP Address [%s]", ifa->ifa_name, addressBuffer);
	        }
    }
    
	if ( ifList )
        freeifaddrs ( ifList );
#endif
    
#endif

EndOfMethod:
	if ( pthread_mutex_unlock ( &localNetsMutex ) ) {
		CErr ( "LoadNetworks: Failed to release mutex on localNets!" );
	}
	return true;
}


unsigned int Mediator::GetBroadcast ( unsigned int ip, unsigned int netmask )
{
    u_long host_ip = ip;
    u_long net_mask = netmask;
    u_long net_addr = host_ip & net_mask;
    u_long bcast_addr = net_addr | (~net_mask);
    
    return (unsigned int) bcast_addr;
}


void Mediator::AddNetwork ( NetPack * &pack, unsigned int ip, unsigned int bcast, unsigned int netmask )
{
    if ( pack->ip ) {
        // This NetPack is already filled, create and attach a new one
		NetPack * newPack = (NetPack *)malloc ( sizeof(NetPack) );
        if ( !newPack ) {
            CErr ( "AddNetwork: ERROR - failed to allocate memory to store ip/broadcast! Low memory problem!" );
            return;
        }
        pack->next = newPack;
        pack = newPack;
        memset ( pack, 0, sizeof(NetPack) );
    }
    
	pack->ip = ip;
	pack->bcast = bcast;
	pack->mask = netmask;
}


bool Mediator::IsStarted ()
{
	return isRunning;
}


bool Mediator::Start ()
{
	CVerb ( "Start" );

	isRunning = true;
	
	// Create socket for broadcast thread
	if ( broadcastSocketID == -1 ) {
		int sock = (int) socket ( PF_INET, SOCK_DGRAM, IPPROTO_UDP );
		if ( sock == -1 ) {
			CErr ( "Start: Failed to create broadcast socket!" );
			return false;
		}
		broadcastSocketID = sock;

		int value = 1;
		if ( setsockopt ( sock, SOL_SOCKET, SO_BROADCAST, (const char *) &value, sizeof ( value ) ) == -1 ) {
			CErr ( "Start: Failed to set broadcast option on socket!" );
			return false;
		}

		value = 1;
		if ( setsockopt ( sock, SOL_SOCKET, SO_REUSEADDR, (const char *) &value, sizeof ( value ) ) != 0 ) {
			CErr ( "Start: Failed to set reuseaddr option on socket!" );
			return false;
		}

#ifdef SO_REUSEPORT // Fix for bind error on iOS simulator due to crash of app, leaving the port still bound
		value = 1;
		if ( setsockopt ( sock, SOL_SOCKET, SO_REUSEPORT, (const char *) &value, sizeof (value)) != 0 ) {
			CErr ( "Start: Failed to set reuseaddr option on socket!" );
			return false;
		}
#endif

		/// Create broadcast thread
		int s = pthread_create ( &broadcastThreadID, 0, BroadcastThreadStarter, (void *)this );
		if ( s ) {
			CErrArg ( "Start: Creating thread for broadcast failed. (pthread_create [%i])", s );
			return false;
		}

		Sleep ( 300 );
	}
	
	OnStarted ( );

	SendBroadcast ( );
	
	CVerb ( "Start: success" );
	return true;
}


void Mediator::OnStarted ( )
{
}


bool Mediator::SendBroadcast ( )
{
	CVerb ( "SendBroadcast" );

    if ( /*!isRunning ||*/ broadcastSocketID == -1 )
        return false;

	bool ret = true;
	size_t sentBytes = 0;
	
	CVerbArg ( "SendBroadcast: Broadcasting message [%s] (%d)...", broadcastMessage + 4, broadcastMessageLen );

	struct 	sockaddr_in		broadcastAddr;
	Zero ( broadcastAddr );

	broadcastAddr.sin_family = AF_INET;
	broadcastAddr.sin_port = htons ( DEFAULT_BROADCAST_PORT );

	broadcastAddr.sin_addr.s_addr = inet_addr("255.255.255.255");
	
	sentBytes = sendto ( broadcastSocketID, broadcastMessage + 4, broadcastMessageLen, 0, (struct sockaddr *) &broadcastAddr, sizeof(struct sockaddr) );
	if ( sentBytes != broadcastMessageLen ) {
		CErr ( "SendBroadcast: Broadcast failed!" );
		ret = false;
	}
	
	/// We need to broadcast to each interface, because (at least with win32): the default 255.255.... broadcasts only on one (most likely the main or internet) network interface
	NetPack * net = 0;

	if ( pthread_mutex_lock ( &localNetsMutex ) ) {
		CErr ( "SendBroadcast: Failed to aquire mutex!" );
		ret = false;
		goto EndWithStatus;
	}

	net = &localNets;
	while ( net ) {
		broadcastAddr.sin_addr.s_addr = net->bcast;

		sentBytes = sendto ( broadcastSocketID, broadcastMessage + 4, broadcastMessageLen, 0, (struct sockaddr *) &broadcastAddr, sizeof(struct sockaddr) );
		if ( sentBytes != broadcastMessageLen ) {
			CErr ( "SendBroadcast: Broadcast failed!" );
			ret = false;
		}

		net = net->next;
	}
	
	if ( pthread_mutex_unlock ( &localNetsMutex ) ) {
		CErr ( "SendBroadcast: Failed to release mutex!" );
	}

EndWithStatus:

	return ret;
}


bool Mediator::IsLocalIP ( unsigned int ip )
{
	bool ret = false;

	CVerb ( "IsLocalIP" );
	
	if ( pthread_mutex_lock ( &localNetsMutex ) ) {
		CErr ( "IsLocalIP: Failed to aquire mutex!" );
		return false;
	}

	NetPack * net = &localNets;

	while ( net ) {
		if ( ip == net->ip ) {
			ret = true;
			break;
		}
		net = net->next;
	}
	
	if ( pthread_mutex_unlock ( &localNetsMutex ) ) {
		CErr ( "IsLocalIP: Failed to release mutex!" );
		ret = false;
	}

	return ret;
}


unsigned int Mediator::GetLocalIP ( )
{
	CVerb ( "GetLocalIP" );

    if ( !LoadNetworks() )
        return 0;
    
	return localNets.ip;
}


unsigned int Mediator::GetLocalSN ( )
{
	CVerb ( "GetLocalSN" );

	return localNets.mask;
}



DeviceInstanceList * Mediator::UpdateDevices ( unsigned int ip, char * msg, char ** uid, bool * created, char broadcastFound )
{
	CVerbVerb ( "UpdateDevices" );

	if ( !msg )
		return 0;

	int				value;
	//unsigned int	flags		= 0;
	bool			found		= false;
	bool			changed		= false;
	char		*	deviceName	= 0;
	char		*	projectName = 0;
	char		*	appName		= 0;
	char		*	userName	= 0;
	DeviceInstanceList * device		= 0;
	DeviceInstanceList * devicePrev	= 0;
	DeviceInstanceList ** listRoot	= 0;
	pthread_mutex_t * mutex		= 0;

	int *					pDevicesAvailable	= 0;

	ApplicationDevices *	appDevices			= 0;
    
	// Get the id at first (>0)	
	int * pIntBuffer = (int *) (msg + MEDIATOR_BROADCAST_DEVICEID_START);
	value = *pIntBuffer;
	if ( !value )
		return 0;
	
	// Get the projectname, appname, etc..
	char * context = NULL;
	char * psem = strtok_s ( msg + MEDIATOR_BROADCAST_DESC_START, ";", &context );
	if ( !psem )
		return 0;
	deviceName = psem;
	
	psem = strtok_s ( NULL, ";", &context );
	if ( !psem )
		return 0;
	projectName = psem;
	
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
	if ( value == environs.deviceID && !strncmp ( projectName, environs.projectName, sizeof(environs.projectName) )
		&& !strncmp ( appName, environs.appName, sizeof(environs.appName) ) )
		return 0;

	if ( opt_mediatorFilterLevel > 0 ) {
		if ( strncmp ( projectName, environs.projectName, sizeof(environs.projectName) ) )
			return 0;

		if ( opt_mediatorFilterLevel > 1 ) {
			if ( strncmp ( appName, environs.appName, sizeof(environs.appName) ) )
				return 0;
		}
	}
#endif

	listRoot = GetDeviceList ( projectName, appName, &mutex, &pDevicesAvailable, &appDevices );

#ifdef MEDIATORDAEMON
	if ( !listRoot ) {
		return 0;
	}
#endif
	
	if ( pthread_mutex_lock ( mutex ) ) {
		CErr ( "UpdateDevices: Failed to aquire mutex!" );
		goto Finish;
	}

	device = *listRoot;

	while ( device ) 
	{
		CVerbVerbArg ( "UpdateDevices: Comparing [0x%X / 0x%X] Project [%s / %s] App [%s / %s]", device->info.deviceID, value, device->info.projectName, projectName, device->info.appName, appName );

		if ( device->info.deviceID == value
#ifndef MEDIATORDAEMON
			&& !strncmp ( device->info.projectName, projectName, sizeof(device->info.projectName) ) && !strncmp ( device->info.appName, appName, sizeof(device->info.appName) )
#endif
		) {			
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
		if ( pthread_mutex_unlock ( mutex ) ) {
			CErr ( "UpdateDevices: Failed to release mutex!" );
		}
		return device;
	}
	if ( created )
		*created = true;
#endif

	if ( !found ) {
		// Create a new list item
		DeviceInstanceList * dev = (DeviceInstanceList *)calloc ( 1, sizeof(DeviceInstanceList) );
		if ( dev ) {
			dev->info.deviceID = value;
			(*pDevicesAvailable)++;

#ifdef MEDIATORDAEMON
			dev->root = appDevices;
#else
			dev->info.broadcastFound = broadcastFound;
			
            // Build the key
#ifdef USE_MEDIATOR_OPT_KEY_MAPS_COMP
            *((int *)dev->key) = value;
            
            int copied = sprintf_s ( dev->key + 4, sizeof ( dev->key ) - 4, "%s %s", projectName, appName );
#else
			int copied = sprintf_s ( dev->key, sizeof ( dev->key ), "%011i %s %s", value, projectName, appName );
#endif
			if ( copied <= 0 ) {
				CErr ( "UpdateDevices: Failed to build the key for new device!" );
			}
#endif
			if ( devicePrev ) {
                if ( devicePrev->next ) {
                    dev->next = devicePrev->next;
                    devicePrev->next = dev;
                }
                else
                    devicePrev->next = dev;
			}
			else {
				if ( device ) {
					if ( device->info.deviceID > value ) {
						/// Device must be the listRoot (because there is no previous device)
						dev->next = device;
						*listRoot = dev;
					}
					else {
						if ( device->next ) {
							dev->next = device->next;
							device->next = dev;
						}
						else
							device->next = dev;
					}
				}
				else
					*listRoot = dev;
			}
			device = dev;
			
			changed = true; //flags |= DEVICE_INFO_ATTR_PROJECT_NAME | DEVICE_INFO_ATTR_DEVICE_NAME;
			strcpy_s ( device->info.projectName, sizeof(device->info.projectName), projectName );
			strcpy_s ( device->info.appName, sizeof(device->info.appName), appName );
			//if ( userName ) {
			//	strcpy_s ( device->userName, sizeof(device->userName), userName );
			//	//flags |= DEVICE_INFO_ATTR_USER_NAME;
			//}
		}
		else {
			CErr ( "UpdateDevices: Failed to allocate memory for new device!" );
			device = 0;
		}
	}

	if ( device ) {
		device->info.updates = greetUpdates;

#ifndef MEDIATORDAEMON
		if ( !device->info.ipe || device->info.broadcastFound == DEVICEINFO_DEVICE_BROADCAST )
#endif
		if ( ip != device->info.ipe ) {
			device->info.ipe = ip; changed = true; //flags |= DEVICE_INFO_ATTR_IPE;
		}

		/*if ( device->info.deviceType != msg [MEDIATOR_BROADCAST_DEVICETYPE_START] ) {
			device->info.deviceType = msg [MEDIATOR_BROADCAST_DEVICETYPE_START]; changed = true; //flags |= DEVICE_INFO_ATTR_DEVICE_TYPE;
		}
        */

		int platform = *((int *) (msg + MEDIATOR_BROADCAST_PLATFORM_START));

		if ( device->info.platform != platform ) {
			device->info.platform = platform; changed = true; //flags |= DEVICE_INFO_ATTR_DEVICE_TYPE;
		}

		pIntBuffer += 1;

		// Get the ip
		value = *pIntBuffer;
		if ( device->info.ip != (unsigned) value ) {
			device->info.ip = value; changed = true; //flags |= DEVICE_INFO_ATTR_IP;
		}	
		// Get ports
		unsigned short * pUShortBuffer = (unsigned short *) (msg + MEDIATOR_BROADCAST_PORTS_START);
		unsigned short svalue = *pUShortBuffer++;
		if ( svalue != device->info.tcpPort ) {
			device->info.tcpPort = svalue; changed = true; //flags |= DEVICE_INFO_ATTR_PORT_TCP;
		}

		svalue = *pUShortBuffer;
		if ( svalue != device->info.udpPort ) {
			device->info.udpPort = svalue; changed = true; //flags |= DEVICE_INFO_ATTR_PORT_UDP;
		}
		
		if ( strncmp ( device->info.deviceName, deviceName, sizeof(device->info.deviceName) ) ) {
			strcpy_s ( device->info.deviceName, sizeof(device->info.deviceName), deviceName ); changed = true; //flags |= DEVICE_INFO_ATTR_DEVICE_NAME;
		}

#ifdef MEDIATORDAEMON
		if ( device->info.broadcastFound != broadcastFound ) {
			device->info.broadcastFound = broadcastFound; changed = true; //flags |= DEVICE_INFO_ATTR_BROADCAST_FOUND;
		}
#endif
		if ( userName && *userName && strncmp ( device->userName, userName, sizeof(device->userName) ) ) {
			strcpy_s ( device->userName, sizeof(device->userName), userName ); changed = true; //flags |= DEVICE_INFO_ATTR_USER_NAME;
		}

		if ( changed ) {		
			CLogArg ( "UpdateDevices: Device      = [0x%X / %s / %s]", device->info.deviceID, device->info.deviceName, device->info.broadcastFound ? "on same network" : "by mediator" );
			if ( device->info.ip != device->info.ipe ) {
				CLogArg ( "UpdateDevices: Device IPe != IP  [%s]", inet_ntoa ( *((struct in_addr *) &device->info.ip) ) );
				CLogArg ( "UpdateDevices: Device IP  != IPe [%s]", inet_ntoa ( *((struct in_addr *) &device->info.ipe) ) );
			}
			else {
				CListLogArg ( "UpdateDevices: Device IPe [%s]", inet_ntoa ( *((struct in_addr *) &device->info.ipe) ) );
			}

			CListLogArg ( "UpdateDevices: Project/App = [%s / %s]", device->info.projectName, device->info.appName );
			CListLogArg ( "UpdateDevices: Device  IPe = [%s (from socket), tcp [%d], udp [%d]]", inet_ntoa ( *((struct in_addr *) &ip) ), device->info.tcpPort, device->info.udpPort );
		}
	}
	
	if ( pthread_mutex_unlock ( mutex ) ) {
		CErr ( "UpdateDevices: Failed to release mutex!" );
		device = 0;
	}

    /*value = 0;
    if ( device )
		value = device->info.deviceID;
    */
	UpdateDeviceInstance ( device, !found, changed );

Finish:

#ifdef MEDIATORDAEMON
	if ( appDevices )
		__sync_sub_and_fetch ( &appDevices->access, 1 );
#endif

	return device;
}


void Mediator::DevicesHasChanged ( int type )
{
	CVerb ( "DevicesHasChanged" );
}


void printDevices ( DeviceInstanceList * device )
{
	while ( device ) 
	{
		CLogArg ( "printDevices: Device id      = 0x%X", device->info.deviceID );
		CLogArg ( "printDevices: Project name   = %s", device->info.projectName );
		CLogArg ( "printDevices: App name       = %s", device->info.appName );
		CLogArg ( "printDevices: Platform       = %i", device->info.platform );
		CLogArg ( "printDevices: Device IPe     = %s (from socket)", inet_ntoa ( *((struct in_addr *) &device->info.ip) ) );
		CLogArg ( "printDevices: Device IPe != IP (%s)", inet_ntoa ( *((struct in_addr *) &device->info.ipe) ) );
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
				if ( pthread_mutex_trylock ( &mediatorMutex ) ) {
					CVerb ( "IsKnownMediator: Failed to aquire mutex on mediator!" );
					return false;
				}

				RegisterAtMediator ( med );

				if ( pthread_mutex_unlock ( &mediatorMutex ) ) {
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
	if ( !arg  ) {
		CErr ( "BroadcastThreadStarter: Called with invalid (NULL) argument.\n" );
		return 0;
	}

	Mediator * mediator = (Mediator *) arg;

	// Execute thread
	return mediator->BroadcastThread ( );
}


bool Mediator::AddMediator ( unsigned int ip, unsigned short port )
{
	CVerbArg ( "AddMediator: IP [%s] Port [%d]", inet_ntoa ( *((struct in_addr *) &ip) ) , port );

	if ( !port ) {
		CWarn ( "AddMediator: port [0] is invalid." );
		return false;
	}

	if ( IsKnownMediator ( ip, port ) ) {
		CVerbArg ( "AddMediator: mediator (%s) already available.", inet_ntoa ( *((struct in_addr *) &ip) ) );
		return true;
	}

	MediatorInstance * med = (MediatorInstance *)calloc ( 1, sizeof(MediatorInstance) );
	if ( !med ) {
		CErr ( "AddMediator: Failed to allocate memory for new Mediator!" );
		return false;
	}

	Zero ( med->connection.rec_mutex );
	if ( pthread_mutex_init ( &med->connection.rec_mutex, NULL ) ) {
		CErr ( "AddMediator: Failed to init rec_mutex!" );
		goto Failed;
	}
	//pthread_cond_init ( &med->connection.rec_signal, NULL );

	Zero ( med->connection.rec_signal );
	if ( pthread_cond_manual_init ( &med->connection.rec_signal, NULL ) ) {
		CErr ( "AddMediator: Failed to init rec_signal!" );
		goto Failed;
	}

	Zero ( med->connection.send_mutex );
	if ( pthread_mutex_init ( &med->connection.send_mutex, NULL ) ) {
		CErr ( "AddMediator: Failed to init send_mutex!" );
		goto Failed;
	}
	//pthread_mutex_init	( &med->connection.request_mutex, NULL );

	med->available = false;
	med->connection.instance.socket = -1;
	med->connection.instance.spareSocket = -1;
	med->ip = ip;
	med->port = port;

	med->connection.buffer = (char *) malloc ( MEDIATOR_BUFFER_SIZE_MAX );
	if ( !med->connection.buffer ) {
		CErr ( "AddMediator: Failed to allocate memory for new Mediator!" );
		goto Failed;
	}

	AddMediator ( med );

	return true;

Failed:
	free ( med );
	return false;
}


MediatorInstance * Mediator::AddMediator ( MediatorInstance * med )
{
	CVerb ( "AddMediator" );

	if ( !med ) {
		CErr ( "AddMediator: Mediator object passed in is invalid!" );
		return 0;
	}
	MediatorInstance * added = med;

	if ( pthread_mutex_lock ( &mediatorMutex ) ) {
		CErr ( "AddMediator: Failed to aquire mutex on mediator!" );
		free ( med->connection.buffer );
		free ( med );
		return 0;
	}

	if ( mediator.ip ) {
		// Find the last and make sure that we do not have this mediator already in the list
		MediatorInstance * t = &mediator;
		while ( t->next ) {
			t = t->next;
			if ( t->ip == med->ip && t->port == med->port ) {
				CVerbArg ( "AddMediator: Mediator [%s] Port [%d] is already in our list.", inet_ntoa ( *((struct in_addr *) &t->ip) ), t->port );
				free ( med );
				added = 0;
				goto Finish;
			}
		}

		// Attach the new one
		t->next = med;
	}
	else {
		memcpy ( &mediator, med, sizeof(MediatorInstance) );
		free ( med );
		added = &mediator;
	}

Finish:
	if ( pthread_mutex_unlock ( &mediatorMutex ) ) {
		CErr ( "AddMediator: Failed to releaes mutex on mediator!" );
	}

	return added;
}


bool Mediator::RemoveMediator ( unsigned int ip )
{
	bool ret = true;

	CVerb ( "RemoveMediator" );

	if ( pthread_mutex_lock ( &mediatorMutex ) ) {
		CErr ( "RemoveMediator: Failed to aquire mutex on mediator!" );
		return false;
	}

	// Find the mediator
	MediatorInstance * s = 0;
	MediatorInstance * t = &mediator;
	do {
		if ( ip == t->ip )
			break;

		s = t;
		t = t->next;
	} while ( t );

	if ( t ) {
		MediatorInstance * src = t->next;
		if ( t == &mediator ) {
			if ( src ) {
				// There is a next instance
				memcpy ( t, src, sizeof(MediatorInstance) );
				ReleaseMediator ( src );
			}
			else {
				// No more instances
				ReleaseMediator ( src );
				memset ( t, 0, sizeof(MediatorInstance) );
			}
		}
		else {
			s->next = t->next;
			ReleaseMediator ( t );
		}
	}

	if ( pthread_mutex_unlock ( &mediatorMutex ) ) {
		CErr ( "RemoveMediator: Failed to releaes mutex on mediator!" );
		ret = false;
	}

	return ret;
}



} /* namepace environs */

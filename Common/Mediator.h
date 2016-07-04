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
#pragma once
#ifndef INCLUDE_HCM_ENVIRONS_MEDIATOR_H
#define INCLUDE_HCM_ENVIRONS_MEDIATOR_H


#ifdef MEDIATORDAEMON
#   define  DAEMONEXP(exp)                      exp
#   define  CLIENTEXP(exp)

#   define MAX_STUNT_SOCKETS_IN_QUEUE			20
#   define ENABLE_MEDIATOR_LOCK

#	ifndef NDEBUG
//#		define	TRACE_MEDIATOR_OBJECTS
#	endif
#else
#   define  DAEMONEXP(exp)
#   define  CLIENTEXP(exp)                      exp

#   define MAX_STUNT_SOCKETS_IN_QUEUE			50

//#   define ENABLE_MEDIATOR_LOCK
#endif

//#define MEDIATOR_USE_TCP_NODELAY
//#define MEDIATOR_USE_SOCKET_BUFFERS_APPLY_AT_SERVER
//#define MEDIATOR_USE_SOCKET_BUFFERS_APPLY_AT_CLIENT
#define MEDIATOR_USE_HEX_APPAREAKEY

#ifdef MEDIATOR_USE_HEX_APPAREAKEY
#   define MEDIATOR_APP_AREA_KEY_FORMAT            "%08X%s %s"
#else
#   define MEDIATOR_APP_AREA_KEY_FORMAT            "%011i %s %s"
#endif

#ifndef MEDIATORDAEMON

//#   define DEBUG_SEQNR
#else
#   ifdef _WIN32
#       define ENABLE_WINSOCK_CLIENT_THREADS

#       define ENABLE_WINSOCK_SEND_THREADS
#   endif

#endif

#include "Interop/Threads.h"
#include "Interop/Sock.h"
#include "Device.Info.h"
#include "Environs.Crypt.h"
#include "Interop/Smart.Pointer.h"
#include "Queue.Vector.h"
#include "Tracer.h"

#ifdef MEDIATORDAEMON
#   include <string>
#   define USE_WRITE_LOCKS1
#   define USE_WRITE_LOCKS2
#   define USE_WRITE_LOCKS3
#   define USE_NOTIFY_TARGET_INDEX
#endif

#define MEDIATOR_PROTOCOL_VERSION               '8'

#define DEFAULT_MEDIATOR_PORT					5898
//#define DEFAULT_BROADCAST_PORT					5899

#define	BUFFERSIZE								1024
#define MESSAGE_BUFFER_SIZE						512
#define	IP_MAX_SIZE								22

#define MEDIATOR_ANONYMOUS_USER					"anonymous"
#define MEDIATOR_ANONYMOUS_PASSWORD				"secret"

#define	MAX_MEDIATOR_PORTS						4	// We allow max. 4 ports for connections to the mediator

#define MEDIATOR_MESSAGE_UNIT_ALIGN				4
#define MEDIATOR_MESSAGE_LENGTH_FLAG_PAD		0x10000000

#define MEDIATOR_BROADCAST						"E.Mediator"
#define MEDIATOR_BROADCAST_DEVICE				"E.D."
#define MEDIATOR_STUNT_SOCKET_REQ				"E.DS"
#define MEDIATOR_BROADCAST_DEVICEINFO			"E.D.Client"
#define MEDIATOR_BROADCAST_DEVICE_UPDATE		"E.D.DFlags"
#define MEDIATOR_BROADCAST_GREET				"E.D.Helo"
#define MEDIATOR_BROADCAST_BYEBYE				"E.D.Bye"
#define MEDIATOR_PROTOCOL_VERSION_MIN			'2'

#define	MEDIATOR_CMD_SET						's'
#define	MEDIATOR_CMD_GET						'g'
#define	MEDIATOR_CMD_GET_DEVICES				'd'
#define MEDIATOR_CMD_GET_DEVICES_LEN			16
#define MEDIATOR_CMD_GET_DEVICES_RESP_LEN		8
#define MEDIATOR_CMD_GET_DEVICES_COUNT_LEN		12
#define MEDIATOR_CMD_GET_DEVICES_COUNT_LEN_V6	16
#define MEDIATOR_CMD_GET_DEVICES_COUNT_RESP_LEN	12
#define MEDIATOR_CMD_GET_DEVICES_COUNT_RESP_LENV6	16
#define	MEDIATOR_CMD_HEARTBEAT					'h'
#define	MEDIATOR_CMD_GETALL						'a'
#define	MEDIATOR_CMD_SET_FILTERMODE				'f'
#define	MEDIATOR_CMD_SET_SOCKET_BUFFERS			'b'
#define	MEDIATOR_CMD_QUIT						'q'
#define	MEDIATOR_CMD_NATSTAT					'n'
#define	MEDIATOR_CMD_NOTIFICATION_SUBSCRIBE		'N'
#define	MEDIATOR_CMD_SHORT_MESSAGE				'm'
#define	MEDIATOR_CMD_MESSAGE_SUBSCRIBE          'M'
#define	MEDIATOR_CMD_REQ_STUNT_ID				'r'
#define	MEDIATOR_SRV_CMD_ALIVE_REQUEST			'A'
#define	MEDIATOR_SRV_CMD_SESSION_RETRY			'R'
#define	MEDIATOR_SRV_CMD_SESSION_LOCKED			'L'
#define	MEDIATOR_CMD_STUNT						'x'
#define	MEDIATOR_CMD_STUN						'y'
#define	MEDIATOR_CMD_STUNT_CLEAR				'k'
#define	MEDIATOR_CMD_HELP_TLS_GEN				't'
#define	MEDIATOR_CMD_SESSION_ASSIGN				's'
#define	MEDIATOR_CMD_DEVICE_FLAGS				'o'
#define	MEDIATOR_CMD_AUTHTOKEN_ASSIGN			'a'
#define	MEDIATOR_CMD_MEDIATOR_NOTIFY			'i'
#define	MEDIATOR_CMD_MEDIATOR_NOTIFY_SIZE		16
#define	MEDIATOR_OPT_NULL						';'
#define	MEDIATOR_OPT_BLANK						'.'
#define	MEDIATOR_OPT_SIGNED_CERT_RESPONSE		's'
#define	MEDIATOR_CMD_DEVICE_LIST_QUERY			'd'
#define	MEDIATOR_CMD_DEVICE_LIST_QUERY_COUNT	'c'
#define	MEDIATOR_CMD_DEVICE_LIST_QUERY_RESPONSE	'l'
#define	MEDIATOR_CMD_DEVICE_LIST_QUERY_ERROR	'e'
#define	MEDIATOR_OPT_DEVICE_LIST_DEVICE_ID		'i'
#define	MEDIATOR_CMD_DEVICE_LIST_QUERY_SEARCH	's'

#define	MEDIATOR_CMD_GET_VERSION				'v'

#define MEDIATOR_STUNT_REQ_SIZE					12
#define MEDIATOR_STUNT_REQ_EXT_SIZE             76
#define MEDIATOR_STUNT_CHANNEL_VERSATILE		';'
#define MEDIATOR_STUNT_CHANNEL_MAIN				':'
#define MEDIATOR_STUNT_CHANNEL_BULK				'.'
#define MEDIATOR_STUNT_CHANNEL_PORTAL			','
#define MEDIATOR_STUNT_RESP_SIZE				28
#define MEDIATOR_STUNT_ACK_SIZE					12
#define MEDIATOR_STUNT_ACK_SIZE_V6				16
#define MEDIATOR_STUNT_ACK_EXT_SIZE				20
#define MEDIATOR_STUNT_ACK_EXT_SIZE_V6			24
#define MEDIATOR_STUN_REQ_SIZE					16
#define MEDIATOR_STUN_RESP_SIZE					20

#define MEDIATOR_STUN_REQUEST                   'u'

#define MEDIATOR_NAT_REQ_SIZE					12
#define MEDIATOR_MSG_VERSION_SIZE				16
#define MEDIATOR_MSG_SOCKET_BUFFERS_SIZE		16

#define MEDIATOR_DEVICE_RELOAD					-1
#define MEDIATOR_DEVICE_CHANGE_NEARBY			1
#define MEDIATOR_DEVICE_CHANGE_MEDIATOR			2


const unsigned int		maxStuntSocketAlive	= 1000 * 60 * 5; // 5 min. (in ms)


namespace environs	/// Namespace: environs ->
{
    extern bool BuildAppAreaField ( unsigned char * sizesDst, const char * app, const char * area, bool ext );
    extern bool BuildAppAreaID ( char * sizesDst, int deviceID, const char * app, const char * area, char channel, unsigned int token );
	extern bool SetNonBlockSocket ( int &sock, bool set, const char * name );
    
    extern bool SocketTimeout ( int &sock, int recvSec, int sendSec, bool isMS = false );
    
    extern bool FakeConnect ( int sock, unsigned int port );

    extern void ShutdownCloseSocketI ( int sock, bool doClose CLOSE_SOCKET_ARG (const char * msg) );

#ifdef DEBUG_TRACK_SOCKET
#   define ShutdownCloseSocket(s,c,m)      ShutdownCloseSocketI ( s, c, m )
#else
#   define ShutdownCloseSocket(s,c,m)      ShutdownCloseSocketI ( s, c )
#endif

    typedef struct NetPack
	{
		unsigned int		ip;
		unsigned int		bcast;
		unsigned int		mask;
		unsigned int		gw;
		NetPack			*	next;
	}
	NetPack;
    
    
    class ILock
    {
        bool init;
        
    public:
        pthread_mutex_t		lock;
        
        bool Init ();
        bool Lock ( const char * func );
        bool Unlock ( const char * func );
        ILock ();
        ~ILock ();
    };

    class MediatorDaemon;

    class ILock1
    {
        friend class MediatorDaemon;

        pthread_mutex_t lock1;
        bool			init1;

    public:
        bool Init1 ();
        bool Lock1 ( const char * func );
        bool Unlock1 ( const char * func );
        ILock1 ();
        ~ILock1 ();
    };

    class ILock2
    {
        friend class MediatorDaemon;

        pthread_mutex_t lock2;
        bool			init2;

    public:
        bool Init2 ();
        bool Lock2 ( const char * func );
        bool Unlock2 ( const char * func );
        ILock2 ();
        ~ILock2 ();
    };


    class INoRWLock
    {
        friend class MediatorDaemon;

        pthread_mutex_t lock1;
        bool			init1;
        
    public:
        INoRWLock ();
        ~INoRWLock ();

        bool Init ();

        bool LockRead ( const char * f );

        bool UnlockRead ( const char * f );

        bool LockWrite ( const char * f );

        bool UnlockWrite ( const char * f );
    };

    
    // ILock1 is writer lock
    // EnvSignal is reader lock
    class IRWLock : ILock1, EnvSignal
    {
        friend class MediatorDaemon;

        int reader;

    public:
        bool Init ()
        {
            reader = 0;

            if ( !EnvSignal::Init () )
                return false;

			Notify ( "" );
            return Init1 ();
        }

        bool LockRead ( const char * f )
        {
            if ( !Lock1 ( f ) )
                return false;
            if ( !Lock ( f ) )
                return false;

            reader++;

            if ( reader == 1 )
                ResetSync ( f );

            if ( !Unlock ( f ) )
                return false;

            return Unlock1 ( f );
        }


        bool UnlockRead ( const char * f )
        {
            if ( !Lock ( f ) )
                return false;

            reader--;

            if ( reader == 0 )
                Notify ( f );

            return Unlock ( f );
        }


        bool LockWrite ( const char * f )
        {
            if ( !Lock1 ( f ) )
                return false;

            WaitOne ( f );
            return true;
        }
        
        
        bool UnlockWrite ( const char * f )
        {
            return Unlock1 ( f );
        }
    };


#ifdef USE_MEDIATOR_OPT_KEY_MAPS_COMP
	typedef struct AppAreaKey
	{
		int			deviceID;
		char		appArea [ MAX_NAMEPROPERTY * 2 ];
	}
	AppAreaKey;
#endif


    class SendLoad
    {
    public:
        SendLoad () : buffer ( 0 ) {}
        
        ~SendLoad () {
            if ( buffer )
                free ( buffer );
        }
        
        void * buffer;
    };
    
    
    class SendContext
    {
    public:
		// size ( 0 ),  Init of size not necessary, since it is guaranteed to be initialized by enqueue
        SendContext () : buffer ( 0 ), seqNr ( 0 ),
#   if defined(MEDIATORDAEMON)
        done ( false ),
#   endif
        freeSendBuffer ( false ), sendBuffer ( 0 ), sendCurrent ( 0 ), sendSize ( 0 )
#	if !defined(MEDIATORDAEMON)
		//, addr ( EmptyStructValue( sockaddr_in ) ), sock ( 0 ),  Init of addr not necessary, since it is guaranteed to be initialized by enqueue
#	endif
        {
            TraceSendContextAdd ( this );
        }
        
        ~SendContext ()
        {
            if ( freeSendBuffer && sendBuffer )
                free ( sendBuffer );
            
            if ( buffer )
                free ( buffer );

            TraceSendContextRemove ( this );
        }

        
#   ifdef MEDIATORDAEMON
        sp ( SendLoad )         dataSP;
#   endif
        
        char *                  buffer;
        unsigned int            size;
        unsigned int            seqNr;
        
#   if defined(MEDIATORDAEMON)
        bool                    done;
#   endif
        
        bool					freeSendBuffer;
        char *                  sendBuffer;
        unsigned int            sendCurrent;
        unsigned int            sendSize;
        
#if !defined(MEDIATORDAEMON)
        struct 	sockaddr_in		addr;
        //int                     sock; // obsolete
#endif
    };
    
#	if defined(TRACE_MEDIATOR_OBJECTS) && defined(MEDIATORDAEMON)
#		ifdef CLASS_NAME
#			undef CLASS_NAME
#		endif
#	endif
    
    
    typedef struct ClientProcContext
    {
        char *          msg;
        char *          msgEnd;
        unsigned int	remainingSize;
    }
    ClientProcContext;


	struct DeviceInstanceNode;
    
    typedef struct StuntRegisterContext
    {
        StuntRegisterContext () : registerTime ( 0 ), token ( 0 ), sockI ( INVALID_FD ), sockC ( INVALID_FD )
        {
            Zero ( addrI ); Zero ( addrC );
        }
        
		~StuntRegisterContext () {
			if ( IsValidFD ( sockI ) ) {
				ShutdownCloseSocket ( sockI, true, "StuntRegisterContext interact" );
			}
			if ( IsValidFD ( sockC ) ) {
				ShutdownCloseSocket ( sockC, true, "StuntRegisterContext comdat" );
			}
		}

        unsigned int            registerTime;
        unsigned int            token;
		int						sockI;
        struct 	sockaddr_in		addrI;
		int						sockC;
        struct 	sockaddr_in		addrC;
    }
    StuntRegisterContext;
    

#ifdef USE_MEDIATOR_CLIENT_INSTANCE_BUFFER_UP
	typedef struct AssignableBufferUP
	{
		up ( char[] )             bufferUP;

		AssignableBufferUP& operator=( AssignableBufferUP& other )
		{
			if ( &other == this )
				return *this;

			bufferUP = std::move ( other.bufferUP );
			return *this;
		}
	}
	AssignableBufferUP;
#endif


    typedef struct ThreadInstance DAEMONEXP ( : public ILock )
	{
		void				*	daemon;
        int						deviceID;
		unsigned int			version;
		long long				sessionID;
        
        ThreadSync              thread;        

		INTEROPTIMEVAL			aliveLast;
		INTEROPTIMEVAL			connectTime;
        
        int                     socket;
        DAEMONEXP ( int         socketToClose; )
        
		unsigned short			port;
		unsigned short			portUdp;

		struct 	sockaddr_in		addr;
        
        std::map < std::string, sp ( StuntRegisterContext ) > stuntSocketsLog;

        pthread_mutex_t         stuntSocketLock;

		short                   filterMode;
		
		int						encrypt;
		int						authenticated;
		AESContext				aes;

        int                     authLevel;
		bool					createAuthToken;
        char					uid [ MAX_NAMEPROPERTY * 6 ];
        char                    ips [ 20 ];
        
        unsigned int			seqNr;
       
        DAEMONEXP ( bool subscribedToNotifications );
        DAEMONEXP ( bool subscribedToMessages );
        
#ifdef __cplusplus
                
        void                        SendTcpFin ();
        
		sp ( ThreadInstance )       clientSP;
		sp ( DeviceInstanceNode )   deviceSP;
        
        
        lib::QueueVector			sendQueue;
        lib::QueueVector			sendQueuePrior;
        pthread_mutex_t				sendQueueLock;

#	ifdef USE_MEDIATOR_CLIENT_WINSOCK_SOCKETS
		HANDLE						receiveEvent;
#	endif
        
#ifdef MEDIATORDAEMON
        LONGSYNC                    sendBusy1;

#ifdef USE_NOTIFY_TARGET_INDEX
        int                         inNotifierList;
#endif
		bool						inAcceptorList;
        short                       sendFails;

#ifdef USE_MEDIATOR_CLIENT_INSTANCE_BUFFER_UP
		AssignableBufferUP          bufferUP;
#else
		char *						bufferPtr;
#endif
        ClientProcContext           ctx;
#endif        
        bool                        allocated;
        
        ThreadInstance ();
        ~ThreadInstance ();
        
        void                        Reset ();
        bool                        Init ();
        void                        Dispose ();
#endif
	}
	ThreadInstance;

    void DisposeSendContexts ( ThreadInstance * client );
    

    class ApplicationDevices :
#ifdef USE_WRITE_LOCKS1
    public IRWLock
#else
    public INoRWLock
#endif
	{
	public:
		ApplicationDevices ();
		~ApplicationDevices ();

		unsigned int			id;
		unsigned int			areaId;
		int						count;
		long					access;
        
#ifdef MEDIATORDAEMON
        std::string             name;
#endif
		long					latestAssignedID;

		DeviceInstanceNode	*	devices;

#ifdef USE_WRITE_LOCKS2
        pthread_mutex_t         deviceCacheLock;
#endif
		bool					deviceCacheDirty;
        int						deviceCacheCount;
        int						deviceCacheCapacity;
        char                *   devicesCache;
	};

    
    // "int11 1s areaNameMAX_PROP 1s appNameMAX_PROP 1e"
#ifdef USE_MEDIATOR_OPT_KEY_MAPS_COMP
#   define MAX_DEVICE_INSTANCE_KEY_LENGTH      sizeof(AppAreaKey)
#else
#   define MAX_DEVICE_INSTANCE_KEY_LENGTH      (11 + MAX_NAMEPROPERTY + MAX_NAMEPROPERTY + 9)
#endif

    CLIENTEXP ( class DeviceController; )
    
    
	typedef struct DeviceInstanceNode
	{
		lib::DeviceInfo info;
        
        DAEMONEXP ( sp ( ApplicationDevices ) rootSP; )
        DAEMONEXP ( sp ( ThreadInstance )     clientSP; )

#ifdef USE_MEDIATOR_OPT_KEY_MAPS_COMP
        CLIENTEXP ( AppAreaKey			key; );
#else
        // "int11 1s areaNameMAX_PROP 1s appNameMAX_PROP 1e"
        CLIENTEXP ( char				key [ MAX_DEVICE_INSTANCE_KEY_LENGTH ]; );
#endif
		//char							userName	[ MAX_NAMEPROPERTY + MAX_NAMEPROPERTY + 1 ]; // 31

        // Links used by Mediator base layer
		struct DeviceInstanceNode	*	next;  // 4
		struct DeviceInstanceNode	*	prev;  // 4
		
        CLIENTEXP ( int					hEnvirons; )
        CLIENTEXP ( bool				allowConnect; )
		        
#ifdef __cplusplus        
        sp ( DeviceInstanceNode )		baseSP;         // SP used by Mediator base layer
        
        CLIENTEXP ( sp ( DeviceInstanceNode ) mapSP; )  // SP used by Mediator client layer
        
        CLIENTEXP ( wp ( DeviceController ) deviceSP; )  // WP used by Mediator client layer
        
        DeviceInstanceNode ();
        
		~DeviceInstanceNode ();
#endif
        
	}
	DeviceInstanceNode;


	typedef struct DeviceInfoNode
    {
        // Links used by Mediator base layer
        struct DeviceInfoNode	*	next;  // 4
        struct DeviceInfoNode	*	prev;  // 4
        
		lib::DeviceInfoShort info;
	}
	DeviceInfoNode;

    
	typedef struct DeviceInfoNodeFirst
    {
        // Links used by Mediator base layer
        struct DeviceInfoNode	*	next;  // 4
        struct DeviceInfoNode	*	prev;  // 4
        
		lib::DeviceInfo info;
    
	}
	DeviceInfoNodeFirst;


	typedef struct DeviceInfoNodeV4
	{
		lib::DeviceInfo info;

		// Links used by Mediator base layer
		struct DeviceInfoNodeV4	*	next;  // 4        
	}
	DeviceInfoNodeV4;


	typedef struct MediatorThreadInstance
    {
        bool                    allocated;
		ThreadInstance			instance;
		int						socketUdp;
        
        ThreadSync              udp;
        ThreadSync              watchdog;
        
#ifdef __cplusplus
        MediatorThreadInstance ();
        ~MediatorThreadInstance ();
        
        bool                    Init ();
        void                    Reset ();
        void                    Dispose ();
#endif
	}
    MediatorThreadInstance;
    
    
    typedef struct MediatorResponseBuffer
    {
        int         size;
        char    *   buffer;
        bool        success;
    }
    MediatorResponseBuffer;


	typedef struct MediatorConnection
	{
		ThreadInstance			instance;
        bool                    allocated;

		char				*	buffer;

		pthread_mutex_t			receiveLock;
		pthread_cond_t			receiveEvent;
        bool                    longReceive;
        
        std::map < int, MediatorResponseBuffer * > responseBuffers;
        
        Instance            *   env;
        
        MediatorConnection ();
        ~MediatorConnection ();
        
#ifdef __cplusplus
        bool                    Init ();
        void                    Reset ();
        void                    Dispose ();
#endif
	}
	MediatorConnection;


	typedef struct MediatorInstance
	{
		unsigned int			ip;
		unsigned short			port;
        
		bool					enabled;
		bool					listening;
		MediatorConnection		connection;
        void                *   mediatorObject;
		MediatorInstance	*	next;
        
        MediatorInstance ();
        ~MediatorInstance ();
        
#ifdef __cplusplus
        bool                    Init ();
        void                    Reset ();
        void                    Dispose ();
#endif
	}
	MediatorInstance;

	typedef void ( *MediatorCallback )(void *);

#if (!defined(MEDIATORDAEMON) && defined(__cplusplus))
	class Instance;
#endif

	/**
	*	Mediator base functionality
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@created	02/01/13
	*	@version	1.0
	*	@remarks
	* ****************************************************************************************
	*/
	class Mediator
	{
	public:
		Mediator ( );
		virtual ~Mediator ( );

		bool					Init ();
		bool					Start ();
		bool					IsStarted ();

		void					StopMediators ();

		virtual void			BuildBroadcastMessage ( bool withStatus = true ) = 0;
		bool					SendBroadcast ( bool enforce = false, bool sendStatus = false, bool sendToAny = false, bool enforceEnqueue = false );
		bool					SendBroadcastWithSocket ( bool enforce = false, bool sendStatus = false, bool sendToAny = false, int sock = -1 );

		bool					AddMediator ( unsigned int ip, unsigned short port );
        
        void					BroadcastByeBye ( int sock = -1 );
		void					BroadcastGenerateToken ();
        
        void					ReleaseMediators ( );
        
        static bool				LoadNetworks ();
        static unsigned int		GetLocalIP ();
        static unsigned int		GetLocalSN ();

        static bool				InitClass ();
        static void				DisposeClass ();
        
        static bool             IsAnonymousUser ( const char * user );
		virtual bool			IsServiceAvailable () { return true; };
        
        static int              Connect ( int deviceID, int &sock, struct sockaddr * addr, int timeoutSeconds = 0, const char * name = "NoName" );
        
        static void             UnConnectUDP ( int sock );
        
		static int              localNetsSize;
		static unsigned long long	wifiMAC;
		static unsigned long long	btMAC;

	protected:
        bool					allocated;
        static bool				allocatedClass;

#if (!defined(MEDIATORDAEMON) && defined(__cplusplus))
		Instance			*	env;
#endif        
		bool					isRunning;
		bool					broadcastRunning;
		bool					aliveRunning;
		char			*		certificate;

		static NetPack          localNets;
        static pthread_mutex_t  localNetsLock;
        
#ifdef ENABLE_MEDIATOR_LOCK
		pthread_mutex_t			mediatorLock;
#endif
		MediatorInstance		mediator;

		pthread_mutex_t			devicesLock;
        
        int                     broadcastReceives;
		unsigned int			broadcastMessageLen;
		unsigned int			broadcastMessageLenExt;
        char					broadcastMessage [ MEDIATOR_BROADCAST_DESC_START + ((MAX_NAMEPROPERTY + 2) * 6) + 188 ]; // 4; 12; 4; 4; 2; 2; => 24 byte; max. 50 byte for areaName
        
        unsigned int            udpStatusMessageOffset;
        unsigned int            udpStatusMessageLen;
        char                    udpStatusMessage [ MEDIATOR_BROADCAST_DESC_START + ((MAX_NAMEPROPERTY + 2) * 6) + 4]; // 4; 12; 4; 4; 2; 2; => 24 byte; max. 50 byte for areaName

		unsigned int            lastGreetUpdate;

        int                     broadcastThreadRestarts;
		int						broadcastSocket;
        ThreadSync              broadcastThread;

#ifdef USE_MEDIATOR_NON_BLOCK_BROADCAST_WINSOCK
		HANDLE					broadcastEvent;
#endif

        bool                    PrepareAndStartBroadcastThread ( bool runThread = true );
        
        virtual bool            PushSendBC ( bool copy, void * buffer, unsigned int size, unsigned int ip, unsigned short port ) { return false; };
        virtual bool            SendBufferOrEnqueueBC ( bool copy, void * buffer, unsigned int size, unsigned int ip, unsigned short port ) { return false; };        

		bool					IsLocalIP ( unsigned int ip );
        static void				VerifySockets ( ThreadInstance * inst, bool waitThread );
        
        static bool				IsSocketAlive ( int &sock );
        
		virtual void			OnStarted ( );

		void					VanishedDeviceWatcher ();

		virtual sp ( ApplicationDevices ) GetDeviceList ( char * areaName, char * appName, pthread_mutex_t ** mutex, int ** pDevicesAvailable, DeviceInstanceNode ** &list ) = 0;

		virtual void			RemoveDevice ( unsigned int ip, char * msg ) {};
		virtual void			RemoveDevice ( DeviceInstanceNode * device, bool useLock = true ) = 0;

		int                     randBroadcastToken;
        
#ifdef __cplusplus
        
#	ifdef MEDIATORDAEMON
		sp ( DeviceInstanceNode ) UpdateDevicesV4 ( unsigned int ip, char * msg, char ** uid, bool * created, char isBroadcast = 0 );
#	endif

#   ifdef MEDIATORDAEMON
	 	sp ( DeviceInstanceNode )
#   else        
        bool
#   endif
            UpdateDevices ( unsigned int ip, char * msg, char ** uid, bool * created, char isBroadcast = 0 );

		virtual void			UpdateDeviceInstance ( const sp ( DeviceInstanceNode ) & device, bool added, bool changed ) = 0;
#endif
		virtual void			ReleaseDevices ( ) = 0;
		
		MediatorInstance *		IsKnownMediator ( unsigned int ip, unsigned short port );
		MediatorInstance *		AddMediator ( MediatorInstance * med );
		virtual bool			RegisterAtMediator ( MediatorInstance * med );
		bool					RemoveMediator ( unsigned int ip );

		static void *			BroadcastThreadStarter ( void *arg );
		virtual void *			BroadcastThread ( ) = 0;

		void					Dispose ( );
		void					ReleaseMediator ( MediatorInstance * med );
        bool					ReleaseThreads ( bool wait );
        
        static void				ReleaseNetworks ( );

	private:
        static void				AddNetwork ( unsigned int ip, unsigned int bcast, unsigned int netmask, unsigned int gw );
        static unsigned int		GetBroadcast ( unsigned int ip, unsigned int netmask );

		virtual void 			DevicesHasChanged ( int type );
	};

	/// Attention: 64 bit value must be aligned with 8 bytes
	typedef struct _MediatorMsg
	{
		unsigned int	size;
		char			cmd0;
		char			cmd1;
		char			opt0;
		char			opt1;

		union {
			long long		sessionID;

			struct {
				unsigned int	msgID;

				union {
					unsigned int	id;
					int				notifyDeviceID;
					char			msg;
				}
				data;
			} id2;
		} ids;

	}
	MediatorMsg;
    
    
    NET_PACK_PUSH1
    
    /**
     * Mediator status (Device updates) message structure
     */
	typedef struct _MediatorStatusMsg
	{
		unsigned int	size;
		char			cmd0;
		char			cmd1;
		char			opt0;
		char			opt1;

		int				deviceID;
		int				status1;
		int				status2;
        int				status3;
        
        char            status45 [ 2 ];
        unsigned char   sizes [ 2 ];
	}
	NET_PACK_ALIGN MediatorStatusMsg;
    
    
    /**
     * Mediator status (Device updates) message structure
     * with appArea space
     */
	typedef struct _MediatorStatusMsgExt
	{
        MediatorStatusMsg header;
		char			  appArea [ ( MAX_NAMEPROPERTY + 2 ) * 2 ];
	}
	NET_PACK_ALIGN MediatorStatusMsgExt;

    
    /**
     * Mediator registration/deviceID request/authoToken message structure
     */
	typedef struct _MediatorReqHeader
	{
		unsigned int	size;
		char			cmd0;
		char			cmd1;
		char			opt0;
        char			opt1;
        
        char            pads [ 2 ];
        unsigned char   sizes [ 2 ];
	}
    NET_PACK_ALIGN MediatorReqHeader;
    
    
    /**
     * Mediator registration/deviceID request/authoToken message structure
     * with appArea space
     */
    typedef struct _MediatorReqMsg
    {
        MediatorReqHeader   header;
        
        char                pad;
        
        /** The machine name of the device */
        char                deviceUID [MAX_NAMEPROPERTY * 6]; // 31
        
        char                appArea [ ( MAX_NAMEPROPERTY + 2 ) * 2 ];
    }
    NET_PACK_ALIGN MediatorReqMsg;
    
    
    /**
     * Mediator registration/deviceID request/authoToken message structure
     * Deprecated V4 protocol
     */
    typedef struct _MediatorReqMsgV4
    {
        unsigned int	size;
        char			cmd0;
        char			cmd1;
        char			opt0;
        char			opt1;
        
        /** The area name of the application environment. */
        char			areaName [MAX_NAMEPROPERTY + 1]; // 31
        
        /** The application name of the application environment. */
        char			appName [MAX_NAMEPROPERTY + 1]; // 31
        
        /** The machine name of the device */
        char			deviceUID [MAX_NAMEPROPERTY * 6]; // 31
    }
    NET_PACK_ALIGN MediatorReqMsgV4;


	typedef struct MediatorGetPacketV6
	{
		unsigned int	size;
        char			version;
		char			cmd;
		char			opt0;
        char			opt1;
        unsigned int	seqNr;
	}
    NET_PACK_ALIGN MediatorGetPacketV6;
    
    
    typedef struct _MediatorGetPacket
    {
        unsigned int	size;
        char			version;
        char			cmd;
        char			opt0;
        char			opt1;
    }
    NET_PACK_ALIGN MediatorGetPacket;
    
    
    /**
     * Mediator notification message structure
     * Deprecated V4 protocol
     */
	typedef struct _MediatorNotifyV4
	{
		unsigned int	size;
		char			cmd0;
		char			cmd1;
		char			opt0;
		char			opt1;

		unsigned int	msgID;
		int				notifyDeviceID;

		/** The area name of the application environment. */
		char			areaName [MAX_NAMEPROPERTY + 1]; // 31

		/** The application name of the application environment. */
		char			appName [MAX_NAMEPROPERTY + 1]; // 31
	}
    MediatorNotifyV4;
    
    
    
    /**
     * Mediator notification message structure
     */
    typedef struct _MediatorNotifyHeader
    {
        unsigned int	size;
        char			cmd0;
        char			cmd1;
        char			opt0;
        char			opt1;
        
        unsigned int	msgID;
        int				notifyDeviceID;
        
        unsigned char   pads [ 2 ];
        unsigned char   sizes [ 2 ];
    }
    NET_PACK_ALIGN MediatorNotifyHeader;
    
    
    /**
     * Mediator notification message structure
     * with appArea space
     */
    typedef struct _MediatorNotify
    {
        MediatorNotifyHeader header;
        char				 appArea [ ( MAX_NAMEPROPERTY + 2 ) * 2 ];
    }
    NET_PACK_ALIGN MediatorNotify;
    
    
    /**
     * Mediator device list query message structure
     */
	typedef struct _MediatorQueryHeaderV6
	{
		unsigned int	size;
		char			cmdVersion;
		char			cmd1;
		char			opt0;
        char			opt1;
        unsigned int	seqNr;

		unsigned int	msgID;
		unsigned int	resultCount;
		int				deviceID;

		unsigned char   sizes [ 2 ];
    }
	NET_PACK_ALIGN MediatorQueryHeaderV6;
    
    
    /**
     * Mediator device list query message structure
     * with appArea space
     */
	typedef struct MediatorQueryMsgV6
	{
		MediatorQueryHeaderV6	header;
		char                    appArea [ ( MAX_NAMEPROPERTY + 2 ) * 2 ];
    }
    NET_PACK_ALIGN MediatorQueryMsgV6;
    
    /**
     * Mediator device list query message structure
     */
    typedef struct _MediatorQueryHeader
    {
        unsigned int	size;
        char			cmdVersion;
        char			cmd1;
        char			opt0;
        char			opt1;
        
        unsigned int	msgID;
        unsigned int	resultCount;
        int				deviceID;
        
        unsigned char   sizes [ 2 ];
    }
    NET_PACK_ALIGN MediatorQueryHeader;
    
    
    /**
     * Mediator device list query message structure
     * with appArea space
     */
    typedef struct _MediatorQueryMsg
    {
        MediatorQueryHeader	header;
        char				appArea [ ( MAX_NAMEPROPERTY + 2 ) * 2 ];
    }
    NET_PACK_ALIGN MediatorQueryMsg;
    
    NET_PACK_POP
    
    
    /**
     * Mediator device list query message structure
     * Deprecated V4 protocol
     */
	typedef struct _MediatorQueryMsgV4
	{
		unsigned int	size;
		char			cmdVersion;
		char			cmd1;
		char			opt0;
		char			opt1;

		unsigned int	msgID;
		unsigned int	startIndex;
		int				deviceID;

		/** The area name of the application environment. */
		char			areaName [ MAX_NAMEPROPERTY + 1 ]; // 31

														   /** The application name of the application environment. */
		char			appName [ MAX_NAMEPROPERTY + 1 ]; // 31
	}
	MediatorQueryMsgV4;

    
    /**
     * Mediator device list response message structure
     * Header part
     */
	typedef struct _MediatorQueryResponse
	{
		unsigned int	size;
		char			cmd0;
		char			cmd1;
		char			opt0;
		char			opt1;

		lib::DeviceHeader	deviceHead;
	}
    MediatorQueryResponse;
    
    
    /**
     * Mediator device list response message structure
     * Header part
     */
    typedef struct MediatorQueryResponseV6
    {
        unsigned int	size;
        char			cmd0;
        char			cmd1;
        char			opt0;
        char			opt1;
        unsigned int	seqNr;
        
        lib::DeviceHeader	deviceHead;
    }
    MediatorQueryResponseV6;
    
    
    /**
     * Mediator stunt socket registration message structure
     */
	typedef struct StuntSockRegPack
	{
		unsigned int	alignPad;
		unsigned int	sizeEncrypted;
		unsigned int	sizeReq;

		char			ident [4];
		long long		sessionID;
		unsigned int	sizePayload;
		char			payload;
	}
    StuntSockRegPack;

    
    /**
     * Mediator stunt socket registration message structure
     */
	typedef struct StuntSockDecPack
	{
		unsigned int	sizeReq;

		char			ident [4];
		long long		sessionID;
		unsigned int	sizePayload;
		char			payload;
	}
	StuntSockDecPack;
    
    
    NET_PACK_PUSH1

		typedef struct AppAreaBuffer
	{
		char            appArea [ ( MAX_NAMEPROPERTY + 2 ) * 2 ];
	}
	NET_PACK_ALIGN AppAreaBuffer;


	/**
	* Mediator stunt socket clear message structure
	*/
	typedef struct StuntClearTarget
	{
		unsigned int	size;
		char			version;
		char			ident [ 3 ];

		int				deviceID;
		unsigned char   sizes [ 2 ];
	}
	NET_PACK_ALIGN StuntClearTarget;


	/**
	* Mediator stunt socket registration message structure
	*/
	typedef struct StuntSockRegTarget
	{
		unsigned int	size;
        
        int				deviceID;
        unsigned int    token;
        char			channelType;
        char			pad0;
        unsigned char   sizes [ 2 ];
	}
    NET_PACK_ALIGN StuntSockRegTarget;
    
    
    typedef struct StuntSockRegKey
    {
        unsigned int	size;
        char          * key;
    }
    NET_PACK_ALIGN StuntSockRegKey;
    
    
	typedef struct STUNTRegReqPacketV8
	{
		unsigned int	size;
		char			version;
		char			ident [ 3 ];

		unsigned int	notify;

        int				deviceID;
        unsigned int    token;
		char			channelType;
		char			pad0;
		unsigned char   sizes [ 2 ];
	}
    NET_PACK_ALIGN STUNTRegReqPacketV8;
    
    
    typedef struct STUNTRegReqBufferV8
    {
        STUNTRegReqPacketV8 header;
        AppAreaBuffer   appArea;
    }
    NET_PACK_ALIGN STUNTRegReqBufferV8;
    
    
    /**
     * Mediator STUNT request message structure
     */
    typedef struct STUNTReqHeaderV8
    {
        unsigned int	size;
        char			version;
        char			ident [ 2 ];
        char			channel;
        unsigned int	seqNr;
        
        int				deviceID;
        unsigned int    token;
        
        unsigned char   sizes [ 2 ];
    }
    NET_PACK_ALIGN STUNTReqHeaderV8;
    
    
    typedef struct STUNTReqHeaderV6
	{
		unsigned int	size;
        char			version;
		char			ident [ 2 ];
        char			channel;
        unsigned int	seqNr;

		int				deviceID;

		unsigned char   sizes [ 2 ];
	}
    NET_PACK_ALIGN STUNTReqHeaderV6;
    
    
    /**
     * Mediator STUNT request message structure
     * with appArea space
     */
    typedef struct STUNTReqPacketV8
    {
        STUNTReqHeaderV8	header;
        char                appArea [ ( MAX_NAMEPROPERTY + 2 ) * 2 ];
    }
    NET_PACK_ALIGN STUNTReqPacketV8;
    
    
    typedef struct STUNTReqPacketV6
    {
        STUNTReqHeaderV6	header;
        char                appArea [ ( MAX_NAMEPROPERTY + 2 ) * 2 ];
    }
    NET_PACK_ALIGN STUNTReqPacketV6;
    
    /**
     * Mediator STUNT request message structure
     */
    typedef struct _STUNTReqHeader
    {
        unsigned int	size;
        char			version;
        char			ident [ 2 ];
        char			channel;
        
        int				deviceID;
        
        unsigned char   sizes [ 2 ];
    }
    NET_PACK_ALIGN STUNTReqHeader;
    
    
    /**
     * Mediator STUNT request message structure
     * with appArea space
     */
	typedef struct _STUNTReqPacket
	{
		STUNTReqHeader	header;
		char			appArea [ ( MAX_NAMEPROPERTY + 2 ) * 2 ];
	}
    NET_PACK_ALIGN STUNTReqPacket;
    
    NET_PACK_POP
    
    
    /**
     * Mediator STUNT request message structure
     * Deprecated V4 protocol
     */
	typedef struct _STUNTReqPacketV4
	{
		unsigned int	size;
		char			version;
		char			ident [ 2 ];
		char			channel;

		int				deviceID;

		/** The area name of the application environment. */
		char			areaName [ MAX_NAMEPROPERTY + 1 ]; // 31

														   /** The application name of the application environment. */
		char			appName [ MAX_NAMEPROPERTY + 1 ]; // 31

		char			pad [ 2 ];
	}
	STUNTReqPacketV4;

    
    
    /**
     * Mediator STUNT response message structure
     */
	typedef struct _STUNTRespPacket
	{
		unsigned int	size;
		char			respCode;
		char			ident [2];
		char			channel;

        unsigned short	porti;
        unsigned short	porte;
		unsigned int	ip; // 12
		unsigned int	ipe; // 16
	}
    STUNTRespPacket;
    
    
    
    /**
     * Mediator STUNT response message structure
     */
    typedef struct STUNTRespPacketV6
    {
        unsigned int	size;
        char			respCode;
        char			ident [2];
        char			channel;
        unsigned int	seqNr;
        
        unsigned short	porti;
        unsigned short	porte;
        unsigned int	ip; // 12
        unsigned int	ipe; // 16
    }
    STUNTRespPacketV6;
    
    
    NET_PACK_PUSH1

	/**
	* Mediator STUNT response/request message structure
	*/
	typedef struct STUNTRespReqHeaderV8
	{
		unsigned int	size;
		char			ident [ 3 ];
		char			channel;
		unsigned int	seqNr;

		int				deviceID;
		int				token;
		unsigned int	ip;
		unsigned int	ipe;
		unsigned short	porti;
		unsigned short	porte;

		unsigned char   sizes [ 2 ];
	}
	NET_PACK_ALIGN STUNTRespReqHeaderV8;


	/**
	* Mediator STUNT response/request message structure
	* with appArea space
	*/
	typedef struct STUNTRespReqPacketV8
	{
		STUNTRespReqHeaderV8	header;
		char                    appArea [ ( MAX_NAMEPROPERTY + 2 ) * 2 ];
	}
	NET_PACK_ALIGN STUNTRespReqPacketV8;

    
    /**
     * Mediator STUNT response/request message structure
     */
	typedef struct STUNTRespReqHeaderV6
	{
		unsigned int	size;
		char			ident [ 3 ];
        char			channel;
        unsigned int	seqNr;

		int				deviceID;
		unsigned int	ip;
		unsigned int	ipe;
		unsigned short	porti;
		unsigned short	porte;

		unsigned char   sizes [ 2 ];
	}
	NET_PACK_ALIGN STUNTRespReqHeaderV6;

    
    /**
     * Mediator STUNT response/request message structure
     * with appArea space
     */
	typedef struct STUNTRespReqPacketV6
	{
		STUNTRespReqHeaderV6	header;
		char                    appArea [ ( MAX_NAMEPROPERTY + 2 ) * 2 ];
	}
    NET_PACK_ALIGN STUNTRespReqPacketV6;
    
    
    /**
     * Mediator STUNT response/request message structure
     */
    typedef struct _STUNTRespReqHeader
    {
        unsigned int	size;
        char			ident [ 3 ];
        char			channel;
        
        int				deviceID;
        unsigned int	ip;
        unsigned int	ipe;
        unsigned short	porti;
        unsigned short	porte;
        
        unsigned char   sizes [ 2 ];
    }
    NET_PACK_ALIGN STUNTRespReqHeader;
    
    
    /**
     * Mediator STUNT response/request message structure
     * with appArea space
     */
    typedef struct _STUNTRespReqPacket
    {
        STUNTRespReqHeader	header;
        char				appArea [ ( MAX_NAMEPROPERTY + 2 ) * 2 ];
    }
    NET_PACK_ALIGN STUNTRespReqPacket;
    
    
    /**
     * Mediator STUNT response/request message structure
     * Deprecated V4 protocol
     */
	typedef struct _STUNTRespReqPacketV4
	{
		unsigned int	size;
		//char			respCode;
		char			ident [ 3 ];
		char			channel;

		int				deviceID;
		unsigned int	ip;
		unsigned int	ipe;
		unsigned short	porti;
		unsigned short	porte;

		/** The area name of the application environment. */
		char			areaName [ MAX_NAMEPROPERTY + 1 ]; // 31

														   /** The application name of the application environment. */
		char			appName [ MAX_NAMEPROPERTY + 1 ]; // 31
		unsigned int	pad0;
	}
	STUNTRespReqPacketV4;

    
    /**
     * Mediator STUN UDP request message structure
     * Deprecated V4 protocol
     */
	typedef struct _STUNReqPacketV4
	{
		char			ident [4];

		int             sourceID;
		int             destID;

		/** The area name of the application environment. */
		char			areaName [MAX_NAMEPROPERTY + 1]; // 31

		/** The application name of the application environment. */
		char			appName [MAX_NAMEPROPERTY + 1]; // 31

		/** The area name of the application environment. */
		char			areaNameSrc [MAX_NAMEPROPERTY + 1]; // 31

		/** The application name of the application environment. */
		char			appNameSrc [MAX_NAMEPROPERTY + 1]; // 31
	}
    NET_PACK_ALIGN STUNReqPacketV4;
    
    
    /**
     * Mediator STUN UDP request message structure
     */
    typedef struct _STUNReqHeader
    {
        char			ident [4];
        
        int             sourceID;
        int             destID;
        
        unsigned char   pads [ 2 ];
        unsigned char   sizes [ 2 ];
    }
    NET_PACK_ALIGN STUNReqHeader;
    
    
    /**
     * Mediator STUNT response/request message structure
     * with appArea space
     */
    typedef struct STUNReqPacket
    {
        STUNReqHeader	header;
        char            appArea [ ( MAX_NAMEPROPERTY + 2 ) * 4 ];
    }
    NET_PACK_ALIGN STUNReqPacket;
    
    
    /**
     * Mediator to client STUN UDP -> TCP request response message structure
     */
    typedef struct _STUNReqReqHeader
    {
        unsigned int	size;
        char			ident [4];
        
        int				deviceID;
        unsigned int	IPi;
        unsigned int	IPe;
        unsigned short	Porti;
        unsigned short	Porte;
        
        unsigned char   pads [ 2 ];
        unsigned char   sizes [ 2 ];
    }
    NET_PACK_ALIGN STUNReqReqHeader;
    
    
    /**
     * Mediator to client STUN UDP -> TCP request response message structure
     * with appArea space
     */
    typedef struct _STUNReqReqPacket
    {
        STUNReqReqHeader header;
        char			 appArea [ ( MAX_NAMEPROPERTY + 2 ) * 2 ];
    }
    NET_PACK_ALIGN STUNReqReqPacket;
    
    NET_PACK_POP
    
    /**
     * Mediator to client STUN UDP -> TCP request response message structure
     * Deprecated V4 protocol
     */
	typedef struct _STUNReqReqPacketV4
	{
		unsigned int	size;
		char			ident [4];

		int				deviceID;
        unsigned int	IPi;
        unsigned int	IPe;
        unsigned short	Porti;
        unsigned short	Porte;

		/** The area name of the application environment. */
		char			areaName [MAX_NAMEPROPERTY + 1]; // 31

		/** The applcation name of the application environment. */
		char			appName [MAX_NAMEPROPERTY + 1]; // 31
	}
	STUNReqReqPacketV4;

	
	typedef struct _STUNTRegReqPacket
	{
		unsigned int	size;
		char			version;
		char			ident [3];

		unsigned int	notify;
		unsigned int	pad0;
	}
	STUNTRegReqPacket;

    
    /**
     * Device to device udp hello message structure
     */
	typedef struct _UdpHelloPacket
	{
		char			ident [3];
		char			version;

		int				deviceID;

		/** The area name of the application environment. */
		char			areaName [MAX_NAMEPROPERTY + 1]; // 31

		/** The application name of the application environment. */
		char			appName [MAX_NAMEPROPERTY + 1]; // 31
	}
	UdpHelloPacket;

    
    /**
     * Mediator server encoded udp STUN message structure
     */
	typedef struct _UdpEncHelloPacket
	{
		unsigned int	size;
		unsigned int	alignPad;
		long long		sessionID;
		char			aes;
	}
    UdpEncHelloPacket;
    
    
    
    NET_PACK_PUSH1
    
    /**
     * Mediator short message to Server and response message structure
     */
    typedef struct _ShortMsgPacketHeader
    {
        unsigned int	size;
        char			version;
        char			ident [3];
        
        int				deviceID;
        
        char			pads [2];
        unsigned char   sizes [2];
    }
    NET_PACK_ALIGN ShortMsgPacketHeader;
    
    
    /**
     * Mediator short message to Server and response message structure
     * with appArea space
     */
	typedef struct _ShortMsgPacket
	{
		ShortMsgPacketHeader	header;
        char                    appArea [ ( MAX_NAMEPROPERTY + 2 ) * 2 ];
	}
    NET_PACK_ALIGN ShortMsgPacket;

    
    /**
     * Device short message between devices message structure
     */
	typedef struct _ShortMsgHeader
	{
		char 			preamble [ 4 ];	// 4 bytes
		unsigned int	length;			// 4 bytes
		char			version;		// 1 byte
		char			type;			// 1 byte
		unsigned short	payloadType;	// 2 byte

		int				deviceID;

		unsigned char   sizes [ 2 ];
	}
	NET_PACK_ALIGN ShortMsgHeader;

    
    /**
     * Device short message between devices message structure
     * with appArea space
     */
	typedef struct _ShortMsg
	{
		ShortMsgHeader	header;
		char			appArea [ ( MAX_NAMEPROPERTY + 2 ) * 2 ];
	}
	NET_PACK_ALIGN ShortMsg;
    
    NET_PACK_POP
    
    
    
    /**
     * Mediator Short message to Server and response message structure
     * Deprecated V4 protocol
     */
    typedef struct _ShortMsgPacketV4
    {
        unsigned int	size;
        char			version;
        char			ident [3];
        
        int				deviceID;
        
        /** The area name of the application environment. */
        char			areaName [MAX_NAMEPROPERTY + 1]; // 31
        
        /** The application name of the application environment. */
        char			appName [MAX_NAMEPROPERTY + 1]; // 31
        
        char			msg;
    }
    ShortMsgPacketV4;
    
    
    /**
     * Device short message between devices message structure
     * Deprecated V4 protocol
     */
	typedef struct _ShortMsgV4
	{
		char			version;
		char			deviceType;
		char			ident [2];
		int				deviceID;

		/** The area name of the application environment. */
		char			areaName [MAX_NAMEPROPERTY + 1]; // 31

		/** The application name of the application environment. */
		char			appName [MAX_NAMEPROPERTY + 1]; // 31

		char			text;
	}
	ShortMsgV4;


} /* namepace Environs */


#endif // INCLUDE_HCM_ENVIRONS_MEDIATOR_H

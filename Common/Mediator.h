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

#include "Interop/Threads.h"
#include "Interop/Sock.h"
#include "Device.Instance.h"
#include "Environs.Crypt.h"


#define USE_INTEGER_PROJECT_APP
#define USE_INTEGER_PROJECT_APP_MAPS

#define DEFAULT_MEDIATOR_PORT					5898
#define DEFAULT_BROADCAST_PORT					5899
#define	BUFFERSIZE								1024
#define MESSAGE_BUFFER_SIZE						512
#define	IP_MAX_SIZE								22

#define	MAX_MEDIATOR_PORTS						4	// We allow max. 4 ports for connections to the mediator

#define MEDIATOR_MESSAGE_UNIT_ALIGN				4
#define MEDIATOR_MESSAGE_LENGTH_FLAG_PAD		0x10000000

#define MEDIATOR_BROADCAST						"E.Mediator"
#define MEDIATOR_BROADCAST_DEVICE				"E.D."
#define MEDIATOR_SPARE_SOCKET_REQ				"E.DS"
#define MEDIATOR_BROADCAST_DEVICEINFO			"E.D.Client"
#define MEDIATOR_BROADCAST_GREET				"E.D.Helo"
#define MEDIATOR_BROADCAST_BYEBYE				"E.D.Bye"
#define MEDIATOR_PROTOCOL_VERSION_MIN			'2'
#define MEDIATOR_PROTOCOL_VERSION				'3'

#define	MEDIATOR_CMD_SET						's'
#define	MEDIATOR_CMD_GET						'g'
#define	MEDIATOR_CMD_GET_DEVICES				'd'
#define MEDIATOR_CMD_GET_DEVICES_LEN			16
#define MEDIATOR_CMD_GET_DEVICES_RESP_LEN		8
#define MEDIATOR_CMD_GET_DEVICES_COUNT_LEN		12
#define MEDIATOR_CMD_GET_DEVICES_COUNT_RESP_LEN	12
#define	MEDIATOR_CMD_HEARTBEAT					'h'
#define	MEDIATOR_CMD_GETALL						'a'
#define	MEDIATOR_CMD_SET_FILTERMODE				'f'
#define	MEDIATOR_CMD_QUIT						'q'
#define	MEDIATOR_CMD_NATSTAT					'n'
#define	MEDIATOR_CMD_SHORT_MESSAGE				'm'
#define	MEDIATOR_CMD_REQ_SPARE_ID				'r'
#define	MEDIATOR_CMD_STUNT						'x'
#define	MEDIATOR_CMD_STUN						'y'
#define	MEDIATOR_CMD_HELP_TLS_GEN				't'
#define	MEDIATOR_CMD_SESSION_ASSIGN				's'
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

#define	MEDIATOR_CMD_GET_VERSION				'v'

#define MEDIATOR_STUNT_REQ_SIZE					12
#define MEDIATOR_STUNT_REQ_EXT_SIZE             76
#define MEDIATOR_STUNT_CHANNEL_VERSATILE		';'
#define MEDIATOR_STUNT_CHANNEL_MAIN				':'
#define MEDIATOR_STUNT_CHANNEL_BULK				'.'
#define MEDIATOR_STUNT_CHANNEL_PORTAL			','
#define MEDIATOR_STUNT_RESP_SIZE				28
#define MEDIATOR_STUNT_ACK_SIZE					12
#define MEDIATOR_STUNT_ACK_EXT_SIZE				20
#define MEDIATOR_STUN_REQ_SIZE					16
#define MEDIATOR_STUN_RESP_SIZE					20

#define MEDIATOR_NAT_REQ_SIZE					12
#define MEDIATOR_MSG_VERSION_SIZE				16

#define MEDIATOR_DEVICE_RELOAD					-1
#define MEDIATOR_DEVICE_CHANGE_NEARBY			1
#define MEDIATOR_DEVICE_CHANGE_MEDIATOR			2


namespace environs	/// Namespace: environs ->
{
	typedef struct NetPack
	{
		unsigned int		ip;
		unsigned int		bcast;
		unsigned int		mask;
		NetPack			*	next;
	}
	NetPack;

	struct DeviceInstanceList;

	typedef struct _ThreadInstance
	{
		int						deviceID;
		pthread_t				threadID;
		pthread_mutex_t			accessMutex;
		unsigned int			version;
		long long				sessionID;

		long					aliveLast;

		int						socket;
		unsigned short			port;
		unsigned short			portUdp;

		struct 	sockaddr_in		addr;

		int						spareSocket;
		unsigned short			sparePort;
		unsigned short			filterMode;
		
		int						encrypt;
		int						authenticated;
		AESContext				aes;
		DeviceInstanceList	*	device;

		bool					createAuthToken;
		char					uid [ MAX_NAMEPROPERTY * 6 ];
	}
	ThreadInstance;


	typedef struct _ApplicationDevices
	{
		unsigned int			id;
		unsigned int			projId;
		pthread_mutex_t			mutex;
		int						count;
		long					access;
		
		unsigned int			latestAssignedID;

		DeviceInstanceList	*	devices;
	}
	ApplicationDevices;

    
    // "int11 1s projectNameMAX_PROP 1s appNameMAX_PROP 1e"
#define MAX_DEVICE_INSTANCE_KEY_LENGTH      (11 + MAX_NAMEPROPERTY + MAX_NAMEPROPERTY + 5)

    
	typedef struct DeviceInstanceList
	{
		DeviceInfo info;

#ifdef MEDIATORDAEMON
		ApplicationDevices		* root;
		ThreadInstance			* client;
#else
		char					  key [MAX_DEVICE_INSTANCE_KEY_LENGTH]; // "int11 1s projectNameMAX_PROP 1s appNameMAX_PROP 1e"
#endif
		char					  userName	[ MAX_NAMEPROPERTY + MAX_NAMEPROPERTY + 1 ]; // 31

		struct DeviceInstanceList	* next;  // 4
	}
    DeviceInstanceList;
    
    
    typedef struct DeviceInstanceItem
    {
        DeviceInfo info;
        
        char					  key [MAX_DEVICE_INSTANCE_KEY_LENGTH]; // "int11 1s projectNameMAX_PROP 1s appNameMAX_PROP 1e"
    }
    DeviceInstanceItem;


	typedef struct _MediatorThreadInstance
	{
		ThreadInstance			instance;
		int						socketUdp;
		pthread_t				threadIDUdp;
		pthread_t				threadIDWatchdog;
	}
	MediatorThreadInstance;


	typedef struct _MediatorConnection
	{
		ThreadInstance			instance;

		char				*	buffer;
		pthread_mutex_t			send_mutex;

		pthread_mutex_t			rec_mutex;
		pthread_cond_t			rec_signal;
		char				*	responseBuffer;

		INTEROPTIMEVAL			lastSpareSocketRenewal;
		long					renewerAccess;
	}
	MediatorConnection;


	typedef struct MediatorInstance
	{
		unsigned int			ip;
		unsigned short			port;
        
		bool					available;
		bool					listening;
		MediatorConnection		connection;
		MediatorInstance	*	next;
	}
	MediatorInstance;

	typedef void ( *MediatorCallback )(void *);


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

		bool					Init ( );
		bool					Start ( );
		bool					IsStarted ( );

		virtual void			BuildBroadcastMessage ( ) = 0;
		bool					SendBroadcast ( );

		bool					AddMediator ( unsigned int ip, unsigned short port );
        void					BroadcastByeBye ();
        
        static bool				LoadNetworks ( );
        static unsigned int		GetLocalIP ( );
        static unsigned int		GetLocalSN ( );

        static bool				InitClass ();
        static void				DisposeClass ();

	protected:
        bool					allocated;
        static bool				allocatedClass;
        
		bool					isRunning;
		char			*		certificate;

		static NetPack          localNets;
        static pthread_mutex_t  localNetsMutex;
        
#ifdef USE_INTEGER_PROJECT_APP_MAPS
        pthread_mutex_t         idMapMutex;
#endif
		pthread_mutex_t			mediatorMutex;
		MediatorInstance		mediator;

		pthread_mutex_t			devicesMutex;

		unsigned int			broadcastMessageLen;
		char					broadcastMessage [MEDIATOR_BROADCAST_DESC_START + ((MAX_NAMEPROPERTY + 2) * 6) + 4]; // 4; 12; 4; 4; 2; 2; => 24 byte; max. 50 byte for projectname
		unsigned int			greetUpdates;

		int						broadcastSocketID;
		pthread_t				broadcastThreadID;

		bool					IsLocalIP ( unsigned int ip );
		static void				VerifySockets ( ThreadInstance * inst, bool waitThread );
		static bool				IsSocketAlive ( int &sock );

		virtual void			OnStarted ( );

		DeviceInstanceList *	UpdateDevices ( unsigned int ip, char * msg, char ** uid, bool * created, char isBroadcast = 0 );

		virtual DeviceInstanceList ** GetDeviceList ( char * projectName, char * appName, pthread_mutex_t ** mutex, int ** pDevicesAvailable, ApplicationDevices ** appDevices ) = 0;

		virtual void			RemoveDevice ( unsigned int ip, char * msg ) {};
		virtual void			RemoveDevice ( DeviceInstanceList * device, bool useLock = true ) = 0;
		virtual void			UpdateDeviceInstance ( DeviceInstanceList * device, bool added, bool changed ) = 0;
		
		virtual void			ReleaseDevices ( ) = 0;

		virtual void			NotifyClientsStart ( unsigned int notify, const char * projectName, const char * appName, int deviceID ) {};
		
		MediatorInstance *		IsKnownMediator ( unsigned int ip, unsigned short port );
		MediatorInstance *		AddMediator ( MediatorInstance * med );
		virtual bool			RegisterAtMediator ( MediatorInstance * med );
		bool					RemoveMediator ( unsigned int ip );

		static void *			BroadcastThreadStarter ( void *arg );
		virtual void *			BroadcastThread ( ) = 0;

		void					Dispose ( );
		void					ReleaseMediators ( );
		void					ReleaseMediator ( MediatorInstance * med );
        bool					ReleaseThreads ( );
        
        static void				ReleaseNetworks ( );

	private:
        static void				AddNetwork ( NetPack * &pack, unsigned int ip, unsigned int bcast, unsigned int netmask );
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


	typedef struct _MediatorReqMsg
	{
		unsigned int	size;
		char			cmd0;
		char			cmd1;
		char			opt0;
		char			opt1;

		/** The project name of the appliction environment. */
		char			projectName [MAX_NAMEPROPERTY + 1]; // 31

		/** The applcation name of the appliction environment. */
		char			appName [MAX_NAMEPROPERTY + 1]; // 31
        
		/** The applcation name of the appliction environment. */
		char			deviceUID [MAX_NAMEPROPERTY * 6]; // 31
	}
	MediatorReqMsg;


	typedef struct _MediatorGetPacket
	{
		unsigned int	size;
		char			version;
		char			cmd;
		char			opt0;
		char			opt1;
	}
	MediatorGetPacket;


	typedef struct _MediatorNotify
	{
		unsigned int	size;
		char			cmd0;
		char			cmd1;
		char			opt0;
		char			opt1;

		unsigned int	msgID;
		int				notifyDeviceID;

		/** The project name of the appliction environment. */
		char			projectName [MAX_NAMEPROPERTY + 1]; // 31

		/** The applcation name of the appliction environment. */
		char			appName [MAX_NAMEPROPERTY + 1]; // 31
	}
	MediatorNotify;


	typedef struct _MediatorQueryMsg
	{
		unsigned int	size;
		char			cmdVersion;
		char			cmd1;
		char			opt0;
		char			opt1;

		unsigned int	msgID;
		unsigned int	startIndex;
		int				deviceID;

		/** The project name of the appliction environment. */
		char			projectName [MAX_NAMEPROPERTY + 1]; // 31

		/** The applcation name of the appliction environment. */
		char			appName [MAX_NAMEPROPERTY + 1]; // 31
	}
	MediatorQueryMsg;


	typedef struct _MediatorQueryResponse
	{
		unsigned int	size;
		char			cmd0;
		char			cmd1;
		char			opt0;
		char			opt1;

		DeviceHeader	deviceHead;
	}
	MediatorQueryResponse;


	typedef struct _SpareSockRegPack
	{
		unsigned int	alignPad;
		unsigned int	sizeEncrypted;
		unsigned int	sizeReq;

		char			ident [4];
		long long		sessionID;
		unsigned int	sizePayload;
		char			payload;
	}
	SpareSockRegPack;


	typedef struct _SpareSockDecPack
	{
		unsigned int	sizeReq;

		char			ident [4];
		long long		sessionID;
		unsigned int	sizePayload;
		char			payload;
	}
	SpareSockDecPack;


	typedef struct _STUNTReqPacket
	{
		unsigned int	size;
		char			version;
		char			ident [2];
		char			channel;

		int				deviceID;
        
		/** The project name of the appliction environment. */
		char			projectName [MAX_NAMEPROPERTY + 1]; // 31
        
		/** The applcation name of the appliction environment. */
		char			appName [MAX_NAMEPROPERTY + 1]; // 31

		char			pad [2];
	}
	STUNTReqPacket;


	typedef struct _STUNTRespPacket
	{
		unsigned int	size;
		char			respCode;
		char			ident [2];
		char			channel;

		unsigned short	port;
		unsigned short	portUdp;
		unsigned int	ip; // 12
		unsigned int	ipe; // 16
	}
	STUNTRespPacket;

	typedef struct _STUNTRespReqPacket
	{
		unsigned int	size;
		//char			respCode;
		char			ident [3];
		char			channel;

		int				deviceID;
		unsigned int	ip;
		unsigned int	ipe;
		unsigned int	port;

		/** The project name of the appliction environment. */
		char			projectName [MAX_NAMEPROPERTY + 1]; // 31

		/** The applcation name of the appliction environment. */
		char			appName [MAX_NAMEPROPERTY + 1]; // 31
		unsigned int	pad0;
	}
	STUNTRespReqPacket;


	typedef struct _STUNReqPacket
	{
		char			ident [4];

		unsigned int	sourceID;
		unsigned int	destID;

		/** The project name of the appliction environment. */
		char			projectName [MAX_NAMEPROPERTY + 1]; // 31

		/** The applcation name of the appliction environment. */
		char			appName [MAX_NAMEPROPERTY + 1]; // 31

		/** The project name of the appliction environment. */
		char			projectNameSrc [MAX_NAMEPROPERTY + 1]; // 31

		/** The applcation name of the appliction environment. */
		char			appNameSrc [MAX_NAMEPROPERTY + 1]; // 31
	}
	STUNReqPacket;


	typedef struct _STUNReqReqPacket
	{
		unsigned int	size;
		char			ident [4];

		int				deviceID;
		unsigned int	IP;
		unsigned int	Port;

		/** The project name of the appliction environment. */
		char			projectName [MAX_NAMEPROPERTY + 1]; // 31

		/** The applcation name of the appliction environment. */
		char			appName [MAX_NAMEPROPERTY + 1]; // 31
	}
	STUNReqReqPacket;

	
	typedef struct _STUNTRegReqPacket
	{
		unsigned int	size;
		char			version;
		char			ident [3];

		unsigned int	notify;
		unsigned int	pad0;
	}
	STUNTRegReqPacket;


	typedef struct _UdpHelloPacket
	{
		char			ident [3];
		char			version;

		int				deviceID;

		/** The project name of the appliction environment. */
		char			projectName [MAX_NAMEPROPERTY + 1]; // 31

		/** The applcation name of the appliction environment. */
		char			appName [MAX_NAMEPROPERTY + 1]; // 31
	}
	UdpHelloPacket;


	typedef struct _UdpEncHelloPacket
	{
		unsigned int	size;
		unsigned int	alignPad;
		long long		sessionID;
		char			aes;
	}
	UdpEncHelloPacket;


	typedef struct _ShortMsgPacket
	{
		unsigned int	size;
		char			version;
		char			ident [3];

		int				deviceID;

		/** The project name of the appliction environment. */
		char			projectName [MAX_NAMEPROPERTY + 1]; // 31

		/** The applcation name of the appliction environment. */
		char			appName [MAX_NAMEPROPERTY + 1]; // 31

		char			msg;
	}
	ShortMsgPacket;


	typedef struct _ShortMsg
	{
		char			version;
		char			deviceType;
		char			ident [2];
		int				deviceID;

		/** The project name of the appliction environment. */
		char			projectName [MAX_NAMEPROPERTY + 1]; // 31

		/** The applcation name of the appliction environment. */
		char			appName [MAX_NAMEPROPERTY + 1]; // 31

		char			text;
	}
	ShortMsg;


} /* namepace Environs */


#endif // INCLUDE_HCM_ENVIRONS_MEDIATOR_H

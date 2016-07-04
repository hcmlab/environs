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
#pragma once
#ifndef INCLUDE_HCM_ENVIRONS_MEDIATOR_CLIENT_H
#define INCLUDE_HCM_ENVIRONS_MEDIATOR_CLIENT_H

#include "Mediator.h"
#include <map>
#include <cstring>


#define MEDIATOR_CLIENT_MAX_BUFFER_SIZE_MAX	(MEDIATOR_CLIENT_MAX_BUFFER_SIZE >> 2)


namespace environs
{
	extern const char * getChanelDescriptor ( char channel );

#define	getChannel()	getChanelDescriptor(channel)

    bool IsSameAppEnv ( Instance * env, const char * appName, const char * areaName );
    
#ifndef _WIN32
	extern void IncreaseThreadPriority ( const char * name );
#else
#	define IncreaseThreadPriority(name)
#endif


	typedef struct _DeviceChangePacket
	{
		int							notification;
        int							deviceID;
        
        struct _DeviceChangePacket	* next;
        
        char			pads [2];
        unsigned char   sizes [2];
        
        char			appArea;
	}
	DeviceChangePacket;

#ifdef USE_MEDIATOR_OPT_KEY_MAPS_COMP
	struct compare_char_key : public std::binary_function<APPAREATYPE *, APPAREATYPE  *, bool>
	{
	public:
		bool operator() ( APPAREATYPE * left, APPAREATYPE * right ) const
		{
#ifndef NDEBUG
            if ( !left || !right ) {
#if (defined(ENVIRONS_CORE_LIB1) && !defined(NDEBUG))
				CErrArg ( "compare_char_key: Invalid left [%s] or right [%s]", left ? "ok" : "!", right ? "ok" : "!" );
#endif
				//_EnvDebugBreak ();
			}
            if ( strlen ( left->appArea ) <= 0 || strlen ( right->appArea ) <= 0 ) {
#if (defined(ENVIRONS_CORE_LIB1) && !defined(NDEBUG))
				CErrArg ( "compare_char_key: Empty left [%s] or right [%s]", left->appArea, right->appArea );
#endif
			}
			//WARNING ( "Should be removed!!!! Only for debug runs!" )
#endif
			if ( left->deviceID < right->deviceID )
				return true;
			else if ( left->deviceID > right->deviceID )
				return false;
			return ( strncmp ( left->appArea, right->appArea, MAX_NAMEPROPERTY ) < 0 );
		}
	};
#else

#define CLASS_NAME "MediatorClient"

	struct compare_char_key : public std::binary_function<const char*, const char*, bool>
	{
	public:
		bool operator() ( char const * left, char const * right ) const
		{
			if ( !left || !right ) {
#if (defined(ENVIRONS_CORE_LIB) && defined(NDEBUG))
				CErrArg ( "compare_char_key: Invalid left [%s] or right [%s]", left ? "ok" : "!", right ? "ok" : "!" );
#endif
				return false;
			}

#if (defined(ENVIRONS_CORE_LIB) && defined(NDEBUG))
			if ( strlen ( left ) <= 0 || strlen ( right ) <= 0 ) {
				CErrArg ( "compare_char_key: Empty src [%s] or dst [%s]", left, right );
			}
				//WARNING ( "Should be removed!!!! Only for debug runs!" )
#endif
			//return std::strcmp ( left, right ) < 0;
			return ( strncmp ( left, right, MAX_NAMEPROPERTY ) < 0 );
		}
    };
#endif
    
    class RegisterStuntThread : public ThreadSync
    {
    public:
        int                     deviceID;
        char                    channel;
        unsigned int            token;
        char                    keyID [ 36 + sizeof ( AppAreaBuffer ) ];
        char                    buffer [ MEDIATOR_BROADCAST_DESC_START + ( MAX_NAMEPROPERTY * 4 ) + sizeof ( StuntSockRegTarget ) + sizeof ( AppAreaBuffer ) ];
        
        sp ( MediatorClient )   mediator;
        MediatorInstance    *   med;
        int                 *   s;
        struct sockaddr_in  *   addr;
        sp ( StuntRegisterContext ) ctxSP;
        sp ( RegisterStuntThread ) aliveSP;

        
        RegisterStuntThread () : deviceID ( 0 ), channel ( 0 ), token ( 0 ), med ( nill ), s ( nill ), addr ( nill ) {
            *keyID = 0;
#ifndef NDEBUG
            Zero ( buffer );
#endif
        };
        
        ~RegisterStuntThread () {
        };
        
        
        bool Set ( MediatorInstance * mi, const sp ( StuntRegisterContext ) &ctx, int * sock, struct sockaddr_in * a, int devID, char c, unsigned int t )
        {
            if ( !mi || !ctx || !sock ) return false;
            
            med = mi; ctxSP = ctx; s = sock; deviceID = devID; channel = c; token = t; addr = a;
            
            return true;
        }
        
        bool Init ( const sp ( MediatorClient ) &m )
        {
            if ( !m ) return false;
            
            mediator = m;
            
            return ThreadSync::Init ();
        }
    };
    
#undef CLASS_NAME

	class DeviceBase;

	class MediatorClient : public Mediator
	{
		friend              class StunRequest;
		friend				class StunTRequest;
		friend				class DeviceBase;
        friend              class NotificationQueue;
        friend              class AsyncWorker;
        friend				class Core;

	public:
		MediatorClient ();
		~MediatorClient ( );

		unsigned int        connectFails;

        bool				Init ( const sp ( Instance ) & obj );
		static bool			InitInstance ();

		void				Stop ( bool wait );

		bool				StartAliveThread ();
		void				StopAliveThread ( bool wait );

		void				BuildBroadcastMessage ( bool withStatus = true );
        
        bool				RegisterAtMediators ( bool wait );

		bool				IsServiceAvailable ();
        bool				IsRegistered ( );

		bool				SendMessageToMediator ( void * msgResp, bool withResponse, unsigned int returnMaxSize, int msTimeout = 0 );

		bool				SendMessageToDevice ( int deviceID, const char * areaName, const char * appName, void * sendBuffer, int bufferSize );

        static unsigned int	GetLocalIPe ( );
        bool				GetNATStat ( );
        bool				GetMediatorServiceVersion ( unsigned int &version, unsigned int &revision );
		bool				GetMediatorLocalEndpoint ( MediatorInstance * med, struct sockaddr_in * addr );

		bool				IsIPInSameNetwork ( unsigned int ip );
		bool				IsDeviceInSameNetwork ( int deviceID, const char * areaName, const char * appName );

#ifdef __cplusplus
        sp ( DeviceInstanceNode ) GetDeviceNearbySP ( int deviceID, const char * areaName, const char * appName, bool useLock = true );
        
        sp ( DeviceInstanceNode ) GetDeviceSP ( int deviceID, const char * areaName, const char * appName, int * success = 0, bool useLock = true );
        
        static sp ( DeviceInstanceNode ) GetDeviceSP ( int hInst, int objID );        
        
        sp ( DeviceInstanceNode ) BuildDeviceListItem ( DeviceInfo * device, bool copy );
        
        sp ( DeviceInstanceNode ) GetDeviceSP ( int objID );
#endif
		int					GetDevicesBroadcast ( char * buffer, int bufferSize, int startIndex, int deviceID = 0, const char * areaName = 0, const char * appName = 0 );
        
        /**
         * Query the number of Mediator managed devices within the environment.
         *
         * @return numberOfDevices (or -1 for error)
         */
		int					GetDevicesFromMediatorCount ( );
		int					GetDevicesFromMediator ( char *& buffer, int deviceID = 0, const char * areaName = 0, const char * appName = 0 );
        
        /**
         * Query the number of Mediator managed devices within the environment from cached list.
         *
         * @return numberOfDevices (or -1 for error)
         */
		int					GetDevicesFromMediatorCountCached ();
		int					GetDevicesFromMediatorCached ( char * buffer, int bufferSize, int startIndex );
		int					GetDeviceFromMediatorCached ( char * buffer, int bufferSize, int deviceID, const char * areaName, const char * appName );

        int					GetDevicesAvailableCachedBestMatch ( char ** buffer, int bufferSize, int deviceID );
        int					GetDevicesAvailableCached ( char ** buffer, int bufferSize, OBJIDType objID );

		int					GetDevicesNearbyCount ( );
		int					GetDevicesNearby ( char * buffer, int bufferSize, int startIndex, int deviceID = 0, const char * areaName = 0, const char * appName = 0 );

		int					GetDevicesAvailableCount ( );

		int					GetDevicesAvailableCountCached ();
		int					GetDevicesAvailableCached ( char * &buffer );
        
        static bool			SocketKeepAlive ( int sock, int opt );
		static bool			SendUDPFIN ( int sock );

        static int			Receive ( int &sock, char * buffer, unsigned int bufferSize, unsigned int minReceiveBytes, const char * receiver );
        static int			ReceiveOneMessage ( bool encrypt, AESContext * aes, int &sock, char * buffer, unsigned int bufferSize, char *& decrypted );
		
		bool				SetParam ( int deviceID, const char * areaName, const char * appName, const char * key, const char * value );

		bool                GetParam ( const char * areaName, const char * appName, const char * key, char * buffer, unsigned int bufferSize );
		bool                GetParam ( int deviceID, const char * areaName, const char * appName, const char * key, char * buffer, unsigned int bufferSize );

		bool				GetPortTCP ( int deviceID, const char * areaName, const char * appName, int &value );
		bool				GetPortUDP ( int deviceID, const char * areaName, const char * appName, int &value );
		bool				GetIP ( int deviceID, const char * areaName, const char * appName, unsigned int &value );
        bool				GetIPe ( int deviceID, const char * areaName, const char * appName, unsigned int &value );
        bool				GetConnectionDetails ( int deviceID, const char * areaName, const char * appName, volatile DeviceStatus_t * deviceStatus, unsigned int &ip, int &portTcp, unsigned int &ipe, int &portUdp );

		bool				GetIntParam ( int deviceID, const char * areaName, const char * appName, const char * key, int &value );
		bool                GetStringParam ( int deviceID, const char * areaName, const char * appName, const char * key, char * buffer, unsigned int bufferSize );
        
        bool				RequestSTUNT ( volatile DeviceStatus_t * deviceStatus, int deviceID, const char * areaName, const char * appName, unsigned short &port, unsigned int &ip, unsigned short &porte, unsigned int &ipe, struct sockaddr_in * addr, char channelType, unsigned int token );

		bool				RequestSTUN ( int deviceID, const char * areaName, const char * appName, int sock );

		void                SetDeviceFlags ( DeviceInstanceNode * device, bool set );

        void                SetNotificationSubscription ( bool enable );
        bool                GetNotificationSubscription ();
        
        void                SetMessagesSubscription ( bool enable );
        bool                GetMessagesSubscription ();
        
        void				UpdateDeviceState ( DeviceBase * deviceBase, int nativeID );
        
        bool				HasMediatorCredentials ( MediatorInstance * med );
        
	private:
        bool                allocatedClient;

		sp ( Instance )		envSP;

        static unsigned int IPe;
        static bool         behindNAT;
        
        ThreadSync          sendThread;
        int                 sendThreadRestarts;
        bool                sendThreadAlive;
		bool				sendThreadDisposeContexts;
        
        bool				StartSendThread ();
        void				StopSendThread ();
        static void		*	SendThreadStarter ( void * arg );
        void				SendThread ();
        int                 SendBuffer ( ThreadInstance * client, SendContext * ctx );
        int                 SendBufferNoQueue ( ThreadInstance * client, void * msg, unsigned int size );
        
#ifdef USE_MEDIATOR_TRYSEND_BEFORE_ENQUEUE
        bool                SendBufferOrEnqueue ( ThreadInstance * client, void * msg, unsigned int size );
#else
#   define SendBufferOrEnqueue(c,m,s,n)   PushSend ( c, m, s,n )
        int                 PushSend ( void * msg, unsigned int size, unsigned int seqNr = 0 );
#endif
        
        void                SendContextsCompress ( lib::QueueVector * queue );

        bool                PushSend ( ThreadInstance * client, char * toSend, unsigned int toSendSize, unsigned int toSendCurrent, bool copy );
        
        bool                PushSend ( ThreadInstance * client, void * buffer, unsigned int size, unsigned int seqNr );
        
        lib::QueueVector	sendQueue;
        pthread_mutex_t		sendQueueLock;
        
        int                 SendBufferBC ( SendContext * ctx );
        bool                SendBufferOrEnqueueBC ( bool copy, void * buffer, unsigned int size, unsigned int ip, unsigned short port );
        bool                PushSendBC ( bool copy, void * buffer, unsigned int size, unsigned int ip, unsigned short port );
        
        void                DisposeSendContextsBC ();
        
        void                SyncDeviceUpdateFlags ( DeviceInstanceNode * device, bool set );
        void                HandleDeviceUpdateMessage ( char * msg );
        
		DeviceInstanceNode	*	devices;
		int					devicesAvailable;
		pthread_mutex_t     devicesMapLock;
		int                 devicesMapUpdates;
		pthread_mutex_t     devicesCacheLock;

        unsigned int        registerFails;
        bool                subscribedToNotifications;
        bool                subscribedToMessages;
		
		bool				IsConnectAllowed ( int deviceID, const char * appName, const char * areaName );

		void				ReleaseDevices ( );
		void				RemoveDevice ( unsigned int ip, char * msg );
		void				RemoveDevice ( DeviceInstanceNode * device, bool useLock = true );

#ifdef ENABLE_EXT_BIND_IN_STUNT
        static unsigned int primaryInterface;
        
        void                MatchExtIPWithInterfaces ();
        void                BindSocketToLocalInterface ( int sock );
#endif
        
#ifdef __cplusplus
		void				UpdateDeviceInstance ( const sp ( DeviceInstanceNode ) & device, bool added, bool changed );
#endif

		MediatorInstance *	GetAvailableMediator ();
		sp ( ApplicationDevices ) GetDeviceList ( char * areaName, char * appName, pthread_mutex_t ** lock, int ** pDevicesAvailable, DeviceInstanceNode ** &list  );

		void				OnStarted ( );
        void			*	BroadcastThread ( );
		bool                SendData ( DeviceInstanceNode * device, char * data, int dataSize );

        bool				InformMediator ( MediatorInstance * med );

		MediatorInstance *	HandleMediatorMessage ( unsigned int ip, char * msg );

		bool				CommitSend ( MediatorInstance * med, char * msgResp, bool withResponse, unsigned int returnMaxSize, int msTimeout = 0 );

        void                ApplyAnonymousCredentials ();
        void                RequestLogonCredentials ();
        bool				RegisterAtMediator ( MediatorInstance * med );
		bool				ConnectToMediator ( MediatorInstance * med );
		bool				RegisterAuth ( MediatorInstance * med );
        void                SetFilterMode ( MediatorInstance * med );
        
		LONGSYNC			secureChannelAuth;
		bool				SecureChannelAuth ( MediatorInstance * med );
        bool                CheckCertificateAndKey ( MediatorInstance * med );
		void                HandleCertificateResponse ( MediatorMsg * msg, unsigned int msgSize );
        
		pthread_mutex_t		registerLock;

        bool                RegisterStuntSocket ( bool invokeThread, MediatorInstance * med, int deviceID, const char * appName, const char * areaName, struct sockaddr_in * addr, char channelType, unsigned int token, bool isMediatorRequest );
        
        
        bool                RegisterStuntSocketDo ( RegisterStuntThread * stuntCtx );
        
        pthread_mutex_t		stuntThreadsLock;
        msp ( void *, RegisterStuntThread ) stuntThreads;
        
        static void		*	StuntThread ( void * arg );
        
        void				StuntThreadCreate ( MediatorInstance * med, STUNTRegReqPacketV8 * req );

        void                StuntThreadsDispose ();
        
		void				RemoveStuntDevice ( DeviceBase * device );

		static void		*	MediatorCallbackStarter ( void * arg );
        
        int                 RequestDeviceID ( MediatorInstance * med );
        
        sp ( Instance )     registratorSP;
        LONGSYNC            registratorState;
        pthread_t			registratorThreadID;
        static void		*	MediatorRegistrator ( void *arg );

        bool				RegisterAtMediatorsDo ( );

        static void		*	MediatorListenerStarter ( void *arg );
		virtual void	*	MediatorListener ( void *arg );

        ThreadSync          aliveThread;
        
        int                 aliveThreadRestarts;
		static void		*	AliveThreadStarter ( void * object );
		void				AliveThread ( );

		// DeviceList cache
		std::map<APPAREATYPE *, DeviceInstanceNode *, compare_char_key>  *   devicesMapAvailable;

		char			*	deviceAvailableCached;
		bool				deviceAvailableCacheDirty;
		int					deviceAvailableCachedCount;

		char			*	deviceMediatorCached;
		bool				deviceMediatorCacheDirty;
		int					deviceMediatorCachedCount;

		void				UpdateDirtyFlags ( bool available, bool mediator );
		bool				UpdateDirtyCaches ( bool available, bool mediator );

		int					deviceMediatorQueryCount;

		bool				DevicesMediatorClear ( bool useLock );
		bool				DevicesMediatorReload ();

		bool				DevicesAvailableReload ();
		bool				GC_DevicesAvailable ();

		bool                DevicesCacheRebuild ( bool withMediator, bool useLock );


		void 				DevicesHasChanged ( int type );
		int					deviceChangeIndicator;

		DeviceChangePacket*	deviceChangePackets;
		DeviceChangePacket*	deviceChangePacketsEnd;

        int                 DeviceCompareAndTakeOver ( DeviceInstanceNode * listDevice, lib::DeviceInfo * device, bool notify );
		bool				DevicePacketUpdater ();
		bool				DeviceListsUpdate ( DeviceChangePacket * packet );

		int					DeviceRemove ( DeviceChangePacket * packet );
		int					DeviceRemove ( APPAREATYPE * key, bool mediatorRequest, bool notify = true );

		int					DeviceChange ( DeviceInstanceNode * nearbyDevice, lib::DeviceInfo * mediatorDevice );
		int					DeviceChange ( std::map<APPAREATYPE *, DeviceInstanceNode *, compare_char_key>::iterator &foundIt, DeviceInstanceNode * nearbyDevice, lib::DeviceInfo * mediatorDevice );

		int					DeviceAdd ( DeviceInstanceNode * nearbyDevice, lib::DeviceInfo * mediatorDevice );
	};

}	// namespace environs


#endif	// INCLUDE_HCM_ENVIRONS_MEDIATOR_CLIENT_H



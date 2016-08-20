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
#pragma once
#ifndef INCLUDE_HCM_ENVIRONS_DEVICEBASE_H
#define INCLUDE_HCM_ENVIRONS_DEVICEBASE_H

#ifdef __cplusplus


#include "Human.Input.Decl.h"
#include "Device.Display.Decl.h"
#include "Portal.Info.Base.h"
#include "Portal.Stream.Options.h"
#include "Core/Stunt.Request.h"
#include "Decoder/Decoder.Base.h"
#include "Portal/Portal.Generator.h"
#include "Portal/Portal.Receiver.h"
#include "Portal/Portal.Device.h"
#include "Environs.Msg.Types.h"
#include "Environs.Crypt.h"
#include "Interop/Smart.Pointer.h"
#include <map>

#define	DATASTORE_PATH_APPEND_LEN	512


//#define ENABLE_DEVICEBASE_SINGLE_COMDAT_THREAD
#ifdef _WIN32
//#	define ENABLE_DEVICEBASE_WINSOCK_SINGLE_COMDAT_THREAD
#endif

#ifdef ENABLE_DEVICEBASE_WINSOCK_SINGLE_COMDAT_THREAD
#	define FDTYPE	HANDLE
#else
#	define FDTYPE	struct pollfd
#endif

namespace environs 
{
    class DeviceBase;
	class Instance;
    struct DeviceInstanceNode;
    
    typedef bool ( DeviceBase::*MessageHandler )( environs::lib::ComMessageHeader * header, bool isComDatChannel );
    typedef void ( DeviceBase::*PortalHandler )( int portalID );
    
    
    
	/**
	*	Base functionality common for all devices (mobile, display, surface, ...)
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@version	1.0
	*	@remarks	Device specific classes derive from this class and extends appropriately.
	* ****************************************************************************************
	*/
	class DeviceBase
	{
        friend class Core;
        friend class NotificationQueue;
        friend class MediatorClient;
        friend class EnvironsNative;
        friend class AsyncWorker;
		friend class PortalGenerator;
		friend class StunTRequest;
		friend class StunRequest;

		/*
		 * State members
		 */
	public:
		LONGSYNC					accessLocks;
        volatile DeviceStatus_t     deviceStatus;
        unsigned int                connectTime;
        
        unsigned int                connectToken;

        int							nativeID;
#ifndef NDEBUG
		int							nativeIDTrace;
#endif
        
        sp ( DeviceInstanceNode )   deviceNode;
        
		int							deviceID;
		char 				*		deviceAppName;
		char 				*		deviceAreaName;

		PortalStreamOptions			streamOptions;

        Platforms_t					platform;
        Instance				*	env;

		pthread_mutex_t				spLock;
        
#ifdef ENABLE_DEVICEBASE_WEAK_REFERENCE
        wp ( DeviceController )     myself;
        sp ( DeviceController )     myselfSP;
#else
        sp ( DeviceController )     myself;
#endif
		DeviceDisplay				display;
		int							width_coverage;
		int							height_coverage;

        WNDHANDLE					portalhWnd;

		struct 	sockaddr_in			udpAddr;
        
		char					*	udpBuffer;

		static unsigned int			packetSize;
        int							receiveBufferSize;

        pthread_mutex_t				portalMutex;
        pthread_mutex_t				portalReceiversMutex;

		unsigned char				portalReceiversCount;
		unsigned char				portalReceiversLastAssigned;
		PortalReceiver			*	portalReceivers			[ MAX_PORTAL_STREAMS_A_DEVICE ];
		int							portalReceiversDevice	[ MAX_PORTAL_STREAMS_A_DEVICE ];

		unsigned char				portalGeneratorsCount;
		unsigned char				portalGeneratorsLastAssigned;
		PortalGenerator			*	portalGenerators		[ MAX_PORTAL_STREAMS_A_DEVICE ];
		int							portalGeneratorsDevice	[ MAX_PORTAL_STREAMS_A_DEVICE ];
		int							portalGeneratorsDeviceInput;
        
        unsigned int                sensorSender;

	protected:
        bool                        disposed;
        sp ( Instance )             envSP;

		unsigned int				activityStatus;
		
		bool						behindNAT;
		unsigned short				comPort;
		unsigned short				dataPort;

		int							hasPhysicalContact; // with a stationary device, such as a surface table

		int							lastSensorFrameSeqNr;
		unsigned int				packetSequence;

		ThreadSync                  connectThread;
		LONGSYNC					connectThreadState;
        
		pthread_mutex_t				interactSocketLock;
		int							interactSocket;
		int							interactSocketForClose;
		struct 	sockaddr_in			interactAddr;
		ThreadSync                  interactThread;
        
        pthread_mutex_t				comDatSocketLock;
		int							comDatSocket;
		int							comDatSocketForClose;
		struct 	sockaddr_in			comDatAddr;
		ThreadSync                  comDatThread;

		int							udpSocket;
		int							udpSocketForClose;
        ThreadSync                  udpThread;		
        
        MessageHandler              msgHandlers [ MSG_TYPE_MAX_COUNT ];

        FILE                    *   lastFile;
        int                         lastFileID;
        int                         lastFilePart;
		PortalInfoBase				portalInfoOff;

        PortalHandler               portalHandlers [ MSG_PORTAL_MAX_COUNT ];
		
		char				*		dataStorePath;
		unsigned short              dataStorePathLen;
		unsigned short              dataStorePathRemainingLen;
		char				*		dataStorePathForRequests;
		        

	private:        
		LONGSYNC					dbgAlive;

        bool                        allocated;
        
		char						encrypt;
		AESContext					aes;
        char                    *   aesBlob;

        int                         connectStatus;
        bool                        stuntRedundant;

#ifdef ENABLE_DEVICEBASE_WP_STUNT
		wp ( StunTRequest )         stuntInteract;
#else
        sp ( StunTRequest )         stuntInteract;
#endif
        LONGSYNC					stuntInteractState;

#ifdef ENABLE_DEVICEBASE_WP_STUNT
		wp ( StunTRequest )         stuntComDat;
#else
        sp ( StunTRequest )         stuntComDat;
#endif
        LONGSYNC					stuntComDatState;        
        
#ifdef ENABLE_DEVICEBASE_WP_STUN
		wp ( StunRequest )          stun;
#else
        sp ( StunRequest )          stun;
#endif
        LONGSYNC					stunState;
        bool                        udpCoreConnected;
        
        short                       interactThreadState;
        char    *                   interactBuffer;
        unsigned int                interactBufferSize;
                
#ifdef ENABLE_DEVICEBASE_SINGLE_COMDAT_THREAD
        bool                        comDatListen;

#ifdef ENABLE_DEVICEBASE_WINSOCK_SINGLE_COMDAT_THREAD
		HANDLE						comDatEvent;
#endif

		ByteBuffer *                comDatByteBuffer;
        char *                      comDatBuffer;
        unsigned int                comDatBufferSize;

        char 	*                   comDat_Start;
        char	*                   comDat_BufferEnd;
        char 	*                   comDat_CurrentEnd;
#endif

		/*
		 * Methods
		 */
	public:
		DeviceBase ();
		virtual ~DeviceBase();
        virtual bool                Init ( const sp ( Instance ) &envObj, const char * areaName, const char * appName );
        
        void                        Dispose ();
        void                        PreDispose ();
		bool						IsPreDisposed ();
        virtual void				DisposePlatform ();

#ifdef ENABLE_DEVICEBASE_WP_STUNT
		void						CloseStunt ( const wp ( StunTRequest ) &req, bool disposeDevice, LONGSYNC * state, const char * channel, bool wait = true );
#else
        void						CloseStunt ( const sp ( StunTRequest ) &req, bool disposeDevice, LONGSYNC * state, const char * channel, bool wait = true );
#endif	
		static bool					InitDeviceStorage ( Instance * env, int deviceID, const char * areaName, const char * appName, char *&dest, unsigned short &length, unsigned short &remainLength, char *&destRequests, bool assingNewBuffers, bool buildReqPath );
		
		void						SetDirectContactStatus ( bool hasContact );
		virtual bool				GetDirectContactStatus ( );
        
        /**
         * Get the status, whether the device (id) has established an active portal
         *
         * @param	portalType  0 = Any type, or PORTAL_DIR_INCOMING, PORTAL_DIR_OUTGOING
         * @return	success 	true = yes, false = no
         */
		virtual bool				GetPortalEnabled ( int portalType );

		/**
		* Find a free portalID slot for the direction encoded into the given portalDetails.
		*
		* @param	portalDetails	Required PORTAL_DIR_INCOMING or PORTAL_DIR_OUTGOING
		*
		* @return	portalID 		The portal ID with the free id slot encoded in bits 0xFF.
		*/
		int							GetPortalIDFreeSlot ( int portalDirection );
        
        /**
         * Get the portalID of the first active portal that matches the given portalType.
         *
		 * @param	portalType  0 = Any type, or PORTAL_DIR_INCOMING, PORTAL_DIR_OUTGOING
		 * @return	portalID 	The portal ID.
         */
        virtual int                 GetPortalID ( int portalType );
        
		virtual bool				GetActiveStatus ();

		void						GetAlignedDimensions ( int &width, int &height );

		bool						SendComDatData ( int fileID, const char * buffer, unsigned int length );
		bool						LoadFromStorage ( int fileID, char * buffer, int * capacity );
		const char *				GetFilePath ( int fileID );

		static char *				GetFilePath ( Instance * env, int deviceID, const char * areaName, const char * appName, unsigned short &pathLen );

		bool 						UdpSendHelo ( );

		static bool					SendMessage ( int hInst, volatile DeviceStatus_t * deviceStatus, int deviceID, const char * areaName, const char * appName, const char * message, unsigned int length );
        
        static bool					SendFile ( int nativeID, int fileID, const char * fileDescriptor, const void * file );
        static bool					SendBuffer ( int nativeID, int fileID, const char * fileDescriptor, const char * buffer, size_t size );

		static bool					SendPushNotification ( Instance * env, int deviceID, const char * areaName, const char * appName, const char * message );

		static bool					SendHeaderedBuffer ( Instance * env, int deviceID, const char * areaName, const char * appName, void * message, unsigned int size );

		bool						SendPortalMessage ( unsigned short portalMessage, int typeID );
        
        bool						SendDataPacket ( const char * msg, unsigned int length );

		void						SendTcpFin ( int sock );
        
        bool                        SendPing ();

		void						SendUdpPortal ( unsigned short payloadType, int portalID, int frameCounter, char * prefix, int prefixSize, char * payload, int packetSize );
		bool						SendTcpPortal ( unsigned short payloadType, int portalID, int frameCounter, char * prefix, int prefixSize, char * payload, int packetSize );

		int							SendBuffer ( bool comDat, char msgType, int fileID, const char * fileDescriptor, unsigned short payloadType, const void * payload, int length );
		int							SendBufferInParts ( bool comDat, char msgType, int fileID, const char * fileDescriptor, unsigned short payloadType, const void * payload, int length );

        INTEROPTIMEVAL              lastPortalUpdate;
        virtual bool                SetPortalInfo ( void * portalInfo, bool preventFrameOverflow = true );
        
        virtual bool                SetPortalInfoPosibble ( bool updateCounter = false );
        
        bool						ProvidePortal ( int portalID );

        bool                        IsConnectingValid ();
        
		/**
		* Connect to device with the given ID.
		*
		* @param deviceID	Destination device ID
		* @return	0 Connection can't be conducted (maybe environs is stopped or the device ID is invalid)
		* @return	1 A connection to the device already exists or a connection task is already in progress)
		* @return	2 A new connection has been triggered and is in progress
		*/
		static int					ConnectToDevice ( int hInst, int Environs_CALL_, int deviceID, const char * areaName, const char * appName );

        bool                        SendTcpBuffer ( bool comDat, char msgType, unsigned short portalMessage, void * data, int size );
        static bool 				SendTcpBuffer ( int nativeID, bool comDat, char msgType, unsigned short portalMessage, void * data, int size );
		static bool					SendMessageToDevice ( Instance * env, volatile DeviceStatus_t * deviceStatus, int deviceID, const char * areaName, const char * appName, void * sendBuffer, unsigned int bufferLength );
		
		virtual void				PerformEnvironsTouch ( Input * pack ) = 0;

		virtual bool				GetPortalInfo ( PortalInfoBase * info, bool useLock = true );

		virtual void				UpdateAngle ( int portalID, float orientation );
		virtual void				UpdatePosition ( int portalID, int x, int y, float orientation, bool preventFrameOverflow = true );
		virtual void				UpdatePortalSize ( int portalID, int width, int height, bool updateAll = true, bool preventFrameOverflow = true );

		bool						CreatePortalReceiver ( int portalID );
		virtual void				CreatePortalReceiverPlatform ( int portalIDent ) = 0;
        
		void						DisposePortal ( int portalID );

		void						EnqueueToReceiverStream ( void * payload, unsigned int size, unsigned short type, int portalID );
        
	protected:
		// Accessed by connection accept listener to handle a new connection request
		static bool					HandshakeAndResponse ( Instance * env, int sock, struct sockaddr_in * addr );
		static bool					HandshakeComDatChannel ( Instance * env, bool isStunt, int & sock, struct sockaddr_in * addr, char * buffer, int &bytesRead, AESContext * aes );
		static bool					HandshakeShortMessage ( Instance * env, environs::lib::ComMessageHeader * headeredPkg );
		static bool					HandshakeInteractChannel ( Instance * env, bool isStunt, int & sock, struct sockaddr_in * addr, char * payload, AESContext * aes );
		static DeviceBase *			HandshakeDevice ( Instance * env, char * handshake, bool interactChannel, int & sock, struct sockaddr_in * addr, AESContext * aes );
		static DeviceBase *			GetDeviceForHandshake ( Instance * env, int deviceID, const char * areaName, const char * appName, bool interactChannel, int & sock, struct sockaddr_in * addr );

		virtual void				TuneReceiveBuffer ( int sock );
		virtual void				TuneSendBuffer ( int sock );

		int							SendComDatMessage ( void * msg, int length );
		bool						SendComDatBuffer ( char msgType, int fileID, const char * fileDescriptor, unsigned short payloadType, const char * rawData, size_t size  );

		int							SendHeaderedBuffer ( void * msg, int length );

        bool						SendDataPacket ( const char * msg, int length, struct sockaddr * dest);

		virtual bool				EvaluateDeviceConfig ( char * msg ) = 0;
        
        bool                        HandleNullMessage   ( environs::lib::ComMessageHeader * header, bool isComDatChannel );
        bool						HandlePortalMessage ( environs::lib::ComMessageHeader * header, bool isComDatChannel );
        bool						HandleOptionsMessage ( environs::lib::ComMessageHeader * header, bool isComDatChannel );
        bool						HandleOptionsResponse ( environs::lib::ComMessageHeader * header, bool isComDatChannel );
        bool						HandleStreamMessage ( environs::lib::ComMessageHeader * header, bool isComDatChannel );
        bool						HandleStringMessage ( environs::lib::ComMessageHeader * header, bool isComDatChannel );
        bool						HandleHeloMessage ( environs::lib::ComMessageHeader * header, bool isComDatChannel );
        bool						HandleFileTransfer ( environs::lib::ComMessageHeader * header, bool isComDatChannel );
        bool						HandleSensorData ( environs::lib::ComMessageHeader * header, bool isComDatChannel );                

		PortalDevice			*	GetPortalDeviceAccess ( int portalID );

		bool						CreatePortalGenerator ( int portalID );
		virtual void				CreatePortalGeneratorPlatform ( int portalIDdent );
        
        void                        HandleNullPortal ( int PortalID );
        void                        ProccessPortalProvidedStream ( int portalID );
        void                        ProccessPortalProvidedImages ( int portalID );
        void                        ProccessPortalRequestFailed ( int portalID );
        void                        ProccessPortalStop ( int portalID );
        
        virtual void				ProccessPortalStartAckBase ( int portalID );
        virtual void				ProccessPortalPauseAckBase ( int portalID );
        virtual void				ProccessPortalStopAckBase ( int portalID );

        void						ProccessPortalRequest ( int portalID, environs::lib::ComMessageHeader * header );
        void						ProccessPortalAskForRequest ( int portalID );
		void						ProccessPortalStart ( int portalID );
        
#ifdef ENABLE_PORTAL_STALL_MECHS
        void						ProccessPortalStall ( int portalID );
        void						ProccessPortalUnStall ( int portalID );
#endif
        void						ProccessPortaliFrameRequest ( int portalID );
        
		void						ProccessPortalPause ( int portalID );

		virtual void				ProccessPortalProvided ( int portalID, PortalStreamType_t streamType );
		virtual void				ProccessPortalStartAck ( int portalID );
		virtual void				ProccessPortalPauseAck ( int portalID );
		virtual void				ProccessPortalStopAck ( int portalID );

		virtual void				HandleOptionsMessage ( unsigned short payloadType, char * payload );
        void                        SyncPortalInfo ( int portalID );
        
		virtual void				OnPreConnectionEstablished ( );
		virtual void				OnUdpConnectionEstablished ();

		bool						PrepareStorage ();

		virtual void				OnInteractListenerClosed ();
		virtual void				OnConnectionEstablished ();
		virtual bool				SendDeviceConfig ();

		bool						AllocateInteractSocket ( );
		bool						AllocateComDatSocket ( );

		bool						AllocateUdpSocket ( );
		void 						CloseConnectorThread ();
		
		void                        CloseListeners ( bool wait = true );
		void 						CloseUdpListener ( bool wait = true );
		//void 						CloseComDatListener ( bool wait = true );
		//void 						CloseInteractListener ( bool wait = true );
        
	private:
        void                        UpdateConnectStatus ( int value, bool set = false );
        
		bool						Connect ( int Environs_CALL_ );
		void 		 				ConnectorThread ( int Environs_CALL_ );
		static void *				ConnectorThreadStarter ( void * arg );
		static int					GetConnectionToDevice ( Instance * env, volatile DeviceStatus_t * deviceStatus, int deviceID, const char * areaName, const char * appName, unsigned int IP, unsigned int IPe, int natStat );
		static int					DetectNATStatToDevice ( Instance * env, volatile DeviceStatus_t * deviceStatus, int deviceID, const char * areaName, const char * appName, unsigned int &IP, unsigned int &IPe, int &Port );
		
        bool						SaveToStorage ( int fileID, FILE ** fp, int pos, char * buffer, unsigned int length );
        bool						SaveToStorageDescription ( int fileID, const char * descriptor, unsigned int length );
        bool						SaveToStorageBuildPath ( int fileID, bool isBin );
        
        bool						SaveToStorageMessages ( const char * prefix, const char * message, int length );
		static bool					SaveToStorageMessages ( Instance * env, const char * prefix, int deviceID, const char * areaName, const char * appName, const char * message, int length );
        static bool					SaveToStorageMessages ( const char * prefix, const char * path, const char * message, int length );

		bool 						HandleHeloMessage ( unsigned short packetType, char * msg, unsigned int msgLen );
		bool 						HandleHeloMessageComDat ( unsigned short packetType, char * msg, unsigned int msgLen );
		

		bool 						InitiateInteractChannel ( );

		bool						StartInteractListener ( );
        
        bool						StartComDatListener ( );

		static int					SecureChannelEstablish ( Instance * env, int deviceID, int &sock, AESContext * aes, char * aesBlob );
		static int					SecureChannelProvide ( Instance * env, int &sock, AESContext * aes );

		bool						EstablishInteractChannel ( );
		bool						EstablishComDatChannel ( );

		bool						StartUdpListener ( );

        int							ReceiveOneMessage ( char * buffer, unsigned int bufferSize );
        
        unsigned int                PrepareHandshakeBuffer ( char * buffer );
        
        bool                        BringUpInteractThread ();

		void 		* 				InteractListener ( );
        static void *				InteractListenerStarter ( void * arg );
        
        void 		* 				ComDatListener ( );
        static void *				ComDatListenerStarter ( void * arg );
        
#ifdef ENABLE_DEVICEBASE_SINGLE_COMDAT_THREAD
		static bool                 StartComDat ();
		static void                 StopComDat ();
		
        static void *				ComDat ( void * arg );
        static void 				ComDatLoop ();
		void 		 				ComDatHandler ();

        static void 				ComDatSignal ();
        
        static bool                 BuildDevices ( DeviceBase ** &fdDevices, FDTYPE * &fds, int &size, int &capacity );
        static bool                 AddSignalSocket ( FDTYPE * fds, int &size );
#endif        
		void 		* 				UdpListener ( );
		static void *				UdpListenerStarter ( void * arg );

		bool 						HandleComDatChannel ( int sock, struct sockaddr_in * addr );
        
        bool 						InitUdpChannel ( );
        
        void						HandleSensorPacket ( char * buffer, int length );
	};


} /* namespace environs */


#endif


#endif /* INCLUDE_HCM_ENVIRONS_DEVICEBASE_H */

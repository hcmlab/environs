/**
 * Async Worker
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
#ifndef INCLUDE_HCM_ENVIRONS_ASYNCWORKER_H_
#define INCLUDE_HCM_ENVIRONS_ASYNCWORKER_H_

#include "Environs.Native.h"
#include "Device.Info.h"
#include "Interop/jni.h"
#include "Interop/Smart.Pointer.h"
#include "Queue.Vector.h"


namespace environs
{
#ifdef DISPLAYDEVICE
#   define	ASYNCWORK_SEND_POOL_SIZE            32
#else
#   define	ASYNCWORK_SEND_POOL_SIZE            12
#endif
    
#define	ASYNCWORK_TYPE_GET_MEDIATOR_VERSION		1
#define	ASYNCWORK_TYPE_RENEW_STUNT_SOCKETS		2
#define	ASYNCWORK_TYPE_REGISTER_STUNT_SOCKETS	3

#define	ASYNCWORK_TYPE_DEVICE_CONNECT           0x10
#define	ASYNCWORK_TYPE_DEVICE_UPDATE_XYANG      0x11
#define	ASYNCWORK_TYPE_DEVICE_UPDATE_ANG        0x12
#define	ASYNCWORK_TYPE_DEVICE_REMOVED           0x13
#define	ASYNCWORK_TYPE_DEVICE_REMOVED_ID        0x13
#define	ASYNCWORK_TYPE_DEVICE_DISCONNECT        0x16

#define	ASYNCWORK_PARAM_DEVICE_UPDATE_STATUS	0
#define	ASYNCWORK_PARAM_DEVICE_UPDATE_ANG		1
#define	ASYNCWORK_PARAM_DEVICE_UPDATE_XYANG		2

#define	ASYNCWORK_TYPE_SEND_FILE                0x21
#define	ASYNCWORK_TYPE_SEND_BUFFER              0x22
#define	ASYNCWORK_TYPE_SEND_MESSAGE             0x23
#define	ASYNCWORK_TYPE_SEND_UDP                 0x24


#define	ASYNCWORK_TYPE_PORTAL_INIT_REQUEST      0x31
#define	ASYNCWORK_TYPE_PORTAL_PROVIDE           0x32
#define	ASYNCWORK_TYPE_PORTAL_PROVIDE_REQUEST   0x33
#define	ASYNCWORK_TYPE_PORTAL_SEND_INIT         0x34

#define	ASYNCWORK_TYPE_PORTAL_RELEASE_RENDERSURFACE 0x35
#define	ASYNCWORK_TYPE_PORTAL_SET_RENDERCALLBACK 0x36

#define	ASYNCWORK_TYPE_PORTAL_START             0x37
#define	ASYNCWORK_TYPE_PORTAL_PAUSE             0x38
#define	ASYNCWORK_TYPE_PORTAL_STOP              0x39



#define	ASYNCWORK_TYPE_PORTAL_RELEASE_STREAMLOCK	0x51


#define	ASYNCWORK_TYPE_TRACKER_USAGE            0x80
#define	ASYNCWORK_TYPE_TRACKER_COMMAND          0x81
    
#define	ASYNCWORK_TYPE_STUN                     0x91
#define	ASYNCWORK_TYPE_SAVE_MEDIATOR_TOKENS     0x92
#define	ASYNCWORK_TYPE_DEVICE_FLAGS_UPDATE      0x93
#define	ASYNCWORK_TYPE_DEVICE_FLAGS_SYNC        0x94
#define	ASYNCWORK_TYPE_DEVICE_FLAGS_SYNC_ASYNC  0x95
    
//#define	ASYNCWORK_TYPE_SEND_MEDIATOR_ALIVE      0x97
#define	ASYNCWORK_TYPE_SENSOR_DATA              0x98



	typedef struct AsyncWork
	{
		int             deviceID;
		int				type;
		void        *   context;
	}
	AsyncWork;


	typedef struct AsyncWorkDevice
	{
		int             deviceID;
		int				type;
		void        *   context;
		int             arg0;  // also contextID/portalID
		int             arg1;  // also width
		int             arg2;  // also height
		float           argFloat;
		void        *   contextManaged;

		/** The area name of the appliction environment. */
		char			areaName [ MAX_LENGTH_AREA_NAME ]; // 31

		/** The applcation name of the appliction environment. */
		char			appName [ MAX_LENGTH_APP_NAME ]; // 31
	}
	AsyncWorkDevice;


	typedef struct AsyncWorkSend
	{
		bool			disposed;
        bool            isNativeID;
        int             deviceID;
		int				type;
		char        *   descriptor;
		int				fileID;
		char        *   filePath;
		int             bufferSize;

		/** The area name of the appliction environment. */
		char			areaName [ MAX_LENGTH_AREA_NAME ]; // 31

		/** The applcation name of the appliction environment. */
		char			appName [ MAX_LENGTH_APP_NAME ]; // 31
	}
	AsyncWorkSend;


	class Instance;

	/**
	*	Async Worker
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@created	01/05/15
	*	@version	1.0
	*	@remarks
	* ****************************************************************************************
	*/
	class AsyncWorker
	{
	private:
		bool                allocated;
		bool				active;
		Instance		*	env;

        ThreadSync          thread;
        
        bool                isRunning;

		lib::QueueVector	queue;
        
        bool                isSendRunning;
        volatile DeviceStatus_t deviceStatus;
        
        EnvSignal           sendThreadsSync;
        EnvThread           sendThreads [ ASYNCWORK_SEND_POOL_SIZE ];
        int                 sendThreadsRunning;

		bool				sendEnabled;
		lib::QueueVector	queueSend;

		AsyncWork       *	Pop ();
		AsyncWorkSend   *	PopSend ();

		static void		*	StartQueue ( void * arg );
		void			*	Worker ();
        sp ( Instance )     workerAlive;

		static void		*	StartSendQueue ( void * arg );
        void			*	WorkerSend ( int i );
        
        sp ( Instance )     workerSendAlives  [ ASYNCWORK_SEND_POOL_SIZE ];

	public:
		AsyncWorker ();
		~AsyncWorker ();

		bool				GetIsActive ();
		bool                Init ( Instance * obj );
		bool                Start ();
		void                Stop ();
		void                StopSend ();
		void                SignalStopSend ();

		/**
		* Start a particular send thread from the thread pool. A  call to this method must be locked/secured by the caller (on sendThread[0]).
		*
		* @param	i		The sender thread within the threadpool identified by an index.
		*/
		void                StartSend ( int i );
		void                Dispose ();

		void                Push ( int deviceID, unsigned int workType );

		void                Push ( AsyncWork * work );

		int                 Push ( int nativeID, unsigned int workType, int portalID, int x, int y, float angle );
		int                 Push ( int nativeID, unsigned int workType, int portalID, int x, int y );
		int                 Push ( int nativeID, unsigned int workType, int portalID, float angle );
		int                 Push ( int nativeID, unsigned int workType, int typeID );

		int                 Push ( int nativeID, unsigned int workType, int contextID, void * contextManaged, int contextAdd1, int contextAdd2 = 0 );
		int                 Push ( int deviceID, unsigned int workType, const char * areaName, const char * appName, int portalID, int x, int y, float angle );

		int                 PushPortal ( unsigned int workType, int contextID, void * contextManaged, int contextAdd1, int contextAdd2 = 0 );
		int                 PushPortal ( unsigned int workType, int contextID );
		int                 PushPortal ( unsigned int workType, int contextID, int contextAdd1, int contextAdd2 );
        
        bool                PushData ( unsigned int workType, int nativeID, void * data, int size );
        
		EBOOL				UpdateDevice ( int nativeID, int portalID, int updTypes, int x, int y, float angle, bool contactStatus );

		EBOOL               PortalRequest ( int nativeID, int portalDetails, int width, int height );

		EBOOL               PortalProvide ( int nativeID, int portalID );
		EBOOL               PortalProvideRequest ( int nativeID, int portalID );
		EBOOL               PortalStart ( int portalID );
		EBOOL               PortalPause ( int portalID );
		EBOOL               PortalStop ( int nativeID, int portalID );
		EBOOL               PortalSendInit ( int portalID, int width, int height );

		EBOOL               PortalSetRenderCallback ( int portalDeviceID, void * callback, int type );

		EBOOL               PortalSetRenderSurface ( int portalID, void * surface, int width, int height );
		EBOOL               PortalReleaseRenderSurface ( int portalDeviceID );

		int                 TrackerUsage ( const char * module, int command );
		EBOOL               TrackerCommand ( int module, int command );
        
        void                PushMediatorMsg ( char * msg, int workType );
        
        void				RegisterStuntSockets ( void * msg );
        
        void                SyncDeviceFlagsAsync ( int objID, int flags, bool set );
        
        /**
         * Update flags from mediator daemon to a particular device.
         *
         * @param	msgDec	privdes flags to set or clear. (of type DeviceFlagsInternal::Observer*)
         *					We change them to CPTypes of DeviceFlagsInternal
         */
		void				UpdateDeviceFlags ( char * msg );

		/**
		* Sync flags from a particular device to the Mediator server.
		*
		* @param	objID	The objID that identifies the target device.
		*/
		void				SyncDeviceFlags ( int objID );

        bool                SendDataUdp ( int nativeID, void * data, int size );
        
        bool                SendDataUdpPrefix ( int nativeID, void * data, int size );

		void                PushSend ( AsyncWorkSend * work );

		int                 PushSend ( int deviceID, unsigned int workType, const char * areaName, const char * appName,
			const char * msg, int length );
		int                 PushSend ( int nativeID, int fileID, const char * fileDescriptor, const char * filePath );

		int                 PushSend ( int nativeID, int fileID, const char * fileDescriptor, const char * buffer, int size );

		void                DisposeSends ( DeviceInfo * device );

	};

	extern AsyncWorker * asyncWorker;
}

#endif /* INCLUDE_HCM_ENVIRONS_ASYNCWORKER_H_ */

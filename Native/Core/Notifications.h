/**
 * Notification Queue
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
#ifndef INCLUDE_HCM_ENVIRONS_NOTIFICATIONQUEUE_H_
#define INCLUDE_HCM_ENVIRONS_NOTIFICATIONQUEUE_H_

#include "Interop/Threads.h"
#include "Device.Info.h"
#include "Environs.Native.h"
#include "Interop/Smart.Pointer.h"
#include "Queue.Vector.h"


namespace environs
{
	/// Moved to Environs.Obj.h
//#define	NOTIFY_TYPE_NOTIFY		0
//#define	NOTIFY_TYPE_STATUS		1
//#define	NOTIFY_TYPE_MSG			2
//#define	NOTIFY_TYPE_DATA		3
    
    int CopyAppEnvirons ( char * destAreaName, char * destAppName, const char * areaName, const char * appName );
    
	typedef struct EnvironsNotify {
		OBJIDType			sourceID;
		char			*	areaName;
		char			*	appName;
		int					type; // this may be used as percentage value
		int					notification;
		int					sourceIdent; // this may be used as fileID
        void			*   contextPtr;
		int					contextSize;
	}
	EnvironsNotify;

	typedef struct EnvironsNotifyExt {
		OBJIDType			sourceID;
		char			*	areaName;
		char			*	appName;
		int					type;
		int					notification;
        int					sourceIdent;
        void			*   contextPtr;
        int					contextSize;
        
        // Ext
        int                 context;

		int					fileID;
		void *				payload;
		//unsigned int		size;
	}
	EnvironsNotifyExt;
    
	typedef struct EnvironsRepsonse {
        int                 deviceID;
		unsigned int		responseID;
        pthread_cond_t 		responseEvent;
		void			 *	response;
		unsigned int	 	responseBytes;

		/** The area name of the application environment. */
		char				areaName [MAX_NAMEPROPERTY + 1]; // 31

		/** The application name of the application environment. */
		char				appName [MAX_NAMEPROPERTY + 1]; // 31

		EnvironsRepsonse *	next;
	}
	EnvironsRepsonse;


	class Instance;
    
	/**
	*	Notification Queue
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@created	06/22/13
	*	@version	1.0
	*	@remarks
	* ****************************************************************************************
	*/
	class NotificationQueue
	{
        friend class Core;
        
	private:
		bool				active;
        bool                allocated;
        int                 hEnvirons;
		Instance		*	env;

        unsigned int        responseCounter;
        pthread_mutex_t 	responseLock;

		EnvironsRepsonse *	responseQueue;
		EnvironsRepsonse *	responseQueueLast;
		lib::QueueVector	queue;

		//EnvironsNotify	*	locationUpdCache;

        bool                isRunning;
        ThreadSync          thread;
        
        void                DisposeNotif ( EnvironsNotify * notif );
        
        static void		*	StartQueueWorker ( void * arg );
        sp ( Instance )     workerAliveSP;
		void			*	Worker ();
        void				WorkTextNotify ( EnvironsNotify * notif, int deviceID, int notifyID );
        void				WorkNotify ( EnvironsNotify * notif, int deviceID, int notifyID );

		EnvironsNotify	*	Pop ();
        
        bool                PushResponse ( EnvironsRepsonse * er );
        bool                RemoveResponse ( unsigned int responseID );

		void				AddToQueue ( EnvironsNotify * notif, bool useLock = true );
        
        
	public:
		NotificationQueue ();
		~NotificationQueue ();

		bool                Init ( Instance * obj );
        bool                Start ();
        void                Stop ();
        
        void                Push ( OBJIDType objID, int notification );
        void                Push ( OBJIDType objID, int notification, int source, int type );
        void                Push ( OBJIDType objID, int sourceIdent, const void * buffer, int length );
        void                Push ( OBJIDType objID, int sourceIdent, bool comDat, char msgType, unsigned short payloadTyp, const void * payload, unsigned int payloadSize );
        void                PushData ( OBJIDType objID, int nativeID, int sourceIdent, int fileID, const char * descriptor, unsigned int descLen, unsigned int fileSize );
        
        void				PushContext ( OBJIDType objID, int notification, int source, void * context, int contextSize, bool useLock = true );
        void                PushContext ( OBJIDType objID, const char * areaName, const char * appName, int notification, void * context, int contextSize );               
        
        void                Push ( int deviceID, const char * areaName, const char * appName, int notification, int source );
        
        void                Push ( int deviceID, const char * areaName, const char * appName, int sourceIdent, const void * buffer, int length, const char * prefix );

		void                Dispose ();
        
		void                DisposeResonses ();
        
        unsigned int        GetResponse ( int nativeID, unsigned short optionID, unsigned int resultCapacity, void * result );

		void                HandleResponse ( unsigned int payloadSize, char * payload );
	};

	extern NotificationQueue * notificationQueue;
}

#endif /* INCLUDE_HCM_ENVIRONS_NOTIFICATIONQUEUE_H_ */
